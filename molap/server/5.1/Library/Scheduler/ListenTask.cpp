/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * 
 *
 */

#include "Scheduler/ListenTask.h"

#include <iostream>

#include "Logger/Logger.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// IoTask methods
// /////////////////////////////////////////////////////////////////////////////

socket_t ListenTask::waitConnection()
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));

	socket_t connfd = accept(readSocket, (sockaddr*)&addr, &len);

	if (connfd == INVALID_SOCKET) {
		Logger::warning << "accept failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

		return connfd;
	}

	// disable nagle's algorithm
	int n = 1;
	int res = setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char*)&n, sizeof(n));

	if (res != 0) {
		closesocket(connfd);

		Logger::warning << "setsockopt failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

		connfd = INVALID_SOCKET;
	}

	return connfd;
}

bool ListenTask::Listen(const string& address, int port)
{

	// create a new socket
	readSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (readSocket < 0) {
		Logger::error << "socket failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;
		return false;
	}

	// bind to an address or any address
	sockaddr_in addr;

	if (address.empty()) {
		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);
	} else {

		// resolve name
		struct ::hostent * sheep = ::gethostbyname(address.c_str());

		if (sheep == 0) {
			Logger::error << "cannot resolve hostname in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;
			return false;
		}

		// bind socket to an address
		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		memcpy(&(addr.sin_addr.s_addr), sheep->h_addr, sheep->h_length);
		addr.sin_port = htons(port);
	}

#if !defined(_MSC_VER)
	u_int yes = 1;
	if (0 > setsockopt(readSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes))) {
		Logger::error << "cannot setup listener in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;
		return false;
	}
#endif
	int res = ::bind(readSocket, (const sockaddr*)&addr, sizeof(addr));

	if (res < 0) {
#if defined(_MSC_VER)
		int wsaError = WSAGetLastError();
		if (wsaError == WSAEADDRINUSE) {
			Logger::error << "bind failed, port " << port << " already in use" << endl;
		} else {
#endif
		Logger::error << "bind failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;
#if defined(_MSC_VER)
		}
#endif
		closesocket(readSocket);
		readSocket = INVALID_SOCKET;

		return false;
	}

	// listen for new connection
	res = listen(readSocket, 10);

	if (res < 0) {
		closesocket(readSocket);
		readSocket = INVALID_SOCKET;

		Logger::error << "listen failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

		return false;
	}

	return true;
}
}
