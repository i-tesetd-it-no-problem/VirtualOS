/**
 * @file virtual_os_config.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 框架配置
 * @version 1.0
 * @date 2025-03-04
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

#ifndef __VIRTUAL_OS_CONFIG_H__
#define __VIRTUAL_OS_CONFIG_H__

/**
* @brief 
* 
* 请在编译静态库之前修改此文件的宏定义
* 在编译静态库后修改此文件的宏定义将不会生效
* 
*/

// 设备数量配置
#define VIRTUALOS_MAX_DEV_NUM (10)		/* 最大设备数量 同时用于驱动数量以及文件描述符的数量 */
#define VIRTUALOS_MAX_DEV_NAME_LEN (16) /* 最大设备名长度(包括\0) */

// Shell使能配置
// 注: 如果框架使用静态库编译则不建议使用此功能，因为Shell与具体的芯片平台串口有强依赖关系，不适用于静态库链接
// 使用静态库链接时，用户可以通过在应用层单独使用`utils/simple_shell`组件，使用应用层的接口进行注册
// 若要使用此功能，请将框架源码与应用工程一起编译，使能此宏会提供一些框架提供的内置命令
#define VIRTUALOS_SHELL_ENABLE (0)	  /* 使能shell 1:使能 0:禁止 */
#define VIRTUALOS_SHELL_PRIOD_MS (25) /* shell任务周期 默认25ms */

#endif /* __VIRTUAL_OS_CONFIG_H__ */