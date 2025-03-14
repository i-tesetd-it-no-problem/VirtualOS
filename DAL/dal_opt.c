/**
 * @dev dal_opt.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 应用接口
 * @version 1.0
 * @date 2024-08-12
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

#include <stdint.h>
#include <stdbool.h>
#include "dal/dal_opt.h"
#include "driver/virtual_os_driver.h"
#include "core/virtual_os_config.h"

#define FD_MAX_SIZE (VIRTUALOS_MAX_DEV_NUM + RESERVED_FDS)

struct fd_t {
	struct drv_device *dev;
	bool is_used;
	size_t offset;
};

static struct fd_t fds[FD_MAX_SIZE] = { 0 };

static int alloc_fd(void)
{
	for (uint16_t i = RESERVED_FDS; i < FD_MAX_SIZE; i++) {
		if (!fds[i].is_used) {
			fds[i].is_used = true;
			return i;
		}
	}
	return DAL_ERR_OVERFLOW;
}

static void free_fd(int fd)
{
	if (fd >= RESERVED_FDS && fd < FD_MAX_SIZE) {
		fds[fd].is_used = false;
		fds[fd].dev = NULL;
	}
}

static int check_fd(int fd, struct drv_device **dev)
{
	if (fd < RESERVED_FDS || fd >= FD_MAX_SIZE || !fds[fd].is_used)
		return DAL_ERR_INVALID;

	if (!dev)
		return DAL_ERR_INVALID;

	*dev = fds[fd].dev;
	return DAL_ERR_NONE;
}

int dal_open(const char *node_name)
{
	struct drv_device *dev = find_device(node_name);
	if (!dev)
		return DAL_ERR_NOT_EXIST;

	int new_fd = alloc_fd();
	if (new_fd == DAL_ERR_OVERFLOW)
		return DAL_ERR_OVERFLOW;

	if (!dev->file || !dev->file->opts->open) {
		free_fd(new_fd);
		return DAL_ERR_EXCEPTION;
	}

	int ret = dev->file->opts->open(dev->file);
	if (ret != DAL_ERR_NONE) {
		free_fd(new_fd);
		return ret;
	}

	fds[new_fd].dev = dev;
	return new_fd;
}

int dal_close(int fd)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->close)
		return DAL_ERR_EXCEPTION;

	int ret = dev->file->opts->close(dev->file);
	if (ret != DAL_ERR_NONE)
		return ret;

	free_fd(fd);
	return DAL_ERR_NONE;
}

size_t dal_read(int fd, void *buf, size_t len)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->read)
		return DAL_ERR_EXCEPTION;

	size_t real_len = len;

	// 如果初始化驱动时设置了设备大小，则防止溢出
	if (dev->dev_size > 0)
		real_len = ((dev->dev_size - dev->offset) < len) ? (dev->dev_size - dev->offset) : len;

	return dev->file->opts->read(dev->file, buf, real_len, &dev->offset);
}

size_t dal_write(int fd, void *buf, size_t len)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->write)
		return DAL_ERR_EXCEPTION;

	size_t real_len = len;

	// 如果初始化驱动时设置了设备大小，则防止溢出
	if (dev->dev_size > 0)
		real_len = ((dev->dev_size - dev->offset) < len) ? (dev->dev_size - dev->offset) : len;

	return dev->file->opts->write(dev->file, buf, real_len, &dev->offset);
}

int dal_ioctl(int fd, int cmd, void *arg)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->ioctl)
		return DAL_ERR_EXCEPTION;

	return dev->file->opts->ioctl(dev->file, cmd, arg);
}

int dal_lseek(int fd, int offset, enum dal_lseek_whence whence)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE || dev->dev_size == 0)
		return err;

	uint32_t cur_offset = dev->offset;
	uint32_t dev_size = dev->dev_size;
	uint32_t dest_offset = 0;

	switch (whence) {
	case DAL_LSEEK_WHENCE_HEAD:
		if (offset < 0)
			return DRV_ERR_INVALID;

		dest_offset = (uint32_t)offset;
		break;

	case DAL_LSEEK_WHENCE_SET:
		dest_offset = cur_offset + offset;
		if (dest_offset > dev_size)
			return DRV_ERR_INVALID;
		break;

	case DAL_LSEEK_WHENCE_TAIL:
		dest_offset = dev_size + offset;
		if (dest_offset > dev_size)
			return DRV_ERR_INVALID;

		break;

	default:
		return DRV_ERR_INVALID;
	}

	dev->offset = dest_offset;

	return dest_offset;
}

void dal_init(void)
{
	for (uint16_t i = 0; i < FD_MAX_SIZE; i++) {
		fds[i].is_used = i < RESERVED_FDS ? true : false;
		fds[i].dev = NULL;
	}
}