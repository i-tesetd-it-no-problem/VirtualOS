/**
 * @file button.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 按键组件
 * @version 0.1
 * @date 2024-12-27
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

#ifndef __VIRTUAL_OS_BUTTON_H__
#define __VIRTUAL_OS_BUTTON_H__

#include <stdint.h>

typedef struct button *btn_handle;

// 用户按键事件类型
enum usr_btn_ev {
	USR_BTN_EV_NONE,  // 无事件
	USR_BTN_EV_POPUP, // 弹起事件
	/* 以上事件用户无需判断 */

	USR_BTN_EV_SINGLE_CLICK = 2, // 单击事件
	USR_BTN_EV_DOUBLE_CLICK,	 // 双击事件
	USR_BTN_EV_MORE_CLICK,		 // 多击事件
	USR_BTN_EV_LONG_CLICK,		 // 长按事件
};

// 按键事件数据
struct btn_ev_data {
	enum usr_btn_ev ev_type; /* 按键的事件类型 */
	uint32_t clicks;		 /* 按下的次数 */
};

enum button_level {
	BUTTON_LEVEL_LOW,  // 低电平有效
	BUTTON_LEVEL_HIGH, // 高电平有效
};

typedef void (*btn_usr_cb)(const struct btn_ev_data *ev_data); // 按键事件回调

// 按键配置结构体
struct btn_cfg {
	uint8_t (*f_io_read)(void);	 /* 读 IO 回调 */
	uint32_t long_min_cnt;		 /* 按下按键切换到长按事件最少维持的周期次数 */
	uint32_t up_max_cnt;		 /* 按键弹起到再次按下事件最多维持的周期次数 */
	enum button_level active_lv; /* 按下的有效电平 */
};

/**
 * @brief 创建按键实例
 * 
 * @param p_cfg 参数配置
 * @param cb 按键事件回调
 * @return btn_handle 成功返回句柄，失败时返回 NULL
 */
btn_handle button_ctor(const struct btn_cfg *p_cfg, btn_usr_cb cb);

/**
 * @brief 销毁按键实例，释放资源
 * 
 * @param btn 按键句柄
 */
void button_destroy(btn_handle btn);

/**
 * @brief 对按键进行扫描，需要由调用方周期性执行，这个周期一般设定为按键的防抖间隔
 * 
 * @param btn 按键句柄
 */
void button_scan(btn_handle btn);

#endif /* __VIRTUAL_OS_BUTTON_H__ */
