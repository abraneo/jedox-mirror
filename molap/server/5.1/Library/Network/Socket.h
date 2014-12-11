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

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "../Exceptions/ErrorException.h"

#include "SocketAddress.h"

namespace palo {

class Socket {
public:
	virtual ~Socket() throw ();

	enum Type {
		TCP = 1, UDP = 2
	};

	bool isInputShutdown();
	bool isOutputShutdown();
	bool isConnected();
	bool isRoutingEnabled();
	bool isKeepAlive();
	bool isLingering();
	bool isTimeoutSet();
	bool isBlocking();

	DWORD getInputTimeout();
	DWORD getOutputTimeout();

#if defined(WIN32) || defined(WIN64)
	DWORD getConnectionTime();
#endif

	USHORT getLingerTime();

	virtual void close();
	void setLinger(bool activate, USHORT timeout);
	void setDontLinger();
	void disableTimeout();
	void enableRouting();
	void disableRouting();
	void enableKeepalive();
	void disableKeepalive();
	void setInputTimeout(DWORD timeout);
	void setOutputTimeout(DWORD timeout);
	void shutdownOutput();
	void shutdownInput();
	void shutdownBoth();
	void disableNagleAlgorithm();
	void enableNagleAlgorithm();

	boost::shared_ptr<SocketAddress> getAddress() const;
	std::streamsize write(const char* data, std::streamsize size);
	std::streamsize read(char* buffer, std::streamsize size);

protected:
	explicit Socket(Type type = TCP);
	Socket(SOCKET socket, const SocketAddress& addr, Type type = TCP);

	template<typename T> int getSocketOption(long option, T& value, int level = 0);
	template<typename T> void setSocketOption(long option, T& value, int level = 0);

	boost::shared_ptr<SocketAddress> m_SocketAddress;

	class SocketGuard : private boost::noncopyable {
	public:
		explicit SocketGuard(SOCKET socket = 0, Socket::Type type = Socket::TCP);
		~SocketGuard() throw ();
		const SOCKET& getSocket() const;
		Socket::Type getSockType() const;

	private:
		SOCKET m_Socket;
		Socket::Type m_Type;
	};

	boost::scoped_ptr<SocketGuard> m_Socket;

private:

	bool m_WouldBlock;
};
} /* palo */
#endif
