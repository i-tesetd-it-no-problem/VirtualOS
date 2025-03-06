/**
 * @file list.c
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 双向循环链表组件
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

#include "utils/list.h"

void list_init(list_item *head)
{
	head->next = head;
	head->pre = head;
}

static inline uint8_t is_head_valid(list_item *head)
{
	return (head && head->next && head->pre);
}

static inline uint8_t is_head_empty(list_item *head)
{
	return is_head_valid(head) ? (head->next == head) : 1;
}

static inline void list_insert(list_item *item, list_item *pre, list_item *next)
{
	next->pre = item;
	item->next = next;
	item->pre = pre;
	pre->next = item;
}

static inline void list_delete(list_item *pre, list_item *next)
{
	next->pre = pre;
	pre->next = next;
}

static inline void list_free(list_item *item)
{
	item->next = NULL;
	item->pre = NULL;
}

uint8_t list_delete_item(list_item *item)
{
	if (!item || !item->pre || !item->next)
		return 1;

	list_delete(item->pre, item->next);
	list_free(item);
	return 0;
}

list_item *list_delete_tail(list_item *head)
{
	list_item *del = NULL;

	if (is_head_empty(head))
		return NULL;

	del = head->next;
	list_delete(head->pre, head);
	list_free(del);
	return del;
}

uint8_t list_add_tail(list_item *head, list_item *item)
{
	if (!is_head_valid(head) || !item)
		return 1;

	list_insert(item, head->pre, head);
	return 0;
}
