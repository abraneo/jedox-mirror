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

#include "PHPPaloSpreadsheetFuncsWrapper.h"
#include "PHPPaloLogger.h"
#include "ArrayGuard.h"

#include "ZendValue.h"

#include <PaloSpreadsheetFuncs/WrongParamCountException.h>
#include <PaloSpreadsheetFuncs/InvalidRequestException.h>

using namespace Palo::PHPPalo;
using namespace Palo::SpreadsheetFuncs;
using namespace Palo::Util;
using namespace std;

#define SEM_INITIAL_COUNT 1
#define SEM_INITIAL_MAX_COUNT 1

PHPPaloSpreadsheetFuncsWrapper::PHPPaloSpreadsheetFuncsWrapper(PHPPaloSpreadsheetFuncs* thisptr, GenericContext& opts, void(PHPPaloSpreadsheetFuncs::*p)(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)) :
		Wrapper<PHPPaloSpreadsheetFuncs>(thisptr->getLogger(), thisptr, opts, p)
{
}

void PHPPaloSpreadsheetFuncsWrapper::operator()(INTERNAL_FUNCTION_PARAMETERS) throw ()
{
	try {
		try {
			const int argc = ZEND_NUM_ARGS();

			vector<GenericCell*> v;
			ArrayGuard<zval**> zargs(new zval**[argc]);

			try {
				if (zend_get_parameters_array_ex( argc, zargs.get() ) == FAILURE) {
					RETURN_XLERROR( VALUExl);
				}

				v.reserve(argc);
				for (int i = 0; i < argc; i++) {
					v.push_back(new ZendValue(opts, *(zargs.get())[i]));
				}
				ZendValue zend(opts, return_value);
				GenericArgumentArray gv(v);

				Wrapper<PHPPaloSpreadsheetFuncs>::operator()(zend, opts, gv);

				// cleanup
				for (vector<GenericCell*>::iterator i = v.begin(); i != v.end(); i++) {
					delete *i;
				}
			} catch (...) {
				// cleanup
				for (vector<GenericCell*>::iterator i = v.begin(); i != v.end(); i++) {
					delete *i;
				}

				throw;
			}
		} catch (const WrongParamCountException ) {
			ZendValue(opts, return_value).setError(ErrorInfo(XLError::VALUExl, WRONG_PARAM_COUNT_EXCEPTION_ERROR_CODE, WRONG_PARAM_COUNT_EXCEPTION_ERROR_STRING));
			ZEND_WRONG_PARAM_COUNT();
		} catch (const InvalidRequestException& e) {
#undef GetMessage
			log.log(AbstractLogger::LOG_WARNING, "Invalid request occured" + e.GetMessage());
		}
	} catch (...) {
		try {
			log.log(AbstractLogger::LOG_WARNING, "Unexpected exception caught! (This should never happen).");
		} catch (...) {
		}
	}
}
