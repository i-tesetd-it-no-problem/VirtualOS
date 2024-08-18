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

typedef enum {
	BTN_EVENT_NONE,
	BTN_EVENT_POPUP,
	BTN_EVENT_SINGLE_CLICK,
	BTN_EVENT_DOUBLE_CLICK,
	BTN_EVENT_MORE_CLICK,
	BTN_EVENT_LONG_CLICK,
} button_event_e;

typedef struct {
	button_event_e ev_type; /* 按键的事件 */
	uint32_t clicks; /* 按下的次数 */
} button_ev_t;

typedef enum {
	BUTTON_LEVEL_LOW,
	BUTTON_LEVEL_HIGH,
} button_level_e;

typedef struct {
	uint8_t previous;
	uint8_t asserted;
} button_jitter_t;

typedef struct {
	void *state;
	button_jitter_t jit;
	uint32_t click_cnt;
	uint32_t counter;
} button_state_t;

typedef uint8_t (*btn_io_read)(void);

typedef void (*btn_event_callback)(const button_ev_t *p_ev);

typedef struct {
	btn_io_read f_io_read; /* 读按键 IO 的函数指针 */
	uint32_t long_min_cnt; /* 按下按键切换到长按事件最少维持的周期 */
	uint32_t up_max_cnt; /* 按键弹起到再次按下事件最多维持的周期 */
	button_level_e active_lv; /* 按下的有效电平 */
} button_cfg_t;

typedef struct {
	button_cfg_t cfg;
	button_state_t state;
	btn_event_callback f_ev_cb;
} button_t;

/**
 * 按键实例的构造函数
 * @param cfg 按键的一些关键配置参数
 * @param cb 按键事件的回调
 * @return 返回按键的实例
 */
button_t button_ctor(button_cfg_t *cfg, btn_event_callback cb);

/**
 * 对按键进行扫描，需要由调用方周期性执行，这个周期一般设定为按键的防抖间隔
 * @param p_btn 按键的实例
 * @return 返回当前的事件
 */
button_event_e button_scan(button_t *p_btn);

#endif /*_VIRTUAL_OS_BUTTON_H*/