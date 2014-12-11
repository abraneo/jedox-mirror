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

#include "HttpClientRequest.h"
#include "Url.h"

namespace jedox {
namespace palo {

HttpClientRequest::HttpClientRequest(const Url& url, Method method) :
	m_Url(url), m_Method(method)
{
}

HttpClientRequest::HttpClientRequest(const Url& url, HEADER_LIST& headerList, Method method) :
	m_Url(url), m_Method(method), m_HeaderList(headerList)
{
}

void HttpClientRequest::setMethod(Method method)
{
	m_Method = method;
}

HttpClientRequest::Method HttpClientRequest::getMethod() const
{
	return m_Method;
}

const Url& HttpClientRequest::getUrl() const
{
	return m_Url;
}

void HttpClientRequest::setUrl(const Url& /* url */)
{
	// TODO implement
}

const HEADER_LIST& HttpClientRequest::getHeaderList() const
{
	return m_HeaderList;
}

} /* palo */

} /* jedox */
