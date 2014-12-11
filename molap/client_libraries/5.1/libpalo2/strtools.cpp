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
#pragma warning(disable : 4100)
#include <io.h>
#endif

#include <wchar.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "error.h"
#include "stdaliases.h"
#include "strtools.h"

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

#define IS_WHITESPACE(a) (a=='\t' || a=='\n' || a==' ')

palo_err string_get_word_or_whitespace_a(char **to, const char *from, size_t *index) {
	size_t idx, last_idx;

	assert(to != NULL);
	assert(from != NULL);
	assert(index != NULL);

	if (IS_WHITESPACE(from[*index])) {
		*to = strdupf("%c", from[*index]);
		if (*to == NULL) {
				return PALO_ERR_NO_MEM;
		}
		++(*index);
		return PALO_SUCCESS;
	}

	/* skip data */
	for (idx = *index; !IS_WHITESPACE(from[idx]) && from[idx] != L'\0'; idx++);
	last_idx = idx;

	*to = (char*)malloc((last_idx - *index + 1) * sizeof(char));
	if (*to == NULL) {
			return PALO_ERR_NO_MEM;
	}
	memcpy(*to, from + *index, (last_idx - *index)*sizeof(char));
	(*to)[last_idx-*index] = '\0';

	*index = idx;

	return PALO_SUCCESS;
}

palo_err init_conversions(struct conversions *convs) {
#if !defined(WIN32) && !defined(WIN64)
	char *local = nl_langinfo(CODESET);
	convs->utf82local = iconv_open(local, "UTF-8");
	if (convs->utf82local == (iconv_t)(-1)) {
		return PALO_ERR_STRING;
	}
	convs->local2utf8 = iconv_open("UTF-8", local);
	if (convs->local2utf8 == (iconv_t)(-1)) {
		iconv_close(convs->utf82local);
		return PALO_ERR_STRING;
	}
	convs->wchar_t2utf8 = iconv_open("UTF-8", "WCHAR_T");
	if (convs->wchar_t2utf8 == (iconv_t)(-1)) {
		iconv_close(convs->utf82local);
		iconv_close(convs->local2utf8);
		return PALO_ERR_STRING;
	}
	convs->utf82wchar_t = iconv_open("WCHAR_T", "UTF-8");
	if (convs->utf82wchar_t == (iconv_t)(-1)) {
		iconv_close(convs->utf82local);
		iconv_close(convs->local2utf8);
		iconv_close(convs->wchar_t2utf8);
		return PALO_ERR_STRING;
	}
#endif
	return PALO_SUCCESS;
}

void free_conversions(struct conversions *convs) {
#if !defined(WIN32) && !defined(WIN64)
	iconv_close(convs->utf82local);
	iconv_close(convs->local2utf8);
	iconv_close(convs->wchar_t2utf8);
	iconv_close(convs->utf82wchar_t);
#endif
}

#if !defined(WIN32) && !defined(WIN64)

#define ICONV_BUFSIZE 128
palo_err do_iconv(iconv_t conv, const char *from, size_t from_len, char **to) {
	char *out, *out_start, *from_temp;
	size_t out_size, out_idx, out_free, result;
	palo_err ret;

	assert(from != NULL);
	assert(to != NULL);

	/* reset iconv */
	iconv(conv, NULL, NULL, NULL, NULL);

	out_size = from_len;
	out_start = out = (char*)malloc(out_size * sizeof(char));
	if (out_start == NULL) {
			return PALO_ERR_NO_MEM;
	}
	from_temp = (char*)from;

	out_free = out_size;
	while (from_len > 0) {
		result = iconv(conv, &from_temp, &from_len, &out, &out_free);
		if (result == (size_t)(-1)) {
			if (errno == E2BIG) {
				out_size += ICONV_BUFSIZE;
				out_idx = out - out_start;
				ret = realloc_free((void**) & out_start, out_size * sizeof(char));
				if (ret != PALO_SUCCESS) {
					free(out_start);
					return ret;
				}
				/* might have been moved by realloc() */
				out = out_start + out_idx;
				out_free += ICONV_BUFSIZE;
			} else {
				free(out_start);
				return PALO_ERR_STRING;
			}
		} else if (result == 0) {
			break;
		} else {
			free(out_start);
			return PALO_ERR_STRING;
		}
	}

	out_start = (char*)realloc((void*)out_start, out_size - out_free);
	assert(out_start != NULL);
	*to = out_start;

	return PALO_SUCCESS;
}
#endif

palo_err wcs2utf8(struct conversions *convs, char **utf8_str, const wchar_t *s) {
#if defined(WIN32) || defined(WIN64)
	size_t len;

	if ((len = wcs2utf8_len(s, (size_t) - 1)) == (size_t) - 1) {
		return PALO_ERR_STRING;
	}

	/* len in bytes */
	*utf8_str = (char*)malloc(len);
	if (*utf8_str == NULL) {
			return PALO_ERR_NO_MEM;
	}
	wcs2utf8_conv(*utf8_str, s, len);

	return PALO_SUCCESS;
#else
	return do_iconv(convs->wchar_t2utf8, (char*)s, (wcslen(s) + 1)*sizeof(wchar_t), utf8_str);
#endif
}

palo_err utf82wcs(struct conversions *convs, wchar_t **wcs, const char *utf8_str) {
#if defined(WIN32) || defined(WIN64)
	int len;

	if ((len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0)) == 0) {
		return PALO_ERR_STRING;
	}
	
	/* len in wchar's */
	*wcs = (wchar_t*)malloc(len*sizeof(wchar_t));
	if (*wcs == NULL) {
			return PALO_ERR_NO_MEM;
	}
	utf82wcs_conv(*wcs, utf8_str, len);

	return PALO_SUCCESS;
#else
	return do_iconv(convs->utf82wchar_t, utf8_str, (strlen(utf8_str) + 1)*sizeof(char), (char**)wcs);
#endif
}

palo_err utf82mbs(struct conversions *convs, char **mbs, const char *utf8_str) {
#if defined(WIN32) || defined(WIN64)
	palo_err result;
	wchar_t *t;

	result = utf82wcs(convs, &t, utf8_str);
	if (result != PALO_SUCCESS) {
		return PALO_ERR_STRING;
	}

	result = wcs2mbs(mbs, t);
	free(t);
	if (result != PALO_SUCCESS) {
		return result;
	}

	return PALO_SUCCESS;
#else
	return do_iconv(convs->utf82local, utf8_str, (strlen(utf8_str) + 1)*sizeof(char), mbs);
#endif
}

palo_err mbs2utf8(struct conversions *convs, char **utf8_str, const char *mbs_str) {
#if defined(WIN32) || defined(WIN64)
	palo_err result;
	wchar_t *u;

	result = mbs2wcs(&u, mbs_str);
	if (result != PALO_SUCCESS) {
		return result;
	}

	result = wcs2utf8(convs, utf8_str, u);
	free(u);
	if (result != PALO_SUCCESS) {
		return result;
	}

	return PALO_SUCCESS;
#else
	return do_iconv(convs->local2utf8, mbs_str, (strlen(mbs_str) + 1)*sizeof(char), utf8_str);
#endif
}

wchar_t *vwcsdupf(const wchar_t *format, va_list l) {
	size_t len;
	wchar_t *s;
	va_list temp;
#if !defined(WIN32) && !defined(WIN64)
	size_t temp_len;
#endif

#if defined(WIN32) || defined(WIN64)
	va_copy(temp, l);
	len = _vscwprintf(format, l) + 1;
	va_end(temp);
	s = (wchar_t*)malloc(len * sizeof(wchar_t));
	vsnwprintf(s, len, format, l);
#else
	// TODO: faster
	len = BUFSIZE;
	s = NULL;
	do {
		len += BUFSIZE;
		if (realloc_free((void**)&s, len*sizeof(wchar_t)) != PALO_SUCCESS) {
			va_end(l);
			return NULL;
		}
		va_copy(temp, l);
		temp_len = vswprintf(s, len, format, temp);
		va_end(temp);
	} while (temp_len == -1);
	s = (wchar_t*)realloc((void*)s, (temp_len + 1) * sizeof(wchar_t));
	assert(s != NULL);
#endif

	return s;
}

wchar_t *wcsdupf(const wchar_t *format, ...) {
	wchar_t *s;
	va_list l;

	va_start(l, format);
	s = vwcsdupf(format, l);
	va_end(l);

	return s;
}

char *strdupf(const char *format, ...) {
	va_list l, temp;
	int len;
	char *s;

	va_start(l, format);

	va_copy(temp, l);
	len = vsnprintf(NULL, 0, format, temp) + 1;
	va_end(temp);

	if (len < 0) {
		va_end(l);
		return NULL;
	}

	s = (char*)malloc(len * sizeof(char));

	if (s == NULL) {
		va_end(l);
		return NULL;
	}

	vsprintf(s, format, l);
	va_end(l);

	return s;
}

palo_err wcs2mbs(char **r, const wchar_t *s) {
	size_t l = 0;

	assert(r != NULL);
	assert(s != NULL);

	/* length */
	l = wcstombs(NULL, s, 0);
	if (l == -1) {
		*r = NULL;
		return PALO_ERR_STRING;
	}

	/* malloc */
	*r = (char*)malloc((l + 1) * sizeof(char));
	if (r == NULL) {
		return PALO_ERR_NO_MEM;
	}

	/* convert */
	if (wcstombs(*r, s, l + 1) == -1) {
		free(*r);
		*r = NULL;
		return PALO_ERR_STRING;
	}

	return PALO_SUCCESS;
}

palo_err mbs2wcs(wchar_t **r, const char *s) {
	size_t l = 0;

	assert(r != NULL);
	assert(s != NULL);

	/* length */
	l = mbstowcs(NULL, s, 0);
	if (l == -1) {
		*r = NULL;
		return PALO_ERR_STRING;
	}

	/* malloc */
	*r = (wchar_t*)malloc((l + 1) * sizeof(wchar_t));
	if (r == NULL) {
		return PALO_ERR_NO_MEM;
	}

	/* convert */
	if (mbstowcs(*r, s, l) == -1) {
		free(*r);
		*r = NULL;
		return PALO_ERR_STRING;
	}

	(*r)[l] = L'\0';

	return PALO_SUCCESS;
}

#if defined(WIN32) || defined(WIN64)
palo_err utf82wcs_conv(wchar_t *r, const char *s, size_t max_len) {
	if (MultiByteToWideChar(CP_UTF8, 0, s, -1, r, SIZE_T_TO_MAX_INT(max_len)) == 0) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			PALO_ERR_STRING;
		}
	}
	return PALO_SUCCESS;
}

palo_err wcs2utf8_conv(char *r, const wchar_t *s, size_t max_len) {
	if (WideCharToMultiByte(CP_UTF8, 0, s, -1, r, SIZE_T_TO_MAX_INT(max_len), NULL, NULL) == 0) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			PALO_ERR_STRING;
		}
	}
	return PALO_SUCCESS;
}

size_t wcs2utf8_len(const wchar_t *s, size_t max_len) {
	size_t len;

	if (max_len == 0) {
		return 0;
	}

	len = WideCharToMultiByte(CP_UTF8, 0, s, SIZE_T_TO_MAX_INT(max_len), NULL, 0, NULL, NULL);
	if (len == 0) {
		return (size_t) - 1;
	}

	return len;
}
#endif

size_t mymbslen(const char *s, size_t max_len) {
#if !defined(WIN32) && !defined(WIN64)
	size_t result, pos = 0, len = 0;
	mbstate_t mbstate;

	memset(&mbstate, '\0', sizeof(mbstate));

	while (pos < max_len) {
		result = mbrlen(s + pos, max_len - pos, &mbstate);
		if (result < (size_t)0) {
			return -1;
		}
		if (result == 0) {	 /* L'\0' encountered */
				return len;
		}
		pos += result;
		len++;
	}

	return len;
#else
	size_t len;

	if (max_len == 0) {
		return 0;
	}

	len = MultiByteToWideChar(CP_UTF8, 0, s, SIZE_T_TO_MAX_INT(max_len), NULL, 0);

	return len == 0 ? -1 : len;
#endif
}

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif
