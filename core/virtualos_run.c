/**
 * @file virtualos_run.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 系统启动入口
 * @version 1.0
 * @date 2024-08-21
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

#include "driver/driver.h"
#include "core/virtualos_run.h"
#include <stdint.h>

extern void dal_init(void);

// 驱动初始化
extern void (*__start_early_driver[])(void);
extern void (*__stop_early_driver[])(void);

static void register_drivers(void)
{
	void (**driver)(void) = __start_early_driver;
	while (driver < __stop_early_driver) {
		if (*driver)
			(*driver)();
		driver++;
	}
}

void virtual_os_init(struct timer_port *port)
{
	driver_manage_init(); /* 初始化驱动管理 */

	register_drivers(); /* 注册所有驱动 */

	dal_init(); /* 初始化应用层接口 */

	stimer_init(port); /* 初始化调度定时器 */
}