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
 *
 */

#ifndef HTTPCLIENTEXCEPTION_H
#define HTTPCLIENTEXCEPTION_H

#include <exception>
#include <string>

#include <libpalo_ng/Palo/Exception/PaloException.h>

#include "HttpExceptionFactory.h"

namespace jedox {
namespace palo {

class HttpClientException : public PaloException {
public:
	explicit HttpClientException() :
		PaloException(HttpExceptionFactory::HTTP_CLIENT_GENERAL_ERROR)
	{
		;
	}
	HttpClientException(const std::string& longText, const std::string& shortText, unsigned int type) :
		PaloException(longText, shortText, type)
	{
		;
	}
};

} /* palo */
} /* jedox */
#endif							 // HTTPCLIENTEXCEPTION_H
