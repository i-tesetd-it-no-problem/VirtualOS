/**
 * @file string_hash.h
 * @author wenshuyu (wsy2161826815@163.com)
 * @brief 字符串哈希组件
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

#ifndef _VIRTUAL_OS_STRING_HASH_H
#define _VIRTUAL_OS_STRING_HASH_H

#include <stdint.h>
#include <stddef.h>
#include "utils/list.h"

enum hash_error {
	HASH_KEY_NOT_FOUND = -2, //键不存在
	HASH_POINT_ERROR,		 //指针错误
	HASH_SUCCESS,			 //无错误
};

struct string_hash_node {
	char *key;
	void *private;
	list_item list;
};

struct hash_table {
	list_item *table;
	size_t table_size;
};

/**
 * @brief 哈希表初始化
 *
 * @param hash_table 一个实例
 * @param table_size 哈希表大小,建议为所有需要哈希处理的键大小的2-3倍
 * @return enum hash_error 错误码
 */
enum hash_error init_hash_table(struct hash_table *hash_table, size_t table_size);

/**
 * @brief 哈希插入
 *
 * @param hash_table 表实例
 * @param key 字符串
 * @param private 需要存储数据的指针
 * @return enum hash_error 错误码
 */
enum hash_error hash_insert(struct hash_table *hash_table, const char *key, void *private);

/**
 * @brief 哈希查找
 *
 * @param hash_table 表实例
 * @param key 字符串
 * @param error 错误码,不考虑设为NULL
 * @return void* 返回存储的指针,自行强转
 */
void *hash_find(struct hash_table *hash_table, const char *key, enum hash_error *error);

/**
 * @brief 删除哈希键
 *
 * @param hash_table 表实例
 * @param key 字符串
 * @return enum hash_error 错误码
 */
enum hash_error hash_delete(struct hash_table *hash_table, const char *key);

/**
 * @brief 获取哈希表中所有的键
 *
 * @param hash_table 表实例
 * @param keys 输出参数，用于存储所有的键
 * @param num_keys 输出参数，用于返回键的数量
 * @return enum hash_error 错误码
 */
enum hash_error hash_get_all_keys(struct hash_table *hash_table, char ***keys, size_t *num_keys);

/**
 * @brief 删除表
 *
 * @param hash_table
 */
void destroy_hash_table(struct hash_table *hash_table);

#endif /* _VIRTUAL_OS_STRING_HASH_H */