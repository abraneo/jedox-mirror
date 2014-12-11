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

#include "Url.h"

namespace jedox {
namespace palo {

Url::Url(const std::string& /* url */)
{
}

Url::Url(const std::string& host, unsigned int port) :
	m_Host(host), m_Port(port), m_Path("/"), m_Scheme(HTTP)
{
}

Url::Url(const std::string& host, unsigned int port, const std::string& path) :
	m_Host(host), m_Port(port), m_Path(path), m_Scheme(HTTP)
{
}

Url::Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query) :
	m_Host(host), m_Port(port), m_Path(path), m_Query(query), m_Scheme(HTTP)
{
}

Url::Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query, const std::string& username, const std::string& password) :
	m_Host(host), m_Port(port), m_Path(path), m_Query(query), m_Username(username), m_Password(password), m_Scheme(HTTP)
{
}

Url::Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query, const std::string& username, const std::string& password, Scheme scheme) :
	m_Host(host), m_Port(port), m_Path(path), m_Query(query), m_Username(username), m_Password(password), m_Scheme(scheme)
{
}

const std::string& Url::getQuery() const
{
	return m_Query;
}

std::string Url::getUrl() const
{
	static std::string empty;
	return empty;
}

const PARAMETER_LIST& Url::getParameterList() const
{
	return m_ParameterList;
}

const std::string& Url::getPath() const
{
	return m_Path;
}

const std::string& Url::getUsername() const
{
	return m_Username;
}

const std::string& Url::getPassword() const
{
	return m_Password;
}

Url::Scheme Url::getScheme() const
{
	return m_Scheme;
}

const QUERY_LIST& Url::getQueryList() const
{
	return m_QueryList;
}

const std::string& Url::getHostname() const
{
	return m_Host;
}

unsigned int Url::getPort() const
{
	return m_Port;
}

size_t Url::getQuerySize() const
{
	return m_QueryList.size();
}

bool Url::isQuerySet() const
{
	return m_Query.empty() == false;
}

bool Url::isAuthenticatable() const
{
	return m_Username.empty() == false;
}

} /* palo */
} /* jedox */
