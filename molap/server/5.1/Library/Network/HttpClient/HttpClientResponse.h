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

#ifndef HTTPCLIENTRESPONSE_H
#define HTTPCLIENTRESPONSE_H

#include <string>
#include <vector>
#include <map>

namespace palo {

class HttpClientRequest;
class Url;

typedef std::map<std::string, std::string> HEADER_LIST;

class HttpClientResponse {
public:
	HttpClientResponse(const HttpClientRequest& clientRequest, const std::string& httpVersion, unsigned int code, const HEADER_LIST& headers, const std::vector<char>& body);

	HttpClientResponse& operator=(const HttpClientResponse& rhs);

	unsigned int getResponseCode() const;

	const Url& getRequestUrl() const;

	const HEADER_LIST& getHeaders() const;

	const std::string& getHeader(const std::string& key) const;

	const std::vector<char>& getBody() const;

	double getHttpVersionNumber() const;

	const std::string& getHttpVersion() const;

private:
	const HttpClientRequest& m_HttpClientRequest;
	const std::string m_HttpVersion;
	unsigned int m_Code;
	const HEADER_LIST m_Headers;
	const std::vector<char> m_Body;
};
} /* palo */
#endif							 //HTTPCLIENTRESPONSE_H
