/**
 * @file virtual_os_run.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 框架宏定义
 * @version 1.0
 * @date 2025-03-04
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

#ifndef __VIRTUAL_OS_DEFINES_H__
#define __VIRTUAL_OS_DEFINES_H__

/**
 * @brief 断言
 * 
 * @param cond 条件表达式
 *
 * cond 为 false 时，会进入死循环
 * cond 为 true  时，什么都不做 
 */
#define virtual_os_assert(cond)                                                                                        \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			while (1)                                                                                                  \
				;                                                                                                      \
		}                                                                                                              \
	} while (0)

/**
 * @brief 分支预测
 * 
 * likely(x)   用于指示 x 表达式的结果是经常为真的，编译器可以对其进行优化，以提高效率；
 * unlikely(x) 用于指示 x 表达式的结果是经常为假的，编译器可以对其进行优化，以提高效率；
 * 
 */
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif /* __VIRTUAL_OS_DEFINES_H__ */