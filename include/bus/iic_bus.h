/**
 * @file iic_bus.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief IIC总线定义
 * @version 0.1
 * @date 2024-12-31
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

#ifndef __VIRTUAL_OS_IIC_BUS_H__
#define __VIRTUAL_OS_IIC_BUS_H__

#include "stddef.h"
#include <stdint.h>

#define I2C_FLAG_WRITE (0x00) // 写标志
#define I2C_FLAG_READ (0x01)  // 读标志

/**
 * @brief IIC消息结构体
 * 
 */
struct i2c_msg {
	size_t len;	   // 数据长度
	uint8_t *buf;  // 数据指针
	uint8_t addr;  // 只支持7位地址
	uint8_t flags; // 读写标志
};

#endif /* __VIRTUAL_OS_IIC_BUS_H__ */