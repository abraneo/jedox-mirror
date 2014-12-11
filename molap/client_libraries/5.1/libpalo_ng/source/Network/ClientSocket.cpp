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

#if defined(WIN32) || defined(WIN64)
#   include <Winsock2.h>
#   include <mswsock.h>
#elif defined(__UNIX__)
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <errno.h>
#   include <fcntl.h>
#else
#error No System Definition found
#endif

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Network/SocketException.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "ClientSocket.h"

#if defined(WIN32) || defined(WIN64)
#if !defined(EWOULDBLOCK)
#   define EWOULDBLOCK WSAEWOULDBLOCK
#endif
//  windows confuses EWOULDBLOCK with EINPROGRESS so we redifine it here
#undef EINPROGRESS
#   define EINPROGRESS WSAEWOULDBLOCK

#if !defined(EINTR)
#   define EINTR WSAEINTR
#endif

#   define errno_socket WSAGetLastError()
#elif defined(__UNIX__)
#   define errno_socket errno
#   define EINTR_SOCKET EINTR
#   define EWOULDBLOCK_SOCKET EWOULDBLOCK
#   define EINPROGRESS_SOCKET EINPROGRESS
#   define closesocket close
#else
#error No System Definition found
#endif

#define TIMEOUT_MSG std::string("connection to '") + server.getIPAsString() + ":" + util::lexicalConversion(std::string, unsigned int, server.getPort() ) + std::string("' timed out.")

namespace jedox {
namespace palo {

ClientSocket::ClientSocket(Type type) :
	Socket(type)
{
}

ClientSocket::~ClientSocket() throw ()
{
}

void ClientSocket::connect(const SocketAddress &server)
{
	sockaddr_in endpoint = server.getEndpoint();

	int ret = ::connect((*m_Socket).getSocket(), (sockaddr*)&endpoint, sizeof(sockaddr_in));

	if (ret < 0) {
		throw SocketExceptionConnectionTimedOut(TIMEOUT_MSG);
	}

	m_SocketAddress.reset(new SocketAddress(server));
}

} /* palo */
} /* jedox */
