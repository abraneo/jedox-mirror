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

#ifndef TCPDEVICE_H
#define TCPDEVICE_H

#include <ios>
#include <boost/iostreams/concepts.hpp>

using boost::iostreams::device;
using boost::iostreams::bidirectional;

namespace palo {

class ClientSocket;

class TcpDevice : public device<bidirectional> {
public:
	TcpDevice(ClientSocket& socket);

	std::streamsize read(char *s, std::streamsize n);

	std::streamsize write(const char *s, std::streamsize n);

	void close(std::ios_base::openmode mode);

	bool is_open() const;

private:
	ClientSocket& m_ClientSocket;
};

} /* palo */
#endif							 // TCPDEVICE_H
