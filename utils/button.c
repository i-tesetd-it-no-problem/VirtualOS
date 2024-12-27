/**
 * @file button.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 按键组件
 * @version 0.1
 * @date 2024-12-27
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

#include "utils/button.h"

#include <stdlib.h>
#include <stddef.h>

struct button_jitter {
	uint8_t previous;
	uint8_t asserted;
};

enum btn_io_event {
	BTN_IO_EVENT_UP,   // 按键弹起
	BTN_IO_EVENT_DOWN, // 按键按下
};

typedef enum usr_btn_ev (*on_state_handler)(btn_handle handle, enum btn_io_event io_ev);

struct button_state {
	on_state_handler state;	  // 当前状态
	struct button_jitter jit; // 抖动处理
	uint32_t click_cnt;		  // 点击次数计数
	uint32_t counter;		  // 状态计数器
};

struct button {
	struct btn_cfg cfg;		   // 按键配置
	struct button_state state; // 按键状态
	btn_usr_cb f_ev_cb;		   // 按键事件回调
};

static enum usr_btn_ev on_idle_handler(btn_handle handle, enum btn_io_event io_ev);		   // 空闲状态
static enum usr_btn_ev on_up_handler(btn_handle handle, enum btn_io_event io_ev);		   // 弹起状态
static enum usr_btn_ev on_down_handler(btn_handle handle, enum btn_io_event io_ev);		   // 按下状态
static enum usr_btn_ev on_up_suspense_handler(btn_handle handle, enum btn_io_event io_ev); // 悬挂等待状态
static enum usr_btn_ev on_down_short_handler(btn_handle handle, enum btn_io_event io_ev);  // 短按状态
static enum usr_btn_ev on_down_long_handler(btn_handle handle, enum btn_io_event io_ev);   // 长按状态

/**
 * @brief 分发点击类型事件，根据点击次数返回相应的按键事件
 * 
 * @param click_cnt 点击次数
 * @return 对应的按键事件
 */
static inline enum usr_btn_ev dispatch_click_type(uint32_t click_cnt)
{
	static const enum usr_btn_ev click_type[] = {
		USR_BTN_EV_NONE,		 // 无事件
		USR_BTN_EV_SINGLE_CLICK, // 单击事件
		USR_BTN_EV_DOUBLE_CLICK, // 双击事件
	};

	if (click_cnt < sizeof(click_type) / sizeof(click_type[0]))
		return click_type[click_cnt];
	else
		return USR_BTN_EV_MORE_CLICK; // 多次点击事件
}

// 空闲状态
static enum usr_btn_ev on_idle_handler(btn_handle handle, enum btn_io_event io_ev)
{
	if (io_ev == BTN_IO_EVENT_DOWN) {
		handle->state.counter = 0;
		handle->state.click_cnt = 1;
		handle->state.state = on_down_handler; // 被按下
	}

	return USR_BTN_EV_NONE;
}

// 按下状态
static enum usr_btn_ev on_down_handler(btn_handle handle, enum btn_io_event io_ev)
{
	enum usr_btn_ev ev = USR_BTN_EV_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = USR_BTN_EV_POPUP;
		handle->state.counter = 0;
		handle->state.state = on_up_suspense_handler; // 等待判断后续是否还有点击事件
	} else {
		if (++handle->state.counter >= handle->cfg.long_min_cnt) {
			// 按下超过最大长按时间
			ev = USR_BTN_EV_LONG_CLICK;
			handle->state.counter = 0;
			handle->state.state = on_down_long_handler;
		}
	}

	return ev;
}

// 悬挂等待状态
static enum usr_btn_ev on_up_suspense_handler(btn_handle handle, enum btn_io_event io_ev)
{
	enum usr_btn_ev ev = USR_BTN_EV_NONE;

	// 弹起
	if (io_ev == BTN_IO_EVENT_UP) {
		if (++handle->state.counter >= handle->cfg.up_max_cnt) {
			// 弹起一定时间后不再按下
			handle->state.counter = 0;
			ev = dispatch_click_type(handle->state.click_cnt);
			handle->state.state = on_up_handler;
		}
	} else {
		handle->state.counter = 0;
		++handle->state.click_cnt;
		handle->state.state = on_down_short_handler; // 又继续按下
	}

	return ev;
}

// 弹起状态
static enum usr_btn_ev on_up_handler(btn_handle handle, enum btn_io_event io_ev)
{
	if (io_ev == BTN_IO_EVENT_DOWN) {
		handle->state.counter = 0;
		handle->state.click_cnt = 1;
		handle->state.state = on_down_handler;
	}

	return USR_BTN_EV_NONE;
}

// 短按状态
static enum usr_btn_ev on_down_short_handler(btn_handle handle, enum btn_io_event io_ev)
{
	enum usr_btn_ev ev = USR_BTN_EV_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = USR_BTN_EV_POPUP;
		handle->state.counter = 0;
		handle->state.state = on_up_suspense_handler; // 又弹起了，等待判断后续是否还有点击事件
	}

	return ev;
}

// 长按状态
static enum usr_btn_ev on_down_long_handler(btn_handle handle, enum btn_io_event io_ev)
{
	enum usr_btn_ev ev = USR_BTN_EV_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = USR_BTN_EV_POPUP;
		handle->state.state = on_up_handler;
	}
	return ev;
}

// 消抖
static inline uint8_t button_debounce(btn_handle handle)
{
	uint8_t cur_lv = handle->cfg.f_io_read();

	handle->state.jit.asserted |= (handle->state.jit.previous & cur_lv);
	handle->state.jit.asserted &= (handle->state.jit.previous | cur_lv);

	handle->state.jit.previous = cur_lv;

	return handle->state.jit.asserted;
}

// 按键初始化
btn_handle button_ctor(const struct btn_cfg *p_cfg, btn_usr_cb cb)
{
	if (!p_cfg || !p_cfg->f_io_read)
		return NULL;

	btn_handle handle = malloc(sizeof(struct button));
	if (!handle) {
		return NULL;
	}

	handle->cfg = *p_cfg;
	handle->f_ev_cb = cb;

	handle->state.jit.previous = (p_cfg->active_lv == BUTTON_LEVEL_HIGH) ? 0 : 1;
	handle->state.jit.asserted = handle->state.jit.previous;

	// 初始化为 idle 状态
	handle->state.state = on_idle_handler;

	handle->state.click_cnt = 0;
	handle->state.counter = 0;

	return handle;
}

// 释放句柄
void button_destroy(btn_handle handle)
{
	if (handle) {
		free(handle); // 释放动态分配的按键结构体内存
	}
}

// 按键扫描
void button_scan(btn_handle handle)
{
	if (!handle || !handle->cfg.f_io_read || !handle->state.state)
		return;

	struct btn_ev_data ev;
	ev.ev_type = USR_BTN_EV_NONE;
	ev.clicks = 0;

	uint8_t cur_level = button_debounce(handle); // 当前有效电平

	if (cur_level == (handle->cfg.active_lv == BUTTON_LEVEL_HIGH ? 1 : 0))
		ev.ev_type = handle->state.state(handle, BTN_IO_EVENT_DOWN);
	else
		ev.ev_type = handle->state.state(handle, BTN_IO_EVENT_UP);

	if (ev.ev_type != USR_BTN_EV_NONE && ev.ev_type != USR_BTN_EV_POPUP && handle->f_ev_cb) {
		ev.clicks = handle->state.click_cnt;
		handle->f_ev_cb(&ev);
	}

	return;
}
