/**
 * @dev dal_opt.c
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

#include <stdint.h>
#include "dal/dal_opt.h"

typedef struct {
	struct drv_device *dev;
	uint8_t is_used;
	size_t offset;
} fd_t;

static fd_t fds[FD_MAX_SIZE] = { 0 };

static int alloc_fd(void)
{
	for (uint16_t i = RESERVED_FDS; i < FD_MAX_SIZE; i++) {
		if (!fds[i].is_used) {
			fds[i].is_used = 1;
			return i;
		}
	}
	return DAL_ERR_OVERFLOW;
}

static void free_fd(int fd)
{
	if (fd >= RESERVED_FDS && fd < FD_MAX_SIZE) {
		fds[fd].is_used = 0;
		fds[fd].dev = NULL;
	}
}

static int check_fd(int fd, struct drv_device **dev)
{
	if (fd < RESERVED_FDS || fd >= FD_MAX_SIZE || !fds[fd].is_used || !dev)
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

	if (!dev || !dev->file || !dev->file->opts->open) {
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

size_t dal_read(int fd, uint8_t *buf, size_t len)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->read)
		return DAL_ERR_EXCEPTION;

	return dev->file->opts->read(dev->file, buf, len, &dev->offset);
}

size_t dal_write(int fd, uint8_t *buf, size_t len)
{
	struct drv_device *dev;
	int err = check_fd(fd, &dev);
	if (err != DAL_ERR_NONE)
		return err;

	if (!dev->file->opts->write)
		return DAL_ERR_EXCEPTION;

	return dev->file->opts->write(dev->file, buf, len, &dev->offset);
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
	if (err != DAL_ERR_NONE)
		return err;

	uint32_t cur_offset = dev->offset; // 当前偏移
	uint32_t dev_size = dev->dev_size; // 设备大小
	uint32_t dest_offset = 0;		   // 目标偏移

	switch (whence) {
	case DAL_LSEEK_WHENCE_HEAD: /* 从文件头开始 */
		if (offset < 0)
			return DRV_ERR_INVALID;

		dest_offset = (uint32_t)offset;
		break;

	case DAL_LSEEK_WHENCE_SET: /* 从当前位置开始 */
		dest_offset = cur_offset + offset;
		if (dest_offset > dev_size)
			return DRV_ERR_INVALID; // 超出设备范围
		break;

	case DAL_LSEEK_WHENCE_TAIL: /* 从文件尾部开始 */
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
		fds[i].is_used = i < RESERVED_FDS ? 1 : 0;
		fds[i].dev = NULL;
	}
}