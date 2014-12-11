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

#ifndef HTTPCLIENTCONNECTION_H
#define HTTPCLIENTCONNECTION_H

#include <iosfwd>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "../ClientSocket.h"

namespace palo {

class HttpClientConnection : public boost::noncopyable {
public:
	HttpClientConnection(const std::string& hostname, unsigned int port);
	~HttpClientConnection();

	void reset();

	unsigned int getPort() const;

	const std::string& getHostname() const;

	unsigned int getRequestCount() const;

	void incrementRequestCount();

	boost::shared_ptr<std::iostream> getStream();

private:
	std::string m_Hostname;
	ClientSocket m_ClientSocket;
	unsigned int m_Port;
	unsigned int m_RequestCount;
};

} /* palo */
#endif							 //HTTPCLIENTCONNECTION_H
