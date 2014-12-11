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

#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "const.h"
#include "strtools.h"
#include "stdaliases.h"
#include "error.h"

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

	palo_err realloc_free(void **p, size_t new_size) {
		palo_err result;

		result = realloc_safe(p, new_size);
		if (result != PALO_SUCCESS) {
			free(*p);
			*p = NULL;
		}

		return result;
	}

	palo_err realloc_safe(void **p, size_t new_size) {
		void *t;

		t = realloc(*p, new_size);
		if (t != NULL || new_size == 0) {
			*p = t;
			return PALO_SUCCESS;
		} else {
			return PALO_ERR_NO_MEM;
		}
	}

#ifndef VS6
	size_t vsnwprintf(wchar_t *s, size_t len, const wchar_t *format, va_list l) {
#if defined(WIN32) || defined(WIN64)
	return (s == NULL) ? _vscwprintf(format, l) : _vsnwprintf(s, len, format, l);
#else
		return vswprintf(s, len, format, l);
#endif
	}
#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif
#endif
