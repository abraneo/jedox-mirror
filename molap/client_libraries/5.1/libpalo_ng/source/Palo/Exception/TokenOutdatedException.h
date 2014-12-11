/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Florian Schaper <florian.schaper@jedox.com>
 * 
 *
 */

#ifndef TOKENOUTDATEDEXCEPTION_H
#define TOKENOUTDATEDEXCEPTION_H

#include <libpalo_ng/Palo/Exception/PaloServerException.h>

namespace jedox {
namespace palo {

class TokenOutdatedException : public PaloServerException {
public:
	explicit TokenOutdatedException(unsigned int code) :
		PaloServerException(code)
	{
	}

	TokenOutdatedException(const std::string& longText, const std::string& shortText, unsigned int code) :
		PaloServerException(longText, shortText, code)
	{
	}
};

} /* palo */
} /* jedox */
#endif							 // TOKENOUTDATEDEXCEPTION_H
