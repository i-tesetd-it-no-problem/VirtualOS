/**
 * @file button.h
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

#ifndef _VIRTUAL_OS_BUTTON_H
#define _VIRTUAL_OS_BUTTON_H

#include <stdint.h>

enum button_event {
	BTN_EVENT_NONE,
	BTN_EVENT_POPUP,
	BTN_EVENT_SINGLE_CLICK,
	BTN_EVENT_DOUBLE_CLICK,
	BTN_EVENT_MORE_CLICK,
	BTN_EVENT_LONG_CLICK,
};

typedef struct {
	enum button_event ev_type; /* 按键的事件 */
	uint32_t clicks;		   /* 按下的次数 */
} button_ev_t;

enum button_level {
	BUTTON_LEVEL_LOW,
	BUTTON_LEVEL_HIGH,
};

struct button_jitter {
	uint8_t previous;
	uint8_t asserted;
};

struct button_state {
	void *state;
	struct button_jitter jit;
	uint32_t click_cnt;
	uint32_t counter;
};

typedef uint8_t (*btn_io_read)(void);

typedef void (*btn_event_callback)(const button_ev_t *p_ev);

struct button_cfg {
	btn_io_read f_io_read;		 /* 读按键 IO 的函数指针 */
	uint32_t long_min_cnt;		 /* 按下按键切换到长按事件最少维持的周期 */
	uint32_t up_max_cnt;		 /* 按键弹起到再次按下事件最多维持的周期 */
	enum button_level active_lv; /* 按下的有效电平 */
};

struct button {
	struct button_cfg cfg;
	struct button_state state;
	btn_event_callback f_ev_cb;
};

/**
 * 按键实例的构造函数
 * @param cfg 按键的一些关键配置参数
 * @param cb 按键事件的回调
 * @return 返回按键的实例
 */
struct button button_ctor(struct button_cfg *cfg, btn_event_callback cb);

/**
 * 对按键进行扫描，需要由调用方周期性执行，这个周期一般设定为按键的防抖间隔
 * @param p_btn 按键的实例
 * @return 返回当前的事件
 */
enum button_event button_scan(struct button *p_btn);

#endif /*_VIRTUAL_OS_BUTTON_H*/