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

#if defined(WIN32) || defined(WIN64)
#   pragma warning( push )
#   pragma warning( disable : 4996 )
#endif

#include <boost/iostreams/stream.hpp>

#if defined(WIN32) || defined(WIN64)
#   pragma warning( pop )
#endif

#include "HttpClientConnection.h"
#include "../SocketAddress.h"
#include "../ClientSocket.h"
#include "../TcpDevice/TcpDevice.h"

namespace palo {

HttpClientConnection::HttpClientConnection(const std::string& hostname, unsigned int port) :
	m_Hostname(hostname), m_Port(port)
{
	m_ClientSocket.disableNagleAlgorithm();
	m_ClientSocket.connect(SocketAddress(m_Hostname, m_Port));
	m_RequestCount = 0;
}

HttpClientConnection::~HttpClientConnection()
{
	try {
#if defined(WIN32) || defined(WIN64)
		if ( m_ClientSocket.isConnected() == true ) {
#endif
		m_ClientSocket.shutdownBoth();
		m_ClientSocket.close();
#if defined(WIN32) || defined(WIN64)
	}
#endif
	} catch (std::exception&) {
#ifndef _DEBUG
	} catch (...) {
#endif
	}
}

void HttpClientConnection::reset()
{
	// Close the connection if still open
	if (m_ClientSocket.isConnected() == false) {
		m_ClientSocket.close();
	}

	// Re-open the connection to the specified server
	m_ClientSocket.connect(SocketAddress(m_Hostname, m_Port));
	m_RequestCount = 0;
}

const std::string& HttpClientConnection::getHostname() const
{
	return m_Hostname;
}

unsigned int HttpClientConnection::getPort() const
{
	return m_Port;
}

void HttpClientConnection::incrementRequestCount()
{
	m_RequestCount++;
}

boost::shared_ptr<std::iostream> HttpClientConnection::getStream()
{
	return boost::shared_ptr<std::iostream>(new boost::iostreams::stream<TcpDevice>(TcpDevice(m_ClientSocket)));
}
} /* palo */
