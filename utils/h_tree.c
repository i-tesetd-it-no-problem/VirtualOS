/**
 * @file h_tree.c
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

#include <stdlib.h>
#include "utils/h_tree.h"

bool init_tree_node(struct tree_node *node)
{
	if (node == NULL)
		return false;

	*node = (struct tree_node){ 0 };

	return true;
}

struct tree_node *create_tree_node()
{
	struct tree_node *node = (struct tree_node *)calloc(1, sizeof(struct tree_node));
	if (node != NULL) {
		if (!init_tree_node(node)) {
			free(node);
			return NULL;
		}
	}
	return node;
}

/**
 * @brief 递归销毁树节点
 * 
 * @param node 树节点
 */
void destroy_tree_node(struct tree_node *node, visit_func visit)
{
	if (node == NULL)
		return;

	struct tree_node *child = node->first_child;
	while (child != NULL) {
		struct tree_node *next = child->next_sibling;
		destroy_tree_node(child, visit);
		child = next;
	}

	if (visit)
		visit(node);

	free(node);
}

bool add_tree_child(struct tree_node *parent, struct tree_node *child)
{
	if (parent == NULL || child == NULL)
		return false;

	child->parent = parent;

	if (parent->first_child == NULL) {
		parent->first_child = child;
		parent->last_child = child;
	} else {
		parent->last_child->next_sibling = child;
		parent->last_child = child;
	}

	return true;
}

bool remove_tree_child(struct tree_node *parent, struct tree_node *child, visit_func visit)
{
	if (parent == NULL || child == NULL)
		return false;

	struct tree_node *prev = NULL;
	struct tree_node *current = parent->first_child;

	while (current) {
		if (current == child) {
			if (prev)
				prev->next_sibling = current->next_sibling;
			else
				parent->first_child = current->next_sibling;

			if (parent->last_child == child)
				parent->last_child = prev;

			destroy_tree_node(child, visit);

			return true;
		}
		prev = current;
		current = current->next_sibling;
	}

	return false;
}

/* 深度优先遍历 */
void traverse_tree_dfs(struct tree_node *root, visit_func visit)
{
	if (root == NULL || visit == NULL)
		return;

	visit(root);

	struct tree_node *child = root->first_child;
	while (child) {
		traverse_tree_dfs(child, visit);
		child = child->next_sibling;
	}
}

static size_t get_tree_node_count(struct tree_node *root)
{
	if (root == NULL)
		return 0;

	size_t count = 1;

	struct tree_node *child = root->first_child;
	while (child) {
		count += get_tree_node_count(child);
		child = child->next_sibling;
	}

	return count;
}

/* 广度优先遍历 */
void traverse_tree_bfs(struct tree_node *root, visit_func visit)
{
	if (root == NULL || visit == NULL)
		return;

	size_t node_count = get_tree_node_count(root);

	struct tree_node **queue = (struct tree_node **)calloc(node_count, sizeof(struct tree_node *));
	if (queue == NULL)
		return;

	uint16_t front = 0;
	uint16_t rear = 0;

	queue[rear++] = root;

	while (front < rear) {
		struct tree_node *current = queue[front++];

		visit(current);

		struct tree_node *child = current->first_child;
		while (child) {
			queue[rear++] = child;
			child = child->next_sibling;
		}
	}

	free(queue);
}

struct tree_node *get_tree_root(struct tree_node *node)
{
	if (node == NULL)
		return NULL;

	while (node->parent)
		node = node->parent;

	return node;
}
