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

#include <PaloSpreadsheetFuncs/CellValue.h>
#include <PaloSpreadsheetFuncs/InvalidRequestException.h>

using namespace Palo::Types;

std::vector<std::string> InitializePropNames()
{
	std::vector < std::string > v(3);
	v[PropertyTypeRight] = "#_Rights";
	v[PropertyFormatString] = "FormatString";
	v[PropertyCellNote] = "CellNote";
	return std::vector < std::string > (v);
}

const std::vector<std::string> PropertyNames::PropNames = InitializePropNames();
const std::string CellValue::strInitValue(MSG_NO_VALUE_YET " (initial)");

jedox::palo::CELL_VALUE CellValue::toPalo() const
{
	using namespace jedox::palo;

	CELL_VALUE cv;

	cv.ruleID = ruleId;
	cv.lock_status = lock_status;

	if (type == CellValue::STRING) {
		cv.type = CELL_VALUE::STRING;
		cv.val.s = val.s;
	} else if (type == CellValue::NUMERIC) {
		cv.type = CELL_VALUE::NUMERIC;
		cv.val.d = val.d;
	} else {
		throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_TYPE);
	}

	return cv;
}

namespace Palo {
namespace Types {
std::ostream& operator<<(std::ostream& ostr, CellValue& cv)
{
	switch (cv.type) {
	case CellValue::ERR:
		ostr << cv.val.err.have_desc ? cv.val.err.desc : "Unknown error.";
		break;

	case CellValue::NUMERIC:
		ostr << cv.val.d;
		break;

	case CellValue::STRING:
		ostr << cv.val.s;
		break;

	default:
		ostr << "Unknown cell value type!";
		break;
	}

	return ostr;
}
}
}

#include "Poolable.ipp"
template class Palo::Util::Poolable<CellValue>;
