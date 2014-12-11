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

#ifndef CELL_VALUE_H
#define CELL_VALUE_H

#include <string>
#include <sstream>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Exception/LibPaloNGExceptionFactory.h>

#include "XLError.h"
#include "ErrorInfo.h"

#include "ArgumentException.h"

#include "Poolable.h"

#define MSG_NO_VALUE_YET "No value available yet"

namespace Palo {
namespace Types {

class PropertyNames {
public:
	const static std::vector<std::string> PropNames;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Contains Palo cell data.
 */
class CellValue : public Util::Poolable<CellValue> {
public:
	enum CellValueType {
		NUMERIC, STRING, ERR
	} type;

	struct CellValueUnion {
		inline CellValueUnion()
		{
		}

		std::string s;
		double d;
		ErrorInfo err;
	} val;

	long ruleId;

	jedox::palo::LOCK_STATUS lock_status;
	static const std::string strInitValue;

	inline CellValue()
	{
		type = CellValue::ERR;
		val.err = ErrorInfo(XLError::NAxl, jedox::palo::LibPaloNGExceptionFactory::PALO_NG_ERROR_NO_VALUE_YET);
		ruleId = -1;
		lock_status = jedox::palo::Unlocked;
	}

	inline CellValue(const std::string& s)
	{
		type = CellValue::STRING;
		val.d = 0;
		val.s = s;
		ruleId = -1;
		lock_status = jedox::palo::Unlocked;
	}

	inline CellValue(double d)
	{
		type = CellValue::NUMERIC;
		val.d = d;
		ruleId = -1;
		lock_status = jedox::palo::Unlocked;
	}

	inline CellValue(const ErrorInfo& ei)
	{
		type = CellValue::ERR;
		val.err = ei;
		ruleId = -1;
		lock_status = jedox::palo::Unlocked;
	}

	inline bool operator!=(const CellValue& other)
	{
		if (type == other.type) {
			switch (type) {
			case CellValue::NUMERIC:
				return val.d != other.val.d;
			case CellValue::STRING:
				return val.s != other.val.s;
			default:
				return true;
			}
		} else {
			return true;
		}
	}

	CellValue(const jedox::palo::CELL_VALUE& cv);

	CellValue(const jedox::palo::CELL_VALUE_PATH_EXPORTED& cv);

	jedox::palo::CELL_VALUE toPalo() const;
};

inline CellValue::CellValue(const jedox::palo::CELL_VALUE_PATH_EXPORTED& cv)
{
	using namespace jedox::palo;

	ruleId = -1;
	lock_status = Unlocked;

	switch (cv.type) {
	case CELL_VALUE_PATH_EXPORTED::STRING:
		type = CellValue::STRING;
		val.s = cv.val.s;
		break;

	case CELL_VALUE_PATH_EXPORTED::NUMERIC:
		type = CellValue::NUMERIC;
		val.d = cv.val.d;
		break;

	case CELL_VALUE_PATH_EXPORTED::ERROR:
		type = CellValue::ERR;
		val.err = ErrorInfo(XLError::libpalo_ng_code2XLError(cv.val.errorcode), cv.val.errorcode, cv.val.s);
		break;

	default:
		throw InvalidRequestException(CurrentSourceLocation);
	}

}

inline CellValue::CellValue(const jedox::palo::CELL_VALUE& cv)
{
	using namespace jedox::palo;

	ruleId = cv.ruleID;
	lock_status = cv.lock_status;

	switch (cv.type) {
	case CELL_VALUE::STRING:
		type = CellValue::STRING;
		val.s = cv.val.s;
		break;

	case CELL_VALUE::NUMERIC:
		type = CellValue::NUMERIC;
		val.d = cv.val.d;
		break;

	case CELL_VALUE::ERROR:
		type = CellValue::ERR;
		val.err = ErrorInfo(XLError::libpalo_ng_code2XLError(cv.val.errorcode), cv.val.errorcode, cv.val.s);
		break;

	default:
		throw InvalidRequestException(CurrentSourceLocation);
	}
}

std::ostream& operator<<(std::ostream& ostr, CellValue& cv);

enum PropertyType {
	PropertyTypeRight = 0, PropertyFormatString, PropertyCellNote
};

class CellValueWithProperties : public CellValue {
public:
	std::vector<std::string> PropValues;

	inline CellValueWithProperties(const jedox::palo::CELL_VALUE_PATH_PROPS& cv) :
			CellValue(cv), PropValues(cv.prop_vals)
	{
	}

	inline CellValueWithProperties(const jedox::palo::CELL_VALUE_PROPS& cv) :
			CellValue(cv), PropValues(cv.prop_vals)
	{
	}

	inline CellValueWithProperties(const CellValue& cv) :
			CellValue(cv)
	{
	}
};
}
}
#endif
