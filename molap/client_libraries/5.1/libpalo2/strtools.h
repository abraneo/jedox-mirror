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

#ifndef HAS_STRTOOLS_H
#define HAS_STRTOOLS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>

#include "error.h"
#include "conversions.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "const.h"

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
extern "C" {
#endif
#endif

	wchar_t *wcsdupf(const wchar_t *format, ...);
	wchar_t *vwcsdupf(const wchar_t *format, va_list l);
	char *strdupf(const char *format, ...);

#if defined(WIN32) || defined(WIN64)
	palo_err utf82wcs_conv(wchar_t *r, const char *s, size_t max_len);
	palo_err wcs2utf8_conv(char *r, const wchar_t *s, size_t max_len);
	size_t wcs2utf8_len(const wchar_t *s, size_t max_len);
#endif

	palo_err wcs2mbs(char **r, const wchar_t *s);
	palo_err mbs2wcs(wchar_t **r, const char *s);
	palo_err wcs2utf8(struct conversions *convs, char **utf8_str, const wchar_t *s);
	palo_err utf82wcs(struct conversions *convs, wchar_t **wcs, const char *utf8_str);

	palo_err mbs2utf8(struct conversions *convs, char **utf8_str, const char *mbs_str);
	palo_err utf82mbs(struct conversions *convs, char **mbs, const char *utf8_str);

	size_t mymbslen(const char *s, size_t max_len);

	palo_err init_conversions(struct conversions *convs);
	void free_conversions(struct conversions *convs);

	palo_err string_get_word_or_whitespace_a(char **to, const char *from, size_t *idx);

#ifndef IGNORE_CPLUSPLUS
#ifdef __cplusplus
}
#endif
#endif
#endif
