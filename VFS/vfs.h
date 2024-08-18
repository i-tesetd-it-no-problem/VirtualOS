/**
 * @file vfs.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 虚拟文件系统实现
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

#ifndef _VIRTUAL_OS_VFS_H_
#define _VIRTUAL_OS_VFS_H_

#include "device.h"
#include "h_tree.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

enum fs_node_type {
	FS_FILE,
	FS_DIRECTORY,
};

struct fs_node {
	struct tree_node tree;
	char *name;
	enum fs_node_type node_type;
	drv_file_t *file;
	bool privalage;
};

/**
 * @brief 通过路径获取节点
 * 
 * @param path 路径
 * @return struct fs_node* 
 */
struct fs_node *get_node_by_path(const char *path);

/**
 * @brief 在目录下创建文件
 * 
 * @param dir 所在目录名
 * @param name 文件名
 * @param file_opts 操作指针
 * @param privalage 是否具有最高权限
 * @return true 
 * @return false 
 */
bool create_file_in_dir(struct fs_node *dir, const char *name, drv_file_opts_t *file_opts, bool privalage);
#endif /* _VIRTUAL_OS_VFS_H_ */