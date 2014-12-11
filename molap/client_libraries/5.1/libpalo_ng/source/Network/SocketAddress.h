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

#ifndef SOCKETADDRESS_H
#define SOCKETADDRESS_H

#include <iosfwd>
#include <string>
#include <exception>
#include <boost/scoped_ptr.hpp>

#ifdef __UNIX__
#include <sys/socket.h>
#endif

#include <libpalo_ng/config_ng.h>

struct sockaddr_in;

namespace jedox {
namespace palo {

class CouldNotResolveHostnameException : public std::exception {
public:
	CouldNotResolveHostnameException(const std::string &hostname)
	{
		message = std::string("Can't resolve hostname: ") + hostname;
	}

	virtual ~CouldNotResolveHostnameException() throw() {}

	virtual const char* what() const throw ()
	{
		return message.c_str();
	}

private:
	std::string message;
};

class SocketAddress {
public:
	SocketAddress(IP ip, PORT port = 0);
	SocketAddress(const std::string& hostname, PORT port = 0);
	SocketAddress(sockaddr_in saddrIn);
	SocketAddress(const SocketAddress& sock);
	~SocketAddress();

	const std::string getIPAsString() const;
	const std::string getNetmaskAsString() const;
	ULONG getIPInNetworkByteOrder() const;
	ULONG getNetmaskInNetworkByteOrder() const;
	ULONG getPortInNetworkByteOrder() const;
	sockaddr_in getEndpoint() const;
	PORT getPort() const;

	friend std::ostream& operator <<(std::ostream& os, const SocketAddress&)
	{
		return os; //<< sock.getIPAsString() << "/" << sock.getNetmaskAsString();
	}

	bool operator ==(const SocketAddress& sock);
	SocketAddress& operator =(const SocketAddress& sock);

private:
	struct SocketAddressImpl;
	boost::scoped_ptr<SocketAddressImpl> m_SocketAddressImpl;
};

} /* palo */
} /* jedox */
#endif							 // SOCKETADDRESS_H
