/**
 * @file simple_shell.c
 * @author 
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

#include "utils/simple_shell.h"
#include "dal/dal_opt.h"
#include "utils/queue.h"
#include "utils/string_hash.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 宏定义集中管理
#define RX_QUEUE_SIZE 256
#define TX_QUEUE_SIZE 512

// 所有命令
const struct sp_shell_cmd_t *command_list[MAX_COMMANDS];
int command_count = 0;

static struct hash_table shell_table; // 命令哈希表

static uint8_t rx_buffer[RX_QUEUE_SIZE]; // 接收缓冲
static uint8_t tx_buffer[TX_QUEUE_SIZE]; // 发送缓冲

typedef struct {
	struct sp_shell_opts *opts;
	struct queue_info rx_queue;
	struct queue_info tx_queue;
	uint16_t pdu_len;
	uint8_t cmd[SPS_CMD_MAX];
	bool is_start;
	bool is_hash_saved;
} sp_shell_t;

static sp_shell_t sps = { 0 };

static void parse_command(char *cmd, char *argv[], int *argc)
{
	*argc = 0;
	bool in_quotes = false;
	char *p = cmd;
	char *arg_start = NULL;

	while (*p != '\0') {
		if (*p == '"') {
			in_quotes = !in_quotes;
			if (in_quotes) {
				if (!arg_start)
					arg_start = p + 1;
			} else {
				*p = '\0';
				if (*argc < SPS_CMD_MAX_ARGS) { // 添加 argc 检查
					argv[(*argc)++] = arg_start;
				}
				arg_start = NULL;
			}
		} else if (*p == '\\' && in_quotes && (*(p + 1) == 'n' || *(p + 1) == 't')) {
			*p = (*(p + 1) == 'n') ? '\n' : '\t';
			memmove(p + 1, p + 2, strlen(p + 2) + 1);
		} else if (*p == ' ' && !in_quotes) {
			if (arg_start) {
				*p = '\0';
				if (*argc < SPS_CMD_MAX_ARGS) { // 添加 argc 检查
					argv[(*argc)++] = arg_start;
				}
				arg_start = NULL;
			}
		} else if (!arg_start) {
			arg_start = p;
		}
		p++;
	}

	if (arg_start && *argc < SPS_CMD_MAX_ARGS) // 添加 argc 检查
		argv[(*argc)++] = arg_start;
}

static void add_msg(sp_shell_t *sps, uint8_t *msg, size_t len)
{
	if (!sps || !msg || len == 0)
		return;

	// 检查 queue_add 的返回值
	if (queue_add(&sps->tx_queue, (uint8_t *)&len, sizeof(size_t)) != sizeof(size_t)) {
		// 处理队列添加失败，例如记录错误或丢弃消息
		return;
	}
	if (queue_add(&sps->tx_queue, msg, len) != len) {
		// 处理队列添加失败，例如记录错误或丢弃消息
		return;
	}
}

static void shell_cmd_handler(sp_shell_t *sps)
{
	if (!sps)
		return;

	int argc = 0;
	char *argv[SPS_CMD_MAX_ARGS] = { NULL };

	uint8_t out[MAX_OUT_LEN] = { 0 };
	size_t out_buffer_size = MAX_OUT_LEN;
	size_t out_len = MAX_OUT_LEN;

	parse_command((char *)sps->cmd, argv, &argc);

	if (argc > 0) {
		enum hash_error error;

		struct sp_shell_cmd_t *cmd = (struct sp_shell_cmd_t *)hash_find(&shell_table, argv[0], &error);

		if (cmd && cmd->cb && out_len <= out_buffer_size && error == HASH_SUCCESS) {
			cmd->cb(argc, argv, out, out_buffer_size, &out_len);
			if (out_len <= out_buffer_size)
				add_msg(sps, out, out_len);
		}
	}

	sps->pdu_len = 0;

	memset(sps->cmd, 0, SPS_CMD_MAX);
}

static void shell_parser(sp_shell_t *sps)
{
	if (!sps)
		return;

	uint8_t c;
	bool is_cmd = false;

	while (!is_queue_empty(&sps->rx_queue)) {
		if (queue_get(&sps->rx_queue, &c, sizeof(uint8_t)) != sizeof(uint8_t))
			break;

		if (c == '\r')
			continue;
		else if (c == '\n') {
			is_cmd = true;
			break;
		} else {
			if (sps->pdu_len < SPS_CMD_MAX - 1) {
				sps->cmd[sps->pdu_len++] = c;
			} else {
				sps->pdu_len = 0;
				memset(sps->cmd, 0, SPS_CMD_MAX);
				return;
			}
		}
	}

	if (is_cmd)
		shell_cmd_handler(sps);
}

int compare_commands(const void *a, const void *b)
{
	const struct sp_shell_cmd_t *cmd_a = *(const struct sp_shell_cmd_t **)a;
	const struct sp_shell_cmd_t *cmd_b = *(const struct sp_shell_cmd_t **)b;

	if (!cmd_a && !cmd_b)
		return 0;
	if (!cmd_a)
		return -1;
	if (!cmd_b)
		return 1;

	return strcmp(cmd_a->name, cmd_b->name);
}

static void hash_save_cmd(void)
{
	if (!sps.is_hash_saved) {
		// 确保 command_list 中不包含 NULL
		for (int i = 0; i < command_count; i++) {
			if (command_list[i] == NULL) {
				// 跳过 NULL 元素
				continue;
			}
		}

		qsort(command_list, command_count, sizeof(struct sp_shell_cmd_t *), compare_commands);

		for (int i = 0; i < command_count; i++) {
			const struct sp_shell_cmd_t *cmd = command_list[i];
			if (cmd)
				hash_insert(&shell_table, cmd->name, (void *)cmd);
		}

		sps.is_hash_saved = true;
	}
}

static void list(int argc, char *argv[], uint8_t *out, size_t buffer_size, size_t *out_len)
{
	if (!sps.opts->write || !sps.opts->check_over || !sps.opts->check_over())
		return;

	if (argc == 1) {
		size_t pos = 0;

		const char *header = "Available commands:\n";
		size_t header_len = strlen(header);
		if (pos + header_len < buffer_size) {
			memcpy(out + pos, header, header_len);
			pos += header_len;
		} else
			return; // 输出缓冲区不足，退出

		for (int i = 0; i < command_count; i++) {
			const struct sp_shell_cmd_t *cmd = command_list[i];
			if (cmd) {
				char buffer[256];
				int len = snprintf(buffer, sizeof(buffer), "  %-20s - %s\n", cmd->name, cmd->description);
				if (len < 0) {
					// snprintf 出错，跳过此命令
					continue;
				}
				if ((size_t)len >= sizeof(buffer)) {
					// 输出被截断，调整 len
					len = sizeof(buffer) - 1;
				}
				if (pos + len < buffer_size) {
					memcpy(out + pos, buffer, len);
					pos += len;
				} else
					return; // 输出缓冲区不足，退出
			}
		}

		const char *footer = "\n";
		size_t footer_len = strlen(footer);
		if (pos + footer_len < buffer_size) {
			memcpy(out + pos, footer, footer_len);
			pos += footer_len;
		} else
			return;

		// 更新输出长度
		*out_len = pos;
	}
}

SPS_EXPORT_CMD(list, list, "List all available commands")

static void flush_tx_queue(sp_shell_t *sps)
{
	if (!sps || !sps->opts->write || !sps->opts->check_over || !sps->opts->check_over())
		return;

	// 获取长度
	size_t flush_len = 0;
	if (queue_get(&sps->tx_queue, (uint8_t *)&flush_len, sizeof(size_t)) != sizeof(size_t)) {
		return;
	}

	if (flush_len == 0 || flush_len > TX_QUEUE_SIZE)
		return;

	// 获取内容
	uint8_t tmp_buf[TX_QUEUE_SIZE];
	if (queue_get(&sps->tx_queue, tmp_buf, flush_len) != flush_len)
		return;

	// 发送
	if (sps->opts->write(tmp_buf, flush_len) != flush_len) {
	}
}

/***************************************API***************************************/

bool simple_shell_init(struct sp_shell_opts *opts)
{
	if (!opts || !opts->read || !opts->write || !opts->check_over)
		return false;

	sps.opts = opts;

	if (init_hash_table(&shell_table, SPS_CMD_MAX) != HASH_SUCCESS)
		return false;

	if (!queue_init(&sps.rx_queue, sizeof(uint8_t), rx_buffer, RX_QUEUE_SIZE)) {
		destroy_hash_table(&shell_table);
		return false;
	}

	if (!queue_init(&sps.tx_queue, sizeof(uint8_t), tx_buffer, TX_QUEUE_SIZE)) {
		queue_destroy(&sps.rx_queue);
		destroy_hash_table(&shell_table);
		return false;
	}

	sps.is_start = true;

	return true;
}

void shell_dispatch(void)
{
	if (!sps.is_start)
		return;

	hash_save_cmd();

	flush_tx_queue(&sps);

	uint8_t tmp[RX_QUEUE_SIZE] = { 0 };
	size_t ret = sps.opts->read(tmp, RX_QUEUE_SIZE);
	if (ret == 0)
		return;

	size_t add_ret = queue_add(&sps.rx_queue, tmp, ret);
	if (add_ret != ret)
		return;

	shell_parser(&sps);
}
