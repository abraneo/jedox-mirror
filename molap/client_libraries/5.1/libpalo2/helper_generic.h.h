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

palo_err PALO_SUFFIX(database_info)(struct errstack *errs, struct arg_db_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(dimension_info)(struct errstack *errs, struct arg_dim_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(dimension_info_simple)(struct errstack *errs, struct arg_dim_info_simple *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(element_info_simple)(struct errstack *errs, struct arg_dim_element_info2_raw *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *element);
palo_err PALO_SUFFIX(cube_info)(struct errstack *errs, struct arg_cube_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(database_list_cubes)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(database_list_cubes2)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int cube_type, unsigned short int only_cubes_with_cells);
palo_err PALO_SUFFIX(cube_list_dimensions)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(database_list_dimensions)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(database_list_dimensions2)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, int dim_type);
palo_err PALO_SUFFIX(dimension_list_cubes)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(dimension_list_cubes2)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int cube_type, unsigned short int only_cubes_with_cells);
palo_err PALO_SUFFIX(root_list_databases)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(root_list_databases2)(struct errstack *errs, struct arg_str_array *result, struct sock_obj *so, struct conversions *convs, int db_type);
palo_err PALO_SUFFIX(get_database_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(get_dimension_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(get_cube_type)(struct errstack *errs, unsigned int *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(get_attribute_dimension)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(get_attribute_cube)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(get_rights_cube)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(erename)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_oldname, const CHAR_T *de_newname);
palo_err PALO_SUFFIX(edelete)(struct errstack *errs,  struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(emove)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, unsigned int new_position);
palo_err PALO_SUFFIX(dimension_list_elements)(struct errstack *errs, struct arg_dim_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(dimension_list_elements2_raw)(struct errstack *errs, struct arg_dim_element_info2_raw_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(dimension_restricted_flat_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
palo_err PALO_SUFFIX(dimension_restricted_top_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long start, long limit);
palo_err PALO_SUFFIX(dimension_restricted_children_list_elements)(struct errstack *errs, struct arg_dim_element_info2_raw_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, long element_identifier, long start, long limit);
palo_err PALO_SUFFIX(eadd_or_update)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, dimension_add_or_update_element_mode mode, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements, palo_bool append_consolidation_elements);
palo_err PALO_SUFFIX(element_create)(struct errstack *errs, struct arg_dim_element_info2_raw *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, de_type type, const struct arg_consolidation_element_info_array *consolidation_elements);
palo_err PALO_SUFFIX(element_list_consolidation_elements)(struct errstack *errs, struct arg_consolidation_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(etoplevel)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(elevel)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(eweight)(struct errstack *errs, double *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(etype)(struct errstack *errs, de_type *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(eprev)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(enext)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(ename)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, size_t n);
palo_err PALO_SUFFIX(eindex)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(efirst)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(ecount)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(eparentcount)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(echildcount)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(cube_commit_log)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(cube_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(cube_unload)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(cube_load)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(dimension_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(database_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(database_unload)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(database_load)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(server_shutdown)(struct errstack *errs, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(database_rename_dimension)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension_oldname, const CHAR_T *dimension_newname);
palo_err PALO_SUFFIX(database_rename_cube)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube_oldname, const CHAR_T *cube_newname);
palo_err PALO_SUFFIX(database_add_dimension)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(database_add_dimension2)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, dim_type type);
palo_err PALO_SUFFIX(database_add_cube)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *dimensions);
palo_err PALO_SUFFIX(database_add_cube2)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *dimensions, cube_type type);
palo_err PALO_SUFFIX(root_add_database)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(ping)(struct errstack *errs, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(init_ssl)(struct errstack *errs, struct conversions *convs, const CHAR_T *trustfile);
palo_err PALO_SUFFIX(auth_ssl)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile, unsigned short use_prefered_ssl);
palo_err PALO_SUFFIX(test_connection)(struct errstack *errs, struct conversions *convs, const CHAR_T *hostname, unsigned int service, const CHAR_T *username, const CHAR_T *pw_hash, const CHAR_T *trustfile);
palo_err PALO_SUFFIX(check_validity)(struct errstack *errs, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(database_save)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database);
palo_err PALO_SUFFIX(element_list_consolidation_elements)(struct errstack *errs, struct arg_consolidation_element_info_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(eindent)(struct errstack *errs, unsigned int *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element);
palo_err PALO_SUFFIX(eweight)(struct errstack *errs, double *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
palo_err PALO_SUFFIX(eischild)(struct errstack *errs,  palo_bool *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *de_parent, const CHAR_T *de_child);
palo_err PALO_SUFFIX(eparentname)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n);
palo_err PALO_SUFFIX(echildname)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, size_t n);
palo_err PALO_SUFFIX(esibling)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const CHAR_T *dimension_element, int n);
palo_err PALO_SUFFIX(edimension)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, struct arg_str_array *dimension_elements, palo_bool should_be_unique);
palo_err PALO_SUFFIX(ecubedimension)(struct errstack *errs, CHAR_T **result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, struct arg_str_array *dimension_elements, palo_bool should_be_unique);
palo_err PALO_SUFFIX(dimension_clear)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(getdata)(struct errstack *errs, struct arg_palo_value *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates);
palo_err PALO_SUFFIX(getdata_area)(struct errstack *errs, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *coordinates);
palo_err PALO_SUFFIX(getdata_area2)(struct errstack *errs, struct arg_palo_dataset_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *coordinates);
palo_err PALO_SUFFIX(getdata_multi)(struct errstack *errs, struct arg_palo_value_array *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_2d *coordinates);
palo_err PALO_SUFFIX(setdata)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset *ds, splash_mode mode);
palo_err PALO_SUFFIX(setdata_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset *ds, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
palo_err PALO_SUFFIX(setdata_multi)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array *dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor);
palo_err PALO_SUFFIX(setdata_multi_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_palo_dataset_array *dsa, splash_mode mode, unsigned short add, unsigned short eventprocessor, const struct arg_str_array_2d *locked_coordinates);
palo_err PALO_SUFFIX(getdata_export)(struct errstack *errs, struct arg_getdata_export_result *result, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *export_area, struct arg_getdata_export_options *options);
palo_err PALO_SUFFIX(getdata_export_rule)(struct errstack *errs, struct arg_getdata_export_result *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array_array *export_area, struct arg_getdata_export_options *options, unsigned short use_rules);
palo_err PALO_SUFFIX(cube_clear)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short complete, const struct arg_str_array_array *elements);
palo_err PALO_SUFFIX(cell_copy)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, unsigned short withValue, double val);
palo_err PALO_SUFFIX(cell_copy_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, cell_copy_mode mode, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *from, const struct arg_str_array *to, const struct arg_str_array_array *predict_area, const struct arg_str_array_2d *locked_coordinates, double* val, unsigned short use_rules);
palo_err PALO_SUFFIX(event_lock_begin)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, CHAR_T *source, CHAR_T *AreaID);
palo_err PALO_SUFFIX(event_lock_end)(struct errstack *errs, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(server_info)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, struct server_info *retresult);
palo_err PALO_SUFFIX(api_version)(struct errstack *errs, struct api_info *retresult);
palo_err PALO_SUFFIX(rule_add)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
palo_err PALO_SUFFIX(rule_modify)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, const CHAR_T *definition, unsigned short use_identifier, const CHAR_T *extern_id, const CHAR_T *comment, unsigned short activate, double position);
palo_err PALO_SUFFIX(rules_activate)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, rule_activation_mode mode);
palo_err PALO_SUFFIX(rule_info)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long id, unsigned short use_identifier);
palo_err PALO_SUFFIX(cell_rule_info)(struct errstack *errs, struct arg_rule_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array *coordinates);
palo_err PALO_SUFFIX(rule_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, long identifier);
palo_err PALO_SUFFIX(rules_delete)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids);
palo_err PALO_SUFFIX(rules_move)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, size_t len, long* ids, double start_position, double below_position);
palo_err PALO_SUFFIX(list_rules)(struct errstack *errs, struct arg_rule_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned short use_identifier);
palo_err PALO_SUFFIX(list_rulefunctions)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(rule_parse)(struct errstack *errs, CHAR_T **retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const CHAR_T *definition);
palo_err PALO_SUFFIX(cube_lock)(struct errstack *errs, struct arg_lock_info *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube,  const struct arg_str_array_array *locked_area);
palo_err PALO_SUFFIX(cube_locks)(struct errstack *errs, struct arg_lock_info_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube);
palo_err PALO_SUFFIX(cube_rollback)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid, long steps);
palo_err PALO_SUFFIX(cube_commit)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, unsigned long lockid);
palo_err PALO_SUFFIX(dimension_subset)(struct errstack *errs, struct arg_subset_result_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, int indent,
										struct arg_alias_filter_settings *alias,
										struct arg_field_filter_settings *field,
										struct arg_basic_filter_settings *basic,
										struct arg_data_filter_settings *data,
										struct arg_sorting_filter_settings *sorting,
										struct arg_structural_filter_settings *structural,
										struct arg_text_filter_settings *text);
palo_err PALO_SUFFIX(cell_drill_through)(struct errstack *errs, struct arg_str_array_array *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *coordinates, drillthrough_type mode);
palo_err PALO_SUFFIX(edelete_bulk)(struct errstack *errs,  struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, const arg_str_array *dimension_elements);
palo_err PALO_SUFFIX(get_svs_info)(struct errstack *errs, struct arg_svs_info *retresult, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(cell_goalseek)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val);
palo_err PALO_SUFFIX(cell_goalseek_extended)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, const struct arg_str_array *path, double val, goalseek_mode mode, const struct arg_str_array_array *goalseek_area);
palo_err PALO_SUFFIX(ecreate_bulk_simple)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension, de_type type, const struct arg_str_array *elements, const struct arg_consolidation_element_info_array_array *consolidation_elements);
palo_err PALO_SUFFIX(cube_convert)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *cube, cube_type newtype);
palo_err PALO_SUFFIX(set_user_password)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *user, const CHAR_T *password);
palo_err PALO_SUFFIX(change_password)(struct errstack *errs, struct sock_obj *so, struct conversions *convs, const CHAR_T *oldpassword, const CHAR_T *newpassword);
palo_err PALO_SUFFIX(dimension_flat_count)(struct errstack *errs, size_t *retresult, struct sock_obj *so, struct conversions *convs, const CHAR_T *database, const CHAR_T *dimension);
palo_err PALO_SUFFIX(set_client_version)(struct errstack *errs, struct conversions *convs, const CHAR_T *client_version);
palo_err PALO_SUFFIX(is_admin_user)(struct errstack *errs, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(is_gpu_server)(struct errstack *errs, unsigned short int *retresult, struct sock_obj *so, struct conversions *convs);
palo_err PALO_SUFFIX(get_rights)(struct errstack *errs, permission_type *retresult, struct sock_obj *so, struct conversions *convs, permission_art pa);
