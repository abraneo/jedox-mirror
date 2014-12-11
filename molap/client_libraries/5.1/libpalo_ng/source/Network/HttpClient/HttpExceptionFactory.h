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

#ifndef HTTPEXCEPTIONFACTORY_H
#define HTTPEXCEPTIONFACTORY_H

#include <exception>
#include <string>

#include <libpalo_ng/config_ng.h>

namespace jedox {
namespace palo {

class LIBPALO_NG_CLASS_EXPORT HttpExceptionFactory {
public:
	enum ErrorType {
		HTTP_CLIENT_GENERAL_ERROR = 0, HTTP_CLIENT_UNEXPECTED_ENDOFCONNECTION = 1, HTTP_CLIENT_UNDEFINED_REQUEST = 2, HTTP_CLIENT_COMPRESSION_ERROR = 3, HTTP_CLIENT_INVALID_RESPONSEHEADER = 4,
	};
	static void raise(unsigned int errorCode);
};

} /* palo */
} /* jedox */
#endif							 // HTTPEXCEPTIONFACTORY_H
