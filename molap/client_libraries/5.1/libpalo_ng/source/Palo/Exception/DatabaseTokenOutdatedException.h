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

#ifndef DATABASETOKENOUTDATEDEXCEPTION_H
#define DATABASETOKENOUTDATEDEXCEPTION_H

#include "TokenOutdatedException.h"

namespace jedox {
namespace palo {

class DatabaseTokenOutdatedException : public TokenOutdatedException {
public:
	DatabaseTokenOutdatedException() :
		TokenOutdatedException(PaloExceptionFactory::ERROR_CUBE_TOKEN_OUTDATED)
	{
	}
	DatabaseTokenOutdatedException(const std::string& longText, const std::string& shortText) :
		TokenOutdatedException(longText, shortText, PaloExceptionFactory::ERROR_CUBE_TOKEN_OUTDATED)
	{
	}
};
} /* palo */
} /* jedox */
#endif							 // DATABASETOKENOUTDATEDEXCEPTION_H
