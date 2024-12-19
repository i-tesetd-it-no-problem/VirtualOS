/**
 * @file modbus_master.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief modbus主机协议
 * @version 1.0
 * @date 2024-12-18
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

#include "crc.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "modbus_master.h"

#define REPEAT_IDX (0)	// 重发
#define TIMEOUT_IDX (1) // 超时

// 接收状态
enum rx_state {
	RX_STATE_ADDR = 0, // 从机地址
	RX_STATE_FUNC,	   // 功能码

	RX_STATE_DATA_LEN, // 数据长度      (仅对 读 功能玛有效)
	RX_STATE_DATA,	   // 数据内容      (仅对 读 功能玛有效)

	RX_STATE_REG,	  // 寄存器地址    (仅对 写 功能玛有效)
	RX_STATE_REG_LEN, // 寄存器长度    (仅对 写 功能玛有效)

	RX_STATE_CRC, // CRC校验
};
// 两倍最大帧长度 接收缓冲
#define RX_BUFF_SIZE (MODBUS_FRAME_BYTES_MAX * 2)

// 最大请求处理个数
#define MAX_REQUEST (16)
#define REQUEST_BUFFER (MAX_REQUEST * sizeof(struct mb_mst_request *))

// 接收数据信息
struct msg_info {
	uint8_t pdu_in;		 // 接收索引
	uint8_t pdu_len;	 // 接收长度
	uint16_t cal_crc;	 // 计算的CRC
	enum rx_state state; // 当前接收状态

	size_t anchor;	// 滑动左窗口
	size_t forward; // 滑动右窗口

	struct queue_info rx_q;				 // 接收队列
	uint8_t rx_queue_buff[RX_BUFF_SIZE]; // 接收队列缓冲

	struct queue_info tx_q;				   // 发送队列
	uint8_t tx_queue_buff[REQUEST_BUFFER]; // 发送队列缓冲

	uint8_t recv_crc[MODBUS_CRC_BYTES_NUM]; // 接收的CRC
	uint8_t r_data[MODBUS_REG_NUM_MAX * 2]; // 读功能码接收的有效数据
	uint8_t r_data_len;						// 有效数据长度
};

// 主机句柄
struct mb_mst {
	struct serial_opts *opts;  // 用户回调指针
	struct msg_info msg_state; // 接收信息
	size_t period_ms;		   // 任务周期
	bool is_sending;		   // 正在发送
};

static bool _recv_parser(mb_mst_handle handle);		 // 解析数据
static void _dispatch_rtu_msg(mb_mst_handle handle); // 处理数据

// 剩余数据
static inline size_t check_rx_queue_remain_data(const struct msg_info *p_msg)
{
	return (p_msg->rx_q.wr - p_msg->forward);
}

// 获取队首数据
static inline size_t get_rx_queue_remain_data(const struct msg_info *p_msg)
{
	return (p_msg->rx_q.buf[p_msg->forward & (p_msg->rx_q.buf_size - 1)]);
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

/**
 * @brief 检查请求包是否合法
 * 
 * @param request 
 * @return true 
 * @return false 
 */
static bool check_request_valid(struct mb_mst_request *request)
{
	// 空指针 无响应回调 寄存器范围过大 未设置超时时间
	if (!request || !request->resp || request->reg_len > MODBUS_REG_NUM_MAX || !request->timeout_ms)
		return false;

	switch (request->func) {
	case MODBUS_FUN_RD_REG_MUL:
		// 读功能玛
		break;
	case MODBUS_FUN_WR_REG_MUL:
		// 写功能码

		// 无数据 数据长度为0 写入寄存器长度超过buffer最大长度
		if (!request->data || !request->data_len || request->reg_len * 2 > request->data_len)
			return false;

		break;
	default:
		return false;
	}

	return true;
}

/**
 * @brief 解析协议数据帧, 支持粘包断包处理
 * 
 * @param handle 主机句柄
 * @return true 解析成功
 * @return false 解析失败
 */
static bool _recv_parser(mb_mst_handle handle)
{
	// 无请求不接收
	if (!handle || is_queue_empty(&handle->msg_state.tx_q))
		return false;

	uint8_t c;

	struct msg_info *p_msg = &handle->msg_state;
	struct mb_mst_request *request = NULL;
	queue_peek(&handle->msg_state.tx_q, (uint8_t *)&request, 1); // 不出队 只查询

	while (check_rx_queue_remain_data(p_msg)) {
		c = get_rx_queue_remain_data(p_msg);
		++p_msg->forward;
		switch (p_msg->state) {
		case RX_STATE_ADDR:
			if (request->slave_addr == c) {
				p_msg->state = RX_STATE_FUNC;
				p_msg->cal_crc = crc16_update(0xffff, c);
			} else
				rebase_parser(p_msg);
			break;
		case RX_STATE_FUNC:
			if (c == MODBUS_FUN_RD_REG_MUL) {
				p_msg->state = RX_STATE_DATA_LEN;
				p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);

			} else if (c == MODBUS_FUN_WR_REG_MUL) {
				p_msg->pdu_in = 0;
				p_msg->pdu_len = MODBUS_REG_BYTES_NUM;
				p_msg->state = RX_STATE_REG;
				p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			} else
				rebase_parser(p_msg);
			break;

		case RX_STATE_DATA_LEN:
			p_msg->pdu_in = 0;
			p_msg->pdu_len = c;
			p_msg->state = RX_STATE_DATA;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			break;

		case RX_STATE_DATA:
			p_msg->r_data[p_msg->pdu_in++] = c;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				p_msg->r_data_len = p_msg->pdu_in;
				p_msg->pdu_in = 0;
				p_msg->pdu_len = MODBUS_CRC_BYTES_NUM;
				p_msg->state = RX_STATE_CRC;
			}

			break;

		case RX_STATE_REG:
			p_msg->pdu_in++;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				p_msg->pdu_in = 0;
				p_msg->pdu_len = MODBUS_REG_LEN_BYTES_NUM;
				p_msg->state = RX_STATE_REG_LEN;
			}

			break;

		case RX_STATE_REG_LEN:
			p_msg->pdu_in++;
			p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				p_msg->pdu_in = 0;
				p_msg->pdu_len = MODBUS_CRC_BYTES_NUM;
				p_msg->state = RX_STATE_CRC;
			}
			break;

		case RX_STATE_CRC:
			p_msg->recv_crc[p_msg->pdu_in++] = c;
			if (p_msg->pdu_in >= p_msg->pdu_len) {
				uint16_t recv_crc = COMBINE_U8_TO_U16(p_msg->recv_crc[1], p_msg->recv_crc[0]);
				if (p_msg->cal_crc == recv_crc) {
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
 * @brief 处理对应功能码
 *
 * @param handle 主机句柄
 */
static void _dispatch_rtu_msg(mb_mst_handle handle)
{
	if (!handle || is_queue_empty(&handle->msg_state.tx_q))
		return;

	struct mb_mst_request *request = NULL;

	// 出队请求
	queue_get(&handle->msg_state.tx_q, (uint8_t *)&request, 1);
	request->resp(
		handle->msg_state.r_data, handle->msg_state.r_data_len, false); // 用户回调(未超时)
}

/**
 * @brief 检查当前请求回复是否超时
 * 
 * @param handle 主机句柄
 * @param request 请求包
 * @return true 超时
 * @return false 未超时
 */
static bool check_timeout(mb_mst_handle handle, struct mb_mst_request *request)
{
	if (!handle || !check_request_valid(request))
		return false;

	bool ret = (request->_hide_[TIMEOUT_IDX] == 0) ? true : false; // 0超时 初始也算一次

	request->_hide_[TIMEOUT_IDX] += handle->period_ms;
	if (request->_hide_[TIMEOUT_IDX] >= request->timeout_ms)
		request->_hide_[TIMEOUT_IDX] = 0;

	return ret;
}

/**
 * @brief 发送请求
 * 
 * @param handle 主机句柄
 * @param request 请求包
 */
static void _request_pdu(mb_mst_handle handle, struct mb_mst_request *request)
{
	if (!handle || !check_request_valid(request))
		return;

	uint8_t temp_buf[256] = { 0 };
	uint16_t idx = 0;
	temp_buf[idx++] = request->slave_addr;
	temp_buf[idx++] = request->func;

	temp_buf[idx++] = GET_U8_HIGH_FROM_U16(request->reg_addr);
	temp_buf[idx++] = GET_U8_LOW_FROM_U16(request->reg_addr);
	temp_buf[idx++] = GET_U8_HIGH_FROM_U16(request->reg_len);
	temp_buf[idx++] = GET_U8_LOW_FROM_U16(request->reg_len);

	// 写功能玛
	if (request->func == MODBUS_FUN_WR_REG_MUL) {
		uint8_t data_len = request->reg_len * 2;
		temp_buf[idx++] = data_len;

		memcpy(&temp_buf[idx], request->data, data_len);
		idx += data_len;
	}

	uint16_t crc = crc16_update_bytes(0xFFFF, temp_buf, idx);

	temp_buf[idx++] = GET_U8_LOW_FROM_U16(crc);
	temp_buf[idx++] = GET_U8_HIGH_FROM_U16(crc);

	handle->opts->f_dir_ctrl(modbus_serial_dir_tx_only); // 切换到发送模式
	handle->opts->f_write(temp_buf, idx);

	if (handle->opts->f_check_send)
		handle->is_sending = true; // DMA发送
	else
		handle->opts->f_dir_ctrl(modbus_serial_dir_rx_only); // 轮训发送
}

/**
 * @brief 从发送队列中取出数据, 并发送
 * 
 * @param handle 
 */
static void master_write(mb_mst_handle handle)
{
	if (!handle || is_queue_empty(&handle->msg_state.tx_q))
		return;

	struct mb_mst_request *request = NULL;
	queue_peek(&handle->msg_state.tx_q, (uint8_t *)&request, 1); // 不出队 只查询

	// 需要重发
	if (request->_hide_[REPEAT_IDX] < MASTER_REPEATS) {
		if (!check_timeout(handle, request))
			return;

		_request_pdu(handle, request); // 每超时一次发送一次
	} else {
		// 重发完 才出队
		request->_hide_[REPEAT_IDX] = 0;
		queue_get(&handle->msg_state.tx_q, (uint8_t *)&request, 1);
		request->resp(
			handle->msg_state.r_data, handle->msg_state.r_data_len, true); // 用户回调(超时)
	}
}

static void master_read(mb_mst_handle handle)
{
	if (!handle)
		return;

	size_t ptk_len; // 响应长度

	uint8_t temp_buf[MODBUS_FRAME_BYTES_MAX] = { 0 };

	ptk_len = handle->opts->f_read(temp_buf, MODBUS_FRAME_BYTES_MAX);
	if (!ptk_len) // 无数据
		return;

	size_t ret_q = queue_add(&(handle->msg_state.rx_q), temp_buf, ptk_len);
	if (ret_q != ptk_len)
		return; // 空间不足

	bool ret_parser = _recv_parser(handle); // 解析数据帧

	if (!ret_parser)
		return;

	_dispatch_rtu_msg(handle); // 处理数据帧, 调用用户回调注册表
}

/***************************API***************************/

/**
 * @brief 主机初始化并申请句柄
 *
 * @param opts 读写等回调函数指针
 * @param period_ms 轮训周期
 * @return mb_mst_handle 成功返回句柄,失败返回NULL
 */
mb_mst_handle mb_mst_init(struct serial_opts *opts, size_t period_ms)
{
	bool ret = false;

	if (!opts || !opts->f_init || !opts->f_read || !opts->f_write || !opts->f_dir_ctrl)
		return NULL;

	struct mb_mst *handle = calloc(1, sizeof(struct mb_mst));
	if (!handle)
		return NULL;

	handle->opts = opts;
	handle->period_ms = period_ms;
	handle->is_sending = false;

	// 接收队列
	ret = queue_init(
		&handle->msg_state.rx_q, sizeof(uint8_t), handle->msg_state.rx_queue_buff, RX_BUFF_SIZE);
	if (!ret) {
		free(handle);
		return NULL;
	}

	// 发送队列
	ret = queue_init(&handle->msg_state.tx_q, sizeof(struct mb_mst_request *),
		handle->msg_state.tx_queue_buff, MAX_REQUEST);
	if (!ret) {
		free(handle);
		return NULL;
	}

	// 用户串口初始化
	ret = opts->f_init();
	if (!ret) {
		free(handle);
		return NULL;
	}

	opts->f_dir_ctrl(modbus_serial_dir_rx_only);

	return handle;
}

/**
 * @brief 释放主机句柄
 *
 * @param handle
 */
void mb_mst_destroy(mb_mst_handle handle)
{
	if (!handle)
		return;

	free(handle);
}

/**
 * @brief modbus主机轮询函数
 *
 * @param handle 主机句柄
 */
void mb_mst_poll(mb_mst_handle handle)
{
	if (!handle)
		return;

	// 发送中
	if (handle->opts->f_check_send && handle->is_sending) {
		bool complete = handle->opts->f_check_send();
		if (complete) {
			handle->is_sending = false;
			handle->opts->f_dir_ctrl(modbus_serial_dir_rx_only); // 发送完成, 切回接收
		}
		return;
	}

	master_write(handle); // 发送处理

	master_read(handle); // 接收处理
}

/**
 * @brief 主机发送报文
 * 
 * @param handle 主机句柄
 * @param request 请求结构体
 */
void mb_mst_pdu_request(mb_mst_handle handle, struct mb_mst_request *request)
{
	if (!handle || !check_request_valid(request))
		return;

	request->_hide_[REPEAT_IDX] = 0;
	request->_hide_[TIMEOUT_IDX] = 0;
	queue_add(&handle->msg_state.tx_q, (uint8_t *)&request, 1);
}