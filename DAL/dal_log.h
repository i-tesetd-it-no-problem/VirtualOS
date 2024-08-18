/**
 * @file dal_log.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 日志接口
 * @version 1.0
 * @date 2024-08-13
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

#ifndef _VIRTUAL_OS_DAL_LOG_H
#define _VIRTUAL_OS_DAL_LOG_H

#include "dal_opt.h"
#include <stdint.h>

typedef enum {
	DAL_LOG_LEVEL_ALL = 0, /* 所有日志 */
	DAL_LOG_LEVEL_DEBUG = 1, /* 调试日志 */
	DAL_LOG_LEVEL_INFO = 2, /* 信息日志 */
	DAL_LOG_LEVEL_WARN = 3, /* 警告日志 */
	DAL_LOG_LEVEL_ERROR = 4, /* 错误日志 */
	DAL_LOG_LEVEL_NONE = 5 /* 关闭日志 */
} dal_log_level_e;

void dal_log(dal_log_level_e level, const char *file, int line, const char *format, ...); /* 不建议使用，建议使用宏定义 */

/****************************************API****************************************/

/* 设置系统时间戳 */
void dal_log_set_time(uint32_t timestamp);

/* 获取系统当前时间戳 */
uint32_t dal_log_get_time(void);

/* 设置日志等级 */
void dal_log_set_level(dal_log_level_e level);

#define dal_log_d(format, ...) dal_log(DAL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__) /* 调试日志 */
#define dal_log_i(format, ...) dal_log(DAL_LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__) /* 信息日志 */
#define dal_log_w(format, ...) dal_log(DAL_LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__) /* 警告日志 */
#define dal_log_e(format, ...) dal_log(DAL_LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__) /* 错误日志 */

#endif /*_VIRTUAL_OS_DAL_LOG_H*/