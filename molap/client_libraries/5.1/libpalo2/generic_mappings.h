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

#ifndef HAVE_GENERIC_MAPPINGS_H
#define HAVE_GENERIC_MAPPINGS_H

#define CONCAT2(a, b)								a ## b
#define CONCAT(a, b)								CONCAT2(a, b)
#define PALO_SUFFIX(v)						        CONCAT(v, PALO_SUFFIX_STR)
#define PALO_SUFFIX2(v)							    CONCAT(v, PALO_SUFFIX2_STR)

#define LIBPALO_FUNC(v)								PALO_SUFFIX(CONCAT(palo_, v))
#define LIBPALO_FUNC2(v)	                        PALO_SUFFIX2(CONCAT(Palo, v))
#define LIBPALO_FUNC_R(v)		                    CONCAT(LIBPALO_FUNC(v), _r)
#define LIBPALO_LIBFUNC(v)			                PALO_SUFFIX(CONCAT(libpalo_, v))
#define LIBPALO_LIBFUNC_R(v)			            CONCAT(PALO_SUFFIX(CONCAT(libpalo_, v)), _r)
#define arg_parent_info_raw						    PALO_SUFFIX(arg_parent_info_raw)
#define arg_child_info_raw							PALO_SUFFIX(arg_child_info_raw)
#define arg_dim_element_info						PALO_SUFFIX(arg_dim_element_info)
#define arg_dim_element_info2_raw				    PALO_SUFFIX(arg_dim_element_info2_raw)
#define arg_consolidation_element_info				PALO_SUFFIX(arg_consolidation_element_info)
#define arg_consolidation_element_info2				PALO_SUFFIX(arg_consolidation_element_info2)
#define arg_dim_element_info_array					PALO_SUFFIX(arg_dim_element_info_array)
#define arg_dim_element_info2_raw_array				PALO_SUFFIX(arg_dim_element_info2_raw_array)
#define arg_consolidation_element_info_array		PALO_SUFFIX(arg_consolidation_element_info_array)
#define arg_consolidation_element_info_array_array	PALO_SUFFIX(arg_consolidation_element_info_array_array)
#define arg_info_array								PALO_SUFFIX(arg_info_array)
#define arg_str_array								PALO_SUFFIX(arg_str_array)
#define arg_str_array_2d							PALO_SUFFIX(arg_str_array_2d)
#define arg_str_array_array							PALO_SUFFIX(arg_str_array_array)
#define arg_palo_value								PALO_SUFFIX(arg_palo_value)
#define arg_palo_value_rule							PALO_SUFFIX(arg_palo_value_rule)
#define arg_palo_value_array						PALO_SUFFIX(arg_palo_value_array)
#define arg_palo_value_rule_array					PALO_SUFFIX(arg_palo_value_rule_array)
#define arg_palo_dataset							PALO_SUFFIX(arg_palo_dataset)
#define arg_palo_dataset_rule						PALO_SUFFIX(arg_palo_dataset_rule)
#define arg_palo_dataset_array						PALO_SUFFIX(arg_palo_dataset_array)
#define arg_palo_dataset_rule_array					PALO_SUFFIX(arg_palo_dataset_rule_array)
#define arg_getdata_export_options					PALO_SUFFIX(arg_getdata_export_options)
#define arg_getdata_export_result					PALO_SUFFIX(arg_getdata_export_result)
#define arg_rule_info								PALO_SUFFIX(arg_rule_info)
#define arg_rule_info_array							PALO_SUFFIX(arg_rule_info_array)
#define arg_lock_info								PALO_SUFFIX(arg_lock_info)
#define arg_lock_info_array							PALO_SUFFIX(arg_lock_info_array)
#define arg_svs_info								PALO_SUFFIX(arg_svs_info)
#define arg_cube_info								PALO_SUFFIX(arg_cube_info)
#define arg_db_info									PALO_SUFFIX(arg_db_info)
#define arg_dim_info								PALO_SUFFIX(arg_dim_info)
#define arg_dim_info_simple							PALO_SUFFIX(arg_dim_info_simple)
#define arg_element_info							PALO_SUFFIX(arg_element_info)
#define arg_alias_filter_settings					PALO_SUFFIX(arg_alias_filter_settings)
#define arg_field_filter_settings					PALO_SUFFIX(arg_field_filter_settings)
#define arg_basic_filter_settings					PALO_SUFFIX(arg_basic_filter_settings)
#define arg_data_filter_settings					PALO_SUFFIX(arg_data_filter_settings)
#define arg_sorting_filter_settings					PALO_SUFFIX(arg_sorting_filter_settings)
#define arg_structural_filter_settings				PALO_SUFFIX(arg_structural_filter_settings)
#define arg_text_filter_settings					PALO_SUFFIX(arg_text_filter_settings)
#define arg_bool_str_array							PALO_SUFFIX(arg_bool_str_array)
#define arg_subset_result							PALO_SUFFIX(arg_subset_result)
#define arg_subset_result_array						PALO_SUFFIX(arg_subset_result_array)
#endif	
