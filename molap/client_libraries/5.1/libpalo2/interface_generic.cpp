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
#endif

using namespace std;

#include <string.h>

#include <libpalo_ng/Network/SocketException.h>
#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Palo/Cache/Cache.h>
#include "../JedoxXlHelper/JedoxXLHelper.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libpalo2.h"
#include "error.h"
#include "interface_generic.h"
#include "interface.h"
#include "connect.h"

#include "generic_mappings.h"
#include "stdaliases.h"
#include "strtools.h"
#include "arg_types.h"


static libpalo_result _libpalo_failure() {
	libpalo_result r;

	r.succeeded = PALO_FALSE;

	return r;
}


static libpalo_result _libpalo_success() {
	libpalo_result r;

	r.succeeded = PALO_TRUE;

	return r;
}


libpalo_result _to_libpalo_result(palo_err result, libpalo_err *err, struct errstack *errs, const char *func, const char *file, int line) {
	err->errs = *errs;
	err->palo_error = result;
	err->original_error = (long)err->palo_error;

	switch (err->original_error) {

		case PaloExceptionFactory::ERROR_ID_NOT_FOUND:
			err->palo_error = PALO_ERR_ID_NOT_FOUND;
			break;

		case PaloExceptionFactory::ERROR_INVALID_FILENAME:
			err->palo_error = PALO_ERR_INVALID_FILENAME;
			break;

		case PaloExceptionFactory::ERROR_MKDIR_FAILED:
			err->palo_error = PALO_ERR_MKDIR;
			break;

		case PaloExceptionFactory::ERROR_RENAME_FAILED:
			err->palo_error = PALO_ERR_FILE_RENAME;
			break;

		case PaloExceptionFactory::ERROR_AUTHORIZATION_FAILED:
			err->palo_error = PALO_ERR_AUTH;
			break;

		case PaloExceptionFactory::ERROR_INVALID_TYPE:
			err->palo_error = PALO_ERR_TYPE;
			break;

		case PaloExceptionFactory::ERROR_INVALID_COORDINATES:
			err->palo_error = PALO_ERR_INV_ARG;
			break;

		case PaloExceptionFactory::ERROR_FILE_NOT_FOUND_ERROR:
			err->palo_error = PALO_ERR_FILE;
			break;

		case PaloExceptionFactory::ERROR_NOT_AUTHORIZED:
			err->palo_error = PALO_ERR_AUTH;
			break;

		case PaloExceptionFactory::ERROR_CORRUPT_FILE:
			err->palo_error = PALO_ERR_FILE;
			break;

		case PaloExceptionFactory::ERROR_PARAMETER_MISSING:
			err->palo_error = PALO_ERR_INV_ARG;
			break;

		case PaloExceptionFactory::ERROR_INVALID_SPLASH_MODE:
			err->palo_error = PALO_ERR_INV_ARG;
			break;

		case PaloExceptionFactory::ERROR_API_CALL_NOT_IMPLEMENTED:
			err->palo_error = PALO_ERR_NOT_IMPLEMENTED;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_NOT_FOUND:
			err->palo_error = PALO_ERR_DATABASE_NOT_FOUND;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_STILL_LOADED:
			err->palo_error = PALO_ERR_ALREADY_LOADED;
			break;

		case PaloExceptionFactory::ERROR_INVALID_DATABASE_NAME:
			err->palo_error = PALO_ERR_INV_DATABASE_NAME;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_NAME_IN_USE:
			err->palo_error = PALO_ERR_IN_USE;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND:
			err->palo_error = PALO_ERR_DIMENSION_NOT_FOUND;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_EMPTY:
			err->palo_error = PALO_ERR_DIM_EMPTY;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_EXISTS:
			err->palo_error = PALO_ERR_DIMENSION_EXISTS;
			break;

		case PaloExceptionFactory::ERROR_INVALID_DIMENSION_NAME:
			err->palo_error = PALO_ERR_INV_DIMENSION_NAME;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_NAME_IN_USE:
			err->palo_error = PALO_ERR_IN_USE;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_IN_USE:
			err->palo_error = PALO_ERR_IN_USE;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NOT_FOUND:
			err->palo_error = PALO_ERR_DIM_ELEMENT_NOT_FOUND;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_EXISTS:
			err->palo_error = PALO_ERR_DIMENSION_ELEMENT_EXISTS;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_CIRCULAR_REFERENCE:
			err->palo_error = PALO_ERR_CIRCULAR_REF;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NAME_IN_USE:
			err->palo_error = PALO_ERR_IN_USE;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NAME_NOT_UNIQUE:
			err->palo_error = PALO_ERR_NAME_NOT_UNIQUE;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NO_CHILD_OF:
			err->palo_error = PALO_ERR_DIM_ELEMENT_NOT_CHILD;
			break;

		case PaloExceptionFactory::ERROR_INVALID_ELEMENT_NAME:
			err->palo_error = PALO_ERR_INV_DIMENSION_ELEMENT_NAME;
			break;

		case PaloExceptionFactory::ERROR_INVALID_OFFSET:
			err->palo_error = PALO_ERR_INV_OFFSET;
			break;

		case PaloExceptionFactory::ERROR_INVALID_ELEMENT_TYPE:
			err->palo_error = PALO_ERR_DIM_ELEMENT_INV_TYPE;
			break;

		case PaloExceptionFactory::ERROR_INVALID_POSITION:
			err->palo_error = PALO_ERR_INV_OFFSET;
			break;

		case PaloExceptionFactory::ERROR_CUBE_NOT_FOUND:
			err->palo_error = PALO_ERR_CUBE_NOT_FOUND;
			break;

		case PaloExceptionFactory::ERROR_INVALID_CUBE_NAME:
			err->palo_error = PALO_ERR_INV_CUBE_NAME;
			break;

		case PaloExceptionFactory::ERROR_CUBE_EMPTY:
			err->palo_error = PALO_ERR_EMPTY_CUBE;
			break;

		case PaloExceptionFactory::ERROR_CUBE_NAME_IN_USE:
			err->palo_error = PALO_ERR_IN_USE;
			break;

		case PaloExceptionFactory::ERROR_NET_ARG:
			err->palo_error = PALO_ERR_NET_ARG;
			break;

		case PaloExceptionFactory::ERROR_INV_CMD:
			err->palo_error = PALO_ERR_INV_CMD;
			break;

		case PaloExceptionFactory::ERROR_INV_CMD_CTL:
			err->palo_error = PALO_ERR_INV_CMD_CTL;
			break;

		case PaloExceptionFactory::ERROR_NET_SEND:
			err->palo_error = PALO_ERR_NET_SEND;
			break;

		case PaloExceptionFactory::ERROR_NET_CONN_TERM:
			err->palo_error = PALO_ERR_NET_CONN_TERM;
			break;

		case PaloExceptionFactory::ERROR_NET_RECV:
			err->palo_error = PALO_ERR_NET_RECV;
			break;

		case PaloExceptionFactory::ERROR_NET_HS_HALLO:
			err->palo_error = PALO_ERR_NET_HS_HELLO;
			break;

		case PaloExceptionFactory::ERROR_NET_HS_PROTO:
			err->palo_error = PALO_ERR_NET_HS_PROTO;
			break;

		case PaloExceptionFactory::ERROR_INV_ARG_COUNT:
			err->palo_error = PALO_ERR_INV_ARG_COUNT;
			break;

		case PaloExceptionFactory::ERROR_INV_ARG_TYPE:
			err->palo_error = PALO_ERR_INV_ARG_TYPE;
			break;

		case PaloExceptionFactory::ERROR_CLIENT_INV_NET_REPLY:
			err->palo_error = PALO_ERR_CLIENT_INV_NET_REPLY;
			break;

	}

	if (err->palo_error != PALO_SUCCESS) {
		ERRSTACK_PREPARE(&err->errs, result, func, line, file, NULL);
		ERRSTACK_PREPARE(&err->errs, result, __FUNCTION__, __LINE__, __FILE__, NULL);
		return err->result = _libpalo_failure();
	}

	return err->result = _libpalo_success();
}


/* encoding specific (utf-8) */
#define PALO_STRING(s)                  s
#define CHAR_T                          char
#define PALO_SUFFIX_STR                 _m
#define PALO_SUFFIX2_STR                Utf8
#define STR_TO_UTF8(a, b, c)            ((*(b)=strdup(c))==NULL ? PALO_ERR_NO_MEM:PALO_SUCCESS)
#define UTF8_TO_STR(a, b, c)            ((*(b)=strdup(c))==NULL ? PALO_ERR_NO_MEM:PALO_SUCCESS)
#define STR_TO_LOCAL(a, b, c)           utf82mbs(a, b, c)
#define WCS_TO_STR(a, b, c)             wcs2utf8(a, b, c)
#define STR_TO_WCS(a, b, c)             utf82wcs(a, b, c)
#define mystrdup                        strdup
#define mystrlen                        strlen
#define ENCODING_M
#include "interface_generic.c.h"
#undef mystrlen
#undef mystrdup
#undef WCS_TO_STR
#undef STR_TO_WCS
#undef ENCODING_M
#undef STR_TO_LOCAL
#undef UTF8_TO_STR
#undef STR_TO_UTF8
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR

/* encoding specific (current codepage) */
#define PALO_STRING(s)                  s
#define CHAR_T                          char
#define PALO_SUFFIX_STR                 _a
#define PALO_SUFFIX2_STR                C
#define STR_TO_UTF8(a, b, c)            mbs2utf8(a, b, c)
#define UTF8_TO_STR(a, b, c)            utf82mbs(a, b, c)
#define STR_TO_LOCAL(a, b, c)           ((*(b)=strdup(c))==NULL ? PALO_ERR_NO_MEM:PALO_SUCCESS)
#define WCS_TO_STR(a, b, c)             wcs2mbs(b, c)
#define STR_TO_WCS(a, b, c)             mbs2wcs(b, c)
#define mystrdup                        strdup
#define mystrlen                        strlen
#define ENCODING_A
#include "interface_generic.c.h"
#undef ENCODING_A
#undef mystrlen
#undef mystrdup
#undef WCS_TO_STR
#undef STR_TO_WCS
#undef STR_TO_LOCAL
#undef UTF8_TO_STR
#undef STR_TO_UTF8
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR

/* encoding specific (wide characters) */
#define PALO_STRING(s)                  L ## s
#define CHAR_T                          wchar_t
#define PALO_SUFFIX_STR                 _w
#define PALO_SUFFIX2_STR                W
#define STR_TO_UTF8(a, b, c)            wcs2utf8(a, b, c)
#define UTF8_TO_STR(a, b, c)            utf82wcs(a, b, c)
#define STR_TO_LOCAL(a, b, c)           wcs2mbs(b, c)
#define WCS_TO_STR(a, b, c)             ((*(b)=wcsdup(c))==NULL ? PALO_ERR_NO_MEM:PALO_SUCCESS)
#define STR_TO_WCS(a, b, c)             ((*(b)=wcsdup(c))==NULL ? PALO_ERR_NO_MEM:PALO_SUCCESS)
#define mystrdup                        wcsdup
#define mystrlen                        wcslen
#define ENCODING_W
#include "interface_generic.c.h"
#undef ENCODING_W
#undef mystrlen
#undef mystrdup
#undef WCS_TO_STR
#undef STR_TO_WCS
#undef STR_TO_LOCAL
#undef UTF8_TO_STR
#undef STR_TO_UTF8
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX_STR
#undef PALO_SUFFIX2_STR
