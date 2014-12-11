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

#include <stdlib.h>
#include <string.h>

#include "libpalo2.h"
#include "strtools.h"

struct conversions libpalo_convs;

LIBEXPORT void libpalo_free(void *p) {
	free(p);
}


LIBEXPORT libpalo_result libpalo_init_r(libpalo_err *err, struct conversions *convs) {
	palo_err result;
	ERRSTACK_DEFINE_ONLY()

	result = init_conversions(convs);
	if (result != PALO_SUCCESS) {
		ERRSTACK_PREPARE(errs, result, __FUNCTION__, __LINE__, __FILE__, wcsdup(L"init_conversions() failed"));
	}

	return _to_libpalo_result(result, err, errs, __FUNCTION__, __FILE__, __LINE__);
}


LIBEXPORT void libpalo_cleanup_r(struct conversions *convs) {
	free_conversions(convs);
}


LIBEXPORT libpalo_result libpalo_init(libpalo_err *err) {
	return libpalo_init_r(err, &libpalo_convs);
}


LIBEXPORT void libpalo_cleanup() {
	libpalo_cleanup_r(&libpalo_convs);
}


LIBEXPORT void libpalo_err_free_contents(libpalo_err *err) {
	errstack_free(&err->errs);
}
