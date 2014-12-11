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

#include "PHPPaloSpreadsheetFuncs.h"
#include "PHPPaloVersion.h"
#include "PHPPaloSpreadsheetFuncsWrapper.h"
#include "wrap.h"

#include "php_jedox_palo.h"

using namespace Palo::PHPPalo;

extern "C" {
	ZEND_FUNCTION( palo_version ) {
		const PHPPaloVersion& ver = PHPPaloVersion::getVersion();

		ZendValue ret = ZendValue( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), return_value );
		GenericArrayBuilder a = ret.setArray( 2 );
		a.append( "php_palo", a.createGenericCell()->set( ver.phppalo_ng_version ) );
		a.append( "libpalo", a.createGenericCell()->set( ver.libpalo_ng_version ) );
		a.append( "php_palo_str", a.createGenericCell()->set( ver.phppalo_ng_version_str ) );
		a.append( "libpalo_str", a.createGenericCell()->set( ver.libpalo_ng_version_str ) );
	}

	ZEND_FUNCTION( palo_emove ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloElementMove )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_emove_bulk ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloElementMoveBulk )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_event_lock_begin ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloEventLockBegin )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_event_lock_end ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloEventLockEnd )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dataav ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloGetdataAV )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_getdata_export ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloGetdataExport )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_setdata_bulk ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloSetdataBulk )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_server_info ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloServerInfo )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_license_info ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloLicenseInfo )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rule_create ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRuleCreate )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rule_modify ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRuleModify )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rule_parse ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRuleParse )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rules_move ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRulesMove )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rule_delete ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRuleDelete )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rules_delete ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRulesDelete )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_rules ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRules )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_database_info ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDatabaseInfo )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_count_top_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionTopElementsCount )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_simple_flat_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionSimpleFlatListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_simple_top_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionSimpleTopListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_simple_children_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionSimpleChildrenListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_reduced_flat_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionReducedFlatListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_reduced_top_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionReducedTopListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_dimension_reduced_children_list_elements ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloDimensionReducedChildrenListElements )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_use_unicode ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloUseUnicode )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_set_svs ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloSetSvs )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_error ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloError )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_clear_error ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloClearError )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_change_password ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloChangePassword )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_change_user_password ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloChangeUserPassword )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_get_user_for_sid ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloGetUserForSID )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_get_groups_for_sid ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloGetGroupsForSID )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cellcopy ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCellCopy )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_goal_seek ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloGoalSeek )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cube_convert ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeConvert )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_cell_drilltrough ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCellDrillTrough )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_svs_restart ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloSVSRestart )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_svs_info ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloSVSInfo )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_activate_license ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloActivateLicense)( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_set_client_description ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloSetClientDescription )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_root_unload_database ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloRootUnloadDatabase )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	ZEND_FUNCTION( palo_root_save_database ) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloRootSaveDatabase )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}
	ZEND_FUNCTION(palo_cube_lock) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeLock )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}
	ZEND_FUNCTION(palo_cube_locks) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeLocks )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}
	ZEND_FUNCTION(palo_cube_rollback) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeRollback )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}
	ZEND_FUNCTION(palo_cube_commit) {
		PHPPaloSpreadsheetFuncsWrapper( PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC ), PHPPaloSpreadsheetFuncs::getThreadLocal( 0 TSRMLS_CC )->getContext(), &PHPPaloSpreadsheetFuncs::FPaloCubeCommit )( INTERNAL_FUNCTION_PARAM_PASSTHRU );
	}

	// auto-generated
#	include "wrap.cpp.h"
}
