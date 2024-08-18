/**
 * @file platform_schedule.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 框架调度核心实现,需要用户实现一个定时器接口内容
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

#include <stdint.h>
#include "sys.h"
#include "stimer.h"

stimer_timeout_process stimer_cb = NULL;

/************************************************用户修改区域************************************************/
// 1. 初始化定时器的初始化函数,并将定时器的回调函数设置为stimer_cb
// 2. 实现定时器的启动函数
// 3. 实现定时器的中断处理函数,并在中断处理函数中调用f_timeout回调函数

#if 1
#error "你必须手动实现以下函数的具体内容,并在中断处理函数中调用f_timeout回调函数"
#error "实现后请手动关闭这条错误警告"
#endif

/* 请勿修改接口定义 */
static void _stimer_base_init(uint32_t period, stimer_timeout_process f_timeout)
{
	/*   实现定时器的初始化操作 period : 周期 , f_timeout : 中断回调函数  */

	stimer_cb = f_timeout;
}

/* 请勿修改接口定义 */
static void _stimer_base_start(void)
{
	/*   实现定时器的启动操作 */
}


/* 实现平台中断处理函数,在中断处理函数中调用stimer_cb回调函数 */

/******************************************************************************************
 *                                      以下部分用户无需修改                            	*
 ******************************************************************************************/
static void _stimer_base_init(uint32_t period, stimer_timeout_process f_timeout);
static void _stimer_base_start(void);

struct timer_port m_tmr = {
	.f_init = _stimer_base_init,
	.f_start = _stimer_base_start,
};

void platform_stimer_init(void);
void platform_stimer_init(void)
{
	stimer_init(&m_tmr);
}

/******************************************************************************************/