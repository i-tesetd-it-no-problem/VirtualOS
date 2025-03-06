/**
 * @file can_bus.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief CAN总线定义
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

#ifndef __VIRTUAL_OS_CAN_BUS_H__
#define __VIRTUAL_OS_CAN_BUS_H__

#include <stdint.h>

#define CAN_MAX_DLEN 8

/*
 * 控制器局域网 (CAN) 标识符结构
 *
 * bit 0-28 : CAN 标识符（11/29 位）
 * bit 29   : 错误帧标志（0 = 数据帧，1 = 错误消息帧）
 * bit 30   : 远程请求帧标志（1 = RTR 帧）
 * bit 31   : 帧格式标志（0 = 标准帧（11 位），1 = 扩展帧（29 位））
 */
typedef uint32_t canid_t;

#define CAN_STANDARD_ID_MASK 0x000007FF // CAN 标准帧标识符掩码
#define CAN_EXTENDED_ID_MASK 0x1FFFFFFF // CAN 扩展帧标识符掩码
#define CAN_ERR_FLAG 0x20000000			// 错误帧标志
#define CAN_RTR_FLAG 0x40000000			// 远程请求帧标志
#define CAN_EXT_FLAG 0x80000000			// 扩展帧标志

// CAN帧
struct can_frame {
	uint8_t data[CAN_MAX_DLEN]; // 数据内容
	canid_t can_id;				// CAN ID + 帧类型
	uint8_t can_dlc;			// 数据长度
};

#endif /* __VIRTUAL_OS_CAN_BUS_H__ */