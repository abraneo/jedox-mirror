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
 *
 */

// TODO Cleanup

#if defined(WIN32) || defined(WIN64)
#   include <Winsock2.h>
#elif defined(__UNIX__)
#   include <errno.h>
#else
#error No System definition found
#endif

#include <libpalo_ng/Network/SocketException.h>

#if defined(WIN32) || defined(WIN64)

#if !defined(ENOTSOCK)
#define ENOTSOCK        WSAENOTSOCK
#endif
#if !defined(ENOTCONN)
#define ENOTCONN        WSAENOTCONN
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT       WSAETIMEDOUT
#endif
#if !defined(ECONNABORTED)
#define ECONNABORTED    WSAECONNABORTED
#endif
#if !defined(ECONNRESET)
#define ECONNRESET      WSAECONNRESET
#endif
#if !defined(EINTR)
#define EINTR           WSAEINTR
#endif
#if !defined(ECONNREFUSED)
#define ECONNREFUSED    WSAECONNREFUSED
#endif
#if !defined(EHOSTUNREACH)
#define EHOSTUNREACH    WSAEHOSTUNREACH
#endif

#endif

namespace jedox {
namespace palo {

void SocketExceptionFactory::raise(unsigned int errorNo)
{
	switch (errorNo) {

	case ENOTSOCK:
		throw wasNotSocket;

	case ENOTCONN:
		throw socketNotConnected;

	case ETIMEDOUT:
		throw connectionTimedOut;

	case ECONNABORTED:
		throw connectionAbort;

	case ECONNRESET:
		throw connectionReset;

	case EINTR:
		throw blockingCallInterrupt;

	case ECONNREFUSED:
		throw connectionRefused;

	case PALO_NGSOCKET_HOSTINVALID:
		throw invalidHost;

	}
	throw unknownError;
}

const SocketExceptionWasNotSocket SocketExceptionFactory::wasNotSocket;
const SocketExceptionBlockingCallInterrupted SocketExceptionFactory::blockingCallInterrupt;
const SocketExceptionConnectionAbort SocketExceptionFactory::connectionAbort;
const SocketExceptionConnectionReset SocketExceptionFactory::connectionReset;
const SocketExceptionConnectionTimedOut SocketExceptionFactory::connectionTimedOut;
const SocketExceptionHostUnreachable SocketExceptionFactory::hostUnreachable;
const SocketExceptionConnectionRefused SocketExceptionFactory::connectionRefused;
const SocketExceptionUnknownError SocketExceptionFactory::unknownError;
const SocketExceptionSocketNotConnected SocketExceptionFactory::socketNotConnected;
const SocketExceptionInvalidHost SocketExceptionFactory::invalidHost;
} /* palo */
} /* jedox */
