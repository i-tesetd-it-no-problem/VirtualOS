/**
 * @file internal_commands.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 所有的内置指令
 * @version 1.0
 * @date 2024-08-20
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

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "simple_shell.h"
#include "sys.h"

/****************************************************向设备写入数据****************************************************/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_STR_LEN 256

/* 引号包裹为字符串 */
bool is_quoted_string(const char *str)
{
	size_t len = strlen(str);
	return len > 1 && str[0] == '"' && str[len - 1] == '"';
}

/* 去掉字符串两端的引号 */
void strip_quotes(char *str)
{
	size_t len = strlen(str);
	if (len > 1 && str[0] == '"' && str[len - 1] == '"') {
		memmove(str, str + 1, len - 2);
		str[len - 2] = '\0';
	}
}

/* 判断是否为纯数字 */
bool is_numeric(const char *str)
{
	if (*str == '\0')
		return false;

	while (*str) {
		if (!isdigit(*str))
			return false;
		str++;
	}

	return true;
}

static void echo_cmd(int argc, char *argv[])
{
	if (argc != 4)
		return;

	if (strcmp(argv[2], ">") != 0 && strcmp(argv[2], ">>") != 0)
		return;

	struct fs_node *node = get_node_by_path(argv[3]);
	if (!node || node->node_type != FS_FILE || !node->file || !node->file->opts || !node->file->opts->write)
		return;

	if (is_quoted_string(argv[1])) {
		char str_value[MAX_STR_LEN];
		strncpy(str_value, argv[1], sizeof(str_value) - 1);
		str_value[sizeof(str_value) - 1] = '\0';
		strip_quotes(str_value);
		size_t len = strlen(str_value);
		node->file->opts->write(node->file, str_value, len);
	} else if (is_numeric(argv[1])) {
		int value = atoi(argv[1]);
		node->file->opts->write(node->file, (uint8_t *)&value, sizeof(value));
	} else {
		const char *str_value = argv[1];
		size_t len = strlen(str_value);
		node->file->opts->write(node->file, str_value, len);
	}
}

SPS_EXPORT_CMD(echo, echo_cmd, "Echo a value to a device or file. Usage: echo [value] > [device] Example: echo 1 > /dev/led")

/****************************************************向设备写入数据****************************************************/