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

#include <string.h>

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
#error No System definition found
#endif

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Network/SocketException.h>

#include "Socket.h"
#include "SocketAddress.h"

#if defined(WIN32) || defined(WIN64)
#   define errno_socket WSAGetLastError()

#if !defined(EWOULDBLOCK)
#   define EWOULDBLOCK  WSAEWOULDBLOCK
#endif

#if !defined(EINTR)
#   define EINTR WSAEINTR
#endif

#if !defined(EMSGSIZE)
#   define EMSGSIZE     WSAEMSGSIZE
#endif

typedef int socket_len;

#elif defined(__UNIX__)
#   define errno_socket         errno
#   define closesocket          close

/* the "socket-constructor" returns -1 if no socket could be opened */
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1

#   define SD_SEND      SHUT_WR
#   define SD_RECEIVE   SHUT_WR
#   define SD_BOTH      SHUT_RDWR
#   define socket_len   socklen_t

typedef struct timeval TIMEVAL;

#else
#error No System definition found
#endif


/*
 *	TODO I wrote this some years back. Needs an rewrite.
 *
 */

namespace jedox {
namespace palo {

Socket::SocketGuard::SocketGuard(SOCKET socket, Socket::Type type) :
	m_Socket(socket), m_Type(type)
{
	if (!socket) {
		m_Socket = ::socket(AF_INET, m_Type == Socket::TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
		if (m_Socket == INVALID_SOCKET) {
			SocketExceptionFactory::raise( errno_socket);
		}
	}
}

Socket::SocketGuard::~SocketGuard() throw ()
{
	if (m_Socket != 0) {
		::closesocket(m_Socket);
		m_Socket = 0;
	}
}

const SOCKET& Socket::SocketGuard::getSocket() const
{
	return m_Socket;
}

Socket::Type Socket::SocketGuard::getSockType() const
{
	return m_Type;
}

Socket::Socket(Type type) :
	m_Socket(new SocketGuard(0, type)), m_WouldBlock(true)
{
}

// TODO SocketType ermitteln
Socket::Socket(SOCKET socket, const SocketAddress& addr, Type type) :
	m_SocketAddress(new SocketAddress(addr)), m_Socket(new SocketGuard(socket, type)), m_WouldBlock(true)
{
}

Socket::~Socket() throw ()
{
}

void Socket::close()
{
	boost::scoped_ptr<SocketGuard> psocketguard(new SocketGuard(0, (*m_Socket).getSockType()));
	m_Socket.swap(psocketguard);
}

template<typename T> void Socket::setSocketOption(long option, T& value, int level /*= 0*/)
{
	int errorCode = setsockopt((*m_Socket).getSocket(), level == 0 ? SOL_SOCKET : level, option, reinterpret_cast<char*> (&value), sizeof(value));
	if (errorCode == SOCKET_ERROR) {
		SocketExceptionFactory::raise( errno_socket);
	}
}

template<typename T> int Socket::getSocketOption(long option, T& value, int level /*= 0*/)
{
	int size = sizeof(T);
	int errorCode = getsockopt((*m_Socket).getSocket(), level == 0 ? SOL_SOCKET : level, option, reinterpret_cast<char*> (&value), (socket_len*)&size);
	if (errorCode == SOCKET_ERROR) {
		SocketExceptionFactory::raise( errno_socket);
	}
	return errorCode;
}

void Socket::setLinger(bool activate, USHORT timeout)
{
	linger ln;
	ln.l_linger = timeout;
	ln.l_onoff = activate ? 1 : 0;
	setSocketOption(SO_LINGER, ln);
}

USHORT Socket::getLingerTime()
{
	linger ln;
	getSocketOption(SO_LINGER, ln);
	return ln.l_linger;
}

bool Socket::isLingering()
{
	return getLingerTime() != 0;
}

void Socket::disableNagleAlgorithm()
{
	// disable nagle's algorithm for TCP
	if ((*m_Socket).getSockType() == TCP) {
		int n = 0;
		setSocketOption(TCP_NODELAY, n, IPPROTO_TCP);
	}
}

void Socket::enableNagleAlgorithm()
{
	// enable nagle's algorithm for TCP
	if ((*m_Socket).getSockType() == TCP) {
		int n = 1;
		setSocketOption(TCP_NODELAY, n, IPPROTO_TCP);
	}
}

void Socket::disableRouting()
{
	DWORD routing = 0;
	setSocketOption(SO_DONTROUTE, routing);
}

void Socket::enableRouting()
{
	DWORD routing = 1;
	setSocketOption(SO_DONTROUTE, routing);
}

bool Socket::isRoutingEnabled()
{
	DWORD enabled;
	getSocketOption(SO_DONTROUTE, enabled);
	return enabled != 0;
}

void Socket::enableKeepalive()
{
	DWORD keepAlive = 1;
	setSocketOption(SO_KEEPALIVE, keepAlive);
}

void Socket::disableKeepalive()
{
	DWORD keepAlive = 0;
	setSocketOption(SO_KEEPALIVE, keepAlive);
}

bool Socket::isKeepAlive()
{
	DWORD keepAlive = 0;
	getSocketOption(SO_KEEPALIVE, keepAlive);
	return keepAlive != 0;
}

void Socket::disableTimeout()
{
	setOutputTimeout(0);
	setInputTimeout(0);
}

void Socket::setOutputTimeout(DWORD timeout)
{
	setSocketOption(SO_SNDTIMEO, timeout);
}

void Socket::setInputTimeout(DWORD timeout)
{
	setSocketOption(SO_RCVTIMEO, timeout);
}

bool Socket::isTimeoutSet()
{
	return getOutputTimeout() != 0 || getInputTimeout() != 0;
}

DWORD Socket::getOutputTimeout()
{
	DWORD timeout = 0;
#if defined(WIN32) || defined(WIN64)
	getSocketOption( SO_SNDTIMEO, timeout );
#endif
	return timeout;
}

DWORD Socket::getInputTimeout()
{
	DWORD timeout;
	getSocketOption(SO_RCVTIMEO, timeout);
	return timeout;
}

#if defined(WIN32) || defined(WIN64)
DWORD Socket::getConnectionTime() {
	DWORD connectionTime;
	getSocketOption( SO_CONNECT_TIME, connectionTime );
	return connectionTime;
}
#endif

void Socket::shutdownOutput()
{
	shutdown((*m_Socket).getSocket(), SD_SEND);
}

void Socket::shutdownInput()
{
	shutdown((*m_Socket).getSocket(), SD_RECEIVE);
}

void Socket::shutdownBoth()
{
	shutdown((*m_Socket).getSocket(), SD_BOTH);
}

bool Socket::isInputShutdown()
{
	fd_set readable;
	FD_ZERO(&readable);
	FD_SET((*m_Socket).getSocket(), &readable);

	TIMEVAL timeout;
	memset(&timeout, 0, sizeof(TIMEVAL));

	int errorCode = select((int)(*m_Socket).getSocket() + 1, &readable, 0, 0, &timeout);
	if (errorCode == SOCKET_ERROR) {
		SocketExceptionFactory::raise( errno_socket);
	}
	return !FD_ISSET((*m_Socket).getSocket(), &readable);
}

bool Socket::isConnected()
{
	if (isInputShutdown() == false) {
		char c;
		bool closed = (recv((*m_Socket).getSocket(), &c, 1, MSG_PEEK) == 0);
		return closed;
	}
	return false;
}

bool Socket::isOutputShutdown()
{
	fd_set writable;
	FD_ZERO(&writable);
	FD_SET((*m_Socket).getSocket(), &writable);

	TIMEVAL timeout;
	memset(&timeout, 0, sizeof(TIMEVAL));

	int errorCode = select((int)(*m_Socket).getSocket() + 1, 0, &writable, 0, &timeout);
	if (errorCode == SOCKET_ERROR) {
		SocketExceptionFactory::raise( errno_socket);
	}
	return !FD_ISSET((*m_Socket).getSocket(), &writable);
}

boost::shared_ptr<SocketAddress> Socket::getAddress() const
{
	return m_SocketAddress;
}

std::streamsize Socket::write(const char* data, std::streamsize size)
{
	int bytesSend = ::send((*m_Socket).getSocket(), data, static_cast<int> (size), 0);
	if (bytesSend < 0) {
		if ((bytesSend = errno_socket) != EWOULDBLOCK) {
			SocketExceptionFactory::raise( errno_socket);
		}
		bytesSend = 0;
	}
	return bytesSend;
}

std::streamsize Socket::read(char* buffer, std::streamsize size)
{
	int ret = 0;
	do {
		ret = recv((*m_Socket).getSocket(), buffer, (UINT)size, 0);
	} while (ret == SOCKET_ERROR && errno_socket == EWOULDBLOCK);
	return ret;
}

bool Socket::isBlocking()
{
	return m_WouldBlock;
}

template int Socket::getSocketOption(long, int&, int);

} /* palo */
} /* jedox */
