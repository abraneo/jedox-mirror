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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include "const.h"
#include "stdaliases.h"
#include "strtools.h"
#define ARG_H_NO_REDIRECT
#include "arg.h"				 /* don't map arg_consolidation_element_info to consolidation_info_w etc. */
#undef ARG_H_NO_REDIRECT
#include "error.h"

void free_arg_error_contents(struct arg_error *err) {
	assert(err != NULL);
}


#define PALO_ARG_M
#define PALO_SUFFIX_STR                         _m
#define PALO_SUFFIX2_STR                        Utf8
#define PALO_ARG_FUNC(f)                        f##_m
#define palo_arg                                palo_arg_m
#define palo_arg_multi                          palo_arg_multi_m
#define PALO_STRING(s)                          s
#define CHAR_T                                  char
#define mystrdup                                strdup
#define mystrdupf                               strdupf
#include "generic_mappings.h"
#include "arg_generic.c.h"
#undef arg_getdata_export_result
#undef arg_getdata_export_options
#undef arg_palo_dataset_array
#undef arg_palo_dataset
#undef arg_str_array_2d
#undef arg_str_array_array
#undef arg_palo_value_array
#undef arg_palo_value
#undef arg_consolidation_element_info_array
#undef arg_dim_element_info_array
#undef arg_str_array
#undef arg_consolidation_element_info
#undef arg_dim_element_info
#undef mystrdup
#undef mystrdupf
#undef PALO_ARG_FUNC
#undef palo_arg
#undef palo_arg_multi
#undef PALO_STRING
#undef CHAR_T
#undef PALO_SUFFIX2_STR
#undef PALO_SUFFIX_STR
#undef PALO_ARG_M

#define PALO_ARG_A
#define PALO_SUFFIX_STR                         _a
#define PALO_SUFFIX2_STR                        C
#define PALO_ARG_FUNC(f)                        f##_a
#define palo_arg                                palo_arg_a
#define palo_arg_multi                          palo_arg_multi_a
#define PALO_STRING(s)                          s
#define CHAR_T                                  char
#define PALO_ARG_TO_M                           palo_arg_a_to_m
#define PALO_ARG_FROM_M                         palo_arg_m_to_a
#define PALO_ARG_TO_M_CONV                      mbs2utf8
#define PALO_ARG_FROM_M_CONV                    utf82mbs
#define PALO_ARGS_TO_M                          palo_args_a_to_m
#define PALO_ARGS_FROM_M                        palo_args_m_to_a
#define mystrdup                                strdup
#define mystrdupf                               strdupf
#define arg_consolidation_element_info          arg_consolidation_element_info_a
#define arg_dim_element_info                    arg_dim_element_info_a
#define arg_str_array                           arg_str_array_a
#define arg_consolidation_element_info_array    arg_consolidation_element_info_array_a
#define arg_dim_element_info_array              arg_dim_element_info_array_a
#define arg_palo_value                          arg_palo_value_a
#define arg_str_array_2d                        arg_str_array_2d_a
#define arg_str_array_array                     arg_str_array_array_a
#define arg_palo_value_array                    arg_palo_value_array_a
#define arg_palo_dataset                        arg_palo_dataset_a
#define arg_palo_dataset_array                  arg_palo_dataset_array_a
#define arg_getdata_export_options              arg_getdata_export_options_a
#define arg_getdata_export_result               arg_getdata_export_result_a
#include "arg_generic.c.h"
#undef arg_getdata_export_result
#undef arg_getdata_export_options
#undef arg_palo_dataset_array
#undef arg_palo_dataset
#undef arg_str_array_2d
#undef arg_str_array_array
#undef arg_palo_value_array
#undef arg_palo_value
#undef arg_consolidation_element_info_array
#undef arg_dim_element_info_array
#undef arg_str_array
#undef arg_consolidation_element_info
#undef arg_dim_element_info
#undef mystrdupf
#undef mystrdup
#undef PALO_ARG_TO_M_CONV
#undef PALO_ARG_FROM_M_CONV
#undef PALO_ARG_FUNC
#undef palo_arg
#undef palo_arg_multi
#undef PALO_STRING
#undef CHAR_T
#undef PALO_ARG_TO_M
#undef PALO_ARG_FROM_M
#undef PALO_ARGS_TO_M
#undef PALO_ARGS_FROM_M
#undef PALO_SUFFIX2_STR
#undef PALO_SUFFIX_STR
#undef PALO_ARG_A

#define PALO_ARG_W
#define PALO_SUFFIX_STR                         _w
#define PALO_SUFFIX2_STR                        W
#define PALO_ARG_FUNC(f)                        f##_w
#define palo_arg                                palo_arg_w
#define palo_arg_multi                          palo_arg_multi_w
#define PALO_STRING(s)                          L##s
#define CHAR_T                                  wchar_t
#define mystrdup                                wcsdup
#define mystrdupf                               wcsdupf
#define PALO_ARG_TO_M                           palo_arg_w_to_m
#define PALO_ARG_FROM_M                         palo_arg_m_to_w
#define PALO_ARG_TO_M_CONV                      wcs2utf8
#define PALO_ARG_FROM_M_CONV                    utf82wcs
#define PALO_ARGS_TO_M                          palo_args_w_to_m
#define PALO_ARGS_FROM_M                        palo_args_m_to_w
#define arg_consolidation_element_info          arg_consolidation_element_info_w
#define arg_dim_element_info                    arg_dim_element_info_w
#define arg_str_array                           arg_str_array_w
#define arg_consolidation_element_info_array    arg_consolidation_element_info_array_w
#define arg_dim_element_info_array              arg_dim_element_info_array_w
#define arg_palo_value                          arg_palo_value_w
#define arg_str_array_2d                        arg_str_array_2d_w
#define arg_str_array_array                     arg_str_array_array_w
#define arg_palo_value_array                    arg_palo_value_array_w
#define arg_palo_dataset                        arg_palo_dataset_w
#define arg_palo_dataset_array                  arg_palo_dataset_array_w
#define arg_getdata_export_options              arg_getdata_export_options_w
#define arg_getdata_export_result               arg_getdata_export_result_w
#include "arg_generic.c.h"
#undef arg_getdata_export_result
#undef arg_getdata_export_options
#undef arg_palo_dataset_array
#undef arg_str_array_2d
#undef arg_palo_dataset
#undef arg_str_array_array
#undef arg_palo_value_array
#undef arg_palo_value
#undef arg_consolidation_element_info_array
#undef arg_dim_element_info_array
#undef arg_str_array
#undef arg_consolidation_element_info
#undef arg_dim_element_info
#undef PALO_ARG_TO_M_CONV
#undef PALO_ARG_FROM_M_CONV
#undef PALO_ARG_FUNC
#undef palo_arg
#undef palo_arg_multi
#undef PALO_STRING
#undef CHAR_T
#undef mystrdup
#undef mystrdupf
#undef PALO_ARG_TO_M
#undef PALO_ARG_FROM_M
#undef PALO_ARGS_TO_M
#undef PALO_ARGS_FROM_M
#undef PALO_SUFFIX2_STR
#undef PALO_SUFFIX_STR
#undef PALO_ARG_W
