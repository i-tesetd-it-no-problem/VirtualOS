/**
 * @file simple_shell.c
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

#include "simple_shell.h"
#include <stdio.h>
#include <string.h>

#define HANDLE_QUEUE_SIZE 256
#define MAX_ARGC SPS_CMD_MAX_ARGS

// 全局变量
const sp_shell_cmd_t *command_list[MAX_COMMANDS];
int command_count = 0;
static hash_table_t shell_table;
static uint8_t handle_queue_buffer[HANDLE_QUEUE_SIZE];

typedef struct {
	sp_shell_opts_t opts;
	queue_info_t handle_queue;
	uint16_t pdu_len;
	uint8_t cmd[SPS_CMD_MAX];
	bool is_start;
	bool is_hash_saved;
} sp_shell_t;

static sp_shell_t sps = {
	.is_start = false,
	.is_hash_saved = false,
	.pdu_len = 0,
	.cmd = { 0 },
	.handle_queue = { 0 },
};

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
				argv[(*argc)++] = arg_start;
				arg_start = NULL;
			}
		} else if (*p == '\\' && in_quotes && (*(p + 1) == 'n' || *(p + 1) == 't')) {
			*p = (*(p + 1) == 'n') ? '\n' : '\t';
			memmove(p + 1, p + 2, strlen(p + 2) + 1);
		} else if (*p == ' ' && !in_quotes) {
			if (arg_start) {
				*p = '\0';
				argv[(*argc)++] = arg_start;
				arg_start = NULL;
			}
		} else if (!arg_start)
			arg_start = p;
		p++;
	}

	if (arg_start)
		argv[(*argc)++] = arg_start;
}

static void shell_cmd_handler(sp_shell_t *sps)
{
	int argc = 0;
	char *argv[MAX_ARGC] = { NULL };

	parse_command((char *)sps->cmd, argv, &argc);

	if (argc > 0) {
		hash_error_t error;
		sp_shell_cmd_t *cmd = (sp_shell_cmd_t *)hash_find(&shell_table, argv[0], &error);
		if (cmd && cmd->cb && error == HASH_SUCCESS)
			cmd->cb(argc, argv);
	}

	sps->pdu_len = 0;
	memset(sps->cmd, 0, SPS_CMD_MAX);
}

static void shell_parser(sp_shell_t *sps)
{
	uint8_t c;
	bool is_cmd = false;

	while (!is_queue_empty(&sps->handle_queue)) {
		queue_get(&sps->handle_queue, &c, 1);

		if (c == '\r')
			continue;
		else if (c == '\n') {
			is_cmd = true;
			break;
		} else if (sps->pdu_len < SPS_CMD_MAX - 1)
			sps->cmd[sps->pdu_len++] = c;
	}

	if (is_cmd)
		shell_cmd_handler(sps);
}

int compare_commands(const void *a, const void *b)
{
	const sp_shell_cmd_t *cmd_a = *(const sp_shell_cmd_t **)a;
	const sp_shell_cmd_t *cmd_b = *(const sp_shell_cmd_t **)b;

	return (cmd_a && cmd_b) ? strcmp(cmd_a->name, cmd_b->name) : 0;
}

static void hash_save_cmd(void)
{
	if (!sps.is_hash_saved) {
		qsort(command_list, command_count, sizeof(sp_shell_cmd_t *), compare_commands);

		for (int i = 0; i < command_count; i++) {
			const sp_shell_cmd_t *cmd = command_list[i];
			if (cmd)
				hash_insert(&shell_table, cmd->name, (void *)cmd);
		}
		sps.is_hash_saved = true;
	}
}

static void list(int argc, char *argv[])
{
	if (argc == 1) {
		const char *header = "Available commands:\n";
		sps.opts.write((uint8_t *)header, strlen(header));

		for (int i = 0; i < command_count; i++) {
			const sp_shell_cmd_t *cmd = command_list[i];
			if (cmd) {
				char buffer[256];
				int len = snprintf(buffer, sizeof(buffer), "  %-20s - %s\n", cmd->name, cmd->description);
				sps.opts.write((uint8_t *)buffer, len);
			}
		}

		const char *footer = "\n";
		sps.opts.write((uint8_t *)footer, 1);
	}
}

SPS_EXPORT_CMD(list, list, "List all available commands")

/***************************************API***************************************/

int simple_shell_init(sp_shell_opts_t *opts)
{
	if (!opts || !opts->read || !opts->write)
		return 1;

	sps.opts = *opts;

	if (init_hash_table(&shell_table, SPS_CMD_MAX) != HASH_SUCCESS)
		return 1;

	if (queue_init(&sps.handle_queue, 1, handle_queue_buffer, HANDLE_QUEUE_SIZE) == 0)
		return 1;

	sps.is_start = true;

	return 0;
}

void shell_dispatch(void)
{
	if (!sps.is_start)
		return;

	hash_save_cmd();

	uint8_t tmp[HANDLE_QUEUE_SIZE];
	int ret = sps.opts.read(tmp, HANDLE_QUEUE_SIZE);
	if (ret <= 0)
		return;

	queue_add(&sps.handle_queue, tmp, ret);
	shell_parser(&sps);
}
