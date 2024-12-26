/**
 * @file modbus_slave.h
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

#ifndef _MODBUS_SLAVE_H
#define _MODBUS_SLAVE_H

#include "modbus.h"

// 响应码
#define MODBUS_RESP_SUCCESS 0x00	// 响应成功
#define MODBUS_RESP_NOT_REPLY 0x01	// 不回复
#define MODBUS_RESP_ERR_FUNC 0x02	// 功能码错误
#define MODBUS_RESP_ERR_REG 0x03	// 寄存器地址错误
#define MODBUS_RESP_ERR_REGNUM 0x04 // 寄存器数量错误
#define MODBUS_RESP_ERR_OTHER 0x05	// 其他错误

/**
 * @brief 从机接收帧处理
 *
 * @param func 功能码
 * @param reg 寄存器地址
 * @param reg_num 寄存器数量
 * @param p_in_out 输入输出缓冲
 *
 * @return uint8_t 参考响应码
 */
typedef uint8_t (*mb_slv_frame_resp)(
	uint8_t func, uint16_t reg, uint16_t reg_num, uint16_t *p_in_out);

// 寄存器区间任务处理
struct mb_slv_work {
	uint16_t start; // 起始寄存器
	uint16_t end; // 结束寄存器, 处理时不包括end, 应该设为实际的结束寄存器+1
	mb_slv_frame_resp resp; // 响应处理函数
};

// 从机句柄
typedef struct mb_slv *mb_slv_handle;

/**
 * @brief 从机初始化并申请句柄
 *
 * @param opts 				读写等回调函数指针
 * @param slv_addr 			从机地址
 * @param table 			任务处理表
 * @param table_num 		任务处理表数量
 * @return mb_slv_handle 	成功返回句柄，失败返回NULL
 */
mb_slv_handle mb_slv_init(
	struct serial_opts *opts, uint8_t slv_addr, struct mb_slv_work *work_table, uint16_t table_num);

/**
 * @brief 释放从机句柄
 *
 * @param handle
 */
void mb_slv_destroy(mb_slv_handle handle);

/**
 * @brief modbus任务轮询
 *
 * @param handle 从机句柄
 */
void mb_slv_poll(mb_slv_handle handle);

#endif