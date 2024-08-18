/**
 * @file vfs.c
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

#include "vfs.h"
#include "simple_shell.h"

extern int shell_write(uint8_t *buf, size_t len);

struct virtual_fs {
	struct fs_node *root;
	struct fs_node *current_dir;
};

static struct virtual_fs vfs;

static bool vfs_valid(void)
{
	return vfs.root && vfs.current_dir;
}

/**
 * @brief 创建节点
 * 
 * @param name 节点名
 * @param type 节点类型
 * @param file_opts 文件选项
 * @param privalage 是否具有最高权限
 * @return struct fs_node* 
 */
static struct fs_node *create_fs_node(const char *name, enum fs_node_type type, drv_file_opts_t *file_opts, bool privalage)
{
	struct fs_node *node = (struct fs_node *)malloc(sizeof(struct fs_node));
	if (!node)
		return NULL;

	init_tree_node(&(node->tree));
	node->node_type = type;
	node->privalage = privalage;
	node->file = NULL;

	if (type == FS_FILE && file_opts) {
		node->file = (drv_file_t *)malloc(sizeof(drv_file_t));
		if (!node->file) {
			free(node);
			return NULL;
		}
		node->file->dev_size = 0;
		node->file->is_opened = false;
		node->file->opts = file_opts;
		node->file->private_data = NULL;
		node->file->seek_pos = 0;
	}

	size_t name_len = strlen(name) + 1;
	node->name = (char *)malloc(name_len);
	if (!node->name) {
		free(node->file);
		free(node);
		return NULL;
	}
	memcpy(node->name, name, name_len);

	return node;
}

static void destroy_fs_node_data(struct tree_node *node)
{
	/* 销毁节点数据 */
	struct fs_node *fs_node = (struct fs_node *)node;
	if (fs_node->name)
		free(fs_node->name);
	if (fs_node->file)
		free(fs_node->file);
}

/**
 * @brief 销毁节点
 * 
 * @param node 
 */
void destroy_fs_node(struct fs_node *node)
{
	if (!node || node->privalage)
		return;

	destroy_tree_node(&(node->tree), destroy_fs_node_data);
}

/**
 * @brief 在目录下创建目录
 * 
 * @param dir 所在目录名
 * @param name 目录名
 * @param file_opts 操作指针
 * @param privalage 是否具有最高权限
 * @return true 
 * @return false 
 */
static bool create_directory_in_dir(struct fs_node *dir, const char *name, drv_file_opts_t *file_opts, bool privalage)
{
	if (!dir || dir->node_type != FS_DIRECTORY || !name)
		return false;

	struct tree_node *child = dir->tree.first_child;
	while (child) {
		if (strcmp(((struct fs_node *)child)->name, name) == 0)
			return false;
		child = child->next_sibling;
	}

	struct fs_node *dir_node = create_fs_node(name, FS_DIRECTORY, file_opts, privalage);
	if (!dir_node)
		return false;

	struct fs_node *dot_node = create_fs_node(".", FS_DIRECTORY, NULL, false);
	struct fs_node *dotdot_node = create_fs_node("..", FS_DIRECTORY, NULL, false);

	if (!dot_node || !dotdot_node) {
		destroy_fs_node(dir_node);
		free(dot_node);
		free(dotdot_node);
		return false;
	}

	add_tree_child(&(dir_node->tree), &(dot_node->tree));
	add_tree_child(&(dir_node->tree), &(dotdot_node->tree));
	add_tree_child(&(dir->tree), &(dir_node->tree));

	return true;
}

/**
 * @brief 切换目录
 * 
 * @param path 目录路径，可以是绝对路径或相对路径
 * @return true 成功切换
 * @return false 切换失败
 */
static bool change_directory(const char *path)
{
	if (!path || !*path || !vfs_valid())
		return false;

	struct fs_node *target_dir = (path[0] == '/') ? vfs.root : vfs.current_dir;
	const char *current_pos = path;

	while (true) {
		const char *next_slash = strchr(current_pos, '/');
		size_t len = next_slash ? (size_t)(next_slash - current_pos) : strlen(current_pos);

		if (len == 0)
			break;

		char temp[256];
		if (len >= sizeof(temp))
			return false;

		strncpy(temp, current_pos, len);
		temp[len] = '\0';

		if (strcmp(temp, ".") == 0) {
			;
		} else if (strcmp(temp, "..") == 0) {
			if (target_dir->tree.parent)
				target_dir = (struct fs_node *)target_dir->tree.parent;
			else
				return false;
		} else {
			bool found = false;
			struct tree_node *child = target_dir->tree.first_child;
			while (child) {
				struct fs_node *fs_child = (struct fs_node *)child;
				if (strcmp(fs_child->name, temp) == 0 && fs_child->node_type == FS_DIRECTORY) {
					target_dir = fs_child;
					found = true;
					break;
				}
				child = child->next_sibling;
			}
			if (!found)
				return false;
		}

		if (!next_slash)
			break;

		current_pos = next_slash + 1;
	}

	vfs.current_dir = target_dir;
	return true;
}

/**
 * @brief 获取当前路径
 * 
 * @param buffer 缓冲
 * @param size 缓冲大小
 */
static void get_current_path(char *buffer, size_t size)
{
	if (!buffer || size == 0 || !vfs_valid())
		return;

	buffer[0] = '\0';
	struct fs_node *node = vfs.current_dir;
	char temp_buffer[256] = { 0 };
	char *temp_ptr = temp_buffer + sizeof(temp_buffer) - 1;

	*temp_ptr = '\0';

	while (node && node->name && size > 1) {
		size_t len = strlen(node->name);

		if (len + 1 >= (size_t)(temp_ptr - temp_buffer))
			break;

		temp_ptr -= len;
		memcpy(temp_ptr, node->name, len);

		node = (struct fs_node *)node->tree.parent;

		if (node && node->name && node->tree.parent)
			*(--temp_ptr) = '/';
	}

	if (*temp_ptr != '/')
		*(--temp_ptr) = '/';

	strncpy(buffer, temp_ptr, size - 1);
	buffer[size - 1] = '\0';
}

/**
 * @brief 打印当前树节点结构
 * 
 * @param node 
 */
static void print_fs_structure(struct fs_node *node)
{
	if (!node)
		return;

	static uint8_t depth = 0;

	for (int i = 0; i < depth; i++)
		shell_write((uint8_t *)"    ", 4);

	if (depth > 0)
		shell_write((uint8_t *)"|__", 3);

	shell_write((uint8_t *)node->name, strlen(node->name));
	shell_write((uint8_t *)"\n", 1);

	struct tree_node *child = node->tree.first_child;
	while (child) {
		depth++;
		print_fs_structure((struct fs_node *)child);
		depth--;
		child = child->next_sibling;
	}
}

/**
 * @brief 打印当前目录下的所有文件和子目录
 * 
 * @param dir 当前目录节点
 */
static void print_current_directory(struct fs_node *dir)
{
	if (!dir || dir->node_type != FS_DIRECTORY)
		return;

	struct tree_node *child = dir->tree.first_child;
	char buffer[512];
	size_t offset = 0;

	while (child) {
		struct fs_node *fs_child = (struct fs_node *)child;

		size_t name_len = strlen(fs_child->name);
		if (offset + name_len < sizeof(buffer)) {
			memcpy(buffer + offset, fs_child->name, name_len);
			offset += name_len;
		}

		if (fs_child->node_type == FS_DIRECTORY && offset + 1 < sizeof(buffer)) {
			buffer[offset] = '/';
			offset++;
		}

		if (offset + 1 < sizeof(buffer)) {
			buffer[offset] = '\n';
			offset++;
		}

		child = child->next_sibling;
	}

	buffer[offset] = '\0';

	shell_write((uint8_t *)buffer, offset);
}

static void show_fs_structure(void)
{
	if (vfs_valid())
		print_fs_structure(vfs.root);
}

/***************************************************************************************************/

/**
 * @brief 通过路径获取节点
 * 
 * @param path 路径
 * @return struct fs_node* 
 */
struct fs_node *get_node_by_path(const char *path)
{
	if (!path || !*path || !vfs_valid())
		return NULL;

	struct fs_node *current = (path[0] == '/') ? vfs.root : vfs.current_dir;

	char *path_copy = strdup(path);
	if (!path_copy)
		return NULL;

	char *token = strtok(path_copy, "/");
	while (token) {
		if (strcmp(token, "..") == 0) {
			if (current->tree.parent)
				current = (struct fs_node *)current->tree.parent;
		} else {
			struct tree_node *child = current->tree.first_child;
			current = NULL;
			while (child) {
				struct fs_node *fs_child = (struct fs_node *)child;
				if (strcmp(fs_child->name, token) == 0) {
					current = fs_child;
					break;
				}
				child = child->next_sibling;
			}
			if (!current) {
				free(path_copy);
				return NULL;
			}
		}
		token = strtok(NULL, "/");
	}

	free(path_copy);
	return current;
}

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
bool create_file_in_dir(struct fs_node *dir, const char *name, drv_file_opts_t *file_opts, bool privalage)
{
	if (!dir || dir->node_type != FS_DIRECTORY || !name)
		return false;

	struct tree_node *child = dir->tree.first_child;
	while (child) {
		struct fs_node *fs_child = (struct fs_node *)child;
		if (strcmp(fs_child->name, name) == 0)
			return false;

		child = child->next_sibling;
	}

	struct fs_node *file_node = create_fs_node(name, FS_FILE, file_opts, privalage);
	if (!file_node)
		return false;

	add_tree_child(&(dir->tree), &(file_node->tree));
	return true;
}

/**
 * @brief 初始化虚拟文件系统
 * 
 */
void vfs_init(void);
void vfs_init(void)
{
	vfs.root = create_fs_node("/", FS_DIRECTORY, NULL, true);
	vfs.current_dir = vfs.root;

	create_directory_in_dir(vfs.root, "dev", NULL, true); /* 禁止删除 */
	create_directory_in_dir(vfs.root, "sys", NULL, true); /* 禁止删除 */
}

// /***************************************************************************************************/

//
// 虚拟文件系统命令
/********************************tree********************************/
static void tree(int argc, char *argv[])
{
	if (argc == 1)
		show_fs_structure();
}
SPS_EXPORT_CMD(tree, tree, "Show file system structure")
/********************************tree********************************/

/********************************ls********************************/
static void ls(int argc, char *argv[])
{
	if (argc == 1 && vfs_valid())
		print_current_directory(vfs.current_dir);
}
SPS_EXPORT_CMD(ls, ls, "Show current directory contents")
/********************************ls********************************/

/********************************cd********************************/
static void cd(int argc, char *argv[])
{
	char path[256] = { 0 };

	if (argc == 1) {
		strcpy(path, "/");
	} else if (argc == 2) {
		strcpy(path, argv[1]);
	} else {
		strcpy(path, argv[1]);
		strcat(path, "/");
		strcat(path, argv[2]);
	}

	char buffer[256];
	size_t offset = 0;

	if (change_directory(path)) {
		const char *msg = "Directory changed to ";
		size_t msg_len = strlen(msg);
		memcpy(buffer + offset, msg, msg_len);
		offset += msg_len;
	} else {
		const char *msg = "Directory not found: ";
		size_t msg_len = strlen(msg);
		memcpy(buffer + offset, msg, msg_len);
		offset += msg_len;
	}

	size_t path_len = strlen(path);
	memcpy(buffer + offset, path, path_len);
	offset += path_len;

	buffer[offset] = '\n';
	offset++;

	shell_write((uint8_t *)buffer, offset);
}

SPS_EXPORT_CMD(cd, cd, "Change current directory")
/********************************cd********************************/

/********************************mkdir********************************/
// static void mkdir(int argc, char *argv[])
// {
// 	if (argc < 2) {
// 		shell_write((uint8_t *)"Usage: mkdir <directory name>\n", 31);
// 		return;
// 	}

// 	if(!vfs_valid())
// 		return;

// 	char path[256] = { 0 };
// 	if (argc == 2) {
// 		strcpy(path, argv[1]);
// 	} else {
// 		strcpy(path, argv[1]);
// 		strcat(path, "/");
// 		strcat(path, argv[2]);
// 	}

// 	char buffer[256];
// 	size_t offset = 0;

// 	if (create_directory_in_dir(vfs.current_dir, path, NULL, false)) {
// 		const char *msg = "Directory created: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;
// 	} else {
// 		const char *msg = "Directory already exists: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;
// 	}

// 	size_t path_len = strlen(path);
// 	memcpy(buffer + offset, path, path_len);
// 	offset += path_len;

// 	buffer[offset] = '\n';
// 	offset++;

// 	shell_write((uint8_t *)buffer, offset);
// }

// SPS_EXPORT_CMD(mkdir, mkdir, "Create a new directory")
/********************************mkdir********************************/

/********************************touch********************************/
// static void touch(int argc, char *argv[])
// {
// 	if (argc != 2) {
// 		shell_write((uint8_t *)"Usage: touch <file name>\n", 26);
// 		return;
// 	}

// 	char path[256] = { 0 };
// 	strcpy(path, argv[1]);

// 	char buffer[256];
// 	size_t offset = 0;

// 	if (create_file_in_dir(vfs.current_dir, path, NULL, false)) {
// 		const char *msg = "File created: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;
// 	} else {
// 		const char *msg = "File already exists: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;
// 	}

// 	size_t path_len = strlen(path);
// 	memcpy(buffer + offset, path, path_len);
// 	offset += path_len;

// 	buffer[offset] = '\n';
// 	offset++;

// 	shell_write((uint8_t *)buffer, offset);
// }

// SPS_EXPORT_CMD(touch, touch, "Create a new file (cannot create directories recursively)") /* 不能递归创建,只能在当前目录下创建文件 */
/********************************touch********************************/

/********************************rm********************************/
// static void rm(int argc, char *argv[])
// {
// 	if (argc != 2) {
// 		shell_write((uint8_t *)"Usage: rm <file or directory name>\n", 36);
// 		return;
// 	}

// 	char path[256] = { 0 };
// 	strcpy(path, argv[1]);

// 	char buffer[512];
// 	size_t offset = 0;

// 	struct fs_node *node = get_node_by_path(path);
// 	if (!node) {
// 		const char *msg = "File or directory not found: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;

// 		size_t path_len = strlen(path);
// 		memcpy(buffer + offset, path, path_len);
// 		offset += path_len;

// 		buffer[offset] = '\n';
// 		offset++;

// 		shell_write((uint8_t *)buffer, offset);
// 		return;
// 	}

// 	if (node->node_type == FS_FILE && node->privalage) {
// 		const char *msg = "Cannot remove protected file: ";
// 		size_t msg_len = strlen(msg);
// 		memcpy(buffer + offset, msg, msg_len);
// 		offset += msg_len;

// 		size_t path_len = strlen(path);
// 		memcpy(buffer + offset, path, path_len);
// 		offset += path_len;

// 		buffer[offset] = '\n';
// 		offset++;

// 		shell_write((uint8_t *)buffer, offset);
// 		return;
// 	}

// 	if (node->node_type == FS_DIRECTORY) {
// 		struct tree_node *child = node->tree.first_child;
// 		bool is_empty = true;
// 		while (child) {
// 			struct fs_node *fs_child = (struct fs_node *)child;
// 			if (strcmp(fs_child->name, ".") != 0 && strcmp(fs_child->name, "..") != 0) {
// 				is_empty = false;
// 				break;
// 			}
// 			child = child->next_sibling;
// 		}

// 		if (!is_empty) {
// 			const char *msg = "Directory is not empty: ";
// 			size_t msg_len = strlen(msg);
// 			memcpy(buffer + offset, msg, msg_len);
// 			offset += msg_len;

// 			size_t path_len = strlen(path);
// 			memcpy(buffer + offset, path, path_len);
// 			offset += path_len;

// 			buffer[offset] = '\n';
// 			offset++;

// 			shell_write((uint8_t *)buffer, offset);
// 			return;
// 		}
// 	}

// 	if (node->tree.parent) {
// 		if (remove_tree_child(node->tree.parent, &(node->tree), destroy_fs_node_data)) {
// 			const char *msg = "File or directory removed: ";
// 			size_t msg_len = strlen(msg);
// 			memcpy(buffer + offset, msg, msg_len);
// 			offset += msg_len;

// 			size_t path_len = strlen(path);
// 			memcpy(buffer + offset, path, path_len);
// 			offset += path_len;

// 			buffer[offset] = '\n';
// 			offset++;

// 			shell_write((uint8_t *)buffer, offset);
// 		}
// 	}
// }

// SPS_EXPORT_CMD(rm, rm, "Remove a file or directory")
/********************************rm********************************/

/********************************pwd********************************/
static void pwd(int argc, char *argv[])
{
	char path[256] = { 0 };
	get_current_path(path, sizeof(path));
	shell_write((uint8_t *)path, strlen(path));
	shell_write((uint8_t *)"\n", 1);
}
SPS_EXPORT_CMD(pwd, pwd, "Show current directory path")
/********************************pwd********************************/
