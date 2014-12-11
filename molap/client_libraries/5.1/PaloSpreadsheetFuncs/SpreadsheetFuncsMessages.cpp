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

#include <PaloSpreadsheetFuncs/SpreadsheetFuncsMessages.h>

using namespace Palo::Types;

SpreadsheetFuncsMessages::SpreadsheetFuncsMessages()
{
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_DIMENSION_ELEMENT_TYPE] = "Invalid dimension element type.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_CONSOLIDATION_ELEMENT_SPECIFICATION] = "Invalid consolidation element specification given.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_SPLASH_MODE_SPECIFICATION] = "Invalid splash mode specified.";
	msgs[SpreadsheetFuncsErrors::ERROR_NOT_ROW_OR_COLUMN] = "Row or column expected.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_FORMAT] = "Parameter has invalid format.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE] = "Parameter has invalid value.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_TYPE] = "Parameter has invalid type.";
	msgs[SpreadsheetFuncsErrors::ERROR_NOT_FOUND] = "A requested object could not be found.";
	msgs[SpreadsheetFuncsErrors::ERROR_NAME_NOT_UNIQUE] = "A specified name is ambiguous.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_OFFSET] = "Invalid offset specified.";
	msgs[SpreadsheetFuncsErrors::ERROR_IS_FIRST] = "Cannot go before first element.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_BOOL_OP] = "Invalid boolean operator.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_COMPARE_OP] = "Invalid comparison operator.";
	msgs[SpreadsheetFuncsErrors::ERROR_INVALID_NUMBER_OF_DIMENSIONS] = "The number of dimensions does not match.";
	msgs[SpreadsheetFuncsErrors::ERROR_UNKNOWN_CELL_VALUE_TYPE] = "Unknown cell value type.";
	msgs[SpreadsheetFuncsErrors::ERROR_GENERICCELL_MISSING_DATABASE] = "No database specified in connection string.";
	msgs[SpreadsheetFuncsErrors::ERROR_DUPLICATE_ARGUMENT] = "An option or argument was specified more than once.";
	msgs[SpreadsheetFuncsErrors::ERROR_FUNCTION_CALL_FAILED] = "An external function/API call has failed or returned an unexpected result.";
}
