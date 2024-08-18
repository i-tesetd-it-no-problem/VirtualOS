/**
 * @file device.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 设备类型定义
 * @version 1.0
 * @date 2024-08-21
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

#ifndef __VIRTUAL_OS_DEVICE_H__
#define __VIRTUAL_OS_DEVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
	DRV_ERR_NOT_EXIST = -6,   	/* 设备不存在 */
	DRV_ERR_OCCUPIED,    		/* 设备被占用 */
	DRV_ERR_EXCEPTION,   		/* 操作异常，例如对只读设备进行写操作,空指针等 */
	DRV_ERR_UNAVALIABLE, 		/* 设备不可用（例如未打开) */
	DRV_ERR_OVERFLOW,    		/* 超过最大设备数量 */
	DRV_ERR_INVALID,     		/* 无效参数 */
	DRV_ERR_NONE,	     		/* 无错误 */
} drv_err_e;

typedef struct drv_file_opts_t drv_file_opts_t;

typedef struct {
	drv_file_opts_t *opts; /*文件操作接口*/
	bool is_opened;	       /*打开标志*/
	size_t seek_pos;	   /*读写索引*/
	size_t dev_size;	   /*设备大小*/
	void *private_data;    /*私有数据*/
} drv_file_t;

typedef enum {
	DRV_LSEEK_WHENCE_HEAD, /* 基准点为文件头部 */
	DRV_LSEEK_WHENCE_SET,  /* 基准点为当前位置 */
	DRV_LSEEK_WHENCE_TAIL, /* 基准点为文件末尾 */
} drv_lseek_whence_e;

struct drv_file_opts_t {
	drv_err_e (*open)(drv_file_t *file);
	drv_err_e (*close)(drv_file_t *file);
	drv_err_e (*ioctrl)(drv_file_t *file, int cmd, void *arg);
	int (*read)(drv_file_t *file, uint8_t *buf, size_t len);
	int (*write)(drv_file_t *file, const uint8_t *buf, size_t len);
	int (*lseek)(drv_file_t *file, int offset, drv_lseek_whence_e whence);
};


/**
 * @brief 设备注册
 * 
 * @param file_opts 
 * @param name 
 * @return true 
 * @return false 
 */
bool device_register(drv_file_opts_t *file_opts, const char *name);

#endif /* __VIRTUAL_OS_DEVICE_H__ */