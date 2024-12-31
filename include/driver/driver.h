/**
 * @file driver.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 驱动管理
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

#ifndef __VIRTUAL_OS_DRIVER_H__
#define __VIRTUAL_OS_DRIVER_H__

#include <stdbool.h>
#include <stddef.h>

#define MAX_DEVICE_NUM (32) /* 最大设备数量 */

#define DRV_ERR_NONE (0)		 /* 无错误 */
#define DRV_ERR_INVALID (-1)	 /* 无效参数 */
#define DRV_ERR_OVERFLOW (-2)	 /* 超过最大设备数量 */
#define DRV_ERR_UNAVAILABLE (-3) /* 设备不可用 例如未打开 */
#define DRV_ERR_EXCEPTION (-4)	 /* 操作异常，例如对只读设备进行写操作,空指针等 */
#define DRV_ERR_OCCUPIED (-5)	 /* 设备被占用 */
#define DRV_ERR_NOT_EXIST (-6)	 /* 设备不存在 */

/**
 * @brief 初始化驱动管理
 * 
 */
void driver_manage_init(void);

/**
 * @brief 查找设备
 * 
 * @param name 哈希表名
 * @return struct drv_device* 设备结构体
 */
struct drv_device *find_device(const char *name);

struct drv_file;

// 驱动文件
struct drv_file {
	const struct file_operations *opts; /* 文件操作接口 */
	bool is_opened;						/* 是否已打开 */
	void *private;						/* 私有数据 */
};

// 驱动设备
struct drv_device {
	struct drv_file *file; // 文件
	size_t dev_size;	   /* 设备大小 */
	size_t offset;		   // 文件偏移
};

/****************************USER API*****************************/
// 文件操作接口
struct file_operations {
	int (*open)(struct drv_file *file);						 /* 打开设备 返回结果参考错误码 */
	int (*close)(struct drv_file *file);					 /* 关闭设备 返回结果参考错误码 */
	int (*ioctl)(struct drv_file *file, int cmd, void *arg); /* 控制命令 返回结果参考错误码 */
	size_t (*read)(struct drv_file *file, void *buf, size_t len, size_t *offset);  /* 读取数据 */
	size_t (*write)(struct drv_file *file, void *buf, size_t len, size_t *offset); /* 写入数据 */
};

// 驱动注册宏
#define EXPORT_DRIVER(_func)                                                                                           \
	void _func(void);                                                                                                  \
	void (*_func##_ptr)(void) __attribute__((section(".early_driver"), used)) = &_func;

// 驱动初始化函数指针
typedef bool (*driver_init)(struct drv_device *dev);

/**
 * @brief 注册设备
 * 
 * @param drv_init 驱动初始化
 * @param file_opts 驱动文件操作
 * @param name 设备名称
 * @return true 
 * @return false 
 */
bool driver_register(driver_init drv_init, const struct file_operations *file_opts, const char *name);

/**
 * @brief 设置设备私有数据
 * 
 * @param dev 设备结构体
 * @param private 私有数据
 */
void set_dev_private(struct drv_device *dev, void *private);

/**
 * @brief 获取设备私有数据
 * 
 * @param name 设备名称
 * @return void* 私有数据
 */
void *get_dev_private(const char *name);

#endif /* __VIRTUAL_OS_DRIVER_H__ */