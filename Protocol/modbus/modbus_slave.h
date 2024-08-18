/**
 * @file modbus_slave.h
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

#ifndef _VIRTUAL_OS_MODBUS_SLAVE_H
#define _VIRTUAL_OS_MODBUS_SLAVE_H

#include "crc.h"
#include "modbus.h"
#include "queue.h"

typedef enum {
	MODBUS_SLAVE_HANDLE_ERR, //回复错误帧
	MODBUS_SLAVE_HANDLE_SUCCESS, //正常回复
	MODBUS_SLAVE_HANDLE_NOT_REPLY, //不回复
} modbus_slave_handle_e;

/**
 * @brief ModBus从机处理回调函数
 * 
 */
typedef modbus_slave_handle_e (*modbus_slave_handler)(uint16_t addr, uint8_t func, uint16_t reg, uint16_t reg_num, uint16_t *p_in_out);

typedef struct {
	uint16_t start; /* 起始地址 */
	uint16_t end; /* 结束地址 */
	modbus_slave_handler handle; /* 处理函数 */
} modbus_slave_handler_t;

/**
 * @brief 初始化ModBus从机
 * 
 * @param p_serial_opt 串口配置
 * @param f_validator 地址校验函数
 * @return int 初始化结果，0表示成功，其他表示失败
 */
int modbus_slave_init(modbus_serial_opt_t *p_serial_opt, rtu_address_validator f_validator);

/**
 * @brief 设置ModBus从机的处理表
 * 
 * @param p_handler_table 处理表
 * @param num 处理表长度
 */
void modbus_slave_set_table(modbus_slave_handler_t *p_handler_table, uint16_t num);

/**
 * @brief 启动调度
 * 
 */
void modbus_slave_poll(void);

#endif /*_VIRTUAL_OS_MODBUS_SLAVE_H*/