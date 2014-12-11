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

#include "HttpClientResponse.h"
#include "Url.h"
#include "HttpClientRequest.h"

namespace palo {

HttpClientResponse::HttpClientResponse(const HttpClientRequest& clientRequest, const std::string& httpVersion, unsigned int code, const HEADER_LIST& headers, const std::vector<char>& body) :
	m_HttpClientRequest(clientRequest), m_HttpVersion(httpVersion), m_Code(code), m_Headers(headers), m_Body(body)
{
}

unsigned int HttpClientResponse::getResponseCode() const
{
	return m_Code;
}

const Url& HttpClientResponse::getRequestUrl() const
{
	return m_HttpClientRequest.getUrl();
}

const std::string& HttpClientResponse::getHeader(const std::string& key) const
{
	const static std::string empty;
	HEADER_LIST::const_iterator it(m_Headers.find(key));
	if (it != m_Headers.end()) {
		return it->second;
	}
	return empty;
}

const HEADER_LIST& HttpClientResponse::getHeaders() const
{
	return m_Headers;
}

const std::vector<char>& HttpClientResponse::getBody() const
{
	return m_Body;
}

double HttpClientResponse::getHttpVersionNumber() const
{
	return lexicalConversion(double, std::string, m_HttpVersion.substr(m_HttpVersion.find_first_of("/") + 1));
}

const std::string& HttpClientResponse::getHttpVersion() const
{
	return m_HttpVersion;
}

} /* palo */
