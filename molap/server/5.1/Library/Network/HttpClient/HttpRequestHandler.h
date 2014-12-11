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

#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

namespace palo {

class HttpClientRequest;
class HttpClientResponse;

class HttpClientRequestHandler {
public:
	enum Action {
		CONTINUE, SKIP
	};

	virtual Action onStartRequest(HttpClientRequest& request)
	{
		return CONTINUE;
	}

	virtual Action onHandleRequest(HttpClientRequest& request, HttpClientResponse& response)
	{
		return CONTINUE;
	}

	virtual Action onResponseHeaders(HttpClientRequest& request, HttpClientResponse& response)
	{
		return CONTINUE;
	}

	virtual Action onResponseBody(HttpClientRequest& request, HttpClientResponse& response)
	{
		return CONTINUE;
	}
};
} /* palo */
#endif							 // HTTPREQUESTHANDLER_H
