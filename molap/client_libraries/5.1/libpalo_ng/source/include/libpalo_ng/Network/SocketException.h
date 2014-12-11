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

#ifndef SOCKETEXCEPTION_H
#define SOCKETEXCEPTION_H

#include <exception>
#include <string>

namespace jedox {
namespace palo {

// TODO extend Exceptions
enum PALONGSOCKETERROR {
	PALO_NGSOCKET_HOSTINVALID = 0
};

class SocketException : public std::exception {
};

class SocketExceptionWasNotSocket : public SocketException {
public:
	SocketExceptionWasNotSocket()
	{
	}

	virtual const char *what() const throw ()
	{
		return "The descriptor is not a socket.";
	}
};

class SocketExceptionBlockingCallInterrupted : public SocketException {
public:
	SocketExceptionBlockingCallInterrupted()
	{
	}

	virtual const char *what() const throw ()
	{
		return "A blocking procedure has been interrupted.";
	}
};

class SocketExceptionConnectionAbort : public SocketException {
public:
	SocketExceptionConnectionAbort()
	{
	}

	virtual const char *what() const throw ()
	{
		return "An existing Connection has been software-terminated by the host.";
	}
};

class SocketExceptionConnectionReset : public SocketException {
public:
	SocketExceptionConnectionReset()
	{
	}

	virtual const char *what() const throw ()
	{
		return "An existing connection has been closed by the Remotehost.";
	}
};

class SocketExceptionConnectionTimedOut : public SocketException {
public:
	SocketExceptionConnectionTimedOut(const std::string& msg) :
		m_msg(msg)
	{
	}

	SocketExceptionConnectionTimedOut() :
		m_msg("connection timed out.")
	{
	}

	~SocketExceptionConnectionTimedOut() throw ()
	{
	}

	virtual const char *what() const throw ()
	{
		return m_msg.c_str();
	}

private:
	std::string m_msg;
};

class SocketExceptionHostUnreachable : public SocketException {
public:
	SocketExceptionHostUnreachable()
	{
	}

	virtual const char *what() const throw ()
	{
		return "targethost not available";
	}
};

class SocketExceptionConnectionRefused : public SocketException {
public:
	SocketExceptionConnectionRefused()
	{
	}

	virtual const char *what() const throw ()
	{
		return "connection refused (hostname or port invalid)";
	}
};

class SocketExceptionUnknownError : public SocketException {
public:
	SocketExceptionUnknownError()
	{
	}

	virtual const char *what() const throw ()
	{
		return "unknown socket error";
	}
};

class SocketExceptionSocketNotConnected : public SocketException {
public:
	SocketExceptionSocketNotConnected()
	{
	}

	virtual const char *what() const throw ()
	{
		return "could not connect to remote point";
	}
};

class SocketExceptionInvalidHost : public SocketException {
public:
	SocketExceptionInvalidHost()
	{
	}

	virtual const char *what() const throw ()
	{
		return "The host-field is invalid or empty";
	}
};

class SocketExceptionFactory {
public:
	static void raise(unsigned int errorNo);

	static const SocketExceptionWasNotSocket wasNotSocket;
	static const SocketExceptionBlockingCallInterrupted blockingCallInterrupt;
	static const SocketExceptionConnectionAbort connectionAbort;
	static const SocketExceptionConnectionReset connectionReset;
	static const SocketExceptionConnectionTimedOut connectionTimedOut;
	static const SocketExceptionHostUnreachable hostUnreachable;
	static const SocketExceptionConnectionRefused connectionRefused;
	static const SocketExceptionUnknownError unknownError;
	static const SocketExceptionSocketNotConnected socketNotConnected;
	static const SocketExceptionInvalidHost invalidHost;
};
} /* palo */
} /* jedox */
#endif
