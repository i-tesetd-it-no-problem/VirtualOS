/**
 * @file syslog.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 系统日志组件
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

#include <stdint.h>
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
// 5. 实现log_device_recieve接口的具体内容

#define USE_TIME_STAMP 1 /* 日志启用时显示时间,0为关闭,1为启用 开启会编译time.h头文件，将占用大量FLASH空间 */
#define MAX_LOG_LENGTH 256  /* 每条日志的最大长度 不建议修改 */

#if (MAX_LOG_LENGTH & (MAX_LOG_LENGTH -1)) != 0
#error "MAX_LOG_LENGTH must be a power of 2"
#endif

/* 请勿修改接口定义 */
static void log_device_init(void)
{
	/* 完成日志所用设备的初始化操作,例如串口的初始化 */
}

/* 此处可以自行实现平台对应中断处理函数,记住无论使用何种方式判断发送完成,最后都要调用 free_log() */

/* 请勿修改接口定义 */
static int log_device_transmit(uint8_t *buf, size_t len)
{
	/* 完成发送接口,例如串口发送, SEGGER_RTT_Write 等 */

	return len; /* 返回发送成功的字节数 */
}

/* 请勿修改接口定义 */
static int log_device_recieve(uint8_t *buf, size_t len)
{
	/* 完成接收接口,例如串口接收, SEGGER_RTT_Read 等 */

	return 0; /* 返回实际接收的字节长度 */
}

/******************************************************************************************
 *                                      以下部分用户无需修改                               *
 ******************************************************************************************/

#if USE_TIME_STAMP
#include <time.h>
#endif

#include "simple_shell.h"

#define LOG_FRAME_SIZE_OCCUPY (2) /* 两个字节存储长度 */
#define TOTAL_FRAME_COUNT (1 << 3) // 一定要是2的幂
#define LOG_BUFFER_SIZE (MAX_LOG_LENGTH * TOTAL_FRAME_COUNT) /* 日志缓冲区总大小 2K */

static struct queue_info log_queue; /* 日志队列 */
static uint8_t log_buffer[LOG_BUFFER_SIZE]; /* 日志缓冲区 */
static const char syslog_name[] = "syslog";
static int syslog_write(drv_file_t *file, const uint8_t *buf, size_t len);
static drv_err_e syslog_ioctrl(drv_file_t *file, int cmd, void *arg);
static void log_device_init(void);

static drv_file_opts_t syslog_dev = {
    .write = syslog_write,
    .ioctrl = syslog_ioctrl,
};

volatile bool dma_transfer_complete = true;

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

    char new_buf[MAX_LOG_LENGTH];
    size_t new_len = snprintf(new_buf, sizeof(new_buf), "%s %.*s", time_buffer, (int)len, buf);

    if (new_len >= MAX_LOG_LENGTH) {
        // 如果日志长度超过最大长度，截断
        new_len = MAX_LOG_LENGTH - 1;
        new_buf[new_len] = '\0';
    }
#else
    const char *new_buf = (const char *)buf;
    size_t new_len = len;
    if (new_len >= MAX_LOG_LENGTH) {
        // 如果日志长度超过最大长度，截断
        new_len = MAX_LOG_LENGTH - 1;
    }
#endif

    uint16_t total_len = (uint16_t)(new_len + LOG_FRAME_SIZE_OCCUPY);  // 2 字节用于存储长度信息

    if (queue_remain_space(&log_queue) < total_len) {
        // 队列空间不足，覆盖旧日志
        queue_advance_rd(&log_queue, total_len);
    }

    // 将长度信息写入队列（高字节在前）
    uint8_t len_high = (uint8_t)((new_len >> 8) & 0xFF);
    uint8_t len_low = (uint8_t)(new_len & 0xFF);

    uint8_t length_bytes[LOG_FRAME_SIZE_OCCUPY] = {len_high, len_low};

    queue_add(&log_queue, length_bytes, LOG_FRAME_SIZE_OCCUPY);  // 写入长度信息

    queue_add(&log_queue, (uint8_t *)new_buf, new_len);  // 写入日志内容

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
    return syslog_write(NULL, buf, len);
}

static sp_shell_opts_t opts = {
    .read = log_device_recieve,
    .write = shell_write,
};

static const char *start_msg = "\
Welcome to VirtualOS!\r\n\
This is a very lightweight shell component.\r\n\
You can use `SPS_EXPORT_CMD` to export and add any commands you want\r\n\
Use the command -- 'list' to view all available commands.\r\n\
\r\n\r\n";

void syslog_init(void);
void syslog_init(void)
{
    log_device_init();

    queue_init(&log_queue, 1, log_buffer, LOG_BUFFER_SIZE, NULL, NULL);

    simple_shell_init(&opts);

    device_register(&syslog_dev, syslog_name);

    syslog_write(NULL, (uint8_t *)start_msg, strlen(start_msg));
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

    if (is_queue_empty(&log_queue) || !dma_transfer_complete)
        return;

    dma_transfer_complete = false;

    // 日志长度信息
    uint8_t length_bytes[LOG_FRAME_SIZE_OCCUPY];
    if (queue_get(&log_queue, length_bytes, LOG_FRAME_SIZE_OCCUPY) != LOG_FRAME_SIZE_OCCUPY) {
        dma_transfer_complete = true;
        return;
    }

    uint16_t log_len = (length_bytes[0] << 8) | length_bytes[1];

    if (log_len == 0 || log_len > MAX_LOG_LENGTH) {
        dma_transfer_complete = true;
        return;
    }

    // 取出日志
	uint8_t tmp_buf[MAX_LOG_LENGTH];
    if (queue_get(&log_queue, tmp_buf, log_len) != log_len) {
        dma_transfer_complete = true;
        return;
    }

    // 发送日志
    log_device_transmit(tmp_buf, log_len);
}

void syslog_task(void)
{
    syslog_show();

    shell_dispatch();
}

static void free_log(void)
{
    dma_transfer_complete = true;
}
