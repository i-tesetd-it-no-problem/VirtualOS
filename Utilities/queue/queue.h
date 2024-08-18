/**
 * @file queue.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 循环队列组件
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

#ifndef _VIRTUAL_OS_QUEUE_H
#define _VIRTUAL_OS_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* 定义辅助宏 */
#define POWER_OF_TWO(x) ((x) && !((x) & ((x)-1))) /* 判断 x 是否为 2 的幂 */
#define Q_MIN(a, b) (((a) <= (b)) ? (a) : (b)) /* 取 a 和 b 的最小值 */

typedef size_t q_size;

/* 环形队列信息结构体 */
typedef struct {
	uint8_t *buf; /* 缓冲区 */
	q_size unit_bytes; /* 单元大小 */
	q_size buf_size; /* 缓冲区大小 */
	volatile q_size rd; /* 读索引 */
	volatile q_size wr; /* 写索引 */
} queue_info_t;

/**
 * @brief 初始化队列
 * 
 * @param q 队列实例
 * @param unit_bytes 单元字节数
 * @param buf 缓冲区
 * @param count 缓冲区大小
 * @return
 */
bool queue_init(queue_info_t *q, q_size unit_bytes, uint8_t *buf, q_size count);

/**
 * @brief 重置队列
 * 
 * @param q 队列实例
 */
void queue_reset(queue_info_t *q);

/**
 * @brief 从添加操作中安全地重置队列
 * 
 * @param q 队列实例
 */
void queue_reset_safe_from_add(queue_info_t *q);

/**
 * @brief 从获取操作中安全地重置队列
 * 
 * @param q 队列实例
 */
void queue_reset_safe_from_get(queue_info_t *q);

/**
 * @brief 获取队列中已使用的空间
 * 
 * @param q 队列实例
 * @return q_size 已使用的空间
 */
q_size queue_used(const queue_info_t *q);

/**
 * @brief 获取队列中剩余的空间
 * 
 * @param q 队列实例
 * @return q_size 剩余的空间
 */
q_size queue_remain_space(const queue_info_t *q);

/**
 * @brief 判断队列是否为空
 * 
 * @param q 队列实例
 * @return 
 */
bool is_queue_empty(const queue_info_t *q);

/**
 * @brief 判断队列是否已满
 * 
 * @param q 队列实例
 * @return 
 */
bool is_queue_full(const queue_info_t *q);

/**
 * @brief 向队列中添加数据
 * 
 * @param q 队列实例
 * @param data 数据
 * @param units 数据单元数
 * @return 
 */
q_size queue_add(queue_info_t *q, const uint8_t *data, q_size units);

/**
 * @brief 从队列中获取数据
 * 
 * @param q 队列实例
 * @param data 存储数据的缓冲区
 * @param units 数据单元数
 * @return 
 */
q_size queue_get(queue_info_t *q, uint8_t *data, q_size units);

/**
 * @brief 查看队列中的数据，但不移除
 * 
 * @param q 队列实例
 * @param data 存储数据的缓冲区
 * @param units 数据单元数
 * @return 
 */
q_size queue_peek(const queue_info_t *q, uint8_t *data, q_size units);

/**
 * @brief 增加读索引
 * 
 * @param q 队列实例
 * @param units 
 */
void queue_advance_rd(queue_info_t *q, q_size units);

/**
 * @brief 增加写索引
 * 
 * @param q 队列实例
 * @param units 
 */
void queue_advance_wr(queue_info_t *q, q_size units);

#endif /* _VIRTUAL_OS_QUEUE_H */
