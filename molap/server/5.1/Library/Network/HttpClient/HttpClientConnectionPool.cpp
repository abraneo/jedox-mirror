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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#include <map>
#include <stack>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include "HttpClientConnectionPool.h"
#include "HttpClientConnection.h"
#include "Url.h"

#define KEY_SEPERATOR   ":"

namespace palo {

// TODO Use DQUEUE for client connections
typedef std::map<std::string, std::stack<HttpClientConnection*> > CONNECTION_MAP;

struct HttpClientConnectionPool::HttpClientConnectionPoolImpl {

	~HttpClientConnectionPoolImpl()
	{
		try {
			// Cleanup all connections
			while (m_ConnectionMap.empty() == false) {
				std::stack<HttpClientConnection*> &stack = (m_ConnectionMap.begin())->second;
				// Clean all connection pointers
				while (stack.empty() == false) {
					// destructors should never throw.
					delete stack.top();
					stack.pop();
				}
				// Erase connection entry
				m_ConnectionMap.erase(m_ConnectionMap.begin());
			}
		} catch (std::exception&) {
			// TODO Write to log that something went wrong here
#ifndef _DEBUG
		} catch (...) {
			// TODO Write to log that something went wrong here
#endif
		}
	}

	void returnClientConnection(std::unique_ptr<HttpClientConnection> &clientConnection)
	{
		// Create the key for the map-entry
		std::string key;
		std::string seperator(KEY_SEPERATOR);
		key.append(clientConnection->getHostname()).append(seperator).append(lexicalConversion(std::string, unsigned int, clientConnection->getPort()));
		{
			// Lock the map for exclusive access
			boost::mutex::scoped_lock guard(m_lockConnectionList);
			// If the push fails we will still destruct the connection
			m_ConnectionMap[key].push(clientConnection.get());
			// safe to release the unique_ptr, since no exception occurred
			clientConnection.release();
		}
	}

	std::unique_ptr<HttpClientConnection> adoptClientConnection(const Url& url)
	{
		std::unique_ptr<HttpClientConnection> returnValue;

		std::string key;
		std::string seperator(KEY_SEPERATOR);
		key.append(url.getHostname()).append(seperator).append(lexicalConversion(std::string, unsigned int, url.getPort()));
		bool noConnectionsInPool = false;
		{
			// Lock the map for exclusive access
			boost::mutex::scoped_lock guard(m_lockConnectionList);
			// Retrieve a connection from the pool, if available
			noConnectionsInPool = m_ConnectionMap[key].empty();
			if (noConnectionsInPool == false) {
				returnValue = std::unique_ptr<HttpClientConnection>(m_ConnectionMap[key].top());
				m_ConnectionMap[key].pop();
			}
		}
		// If there was no connection available create a new one
		if (noConnectionsInPool == true) {
			returnValue = std::unique_ptr<HttpClientConnection>(new HttpClientConnection(url.getHostname(), url.getPort()));
		}
		returnValue->incrementRequestCount();
		return returnValue;
	}

	boost::mutex m_lockConnectionList;
	CONNECTION_MAP m_ConnectionMap;
};

HttpClientConnectionPool::~HttpClientConnectionPool()
{
}

HttpClientConnectionPool::HttpClientConnectionPool() :
	m_HttpClientConnectionPoolImpl(new HttpClientConnectionPoolImpl)
{
}

void HttpClientConnectionPool::setProxy(const std::string& /* host */, unsigned int /* port */)
{
	// TODO To be implemented
}

std::unique_ptr<HttpClientConnection> HttpClientConnectionPool::adoptClientConnection(const Url& url)
{
	return m_HttpClientConnectionPoolImpl->adoptClientConnection(url);
}

void HttpClientConnectionPool::returnClientConnection(std::unique_ptr<HttpClientConnection> &clientConnection)
{
	m_HttpClientConnectionPoolImpl->returnClientConnection(clientConnection);
}

HttpClientConnectionPool &HttpClientConnectionPool::instance()
{
	static HttpClientConnectionPool connectionPool;
	return connectionPool;
}

} /* palo */
