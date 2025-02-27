/**
 * @file modbus_slave.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief modbus从机协议
 * @version 0.1
 * @date 2024-12-17
 *
 * @copyright Copyright (c) 2024
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "utils/crc.h"
#include "utils/log.h"
#include "utils/queue.h"
#include "protocol/modbus/modbus_slave.h"
#include <stdint.h>
#include <stdlib.h>

// 接收状态
enum rx_state {
	RX_STATE_ADDR = 0, // 地址
	RX_STATE_FUNC,	   // 功能码
	RX_STATE_INFO,	   // 数据信息
	RX_STATE_DATA,	   // 数据内容
	RX_STATE_CRC,	   // CRC校验
};

// 读数据帧
struct pdu_read {
	uint8_t reg_h;
	uint8_t reg_l;
	uint8_t num_h;
	uint8_t num_l;
};

// 写数据帧
struct pdu_write {
	uint8_t reg_h;
	uint8_t reg_l;
	uint8_t num_h;
	uint8_t num_l;
	uint8_t len;
};

// 接收缓冲
#define RX_BUFF_SIZE (MODBUS_FRAME_BYTES_MAX * 2)

// 接收数据信息
struct msg_info {
	uint8_t rx_queue_buff[RX_BUFF_SIZE]; // 接收队列缓冲
	union {
		struct pdu_read read;
		struct pdu_write write;
		uint8_t data[MODBUS_FRAME_BYTES_MAX];
	} pdu;					// 数据帧
	struct queue_info rx_q; // 接收队列

	size_t anchor;	// 滑动左窗口
	size_t forward; // 滑动右窗口

	uint16_t cal_crc; // 计算的CRC

	uint8_t addr; // 从机地址
	uint8_t func; // 功能码

	uint8_t pdu_in;	 // 接收索引
	uint8_t pdu_len; // 接收长度

	enum rx_state state; // 当前接收状态
};

// 从机
struct mb_slv {
	struct msg_info msg_state;						   // 接收信息
	uint8_t modbus_frame_buff[MODBUS_FRAME_BYTES_MAX]; // 回复缓冲
	uint16_t data_in_out[MAX_READ_REG_NUM];			   // 用户交互缓冲

	struct serial_opts *opts;		// 回调指针
	struct mb_slv_work *work_table; // 响应处理表
	size_t table_num;				// 响应处理表数量

	uint8_t slave_addr; // 从机地址
};

static bool _recv_parser(mb_slv_handle handle);			 // 解析数据
static uint16_t _dispatch_rtu_msg(mb_slv_handle handle); // 处理数据

// 剩余数据
static inline size_t check_rx_queue_remain_data(const struct msg_info *p_msg)
{
	return (p_msg->rx_q.wr - p_msg->forward);
}

// 获取队首数据
static inline uint8_t get_rx_queue_remain_data(const struct msg_info *p_msg)
{
	uint8_t *p = p_msg->rx_q.buf;
	return p[p_msg->forward & (p_msg->rx_q.buf_size - 1)];
}

/**
 * @brief 右移左滑动窗口
 * 
 * @param p_msg 
 */
static void rebase_parser(struct msg_info *p_msg)
{
	p_msg->state = RX_STATE_ADDR;
	p_msg->rx_q.rd = p_msg->anchor + 1;

	p_msg->anchor = p_msg->rx_q.rd;
	p_msg->forward = p_msg->rx_q.rd;
}

/**
 * @brief 刷新解析状态
 * 
 * @param p_msg 
 */
static void flush_parser(struct msg_info *p_msg)
{
	p_msg->state = RX_STATE_ADDR;

	p_msg->rx_q.rd = p_msg->forward;
	p_msg->anchor = p_msg->rx_q.rd;
}

// 获取读/写帧的信息长度
static uint8_t get_pdu_mini_len(uint8_t func)
{
	switch (func) {
	case MODBUS_FUN_RD_REG_MUL:
		return sizeof(struct pdu_read);
	case MODBUS_FUN_WR_REG_MUL:
		return sizeof(struct pdu_write);
	default:
		return 0;
	}
}

/**
 * @brief 获取需要接收的数据长度
 * 
 * @param p_msg 
 * @return uint8_t 
 */
static uint8_t get_pdu_extern_len(const struct msg_info *p_msg)
{
	uint16_t len = 0;
	uint16_t reg_num;

	if (p_msg->func != MODBUS_FUN_RD_REG_MUL) {
		len += p_msg->pdu.write.len;
		reg_num = COMBINE_U8_TO_U16(p_msg->pdu.write.num_h, p_msg->pdu.write.num_l);

		if ((p_msg->pdu.write.len != (reg_num << 1)) || (len > MODBUS_FRAME_BYTES_MAX))
			len = 0;
	}
	return len;
}

static uint8_t check_user_err_code(uint8_t err_code)
{
	return (err_code <= MODBUS_RESP_ERR_BUSY ? err_code : MODBUS_RESP_ERR_BUSY);
}

/**
 * @brief 解析协议数据帧, 支持粘包断包处理
 * 
 * @param handle 从机句柄
 * @return true 解析成功
 * @return false 解析失败
 */
static bool _recv_parser(mb_slv_handle handle)
{
	if (!handle)
		return false;

	uint8_t c;
	uint8_t pdu_ex_len = 0;

	struct msg_info *p_msg = &handle->msg_state;

	while (check_rx_queue_remain_data(p_msg)) {
		c = get_rx_queue_remain_data(p_msg);
		++p_msg->forward;
		switch (p_msg->state) {
		case RX_STATE_ADDR:
			if (handle->slave_addr == c) {
				handle->msg_state.addr = c;
				p_msg->state = RX_STATE_FUNC;
				p_msg->cal_crc = crc16_update(0xffff, c);
			} else
				rebase_parser(p_msg);
			break;
		case RX_STATE_FUNC:
			if (MODBUS_FUNC_CHECK_VALID(c)) {
				p_msg->state = RX_STATE_INFO;
				p_msg->func = c;
				p_msg->pdu_in = 0;
				p_msg->pdu_len = get_pdu_mini_len(c);
				p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			} else
				rebase_parser(p_msg);
			break;
		case RX_STATE_INFO:
			p_msg->pdu.data[p_msg->pdu_in++] = c;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				if (p_msg->func == MODBUS_FUN_RD_REG_MUL) {
					p_msg->pdu_len += MODBUS_CRC_BYTES_NUM;
					p_msg->state = RX_STATE_CRC;
					break;
				} else {
					pdu_ex_len = get_pdu_extern_len(p_msg);
					if (!pdu_ex_len)
						rebase_parser(p_msg);
					else {
						p_msg->pdu_len += pdu_ex_len;
						p_msg->state = RX_STATE_DATA;
					}
				}
			}
			break;
		case RX_STATE_DATA:
			p_msg->pdu.data[p_msg->pdu_in++] = c;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				p_msg->pdu_len += MODBUS_CRC_BYTES_NUM;
				p_msg->state = RX_STATE_CRC;
			}
			break;
		case RX_STATE_CRC:
			p_msg->pdu.data[p_msg->pdu_in++] = c;
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				if (p_msg->cal_crc ==
					COMBINE_U8_TO_U16(p_msg->pdu.data[p_msg->pdu_in - 1], p_msg->pdu.data[p_msg->pdu_in - 2])) {
					flush_parser(p_msg);
					return true;
				} else
					rebase_parser(p_msg);
			}
			break;
		default:
			break;
		}
	}
	return false;
}

/**
 * @brief 处理注册回调
 *
 * @param handle 从机句柄
 * @return uint8_t 参考头文件响应码
 */
static uint8_t _rtu_handle(mb_slv_handle handle)
{
	if (!handle)
		return MODBUS_RESP_ERR_BUSY;

	struct mb_slv_work *work = NULL; // 注册回调处理

	int res = MODBUS_RESP_ERR_BUSY;

	uint8_t addr = handle->msg_state.addr;
	uint8_t func = handle->msg_state.func;
	uint16_t reg = handle->msg_state.pdu.read.reg_h << 8 | handle->msg_state.pdu.read.reg_l;
	uint16_t reg_num = handle->msg_state.pdu.read.num_h << 8 | handle->msg_state.pdu.read.num_l;

	for (uint16_t i = 0; i < handle->table_num; i++) {
		work = &handle->work_table[i];
		if (work && work->resp && MODBUS_CHECK_REG_RANGE(reg, reg_num, work->start, work->end, func)) {
			res = work->resp(func, reg, reg_num, handle->data_in_out); // 用户回调处理
			break;
		}
	}

	return res;
}

/**
 * @brief 处理读功能码
 *
 * @param handle 从机句柄
 * @return uint16_t 响应长度
 */
static uint16_t _packet_ack_read_frame(mb_slv_handle handle)
{
	if (!handle)
		return 0;

	uint16_t pkt_len = 0;
	uint16_t crc = 0xffff;
	uint16_t *p;

	uint16_t reg = COMBINE_U8_TO_U16(handle->msg_state.pdu.read.reg_h, handle->msg_state.pdu.read.reg_l);
	uint16_t reg_num = COMBINE_U8_TO_U16(handle->msg_state.pdu.read.num_h, handle->msg_state.pdu.read.num_l);

	uint8_t *pdata_out = handle->modbus_frame_buff; // 存储回复的数据

	pdata_out[pkt_len++] = handle->msg_state.addr;
	uint8_t usr_err = _rtu_handle(handle);
	if (usr_err == MODBUS_RESP_ERR_NONE) {
		p = handle->data_in_out; // 用户响应的数据

		pdata_out[pkt_len++] = MODBUS_FUN_RD_REG_MUL; // 读功能码
		pdata_out[pkt_len++] = (reg_num << 1);		  // 数据长度

		for (uint16_t i = 0; i < reg_num; i++, p++) {
			pdata_out[pkt_len++] = GET_U8_HIGH_FROM_U16(*p);
			pdata_out[pkt_len++] = GET_U8_LOW_FROM_U16(*p);
		}
	} else {
		pdata_out[pkt_len++] = handle->msg_state.func | 0x80; // 错误帧
		pdata_out[pkt_len++] = usr_err;
	}

	crc = crc16_update_bytes(crc, pdata_out, pkt_len);

	pdata_out[pkt_len++] = GET_U8_LOW_FROM_U16(crc);
	pdata_out[pkt_len++] = GET_U8_HIGH_FROM_U16(crc);

	return pkt_len;
}

/**
 * @brief 处理写功能码
 *
 * @param handle 从机句柄
 * @return uint16_t 响应长度
 */
static uint16_t _packet_ack_write_frame(mb_slv_handle handle)
{
	if (!handle)
		return 0;

	uint16_t pkt_len = 0;
	uint16_t crc = 0xffff;

	uint8_t usr_err = MODBUS_RESP_ERR_BUSY; // 用户响应结果

	uint16_t reg = COMBINE_U8_TO_U16(handle->msg_state.pdu.write.reg_h, handle->msg_state.pdu.write.reg_l);
	uint16_t reg_num = COMBINE_U8_TO_U16(handle->msg_state.pdu.write.num_h, handle->msg_state.pdu.write.num_l);

	uint8_t *pdata_out = handle->modbus_frame_buff; // 存储响应数据

	uint8_t func = handle->msg_state.func;

	pdata_out[pkt_len++] = handle->msg_state.addr;

	usr_err = _rtu_handle(handle); // 注册回调处理
	if (usr_err == MODBUS_RESP_ERR_NONE) {
		pdata_out[pkt_len++] = MODBUS_FUN_WR_REG_MUL;
		pdata_out[pkt_len++] = GET_U8_HIGH_FROM_U16(reg);
		pdata_out[pkt_len++] = GET_U8_LOW_FROM_U16(reg);

		pdata_out[pkt_len++] = GET_U8_HIGH_FROM_U16(reg_num);
		pdata_out[pkt_len++] = GET_U8_LOW_FROM_U16(reg_num);
	} else {
		pdata_out[pkt_len++] = handle->msg_state.func | 0x80; // 错误帧
		pdata_out[pkt_len++] = usr_err;
	}

	crc = crc16_update_bytes(crc, pdata_out, pkt_len);
	pdata_out[pkt_len++] = GET_U8_LOW_FROM_U16(crc);
	pdata_out[pkt_len++] = GET_U8_HIGH_FROM_U16(crc);

	return pkt_len;
}

/**
 * @brief 处理对应功能码
 *
 * @param handle 从机句柄
 * @return uint16_t 回复响应的数据长度
 */
static uint16_t _dispatch_rtu_msg(mb_slv_handle handle)
{
	if (!handle)
		return 0;

	uint16_t ptk_len = 0;  // 处理结果长度
	uint16_t reg, reg_num; // 寄存器地址, 长度
	uint8_t data_len;

	const uint8_t *p;

	struct msg_info *p_msg = &handle->msg_state; // 接收数据

	switch (p_msg->func) {
	case MODBUS_FUN_RD_REG_MUL:
		return _packet_ack_read_frame(handle); // 读功能码

	case MODBUS_FUN_WR_REG_MUL:
		data_len = p_msg->pdu.write.len;

		// 写入数据
		p = &p_msg->pdu.data[sizeof(struct pdu_write)];
		for (uint8_t i = 0, j = 0; i < data_len; i += 2, j++) {
			handle->data_in_out[j] = (*p++ << 8);
			handle->data_in_out[j] |= *p++;
		}

		return _packet_ack_write_frame(handle); // 写功能码

	default:
		return 0;
	}
}

/***************************API***************************/

/**
 * @brief 从机初始化并申请句柄
 *
 * @param opts 				读写等回调函数指针
 * @param slv_addr 			从机地址
 * @param table 			任务处理表
 * @param table_num 		表长
 * @return mb_slv_handle 	成功返回句柄，失败返回NULL
 */
mb_slv_handle mb_slv_init(
	struct serial_opts *opts, uint8_t slv_addr, struct mb_slv_work *work_table, uint16_t table_num)
{
	bool ret = false;

	if (!opts || !opts->f_init || !opts->f_read || !opts->f_write)
		return NULL;

	struct mb_slv *handle = calloc(1, sizeof(struct mb_slv));
	if (!handle)
		return NULL;

	handle->opts = opts;
	handle->work_table = work_table;
	handle->table_num = table_num;
	handle->slave_addr = slv_addr;

	ret = queue_init(&handle->msg_state.rx_q, sizeof(uint8_t), handle->msg_state.rx_queue_buff, RX_BUFF_SIZE);
	if (!ret) {
		free(handle);
		return NULL;
	}

	ret = opts->f_init();
	if (!ret) {
		free(handle);
		return NULL;
	}

	return handle;
}

/**
 * @brief 释放从机句柄
 *
 * @param handle
 */
void mb_slv_destroy(mb_slv_handle handle)
{
	if (!handle)
		return;

	free(handle);
}

/**
 * @brief modbus从机轮询函数
 * 
 * @param handle 从机句柄
 */
void mb_slv_poll(mb_slv_handle handle)
{
	if (!handle)
		return;

	size_t ptk_len = handle->opts->f_read(handle->modbus_frame_buff, MODBUS_FRAME_BYTES_MAX);
	if (!ptk_len) // 无数据
		return;

	size_t ret_q = queue_add(&(handle->msg_state.rx_q), handle->modbus_frame_buff, ptk_len);
	if (ret_q != ptk_len)
		return; // 空间不足

	bool ret_parser = _recv_parser(handle);
	if (!ret_parser)
		return; // 无完整帧

	ptk_len = _dispatch_rtu_msg(handle);
	if (!ptk_len)
		return; // 无回复数据

	handle->opts->f_write(handle->modbus_frame_buff, ptk_len); // 回复主机
}
