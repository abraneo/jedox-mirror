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

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <libpalo_ng/Palo/Server.h>

#include "PHPPaloSpreadsheetFuncsWrapper.h"
#include "PHPPaloSpreadsheetFuncs.h"
#include "PHPPaloVersion.h"

#include "php_jedox_palo.h"

extern "C" {
#include <ext/standard/info.h>
}

using namespace jedox::palo;

namespace Palo {
	namespace PHPPalo {
		class EventHandler {
		public:
			static EventHandler& getInstance() {
				static EventHandler eh;
				return eh;
			}

			bool OnRequestStartup( int dummy TSRMLS_DC ) {
				std::unique_ptr<PHPPaloSpreadsheetFuncs> p;
				try {
					p.reset( new PHPPaloSpreadsheetFuncs( 0 TSRMLS_CC ) );
				} catch (const std::exception& ) {
					return false;
				}

				try {
					p->calculationBegin();
					PHPPaloSpreadsheetFuncs::setThreadLocal( p.get() TSRMLS_CC );
					p.release();
					return true;
				} catch (const std::exception& ) {
					zend_error( E_ERROR, "Palo::SpreadsheetFuncs::calculationBegin() failed" );
				}

				return false;
			}

			bool OnRequestShutdown( int dummy TSRMLS_DC ) {
				try {
					PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->calculationEnd();
					PHPPaloSpreadsheetFuncs::setThreadLocal( NULL TSRMLS_CC );
					return true;
				} catch (const std::exception& ) {
					PHPPaloSpreadsheetFuncs::setThreadLocal( NULL TSRMLS_CC );
					zend_error( E_ERROR, "Palo::SpreadsheetFuncs::calculationEnd() failed" );
				}

				return false;
			}

			bool OnModulShutdown( int dummy TSRMLS_DC ) {
				try {
					return true;
				} catch (const std::exception& ) {
					zend_error( E_ERROR, "Palo::PHPPaloSpreadsheetFuncsWrapper::cleanup() failed" );
				}

				return false;
			}

		};

		extern "C" short int EventHandlerRequestStartup( int dummy TSRMLS_DC ) {
			return EventHandler::getInstance().OnRequestStartup( 0 TSRMLS_CC );
		}

		extern "C" short int EventHandlerRequestShutdown( int dummy TSRMLS_DC ) {
			return EventHandler::getInstance().OnRequestShutdown( 0 TSRMLS_CC );
		}

		extern "C" short int EventHandlerModulShutdown( int dummy TSRMLS_DC ) {
			return EventHandler::getInstance().OnModulShutdown( 0 TSRMLS_CC );
		}

		extern "C" void connection_resource_destruction_handler( zend_rsrc_list_entry *rsrc TSRMLS_DC ) {
			delete( boost::shared_ptr<jedox::palo::Server>* )rsrc->ptr;
		}

		extern "C" void php_php_palo_globals_ctor( zend_php_palo_globals *php_palo_globals TSRMLS_DC ) {
			php_palo_globals->p = NULL;
		}

		extern "C" void php_php_palo_globals_dtor( zend_php_palo_globals *php_palo_globals TSRMLS_DC ) {
			if ( php_palo_globals->p == NULL ) {
				delete( PHPPaloSpreadsheetFuncs* )php_palo_globals->p;
				php_palo_globals->p = NULL;
			}
		}

		/* module info */
		extern "C" ZEND_MINFO_FUNCTION( php_palo ) {
			const PHPPaloVersion& ver = PHPPaloVersion::getVersion();
			php_info_print_table_start();
			php_info_print_table_header( 2, "jedox_palo", "PHP <-> PALO server connectivity extension NG" );
			php_info_print_table_row( 2, "Status", "installed" );
			php_info_print_table_row( 2, "Company", "Jedox AG" );
			php_info_print_table_row( 2, "Version", ver.phppalo_ng_version_str.c_str() );
			php_info_print_table_row( 2, "LibPalo Version", ver.libpalo_ng_version_str.c_str() );
			php_info_print_table_end();
			DISPLAY_INI_ENTRIES();
		}
	}
}
