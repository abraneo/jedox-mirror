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

#ifndef DIMENSIONNOTFOUNDEXCEPTION_H
#define DIMENSIONNOTFOUNDEXCEPTION_H

#include "ResolveException.h"

namespace jedox {
namespace palo {

class DimensionNotFoundException : public ResolveException {
public:
	DimensionNotFoundException() :
		ResolveException(MSG_ERROR_DIMENSION_NOT_FOUND, MSG_ERROR_DIMENSION_NOT_FOUND, PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND)
	{
	}
	DimensionNotFoundException(const std::string& longText) :
		ResolveException(longText, MSG_ERROR_DIMENSION_NOT_FOUND, PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND)
	{
	}
	DimensionNotFoundException(const std::string& longText, const std::string& shortText) :
		ResolveException(longText, shortText, PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND)
	{
	}
};

} /* palo */
} /* jedox */
#endif							 // DIMENSIONNOTFOUNDEXCEPTION_H
