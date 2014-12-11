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

#ifndef PHP_PALO_EXCEPTION_H
#define PHP_PALO_EXCEPTION_H

#include <PaloSpreadsheetFuncs/PSFException.h>

#include "PHPPaloMessages.h"

namespace Palo {
namespace Types {
class PHPPaloException : public PSFException {
public:
	PHPPaloException(const SourceLocation& loc, PHPPaloErrors::Errors e) :
			PSFException(loc, PHPPaloMessages::getInstance()[e])
	{
	}

	const std::string& GetType()
	{
		static const std::string type = "PHPPaloException";
		return type;
	}

	~PHPPaloException() throw ()
	{
	}
};
}
}
#endif
