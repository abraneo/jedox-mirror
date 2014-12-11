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

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <php.h>
#include <zend.h>
#include <zend_list.h>
#include <zend_ini.h>

#include <locale.h>

#include "php_jedox_palo.h"

#include "wrap.h"

#include "version.h"

#include "ZendValue.h"

#include "c2c_plus_plus.h"

ZEND_DECLARE_MODULE_GLOBALS(php_palo)

int le_connection_resource;

ZEND_MINIT_FUNCTION(php_palo);
ZEND_MSHUTDOWN_FUNCTION(php_palo);
ZEND_RINIT_FUNCTION(php_palo);
ZEND_RSHUTDOWN_FUNCTION(php_palo);

void connection_resource_destruction_handler(zend_rsrc_list_entry *rsrc TSRMLS_DC);

#if PHP_MAJOR_VERSION > 4
    ZEND_BEGIN_ARG_INFO(pass_all_by_reference, 1)
    ZEND_END_ARG_INFO()
#else
    static unsigned char pass_all_by_reference[] = { 1, BYREF_FORCE_REST };
#endif

zend_function_entry php_palo_functions[] = {
	ZEND_FE(palo_use_unicode, NULL)
	ZEND_FE(palo_error, NULL)
	ZEND_FE(palo_clear_error, NULL)
	ZEND_FE(palo_version, NULL)
	ZEND_FE(palo_set_svs, NULL)

    ZEND_FE(palo_emove, NULL)
    ZEND_FE(palo_emove_bulk, NULL)
    // ZEND_FE(palo_eupdate, NULL)
    ZEND_FE(palo_dataav, NULL)
    ZEND_FE(palo_getdata_export, NULL)
    ZEND_FE(palo_setdata_bulk, NULL)
    ZEND_FE(palo_server_info, NULL)
    ZEND_FE(palo_license_info, NULL)
	ZEND_FE(palo_change_password, NULL)

	ZEND_FE(palo_cube_rule_create, NULL)
	ZEND_FE(palo_cube_rule_modify, NULL)
	ZEND_FE(palo_cube_rule_parse, NULL)
	ZEND_FE(palo_cube_rules_move, NULL)
	ZEND_FE(palo_cube_rule_delete, NULL)
	ZEND_FE(palo_cube_rules_delete, NULL)
	ZEND_FE(palo_cube_rules, NULL)

	ZEND_FE(palo_database_info, NULL)

    ZEND_FE(palo_dimension_count_top_list_elements, NULL)
    ZEND_FE(palo_dimension_simple_flat_list_elements, NULL)
    ZEND_FE(palo_dimension_simple_top_list_elements, NULL)
    ZEND_FE(palo_dimension_simple_children_list_elements, NULL)
	ZEND_FE(palo_dimension_reduced_flat_list_elements, NULL)
    ZEND_FE(palo_dimension_reduced_top_list_elements, NULL)
    ZEND_FE(palo_dimension_reduced_children_list_elements, NULL)

	ZEND_FE(palo_get_user_for_sid, NULL)
	ZEND_FE(palo_get_groups_for_sid, NULL)
	ZEND_FE(palo_cellcopy, NULL)
	ZEND_FE(palo_goal_seek, NULL)
	ZEND_FE(palo_cube_convert, NULL)
	ZEND_FE(palo_cell_drilltrough, NULL)
	ZEND_FE(palo_svs_restart, NULL)
	ZEND_FE(palo_svs_info, NULL)
	ZEND_FE(palo_activate_license, NULL)
	ZEND_FE(palo_change_user_password, NULL)
	ZEND_FE(palo_set_client_description, NULL)
    ZEND_FE(palo_root_unload_database, NULL)
    ZEND_FE(palo_root_save_database, NULL)
	ZEND_FE(palo_cube_lock, NULL)
	ZEND_FE(palo_cube_locks, NULL)
	ZEND_FE(palo_cube_rollback, NULL)
	ZEND_FE(palo_cube_commit, NULL)

    // aliases
    ZEND_FALIAS(palo_getdata, palo_dataa, NULL) // deprecated
    ZEND_FALIAS(palo_dataset, palo_setdataa, NULL) // deprecated

#include "php_jedox_palo.c.h"

    { NULL, NULL, NULL }
};

zend_module_entry php_palo_module_entry = {
    STANDARD_MODULE_HEADER,
    "jedox_palo",
    php_palo_functions,
    ZEND_MINIT(php_palo),
    ZEND_MSHUTDOWN(php_palo),
    ZEND_RINIT(php_palo),
    ZEND_RSHUTDOWN(php_palo),
    ZEND_MINFO(php_palo),
    PRODUCT_VERSION_STR,
    STANDARD_MODULE_PROPERTIES
};

ZEND_MINIT_FUNCTION(php_palo)
{
	REGISTER_INI_ENTRIES();

	REGISTER_STRING_CONSTANT("ELEMENTS_ALL", (char*)get_ELEMENTS_ALL(), CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("PALO_ELEMENTS_ALL", (char*)get_ELEMENTS_ALL(), CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("PALO_GOALSEEK_COMPLETE", GetGoalSeekType(PHPPalo_GOALSEEK_COMPLETE), CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PALO_GOALSEEK_EQUAL", GetGoalSeekType(PHPPalo_GOALSEEK_EQUAL), CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PALO_GOALSEEK_RELATIVE", GetGoalSeekType(PHPPalo_GOALSEEK_RELATIVE), CONST_CS | CONST_PERSISTENT);
#ifdef ZTS
	ZEND_INIT_MODULE_GLOBALS(php_palo, php_php_palo_globals_ctor, php_php_palo_globals_dtor);
#else
    php_php_palo_globals_ctor(&php_palo_globals TSRMLS_CC);
#endif

	if (INI_INT(JEDOX_PHPPALO_USE_BACKWARDS_STARTINDEX)==0) {
		ChangeToNewStartIndex();
	}

	PHPInitSSL(INI_STR(JEDOX_PHPPALO_TRUST_FILE));

	InternalPaloWeb(INI_INT(JEDOX_PHPPALO_PALO_WEB_INTERNAL) + INI_INT(JEDOX_PHPPALO_DUMB_IDEA));
	ActivateLogStderr(INI_INT(JEDOX_PHPPALO_LOG_TO_STDERR));

	le_connection_resource=zend_register_list_destructors_ex(connection_resource_destruction_handler, NULL, CONNECTION_RESOURCE_NAME, module_number);

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(php_palo)
{
	UNREGISTER_INI_ENTRIES();

#ifdef ZTS
	ts_free_id(php_palo_globals_id);
#else
	php_php_palo_globals_dtor(&php_palo_globals TSRMLS_CC);
#endif

    return EventHandlerModulShutdown(0 TSRMLS_CC) ? SUCCESS : FAILURE;
}

ZEND_RINIT_FUNCTION(php_palo)
{
	return EventHandlerRequestStartup(0 TSRMLS_CC) ? SUCCESS : FAILURE;
}

ZEND_RSHUTDOWN_FUNCTION(php_palo)
{
    return EventHandlerRequestShutdown(0 TSRMLS_CC) ? SUCCESS : FAILURE;
}

#if COMPILE_DL_PHP_PALO
    ZEND_GET_MODULE(php_palo)
#endif
