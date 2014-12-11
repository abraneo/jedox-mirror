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
 * 
 *
 */

#ifndef PHP_PALO_H
#define PHP_PALO_H
#include <php_version.h>
#if PHP_VERSION_ID < 50300
#ifdef __cplusplus
extern "C" {
#endif
#endif
#include <php.h>
#include <zend.h>
#include <zend_list.h>
#include <zend_ini.h>
#if PHP_VERSION_ID < 50300
#ifdef __cplusplus
}
#endif
#endif

static char JEDOX_PHPPALO_USE_BACKWARDS_STARTINDEX[] = "jedox.phppalo.start_index";
static char JEDOX_PHPPALO_TRUST_FILE[] = "jedox.phppalo.trust_file";
static char JEDOX_PHPPALO_PALO_WEB_INTERNAL[] = "jedox.phppalo.palo_web_internal";
static char JEDOX_PHPPALO_DUMB_IDEA[] = "jedox.phppalo.treat_warnings_as_errors";
static char JEDOX_PHPPALO_LOG_TO_STDERR[] = "jedox.phppalo.log_to_stderr";
static char JEDOX_PHPPALO_1[] = "1";
static char JEDOX_PHPPALO_EMPTY[] = "";
static char JEDOX_PHPPALO_0[] = "0";

ZEND_INI_BEGIN()
ZEND_INI_ENTRY( JEDOX_PHPPALO_USE_BACKWARDS_STARTINDEX, JEDOX_PHPPALO_1, ZEND_INI_SYSTEM, NULL )
ZEND_INI_ENTRY( JEDOX_PHPPALO_TRUST_FILE, JEDOX_PHPPALO_EMPTY, ZEND_INI_SYSTEM, NULL )
ZEND_INI_ENTRY( JEDOX_PHPPALO_PALO_WEB_INTERNAL, JEDOX_PHPPALO_0, ZEND_INI_SYSTEM, NULL )
ZEND_INI_ENTRY( JEDOX_PHPPALO_DUMB_IDEA, JEDOX_PHPPALO_0, ZEND_INI_SYSTEM, NULL )
ZEND_INI_ENTRY( JEDOX_PHPPALO_LOG_TO_STDERR, JEDOX_PHPPALO_0, ZEND_INI_SYSTEM, NULL )
ZEND_INI_END()

ZEND_BEGIN_MODULE_GLOBALS( php_palo )
void *p; // PHPPaloSpreadsheetFuncs*
ZEND_END_MODULE_GLOBALS( php_palo )

#ifdef __cplusplus
extern "C" {
#endif

short int EventHandlerRequestStartup(int dummy TSRMLS_DC);
short int EventHandlerRequestShutdown(int dummy TSRMLS_DC);
short int EventHandlerModulShutdown(int dummy TSRMLS_DC);

void php_php_palo_globals_ctor(zend_php_palo_globals *php_palo_globals TSRMLS_DC);
void php_php_palo_globals_dtor(zend_php_palo_globals *php_palo_globals TSRMLS_DC);

void connection_resource_destruction_handler(zend_rsrc_list_entry *rsrc TSRMLS_DC);
extern int le_connection_resource;
ZEND_EXTERN_MODULE_GLOBALS( php_palo )

ZEND_MINFO_FUNCTION( php_palo );
#ifdef __cplusplus
}
#endif

#define CONNECTION_RESOURCE_NAME "PALO database connection"

#ifdef ZTS
#include "TSRM.h"
#define PHP_PALO_G(v) TSRMG(php_palo_globals_id, zend_php_palo_globals*, v)
#else
#define PHP_PALO_G(v) (php_palo_globals.v)
#endif
#endif
