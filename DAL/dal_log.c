/**
 * @file slog.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 日志接口
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

#include "dal_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "simple_shell.h"

#define MAX_LOG_LENGTH 256           /* 每条日志的最大长度 */

static dal_log_level_e current_log_level = DAL_LOG_LEVEL_ALL;

static struct fs_node *get_syslog_node(void)
{
    static struct fs_node *node = NULL;
    if (!node)
        node = get_node_by_path("/dev/syslog");
    return node;
}

void dal_log_set_time(uint32_t new_time)
{
    uint32_t zone_time = new_time + 8 * 3600;
    struct fs_node *node = get_syslog_node();
    if (node && node->file && node->file->opts && node->file->opts->ioctrl)
        node->file->opts->ioctrl(node->file, 0, &zone_time);
}

uint32_t dal_log_get_time(void)
{
    uint32_t timestamp = 0;
    struct fs_node *node = get_syslog_node();
    if (node && node->file && node->file->opts && node->file->opts->ioctrl)
        node->file->opts->ioctrl(node->file, 1, &timestamp);
    return timestamp;
}

void dal_log_set_level(dal_log_level_e level)
{
    if (level <= DAL_LOG_LEVEL_NONE)
        current_log_level = level;
}

#define LOG_LEVEL_STR(level)                               \
    ((level) == DAL_LOG_LEVEL_DEBUG ? "DEBUG" :            \
     (level) == DAL_LOG_LEVEL_INFO  ? "INFO" :             \
     (level) == DAL_LOG_LEVEL_WARN  ? "WARN" :             \
     (level) == DAL_LOG_LEVEL_ERROR ? "ERROR" :            \
                                      "UNKNOWN")

void dal_log(dal_log_level_e level, const char *func, int line, const char *format, ...)
{
    if (level < current_log_level)
        return;

    char buffer[MAX_LOG_LENGTH];
    int len;
    va_list args;

    va_start(args, format);

    len = snprintf(buffer, sizeof(buffer), "[%s] [%s : %d] : ", LOG_LEVEL_STR(level), func, line);

    int remaining_space = MAX_LOG_LENGTH - len - 1;

    if (remaining_space > 0) {
        int formatted_len = vsnprintf(buffer + len, remaining_space, format, args);
        if (formatted_len > remaining_space) {
            // 截断日志信息
            buffer[MAX_LOG_LENGTH - 1] = '\0';
            len = MAX_LOG_LENGTH - 1;
        } else {
            len += formatted_len;
        }
    } else {
        // 缓冲区已满
        buffer[MAX_LOG_LENGTH - 1] = '\0';
        len = MAX_LOG_LENGTH - 1;
    }

    va_end(args);

    struct fs_node *node = get_syslog_node();
    if (node && node->file && node->file->opts && node->file->opts->write)
        node->file->opts->write(node->file, (uint8_t *)buffer, (size_t)len);
}

void stts(int argc, char *argv[])
{
    if (argc == 2) {
        uint32_t new_time;
        if (sscanf(argv[1], "%u", &new_time) == 1)
            dal_log_set_time(new_time);
    }
}

SPS_EXPORT_CMD(stts, stts, "Set system time by timestamp. Example: stts 1724292500")

void stll(int argc, char *argv[])
{
    if (argc == 2) {
        int level_input;
        if (sscanf(argv[1], "%d", &level_input) == 1 && level_input <= DAL_LOG_LEVEL_NONE)
            dal_log_set_level((dal_log_level_e)level_input);
    }
}

SPS_EXPORT_CMD(stll, stll, "Set log level. Example: stll 1, 0:ALL, 1:DEBUG, 2:INFO, 3:WARN, 4:ERROR, 5:NONE")