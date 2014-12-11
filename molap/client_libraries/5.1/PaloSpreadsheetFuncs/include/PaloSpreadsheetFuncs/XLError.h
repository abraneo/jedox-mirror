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

#ifndef XLERROR_H
#define XLERROR_H

#include <string>

#include <libpalo_ng/Palo/Exception/PaloExceptionFactory.h>
#include <libpalo_ng/Palo/Exception/PaloServerException.h>

#include "InvalidRequestException.h"

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Excel style error.
 *
 *  This class provides functionality to convert exceptions into
 *  excel style errors.
 */
class XLError {
public:
	enum eXLError {
		VALUExl, REFxl, NAxl, DIVxl, NUMxl, NAMExl, NULLxl
	} Errors;

	/*! \brief Return a corresponding Excel style error string (e.g. "#VALUE").
	 */
	static const eXLError getError(const jedox::palo::PaloException& e)
	{
		return libpalo_ng_code2XLError(e.code());
	}

	/*! \brief Return the corresponding Excel style error string (e.g. "#VALUE").
	 */
	static const std::string& getString(eXLError e)
	{
		using namespace Palo::Types;

		static const std::string VALUExl = "#VALUE";
		static const std::string REFxl = "#REF";
		static const std::string NAxl = "#NA";
		static const std::string DIVxl = "#DIV/0";
		static const std::string NUMxl = "#NUM";
		static const std::string NAMExl = "#NAME";
		static const std::string NULLxl = "#NULL!";

		switch (e) {
		case XLError::VALUExl:
			return VALUExl;
		case XLError::REFxl:
			return REFxl;
		case XLError::NAxl:
			return NAxl;
		case XLError::DIVxl:
			return DIVxl;
		case XLError::NUMxl:
			return NUMxl;
		case XLError::NAMExl:
			return NAMExl;
		case XLError::NULLxl:
			return NULLxl;
		default:
			throw InvalidRequestException(CurrentSourceLocation);
		}
	}

	/*! \brief Return a corresponding Excel style error.
	 *
	 *  Code is from libpaloNG.
	 */
	static eXLError libpalo_ng_code2XLError(unsigned int code);
};
}
}
#endif
