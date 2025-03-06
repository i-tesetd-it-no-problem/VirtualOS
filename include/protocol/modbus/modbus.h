/**
 * @file modbus.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief modbus通用协议
 * @version 0.1
 * @date 2024-12-17
 *
 * @copyright Copyright (c) 2024-2025
 * @see repository: https://github.com/i-tesetd-it-no-problem/VirtualOS.git
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

#ifndef __VIRTUAL_OS_MODBUS_H__
#define __VIRTUAL_OS_MODBUS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 当前支持的功能码
#define MODBUS_FUN_RD_REG_MUL (0x03) // 读功能码
#define MODBUS_FUN_WR_REG_MUL (0x10) // 写功能码

// 错误码
#define MODBUS_RESP_ERR_NONE (0x00)		// 无错误
#define MODBUS_RESP_ERR_FUNC (0x01)		// 功能码错误 例如只读寄存器不能写
#define MODBUS_RESP_ERR_REG_ADDR (0x02) // 寄存器地址错误
#define MODBUS_RESP_ERR_DATA (0x03)		// 数据错误
#define MODBUS_RESP_ERR_DEV (0x04)		// 设备错误
#define MODBUS_RESP_ERR_PENDING (0x05)	// 正在处理中,不能及时回复,需要主机后续轮询读取
#define MODBUS_RESP_ERR_BUSY (0x06)		// 设备繁忙, 无法处理

// 一帧最大字节数 256
#define MODBUS_FRAME_BYTES_MAX (256)

// 校验功能码
#define MODBUS_FUNC_CHECK_VALID(f) (((f) == MODBUS_FUN_RD_REG_MUL) || ((f) == MODBUS_FUN_WR_REG_MUL))

#define MAX_READ_REG_NUM (125)	// 最大读寄存器数量
#define MAX_WRITE_REG_NUM (123) // 最大写寄存器数量

// 检查寄存器数量
#define CHECK_REG_NUM_VALID(reg_num, func)                                                                             \
	(((func) == MODBUS_FUN_RD_REG_MUL)                                                                                 \
			? ((reg_num) <= MAX_READ_REG_NUM)                                                                          \
			: (((func) == MODBUS_FUN_WR_REG_MUL) ? ((reg_num) <= MAX_WRITE_REG_NUM) : false))

// 校验寄存器范围
#define MODBUS_CHECK_REG_RANGE(reg, num, from, to, func)                                                               \
	(((reg) < (to)) && ((reg) >= (from)) && CHECK_REG_NUM_VALID((num), (func)) && (((reg) + (num)) <= (to)))

#define MODBUS_ADDR_BYTES_NUM (1)	 // 地址字节数
#define MODBUS_FUNC_BYTES_NUM (1)	 // 功能码字节数
#define MODBUS_REG_BYTES_NUM (2)	 // 寄存器地址字节数
#define MODBUS_REG_LEN_BYTES_NUM (2) // 寄存器长度字节数
#define MODBUS_CRC_BYTES_NUM (2)	 // CRC字节数

#define COMBINE_U8_TO_U16(h, l) ((uint16_t)(h << 8) | (uint16_t)(l))
#define COMBINE_U16_TO_U32(h, l) ((uint32_t)(h << 16) | (uint32_t)(l))
#define GET_U8_HIGH_FROM_U16(u) (((u) >> 8) & 0xff)
#define GET_U8_LOW_FROM_U16(u) ((u) & 0xff)

// 串口方向
enum modbus_serial_dir {
	modbus_serial_dir_all_unuse,
	modbus_serial_dir_rx_only,
	modbus_serial_dir_tx_only,
};

/**
 * @brief 串口初始化函数指针
 *
 * @return true 初始化成功
 * @return false 初始化失败
 */
typedef bool (*modbus_serial_init)(void);

/**
 * @brief 串口写函数指针
 *
 * @param p_data 待写入数据指针
 * @param len 待写入数据长度
 * @return size_t 实际写入数据长度 失败返回0
 */
typedef size_t (*modbus_serial_write)(uint8_t *p_data, size_t len);

/**
 * @brief 串口读函数指针
 *
 * @param p_data 读入数据指针
 * @param len 待读数据长度
 * @return size_t 实际读入数据长度 失败返回0
 */
typedef size_t (*modbus_serial_read)(uint8_t *p_data, size_t len);

/**
 * @brief 检查是否发送完成
 * 
 * 如果是轮训发送可直接设置为NULL
 * DAM发送则需要查询相关标志位
 * 
 * @return ture:发送完/可以接收 false:发送中
 */
typedef bool (*modbus_serial_check_over)(void);

// 串口回调
struct serial_opts {
	modbus_serial_init f_init;	 // 串口初始化函数指针
	modbus_serial_write f_write; // 串口写函数指针
	modbus_serial_read f_read;	 // 串口读函数指针
};

#endif /* __VIRTUAL_OS_MODBUS_H__ */