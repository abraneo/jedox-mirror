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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <memory>
#include <exception>

namespace jedox {
namespace palo {

class HttpClientResponse;
class HttpClientRequest;

typedef std::unique_ptr<HttpClientResponse> CLIENT_RESPONSE_APTR;

class HttpClient {
public:
	enum ClientMode {
		PERFORM_AUTHENTICATION = 1, PERFORM_REDIRECTION = 2, PERFORM_KEEP_ALIVE = 4, PERFORM_SSL = 32,
#ifdef HTTPCLIENT_COMPRESSION_SUPPORT
		PERFORM_CONTENT_DECODING_GZIP = 8,
		PERFORM_CONTENT_DECODING_DEFLATE = 16,
		PERFORM_CONTENT_DECODING = PERFORM_CONTENT_DECODING_GZIP | PERFORM_CONTENT_DECODING_DEFLATE,
		DEFAULT = PERFORM_AUTHENTICATION | PERFORM_CONTENT_DECODING | PERFORM_REDIRECTION | PERFORM_KEEP_ALIVE
#else
		DEFAULT = PERFORM_AUTHENTICATION | PERFORM_REDIRECTION | PERFORM_KEEP_ALIVE
#endif					 // HTTPCLIENT_COMPRESSION_SUPPORT
	};

	HttpClient(ClientMode mode = DEFAULT);

	CLIENT_RESPONSE_APTR sendRequest(bool UseHttps, const HttpClientRequest& clientRequest, const std::string &Target, unsigned int TargetPort) const;

private:

	static const unsigned int REDIRECTION_LIMIT;
	ClientMode m_ClientMode;

};

} /* palo */

} /* jedox */
#endif							 //HTTPCLIENT_H
