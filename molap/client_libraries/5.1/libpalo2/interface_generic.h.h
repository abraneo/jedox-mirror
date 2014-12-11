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

LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata)(libpalo_err *err, struct arg_palo_value *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_area)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_area2)(libpalo_err *err, struct arg_palo_dataset_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_multi)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_multi)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(setdata_multi_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(init_ssl)(libpalo_err *err, struct conversions *convs, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(auth)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw_hash);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(auth_ssl)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(test_connection)(libpalo_err *err, struct conversions *convs, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(check_validity)(libpalo_err *err, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ping)(libpalo_err *err, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_add_database)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_dimension)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_dimension2)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, dim_type type);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_rename_cube)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_rename_dimension)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_cube)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_add_cube2)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions, cube_type type);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_load)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(server_shutdown)(libpalo_err *err, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_unload)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_clear)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_load)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_unload)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_commit_log)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_save)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(echildname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(echildcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eparentcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eparentname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(efirst)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eindex)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eischild)(libpalo_err *err, palo_bool *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ename)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, size_t n);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(edimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, struct arg_str_array dimension_elements, palo_bool should_be_unique);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecubedimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array dimension_elements, palo_bool should_be_unique);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(enext)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eprev)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(etype)(libpalo_err *err, de_type *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(esibling)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eweight)(libpalo_err *err, double *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(elevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eindent)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(etoplevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_list_databases)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(root_list_databases2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, int db_type);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_list_dimensions2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int dim_type);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_list_consolidation_elements)(libpalo_err *err, struct arg_consolidation_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_elements)(libpalo_err *err, struct arg_dim_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_list_elements2_raw)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_flat_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_top_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_restricted_children_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(erename)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(edelete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(eadd_or_update)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array consolidation_elements, palo_bool append_consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_create)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(ecreate_bulk_simple)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *dim_elements, const struct arg_consolidation_element_info_array_array *consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_convert)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, cube_type newtype);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(emove)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_export)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(getdata_export_rule)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options, unsigned short use_rules);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_clear)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array elements);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_database_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_dimension_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_cube_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_attribute_dimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_attribute_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_rights_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_copy)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_copy_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_goalseek)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_goalseek_extended)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(server_info)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, struct server_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(api_version)(libpalo_err *err, struct api_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(database_info)(libpalo_err *err, struct arg_db_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_info)(libpalo_err *err, struct arg_dim_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_info_simple)(libpalo_err *err, struct arg_dim_info_simple *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(element_info_simple)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_info)(libpalo_err *err, struct arg_cube_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(set_client_version)(libpalo_err *err, struct conversions *convs, const CHAR_T *client_version);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(is_admin_user)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(is_gpu_server)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_rights)(libpalo_err *err, permission_type *retresult, struct sock_obj *so, struct conversions *convs, permission_art pa);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(set_user_password)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *user, const CHAR_T *password);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(change_password)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *oldpassword, const CHAR_T *newpassword);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(lib_version)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, struct api_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(event_lock_begin)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, CHAR_T *source, CHAR_T *AreaID);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(event_lock_end)(libpalo_err *err, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_add)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_modify)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_activate)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_parse)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rule_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_delete)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(rules_move)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, double start_position, double below_position);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(list_rules)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(list_rulefunctions)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_lock)(libpalo_err *err, struct arg_lock_info *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array locked_area);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_locks)(libpalo_err *err, struct arg_lock_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_rollback)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cube_commit)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(get_svs_info)(libpalo_err *err, struct arg_svs_info *result, struct sock_obj *so, struct conversions *convs);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_subset)(libpalo_err *err, struct arg_subset_result_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int indent,
															struct arg_alias_filter_settings *alias,
															struct arg_field_filter_settings *field,
															struct arg_basic_filter_settings *basic,
															struct arg_data_filter_settings *data,
															struct arg_sorting_filter_settings *sorting,
															struct arg_structural_filter_settings *structural,
															struct arg_text_filter_settings *text);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(cell_drill_through)(libpalo_err *err, struct arg_str_array_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(edelete_bulk)(libpalo_err *err, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const struct arg_str_array *dimension_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC_R(dimension_flat_count)(libpalo_err *err, size_t *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);

LIBEXPORT libpalo_result LIBPALO_LIBFUNC_R(cache_add_request)(libpalo_err *err, libpalo_cache *c, struct sock_obj *so, struct conversions *convs, const CHAR_T *key, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_LIBFUNC_R(cache_get_result)(libpalo_err *err, libpalo_cache *c, struct conversions *convs, const CHAR_T *key, struct arg_palo_value *value);

LIBEXPORT libpalo_result LIBPALO_FUNC_R(connect)(libpalo_err *err, struct conversions *convs, const CHAR_T *hostname, const CHAR_T *service, struct sock_obj *so);

LIBEXPORT CHAR_T *LIBPALO_LIBFUNC_R(err_get_string)(struct conversions *convs, const libpalo_err *err);

LIBEXPORT libpalo_result LIBPALO_FUNC(etoplevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata)(libpalo_err *err, struct arg_palo_value *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_area)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_area2)(libpalo_err *err, struct arg_palo_dataset_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array coordinates, palo_bool guess_coordinates_order);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_multi)(libpalo_err *err, struct arg_palo_value_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode);
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_multi)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor);
LIBEXPORT libpalo_result LIBPALO_FUNC(setdata_multi_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC(elevel)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(eindent)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(eweight)(libpalo_err *err, double *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_clear)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(esibling)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n);
LIBEXPORT libpalo_result LIBPALO_FUNC(etype)(libpalo_err *err, de_type *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(eprev)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(enext)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(eparentname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n);
LIBEXPORT libpalo_result LIBPALO_FUNC(eparentcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(ename)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, int n);
LIBEXPORT libpalo_result LIBPALO_FUNC(eischild)(libpalo_err *err, palo_bool *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
LIBEXPORT libpalo_result LIBPALO_FUNC(eindex)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(efirst)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(ecount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(echildcount)(libpalo_err *err, unsigned int *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(echildname)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_save)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_commit_log)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_unload)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_load)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_unload)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_load)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(server_shutdown)(libpalo_err *err, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(init_ssl)(libpalo_err *err, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC(auth)(libpalo_err *err, struct sock_obj *so, const CHAR_T *username, const CHAR_T *pw_hash);
LIBEXPORT libpalo_result LIBPALO_FUNC(auth_ssl)(libpalo_err *err, struct sock_obj *so, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC(test_connection)(libpalo_err *err, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile);
LIBEXPORT libpalo_result LIBPALO_FUNC(check_validity)(libpalo_err *err, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(ping)(libpalo_err *err, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(root_list_databases)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(root_list_databases2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, int db_type);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_list_dimensions2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, int dim_type);
LIBEXPORT libpalo_result LIBPALO_FUNC(element_list_consolidation_elements)(libpalo_err *err, struct arg_consolidation_element_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_elements)(libpalo_err *err, struct arg_dim_element_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_elements2_raw)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_flat_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_top_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_restricted_children_list_elements)(libpalo_err *err, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_list_dimensions)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_cubes)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_list_cubes2)(libpalo_err *err, struct arg_str_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells);
LIBEXPORT libpalo_result LIBPALO_FUNC(erename)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC(edelete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
LIBEXPORT libpalo_result LIBPALO_FUNC(eadd_or_update)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array consolidation_elements, palo_bool append_consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC(element_create)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC(ecreate_bulk_simple)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *dim_elements, const struct arg_consolidation_element_info_array_array *consolidation_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_convert)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, cube_type newtype);
LIBEXPORT libpalo_result LIBPALO_FUNC(emove)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position);
LIBEXPORT libpalo_result LIBPALO_FUNC(edimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, struct arg_str_array dimension_elements, palo_bool should_be_unique);
LIBEXPORT libpalo_result LIBPALO_FUNC(ecubedimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array dimension_elements, palo_bool should_be_unique);
LIBEXPORT libpalo_result LIBPALO_FUNC(root_add_database)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_dimension)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_dimension2)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, dim_type type);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_rename_cube)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_rename_dimension)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_cube)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_add_cube2)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array dimensions, cube_type type);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_export)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options);
LIBEXPORT libpalo_result LIBPALO_FUNC(getdata_export_rule)(libpalo_err *err, struct arg_getdata_export_result *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array export_area, struct arg_getdata_export_options options, unsigned short use_rules);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_clear)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array elements);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_database_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_dimension_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_cube_type)(libpalo_err *err, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_attribute_dimension)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_attribute_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_rights_cube)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_copy)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_copy_extended)(libpalo_err *err, struct sock_obj *so, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_goalseek)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_goalseek_extended)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area);
LIBEXPORT libpalo_result LIBPALO_FUNC(server_info)(libpalo_err *err, struct sock_obj *so, struct server_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC(api_version)(libpalo_err *err, struct api_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC(lib_version)(libpalo_err *err, struct sock_obj *so, struct api_info *result);
LIBEXPORT libpalo_result LIBPALO_FUNC(event_lock_begin)(libpalo_err *err, struct sock_obj *so, CHAR_T *source, CHAR_T *AreaID);
LIBEXPORT libpalo_result LIBPALO_FUNC(event_lock_end)(libpalo_err *err, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(database_info)(libpalo_err *err, struct arg_db_info *result, struct sock_obj *so, const CHAR_T *database);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_info)(libpalo_err *err, struct arg_dim_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_info_simple)(libpalo_err *err, struct arg_dim_info_simple *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension);
LIBEXPORT libpalo_result LIBPALO_FUNC(element_info_simple)(libpalo_err *err, struct arg_dim_element_info2_raw *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_info)(libpalo_err *err, struct arg_cube_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(set_client_version)(libpalo_err *err, const CHAR_T *client_version);
LIBEXPORT libpalo_result LIBPALO_FUNC(is_admin_user)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(is_gpu_server)(libpalo_err *err, unsigned short int *retresult, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_rights)(libpalo_err *err, permission_type *retresult, struct sock_obj *so, permission_art pa);
LIBEXPORT libpalo_result LIBPALO_FUNC(set_user_password)(libpalo_err *err, struct sock_obj *so, const CHAR_T *user, const CHAR_T *password);
LIBEXPORT libpalo_result LIBPALO_FUNC(change_password)(libpalo_err *err, struct sock_obj *so, const CHAR_T *oldpassword, const CHAR_T *newpassword);
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_add)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_modify)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
LIBEXPORT libpalo_result LIBPALO_FUNC(rules_activate)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode);
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_rule_info)(libpalo_err *err, struct arg_rule_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_parse)(libpalo_err *err, CHAR_T **result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition);
LIBEXPORT libpalo_result LIBPALO_FUNC(rule_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC(rules_delete)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long identifier, size_t len, long* ids);
LIBEXPORT libpalo_result LIBPALO_FUNC(rules_move)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, long identifier, size_t len, long* ids, double start_position, double below_position);
LIBEXPORT libpalo_result LIBPALO_FUNC(list_rules)(libpalo_err *err, struct arg_rule_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier);
LIBEXPORT libpalo_result LIBPALO_FUNC(list_rulefunctions)(libpalo_err *err, CHAR_T **result, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_lock)(libpalo_err *err, struct arg_lock_info *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array locked_area);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_locks)(libpalo_err *err, struct arg_lock_info_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_rollback)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps);
LIBEXPORT libpalo_result LIBPALO_FUNC(cube_commit)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid);
LIBEXPORT libpalo_result LIBPALO_FUNC(get_svs_info)(libpalo_err *err, struct arg_svs_info *result, struct sock_obj *so);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_subset)(libpalo_err *err, struct arg_subset_result_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int indent,
															struct arg_alias_filter_settings *alias,
															struct arg_field_filter_settings *field,
															struct arg_basic_filter_settings *basic,
															struct arg_data_filter_settings *data,
															struct arg_sorting_filter_settings *sorting,
															struct arg_structural_filter_settings *structural,
															struct arg_text_filter_settings *text);
LIBEXPORT libpalo_result LIBPALO_FUNC(cell_drill_through)(libpalo_err *err, struct arg_str_array_array *result, struct sock_obj *so, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode);
LIBEXPORT libpalo_result LIBPALO_FUNC(edelete_bulk)(libpalo_err *err, struct sock_obj *so, const CHAR_T *database, const CHAR_T *dimension, const struct arg_str_array *dimension_elements);
LIBEXPORT libpalo_result LIBPALO_FUNC(dimension_flat_count)(libpalo_err *err, size_t *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);

LIBEXPORT libpalo_result LIBPALO_LIBFUNC(cache_add_request)(libpalo_err *err, libpalo_cache *c, struct sock_obj *so, const CHAR_T *key, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array coordinates);
LIBEXPORT libpalo_result LIBPALO_LIBFUNC(cache_get_result)(libpalo_err *err, libpalo_cache *c, const CHAR_T *key, struct arg_palo_value *value);

LIBEXPORT libpalo_result LIBPALO_FUNC(connect)(libpalo_err *err, const CHAR_T *hostname, const CHAR_T *service, struct sock_obj *so);

LIBEXPORT const struct arg_str_array LIBPALO_LIBFUNC(make_arg_str_array)(size_t num_strs, const CHAR_T * const *strs);
LIBEXPORT const struct arg_str_array_array LIBPALO_LIBFUNC(make_arg_str_array_array)(size_t num_arrays, const struct arg_str_array *arrays);
LIBEXPORT const struct arg_consolidation_element_info_array LIBPALO_LIBFUNC(make_arg_consolidation_element_info_array)(size_t num_ceis, const struct arg_consolidation_element_info *ceis);
LIBEXPORT const struct arg_palo_value LIBPALO_LIBFUNC(make_arg_palo_value)(const CHAR_T *str_val, double dbl_val);
LIBEXPORT const struct arg_palo_dataset LIBPALO_LIBFUNC(make_arg_palo_dataset)(const struct arg_str_array coordinates, const struct arg_palo_value value);

LIBEXPORT CHAR_T *LIBPALO_LIBFUNC(err_get_string)(const libpalo_err *err);
LIBEXPORT palo_err LIBPALO_FUNC_R(_getdata_multi)(struct errstack *errs, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d *coordinates);

