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

#ifndef URL_H
#define URL_H

#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>

#include "../../Exceptions/ErrorException.h"

#define lexicalConversion(a, b, c) lexicalConversionInt<a, b>(c, #a, #b)
template<class T, class T2> inline T lexicalConversionInt(const T2 &s, const char *tName, const char *t2Name)
{
	try {
		return boost::lexical_cast<T>(s);
	} catch (boost::bad_lexical_cast &) {
		std::stringstream str;
		str << "Server response is syntactically invalid, boost::lexical_cast returned error. Can't convert :\"" << s << "\" from " << t2Name << " to " << tName;
		std::string desc = str.str();
		throw palo::ErrorException(palo::ErrorException::ERROR_INTERNAL, "invalid response");
		throw;
	}
}

namespace palo {

typedef std::vector<std::string> PARAMETER_LIST;
typedef std::vector<std::string> QUERY_LIST;

// Relative Uniform Resource Locators (http://www.ietf.org/rfc/rfc1808.txt)

class Url {
private:
	enum Scheme {
		HTTP, HTTPS
	};

public:

	explicit Url(const std::string& url);

	explicit Url(const std::string& host, unsigned int port = 80);

	Url(const std::string& host, unsigned int port, const std::string& path);

	Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query);

	Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query, const std::string& username, const std::string& password);

	Url(const std::string& host, unsigned int port, const std::string& path, const std::string& query, const std::string& username, const std::string& password, Scheme scheme);

	const PARAMETER_LIST& getParameterList() const;

	const unsigned int getParameterSize() const;

	const std::string& getPassword() const;

	const std::string& getUsername() const;

	bool isAuthenticatable() const;

	const std::string& getPath() const;

	Scheme getScheme() const;

	const QUERY_LIST& getQueryList() const;

	size_t getQuerySize() const;

	std::string getUrl() const;

	const std::string& getQuery() const;

	const std::string& getHostname() const;

	unsigned int getPort() const;

	bool isQuerySet() const;

private:
	std::string m_Host;
	unsigned int m_Port;
	std::string m_Path;
	std::string m_Query;
	std::string m_Username;
	std::string m_Password;
	Scheme m_Scheme;
	PARAMETER_LIST m_ParameterList;
	QUERY_LIST m_QueryList;
	unsigned int m_QuerySize;
	unsigned int m_ParameterSize;
};

} /* palo */
#endif							 //URL_H
