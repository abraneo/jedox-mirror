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

#ifndef HAS_ERROR_H

#include <wchar.h>

typedef wchar_t *palo_err;

// Legacy Error Codes

#define PALO_ERR_INVALID_FILENAME_ID			-4
#define PALO_ERR_NET_ARG_ID						-19
#define PALO_ERR_ALREADY_LOADED_ID				-20
#define PALO_ERR_FILE_ID						-22
#define PALO_ERR_EMPTY_CUBE_ID					-24
#define PALO_ERR_TYPE_ID						-26
#define PALO_ERR_INV_ARG_ID						-27
#define PALO_ERR_IN_USE_ID						-28
#define PALO_ERR_NOT_IMPLEMENTED_ID				-30
#define PALO_ERR_NETWORK_ID						-31
#define PALO_ERR_SYSTEM_ID						-32
#define PALO_ERR_DATABASE_NOT_FOUND_ID			-34
#define PALO_ERR_DIMENSION_NOT_FOUND_ID			-35
#define PALO_ERR_CUBE_NOT_FOUND_ID				-36
#define PALO_ERR_DIM_ELEMENT_NOT_FOUND_ID		-37
#define PALO_ERR_DIM_ELEMENT_INV_TYPE_ID		-38
#define PALO_ERR_INV_OFFSET_ID					-39
#define PALO_ERR_DIM_ELEMENT_NOT_CHILD_ID		-40
#define PALO_ERR_DIM_EMPTY_ID					-41
#define PALO_ERR_AUTH_ID						-44
#define PALO_ERR_MKDIR_ID						-47
#define PALO_ERR_FILE_RENAME_ID					-48
#define PALO_ERR_NAME_NOT_UNIQUE_ID				-49
#define PALO_ERR_DIMENSION_ELEMENT_EXISTS_ID	-50
#define PALO_ERR_DIMENSION_EXISTS_ID			-51
#define PALO_ERR_CIRCULAR_REF_ID				-55
#define PALO_ERR_ID_NOT_FOUND_ID				-56
#define PALO_ERR_INV_CMD_ID						-59
#define PALO_ERR_INV_CMD_CTL_ID					-60
#define PALO_ERR_NET_SEND_ID					-61
#define PALO_ERR_NET_CONN_TERM_ID				-62
#define PALO_ERR_NET_RECV_ID					-63
#define PALO_ERR_NET_HS_HELLO_ID				-64
#define PALO_ERR_NET_HS_PROTO_ID				-65
#define PALO_ERR_INV_DATABASE_NAME_ID			-68
#define PALO_ERR_INV_CUBE_NAME_ID				-69
#define PALO_ERR_INV_DIMENSION_NAME_ID			-70
#define PALO_ERR_INV_DIMENSION_ELEMENT_NAME_ID	-71
#define PALO_ERR_INV_ARG_COUNT_ID				-72
#define PALO_ERR_INV_ARG_TYPE_ID				-73
#define PALO_ERR_CLIENT_INV_NET_REPLY_ID		-74
#define PALO_ERR_FREE_EXCEL_ADVANCED_MODEL_ID	-86

#define PALO_SUCCESS                            (palo_err)0
#define PALO_ERR_NO_MEM                         (palo_err)-1
#define PALO_ERR_XML_SYNTAX                     (palo_err)-2
#define PALO_ERR_INVALID_POINTER                (palo_err)-3
#define PALO_ERR_INVALID_FILENAME               (palo_err)PALO_ERR_INVALID_FILENAME_ID
#define PALO_ERR_XML_WRONG_VERSION              (palo_err)-5
#define PALO_ERR_READ                           (palo_err)-6
#define PALO_ERR_XML_NO_DTD                     (palo_err)-7
#define PALO_ERR_XML_NO_DECL                    (palo_err)-8
#define PALO_ERR_XML_INVALID_TAG                (palo_err)-9
#define PALO_ERR_XML_INVALID_CHAR               (palo_err)-10
#define PALO_ERR_BUFFER_OVERFLOW                (palo_err)-11
#define PALO_ERR_XML_NO_CLOSING_TAG             (palo_err)-12
#define PALO_ERR_XML_ATTR                       (palo_err)-13
#define PALO_ERR_ID_IN_USE                      (palo_err)-14
#define PALO_ERR_ID_NOT_IN_USE                  (palo_err)-15
#define PALO_ERR_NO_FREE_ID                     (palo_err)-16
#define PALO_ERR_THREAD                         (palo_err)-17
#define PALO_ERR_MUTEX                          (palo_err)-18
#define PALO_ERR_NET_ARG                        (palo_err)PALO_ERR_NET_ARG_ID
#define PALO_ERR_ALREADY_LOADED                 (palo_err)PALO_ERR_ALREADY_LOADED_ID
#define PALO_ERR_STRING                         (palo_err)-21
#define PALO_ERR_FILE                           (palo_err)PALO_ERR_FILE_ID
#define PALO_ERR_NOT_FOUND                      (palo_err)-23
#define PALO_ERR_EMPTY_CUBE                     (palo_err)PALO_ERR_EMPTY_CUBE_ID
#define PALO_ERR_IM                             (palo_err)-25
#define PALO_ERR_TYPE                           (palo_err)PALO_ERR_TYPE_ID
#define PALO_ERR_INV_ARG                        (palo_err)PALO_ERR_INV_ARG_ID
#define PALO_ERR_IN_USE                         (palo_err)PALO_ERR_IN_USE_ID
#define PALO_ERR_QUERY                          (palo_err)-29
#define PALO_ERR_NOT_IMPLEMENTED                (palo_err)PALO_ERR_NOT_IMPLEMENTED_ID
#define PALO_ERR_NETWORK                        (palo_err)PALO_ERR_NETWORK_ID
#define PALO_ERR_SYSTEM                         (palo_err)PALO_ERR_SYSTEM_ID
#define PALO_ERR_ROOT_NOT_AVAILABLE             (palo_err)-33
#define PALO_ERR_DATABASE_NOT_FOUND             (palo_err)PALO_ERR_DATABASE_NOT_FOUND_ID
#define PALO_ERR_DIMENSION_NOT_FOUND            (palo_err)PALO_ERR_DIMENSION_NOT_FOUND_ID
#define PALO_ERR_CUBE_NOT_FOUND                 (palo_err)PALO_ERR_CUBE_NOT_FOUND_ID
#define PALO_ERR_DIM_ELEMENT_NOT_FOUND          (palo_err)PALO_ERR_DIM_ELEMENT_NOT_FOUND_ID
#define PALO_ERR_DIM_ELEMENT_INV_TYPE           (palo_err)PALO_ERR_DIM_ELEMENT_INV_TYPE_ID
#define PALO_ERR_INV_OFFSET                     (palo_err)PALO_ERR_INV_OFFSET_ID
#define PALO_ERR_DIM_ELEMENT_NOT_CHILD          (palo_err)PALO_ERR_DIM_ELEMENT_NOT_CHILD_ID
#define PALO_ERR_DIM_EMPTY                      (palo_err)PALO_ERR_DIM_EMPTY_ID
#define PALO_ERR_UNEXPECTED_THREAD_STATE        (palo_err)-42
#define PALO_ERR_AUTH_NA                        (palo_err)-43
#define PALO_ERR_AUTH                           (palo_err)PALO_ERR_AUTH_ID
#define PALO_ERR_INVALID_DIRNAME                (palo_err)-45
#define PALO_ERR_INV_FILE_TYPE                  (palo_err)-46
#define PALO_ERR_MKDIR                          (palo_err)PALO_ERR_MKDIR_ID
#define PALO_ERR_FILE_RENAME                    (palo_err)PALO_ERR_FILE_RENAME_ID
#define PALO_ERR_NAME_NOT_UNIQUE                (palo_err)PALO_ERR_NAME_NOT_UNIQUE_ID
#define PALO_ERR_DIMENSION_ELEMENT_EXISTS       (palo_err)PALO_ERR_DIMENSION_ELEMENT_EXISTS_ID
#define PALO_ERR_DIMENSION_EXISTS               (palo_err)PALO_ERR_DIMENSION_EXISTS_ID
#define PALO_ERR_MISSING_COORDINATES            (palo_err)-52
#define PALO_ERR_CLEANUP_LIST_FULL              (palo_err)-53
#define PALO_ERR_INVALID_STR                    (palo_err)-54
#define PALO_ERR_CIRCULAR_REF                   (palo_err)PALO_ERR_CIRCULAR_REF_ID
#define PALO_ERR_ID_NOT_FOUND                   (palo_err)PALO_ERR_ID_NOT_FOUND_ID
#define PALO_ERR_UNKNOWN_OPERATION              (palo_err)-57
#define PALO_ERR_EOF                            (palo_err)-58
#define PALO_ERR_INV_CMD                        (palo_err)PALO_ERR_INV_CMD_ID
#define PALO_ERR_INV_CMD_CTL                    (palo_err)PALO_ERR_INV_CMD_CTL_ID
#define PALO_ERR_NET_SEND                       (palo_err)PALO_ERR_NET_SEND_ID
#define PALO_ERR_NET_CONN_TERM                  (palo_err)PALO_ERR_NET_CONN_TERM_ID
#define PALO_ERR_NET_RECV                       (palo_err)PALO_ERR_NET_RECV_ID
#define PALO_ERR_NET_HS_HELLO                   (palo_err)PALO_ERR_NET_HS_HELLO_ID
#define PALO_ERR_NET_HS_PROTO                   (palo_err)PALO_ERR_NET_HS_PROTO_ID
#define PALO_ERR_CHILD_ELEMENT_EXISTS           (palo_err)-66
#define PALO_ERR_CHILD_ELEMENT_NOT_FOUND        (palo_err)-67
#define PALO_ERR_INV_DATABASE_NAME              (palo_err)PALO_ERR_INV_DATABASE_NAME_ID
#define PALO_ERR_INV_CUBE_NAME                  (palo_err)PALO_ERR_INV_CUBE_NAME_ID
#define PALO_ERR_INV_DIMENSION_NAME             (palo_err)PALO_ERR_INV_DIMENSION_NAME_ID
#define PALO_ERR_INV_DIMENSION_ELEMENT_NAME     (palo_err)PALO_ERR_INV_DIMENSION_ELEMENT_NAME_ID
#define PALO_ERR_INV_ARG_COUNT                  (palo_err)PALO_ERR_INV_ARG_COUNT_ID
#define PALO_ERR_INV_ARG_TYPE                   (palo_err)PALO_ERR_INV_ARG_TYPE_ID
#define PALO_ERR_CLIENT_INV_NET_REPLY           (palo_err)PALO_ERR_CLIENT_INV_NET_REPLY_ID
#define PALO_ERR_CLIENT_SERVER_RETURNED_ERROR   (palo_err)-75
#define PALO_ERR_CACHE_ENTRY_NOT_FOUND          (palo_err)-76
#define PALO_ERR_CACHE_GOT_ERROR                (palo_err)-77
#define PALO_ERR_CACHE                          (palo_err)-78
#define PALO_ERR_SERVER_VERSION                 (palo_err)-79
#define PALO_ERR_CONSOLE                        (palo_err)-80
#define PALO_ERR_SYS_TIME                       (palo_err)-81
#define PALO_ERR_INTEGER_OVERFLOW               (palo_err)-82
#define PALO_ERR_BASE64                         (palo_err)-83
#define PALO_ERR_FREE_EXCEL_IN_USE              (palo_err)-84
#define PALO_ERR_FREE_EXCEL_NOT_ADMIN_ONLY		(palo_err)-85
#define PALO_ERR_FREE_EXCEL_ADVANCED_MODEL      (palo_err)PALO_ERR_FREE_EXCEL_ADVANCED_MODEL_ID
#define PALO_ERR_FREE_AND_PREMIUM_NOT_POSSIBLE  (palo_err)-87

#define MSG_PALO_ERR_NO_MEM                     wcsdup(L"memory allocation failed")
#define MSG_PALO_ERR_INVALID_POINTER            wcsdup(L"NULL pointer delivered")
#define MSG_PALO_ERR_DIM_ELEMENT_NOT_FOUND      wcsdup(L"could not find dimension element \"%ls\" in dimension \"%ls\"")
#define MSG_PALO_ERR_DIM_NOT_ALL_ELEMENTS_FOUND wcsdup(L"could not find any dimension containing all the elements given")
#define MSG_PALO_ERR_EMPTY_CUBE                 wcsdup(L"cube does not contain any dimensions")
#define MSG_PALO_ERR_DIM_EMPTY                  wcsdup(L"dimension does not contain any elements")
#define MSG_ENCODING_FAILED                     wcsdup(L"encoding conversion failed")
#define MSG_NO_STANDARD_EXCEPTION_OCCURED       wcsdup(L"non standard exception occured")
#define MSG_CONNECT_FAILED                      wcsdup(L"connect failed")
#define MSG_ERROR_NUMBER_DIM_IN_CUBE            wcsdup(L"There's a difference in the number of dimension of the cube")
#define MSG_TYPE_CONVERSION_FAILED              wcsdup(L"type conversion failed")
#define MSG_INVALID_INDEX						wcsdup(L"invalid index: %u + %d")
#define MSG_INVALID_OFFSET						wcsdup(L"invalid offset: %d")
#define MSG_DUPLICATES_IN_DIMS					wcsdup(L"duplicate names in dimensions \"%ls\" and \"%ls\"")


// New Error Codes

#define PALO_ERR_NEWCODES(nr)                   (palo_err)(nr)

#define HAS_ERROR_H
#endif
