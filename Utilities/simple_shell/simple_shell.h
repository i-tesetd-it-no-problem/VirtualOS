/**
 * @file simple_shell.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 简易shell
 * @version 1.0
 * @date 2024-08-19
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

#include "string_hash.h"
#include "queue.h"
#include "dal.h"

typedef void (*sp_shell_cb)(int argc, char *argv[]); /* 命令回调函数 */

typedef struct {
	const char *name; /* 命令名 */
	sp_shell_cb cb; /* 命令回调函数 */
	const char *description; // 命令描述信息
} sp_shell_cmd_t;

extern const sp_shell_cmd_t *command_list[];
extern int command_count;

typedef struct {
	int (*read)(uint8_t *buf, size_t size); /* 读函数指针 */
	int (*write)(uint8_t *buf, size_t size); /* 写函数指针 */
} sp_shell_opts_t;

int simple_shell_init(sp_shell_opts_t *opts); /* 初始化shell */

void shell_dispatch(void); /* 调度shell */

/***********************************以上内容请勿修改***********************************/
//
//
//
//
/*************************************用户使用区域*************************************/

#define SPS_CMD_MAX_ARGS 8 /* 允许的参数最大数量 */
#define SPS_CMD_MAX 256 /* 单条命令最大长度 */
#define MAX_COMMANDS 64 /* 可存储命令最大数量 */

/**
 * @brief 导出命令宏
 * @param _name 命令名
 * @param _callback 命令回调函数
 * @param _description 命令描述信息
 */

#define SPS_EXPORT_CMD(_name, _callback, _description)                                                                                                         \
	const sp_shell_cmd_t shell_cmd_##_name = {                                                                                                             \
		.name = #_name,                                                                                                                                \
		.cb = _callback,                                                                                                                               \
		.description = _description,                                                                                                                   \
	};                                                                                                                                                     \
	__attribute__((constructor)) static void register_##_name(void)                                                                                        \
	{                                                                                                                                                      \
		if (command_count < MAX_COMMANDS) {                                                                                                            \
			command_list[command_count++] = &shell_cmd_##_name;                                                                                    \
		}                                                                                                                                              \
	}
