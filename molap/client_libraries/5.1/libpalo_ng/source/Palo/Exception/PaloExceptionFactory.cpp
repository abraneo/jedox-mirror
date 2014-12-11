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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#include <libpalo_ng/Palo/Exception/PaloExceptionFactory.h>
#include <libpalo_ng/Palo/Exception/DatabaseNotFoundException.h>
#include <libpalo_ng/Palo/Exception/CubeNotFoundException.h>
#include <libpalo_ng/Palo/Exception/DimensionNotFoundException.h>
#include <libpalo_ng/Palo/Exception/ElementNotFoundException.h>

#include "ServerTokenOutdatedException.h"
#include "DatabaseTokenOutdatedException.h"
#include "DimensionTokenOutdatedException.h"
#include "CubeTokenOutdatedException.h"

namespace jedox {
namespace palo {
void PaloExceptionFactory::raise(unsigned int errorCode, const std::string& shortText, const std::string& longText)
{

	if (errorCode < 0) {
		throw PaloServerException(longText, shortText, errorCode);
	} else {
		switch (errorCode) {
			case ERROR_DATABASE_NOT_FOUND:
				throw DatabaseNotFoundException(longText, shortText);

			case ERROR_DIMENSION_NOT_FOUND:
				throw DimensionNotFoundException(longText, shortText);

			case ERROR_ELEMENT_NOT_FOUND:
				throw ElementNotFoundException(longText, shortText);

			case ERROR_CUBE_NOT_FOUND:
				throw CubeNotFoundException(longText, shortText);

			case ERROR_SERVER_TOKEN_OUTDATED:
				throw ServerTokenOutdatedException(longText, shortText);

			case ERROR_DATABASE_TOKEN_OUTDATED:
				throw DatabaseTokenOutdatedException(longText, shortText);

			case ERROR_DIMENSION_TOKEN_OUTDATED:
				throw DimensionTokenOutdatedException(longText, shortText);

			case ERROR_CUBE_TOKEN_OUTDATED:
				throw CubeTokenOutdatedException(longText, shortText);

			default:
				throw PaloServerException(longText, shortText, errorCode);
		}
	}
	// TODO Exception does a strdup?
	std::string unknown("Unknown exception requested from factory! (");
	unknown.append(longText).append(")");
#if defined(WIN32) || defined(WIN64)
	throw std::exception( unknown.c_str() );
#else
	throw std::exception();
#endif
}

} /* palo */
} /* jedox */
