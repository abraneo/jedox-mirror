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

#include "arg_types.h"

#include "generic_mappings.h"

/* encoding specific (utf-8) */
#define PALO_STRING(s)                  s
#define CHAR_T                          char
#define PALO_SUFFIX_STR                 _m
#define PALO_SUFFIX2_STR                Utf8
#include "interface_generic.h.h"
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR

/* encoding specific (current codepage) */
#define PALO_STRING(s)                  s
#define CHAR_T                          char
#define PALO_SUFFIX_STR                 _a
#define PALO_SUFFIX2_STR                C
#include "interface_generic.h.h"
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR

/* encoding specific (wide characters) */
#define PALO_STRING(s)                  L ## s
#define CHAR_T                          wchar_t
#define PALO_SUFFIX_STR                 _w
#define PALO_SUFFIX2_STR                W
#include "interface_generic.h.h"
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR
