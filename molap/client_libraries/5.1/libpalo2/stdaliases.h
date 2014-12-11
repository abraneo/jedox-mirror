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

#ifndef HAS_STDALIASES_H
#define HAS_STDALIASES_H

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#include <sys/types.h>
#include <stdarg.h>

#include "const.h"

#include "error.h"

#define SIZE_T_TO_MAX_INT(s) (((s)>INT_MAX && (s)!=(size_t)-1) ? INT_MAX : (int)(s))
#define SIZE_T_TO_MAX_UINT(s) ((s)>UINT_MAX ? UINT_MAX : (size_t)(s))

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

#if !defined(WIN32) && !defined(WIN64)
#define wcsicmp wcscasecmp
#define stricmp strcasecmp
#define snwprintf swprintf
#ifndef va_copy
#define va_copy(dest, src) __va_copy((dest), (src));
#endif
#else
#define snwprintf _snwprintf
#define va_copy(dest, src) (dest)=(src)
	size_t vsnwprintf(wchar_t *s, size_t len, const wchar_t *format, va_list l);
#endif

#ifdef VS2003
#define vsnprintf _vsnprintf
#endif

#include <stdio.h>

	palo_err realloc_free(void **p, size_t new_size);
	palo_err realloc_safe(void **p, size_t new_size);

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif
#endif
