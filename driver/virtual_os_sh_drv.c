/**
 * @file virtual_os_sh_drv.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 底层Shell驱动模板，需要根据实际情况实现
 * @version 1.0
 * @date 2024-08-21
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
#include "core/virtual_os_config.h"

#if VIRTUALOS_SHELL_ENABLE // 使能此宏

#include <string.h>

#include "utils/simple_shell.h"

#include "driver/virtual_os_driver.h"

/**
 * @brief 初始化平台相关的串口
 * 
 */
static void platform_serial_init(void)
{
	/* 平台相关的串口初始化 */
}

/**
 * @brief 平台相关的串口的接收回调
 * 
 */
static struct sp_shell_opts sh_opts = {
	.read = NULL,
	.write = NULL,
};

/* ====================== 框架内置命令: show_device ====================== */
static void show_device(int argc, char *argv[], uint8_t *out, size_t buf_size, size_t *out_len)
{
	// 列出所有注册的设备

	if (argc != 1) {
		*out_len = 0;
		return;
	}

	char msg[VIRTUALOS_MAX_DEV_NUM * VIRTUALOS_MAX_DEV_NAME_LEN] = { 0 };
	fill_all_device_name(msg, VIRTUALOS_MAX_DEV_NUM * VIRTUALOS_MAX_DEV_NAME_LEN);
	*out_len = strlen(msg);
	if (*out_len > buf_size) {
		*out_len = 0;
		return;
	}
	memcpy(out, msg, *out_len);
}
SPS_EXPORT_CMD(show_device, show_device, "list all devices")

/************************************EXPOSE API************************************/

/**
 * @brief Shell初始化
 * 
 */
void virtual_os_shell_init(void)
{
	platform_serial_init(); // 平台串口初始化

	if (!sh_opts.read || !sh_opts.write)
		return;

#define VIRTUAL_OS_WELCOME_MSG "Welcome to VirtualOS!\r\n"

	simple_shell_init(&sh_opts, VIRTUAL_OS_WELCOME_MSG); // Shell 初始化
}

/**
 * @brief Shell调度
 * 
 */
void virtual_os_shell_task(void)
{
	shell_dispatch();
}

#endif /* VIRTUALOS_SHELL_ENABLE */
