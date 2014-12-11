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

#ifndef HTTPCLIENTREQUEST_H
#define HTTPCLIENTREQUEST_H

#include <map>
#include <string>

namespace jedox {
namespace palo {

class Url;

typedef std::map<std::string, std::string> HEADER_LIST;

class HttpClientRequest {
public:
	enum Method {
		POST, GET, HEAD
	};

	explicit HttpClientRequest(const Url& url, Method method = POST);
	explicit HttpClientRequest(const Url& url, HEADER_LIST& headerList, Method method = POST);

	void setMethod(Method method);

	Method getMethod() const;

	const Url& getUrl() const;

	void setUrl(const Url& url);

	const HEADER_LIST& getHeaderList() const;

private:
	HttpClientRequest& operator =(const HttpClientRequest& rhs);

private:
	const Url& m_Url;
	Method m_Method;
	HEADER_LIST m_HeaderList;
};
} /* palo */
} /* jedox */
#endif							 //HTTPCLIENTREQUEST_H
