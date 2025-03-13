/**
 * @file simple_shell.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 简易shell
 * @version 1.0
 * @date 2024-08-19
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

//

#ifndef __VIRTUAL_OS_SIMPLE_SHELL_H__
#define __VIRTUAL_OS_SIMPLE_SHELL_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define HISTORY_SIZE 10	   // 历史记录最大数量
#define SPS_CMD_MAX 64	   // 单条命令最大长度
#define SPS_CMD_MAX_ARGS 4 // 单条命令的最大参数数量
#define MAX_COMMANDS 16	   // 系统可注册命令的最大数量
#define MAX_OUT_LEN 512	   // 命令输出缓冲区长度

/**
 * @brief 命令回调函数
 * @param argc 参数数量
 * @param argv 参数列表
 * @param out 输出缓冲区
 * @param buffer_size 输出缓冲区大小
 * @param out_len 实际写入输出缓冲区的字节数
 */
typedef void (*sp_shell_cb)(int argc, char *argv[], uint8_t *out, size_t buffer_size, size_t *out_len);

/**
 * @brief Shell命令结构体
 */
struct sp_shell_cmd_t {
	const char *name;		 /* 命令名 */
	sp_shell_cb cb;			 /* 命令回调函数 */
	const char *description; /* 命令描述信息 */
};

/**
 * @brief 读写回调接口
 */
struct sp_shell_opts {
	size_t (*read)(uint8_t *buf, size_t len);  /* 读函数指针 */
	size_t (*write)(uint8_t *buf, size_t len); /* 写函数指针 */
};

// 命令注册宏
#define SPS_EXPORT_CMD(_name, _callback, _description)                                                                 \
	const struct sp_shell_cmd_t shell_cmd_##_name = {                                                                  \
		.name = #_name,                                                                                                \
		.cb = _callback,                                                                                               \
		.description = _description,                                                                                   \
	};                                                                                                                 \
	__attribute__((constructor)) static void register_##_name(void)                                                    \
	{                                                                                                                  \
		extern const struct sp_shell_cmd_t *command_list[MAX_COMMANDS];                                                \
		extern int command_count;                                                                                      \
		if (command_count < MAX_COMMANDS)                                                                              \
			command_list[command_count++] = (struct sp_shell_cmd_t *)&shell_cmd_##_name;                               \
	}

/**
 * @brief shell初始化
 * 
 * @param opts 回调接口
 * @return true 
 * @return false 
 */
bool simple_shell_init(struct sp_shell_opts *opts);

/**
 * @brief 启动调度
 * 
 */
void shell_dispatch(void);

#endif /* __VIRTUAL_OS_SIMPLE_SHELL_H__ */