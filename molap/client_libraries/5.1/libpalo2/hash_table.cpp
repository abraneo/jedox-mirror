////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wchar.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "const.h"
#include "stdaliases.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "hash_table.h"

	size_t key2hash(const wchar_t *key, size_t length) {
		size_t i;
		size_t hash = 0;
		for (i = 0;i < wcslen(key);i++) {
			hash += key[i];
		}
		return hash % length;
	}

	palo_err hash_table_add_or_update(hash_table *ht, const wchar_t *key, void *data) {
		palo_bool updated;
		void *old;

		return hash_table_add_or_update_ex(ht, key, data, &updated, &old);
	}

	palo_err hash_table_add_or_update_ex(hash_table *ht, const wchar_t *key, void *data, palo_bool *updated, void **old_data) {
		struct hash_table_sub_list *p;
		size_t hash = key2hash(key, ht->rows);

		assert(ht != NULL);
		assert(key != NULL);
		assert(updated != NULL);
		assert(old_data != NULL);

		*updated = PALO_FALSE;

		if (ht->table[hash] == NULL) {
			ht->table[hash] = (struct hash_table_sub_list*)malloc(sizeof(struct hash_table_sub_list));
			if (ht->table[hash] == NULL)
				return PALO_ERR_NO_MEM;
			ht->table[hash]->next = NULL;
			ht->table[hash]->prev = NULL;
			ht->table[hash]->data = data;
			ht->table[hash]->key = wcsdup(key);
			if (ht->table[hash]->key == NULL) {
				free(ht->table[hash]);
				ht->table[hash] = NULL;
				return PALO_ERR_NO_MEM;
			}
		} else {
			p = ht->table[hash];
			while (p->next != NULL && wcscmp(p->key, key) != 0)
				p = p->next;
			if (wcscmp(p->key, key) == 0) {
				/* update */
				*old_data = p->data;
				p->data = data;
				*updated = PALO_TRUE;
			} else {
				/* add */
				p->next = (struct hash_table_sub_list*)malloc(sizeof(struct hash_table_sub_list));
				if (p->next == NULL)
					return PALO_ERR_NO_MEM;
				p->next->key = wcsdup(key);
				if (p->next->key == NULL) {
					free(p->next);
					p->next = NULL;
					return PALO_ERR_NO_MEM;
				}
				p->next->next = NULL;
				p->next->prev = p;
				p->next->data = data;
			}
		}
		return PALO_SUCCESS;
	}

	void hash_table_remove(hash_table *ht, const wchar_t *key) {
		size_t hash = key2hash(key, ht->rows);
		struct hash_table_sub_list *p;
		assert(ht != NULL);
		assert(ht->table != NULL);
		assert(key != NULL);
		if (ht->table[hash] == NULL)
			return;
		p = ht->table[hash];
		do {
			if (wcscmp(p->key, key) == 0) {
				if (p == ht->table[hash])
					ht->table[hash] = p->next;
				if (p->prev != NULL)
					p->prev->next = p->next;
				if (p->next != NULL)
					p->next->prev = p->prev;
				assert(p->key != NULL);
				free(p->key);
				free(p);
				return;
			}
			p = p->next;
		} while (p != NULL);
		return;
	}

	void *hash_table_find(const hash_table *ht, const wchar_t *key) {
		size_t hash = key2hash(key, ht->rows);
		struct hash_table_sub_list *p;
		assert(ht != NULL);
		assert(ht->table != NULL);
		assert(key != NULL);
		if (ht->table[hash] == NULL)
			return NULL;
		p = ht->table[hash];
		while (p != NULL) {
			if (wcscmp(p->key, key) == 0)
				return p->data;
			p = p->next;
		}
		return NULL;
	}

	void free_hash_table(hash_table *ht) {
		size_t i;
		struct hash_table_sub_list *p, *q;
		assert(ht != NULL);
		assert(ht->table != NULL);
		for (i = 0;i < ht->rows;i++) {
			p = ht->table[i];
			while (p != NULL) {
				q = p->next;
				free(p->key);
				free(p);
				p = q;
			}
		}
		free(ht->table);
		free(ht);
	}

	void hash_table_init(hash_table **ht) {
		assert(ht != NULL);
		*ht = (hash_table*)malloc(sizeof(hash_table));
		if (*ht == NULL)
			return;
		(*ht)->rows = HASH_TABLE_SIZE;
		(*ht)->table = (struct hash_table_sub_list**)calloc((*ht)->rows, sizeof(struct hash_table_sub_list*));
		if ((*ht)->table == NULL) {
			free(*ht);
			*ht = NULL;
			return;
		}
	}
#ifdef __cplusplus
}
#endif
