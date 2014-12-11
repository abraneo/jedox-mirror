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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * 
 *
 */

#ifndef HTTP_SERVER_HTTP_REQUEST_HANDLER_H
#define HTTP_SERVER_HTTP_REQUEST_HANDLER_H 1

#include "palo.h"

#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpServerTask.h"
#include "Exceptions/ErrorException.h"

namespace palo {
class HttpJobRequest;

////////////////////////////////////////////////////////////////////////////////
/// @brief http request handler
///
/// A http request handler is used to answer the request from client. The
/// function handleHttpRequest is called to generate a suitable response. This
/// class is the abstract base class for all http request handlers. A request
/// handler must provide an implementation for the virtual function
/// handleHttpRequest.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpRequestHandler {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new http request handler
	///
	/// Constructor for the abstract base class. No parameters are necessary.
	////////////////////////////////////////////////////////////////////////////////

	HttpRequestHandler();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~HttpRequestHandler() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates a http response given a http request
	////////////////////////////////////////////////////////////////////////////////

	virtual HttpRequest * createHttpRequest(const string& path) {
		return new HttpRequest(path, this);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates a http response given a http request
	///
	/// This function is called to generate a response to the client. It is pure
	/// virtual, therefore each handler must provide an implementation.
	////////////////////////////////////////////////////////////////////////////////

	virtual HttpJobRequest * handleHttpRequest(HttpRequest*, const HttpServerTask*) = 0;

private:
	HttpRequestHandler(const HttpRequestHandler&);
	HttpRequestHandler& operator=(const HttpRequestHandler&);
};

}

#endif
