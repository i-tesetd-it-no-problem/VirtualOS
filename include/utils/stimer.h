/**
 * @file stimer.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 前后台调度组件
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

#ifndef _VIRTUAL_OS_STIMER_H
#define _VIRTUAL_OS_STIMER_H

#define STIMER_PERIOD_PER_TICK_MS (1)

#include <stdint.h>
#include <stdbool.h>

/**************************************系统API/**************************************/

typedef void (*stimer_timeout_process)(void);
typedef void (*stimer_base_init)(uint32_t period_ms, stimer_timeout_process f_timeout);
typedef void (*stimer_base_start)(void);

typedef void (*stimer_f)(void);

struct timer_port {
	volatile stimer_base_init f_init;
	volatile stimer_base_start f_start;
};

/**
 * @brief 调度定时器初始化
 * 
 * @param port 提供一个 定时器的初始化函数和启动函数
 * @return bool 成功返回true，失败返回false
 */
bool stimer_init(struct timer_port *port);

/**************************************用户可用API/**************************************/

/**
 * @brief 创建周期任务
 * 
 * @param init_f 初始化函数指针
 * @param task_f 任务函数指针
 * @param period_ms 任务周期,单位毫秒
 * @return bool 成功返回true，失败返回false
 */
bool stimer_task_create(stimer_f init_f, stimer_f task_f, uint32_t period_ms);

/**
 * @brief 运行时创建单次任务
 * 
 * @param task_f 任务函数指针
 * @param ms 指定毫秒后执行
 * @return bool 成功返回true，失败返回false
 */
bool defer_task_create(stimer_f task_f, uint32_t ms);

/**
 * @brief 开启调度
 * 
 */
void stimer_start(void);

#endif /*_VIRTUAL_OS_STIMER_H*/