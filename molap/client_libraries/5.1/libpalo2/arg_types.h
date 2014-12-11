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

#ifndef HAVE_ARG_TYPES_H
#define HAVE_ARG_TYPES_H

typedef enum db_type_tag
{
    normal_db = 0,
    system_db = 1,
    user_info_db = 3,
    unknown_db_type = -1
} db_type;

typedef enum db_status_tag
{
    unloaded_db = 0,
    loaded_db = 1,
    changed_db = 2,
    loading_db = 3,
    unknown_db_status = -1
} db_status;

typedef enum dim_type_tag
{
    normal_dim = 0,
    system_dim = 1,
    attribut_dim = 2,
    user_info_dim = 3,
	system_id_dim = 4,
    unknown_dim_type = -1
} dim_type;

typedef enum cube_type_tag
{
    normal_cube = 0,
    system_cube = 1,
    attribut_cube = 2,
    user_info_cube = 3,
    gpu_cube = 4,
    unknown_cube_type = -1
} cube_type;

typedef enum cube_status_tag
{
    unloaded_cube = 0,
    loaded_cube = 1,
    changed_cube = 2,
    unknown_cube_status = -1
} cube_status;

typedef enum element_type_tag
{
    unknown_element = 0,
    numeric_element = 1,
    string_element = 2,
    consolidated_element = 4
} element_type;

typedef enum de_type_tag
{
    de_n,
    de_s,
    de_c,
    de_r,
    de_unknown
} de_type;

typedef enum dimension_add_or_update_element_mode_tag
{
    dimension_add_or_update_element_mode_unspec,
    dimension_add_or_update_element_mode_add,
    dimension_add_or_update_element_mode_force_add,
    dimension_add_or_update_element_mode_update,
    dimension_add_or_update_element_mode_add_or_update
} dimension_add_or_update_element_mode;

typedef enum serialization_mode_tag
{
    serialize_text = 0,
    serialize_binary,
    serialize_xml
} serialization_mode;

typedef enum element_info_type_tag
{
	ei_unknown = 0,
	ei_numeric = 1,
	ei_string = 2,
	ei_consolidated = 4
} element_info_type;

typedef enum drillthrough_type_tag
{
	history = 1, 
	details = 2
} drillthrough_type;

typedef enum svs_login_mode_tag
{
	none,
	information,
	authentification,
	authorization
} svs_login_mode;

typedef enum rule_activation_mode_tag
{
	rule_mode_deactivate = 0,
	rule_mode_activate = 1,
	rule_mode_toggle = 2,
} rule_activation_mode;

typedef enum goalseek_mode_tag
{
	goalseek_complete,
	goalseek_equal,
	goalseek_relative
} goalseek_mode;


typedef enum cell_copy_mode_tag
{
	cell_copy_default,
	cell_copy_predict_linear_regression
} cell_copy_mode;

typedef enum permission_art_tag
{
    permission_art_user,
    permission_art_password,
    permission_art_group,
    permission_art_database,
    permission_art_cube,
    permission_art_dimension,
    permission_art_dimension_element,
    permission_art_cell_data,
    permission_art_rights,
    permission_art_system_operations,
    permission_art_event_processor,
    permission_art_sub_set_view,
    permission_art_user_info,
    permission_art_rule,
    permission_art_drillthrough,
    permission_art_ste_reports,
    permission_art_ste_files,
    permission_art_ste_palo,
    permission_art_ste_users,
    permission_art_ste_etl,
    permission_art_ste_conns,
    permission_art_ste_scheduler,
    permission_art_ste_logs,
    permission_art_ste_licenses,
    permission_art_ste_mobile,
    permission_art_ste_analyzer,
    permission_art_ste_sessions,
	permission_art_ste_settings
} permission_art;

typedef enum permission_type_tag
{
    permission_type_unknown,
    permission_type_none,
    permission_type_read,
    permission_type_write,
    permission_type_delete,
    permission_type_splash
} permission_type;

// Begin Subset
	
typedef	enum alias_filter_flags_tag
{
	search_one = 0x1, // search one attribute for an alias -- DEFAULT show this field in excel
	search_two = 0x2, // search two attributes for an alias -- show this field in excel
	hide_double = 0x4, // hide double aliases, aliases don't have to be unique !! (Makes not really sense)
	display_alias = 0x8,//Choose an attribute-dimension that serves as alias pool and show the aliases instead
	use_filter_exp = 0x10 // use advanced filter expressions for attribute-values
} alias_filter_flags;

typedef	enum data_filter_flags_tag
{
	data_min = 0x1, // use min operator on cell values
	data_max = 0x2, // use max operator on cell values
	data_sum = 0x4, // use sum operator on cell values
	data_average = 0x8, // use average operator on cell values
	data_any = 0x10, //conditions must be true for at least one value
	data_all = 0x20, //conditions must be true for all values
	data_string = 0x40, //the elements could then be sorted according to these strings or flags like TOP could be applied
	only_consildated = 0x80, // compute data only for consolidations (don't filter leaves)
	only_leaves	= 0x100, // compute data only for leaves (don't filter consolidations)
	upper_percentage = 0x200, // sort elements from highest to lowest and choose those that contribute the first p1% (set percentage method)
	lower_percentage = 0x400, // sort elements from lowest to highest and choose those that contribute the first p1% (set percentage method)
	mid_percentage = 0x800, // sort elements from highest to lowest and choose those that contribute p2% after removing the first elements that make up p1%
	top = 0x1000, //pick only the top-x elements. x set by set_top
	no_rules = 0x2000 //do not use rules when extracting cell values
} data_filter_flags;

typedef	enum pick_list_flags_tag
{
	insert_back = 0x1, // Put manually picked elements after the others. Default: before
	merge_elements = 0x2, // Put manually picked elements before the others
	sub = 0x4,	 // Use the picklist as a filter, not as a union.
	insert_front = 0x8 // Default value, put manually picked elements before the others
} pick_list_flags;

typedef	enum sorting_filter_flags_tag
{
	text = 0x1, // sort filtered elements according to name (default, not necessary to pass)
	numeric = 0x2, // sort filtered elements according to value computed by data-filter !! MIGHT BE STRING DATA !!
    use_attribute = 0x4, // sort according to an attribute (to be set separately)
	use_alias = 0x8, // sort according to aliases as determined by alias filter
	reverse_order = 0x10, // // show parents below their children
	leaves_only = 0x20, // do not sort consolidated elements
	standard_order = 0x40, // sort in order highest to lowest. (default, not necessary to pass)
	whole = 0x80, //show whole hierarchy
	position = 0x100, //position --default
	reverse_total = 0x200, //reverse the sorting
	sort_one_level = 0x400, //sort on the level of level-element
	flat_hierarchy = 0x800, //do NOT build a tree -- default
	no_children = 0x1000, // build a tree, but do not follow the children-list of an element that has been filtered out before
	consolidated_only = 0x2000, //only sort consolidations
	sort_not_one_level = 0x4000, // sort all levels except one
	show_duplicates  = 0x8000, //Show duplicates, default value is 0 (flag inactive - duplicates are hidden)
	reverse_total_ex = 0x10000, //this will completely reverse the ordering
	limit = 0x20000, //Limit number of elements returned
	consolidated_order = 0x40000, //Sort by consolidation order
	element_path = 0x80000 	//Return also path
} sorting_filter_flags;

typedef	enum structural_filter_flags_tag
{
	below_inclusive = 0x1, // choose elements below including element passed to set_bound
	below_exclusive = 0x2, // choose elements below excluding element passed to set_bound
	hide_consolidated = 0x4, // Remove all consolidated elements from set, show only leaves
	hide_leaves = 0x8, // Remove all non-consolidated elements, show aggregations only
	hierarchical_level = 0x10, // pick elements from top to bottom (levels to be specified separately)
	aggregated_level = 0x20, // pick elements from bottom to top (levels to be specified separately)
	revolving = 0x40, // revolve (repeat) the list, choose all elements on the same level
	revolve_add_above = 0x80, // add all elements with the same or a higher level than elemname and repeat the list
	revolve_add_below = 0x100, // add all elements with the same or a lower level than elemname and repeat the list
	above_exclusive = 0x200, // choose elements above excluding element passed to set_bound
    above_inclusive = 0x400, // choose elements above including element passed to set_bound
	cyclic = 0x800  //simply repeat the list without further filtering
} structural_filter_flags;

typedef	enum text_filter_flags_tag
{
	additional_fields = 0x1, // use additional columns for searching uses the attribute-columns of this dimension per default
	extended = 0x2 // do not use POSIX-Basic expressions but use Perl-Extended regular expressions
} text_filter_flags;

// End Subset

struct api_info {
	unsigned long major_version;
	unsigned long minor_version;
	unsigned long bugfix_version;
	unsigned long build_number;
};

struct server_info {
	unsigned long major_version;
	unsigned long minor_version;
	unsigned long bugfix_version;
	unsigned long build_number;
	unsigned long encryption;
	unsigned long https_port;
	unsigned long data_sequence_number;
};

struct arg_consolidation_element_info_w {
	wchar_t *name;
	double factor;
	de_type type;
};

struct arg_consolidation_element_info_a {
	char *name;
	double factor;
	de_type type;
};

struct arg_consolidation_element_info_m {
	char *name;
	double factor;
	de_type type;
};

struct arg_dim_element_info_w {
	wchar_t *name;
	de_type type;
};

struct arg_dim_element_info_a {
	char *name;
	de_type type;
};

struct arg_dim_element_info_m {
	char *name;
	de_type type;
};

struct arg_error {
	palo_err code;
};

struct arg_str_array_w {
	size_t len;
	wchar_t **a;
};

struct arg_str_array_m {
	size_t len;
	char **a;
};

struct arg_str_array_a {
	size_t len;
	char **a;
};

struct arg_str_array_array_w {
	size_t len;
	struct arg_str_array_w *a;
};

struct arg_str_array_array_m {
	size_t len;
	struct arg_str_array_m *a;
};

struct arg_str_array_array_a {
	size_t len;
	struct arg_str_array_a *a;
};

struct arg_str_array_2d_w {
	size_t rows;
	size_t cols;
	wchar_t **a;
};

struct arg_str_array_2d_m {
	size_t rows;
	size_t cols;
	char **a;
};

struct arg_str_array_2d_a {
	size_t rows;
	size_t cols;
	char **a;
};

struct arg_consolidation_element_info_array_w {
	size_t len;
	struct arg_consolidation_element_info_w *a;
};

struct arg_consolidation_element_info_array_m {
	size_t len;
	struct arg_consolidation_element_info_m *a;
};

struct arg_consolidation_element_info_array_a {
	size_t len;
	struct arg_consolidation_element_info_a *a;
};

struct arg_consolidation_element_info_array_array_w {
	size_t len;
	struct arg_consolidation_element_info_array_w *a;
};

struct arg_consolidation_element_info_array_array_m {
	size_t len;
	struct arg_consolidation_element_info_array_m *a;
};

struct arg_consolidation_element_info_array_array_a {
	size_t len;
	struct arg_consolidation_element_info_array_a *a;
};

struct arg_dim_element_info_array_w {
	size_t len;
	struct arg_dim_element_info_w *a;
};

struct arg_dim_element_info_array_m {
	size_t len;
	struct arg_dim_element_info_m *a;
};

struct arg_dim_element_info_array_a {
	size_t len;
	struct arg_dim_element_info_a *a;
};

typedef enum palo_value_type_tag
{
    PaloValueTypeNumeric = 1,
    PaloValueTypeStr = 2,
    PaloValueTypeError = 99
} palo_value_type;

typedef enum arg_palo_value_type_tag
{
    argPaloValueTypeStr,
    argPaloValueTypeDouble,
    argPaloValueTypeError
} arg_palo_value_type;

typedef union arg_palo_value_value_typ_w
{
	wchar_t *s;
	double d;
	struct arg_error err;
} arg_palo_value_value_w;

typedef union arg_palo_value_value_typ_m
{
	char *s;
	double d;
	struct arg_error err;
} arg_palo_value_value_m;

typedef union arg_palo_value_value_typ_a
{
	char *s;
	double d;
	struct arg_error err;
} arg_palo_value_value_a;

struct arg_palo_value_w {
	arg_palo_value_type type;
	arg_palo_value_value_w val;
};

struct arg_palo_value_m {
	arg_palo_value_type type;
	arg_palo_value_value_m val;
};

struct arg_palo_value_a {
	arg_palo_value_type type;
	arg_palo_value_value_a val;
};

struct arg_palo_value_array_w {
	size_t len;
	struct arg_palo_value_w *a;
};

struct arg_palo_value_array_m {
	size_t len;
	struct arg_palo_value_m *a;
};

struct arg_palo_value_array_a {
	size_t len;
	struct arg_palo_value_a *a;
};

struct arg_palo_dataset_a {
	struct arg_str_array_a coordinates;
	struct arg_palo_value_a value;
};

struct arg_palo_dataset_m {
	struct arg_str_array_m coordinates;
	struct arg_palo_value_m value;
};

struct arg_palo_dataset_w {
	struct arg_str_array_w coordinates;
	struct arg_palo_value_w value;
};

struct arg_palo_dataset_array_a {
	size_t len;
	struct arg_palo_dataset_a *a;
};

struct arg_palo_dataset_array_m {
	size_t len;
	struct arg_palo_dataset_m *a;
};

struct arg_palo_dataset_array_w {
	size_t len;
	struct arg_palo_dataset_w *a;
};

struct arg_getdata_export_result_w {
	struct arg_palo_dataset_array_w results;
	double progress;			 // fraction
};

struct arg_getdata_export_result_m {
	struct arg_palo_dataset_array_m results;
	double progress;			 // fraction
};

struct arg_getdata_export_result_a {
	struct arg_palo_dataset_array_a results;
	double progress;			 // fraction
};

typedef enum splash_mode_tag
{
    splash_mode_disable,
    splash_mode_default,
    splash_mode_base_set,
    splash_mode_base_add,
    splash_mode_unknown
} splash_mode;

typedef enum compare_op_tag
{
    compare_op_lt,
    compare_op_gt,
    compare_op_lte,
    compare_op_gte,
    compare_op_eq,
    compare_op_neq,
    compare_op_true,			 //  matches always
    compare_op_unknown
} compare_op;

typedef enum bool_op_tag
{
    bool_op_and,
    bool_op_or,
    bool_op_xor,
    bool_op_unknown
} bool_op;

struct arg_getdata_export_options_filter_w {
	compare_op cmp1;
	struct arg_palo_value_w val1;
	bool_op andor12;
	compare_op cmp2;
	struct arg_palo_value_w val2;
};

struct arg_getdata_export_options_filter_m {
	compare_op cmp1;
	struct arg_palo_value_m val1;
	bool_op andor12;
	compare_op cmp2;
	struct arg_palo_value_m val2;
};

struct arg_getdata_export_options_filter_a {
	compare_op cmp1;
	struct arg_palo_value_a val1;
	bool_op andor12;
	compare_op cmp2;
	struct arg_palo_value_a val2;
};

struct arg_getdata_export_options_w {
	palo_bool base_only;
	palo_bool ignore_empty;
	struct arg_getdata_export_options_filter_w filter;
	struct arg_str_array_w last_coordinates;
	unsigned long num_datasets;
};

struct arg_getdata_export_options_m {
	palo_bool base_only;
	palo_bool ignore_empty;
	struct arg_getdata_export_options_filter_m filter;
	struct arg_str_array_m last_coordinates;
	unsigned long num_datasets;
};

struct arg_getdata_export_options_a {
	palo_bool base_only;
	palo_bool ignore_empty;
	struct arg_getdata_export_options_filter_a filter;
	struct arg_str_array_a last_coordinates;
	unsigned long num_datasets;
};

struct arg_parent_info_raw_w {
	long identifier;
};

struct arg_parent_info_raw_m {
	long identifier;
};

struct arg_parent_info_raw_a {
	long identifier;
};

struct arg_child_info_raw_w {
	long identifier;
	double factor;
};

struct arg_child_info_raw_m {
	long identifier;
	double factor;
};

struct arg_child_info_raw_a {
	long identifier;
	double factor;
};

struct arg_dim_element_info2_raw_w {
	long identifier;
	wchar_t *name;
	de_type type;

	unsigned long level;
	unsigned long indent;
	unsigned long depth;
	unsigned long position;

	size_t num_parents;
	size_t num_children;
	struct arg_parent_info_raw_w * parents;
	struct arg_child_info_raw_w * children;
};

struct arg_dim_element_info2_raw_m {
	long identifier;
	char *name;
	de_type type;
	unsigned long level;
	unsigned long indent;
	unsigned long depth;
	unsigned long position;

	size_t num_parents;
	size_t num_children;

	struct arg_parent_info_raw_m * parents;
	struct arg_child_info_raw_m * children;
};

struct arg_dim_element_info2_raw_a {
	long identifier;
	char *name;
	de_type type;
	unsigned long level;
	unsigned long indent;
	unsigned long depth;
	unsigned long position;

	size_t num_parents;
	size_t num_children;

	struct arg_parent_info_raw_a * parents;
	struct arg_child_info_raw_a * children;
};

struct arg_dim_element_info2_raw_array_w {
	size_t len;
	struct arg_dim_element_info2_raw_w *a;
};

struct arg_dim_element_info2_raw_array_m {
	size_t len;
	struct arg_dim_element_info2_raw_m *a;
};

struct arg_dim_element_info2_raw_array_a {
	size_t len;
	struct arg_dim_element_info2_raw_a *a;
};

struct arg_rule_info_w {
	long identifier;
	wchar_t *definition;
	wchar_t *extern_id;
	wchar_t *comment;
	unsigned long long timestamp;
	unsigned short activated;
	double position;
};

struct arg_rule_info_m {
	long identifier;
	char *definition;
	char *extern_id;
	char *comment;
	unsigned long long timestamp;
	unsigned short activated;
	double position;
};

struct arg_rule_info_a {
	long identifier;
	char *definition;
	char *extern_id;
	char *comment;
	unsigned long long timestamp;
	unsigned short activated;
	double position;
};

struct arg_rule_info_array_w {
	size_t len;
	struct arg_rule_info_w *a;
};

struct arg_rule_info_array_m {
	size_t len;
	struct arg_rule_info_m *a;
};

struct arg_rule_info_array_a {
	size_t len;
	struct arg_rule_info_a *a;
};

struct arg_lock_info_w {
	long identifier;
	struct arg_str_array_array_w area;
	wchar_t *user;
	unsigned long steps;
};

struct arg_lock_info_m {
	long identifier;
	struct arg_str_array_array_m area;
	char *user;
	unsigned long steps;
};

struct arg_lock_info_a {
	long identifier;
	struct arg_str_array_array_a area;
	char *user;
	unsigned long steps;
};

struct arg_lock_info_array_w {
	size_t len;
	struct arg_lock_info_w *a;
};

struct arg_lock_info_array_m {
	size_t len;
	struct arg_lock_info_m *a;
};

struct arg_lock_info_array_a {
	size_t len;
	struct arg_lock_info_a *a;
};

struct arg_db_info_w {
	long id;
	wchar_t *name;
	unsigned int number_dimensions;
	unsigned int number_cubes;
	db_status status;
	db_type type;
};

struct arg_db_info_m {
	long id;
	char *name;
	unsigned int number_dimensions;
	unsigned int number_cubes;
	db_status status;
	db_type type;
};

struct arg_db_info_a {
	long id;
	char *name;
	unsigned int number_dimensions;
	unsigned int number_cubes;
	db_status status;
	db_type type;
};

struct arg_dim_info_w {
	long id;
	wchar_t *name;
	wchar_t *assoc_dimension;
	wchar_t *attribut_cube;
	wchar_t *rights_cube;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};

struct arg_dim_info_m {
	long id;
	char *name;
	char *assoc_dimension;
	char *attribut_cube;
	char *rights_cube;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};

struct arg_dim_info_a {
	long id;
	char *name;
	char *assoc_dimension;
	char *attribut_cube;
	char *rights_cube;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};

struct arg_dim_info_simple_w {
	long id;
	wchar_t *name;
	long assoc_dimension_id;
	long attribut_cube_id;
	long rights_cube_id;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};


struct arg_dim_info_simple_m {
	long id;
	char *name;
	long assoc_dimension_id;
	long attribut_cube_id;
	long rights_cube_id;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};

struct arg_dim_info_simple_a {
	long id;
	char *name;
	long assoc_dimension_id;
	long attribut_cube_id;
	long rights_cube_id;
	size_t number_elements;
	size_t maximum_level;
	size_t maximum_indent;
	size_t maximum_depth;
	dim_type type;
};

struct arg_cube_info_w {
	long id;
	wchar_t *name;
	unsigned int number_dimensions;
	struct arg_str_array_w dimensions;
	long double number_cells;
	long double number_filled_cells;
	cube_status status;
	cube_type type;
};

struct arg_cube_info_m {
	long id;
	char *name;
	unsigned int number_dimensions;
	struct arg_str_array_m dimensions;
	long double number_cells;
	long double number_filled_cells;
	cube_status status;
	cube_type type;
};

struct arg_cube_info_a {
	long id;
	char *name;
	unsigned int number_dimensions;
	struct arg_str_array_a dimensions;
	long double number_cells;
	long double number_filled_cells;
	cube_status status;
	cube_type type;
};

struct arg_svs_info_w {
	unsigned short svs_active;
	svs_login_mode login_mode;
	unsigned short cube_worker_active;
	unsigned short drill_through_enabled;
	unsigned short windows_sso_enabled;
};

struct arg_svs_info_m {
	unsigned short svs_active;
	svs_login_mode login_mode;
	unsigned short cube_worker_active;
	unsigned short drill_through_enabled;
	unsigned short windows_sso_enabled;
};

struct arg_svs_info_a {
	unsigned short svs_active;
	svs_login_mode login_mode;
	unsigned short cube_worker_active;
	unsigned short drill_through_enabled;
	unsigned short windows_sso_enabled;
};

// begin subset

struct arg_alias_filter_settings_w {
	unsigned short active;
	alias_filter_flags flags;
	wchar_t *attribute1;
	wchar_t *attribute2;
};

struct arg_alias_filter_settings_m {
	unsigned short active;
	alias_filter_flags flags;
	char *attribute1;
	char *attribute2;
};

struct arg_alias_filter_settings_a {
	unsigned short active;
	alias_filter_flags flags;
	char *attribute1;
	char *attribute2;
};

struct arg_field_filter_settings_w {
	unsigned short active;
	alias_filter_flags flags;
	struct arg_str_array_array_w advanced;
};

struct arg_field_filter_settings_m {
	unsigned short active;
	alias_filter_flags flags;
	struct arg_str_array_array_m advanced;
};

struct arg_field_filter_settings_a {
	unsigned short active;
	alias_filter_flags flags;
	struct arg_str_array_array_a advanced;
};

struct arg_basic_filter_settings_w {
	unsigned short active;
	pick_list_flags flags;
	unsigned short manual_subset_set;
	struct arg_str_array_w manual_subset;
};

struct arg_basic_filter_settings_m {
	unsigned short active;
	pick_list_flags flags;
	unsigned short manual_subset_set;
	struct arg_str_array_m manual_subset;
};

struct arg_basic_filter_settings_a {
	unsigned short active;
	pick_list_flags flags;
	unsigned short manual_subset_set;
	struct arg_str_array_a manual_subset;
};

struct arg_data_comparison_w {
	unsigned short use_strings;
	wchar_t *op1;
	double par1d;
	wchar_t *par1s;
	wchar_t *op2;
	double par2d;
	wchar_t *par2s;
};

struct arg_data_comparison_m {
	unsigned short use_strings;
	char *op1;
	double par1d;
	char *par1s;
	char *op2;
	double par2d;
	char *par2s;
};

struct arg_data_comparison_a {
	unsigned short use_strings;
	char *op1;
	double par1d;
	char *par1s;
	char *op2;
	double par2d;
	char *par2s;
};

struct arg_bool_str_array_w {
	unsigned short boolval;
	struct arg_str_array_w str;
};

struct arg_bool_str_array_m {
	unsigned short boolval;
	struct arg_str_array_m str;
};

struct arg_bool_str_array_a {
	unsigned short boolval;
	struct arg_str_array_a str;
};

struct arg_bool_str_array_array_w {
	unsigned int len;
	struct arg_bool_str_array_w *a;
};

struct arg_bool_str_array_array_m {
	unsigned int len;
	struct arg_bool_str_array_m *a;
};

struct arg_bool_str_array_array_a {
	unsigned int len;
	struct arg_bool_str_array_a *a;
};

struct arg_data_filter_settings_w {
	unsigned short active;
	data_filter_flags flags;
	wchar_t *cube;
	struct arg_data_comparison_w cmp;
	unsigned short coords_set;
	struct arg_bool_str_array_array_w coords;
	unsigned short upper_percentage_set;
	unsigned short lower_percentage_set;
	double upper_percentage;
	double lower_percentage;
	int top;
};

struct arg_data_filter_settings_m {
	unsigned short active;
	data_filter_flags flags;
	char *cube;
	struct arg_data_comparison_m cmp;
	unsigned short coords_set;
	struct arg_bool_str_array_array_m coords;
	unsigned short upper_percentage_set;
	unsigned short lower_percentage_set;
	double upper_percentage;
	double lower_percentage;
	int top;
};

struct arg_data_filter_settings_a {
	unsigned short active;
	data_filter_flags flags;
	char *cube;
	struct arg_data_comparison_a cmp;
	unsigned short coords_set;
	struct arg_bool_str_array_array_a coords;
	unsigned short upper_percentage_set;
	unsigned short lower_percentage_set;
	double upper_percentage;
	double lower_percentage;
	int top;
};

struct arg_sorting_filter_settings_w {
	unsigned short active;
	sorting_filter_flags flags;
	wchar_t *attribute;
	int level;
	unsigned int limit_count;
	unsigned int limit_start;
};

struct arg_sorting_filter_settings_m {
	unsigned short active;
	sorting_filter_flags flags;
	char *attribute;
	int level;
	unsigned int limit_count;
	unsigned int limit_start;
};

struct arg_sorting_filter_settings_a {
	unsigned short active;
	sorting_filter_flags flags;
	char *attribute;
	int level;
	unsigned int limit_count;
	unsigned int limit_start;
};

struct arg_structural_filter_settings_w {
	unsigned short active;
	structural_filter_flags flags;
	wchar_t *bound;
	unsigned short level;
	int level_start;
	int level_end;
	unsigned short revolve;
	wchar_t *revolve_element;
	int revolve_count;
};

struct arg_structural_filter_settings_m {
	unsigned short active;
	structural_filter_flags flags;
	char *bound;
	unsigned short level;
	int level_start;
	int level_end;
	unsigned short revolve;
	char *revolve_element;
	int revolve_count;
};

struct arg_structural_filter_settings_a {
	unsigned short active;
	structural_filter_flags flags;
	char *bound;
	unsigned short level;
	int level_start;
	int level_end;
	unsigned short revolve;
	char *revolve_element;
	int revolve_count;
};

struct arg_text_filter_settings_w {
	unsigned short active;
	text_filter_flags flags;
	struct arg_str_array_w reg_exps;
};

struct arg_text_filter_settings_m {
	unsigned short active;
	text_filter_flags flags;
	struct arg_str_array_m reg_exps;
};

struct arg_text_filter_settings_a {
	unsigned short active;
	text_filter_flags flags;
	struct arg_str_array_a reg_exps;
};

struct arg_subset_result_w {
	wchar_t *name;
	wchar_t *alias;
	wchar_t *path;
	size_t index;
	size_t depth;
	long identifier;
	unsigned short has_children;
};

struct arg_subset_result_m {
	char *name;
	char *alias;
	char *path;
	size_t index;
	size_t depth;
	long identifier;
	unsigned short has_children;
};

struct arg_subset_result_a {
	char *name;
	char *alias;
	char *path;
	size_t index;
	size_t depth;
	long identifier;
	unsigned short has_children;
};

struct arg_subset_result_array_w {
	unsigned int len;
	struct arg_subset_result_w *a;
};

struct arg_subset_result_array_m {
	unsigned int len;
	struct arg_subset_result_m *a;
};

struct arg_subset_result_array_a {
	unsigned int len;
	struct arg_subset_result_a *a;
};

// end subset

#ifdef __cplusplus
extern "C" {
#endif

	void free_arg_str_w(wchar_t *s);
	void free_arg_str_array_contents_w(struct arg_str_array_w *sa);
	void free_arg_dim_element_info_contents_w(struct arg_dim_element_info_w *dei);
	void free_arg_dim_element_info_array_contents_w(struct arg_dim_element_info_array_w *deia);
	void free_arg_dim_element_info2_raw_contents_w(struct arg_dim_element_info2_raw_w *dei);
	void free_arg_dim_element_info2_raw_array_contents_w(struct arg_dim_element_info2_raw_array_w *deia);
	void free_arg_consolidation_element_info_contents_w(struct arg_consolidation_element_info_w *cei);
	void free_arg_consolidation_element_info_array_contents_w(struct arg_consolidation_element_info_array_w *ceia);
	void free_arg_palo_value_contents_w(struct arg_palo_value_w *pv);
	void free_arg_palo_value_array_contents_w(struct arg_palo_value_array_w *pva);
	void free_arg_str_array_2d_contents_w(struct arg_str_array_2d_w *sa);
	void free_arg_str_array_array_contents_w(struct arg_str_array_array_w *sa);
	void free_arg_palo_dataset_contents_w(struct arg_palo_dataset_w *ds);
	void free_arg_palo_dataset_array_contents_w(struct arg_palo_dataset_array_w *dsa);
	void free_arg_getdata_export_result_contents_w(struct arg_getdata_export_result_w *exres);
	void free_arg_rule_info_contents_w(struct arg_rule_info_w *ri);
	void free_arg_rule_info_array_contents_w(struct arg_rule_info_array_w *ria);
	void free_arg_lock_info_contents_w(struct arg_lock_info_w *li);
	void free_arg_lock_info_array_contents_w(struct arg_lock_info_array_w *lia);
	void free_arg_db_info_contents_w(struct arg_db_info_w *dbi);
	void free_arg_dim_info_contents_w(struct arg_dim_info_w *di);
	void free_arg_dim_info_simple_contents_w(struct arg_dim_info_simple_w *di);
	void free_arg_cube_info_contents_w(struct arg_cube_info_w *ci);
	void free_arg_subset_result_contents_w(struct arg_subset_result_w *sr);
	void free_arg_subset_result_array_contents_w(struct arg_subset_result_array_w *sra);

	void free_arg_str_m(char *s);
	void free_arg_str_array_contents_m(struct arg_str_array_m *sa);
	void free_arg_dim_element_info_contents_m(struct arg_dim_element_info_m *dei);
	void free_arg_dim_element_info_array_contents_m(struct arg_dim_element_info_array_m *deia);
	void free_arg_dim_element_info2_raw_contents_m(struct arg_dim_element_info2_raw_m *dei);
	void free_arg_dim_element_info2_raw_array_contents_m(struct arg_dim_element_info2_raw_array_m *deia);
	void free_arg_consolidation_element_info_contents_m(struct arg_consolidation_element_info_m *cei);
	void free_arg_consolidation_element_info_array_contents_m(struct arg_consolidation_element_info_array_m *ceia);
	void free_arg_palo_value_contents_m(struct arg_palo_value_m *pv);
	void free_arg_palo_value_array_contents_m(struct arg_palo_value_array_m *pva);
	void free_arg_str_array_2d_contents_m(struct arg_str_array_2d_m *sa);
	void free_arg_str_array_array_contents_m(struct arg_str_array_array_m *sa);
	void free_arg_palo_dataset_contents_m(struct arg_palo_dataset_m *ds);
	void free_arg_palo_dataset_array_contents_m(struct arg_palo_dataset_array_m *dsa);
	void free_arg_getdata_export_result_contents_m(struct arg_getdata_export_result_m *exres);
	void free_arg_rule_info_contents_m(struct arg_rule_info_m *ri);
	void free_arg_rule_info_array_contents_m(struct arg_rule_info_array_m *ria);
	void free_arg_lock_info_contents_m(struct arg_lock_info_m *li);
	void free_arg_lock_info_array_contents_m(struct arg_lock_info_array_m *lia);
	void free_arg_db_info_contents_m(struct arg_db_info_m *dbi);
	void free_arg_dim_info_contents_m(struct arg_dim_info_m *di);
	void free_arg_dim_info_simple_contents_m(struct arg_dim_info_simple_m *di);
	void free_arg_cube_info_contents_m(struct arg_cube_info_m *ci);
	void free_arg_subset_result_contents_m(struct arg_subset_result_m *sr);
	void free_arg_subset_result_array_contents_m(struct arg_subset_result_array_m *sra);

	void free_arg_str_a(char *s);
	void free_arg_str_array_contents_a(struct arg_str_array_a *sa);
	void free_arg_dim_element_info_contents_a(struct arg_dim_element_info_a *dei);
	void free_arg_dim_element_info_array_contents_a(struct arg_dim_element_info_array_a *deia);
	void free_arg_dim_element_info2_raw_contents_a(struct arg_dim_element_info2_raw_a *dei);
	void free_arg_dim_element_info2_raw_array_contents_a(struct arg_dim_element_info2_raw_array_a *deia);
	void free_arg_consolidation_element_info_contents_a(struct arg_consolidation_element_info_a *cei);
	void free_arg_consolidation_element_info_array_contents_a(struct arg_consolidation_element_info_array_a *ceia);
	void free_arg_palo_value_contents_a(struct arg_palo_value_a *pv);
	void free_arg_palo_value_array_contents_a(struct arg_palo_value_array_a *pva);
	void free_arg_str_array_2d_contents_a(struct arg_str_array_2d_a *sa);
	void free_arg_str_array_array_contents_a(struct arg_str_array_array_a *sa);
	void free_arg_palo_dataset_contents_a(struct arg_palo_dataset_a *ds);
	void free_arg_palo_dataset_array_contents_a(struct arg_palo_dataset_array_a *dsa);
	void free_arg_getdata_export_result_contents_a(struct arg_getdata_export_result_a *exres);
	void free_arg_rule_info_contents_a(struct arg_rule_info_a *ri);
	void free_arg_rule_info_array_contents_a(struct arg_rule_info_array_a *ria);
	void free_arg_lock_info_contents_a(struct arg_lock_info_a *li);
	void free_arg_lock_info_array_contents_a(struct arg_lock_info_array_a *lia);
	void free_arg_db_info_contents_a(struct arg_db_info_a *dbi);
	void free_arg_dim_info_contents_a(struct arg_dim_info_a *di);
	void free_arg_dim_info_simple_contents_a(struct arg_dim_info_simple_a *di);
	void free_arg_cube_info_contents_a(struct arg_cube_info_a *ci);
	void free_arg_subset_result_contents_a(struct arg_subset_result_a *sr);
	void free_arg_subset_result_array_contents_a(struct arg_subset_result_array_a *sra);

	void free_arg_error_contents(struct arg_error *err);

#ifdef __cplusplus
}
#endif
#endif
