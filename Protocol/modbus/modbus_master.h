/**
 * @file modbus_master.h
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

#ifndef _MODBUS_MASTER_H_
#define _MODBUS_MASTER_H_

#include "modbus.h"
#include <stdint.h>

#define MASTER_REPEATS (3) // 超时未回复重发3次

/**
 * @brief 主机接收帧处理
 *
 * @param data 仅对 读 功能码有效  接收到的数据
 * @param len  仅对 读 功能码有效  数据长度
 * @param is_timeout ture:超时未回复 false:收到回复
 *
 * @return uint8_t 参考响应码
 */
typedef void (*mb_mst_pdu_resp)(uint8_t *data, size_t len, bool is_timeout);

// 请求报文 (必须定义为全局变量 运行时再去调整修改成员值)
struct mb_mst_request {
	uint16_t _hide_[2]; // 保留数据 用户无需修改

	/* 用户配置区域 */
	uint32_t timeout_ms;  // 此报文的超时时间
	uint8_t slave_addr;	  // 从机地址
	uint8_t func;		  // 功能玛 读:0x03 写:0x10
	uint16_t reg_addr;	  // 寄存器地址
	uint8_t reg_len;	  // 寄存器长度

	uint8_t *data;		  // 数据缓冲 仅对写功能玛有效 (必须定义为全局变量)
	uint8_t data_len;	  // 缓冲长度

	mb_mst_pdu_resp resp; // 回复处理
};

// 主机句柄
typedef struct mb_mst *mb_mst_handle;

/**
 * @brief 主机初始化并申请句柄
 *
 * @param opts 读写等回调函数指针
 * @param period_ms 轮训周期
 * @return mb_mst_handle 成功返回句柄，失败返回NULL
 */
mb_mst_handle mb_mst_init(struct serial_opts *opts, size_t period_ms);

/**
 * @brief 释放主机句柄
 *
 * @param handle 主机句柄
 */
void mb_mst_destroy(mb_mst_handle handle);

/**
 * @brief modbus任务轮询
 *
 * @param handle 主机句柄
 */
void mb_mst_poll(mb_mst_handle handle);

/**
 * @brief 主机发送报文
 * 
 * @param handle 主机句柄
 * @param request 请求结构体
 */
void mb_mst_pdu_request(mb_mst_handle handle, struct mb_mst_request *request);

#endif