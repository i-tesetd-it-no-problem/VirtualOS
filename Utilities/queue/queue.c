/**
 * @file queue.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 循环队列组件
 * @version 0.1
 * @date 2024-12-19
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
#include "queue.h"

/* 获取较小的值 */
static size_t Q_MIN(size_t a, size_t b) {
    return (a <= b) ? a : b;
}

/* 初始化队列 */
bool queue_init(struct queue_info *q, size_t unit_bytes, uint8_t *buf,
                size_t count, lock_callback_t lock, unlock_callback_t unlock) {
    if (!q || !buf || count == 0)
        return false;

    q->unit_bytes = unit_bytes;
    q->buf = buf;
    q->buf_size = count;
    q->rd = q->wr = 0;
    q->lock = lock;
    q->unlock = unlock;

    return true;
}

/* 销毁队列 */
void queue_destroy(struct queue_info *q) {
    if (!q)
        return;
}

/* 锁定队列 */
static void lock_queue(struct queue_info *q) {
    if (q->lock)
        q->lock();
}

/* 解锁队列 */
static void unlock_queue(struct queue_info *q) {
    if (q->unlock)
        q->unlock();
}

/* 重置队列 */
void queue_reset(struct queue_info *q) {
    if (!q)
        return;

    lock_queue(q);
    q->rd = q->wr = 0;
    unlock_queue(q);
}

/* 向队列中添加数据 */
size_t queue_add(struct queue_info *q, const uint8_t *data, size_t units) {
    if (!q || !data || units == 0)
        return 0;

    lock_queue(q);

    size_t used = q->wr - q->rd;
    size_t spaces = q->buf_size - used;
    if (spaces == 0) {
        unlock_queue(q);
        return 0;
    }

    size_t to_add = Q_MIN(units, spaces);
    size_t index = q->wr % q->buf_size;
    size_t tail_cnt = Q_MIN(to_add, q->buf_size - index);

    memcpy(q->buf + (index * q->unit_bytes), data, tail_cnt * q->unit_bytes);
    if (to_add > tail_cnt)
        memcpy(q->buf, data + (tail_cnt * q->unit_bytes),
               (to_add - tail_cnt) * q->unit_bytes);

    q->wr += to_add;

    unlock_queue(q);
    return to_add;
}

/* 从队列中获取数据 */
size_t queue_get(struct queue_info *q, uint8_t *data, size_t units) {
    if (!q || !data || units == 0)
        return 0;

    lock_queue(q);

    size_t used = q->wr - q->rd;
    if (used == 0) {
        unlock_queue(q);
        return 0;
    }

    size_t to_get = Q_MIN(units, used);
    size_t index = q->rd % q->buf_size;
    size_t tail_cnt = Q_MIN(to_get, q->buf_size - index);

    memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
    if (to_get > tail_cnt)
        memcpy(data + (tail_cnt * q->unit_bytes), q->buf,
               (to_get - tail_cnt) * q->unit_bytes);

    q->rd += to_get;

    unlock_queue(q);
    return to_get;
}

/* 查看队列中的数据,但不移除 */
size_t queue_peek(const struct queue_info *q_const, uint8_t *data,
                  size_t units) {
    if (!q_const || !data || units == 0)
        return 0;

    struct queue_info *q = (struct queue_info *)q_const;

    lock_queue(q);

    size_t used = q->wr - q->rd;
    if (used == 0) {
        unlock_queue(q);
        return 0;
    }

    size_t to_peek = Q_MIN(units, used);
    size_t index = q->rd % q->buf_size;
    size_t tail_cnt = Q_MIN(to_peek, q->buf_size - index);

    memcpy(data, q->buf + (index * q->unit_bytes), tail_cnt * q->unit_bytes);
    if (to_peek > tail_cnt)
        memcpy(data + (tail_cnt * q->unit_bytes), q->buf,
               (to_peek - tail_cnt) * q->unit_bytes);

    unlock_queue(q);
    return to_peek;
}

/* 判断队列是否为空 */
bool is_queue_empty(const struct queue_info *q_const) {
    if (!q_const)
        return true;

    struct queue_info *q = (struct queue_info *)q_const;

    lock_queue(q);
    bool empty = (q->wr - q->rd) == 0;
    unlock_queue(q);

    return empty;
}

/* 判断队列是否已满 */
bool is_queue_full(const struct queue_info *q_const) {
    if (!q_const)
        return false;

    struct queue_info *q = (struct queue_info *)q_const;

    lock_queue(q);
    bool full = (q->wr - q->rd) >= q->buf_size;
    unlock_queue(q);

    return full;
}

/* 获取队列中已使用的单元数 */
size_t queue_used(const struct queue_info *q_const) {
    if (!q_const)
        return 0;

    struct queue_info *q = (struct queue_info *)q_const;

    lock_queue(q);
    size_t used = q->wr - q->rd;
    unlock_queue(q);

    return used;
}

/* 获取队列中剩余的可用单元数 */
size_t queue_remain_space(const struct queue_info *q_const) {
    if (!q_const)
        return 0;

    struct queue_info *q = (struct queue_info *)q_const;

    lock_queue(q);
    size_t used = q->wr - q->rd;
    size_t remain = (used >= q->buf_size) ? 0 : (q->buf_size - used);
    unlock_queue(q);

    return remain;
}

/* 进阶读取索引 */
void queue_advance_rd(struct queue_info *q, size_t units) {
    if (!q)
        return;

    lock_queue(q);

    size_t count = Q_MIN(units, queue_used(q));
    q->rd += count;

    unlock_queue(q);
}

/* 进阶写入索引 */
void queue_advance_wr(struct queue_info *q, size_t units) {
    if (!q)
        return;

    lock_queue(q);

    size_t count = Q_MIN(units, queue_remain_space(q));
    q->wr += count;

    unlock_queue(q);
}
