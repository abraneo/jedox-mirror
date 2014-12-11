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
 * \author Marko Stijak <mstijak@gmail.com>
 * 
 *
 */

#ifndef HTTPSCLIENTCONNECTION_H
#define HTTPSCLIENTCONNECTION_H

#include <exception>
#include <iosfwd>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <openssl/ssl.h>

#include "../ClientSocket.h"

namespace palo {

class SslException : public std::exception {
public:
	SslException(std::string errormsg) :
		m_errormsg(errormsg)
	{
	}

	virtual const char *what() const throw ()
	{
		return m_errormsg.c_str();
	}

	~SslException() throw ()
	{
	}
	;

private:
	std::string m_errormsg;
};

class HttpsClientConnection : public boost::noncopyable {
public:
	HttpsClientConnection(const std::string& hostname, unsigned int port);
	~HttpsClientConnection();

	void reset();

	unsigned int getPort() const;

	const std::string& getHostname() const;

	unsigned int getRequestCount() const;

	void incrementRequestCount();

	bool hasValidCertificate() const;

	boost::shared_ptr<std::iostream> getStream();

private:
	std::string m_Hostname;
	BIO* m_Bio;
	SSL* m_Ssl;
	unsigned int m_Port;
	unsigned int m_RequestCount;
	bool m_hasValidCertificate;
};

} /* palo */
#endif							 //HTTPCLIENTCONNECTION_H
