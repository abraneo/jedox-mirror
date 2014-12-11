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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * 
 *
 */

#ifndef EXCEPTIONS_PARAMETER_EXCEPTION_H
#define EXCEPTIONS_PARAMETER_EXCEPTION_H 1

#include "palo.h"

#include <sstream>

#include "Collections/StringUtils.h"

#include "Exceptions/ErrorException.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief palo parameter exception
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ParameterException : public ErrorException {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ParameterException(ErrorType type, const string& message, const string& parameter, const string& value);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ParameterException(ErrorType type, const string& message, const string& parameter, int value);
	ParameterException(ErrorType type, const string& message, const string& parameter1, int value1, const string& parameter2, int value2);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ParameterException(ErrorType type, const string& message, const string& parameter, unsigned int value);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ParameterException(ErrorType type, const string& message, const string& parameter, double value);

	ParameterException(ErrorType type, const string& cellpath, const string& message, const string& parameter, double value);
	ParameterException(ErrorType type, const string& cellpath, const string& message, const string& parameter, const string& value);

};

}

#endif
