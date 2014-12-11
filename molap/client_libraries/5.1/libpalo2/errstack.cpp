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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "strtools.h"
#include "stdaliases.h"
#include "errstack.h"

#define ERRSTACK_DEFAULT_FUNCTION_TEXT  "unknown_func"
#define ERRSTACK_DEFAULT_FILE_TEXT      "unknown_file"
#define ERRSTACK_DEFAULT_DESCRIPTION    L"-"
#define ERRSTACK_MESSAGE_FUNC_WIDTH     24
#define ERRSTACK_MESSAGE_FILE_WIDTH     16

palo_err _errstack_return(struct errstack *errs, palo_err result, const char *function, int line, const char *file, wchar_t *description_format, ...) {
	va_list l;
	const wchar_t *defformat = L"Error %ld has occured";
	wchar_t *s;
	size_t len;

	if (errs->depth == ERRSTACK_MAX_DEPTH - 1) {
		if (errs->stack[errs->depth].func != NULL) {
			errs->stack[errs->depth].func = __FUNCTION__;
			errs->stack[errs->depth].line = __LINE__;
			errs->stack[errs->depth].file = __FILE__;
			errs->stack[errs->depth].description = wcsdup(L"end of error report - error stack full!");
		}
	} else {
		errs->stack[errs->depth].func = function;
		errs->stack[errs->depth].line = line;
		errs->stack[errs->depth].file = file;

		va_start(l, description_format);

		if ((description_format != NULL) && wcslen(description_format) > 0) {
			errs->stack[errs->depth].description = vwcsdupf(description_format, l);
		} else {
			errs->stack[errs->depth].description = NULL;
		}
		va_end(l);
	}

	free(description_format);

	if (errs->stack[errs->depth].description == NULL) {
		len = wcslen(defformat) + 20;
		s = (wchar_t*)calloc(len, sizeof(wchar_t));

		if (s != NULL) {
			swprintf(s, len, defformat, result);
			errs->stack[errs->depth].description = s;
		}
	}

	if (errs->depth != ERRSTACK_MAX_DEPTH - 1) {
		errs->depth++;
	}

	return result;
}


void errstack_free(struct errstack *errs) {
	size_t i;

	for (i = 0; i < ERRSTACK_MAX_DEPTH; i++) {
		if (errs->stack[i].description != NULL) {
			free(errs->stack[i].description);
			errs->stack[i].description = NULL;
		}
		errs->stack[i].func = NULL;
		errs->stack[i].file = NULL;
		errs->stack[i].line = 0;
	}

	errs->depth = 0;
}


wchar_t *errstack_errmsg_long(const struct errstack *errs, const palo_err result) {
	size_t offset;
	size_t i;
	wchar_t *errmsg, *t;
	size_t len;

	errmsg = wcsdupf(L"Error code: %d\n", result);
	if (errmsg == NULL) {
		return NULL;
	}
	t = errmsg + wcslen(errmsg);

	if (errs == NULL) {
		t = wcsdupf(L"%lsNo errstack given! (errs==NULL)\n", errmsg);
		free(errmsg);
		return t;
	}

	for (i = 0; i < errs->depth; i++) {
		offset = t - errmsg;
		len = 5 
		      + ERRSTACK_MESSAGE_FUNC_WIDTH + 3
		      + ERRSTACK_MESSAGE_FILE_WIDTH + 9
		      + (errs->stack[i].description != NULL ? wcslen(errs->stack[i].description) : wcslen(ERRSTACK_DEFAULT_DESCRIPTION)) + 1;

		if (realloc_free((void**)&errmsg, (offset + len + 1)*sizeof(wchar_t)) != PALO_SUCCESS) {
			return NULL;
		}

		t = errmsg + offset;
		snwprintf(t, len + 1, L"%4d\t"
		           L"%" WIDEN(LINESTR(ERRSTACK_MESSAGE_FUNC_WIDTH)) L"." WIDEN(LINESTR(ERRSTACK_MESSAGE_FUNC_WIDTH)) L"hs() "
		           L"[%" WIDEN(LINESTR(ERRSTACK_MESSAGE_FILE_WIDTH)) L"." WIDEN(LINESTR(ERRSTACK_MESSAGE_FILE_WIDTH)) L"hs:%04d]:\t"
		           L"%ls\n", i,
		           errs->stack[i].func != NULL ? errs->stack[i].func : ERRSTACK_DEFAULT_FUNCTION_TEXT,
		           errs->stack[i].file != NULL ? (strrchr(errs->stack[i].file, PATH_SEPARATOR) != NULL ? strrchr(errs->stack[i].file, PATH_SEPARATOR) + 1 : errs->stack[i].file) : ERRSTACK_DEFAULT_FILE_TEXT,
				   errs->stack[i].line,
				   errs->stack[i].description != NULL ? errs->stack[i].description : ERRSTACK_DEFAULT_DESCRIPTION);
		t += wcslen(t);
	}

	return errmsg;
}

wchar_t *errstack_errmsg_short(const struct errstack *errs, const palo_err result) {
	return wcsdupf(L"%ls (Error %d)", errs->stack[0].description != NULL ? errs->stack[0].description : ERRSTACK_DEFAULT_DESCRIPTION, result);
}
