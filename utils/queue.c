/**
 * @file queue.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 环形队列组件
 * @version 0.1
 * @date 2024-12-25
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

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "utils/queue.h"

/* 获取较小的值 */
static inline size_t q_min(size_t a, size_t b)
{
	return (a <= b) ? a : b;
}

/* 初始化队列 */
bool queue_init(struct queue_info *q, size_t unit_bytes, void *buf, size_t units)
{
	if (!q || !buf || units == 0 || unit_bytes == 0)
		return false;

	q->unit_bytes = unit_bytes;
	q->buf = buf;
	q->buf_size = units;
	q->rd = 0;
	q->wr = 0;

	return true;
}

/* 销毁队列 */
void queue_destroy(struct queue_info *q)
{
	if (!q)
		return;

	q->rd = 0;
	q->wr = 0;
}

/* 重置队列 */
void queue_reset(struct queue_info *q)
{
	if (!q)
		return;

	q->rd = 0;
	q->wr = 0;
}

/* 已使用的空间，以单元为单位 */
size_t queue_used(struct queue_info *q)
{
	if (!q)
		return 0;

	return q->wr - q->rd;
}

/* 剩余空间，以单元为单位 */
size_t queue_remain_space(struct queue_info *q)
{
	if (!q)
		return 0;

	return (q->buf_size - queue_used(q));
}

/* 判断队列是否为空 */
bool is_queue_empty(struct queue_info *q)
{
	if (!q)
		return true;

	return queue_used(q) == 0;
}

/* 判断队列是否已满 */
bool is_queue_full(struct queue_info *q)
{
	if (!q)
		return false;

	return (queue_remain_space(q) == 0);
}

/* 向队列中添加数据，以单元为单位 */
size_t queue_add(struct queue_info *q, void *data, size_t units)
{
	if (!q || !data || units == 0 || is_queue_full(q))
		return 0;

	size_t remain = queue_remain_space(q);
	if (units > remain)
		units = remain; // 限制写入的单元数

	size_t index = q->wr % q->buf_size; // 实际写索引
	size_t tail_cnt = q_min(units, q->buf_size - index);

	memcpy(q->buf + (index * q->unit_bytes), data, tail_cnt * q->unit_bytes);
	if (units > tail_cnt) {
		memcpy(q->buf, data + (tail_cnt * q->unit_bytes), (units - tail_cnt) * q->unit_bytes);
	}

	q->wr += units;

	return units;
}

/* 从队列中获取数据，以单元为单位 */
size_t queue_get(struct queue_info *q, void *data, size_t units)
{
	if (!q || !data || units == 0 || is_queue_empty(q))
		return 0;

	size_t used = queue_used(q);
	if (units > used)
		units = used;

	size_t index = q->rd % q->buf_size; // 实例读索引
	size_t tail_cnt = q_min(units, q->buf_size - index);

	memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
	if (units > tail_cnt)
		memcpy(data + (tail_cnt * q->unit_bytes), q->buf, (units - tail_cnt) * q->unit_bytes);

	q->rd += units;

	return units;
}

/* 查看队列中的数据，但不移除，以单元为单位 */
size_t queue_peek(struct queue_info *q, void *data, size_t units)
{
	if (!q || !data || units == 0 || is_queue_empty(q))
		return 0;

	size_t used = queue_used(q);
	if (units > used)
		units = used;

	size_t index = q->rd % q->buf_size; // 实例读索引
	size_t tail_cnt = q_min(units, q->buf_size - index);

	memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
	if (units > tail_cnt)
		memcpy(data + (tail_cnt * q->unit_bytes), q->buf, (units - tail_cnt) * q->unit_bytes);

	return units;
}

/* 增加读索引，以单元为单位 */
void queue_advance_rd(struct queue_info *q, size_t units)
{
	if (!q)
		return;

	size_t count = q_min(units, queue_used(q));
	q->rd += count;
}

/* 增加写索引，以单元为单位 */
void queue_advance_wr(struct queue_info *q, size_t units)
{
	if (!q)
		return;

	size_t count = q_min(units, queue_remain_space(q));
	q->wr += count;
}
