/**
 * @file button.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 按键组件
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

#include "utils/button.h"

#define NULL ((void *)0)

typedef enum {
	BTN_IO_EVENT_UP,
	BTN_IO_EVENT_DOWN,
} BTN_IO_EVENT_E;

typedef enum button_event (*on_state_handler)(struct button *p_btn, BTN_IO_EVENT_E io_ev);

static enum button_event on_up_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev);
static enum button_event on_down_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev);
static enum button_event on_up_suspense_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev);
static enum button_event on_down_short_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev);
static enum button_event on_down_long_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev);

static inline enum button_event dispatch_click_type(uint32_t click_cnt)
{
	static const enum button_event click_type[3] = {
		BTN_EVENT_NONE,
		BTN_EVENT_SINGLE_CLICK,
		BTN_EVENT_DOUBLE_CLICK,
	};

	return (click_cnt < 3) ? click_type[click_cnt] : BTN_EVENT_MORE_CLICK;
}

static enum button_event on_idle_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	if (io_ev == BTN_IO_EVENT_UP)
		p_btn->state.state = on_up_handler;
	else {
		p_btn->state.counter = 0;
		p_btn->state.click_cnt = 1;
		p_btn->state.state = on_down_handler;
	}

	return BTN_EVENT_NONE;
}

static enum button_event on_up_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	if (io_ev == BTN_IO_EVENT_DOWN) {
		p_btn->state.counter = 0;
		p_btn->state.click_cnt = 1;
		p_btn->state.state = on_down_handler;
	}

	return BTN_EVENT_NONE;
}

static enum button_event on_up_suspense_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	enum button_event ev = BTN_EVENT_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		if (++p_btn->state.counter >= p_btn->cfg.up_max_cnt) {
			p_btn->state.counter = 0;
			ev = dispatch_click_type(p_btn->state.click_cnt);
			p_btn->state.state = on_up_handler;
		}
	} else {
		p_btn->state.counter = 0;
		++p_btn->state.click_cnt;
		p_btn->state.state = on_down_short_handler;
	}

	return ev;
}

static enum button_event on_down_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	enum button_event ev = BTN_EVENT_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = BTN_EVENT_POPUP;
		p_btn->state.counter = 0;
		p_btn->state.state = on_up_suspense_handler;
	} else {
		if (++p_btn->state.counter >= p_btn->cfg.long_min_cnt) {
			ev = BTN_EVENT_LONG_CLICK;
			p_btn->state.counter = 0;
			p_btn->state.state = on_down_long_handler;
		}
	}

	return ev;
}

static enum button_event on_down_short_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	enum button_event ev = BTN_EVENT_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = BTN_EVENT_POPUP;
		p_btn->state.counter = 0;
		p_btn->state.state = on_up_suspense_handler;
	} else {
		if (++p_btn->state.counter >= p_btn->cfg.up_max_cnt) {
			p_btn->state.counter = 0;
			ev = dispatch_click_type(p_btn->state.click_cnt);
			p_btn->state.state = on_down_long_handler;
		}
	}

	return ev;
}

static enum button_event on_down_long_handler(struct button *p_btn, BTN_IO_EVENT_E io_ev)
{
	enum button_event ev = BTN_EVENT_NONE;

	if (io_ev == BTN_IO_EVENT_UP) {
		ev = BTN_EVENT_POPUP;
		p_btn->state.state = on_up_handler;
	}
	return ev;
}

static inline uint8_t button_debounce(struct button *p_btn)
{
	uint8_t cur_lv = p_btn->cfg.f_io_read();

	p_btn->state.jit.asserted |= (p_btn->state.jit.previous & cur_lv);
	p_btn->state.jit.asserted &= (p_btn->state.jit.previous | cur_lv);

	p_btn->state.jit.previous = cur_lv;

	return p_btn->state.jit.asserted;
}

enum button_event button_scan(struct button *p_btn)
{
	uint8_t cur_level;
	button_ev_t ev;

	if (!p_btn || !p_btn->cfg.f_io_read || !p_btn->state.state)
		return BTN_EVENT_NONE;

	cur_level = button_debounce(p_btn);

	if (cur_level == p_btn->cfg.active_lv)
		ev.ev_type = ((on_state_handler)p_btn->state.state)(p_btn, BTN_IO_EVENT_DOWN);
	else
		ev.ev_type = ((on_state_handler)p_btn->state.state)(p_btn, BTN_IO_EVENT_UP);

	if (ev.ev_type != BTN_EVENT_NONE && p_btn->f_ev_cb) {
		ev.clicks = p_btn->state.click_cnt;
		p_btn->f_ev_cb(&ev);
	}

	return ev.ev_type;
}

struct button button_ctor(struct button_cfg *p_cfg, btn_event_callback cb)
{
	struct button btn;

	if (p_cfg) {
		btn.cfg = *p_cfg;
		btn.f_ev_cb = cb;
		btn.state.jit.previous = (p_cfg->active_lv ^ BUTTON_LEVEL_HIGH);
		btn.state.jit.asserted = btn.state.jit.previous;
		btn.state.state = on_idle_handler;
	} else {
		btn.cfg.f_io_read = NULL;
		btn.state.state = NULL;
	}

	return btn;
}
