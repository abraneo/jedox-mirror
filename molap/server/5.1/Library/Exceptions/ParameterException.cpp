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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Exceptions/ParameterException.h"

namespace palo {

ParameterException::ParameterException(ErrorType type, const string& message, const string& parameter, const string& value) :
	ErrorException(type, message)
{
	this->details = "parameter '" + parameter + "' value '" + value + "'";
}

ParameterException::ParameterException(ErrorType type, const string& message, const string& parameter, int value) :
	ErrorException(type, message)
{
	this->details = "parameter '" + parameter + "' value '" + StringUtils::convertToString((int32_t)value) + "'";
}

ParameterException::ParameterException(ErrorType type, const string& message, const string& parameter1, int value1, const string& parameter2, int value2) :
	ErrorException(type, message)
{
	this->details = "parameter '" + parameter1 + "' value '" + StringUtils::convertToString((int32_t)value1) + "'";
	this->details += ", parameter '" + parameter2 + "' value '" + StringUtils::convertToString((int32_t)value2) + "'";
}

ParameterException::ParameterException(ErrorType type, const string& message, const string& parameter, unsigned int value) :
	ErrorException(type, message)
{
	this->details = "parameter '" + parameter + "' value '" + StringUtils::convertToString((uint32_t)value) + "'";
}

ParameterException::ParameterException(ErrorType type, const string& message, const string& parameter, double value) :
	ErrorException(type, message)
{
	this->details = "parameter '" + parameter + "' value '" + StringUtils::convertToString(value) + "'";
}

ParameterException::ParameterException(ErrorType type, const string& cellpath, const string& message, const string& parameter, double value) :
	ErrorException(type, "cell path (" + cellpath + ") " + message)
{
	this->details = "parameter '" + parameter + "' value '" + StringUtils::convertToString(value) + "'";
}

ParameterException::ParameterException(ErrorType type, const string& cellpath, const string& message, const string& parameter, const string& value) :
	ErrorException(type, "cell path (" + cellpath + ") " + message)
{
	this->details = "parameter '" + parameter + "' value '" + value + "'";
}

}
