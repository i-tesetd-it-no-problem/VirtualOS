/**
 * @file h_tree.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 
 * @version 1.0
 * @date 2024-08-20
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

#ifndef __VIRTUALOS_H_TREE_H__
#define __VIRTUALOS_H_TREE_H__

#include <stdbool.h>
#include <stdint.h>

struct tree_node {
	struct tree_node *parent; // 父节点
	struct tree_node *first_child; // 子节点
	struct tree_node *next_sibling; // 兄弟节点
	struct tree_node *last_child; // 最后一个子节点
};

typedef void (*visit_func)(struct tree_node *node); /* 访问函数 */

/**
 * @brief 初始化树节点
 * 
 * @param node 树节点
 * @return bool 返回 true 表示初始化成功，false 表示失败
 */
bool init_tree_node(struct tree_node *node);

/**
 * @brief 创建新节点
 * 
 * @return struct tree_node* 返回节点指针，失败返回 NULL
 */
struct tree_node *create_tree_node(void);

/**
 * @brief 添加 child 到 parent 的子节点列表中
 * 
 * @param parent 父节点
 * @param child 子节点
 * @return bool 
 */
bool add_tree_child(struct tree_node *parent, struct tree_node *child);

/**
 * @brief 递归销毁树节点
 * 
 * @param node 树节点
 */
void destroy_tree_node(struct tree_node *node, visit_func visit);

/**
 * @brief 删除 parent 节点的 child 子节点
 * 
 * @param parent 父节点
 * @param child 子节点
 * @param visit 访问函数(释放资源等)
 * @return bool 
 */
bool remove_tree_child(struct tree_node *parent, struct tree_node *child, visit_func visit);

/**
 * @brief 遍历树，深度优先遍历
 * 
 * @param root 根节点
 * @param visit 访问函数(打印节点等)
 */
void traverse_tree_dfs(struct tree_node *root, visit_func visit);

/**
 * @brief 遍历树，广度优先遍历
 * 
 * @param root 根节点
 * @param visit 访问函数(打印节点等)
 */
void traverse_tree_bfs(struct tree_node *root, visit_func visit);

/**
 * @brief 获取树的根节点
 * 
 * @param node 树节点
 * @return 
 */
struct tree_node *get_tree_root(struct tree_node *node);

#endif /* __VIRTUALOS_H_TREE_H__ */