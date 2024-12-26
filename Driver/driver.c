/**
 * @file driver.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 设备注册
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

#include <stdlib.h>
#include "utils/string_hash.h"
#include "driver/driver.h"

static struct hash_table driver_table = { 0 };

/**
 * @brief 初始化驱动管理
 * 
 */
void driver_manage_init(void)
{
	enum hash_error err = HASH_POINT_ERROR;

	err = init_hash_table(&driver_table, MAX_DEVICE_NUM);
	if (err != HASH_SUCCESS)
		return;
}

/**
 * @brief 注册设备
 * 
 * @param drv_init 驱动初始化
 * @param file_opts 驱动文件操作
 * @param name 设备名称
 * @return true 
 * @return false 
 */
bool driver_register(driver_init drv_init, const struct file_operations *file_opts, const char *name)
{
	enum hash_error err = HASH_POINT_ERROR;

	struct drv_device *dev = calloc(1, sizeof(struct drv_device));
	if (!dev)
		return false;

	dev->file = calloc(1, sizeof(struct drv_file));
	if (!dev->file)
		goto free_dev;

	dev->file->opts = file_opts;

	if (!drv_init())
		goto free_file;

	err = hash_insert(&driver_table, name, (void *)dev);
	if (err != HASH_SUCCESS)
		goto free_file;

	return true;

free_file:
	free(dev->file);

free_dev:
	free(dev);

	return false;
}

/**
 * @brief 查找设备
 * 
 * @param name 哈希表名
 * @return struct drv_device* 设备结构体
 */
struct drv_device *find_device(const char *name)
{
	struct drv_device *dev = NULL;
	enum hash_error err;
	dev = (struct drv_device *)hash_find(&driver_table, name, &err);
	if (err == HASH_SUCCESS)
		return dev;
	else
		return NULL;
}