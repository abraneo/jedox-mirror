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

#ifndef HAS_HASH_TABLE_H

#define HASH_TABLE_SIZE 128

#include <stddef.h>

#include "error.h"
#include "const.h"

struct hash_table_sub_list {
	struct hash_table_sub_list *prev;
	struct hash_table_sub_list *next;
	wchar_t *key;
	void *data;
};

typedef struct hash_table {
	size_t rows;
	struct hash_table_sub_list **table;
}


hash_table;

palo_err hash_table_add_or_update(hash_table *ht, const wchar_t *key, void *data);
palo_err hash_table_add_or_update_ex(hash_table *ht, const wchar_t *key, void *data, palo_bool *updated, void **old_data);
void hash_table_remove(hash_table *ht, const wchar_t *key);
void *hash_table_find(const hash_table *ht, const wchar_t *key);
void free_hash_table(hash_table *ht);
void hash_table_init(hash_table **ht);

#define HAS_HASH_TABLE_H
#endif
