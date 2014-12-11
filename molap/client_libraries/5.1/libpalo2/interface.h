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

#ifndef HAS_INTERFACE_H
#define HAS_INTERFACE_H

#include <wchar.h>

#include "libpalo2.h"
#include "arg_types.h"
#include "hash_table.h"

struct libpalo_cache_entry_m {
	char *err;
	struct sock_obj *so;
	char *database;
	char *cube;

	size_t num_coordinates;

	char **coordinates;
	wchar_t *key;				 /* hash table key */
	/* return value */
	struct arg_palo_value_m value;
};

typedef struct libpalo_cache {
	size_t num_entries;
	struct libpalo_cache_entry_m **entries;
	hash_table *ht;
}


libpalo_cache;

#ifndef LIBPALO

/** @ingroup gen_funcs
 *  @brief
 *  Specifies the type of a dimension element.
 */
enum libpalo_de_type
{
    /// Numeric (base) element
    libpalo_dontuse_de_type_n,
    /// Consolidated element
    libpalo_dontuse_de_type_c,
    /// String (base) element
    libpalo_dontuse_de_type_s,
    /// Rule (not implemented yet)
    libpalo_dontuse_de_type_r
};

/** @ingroup gen_funcs
 *  @brief
 *  Specifies the operation mode of eadd_or_update().
 */
enum libpalo_eadd_or_update_mode
{
    /// Add element (do nothing if it exists)
    libpalo_dontuse_deadd_or_update_mode_add,
    /// Add element (throw error if it exists)
    libpalo_dontuse_eadd_or_update_mode_force_add,
    /// Update element
    libpalo_dontuse_eadd_or_update_mode_update,
    /// Add or update element
    libpalo_dontuse_eadd_or_update_mode_add_or_update
};
#endif

LIBEXPORT libpalo_result libpalo_cache_create(libpalo_err *err, libpalo_cache **c);
void libpalo_cache_free(libpalo_cache *c);

LIBEXPORT libpalo_result libpalo_cache_exec_r(libpalo_err *err, libpalo_cache *c, struct conversions *convs);
LIBEXPORT libpalo_result libpalo_cache_exec(libpalo_err *err, libpalo_cache *c);

palo_err _libpalo_cache_add_request_m_r(struct errstack *errs, libpalo_cache *c, struct sock_obj *so, struct conversions *convs, const char *key, char *database, char *cube, struct arg_str_array_m *coordinates);
palo_err _libpalo_cache_get_result_m_r(struct errstack *errs, libpalo_cache *c, struct conversions *convs, const char *key, struct arg_palo_value_m **const value);

libpalo_result _to_libpalo_result(palo_err result, libpalo_err *err, struct errstack *errs, const char *func, const char *file, int line);

#include "interface_generic.h"
#endif
