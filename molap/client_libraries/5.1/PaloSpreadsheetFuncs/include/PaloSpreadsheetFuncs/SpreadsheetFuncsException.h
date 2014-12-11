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

#ifndef SPREADSHEET_FUNCS_EXCEPTION_H
#define SPREADSHEET_FUNCS_EXCEPTION_H

#include <PaloSpreadsheetFuncs/SpreadsheetFuncsMessages.h>

#define STD_EXCEPTION_ERROR_CODE -1
#define SOCKET_EXCEPTION_ERROR_CODE -2
#define EXCEPTION_ERROR_CODE -3
#define WRONG_PARAM_COUNT_EXCEPTION_ERROR_CODE -4
#define CONNECTION_EXCEPTION_ERROR_CODE -5
#define UNEXPECTED_EXCEPTION_ERROR_CODE -6

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *
 *  What the name suggests. Used mostly by SpreadsheetFuncsBase class and classed derived from it.
 */
class SpreadsheetFuncsException : public PSFException {
public:
	SpreadsheetFuncsException(const SourceLocation& loc, SpreadsheetFuncsErrors::Errors e) :
			PSFException(loc, SpreadsheetFuncsMessages::getInstance()[e])
	{
	}

	const std::string& GetType()
	{
		static const std::string type = "SpreadsheetFuncsException";
		return type;
	}

	~SpreadsheetFuncsException() throw ()
	{
	}
};
}
}
#endif
