/**
 * @file stimer.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 前后台调度组件
 * @version 1.0
 * @date 2024-08-12
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

#include <stdlib.h>
#include "utils/list.h"
#include "utils/stimer.h"

#define HIT_LIST_MASK (STIMER_TASK_HIT_LIST_MAX - 1)
#define HIT_LIST_IDX(t) ((m_timer.pre_tick + (t)) & HIT_LIST_MASK)

#define STIMER_TASK_HIT_LIST_MAX (32)
#define MAX_DEFER_TASK (16)

#define Period_to_Tick(p) (((p) >= STIMER_PERIOD_PER_TICK_MS) ? ((p) / STIMER_PERIOD_PER_TICK_MS) : 1U)

struct stimer_task {
	stimer_f task_f;
	uint32_t period;
	uint32_t arrive;
	list_item item;
	uint8_t reserved;
};

struct timer {
	volatile uint32_t pre_tick;
	volatile uint32_t cur_tick;
	volatile int run_flag;
	stimer_base_start f_start;
	list_item long_tick_list;
	list_item hit_task_list[STIMER_TASK_HIT_LIST_MAX];
	list_item defer_task_list;
};

static struct stimer_task defer_pool[MAX_DEFER_TASK];
static struct timer m_timer = { 0 };

static inline int is_timer_run(void)
{
	return m_timer.run_flag == 1;
}

static inline void _timer_update(void)
{
	++m_timer.cur_tick;
}

static struct stimer_task *defer_task_allocate(void)
{
	for (int i = 0; i < MAX_DEFER_TASK; i++) {
		if (defer_pool[i].reserved == 1)
			return &defer_pool[i];
	}
	return NULL;
}

static void defer_task_free(struct stimer_task *task)
{
	if (task) {
		task->task_f = NULL;
		task->reserved = 1;
		task->arrive = 0;
		task->period = 0;
	}
}

static inline void _add_timer(uint32_t period, list_item *item)
{
	list_delete_item(item);

	if (period > STIMER_TASK_HIT_LIST_MAX)
		list_add_tail(&(m_timer.long_tick_list), item);
	else
		list_add_tail(&(m_timer.hit_task_list[HIT_LIST_IDX(period)]), item);
}

static bool stimer_task_add(struct stimer_task *p_task)
{
	if (!p_task)
		return false;

	p_task->arrive = 0;
	_add_timer(p_task->period, &(p_task->item));
	return true;
}

static uint32_t inline stimer_get_tick(void)
{
	return m_timer.cur_tick;
}

static void stimer_task_dispatch(void)
{
	uint32_t idx, remain;
	struct list_item *cur_item, *next_item;
	struct stimer_task *task;

	if (!is_timer_run() || (m_timer.pre_tick == m_timer.cur_tick))
		return;

	++m_timer.pre_tick;
	idx = HIT_LIST_IDX(0);

	if (idx == 0) {
		list_for_each_safe(cur_item, next_item, &(m_timer.long_tick_list))
		{
			task = container_of(cur_item, struct stimer_task, item);
			task->arrive += STIMER_TASK_HIT_LIST_MAX;
			remain = task->period - task->arrive;

			if (remain == 0) {
				if (task->task_f)
					task->task_f();
				task->arrive = 0;
			} else if (remain < STIMER_TASK_HIT_LIST_MAX) {
				_add_timer(remain, cur_item);
			}
		}
	}

	list_for_each_safe(cur_item, next_item, &(m_timer.hit_task_list[idx]))
	{
		task = container_of(cur_item, struct stimer_task, item);
		if (task->task_f)
			task->task_f();
		task->arrive = -idx;
		_add_timer(task->period, &(task->item));
	}

	list_for_each_safe(cur_item, next_item, &(m_timer.defer_task_list))
	{
		task = container_of(cur_item, struct stimer_task, item);
		if (++task->arrive >= task->period) {
			if (task->task_f)
				task->task_f();
			list_delete_item(cur_item);
			defer_task_free(task);
		}
	}
}

/*************************************API*************************************/

bool stimer_init(struct timer_port *port)
{
	if (!port || !port->f_init || !port->f_start)
		return false;

	list_init(&(m_timer.long_tick_list));
	list_init(&(m_timer.defer_task_list));

	for (int i = 0; i < MAX_DEFER_TASK; i++)
		defer_pool[i] = (struct stimer_task){ .reserved = 1 };

	for (uint8_t i = 0; i < STIMER_TASK_HIT_LIST_MAX; i++)
		list_init(&(m_timer.hit_task_list[i]));

	port->f_init(STIMER_PERIOD_PER_TICK_MS, _timer_update);
	m_timer.f_start = port->f_start;
	return true;
}

bool stimer_task_create(stimer_f init_f, stimer_f task_f, uint32_t period_ms)
{
	if (init_f)
		init_f();

	if (!task_f || !period_ms)
		return false;

	struct stimer_task *task = (struct stimer_task *)calloc(1, sizeof(struct stimer_task));
	if (task) {
		task->period = Period_to_Tick(period_ms);
		task->reserved = 1;
		task->task_f = task_f;
		list_init(&(task->item));

		return stimer_task_add(task);
	}
	return false;
}

bool defer_task_create(stimer_f task_f, uint32_t ms)
{
	if (!is_timer_run())
		return false;

	struct stimer_task *p_task = defer_task_allocate();
	if (!p_task)
		return false;

	p_task->task_f = task_f;
	p_task->period = Period_to_Tick(ms);
	p_task->arrive = 0;
	p_task->reserved = 0;
	list_add_tail(&(m_timer.defer_task_list), &(p_task->item));
	return true;
}

void stimer_start(void)
{
	if (!m_timer.f_start)
		return;

	m_timer.f_start();
	m_timer.run_flag = 1;

	uint32_t pre_tick = stimer_get_tick();
	uint32_t cur_tick;

	while (1) {
		cur_tick = stimer_get_tick();
		if (cur_tick != pre_tick) {
			pre_tick = cur_tick;
			stimer_task_dispatch();
		}
	}
}
