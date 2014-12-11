/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#ifndef WRAP_H
#define WRAP_H

#ifdef __cplusplus
extern "C" {
#endif

	ZEND_FUNCTION( palo_version );
	ZEND_FUNCTION( palo_error );
	ZEND_FUNCTION( palo_clear_error );
	ZEND_FUNCTION( palo_use_unicode );
	ZEND_FUNCTION( palo_set_svs );

	ZEND_FUNCTION( palo_emove );
	ZEND_FUNCTION( palo_emove_bulk );
	ZEND_FUNCTION( palo_eupdate );
	ZEND_FUNCTION( palo_dataav );
	ZEND_FUNCTION( palo_getdata_export );
	ZEND_FUNCTION( palo_setdata_bulk );
	ZEND_FUNCTION( palo_server_info );
	ZEND_FUNCTION( palo_license_info );
	ZEND_FUNCTION( palo_change_password );
	ZEND_FUNCTION( palo_change_user_password );
	ZEND_FUNCTION( palo_cube_rule_create );
	ZEND_FUNCTION( palo_cube_rule_modify );
	ZEND_FUNCTION( palo_cube_rule_parse );
	ZEND_FUNCTION( palo_cube_rules_move );
	ZEND_FUNCTION( palo_cube_rule_delete );
	ZEND_FUNCTION( palo_cube_rules_delete );
	ZEND_FUNCTION( palo_cube_rules );
	ZEND_FUNCTION( palo_database_info );
    ZEND_FUNCTION( palo_dimension_count_top_list_elements );
    ZEND_FUNCTION( palo_dimension_simple_flat_list_elements );
    ZEND_FUNCTION( palo_dimension_simple_top_list_elements );
    ZEND_FUNCTION( palo_dimension_simple_children_list_elements );
	ZEND_FUNCTION( palo_dimension_reduced_flat_list_elements );
    ZEND_FUNCTION( palo_dimension_reduced_top_list_elements );
    ZEND_FUNCTION( palo_dimension_reduced_children_list_elements );
	ZEND_FUNCTION( palo_get_user_for_sid );
	ZEND_FUNCTION( palo_get_groups_for_sid );
	ZEND_FUNCTION( palo_cellcopy );
	ZEND_FUNCTION( palo_goal_seek );
	ZEND_FUNCTION( palo_cube_convert );
	ZEND_FUNCTION( palo_cell_drilltrough );
	ZEND_FUNCTION(palo_svs_restart);
	ZEND_FUNCTION(palo_svs_info);
	ZEND_FUNCTION(palo_activate_license);
	ZEND_FUNCTION(palo_set_client_description);
	ZEND_FUNCTION(palo_root_unload_database);
	ZEND_FUNCTION(palo_root_save_database);
	ZEND_FUNCTION(palo_cube_lock);
	ZEND_FUNCTION(palo_cube_locks);
	ZEND_FUNCTION(palo_cube_rollback);
	ZEND_FUNCTION(palo_cube_commit);

#include "wrap.h.h"

#ifdef __cplusplus
}
#endif
#endif
