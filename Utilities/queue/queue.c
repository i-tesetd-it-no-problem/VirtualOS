/**
 * @file queue.c
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

#include "queue.h"

bool queue_init(queue_info_t *q, q_size unit_bytes, uint8_t *buf, q_size count)
{
	if (!q || !buf || count == 0 || !POWER_OF_TWO(count))
		return false;

	q->unit_bytes = unit_bytes;
	q->buf = buf;
	q->buf_size = count;
	q->rd = q->wr = 0;
	return true;
}

void queue_reset(queue_info_t *q)
{
	q->rd = q->wr = 0;
}

void queue_reset_safe_from_add(queue_info_t *q)
{
	q->wr = q->rd;
}

void queue_reset_safe_from_get(queue_info_t *q)
{
	q->rd = q->wr;
}

q_size queue_used(const queue_info_t *q)
{
	return q->wr - q->rd;
}

q_size queue_remain_space(const queue_info_t *q)
{
	return q->buf_size - queue_used(q);
}

bool is_queue_empty(const queue_info_t *q)
{
	return (queue_used(q) == 0);
}

bool is_queue_full(const queue_info_t *q)
{
	return (queue_remain_space(q) == 0);
}

q_size queue_add(queue_info_t *q, const uint8_t *data, q_size units)
{
	if (is_queue_full(q))
		return 0;

	q_size spaces = queue_remain_space(q);
	units = (units > spaces) ? spaces : units;

	q_size index = q->wr & (q->buf_size - 1);
	q_size tail_cnt = Q_MIN(units, q->buf_size - index);

	memcpy(q->buf + (index * q->unit_bytes), data, tail_cnt * q->unit_bytes);
	memcpy(q->buf, data + (tail_cnt * q->unit_bytes), (units - tail_cnt) * q->unit_bytes);

	q->wr += units;
	return units;
}

q_size queue_get(queue_info_t *q, uint8_t *data, q_size units)
{
	if (is_queue_empty(q))
		return 0;

	q_size used = queue_used(q);
	units = (units > used) ? used : units;

	q_size index = q->rd & (q->buf_size - 1);
	q_size tail_cnt = Q_MIN(units, q->buf_size - index);

	memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
	memcpy(data + tail_cnt * q->unit_bytes, q->buf, (units - tail_cnt) * q->unit_bytes);

	q->rd += units;
	return units;
}

q_size queue_peek(const queue_info_t *q, uint8_t *data, q_size units)
{
	if (is_queue_empty(q))
		return 0;

	q_size used = queue_used(q);
	units = (units > used) ? used : units;

	q_size index = q->rd & (q->buf_size - 1);
	q_size tail_cnt = Q_MIN(units, q->buf_size - index);

	memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
	memcpy(data + tail_cnt * q->unit_bytes, q->buf, (units - tail_cnt) * q->unit_bytes);

	return units;
}

void queue_advance_rd(queue_info_t *q, q_size units)
{
	q->rd += Q_MIN(units, queue_used(q));
}

void queue_advance_wr(queue_info_t *q, q_size units)
{
	q->wr += Q_MIN(units, queue_remain_space(q));
}
