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

//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "utils/simple_shell.h"
#include "utils/queue.h"
#include "utils/string_hash.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MAIN_NAME "VirtualOS"			// 前缀
#define PROMPT MAIN_NAME "@admin\r\n$ " // 终端名

// 全局命令缓存
const struct sp_shell_cmd_t *command_list[MAX_COMMANDS];
int command_count = 0;

// 队列大小
#define RX_QUEUE_SIZE (SPS_CMD_MAX * 2)
#define TX_QUEUE_SIZE MAX_OUT_LEN

// 历史记录
static char history[HISTORY_SIZE][SPS_CMD_MAX];
static int history_count = 0;
static int history_index = -1; // -1表示不在历史回放模式

static struct hash_table shell_cmd_table; // 命令哈希表

// 上下文
struct shell_context {
	struct sp_shell_opts *opts;
	struct queue_info rx_queue;
	struct queue_info tx_queue;
	size_t cmd_len;
	uint8_t cmd_buf[SPS_CMD_MAX];
	bool is_active;
	bool hash_initialized;
};

// 全局上下文
static struct shell_context shell_ctx;

// 按字母排序
static int cmd_compare(const void *p1, const void *p2)
{
	const struct sp_shell_cmd_t *cmdA = *(const struct sp_shell_cmd_t **)p1;
	const struct sp_shell_cmd_t *cmdB = *(const struct sp_shell_cmd_t **)p2;
	if (!cmdA && !cmdB)
		return 0;
	if (!cmdA)
		return 1;
	if (!cmdB)
		return -1;
	return strcmp(cmdA->name, cmdB->name);
}

// 按字母排序后插入哈希表
static void hash_save_cmd_once(void)
{
	if (shell_ctx.hash_initialized) {
		return;
	}

	qsort((void *)command_list, command_count, sizeof(struct sp_shell_cmd_t *), cmd_compare);

	for (int i = 0; i < command_count; i++) {
		if (!command_list[i])
			continue;
		hash_insert(&shell_cmd_table, command_list[i]->name, (void *)command_list[i]);
	}

	shell_ctx.hash_initialized = true;
}

// 添加发送消息
static void add_msg(uint8_t *msg, size_t len)
{
	if (len == 0)
		return;

	// 先写入长度
	if (queue_add(&shell_ctx.tx_queue, &len, sizeof(len)) != sizeof(len)) {
		return;
	}

	// 再写入数据
	(void)queue_add(&shell_ctx.tx_queue, (void *)msg, len);
}

// 加入历史记录
static void add_to_history(const char *cmd)
{
	if (cmd[0] == '\0') {
		return; // 忽略空命令
	}

	if (history_count < HISTORY_SIZE) {
		strncpy(history[history_count], cmd, SPS_CMD_MAX - 1);
		history[history_count][SPS_CMD_MAX - 1] = '\0';
		history_count++;
	} else {
		// 覆盖最早记录
		for (int i = 0; i < HISTORY_SIZE - 1; i++) {
			strcpy(history[i], history[i + 1]);
		}
		strncpy(history[HISTORY_SIZE - 1], cmd, SPS_CMD_MAX - 1);
		history[HISTORY_SIZE - 1][SPS_CMD_MAX - 1] = '\0';
	}

	// 重置索引
	history_index = -1;
}

// 把一行命令切分成argv数组、argc数量
static void parse_command(char *input, char *argv[], int *argc)
{
	*argc = 0;
	bool in_quotes = false;
	char *p = input;

	while (*p && *argc < SPS_CMD_MAX_ARGS - 1) {
		// 跳过空格(如果当前不在引号中)
		while (*p == ' ' && !in_quotes) {
			p++;
		}
		if (!*p)
			break;

		// 引号
		if (*p == '"') {
			in_quotes = !in_quotes;
			p++;
			continue;
		}

		// 记录参数起始
		argv[(*argc)++] = p;
		if (*argc >= SPS_CMD_MAX_ARGS - 1) {
			break;
		}

		// 找参数结束
		while (*p) {
			// 处理转义 \n \t
			if (*p == '\\' && (*(p + 1) == 'n' || *(p + 1) == 't')) {
				*p = (*(p + 1) == 'n') ? '\n' : '\t';
				memmove(p + 1, p + 2, strlen(p + 2) + 1);
				continue;
			}
			if (*p == '"') {
				in_quotes = !in_quotes;
				*p = '\0';
				p++;
				break;
			}
			if (!in_quotes && *p == ' ') {
				*p = '\0';
				p++;
				break;
			}
			p++;
		}
	}
	argv[*argc] = NULL;
}

// 处理用户输入命令
static void process_command(char *cmd_str, uint8_t *output, size_t *output_len)
{
	char *argv[SPS_CMD_MAX_ARGS] = { 0 };
	int argc = 0;
	parse_command(cmd_str, argv, &argc); // 解析命令
	if (argc == 0)
		return;

	enum hash_error err;
	struct sp_shell_cmd_t *cmd = (struct sp_shell_cmd_t *)hash_find(&shell_cmd_table, argv[0], &err);

	if (cmd && cmd->cb) {
		// 存在命令
		cmd->cb(argc, argv, output, *output_len, output_len);
	} else {
		// 不存在命令
		static const char *err_msg = "command not found\r\n";
		memcpy(output, err_msg, strlen(err_msg));
	}
}

// 重新命令 用于切换历史记录
static void rewrite_cmdline(struct shell_context *sh_ctx, size_t del_cnt, uint8_t *new_cmd, size_t new_cmd_len)
{
	uint8_t send_buf[SPS_CMD_MAX * 4]; // 每字符 3字节退格 + 新命令
	size_t pos = 0;

	// 擦除旧命令
	for (size_t i = 0; i < del_cnt && (pos + 3) <= sizeof(send_buf); i++) {
		send_buf[pos++] = '\b';
		send_buf[pos++] = ' ';
		send_buf[pos++] = '\b';
	}

	// 拷贝新命令
	if (pos + new_cmd_len <= sizeof(send_buf)) {
		memcpy(send_buf + pos, new_cmd, new_cmd_len);
		pos += new_cmd_len;
	}

	// 回显
	sh_ctx->opts->write(send_buf, pos);
}

// 处理换行
static void handle_newline(struct shell_context *sh_ctx)
{
	// 查找命令并执行
	uint8_t output[MAX_OUT_LEN] = { '\r', '\n' };
	size_t output_len = MAX_OUT_LEN - 2;

	if (sh_ctx->cmd_len > 0) {
		sh_ctx->cmd_buf[sh_ctx->cmd_len] = '\0';

		add_to_history((const char *)sh_ctx->cmd_buf); // 加入历史记录

		// 执行命令
		process_command((char *)sh_ctx->cmd_buf, output + 2, &output_len);

		// 清空输入
		memset(sh_ctx->cmd_buf, 0, SPS_CMD_MAX);
		sh_ctx->cmd_len = 0;
	} else {
		memcpy(output + 2, "\r\n", 2);
		output_len = 2;
	}

	size_t remain_len = MAX_OUT_LEN - 2 - output_len;
	size_t cpy_len = MIN(remain_len, strlen(PROMPT));

	history_index = -1;
	memcpy(output + 2 + output_len, PROMPT, cpy_len);

	// 加入新行
	add_msg(output, output_len + 2 + strlen(PROMPT));
}

// 删除键
static void handle_backspace(struct shell_context *sh_ctx)
{
	if (sh_ctx->cmd_len > 0) {
		// 输出退格序列覆盖
		static const char bs_seq[] = "\b \b";
		sh_ctx->opts->write((uint8_t *)bs_seq, sizeof(bs_seq) - 1);
		sh_ctx->cmd_len--;
		sh_ctx->cmd_buf[sh_ctx->cmd_len] = '\0';
	}
}

// 上箭头
static void handle_up_arrow(struct shell_context *sh_ctx)
{
	if (history_count == 0)
		return;

	if (history_index == -1)
		history_index = history_count - 1;
	else if (history_index > 0)
		history_index--;

	size_t del_cnt = sh_ctx->cmd_len; // 需要删除的字符数

	// 复制上一条历史记录
	memset(sh_ctx->cmd_buf, 0, SPS_CMD_MAX);
	strncpy((char *)sh_ctx->cmd_buf, history[history_index], SPS_CMD_MAX - 1);
	sh_ctx->cmd_len = strlen((char *)sh_ctx->cmd_buf);

	// 重写
	rewrite_cmdline(sh_ctx, del_cnt, sh_ctx->cmd_buf, sh_ctx->cmd_len);
}

// 下箭头
static void handle_down_arrow(struct shell_context *sh_ctx)
{
	if (history_index == -1 || history_count == 0)
		return;

	history_index++;
	size_t del_cnt = sh_ctx->cmd_len; // 需要删除的字符数

	if (history_index >= history_count) {
		// 已超过最新记录，清空输入行
		history_index = -1;
		rewrite_cmdline(sh_ctx, del_cnt, (uint8_t *)"", 0);
		sh_ctx->cmd_len = 0;
		memset(sh_ctx->cmd_buf, 0, SPS_CMD_MAX);
	} else {
		memset(sh_ctx->cmd_buf, 0, SPS_CMD_MAX);
		strncpy((char *)sh_ctx->cmd_buf, history[history_index], SPS_CMD_MAX - 1);
		sh_ctx->cmd_len = strlen((char *)sh_ctx->cmd_buf);

		rewrite_cmdline(sh_ctx, del_cnt, sh_ctx->cmd_buf, sh_ctx->cmd_len);
	}
}

// 非特殊字符 直接回显
static void handle_regular_char(struct shell_context *sh_ctx, uint8_t ch)
{
	if (sh_ctx->cmd_len < (SPS_CMD_MAX - 1)) {
		sh_ctx->cmd_buf[sh_ctx->cmd_len++] = ch;
		// 回显
		sh_ctx->opts->write(&ch, 1);
	} else {
		// 指令太长
		static const uint8_t err_msg[] = "\r\n!command too long!\r\n";
		sh_ctx->opts->write((uint8_t *)err_msg, sizeof(err_msg) - 1);
		sh_ctx->cmd_len = 0;
		memset(sh_ctx->cmd_buf, 0, SPS_CMD_MAX);
	}
}

// 解析输入的字符串
static void shell_parser(struct shell_context *sh_ctx)
{
	uint8_t ch;
	while (!is_queue_empty(&sh_ctx->rx_queue)) {
		if (queue_get(&sh_ctx->rx_queue, &ch, 1) != 1)
			break;

		if (ch == '\r' || ch == '\n') {
			// 换行
			handle_newline(sh_ctx);
		} else if (ch == 0x08 || ch == 0x7F) {
			// 删除
			handle_backspace(sh_ctx);
		} else if (ch == 0x1B) {
			// 可能是方向键等转义序列
			uint8_t seq[2];
			if (queue_get(&sh_ctx->rx_queue, &seq[0], 1) && queue_get(&sh_ctx->rx_queue, &seq[1], 1)) {
				if (seq[0] == '[') {
					if (seq[1] == 'A') {
						handle_up_arrow(sh_ctx); // 上箭头
					} else if (seq[1] == 'B') {
						handle_down_arrow(sh_ctx); // 下箭头
					}
					// 左右箭头
				}
			}
		} else
			handle_regular_char(sh_ctx, ch); // 其他字符，回显
	}
}

// 刷新发送缓冲
static void flush_tx_buffer(struct shell_context *sh_ctx)
{
	if (!sh_ctx || !sh_ctx->opts->write) {
		return;
	}

	size_t flush_len = 0;

	// 读取长度
	if (queue_get(&sh_ctx->tx_queue, (uint8_t *)&flush_len, sizeof(size_t)) != sizeof(size_t))
		return;

	if (flush_len == 0 || flush_len > TX_QUEUE_SIZE)
		return;

	// 读取消息
	uint8_t tmp_buf[TX_QUEUE_SIZE];
	if (queue_get(&sh_ctx->tx_queue, tmp_buf, flush_len) != flush_len)
		return;

	// 发送
	sh_ctx->opts->write(tmp_buf, flush_len);
}

/* ====================== 内置命令: list ====================== */
static void list_cmd(int argc, char *argv[], uint8_t *out, size_t buf_size, size_t *out_len)
{
	const char *header = "Available commands:\r\n";
	size_t pos = 0;

	size_t header_len = strlen(header);
	size_t copy_len = MIN(header_len, buf_size - pos);
	if (copy_len > 0) {
		memcpy(out + pos, header, copy_len);
		pos += copy_len;
	}

	for (int i = 0; i < command_count && pos < buf_size; i++) {
		if (!command_list[i])
			continue;
		const char *desc = command_list[i]->description ? command_list[i]->description : "";
		int available = buf_size - pos;
		if (available <= 0)
			break;

		int line_len = snprintf(NULL, 0, "  %-20s - %s\r\n", command_list[i]->name, desc);
		if (line_len < 0)
			break;
		if (line_len > available)
			break;

		snprintf((char *)(out + pos), available, "  %-20s - %s\r\n", command_list[i]->name, desc);
		pos += line_len;
	}

	if (pos + 2 <= buf_size) {
		out[pos++] = '\r';
		out[pos++] = '\n';
	} else if (pos + 1 <= buf_size) {
		out[pos++] = '\r';
	}

	*out_len = pos;
}
SPS_EXPORT_CMD(list, list_cmd, "show all available commands")

/* ====================== 内置命令: clear ====================== */
static void cmd_clear(int argc, char *argv[], uint8_t *out, size_t buf_size, size_t *out_len)
{
	// 清屏
	const char *clear_screen = "\x1b[2J\x1b[H";
	size_t clear_len = strlen(clear_screen);

	if (clear_len < buf_size) {
		memcpy(out, clear_screen, clear_len);
		*out_len = clear_len;
	} else {
		*out_len = 0;
		return;
	}
}
SPS_EXPORT_CMD(clear, cmd_clear, "clear the screen")

/* ====================== 内置命令: history ====================== */
static void cmd_history(int argc, char *argv[], uint8_t *out, size_t buf_size, size_t *out_len)
{
	const char *header = "Command history:\r\n";
	size_t pos = 0;

	size_t header_len = strlen(header);
	size_t copy_len = MIN(header_len, buf_size - pos);
	if (copy_len > 0) {
		memcpy(out + pos, header, copy_len);
		pos += copy_len;
	}

	for (int i = 0; i < history_count && pos < buf_size; i++) {
		int available = buf_size - pos;
		if (available <= 0)
			break;

		int line_len = snprintf(NULL, 0, "  %d: %s\r\n", i + 1, history[i]);
		if (line_len < 0)
			break;
		if (line_len > available)
			break;

		snprintf((char *)(out + pos), available, "  %d: %s\r\n", i + 1, history[i]);
		pos += line_len;
	}

	if (pos + 2 <= buf_size) {
		out[pos++] = '\r';
		out[pos++] = '\n';
	} else if (pos + 1 <= buf_size) {
		out[pos++] = '\r';
	}

	*out_len = pos;
}
SPS_EXPORT_CMD(history, cmd_history, "show command history")

/**
 * @brief shell初始化
 * 
 * @param opts 回调接口
 * @return true 
 * @return false 
 */
bool simple_shell_init(struct sp_shell_opts *opts)
{
	if (!opts || !opts->read || !opts->write)
		return false;

	memset(&shell_ctx, 0, sizeof(shell_ctx));
	shell_ctx.opts = opts;

	// 初始化哈希表（容量 = MAX_COMMANDS*2）减少碰撞
	if (init_hash_table(&shell_cmd_table, MAX_COMMANDS * 2) != HASH_SUCCESS)
		return false;

	static uint8_t rx_buffer[RX_QUEUE_SIZE];
	static uint8_t tx_buffer[TX_QUEUE_SIZE];

	if (!queue_init(&shell_ctx.rx_queue, sizeof(uint8_t), rx_buffer, RX_QUEUE_SIZE))
		return false;

	if (!queue_init(&shell_ctx.tx_queue, sizeof(uint8_t), tx_buffer, TX_QUEUE_SIZE)) {
		queue_destroy(&shell_ctx.rx_queue);
		return false;
	}

	shell_ctx.is_active = true;

	// 启动时先发出提示符
	add_msg((uint8_t *)PROMPT, strlen(PROMPT));

	return true;
}

/**
  * @brief 启动调度
  * 
  */
void shell_dispatch(void)
{
	if (!shell_ctx.is_active)
		return;

	// 只在第一次时把命令插入哈希表
	hash_save_cmd_once();

	// 读取
	uint8_t tmp_buf[RX_QUEUE_SIZE];
	size_t bytes_read = shell_ctx.opts->read(tmp_buf, RX_QUEUE_SIZE);
	if (bytes_read > 0)
		queue_add(&shell_ctx.rx_queue, tmp_buf, bytes_read);

	// 解析
	shell_parser(&shell_ctx);

	// 刷新
	flush_tx_buffer(&shell_ctx);
}