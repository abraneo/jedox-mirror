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

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "libpalo2.h"
#include "interface.h"
#include "error.h"
#include "strtools.h"
#include "stdaliases.h"
#include "hash_table.h"

#define  CHUNK_SIZE 1024

static void _libpalo_cache_free_entry(struct libpalo_cache_entry_m *e) {
	size_t i;

	assert(e != NULL);

	if (e->coordinates != NULL) {
		for (i = 0; i < e->num_coordinates; i++) {
			if (e->coordinates[i] != NULL) {
				free(e->coordinates[i]);
			}
		}
		free(e->coordinates);
	}
	if (e->database != NULL) {
		free(e->database);
	}
	if (e->cube != NULL) {
		free(e->cube);
	}
	if (e->key != NULL) {
		free(e->key);
	}
	free_arg_palo_value_contents_m(&e->value);
	free(e);
}


static palo_err _libpalo_cache_create(libpalo_cache **c) {
	assert(c != NULL);

	*c = (libpalo_cache*)malloc(sizeof(libpalo_cache));
	if (*c == NULL) {
		return PALO_ERR_NO_MEM;
	}

	(*c)->num_entries = 0;
	(*c)->entries = NULL;

	hash_table_init(&(*c)->ht);
	if ((*c)->ht == NULL) {
		free(*c);
		*c = NULL;
		return PALO_ERR_NO_MEM;
	}

	return PALO_SUCCESS;
}


void _libpalo_cache_free_entries(libpalo_cache *c) {
	size_t i;

	assert(c != NULL);

	if (c->entries != NULL) {
		for (i = 0; i < c->num_entries; i++) {
			_libpalo_cache_free_entry(c->entries[i]);
		}
		free(c->entries);
	}
}


void libpalo_cache_free(libpalo_cache *c) {
	assert(c != NULL);

	free_hash_table(c->ht);

	_libpalo_cache_free_entries(c);

	free(c);
}


palo_err _libpalo_cache_add_request_m_r(struct errstack *errs, libpalo_cache *c, struct sock_obj *so, struct conversions *convs, const char *key, char *database, char *cube, struct arg_str_array_m *coordinates) {
	struct libpalo_cache_entry_m *e, *old;
	wchar_t *wkey = NULL;
	palo_err result;
	size_t i;
	palo_bool updated;

	assert(c != NULL);
	assert(so != NULL);
	assert(key != NULL);
	assert(database != NULL);
	assert(cube != NULL);
	assert(coordinates != NULL);
	assert(errs != NULL);

	e = (struct libpalo_cache_entry_m*)malloc(sizeof(struct libpalo_cache_entry_m));
	if (e == NULL) {
		ERRSTACK_PREPARE(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
		goto cleanup;
	}

	e->so = so;
	e->err = NULL;
	e->coordinates = coordinates->a;
	e->num_coordinates = coordinates->len;
	e->database = database;
	e->cube = cube;
	memset(&e->value, 0, sizeof(struct arg_palo_value_m));

	result = utf82wcs(convs, &wkey, key);
	if (result != PALO_SUCCESS) {
		ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
		goto cleanup;
	}

	result = hash_table_add_or_update_ex(c->ht, wkey, e, &updated, (void**)&old);
	e->key = wkey;
	if (result != PALO_SUCCESS) {
		ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
		goto cleanup;
	}

	if (updated) {
		/* update */
		for (i = 0; i < c->num_entries; i++) {
			if (c->entries[i] == old) {
				c->entries[i] = e;
				_libpalo_cache_free_entry(old);
				break;
			}
		}
	} else {
		/* add */
		if ((c->num_entries % CHUNK_SIZE) == 0) {
			c->entries = (struct libpalo_cache_entry_m**)realloc((void*)c->entries, (c->num_entries + CHUNK_SIZE) * sizeof(struct libpalo_cache_entry_m*));
			if (c->entries == NULL) {
				ERRSTACK_PREPARE(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
				goto cleanup;
			}
		}

		c->num_entries++;
		c->entries[c->num_entries-1] = e;
	}

	return PALO_SUCCESS;

cleanup:
	if (wkey != NULL) {
		free(wkey);
	}
	if (e != NULL) {
		free(e);
	}

	return result;				 /* errs has been set earlier */
}


palo_err _libpalo_cache_get_result_m_r(struct errstack *errs, libpalo_cache *c, struct conversions *convs, const char *key, struct arg_palo_value_m **const value) {
	wchar_t *wkey;
	struct libpalo_cache_entry_m *e;
	palo_err result;

	assert(value != NULL);
	assert(c != NULL);
	assert(convs != NULL);
	assert(key != NULL);
	assert(errs != NULL);
	assert(value != NULL);

	result = utf82wcs(convs, &wkey, key);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, MSG_ENCODING_FAILED);
	}

	e = (struct libpalo_cache_entry_m*)hash_table_find(c->ht, wkey);
	free(wkey);
	if (e == NULL) {
		ERRSTACK_RETURN(errs, PALO_ERR_CACHE_ENTRY_NOT_FOUND, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"unknown request"));
	}

	*value = &e->value;

	return PALO_SUCCESS;
}


int _compare_libpalo_cache_entries(const void *s1, const void *s2) {
	struct libpalo_cache_entry_m **e1;
	struct libpalo_cache_entry_m **e2;
	int result;
	size_t i;

	e1 = (struct libpalo_cache_entry_m**)s1;
	e2 = (struct libpalo_cache_entry_m**)s2;

	/* connection */
	if ((*e1)->so->socket < (*e2)->so->socket) {
		return -1;
	}
	else {
		if ((*e1)->so->socket > (*e2)->so->socket) {
			return 1;
		}
	}

	/* database */
	result = stricmp((*e1)->database, (*e2)->database);
	if (result != 0) {
		return result;
	}

	/* cube */
	result = stricmp((*e1)->cube, (*e2)->cube);
	if (result != 0) {
		return result;
	}

	/* number of coordinates */
	if ((*e1)->num_coordinates < (*e2)->num_coordinates) {
		return -1;
	}
	else {
		if ((*e1)->num_coordinates > (*e2)->num_coordinates) {
			return 1;
		}
	}

	/* coordiantes */
	for (i = 0; i < (*e1)->num_coordinates; i++) {
		result = stricmp((*e1)->coordinates[i], (*e2)->coordinates[i]);
		if (result != 0) {
			return result;
		}
	}

	return 0;
}


static palo_err _libpalo_cache_remove_dup(struct errstack *errs, libpalo_cache *c) {
	size_t i, j, last_i;
	palo_bool updated;
	struct libpalo_cache_entry_m *old;
	palo_err result;
	void *tmp = NULL;

	assert(c != NULL);
	assert(errs != NULL);

	last_i = 0;
	for (i = 0; i + 1 < c->num_entries; i++) {
		if (_compare_libpalo_cache_entries(c->entries + i, c->entries + i + 1) != 0) {
			for (j = last_i; j < i; j++) {
				/* update ht */
				result = hash_table_add_or_update_ex(c->ht, c->entries[j]->key, (void*)c->entries[i], &updated, (void**) & old);
				if (result != PALO_SUCCESS) {
					ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"failed to update hash table"));
				}
				if (!updated || old != c->entries[j]) {
					ERRSTACK_RETURN(errs, PALO_ERR_CACHE, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"unexpected error updating hash table"));
				}
				_libpalo_cache_free_entry(c->entries[j]);
				memmove(c->entries + j, c->entries + j + 1, (c->num_entries - j - 1)*sizeof(struct libpalo_cache_entry_m*));
				tmp = realloc(c->entries, (--c->num_entries) * sizeof(struct libpalo_cache_entry_m*));
				assert(c->entries != NULL);
				j--;
				i--;
			}
			last_i = i + 1;
		}
	}

	return PALO_SUCCESS;
}


static palo_err _libpalo_cache_sort(struct errstack *errs, libpalo_cache *c) {
	palo_err result;

	assert(c != NULL);
	assert(errs != NULL);

	qsort(c->entries, c->num_entries, sizeof(struct libpalo_cache_entry_m**), &_compare_libpalo_cache_entries);

	/* add optimiziations (recognition of duplicate queries) here */
	result = _libpalo_cache_remove_dup(errs, c);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	return PALO_SUCCESS;
}


palo_err _libpalo_cache_exec_m_r(struct errstack *errs, struct conversions *convs, libpalo_cache *c) {
	size_t i, i_first = 0, j, k, last_num_coordinates = 0;
	char *last_database = const_cast<char *>(""), *last_cube = const_cast<char *>("");
	struct sock_obj *last_so = NULL;
	palo_err result;
	struct arg_palo_value_array_m pva;
	struct arg_str_array_2d_m sa;

	assert(c != NULL);
	assert(errs != NULL);

	if (c->num_entries == 0) {
		return PALO_SUCCESS;
	}

	result = _libpalo_cache_sort(errs, c);
	if (result != PALO_SUCCESS) {
		ERRSTACK_RETURN(errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
	}

	for (i = 0; i <= c->num_entries; i++) {
		if (	(i == c->num_entries)
		        || (last_so != c->entries[i]->so)
		        || (stricmp(last_database, c->entries[i]->database) != 0)
		        || (stricmp(last_cube, c->entries[i]->cube) != 0)
		        || (last_num_coordinates != c->entries[i]->num_coordinates)) {

			if (i - i_first > 0) {
				/* cumulate */

				sa.rows = i - i_first;
				sa.cols = last_num_coordinates;
				sa.a = (char**)calloc(sa.rows * sa.cols, sizeof(char*));
				/* i-i_first > 0 */
				if (sa.a == NULL) {
					ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
				}

				for (j = 0; j < sa.rows; j++) {
					/* add sub-request to query */
					for (k = 0; k < sa.cols; k++) {
						sa.a[j*sa.cols+k] = c->entries[i_first+j]->coordinates[k];
					}
				}

				result = palo__getdata_multi_m_r(errs, &pva, last_so, convs, last_database, last_cube, &sa);

				free(sa.a);
				if (result != PALO_SUCCESS) {
					/* entire query has failed */
					for (j = i_first; j < i; j++) {
						c->entries[j]->value.type = argPaloValueTypeError;
						c->entries[j]->value.val.err.code = result;
					}
				} else {
					if (pva.len != i - i_first) {
						ERRSTACK_RETURN(errs, PALO_ERR_INV_ARG_COUNT, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"server returned too few values"));
					}
					for (j = 0; j < pva.len; j++) {
						c->entries[i_first+j]->value = pva.a[j];
					}
					/* pva.len is > 0; pva.a[i] are stored in c */
					free(pva.a);
				}
			}

			/* cleanup */
			if (i != c->num_entries) {
				last_so = c->entries[i]->so;
				last_database = c->entries[i]->database;
				last_cube = c->entries[i]->cube;
				last_num_coordinates = c->entries[i]->num_coordinates;
				i_first = i;
			}
		}
	}

	return PALO_SUCCESS;
}


LIBEXPORT libpalo_result libpalo_cache_exec_r(libpalo_err *err, libpalo_cache *c, struct conversions *convs) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(_libpalo_cache_exec_m_r(errs, convs, c), err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT libpalo_result libpalo_cache_exec(libpalo_err *err, libpalo_cache *c) {
	return libpalo_cache_exec_r(err, c, &libpalo_convs);
}


LIBEXPORT libpalo_result libpalo_cache_create(libpalo_err *err, libpalo_cache **c) {
	ERRSTACK_DEFINE_ONLY()

	return _to_libpalo_result(_libpalo_cache_create(c), err, errs, __FUNCTION__, __FILE__, __LINE__);
}
