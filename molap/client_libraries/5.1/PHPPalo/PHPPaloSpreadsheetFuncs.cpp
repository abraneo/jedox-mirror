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

#include "PHPPaloSpreadsheetFuncs.h"

#include "PHPPaloException.h"

#include "php_jedox_palo.h"

using namespace Palo::PHPPalo;

PHPPaloSpreadsheetFuncs::PHPPaloSpreadsheetFuncs(int dummy TSRMLS_DC) :
		s(), logger(s)
{
}

PHPPaloSpreadsheetFuncs::~PHPPaloSpreadsheetFuncs()
{
}

PHPPaloContext& PHPPaloSpreadsheetFuncs::getContext()
{
	return s;
}

PHPPaloSpreadsheetFuncs* PHPPaloSpreadsheetFuncs::getThreadLocal(int dummy TSRMLS_DC)
{
	PHPPaloSpreadsheetFuncs* sf = (PHPPaloSpreadsheetFuncs*)PHP_PALO_G( p );

	if (sf == NULL) {
		throw PHPPaloException(CurrentSourceLocation, PHPPaloErrors::ERROR_THREAD_LOCAL_STORAGE);
	}

	return sf;
}

void PHPPaloSpreadsheetFuncs::setThreadLocal(PHPPaloSpreadsheetFuncs* sf TSRMLS_DC)
{
	if (PHP_PALO_G( p ) != NULL) {
		delete (PHPPaloSpreadsheetFuncs*)PHP_PALO_G( p );
	}

	PHP_PALO_G( p ) = (void*)sf;
}

void PHPPaloSpreadsheetFuncs::FPaloUseUnicode(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.checkArgCount(1);

	static_cast<PHPPaloContext&>(opts).setEncodingConversion(!arg[0].getBool());
}

void PHPPaloSpreadsheetFuncs::FPaloError(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.checkArgCount(0);

	GenericArrayBuilder a = retval.setArray(2);
	ErrorInfo ei = opts.getError();
    if ((int)ei.errcode == (int)jedox::palo::LibPaloNGExceptionFactory::PALO_NG_ERROR_NO_VALUE_YET) {
            ei.desc = CellValue::strInitValue;
    }
	a.append("desc", a.createGenericCell()->set(ei.desc));
	a.append("paloerrorcode", a.createGenericCell()->set(ei.paloerrorcode));
}

void PHPPaloSpreadsheetFuncs::FPaloClearError(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.checkArgCount(0);
	opts.setError(ErrorInfo());
}

PHPPaloLogger& PHPPaloSpreadsheetFuncs::getLogger()
{
	return logger;
}
