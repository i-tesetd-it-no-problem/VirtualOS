/**
 * @file dal_opt.c
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
#include "dal_opt.h"

typedef struct {
	struct fs_node *node;
	uint8_t is_used;
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
		fds[fd].node = NULL;
	}
}

static int check_fd(int fd, struct fs_node **node)
{
	if (fd < RESERVED_FDS || fd >= FD_MAX_SIZE || !fds[fd].is_used || !node)
		return DAL_ERR_INVALID;

	*node = fds[fd].node;
	return DAL_ERR_NONE;
}

int dal_open(const char *node_name)
{
	struct fs_node *node = get_node_by_path(node_name);
	if (!node)
		return DAL_ERR_NOT_EXIST;

	int new_fd = alloc_fd();
	if (new_fd == DAL_ERR_OVERFLOW)
		return DAL_ERR_OVERFLOW;

	if (!node->file || !node->file->opts || !node->file->opts->open) {
		free_fd(new_fd);
		return DAL_ERR_EXCEPTION;
	}

	dal_err_e ret = node->file->opts->open(node->file);
	if (ret < 0) {
		free_fd(new_fd);
		return ret;
	}

	fds[new_fd].node = node;
	return new_fd;
}

dal_err_e dal_close(int fd)
{
	struct fs_node *node;
	int err = check_fd(fd, &node);
	if (err != DAL_ERR_NONE)
		return err;

	if (!node->file->opts->close)
		return DAL_ERR_EXCEPTION;

	dal_err_e ret = node->file->opts->close(node->file);
	if (ret < DAL_ERR_NONE)
		return ret;

	free_fd(fd);
	return DAL_ERR_NONE;
}

int dal_read(int fd, uint8_t *buf, size_t len)
{
	struct fs_node *node;
	int err = check_fd(fd, &node);
	if (err != DAL_ERR_NONE)
		return err;

	if (!node->file->opts->read)
		return DAL_ERR_EXCEPTION;

	return node->file->opts->read(node->file, buf, len);
}

int dal_write(int fd, const uint8_t *buf, size_t len)
{
	struct fs_node *node;
	int err = check_fd(fd, &node);
	if (err != DAL_ERR_NONE)
		return err;

	if (!node->file->opts->write)
		return DAL_ERR_EXCEPTION;

	return node->file->opts->write(node->file, buf, len);
}

int dal_ioctrl(int fd, int cmd, void *arg)
{
	struct fs_node *node;
	int err = check_fd(fd, &node);
	if (err != DAL_ERR_NONE)
		return err;

	if (!node->file->opts->ioctrl)
		return DAL_ERR_EXCEPTION;

	return node->file->opts->ioctrl(node->file, cmd, arg);
}

int dal_lseek(int fd, int offset, dal_lseek_whence_e whence)
{
	struct fs_node *node;
	int err = check_fd(fd, &node);
	if (err != DAL_ERR_NONE)
		return err;

	if (!node->file->opts->lseek)
		return DAL_ERR_EXCEPTION;

	return node->file->opts->lseek(node->file, offset, (drv_lseek_whence_e)whence);
}

void dal_init(void)
{
	for (uint16_t i = 0; i < FD_MAX_SIZE; i++) {
		fds[i].is_used = i < RESERVED_FDS ? 1 : 0;
		fds[i].node = NULL;
	}
}