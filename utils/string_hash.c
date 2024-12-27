/**
 * @file string_hash.c
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

#include <stdlib.h>
#include <string.h>
#include "utils/string_hash.h"

// FNV-1a hash
static uint32_t hash(const char *key, size_t table_size)
{
	uint32_t hash = 2166136261u;

	while (*key) {
		hash ^= (uint8_t)*key++;
		hash *= 16777619;
	}

	return hash % table_size;
}

enum hash_error init_hash_table(struct hash_table *hash_table, size_t table_size)
{
	if (!hash_table)
		return HASH_POINT_ERROR;

	hash_table->table = (list_item *)calloc(table_size, sizeof(list_item));

	if (!hash_table->table)
		return HASH_POINT_ERROR;

	for (size_t i = 0; i < table_size; i++)
		list_init(&hash_table->table[i]);

	hash_table->table_size = table_size;
	return HASH_SUCCESS;
}

enum hash_error hash_insert(struct hash_table *hash_table, const char *key, void *private)
{
	if (!hash_table || !key)
		return HASH_POINT_ERROR;

	uint32_t index = hash(key, hash_table->table_size);

	list_item *pos, *tmp;
	list_for_each_safe(pos, tmp, &hash_table->table[index])
	{
		struct string_hash_node *node = container_of(pos, struct string_hash_node, list);

		if (strcmp(node->key, key) == 0) {
			node->private = private;

			return HASH_SUCCESS;
		}
	}

	struct string_hash_node *new_node = (struct string_hash_node *)calloc(1, sizeof(struct string_hash_node));
	list_init(&new_node->list);

	if (!new_node)
		return HASH_POINT_ERROR;

	new_node->key = (char *)calloc(1, strlen(key) + 1);
	if (!new_node->key) {
		free(new_node);

		return HASH_POINT_ERROR;
	}
	strcpy(new_node->key, key);

	new_node->private = private;
	list_add_tail(&hash_table->table[index], &new_node->list);

	return HASH_SUCCESS;
}

void *hash_find(struct hash_table *hash_table, const char *key, enum hash_error *error)
{
	if (!hash_table || !key) {
		if (error)
			*error = HASH_POINT_ERROR;

		return NULL;
	}

	uint32_t index = hash(key, hash_table->table_size);

	list_item *pos, *tmp;
	list_for_each_safe(pos, tmp, &hash_table->table[index])
	{
		struct string_hash_node *node = container_of(pos, struct string_hash_node, list);

		if (strcmp(node->key, key) == 0) {
			if (error)
				*error = HASH_SUCCESS;

			return node->private;
		}
	}

	if (error)
		*error = HASH_KEY_NOT_FOUND;

	return NULL;
}

enum hash_error hash_delete(struct hash_table *hash_table, const char *key)
{
	if (!hash_table || !key)
		return HASH_POINT_ERROR;

	uint32_t index = hash(key, hash_table->table_size);

	list_item *pos, *tmp;
	list_for_each_safe(pos, tmp, &hash_table->table[index])
	{
		struct string_hash_node *node = container_of(pos, struct string_hash_node, list);

		if (strcmp(node->key, key) == 0) {
			list_delete_item(pos);
			free(node->key);
			free(node);

			return HASH_SUCCESS;
		}
	}

	return HASH_KEY_NOT_FOUND;
}

enum hash_error hash_get_all_keys(struct hash_table *hash_table, char ***keys, size_t *num_keys)
{
	if (!hash_table || !keys || !num_keys)
		return HASH_POINT_ERROR;

	size_t total_keys = 0;
	for (size_t i = 0; i < hash_table->table_size; ++i) {
		list_item *pos, *n;
		list_for_each_safe(pos, n, &hash_table->table[i]) total_keys++;
	}

	if (total_keys == 0) {
		*keys = NULL;
		*num_keys = 0;
		return HASH_SUCCESS;
	}

	char **key_array = (char **)calloc(total_keys, sizeof(char *));
	if (!key_array)
		return HASH_POINT_ERROR;

	size_t index = 0;
	for (size_t i = 0; i < hash_table->table_size; ++i) {
		list_item *pos, *n;
		list_for_each_safe(pos, n, &hash_table->table[i])
		{
			struct string_hash_node *node = container_of(pos, struct string_hash_node, list);

			size_t key_len = strlen(node->key) + 1;
			key_array[index] = (char *)calloc(1, key_len);
			if (!key_array[index]) {
				for (size_t j = 0; j < index; ++j)
					free(key_array[j]);

				free(key_array);
				return HASH_POINT_ERROR;
			}

			strcpy(key_array[index], node->key);
			index++;
		}
	}

	*keys = key_array;
	*num_keys = total_keys;
	return HASH_SUCCESS;
}

void destroy_hash_table(struct hash_table *hash_table)
{
	if (!hash_table)
		return;

	for (size_t i = 0; i < hash_table->table_size; ++i) {
		list_item *pos, *tmp;
		list_for_each_safe(pos, tmp, &hash_table->table[i])
		{
			struct string_hash_node *node = container_of(pos, struct string_hash_node, list);
			list_delete_item(pos);
			free(node->key);
			free(node);
		}
	}

	free(hash_table->table);
}
