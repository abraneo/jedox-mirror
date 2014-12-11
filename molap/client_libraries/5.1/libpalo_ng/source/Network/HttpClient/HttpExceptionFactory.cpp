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

#include "HttpExceptionFactory.h"
#include "HttpClientException.h"

namespace jedox {
namespace palo {

void HttpExceptionFactory::raise(unsigned int errorCode)
{

	switch (errorCode) {

	case HTTP_CLIENT_COMPRESSION_ERROR:
		throw HttpClientException("A HTTP-compression error occurred", "Compression problem", HTTP_CLIENT_COMPRESSION_ERROR);

	case HTTP_CLIENT_UNDEFINED_REQUEST:
		throw HttpClientException("The requested HTTP-operation is not supported", "Undefined operation", HTTP_CLIENT_UNDEFINED_REQUEST);

	case HTTP_CLIENT_UNEXPECTED_ENDOFCONNECTION:
		throw HttpClientException("The HTTP-Connection ended unexpectedly", "Connection End", HTTP_CLIENT_UNEXPECTED_ENDOFCONNECTION);

	case HTTP_CLIENT_INVALID_RESPONSEHEADER:
		throw HttpClientException("The response header could not be read", "Invalid response header", HTTP_CLIENT_INVALID_RESPONSEHEADER);

	default:
		//Throw default (see constructor)
		throw HttpClientException();
	}
}
} /* palo */
} /* jedox */
