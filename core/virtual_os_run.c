/**
 * @file virtual_os_run.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 系统启动入口
 * @version 1.0
 * @date 2024-08-21
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

#include <stdint.h>

#include "driver/virtual_os_driver.h"
#include "core/virtual_os_run.h"
#include "core/virtual_os_defines.h"

extern void dal_init(void);

extern void (*__start_early_driver[])(void);
extern void (*__stop_early_driver[])(void);

static void register_drivers(void)
{
	size_t driver_count = __stop_early_driver - __start_early_driver;

	for (size_t i = 0; i < driver_count; i++) {
		if (__start_early_driver[i])
			__start_early_driver[i]();
	}
}

void virtual_os_init(struct timer_port *port)
{
	driver_manage_init();

	register_drivers();

	dal_init();

	virtual_os_assert(stimer_init(port));
}