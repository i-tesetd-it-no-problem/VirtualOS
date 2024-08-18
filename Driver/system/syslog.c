/**
 * @file syslog.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 一个简易Shell组件,实现了日志驱动以及Shell命令解析功能
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"
#include "stimer.h"
#include "queue.h"

static void free_log(void);

/************************************************用户修改区域************************************************/
//
// 1. 如需要使用日志功能,请手动完成以下步骤
// 2. 初始化相关外设,log_device_init,若使用串口,强烈建议使用DMA发送减少CPU占用率,如使用类似JLink组件则可以不用初始化
// 3. 实现log_device_transmit接口的具体内容
// 4. 每当发送完成时调用free_log函数,例如使用DMA+串口时，在中断中调用

#define USE_TIME_STAMP 1 /* 日志启用时显示时间,0为关闭,1为启用 开启会编译time.h头文件，将占用大量FLASH空间 */

/* 请勿修改接口定义 */
static void log_device_init(void)
{
	/* 完成日志所用设备的初始化操作,例如串口的初始化 */
}

/* 此处可以自行实现平台对应中断处理函数,记住无论使用何种方式判断发送完成,最后都要调用 free_log() */

/* 请勿修改接口定义 */
static int log_device_transmit(uint8_t *buf, size_t len)
{
	/* 完成发送接口,例如串口发送,SEGGER_RTT_Write等 */

	return len; /* 返回发送成功的字节数 */
}

/* 请勿修改接口定义 */
static int log_device_recieve(uint8_t *buf, size_t len)
{
	/* 完成接收接口,例如串口接收,SEGGER_RTT_Read等 */

	return 0; /* 返回实际接收的字节长度 */
}

//
//
//
//
//
//
//
/******************************************************************************************
 *                                      以下部分用户无需修改                            	*
 ******************************************************************************************/

#if USE_TIME_STAMP
#include <time.h>
#endif

#include "simple_shell.h"

static const char syslog_name[] = "syslog";
static int syslog_write(drv_file_t *file, const uint8_t *buf, size_t len);
static drv_err_e syslog_ioctrl(drv_file_t *file, int cmd, void *arg);
static void log_device_init(void);

static drv_file_opts_t syslog_dev = {
	.write = syslog_write,
	.ioctrl = syslog_ioctrl,
};

typedef struct {
	size_t len;
	uint8_t *buf;
} msg_t;

msg_t *cur_log = NULL;
volatile bool dma_transfer_complete = true;

static uint8_t log_buf[128] = { 0 }; /* 32条日志 */
static queue_info_t log_q;

static uint32_t pre_time = 0;
static uint32_t timestamp = 0;

static int syslog_write(drv_file_t *file, const uint8_t *buf, size_t len)
{
#if USE_TIME_STAMP
	char time_buffer[64] = "NO_TIME";

	time_t raw_time = (time_t)timestamp;
	struct tm *time_info = localtime(&raw_time);

	if (time_info != NULL) {
		strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S]", time_info);
	} else {
		snprintf(time_buffer, sizeof(time_buffer), "[NO_TIME]");
	}

	size_t new_len = len + strlen(time_buffer) + 2; // 时间戳 + 空格 + 日志内容
	char *new_buf = (char *)malloc(new_len);
	if (new_buf == NULL) {
		return -1;
	}

	snprintf(new_buf, new_len, "%s %s", time_buffer, buf);
#else
	char *new_buf = (char *)malloc(len + 1);
	if (new_buf == NULL) {
		return -1;
	}

	memcpy(new_buf, buf, len);
	new_buf[len] = '\0';
	size_t new_len = len;
#endif

	msg_t *new_msg = (msg_t *)malloc(sizeof(msg_t));
	if (new_msg == NULL) {
		free(new_buf);
		return -1;
	}

	new_msg->buf = (uint8_t *)new_buf;
	new_msg->len = new_len;
	memcpy(new_msg->buf, new_buf, new_len);

	queue_add(&log_q, (uint8_t *)&new_msg, sizeof(new_msg));

	return new_len;
}

static drv_err_e syslog_ioctrl(drv_file_t *file, int cmd, void *arg)
{
	switch (cmd) {
	case 0:
		timestamp = *(uint32_t *)arg;

		break;
	case 1:
		*(uint32_t *)arg = timestamp;
		break;
	default:
		break;
	}

	return DRV_ERR_NONE;
}

int shell_write(uint8_t *buf, size_t len)
{
	char *new_buf = (char *)malloc(len);
	if (!new_buf)
		return -1;

	memcpy(new_buf, buf, len);

	msg_t *new_msg = (msg_t *)malloc(sizeof(msg_t));
	if (!new_msg) {
		free(new_buf);
		return -1;
	}

	new_msg->buf = (uint8_t *)new_buf;
	new_msg->len = len;

	queue_add(&log_q, (uint8_t *)&new_msg, sizeof(new_msg));

	return len;
}

static sp_shell_opts_t opts = {
	.read = log_device_recieve,
	.write = shell_write,
};

static const char *start_msg = "\
Welcome to VirtualOS!\n\
This is a very lightweight shell component.\n\
You can use `SPS_EXPORT_CMD` to export and add any commands you want\n\
Use the command -- 'list' to view all available commands.\n\
\n\n\n";

void syslog_init(void);
void syslog_init(void)
{
	log_device_init();

	queue_init(&log_q, 1, log_buf, sizeof(log_buf));

	simple_shell_init(&opts);

	device_register(&syslog_dev, syslog_name);

	log_device_transmit((uint8_t *)start_msg, strlen(start_msg));
}

static void syslog_show(void)
{
#if USE_TIME_STAMP
	pre_time += STIMER_PERIOD_PER_TICK_MS;

	if (pre_time >= 1000) {
		timestamp++;
		pre_time = 0;
	}
#endif

	if (is_queue_empty(&log_q))
		return;

	if (!dma_transfer_complete)
		return;

	dma_transfer_complete = false;

	queue_get(&log_q, (uint8_t *)&cur_log, sizeof(msg_t *));

	log_device_transmit(cur_log->buf, cur_log->len);
}

void syslog_task(void);
void syslog_task(void)
{
	syslog_show();

	shell_dispatch();
}

static void free_log(void)
{
	if (cur_log) {
		if (cur_log->buf)
			free(cur_log->buf);
		free(cur_log);
		cur_log = NULL;
		dma_transfer_complete = true;
	}
}