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
 * \author Marko Stijak <mstijak@gmail.com>
 * 
 *
 */

#ifndef HTTPSCLIENTCONNECTIONPOOL_H
#define HTTPSCLIENTCONNECTIONPOOL_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <memory>

#include <boost/thread/mutex.hpp>

namespace palo {

class Url;
class HttpsClientConnection;

class HttpsClientConnectionPool : private boost::noncopyable {
protected:
	HttpsClientConnectionPool();
	~HttpsClientConnectionPool();

public:
	static HttpsClientConnectionPool &instance();

	void setProxy(const std::string& host, unsigned int port);

	std::unique_ptr<HttpsClientConnection> adoptClientConnection(const Url& url);

	void returnClientConnection(std::unique_ptr<HttpsClientConnection> &clientConnection);

private:
	struct HttpsClientConnectionPoolImpl;
	boost::scoped_ptr<HttpsClientConnectionPoolImpl> m_HttpsClientConnectionPoolImpl;
};

} /* palo */
#endif							 //HttpsClientConnectionPool_H
