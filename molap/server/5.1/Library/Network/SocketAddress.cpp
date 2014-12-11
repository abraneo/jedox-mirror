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

#include <string.h>

#if defined(WIN32) || defined(WIN64)
#   include <Winsock2.h>
#   include <WS2tcpip.h>
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#include "SocketAddress.h"
#include "../Exceptions/ErrorException.h"

namespace palo {

struct SocketAddress::SocketAddressImpl {
	SocketAddressImpl(IP netmask = 0xffffffff) :
		m_Netmask(netmask)
	{
	}
	void init(IP ip, PORT port, bool networkByteOrder = false);
	sockaddr_in m_AddrIn;
	IP m_Netmask;
	PORT m_Port;
};

void SocketAddress::SocketAddressImpl::init(IP ip, PORT port, bool networkByteOrder)
{
	memset(&m_AddrIn, 0, sizeof(m_AddrIn));
	m_AddrIn.sin_family = AF_INET;
	if (ip == 0) {
		m_AddrIn.sin_addr.s_addr = INADDR_ANY;
	} else {
		m_AddrIn.sin_addr.s_addr = networkByteOrder ? ip : htons(static_cast<USHORT> (ip));
	}
	m_AddrIn.sin_port = htons(port);
	m_Port = port;
}

SocketAddress::SocketAddress(const std::string& hostname, PORT port) :
	m_SocketAddressImpl(new SocketAddressImpl)
{
	struct addrinfo *addr;
	struct addrinfo hints;

	if (hostname.empty() != false) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "error");
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(hostname.c_str(), 0, &hints, &addr)) {
		throw CouldNotResolveHostnameException(hostname);
	}
	m_SocketAddressImpl->init(((struct sockaddr_in *)addr->ai_addr)->sin_addr.s_addr, port, true);
	freeaddrinfo(addr);
}

SocketAddress::SocketAddress(IP ip, PORT port) :
	m_SocketAddressImpl(new SocketAddressImpl)
{
	m_SocketAddressImpl->init(ip, port);
}

SocketAddress::SocketAddress(sockaddr_in saddrIn) :
	m_SocketAddressImpl(new SocketAddressImpl)
{
	m_SocketAddressImpl->m_AddrIn = saddrIn;
}

SocketAddress::SocketAddress(const SocketAddress& sock)
{
	boost::scoped_ptr<SocketAddressImpl> psockaddressimpl(new SocketAddressImpl);
	m_SocketAddressImpl.swap(psockaddressimpl);

	//  m_SocketAddressImpl.swap( boost::scoped_ptr<SocketAddressImpl>( new SocketAddressImpl ) );
	*m_SocketAddressImpl = *sock.m_SocketAddressImpl;
}

SocketAddress::~SocketAddress()
{
}

SocketAddress& SocketAddress::operator =(const SocketAddress& sock)
{
	boost::scoped_ptr<SocketAddressImpl> psockaddressimpl(new SocketAddressImpl);
	m_SocketAddressImpl.swap(psockaddressimpl);
	//  m_SocketAddressImpl.swap( boost::scoped_ptr<SocketAddressImpl>( new SocketAddressImpl ) );
	*m_SocketAddressImpl = *sock.m_SocketAddressImpl;
	return *this;
}

ULONG SocketAddress::getIPInNetworkByteOrder() const
{
	return m_SocketAddressImpl->m_AddrIn.sin_addr.s_addr;
}

ULONG SocketAddress::getNetmaskInNetworkByteOrder() const
{
	return m_SocketAddressImpl->m_Netmask;
}

sockaddr_in SocketAddress::getEndpoint() const
{
	return m_SocketAddressImpl->m_AddrIn;
}

const std::string SocketAddress::getIPAsString() const
{
	return inet_ntoa(m_SocketAddressImpl->m_AddrIn.sin_addr);
}

const std::string SocketAddress::getNetmaskAsString() const
{
	in_addr mask;

#if defined(WIN32) || defined(WIN64)
	mask.S_un.S_addr = m_SocketAddressImpl->m_Netmask;
#else
	mask.s_addr = m_SocketAddressImpl->m_Netmask;
#endif

	std::string socketaddress = inet_ntoa(mask);
	return socketaddress;
}

ULONG SocketAddress::getPortInNetworkByteOrder() const
{
	return m_SocketAddressImpl->m_AddrIn.sin_port;
}

bool SocketAddress::operator ==(const SocketAddress& sock)
{
	return memcmp((void*)&sock.m_SocketAddressImpl->m_AddrIn, (void*)&m_SocketAddressImpl->m_AddrIn, sizeof(sockaddr_in)) == 0 && sock.m_SocketAddressImpl->m_Netmask == m_SocketAddressImpl->m_Netmask;
}

PORT SocketAddress::getPort() const
{
	return m_SocketAddressImpl->m_Port;
}
} /* palo */
