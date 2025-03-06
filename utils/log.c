/**
 * @file log.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 日志组件
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "utils/log.h"
#include "utils/queue.h"

#if USE_TIME_STAMP
#include <time.h>
#endif

#define LOG_LEVEL_STR(level)                                                                                           \
	((level) == LOG_LEVEL_DEBUG			 ? "DEBUG"                                                                     \
			: (level) == LOG_LEVEL_INFO	 ? "INFO"                                                                      \
			: (level) == LOG_LEVEL_WARN	 ? "WARN"                                                                      \
			: (level) == LOG_LEVEL_ERROR ? "ERROR"                                                                     \
										 : "XXXXX")

static uint8_t log_buffer[LOG_BUFFER_SIZE]; /* 日志缓冲区 */

struct syslog_instance {
	struct log_interface *interface;  // 串口接口
	struct queue_info log_queue;	  // 日志队列
	uint32_t timestamp;				  // 时间戳
	uint32_t pre_time;				  // 时间戳计数
	uint32_t period_md;				  // 任务周期
	bool initialized;				  // 是否初始化
	enum log_level current_log_level; // 当前日志等级
};

static struct syslog_instance syslog = { 0 };

/**
 * @brief 检查日志实例是否有效
 * 
 * @param instance 日志实例指针
 * @return true 有效
 * @return false 无效
 */
static bool check_instance(struct syslog_instance *instance)
{
	return instance && instance->initialized && instance->interface && instance->interface->write &&
		instance->interface->read && instance->interface->check_over;
}

/**
 * @brief 向日志队列中写入日志
 * 
 * @param instance 日志实例
 * @param buf 日志内容缓冲区
 * @param len 日志内容长度
 * @return size_t 实际写入的字节数
 */
static size_t syslog_write(struct syslog_instance *instance, uint8_t *buf, size_t len)
{
	if (!check_instance(instance))
		return 0;

#if USE_TIME_STAMP
	char time_buffer[64] = "NO_TIME";

	time_t raw_time = (time_t)instance->timestamp;
	struct tm time_info;

	if (localtime_r(&raw_time, &time_info) != NULL) {
		strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S]", &time_info);
	} else {
		snprintf(time_buffer, sizeof(time_buffer), "[NO_TIME]");
	}

	char new_buf[MAX_LOG_LENGTH];
	size_t new_len = snprintf(new_buf, sizeof(new_buf), "%s %.*s", time_buffer, (int)len, buf);

	if (new_len >= MAX_LOG_LENGTH) {
		// 如果日志长度超过最大长度，截断
		new_len = MAX_LOG_LENGTH - 1;
		// 移除手动添加 '\0'
	}

	// 使用 new_buf 作为要发送的日志内容
	buf = (uint8_t *)new_buf;
	len = new_len;
#endif

	size_t total_len = len + sizeof(size_t); // 2 字节用于存储长度信息

	if (queue_remain_space(&instance->log_queue) < total_len) {
		queue_advance_rd(&instance->log_queue, total_len);
	}

	if (queue_add(&instance->log_queue, (uint8_t *)&len, sizeof(size_t)) != sizeof(size_t))
		return 0;

	if (queue_add(&instance->log_queue, buf, len) != len)
		return 0;

	return len;
}

/**
 * @brief 日志显示
 * 
 * @param instance 任务实例
 */
static void syslog_show(struct syslog_instance *instance)
{
	if (!check_instance(instance))
		return;

#if USE_TIME_STAMP
	instance->pre_time += instance->period_md;

	if (instance->pre_time >= 1000) {
		instance->timestamp++;
		instance->pre_time = 0;
	}
#endif

	while (!is_queue_empty(&instance->log_queue) && instance->interface->check_over()) {
		// 日志长度信息
		size_t flush_len = 0;
		if (queue_get(&instance->log_queue, (uint8_t *)&flush_len, sizeof(size_t)) != sizeof(size_t)) {
			return;
		}

		if (flush_len == 0 || flush_len > MAX_LOG_LENGTH)
			return;

		// 取出日志
		uint8_t tmp_buf[MAX_LOG_LENGTH];
		if (queue_get(&instance->log_queue, tmp_buf, flush_len) != flush_len)
			return;

		// 发送日志
		instance->interface->write(tmp_buf, flush_len);
	}
}

/**************************API**************************/

/**
 * @brief 日志初始化
 * 
 * @param interface 串口接口
 * @param period_ms 任务周期（毫秒）
 */
void syslog_init(struct log_interface *interface, uint32_t period_ms)
{
	if (!interface || !interface->read || !interface->write || !interface->check_over)
		return;

	syslog.interface = interface;
	syslog.period_md = period_ms;
	syslog.current_log_level = LOG_LEVEL_INFO; // 默认日志等级为INFO

	queue_init(&syslog.log_queue, sizeof(uint8_t), log_buffer, LOG_BUFFER_SIZE);

	syslog.initialized = true;
}

/**
 * @brief 日志任务
 * 
 */
void syslog_task(void)
{
	syslog_show(&syslog);
}

/**
 * @brief 日志发送
 * 
 * @param level 日志等级
 * @param func 函数名
 * @param line 行号
 * @param format 日志格式
 * @param ... 可变参数
 */
void origin_log(enum log_level level, const char *func, int line, const char *format, ...)
{
	if (!check_instance(&syslog))
		return;

	if (level < syslog.current_log_level)
		return;

	char buffer[MAX_LOG_LENGTH];
	int len;
	va_list args;

	va_start(args, format);

	len = snprintf(buffer, sizeof(buffer), "[%-5s] [%-20s:%-4d] : ", LOG_LEVEL_STR(level), func, line);

	if (len < 0) {
		// snprintf 出错
		va_end(args);
		return;
	}

	int remaining_space = MAX_LOG_LENGTH - len;

	if (remaining_space > 1) {
		int formatted_len = vsnprintf(buffer + len, remaining_space, format, args);
		if (formatted_len < 0)
			len = MAX_LOG_LENGTH - 1;
		else if (formatted_len >= remaining_space)
			len = MAX_LOG_LENGTH - 1;
		else
			len += formatted_len;
	} else
		len = MAX_LOG_LENGTH - 1;

	va_end(args);

	syslog_write(&syslog, (uint8_t *)buffer, len);
}

/**
 * @brief 设置日志等级
 * 
 * @param level 日志等级
 */
void syslog_set_level(enum log_level level)
{
	if (check_instance(&syslog))
		syslog.current_log_level = level;
}

/* 设置系统时间戳 */
void syslog_set_time(uint32_t timestamp)
{
	if (check_instance(&syslog))
		syslog.timestamp = timestamp;
}

/* 获取系统当前时间戳 */
uint32_t syslog_get_time(void)
{
	if (check_instance(&syslog))
		return syslog.timestamp;
	return 0;
}
