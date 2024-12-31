/**
 * @file dal_opt.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 应用接口
 * @version 1.0
 * @date 2024-08-12
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

#ifndef _VIRTUAL_OS_DAL_OPT_H
#define _VIRTUAL_OS_DAL_OPT_H

#include <stddef.h>
#include "driver/driver.h"

#define RESERVED_FDS (3)		   /* 前三个文件描述符为内部保留值 */
#define FD_MAX_SIZE MAX_DEVICE_NUM /* 文件描述符最大数量 */

#define DAL_ERR_NONE (0)		 /* 无错误 */
#define DAL_ERR_INVALID (-1)	 /* 无效参数 */
#define DAL_ERR_OVERFLOW (-2)	 /* 超过最大设备数量 */
#define DAL_ERR_UNAVAILABLE (-3) /* 设备不可用 例如未打开 */
#define DAL_ERR_EXCEPTION (-4)	 /* 操作异常，例如对只读设备进行写操作,空指针等 */
#define DAL_ERR_OCCUPIED (-5)	 /* 设备被占用 */
#define DAL_ERR_NOT_EXIST (-6)	 /* 设备不存在 */

/**
 * @brief 打开文件
 * 
 * @param file_name 驱动注册提供的文件名
 * @return int 成功返回>0的文件描述符 失败参考错误码
 */
int dal_open(const char *file_name);

/**
 * @brief 关闭文件
 * 
 * @param fd 文件描述符
 * @return 参考错误码
 */
int dal_close(int fd);

/**
 * @brief 从文件读取数据
 * 
 * @param fd 文件描述符
 * @param buf 读缓冲区
 * @param len 大小
 * @return size_t 返回实际读取字节数
 */
size_t dal_read(int fd, void *buf, size_t len);

/**
 * @brief 向文件写入数据
 * 
 * @param fd 文件描述符
 * @param buf 写缓冲区
 * @param len 大小
 * @return 返回实际写入字节数
 */
size_t dal_write(int fd, void *buf, size_t len);

/**
 * @brief 除设备读写以外的操作
 * 
 * @param fd 文件描述符
 * @param cmd 操作命令
 * @param arg 参数
 * @return 参考错误码 
 */
int dal_ioctl(int fd, int cmd, void *arg);

enum dal_lseek_whence {
	DAL_LSEEK_WHENCE_HEAD, /* 基准点为文件头部 */
	DAL_LSEEK_WHENCE_SET,  /* 基准点为当前位置 */
	DAL_LSEEK_WHENCE_TAIL, /* 基准点为文件末尾 */
};

/**
 * @brief 修改文件指针偏移(例如读写FLASH需要不断修改地址偏移)
 * 
 * @param fd 文件描述符
 * @param offset 偏移大小
 * @param whence 偏移基准点
 * @return int 新的偏移
 */
int dal_lseek(int fd, int offset, enum dal_lseek_whence whence);

#endif /*_VIRTUAL_OS_DAL_OPT_H*/
