/**
 * @file modbus_slave.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief ModBus协议从机组件
 * @version 1.0
 * @date 2024-08-12
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#include "modbus_slave.h"

#define RX_BUFF_SIZE (512)
static uint8_t rx_queue_buff[RX_BUFF_SIZE];
static uint8_t modbus_frame_buff[MODBUS_FRAME_BYTES_MAX];

#define MODBUS_SPECIAL_VALUE (0xF4)

typedef enum {
	RX_STATE_ADDR,
	RX_STATE_FUNC,
	RX_STATE_INFO,
	RX_STATE_DATA,
	RX_STATE_CRC,
} RX_STATE_E;

typedef struct {
	uint8_t reg_h;
	uint8_t reg_l;
	uint8_t num_h;
	uint8_t num_l;
} PDU_READ_T;

typedef struct {
	uint8_t reg_h;
	uint8_t reg_l;
	uint8_t num_h;
	uint8_t num_l;
	uint8_t len;
} PDU_WRITE_T;

typedef struct {
	uint8_t func;
	uint8_t pdu_in;
	uint8_t pdu_len;

	uint16_t cal_crc;
	RX_STATE_E state;
	q_size anchor;
	q_size forward;

	queue_info_t rx_q;
	union {
		PDU_READ_T read;
		PDU_WRITE_T write;
		uint8_t data[MODBUS_DATA_BYTES_MAX];
	} pdu;

} msg_state_t;

typedef struct {
	uint8_t len;
	uint8_t slave_addr;
	volatile uint8_t start_flag;
	modbus_serial_opt_t *p_opts;
	rtu_address_validator f_validator;
	uint16_t data_in_out[MODBUS_REG_NUM_MAX];
	msg_state_t msg_state;
} modbus_slave_t;

typedef struct {
	uint16_t num;
	modbus_slave_handler_t *table;
} modbus_slave_handler_table_t;

static modbus_slave_t m_slave = { .start_flag = MODBUS_SPECIAL_VALUE };
static modbus_slave_handler_table_t m_slave_table = { .num = 0 };

static int _recv_parser(msg_state_t *p_msg);
static uint16_t _dispatch_rtu_msg(const msg_state_t *p_msg, uint8_t *ack_frame);

static inline int is_modbus_not_start(void)
{
	return (m_slave.start_flag == MODBUS_SPECIAL_VALUE);
}

int modbus_slave_init(modbus_serial_opt_t *p_serial_opt, rtu_address_validator f_validator)
{
	int ret_code = 0;

	if (!is_modbus_not_start() || !p_serial_opt || !f_validator || !p_serial_opt->f_init || !p_serial_opt->f_read || !p_serial_opt->f_write ||
	    !p_serial_opt->f_dir_ctrl || !p_serial_opt->f_flush) {
		return -1;
	}

	m_slave.f_validator = f_validator;
	m_slave.p_opts = p_serial_opt;
	queue_init(&m_slave.msg_state.rx_q, sizeof(uint8_t), rx_queue_buff, RX_BUFF_SIZE);
	m_slave.msg_state.anchor = 0;
	m_slave.msg_state.forward = 0;
	m_slave.msg_state.state = RX_STATE_ADDR;
	ret_code = p_serial_opt->f_init();

	if (ret_code == 0) {
		p_serial_opt->f_dir_ctrl(modbus_serial_dir_rx_only);
		m_slave.start_flag = 0;
	}

	return ret_code;
}

void modbus_slave_set_table(modbus_slave_handler_t *p_handler_table, uint16_t num)
{
	m_slave_table.table = p_handler_table;
	m_slave_table.num = num;
}

void modbus_slave_poll(void)
{
	int ptk_len;
	int ret_code;

	if (is_modbus_not_start()) {
		return;
	}

	ptk_len = m_slave.p_opts->f_read(modbus_frame_buff, MODBUS_FRAME_BYTES_MAX);

	if (ptk_len > 0) {
		ret_code = queue_add(&(m_slave.msg_state.rx_q), modbus_frame_buff, ptk_len);

		if (ret_code != ptk_len) {
			;
		}

		ret_code = _recv_parser(&(m_slave.msg_state));

		if (ret_code == 0) {
			ptk_len = _dispatch_rtu_msg(&(m_slave.msg_state), modbus_frame_buff);
			m_slave.p_opts->f_flush();

			if (ptk_len > 0) {
				m_slave.p_opts->f_dir_ctrl(modbus_serial_dir_tx_only);
				m_slave.p_opts->f_write(modbus_frame_buff, ptk_len);

				//m_slave.p_opts->f_dir_ctrl(modbus_serial_dir_rx_only); //DMA串口发送情况下建议注释此句

				/**
				* 若使用4DMA， 建议在DMA完成中断中切换发送相关引脚,
				* 可使用stimer组件中的defer_task_create接口，异步延时1-2ms切换发送引脚
				*/
			}
		}
	}
}

static inline q_size check_rx_queue_remain_data(const msg_state_t *p_msg)
{
	return (p_msg->rx_q.wr - p_msg->forward);
}

static inline q_size get_rx_queue_remain_data(const msg_state_t *p_msg)
{
	return (p_msg->rx_q.buf[p_msg->forward & (p_msg->rx_q.buf_size - 1)]);
}

static void rebase_parser(msg_state_t *p_msg)
{
	p_msg->state = RX_STATE_ADDR;
	p_msg->rx_q.rd = p_msg->anchor + 1;
	p_msg->anchor = p_msg->rx_q.rd;
	p_msg->forward = p_msg->rx_q.rd;
}

static void flush_parser(msg_state_t *p_msg)
{
	p_msg->state = RX_STATE_ADDR;
	p_msg->rx_q.rd = p_msg->forward;
	p_msg->anchor = p_msg->rx_q.rd;
}

static uint8_t get_pdu_mini_len(uint8_t func)
{
	uint8_t len = 0;

	switch (func) {
	case MODBUS_FUN_RD_REG_MUL:
		len = sizeof(PDU_READ_T);
		break;

	case MODBUS_FUN_WR_REG_MUL:
		len = sizeof(PDU_WRITE_T);
		break;

	default:
		break;
	}

	return len;
}

static uint8_t get_pdu_extern_len(const msg_state_t *p_msg)
{
	uint16_t len = 0;
	uint16_t reg_num;

	if (p_msg->func != MODBUS_FUN_RD_REG_MUL) {
		len += p_msg->pdu.write.len;
		reg_num = COMBINE_U8_TO_U16(p_msg->pdu.write.num_h, p_msg->pdu.write.num_l);

		if ((p_msg->pdu.write.len != (reg_num << 1)) || (len > MODBUS_DATA_BYTES_MAX)) {
			len = 0;
		}
	}

	return len;
}

static int _recv_parser(msg_state_t *p_msg)
{
	uint8_t c;
	uint8_t pdu_ex_len = 0;

	while (check_rx_queue_remain_data(p_msg)) {
		c = get_rx_queue_remain_data(p_msg);
		++p_msg->forward;

		switch (p_msg->state) {
		case RX_STATE_ADDR:
			if (m_slave.f_validator(c)) {
				m_slave.slave_addr = c;
				p_msg->state = RX_STATE_FUNC;
				p_msg->cal_crc = crc16_update(0xffff, c);
			} else {
				rebase_parser(p_msg);
			}

			break;

		case RX_STATE_FUNC:
			if (MODBUS_FUNC_CHECK_VALID(c)) {
				p_msg->state = RX_STATE_INFO;
				p_msg->func = c;
				p_msg->pdu_in = 0;
				p_msg->pdu_len = get_pdu_mini_len(c);
				p_msg->cal_crc = crc16_update(p_msg->cal_crc, c);
			} else {
				rebase_parser(p_msg);
			}

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

					if (!pdu_ex_len) {
						rebase_parser(p_msg);
					} else {
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
				if (p_msg->cal_crc == COMBINE_U8_TO_U16(p_msg->pdu.data[p_msg->pdu_in - 1], p_msg->pdu.data[p_msg->pdu_in - 2])) {
					flush_parser(p_msg);
					return 0;
				} else {
					rebase_parser(p_msg);
				}
			}

			break;

		default:
			break;
		}
	}

	return -1;
}

static int _rtu_handle(uint8_t addr, uint8_t func, uint16_t reg, uint16_t reg_num)
{
	modbus_slave_handler_t *p_handle;
	int res = MODBUS_SLAVE_HANDLE_NOT_REPLY;

	for (uint16_t i = 0; i < m_slave_table.num; i++) {
		p_handle = &m_slave_table.table[i];

		if (p_handle && MODBUS_CHECK_REG_RANGE(reg, reg_num, p_handle->start, p_handle->end)) {
			if (p_handle->handle) {
				res = p_handle->handle(addr, func, reg, reg_num, m_slave.data_in_out);
			}

			break;
		}
	}

	return res;
}

static uint16_t _packet_ack_read_frame(uint16_t reg, uint16_t reg_num, uint8_t *p_data_out)
{
	uint16_t pkt_len = 0;
	uint16_t crc = 0xffff;
	uint16_t *p;
	p_data_out[pkt_len++] = m_slave.slave_addr;
	modbus_slave_handle_e ret_flag = MODBUS_SLAVE_HANDLE_ERR;
	ret_flag = _rtu_handle(m_slave.slave_addr, MODBUS_FUN_RD_REG_MUL, reg, reg_num);

	if (ret_flag == MODBUS_SLAVE_HANDLE_ERR) {
		p_data_out[pkt_len++] = MODBUS_FUN_RD_REG_MUL | 0x80;
		p_data_out[pkt_len++] = MODBUS_SLAVE_HANDLE_ERR;

	} else if (ret_flag == MODBUS_SLAVE_HANDLE_SUCCESS) {
		p = m_slave.data_in_out;

		p_data_out[pkt_len++] = MODBUS_FUN_RD_REG_MUL;
		p_data_out[pkt_len++] = (reg_num << 1);

		for (uint16_t i = 0; i < reg_num; i++, p++) {
			p_data_out[pkt_len++] = GET_U8_HIGH_FROM_U16(*p);
			p_data_out[pkt_len++] = GET_U8_LOW_FROM_U16(*p);
		}
	} else
		return 0;

	crc = crc16_update_bytes(crc, p_data_out, pkt_len);

	p_data_out[pkt_len++] = GET_U8_LOW_FROM_U16(crc);
	p_data_out[pkt_len++] = GET_U8_HIGH_FROM_U16(crc);

	return pkt_len;
}

static uint16_t _packet_ack_write_frame(uint8_t func, uint16_t reg, uint16_t reg_num, uint8_t *p_data_out)
{
	uint16_t pkt_len = 0;
	uint16_t crc = 0xffff;
	modbus_slave_handle_e ret_flag = MODBUS_SLAVE_HANDLE_ERR;

	if (func == MODBUS_FUN_WR_REG_MUL) {
		p_data_out[pkt_len++] = m_slave.slave_addr;
		ret_flag = _rtu_handle(m_slave.slave_addr, func, reg, reg_num);

		if (ret_flag == MODBUS_SLAVE_HANDLE_ERR) {
			p_data_out[pkt_len++] = MODBUS_FUN_WR_REG_MUL | 0x80;
			p_data_out[pkt_len++] = MODBUS_SLAVE_HANDLE_ERR;

		} else if (ret_flag == MODBUS_SLAVE_HANDLE_SUCCESS) {
			p_data_out[pkt_len++] = MODBUS_FUN_WR_REG_MUL;
			p_data_out[pkt_len++] = GET_U8_HIGH_FROM_U16(reg);
			p_data_out[pkt_len++] = GET_U8_LOW_FROM_U16(reg);
			p_data_out[pkt_len++] = GET_U8_HIGH_FROM_U16(reg_num);
			p_data_out[pkt_len++] = GET_U8_LOW_FROM_U16(reg_num);
		} else
			return 0;

		crc = crc16_update_bytes(crc, p_data_out, pkt_len);
		p_data_out[pkt_len++] = GET_U8_LOW_FROM_U16(crc);
		p_data_out[pkt_len++] = GET_U8_HIGH_FROM_U16(crc);
	}

	return pkt_len;
}

static uint16_t _dispatch_rtu_msg(const msg_state_t *p_msg, uint8_t *ack_frame)
{
	uint16_t ptk_len = 0;
	uint16_t reg, reg_num;
	uint8_t data_len;
	const uint8_t *p;

	switch (p_msg->func) {
	case MODBUS_FUN_RD_REG_MUL:
		reg = COMBINE_U8_TO_U16(p_msg->pdu.write.reg_h, p_msg->pdu.write.reg_l);
		reg_num = COMBINE_U8_TO_U16(p_msg->pdu.write.num_h, p_msg->pdu.write.num_l);
		ptk_len = _packet_ack_read_frame(reg, reg_num, ack_frame);
		break;

	case MODBUS_FUN_WR_REG_MUL:
		reg = COMBINE_U8_TO_U16(p_msg->pdu.write.reg_h, p_msg->pdu.write.reg_l);
		reg_num = COMBINE_U8_TO_U16(p_msg->pdu.write.num_h, p_msg->pdu.write.num_l);
		data_len = p_msg->pdu.write.len;
		p = &p_msg->pdu.data[sizeof(PDU_WRITE_T)];

		for (uint8_t i = 0, j = 0; i < data_len; i += 2, j++) {
			m_slave.data_in_out[j] = (*p++ << 8);
			m_slave.data_in_out[j] |= *p++;
		}

		ptk_len = _packet_ack_write_frame(p_msg->func, reg, reg_num, ack_frame);
		break;

	default:
		break;
	}

	return ptk_len;
}