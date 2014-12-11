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

#ifndef HAS_CONST_H

#ifndef PALO
#define PALO
#endif

#define BUFSIZE             512

#define WIDEN2(x)           L ## x
#define WIDEN(x)            WIDEN2(x)
#define LINESTR2(x)         #x
#define LINESTR(x)          LINESTR2(x)

#if defined(WIN32) || defined(WIN64)
#define PATH_SEPARATOR          '\\'
#else
#define PATH_SEPARATOR          '/'
#endif

#ifdef VS6
#define __FUNCTION__    "dummy_string"
#define STRBUFSIZE  1024
#endif

#ifdef VS2005
#define strdup(a)       _strdup(a)
#define wcsdup(a)       _wcsdup(a)
#define getcwd(b, m)    _getcwd(b, m)
#define wcsicmp(a, b)   _wcsicmp(a, b)
#define chdir(d)        _chdir(d)
#define mktemp(t)       _mktemp(t)
#define mkdir(d)        _mkdir(d)
#endif

typedef char *palo_bool;
#define PALO_TRUE (palo_bool)1
#define PALO_FALSE (palo_bool)NULL
#define PALO_NOT(b) (b ? PALO_FALSE:PALO_TRUE)

#define HAS_CONST_H
#endif
