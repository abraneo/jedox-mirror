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

#ifndef PHP_PALO_SPREADSHEET_FUNCS_H
#define PHP_PALO_SPREADSHEET_FUNCS_H

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <memory>

#include <PaloSpreadsheetFuncs/SpreadsheetFuncs.h>

#include "PHPPaloContext.h"
#include "PHPPaloLogger.h"

extern "C" {
#include <zend.h>
#include "fix_zend.h"
}

namespace Palo {
namespace PHPPalo {
class PHPPaloSpreadsheetFuncs : public Palo::SpreadsheetFuncs::SpreadsheetFuncs {
public:
	PHPPaloSpreadsheetFuncs(int dummy TSRMLS_DC);
	~PHPPaloSpreadsheetFuncs();

	void FPaloUseUnicode(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloError(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloClearError(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	static void setThreadLocal(PHPPaloSpreadsheetFuncs* sf TSRMLS_DC);
	static PHPPaloSpreadsheetFuncs* getThreadLocal(int dummy TSRMLS_DC);

	PHPPaloContext& getContext();

	PHPPaloLogger& getLogger();

private:
	PHPPaloContext s;
	PHPPaloLogger logger;
};
}
}
#endif
