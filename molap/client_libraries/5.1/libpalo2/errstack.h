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

#ifndef HAS_ERRSTACK_H
#define HAS_ERRSTACK_H

#include "error.h"

#define ERRSTACK_MAX_DEPTH              16

struct errstack_entry {
	int line;
	const char *file;
	const char *func;
	wchar_t *description;
};

struct errstack {
	struct errstack_entry stack[ERRSTACK_MAX_DEPTH];
	size_t depth;
};

palo_err _errstack_return(struct errstack *errs, palo_err result, const char *function, int line, const char *file, wchar_t *description_format, ...);

#define ERRSTACK_DECLARE_EX(name) struct errstack name##_temp, *name=&name##_temp;
#define ERRSTACK_INIT_EX(name) memset((name), 0, sizeof(struct errstack));
#define ERRSTACK_DEFINE_ONLY() ERRSTACK_DECLARE_EX(errs) ERRSTACK_INIT_EX(errs)
#define ERRSTACK_DEFINE()	ERRSTACK_DEFINE_ONLY() \
							palo_err res; \
							if (so == NULL) { \
									res = _errstack_return(errs, PALO_ERR_INVALID_POINTER, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"Sockobject is null\n")); \
									return _to_libpalo_result(res, err, errs, __FUNCTION__, __FILE__, __LINE__); \
							} else { \
								if (so->myServer == NULL) { \
									res = _errstack_return(errs, PALO_ERR_INVALID_POINTER, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"Server of Sockobject is null\n")); \
									return _to_libpalo_result(res, err, errs, __FUNCTION__, __FILE__, __LINE__); \
								} \
							}
#define ERRSTACK_RETURN return _errstack_return
#define ERRSTACK_SET { palo_err errstack_temp_result=_errstack_return
#define ERRSTACK_DONE_NORETURN(errs) errstack_free((errs)); }
#define ERRSTACK_DONE() return errstack_temp_result; }
#define ERRSTACK_PREPARE result=_errstack_return

void errstack_free(struct errstack *errs);
wchar_t *errstack_errmsg_long(const struct errstack *errs, const palo_err result);
wchar_t *errstack_errmsg_short(const struct errstack *errs, const palo_err result);
#endif
