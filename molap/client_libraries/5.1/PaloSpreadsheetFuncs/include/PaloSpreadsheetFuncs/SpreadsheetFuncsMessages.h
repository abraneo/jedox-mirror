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

#ifndef SPREADSHEET_FUNCS_MESSAGES_H
#define SPREADSHEET_FUNCS_MESSAGES_H

#include <PaloSpreadsheetFuncs/Messages.h>

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief SpreadsheetFuncsErrors.
 *
 *  Declares error Messages.
 */
class SpreadsheetFuncsErrors {
public:
	enum Errors {
		ERROR_INVALID_DIMENSION_ELEMENT_TYPE,
		ERROR_INVALID_CONSOLIDATION_ELEMENT_SPECIFICATION,
		ERROR_INVALID_SPLASH_MODE_SPECIFICATION,
		ERROR_NOT_ROW_OR_COLUMN,
		ERROR_INVALID_ARGUMENT_FORMAT,
		ERROR_INVALID_ARGUMENT_VALUE,
		ERROR_INVALID_ARGUMENT_TYPE,
		ERROR_NOT_FOUND,
		ERROR_NAME_NOT_UNIQUE,
		ERROR_INVALID_OFFSET,
		ERROR_IS_FIRST,
		ERROR_INVALID_BOOL_OP,
		ERROR_INVALID_COMPARE_OP,
		ERROR_INVALID_NUMBER_OF_DIMENSIONS,
		ERROR_UNKNOWN_CELL_VALUE_TYPE,
		ERROR_GENERICCELL_MISSING_DATABASE,
		ERROR_DUPLICATE_ARGUMENT,
		ERROR_FUNCTION_CALL_FAILED
	};
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief SpreadsheetFuncsMessages.
 *
 *  Defines error Messages. See base class documentation for details.
 */
class SpreadsheetFuncsMessages : public Palo::Util::Messages<SpreadsheetFuncsErrors::Errors> {
private:
	SpreadsheetFuncsMessages();

public:
	static SpreadsheetFuncsMessages& getInstance()
	{
		static SpreadsheetFuncsMessages m;
		return m;
	}
};
}
}
#endif
