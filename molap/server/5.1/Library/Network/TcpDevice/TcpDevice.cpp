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

#include "TcpDevice.h"
#include "../ClientSocket.h"

namespace palo {

TcpDevice::TcpDevice(ClientSocket& socket) :
	m_ClientSocket(socket)
{
}

std::streamsize TcpDevice::read(char *s, std::streamsize n)
{
	// TODO return -1 to signal end of stream
	return m_ClientSocket.read(s, n);
}

std::streamsize TcpDevice::write(const char *s, std::streamsize n)
{
	return m_ClientSocket.write(s, n);
}

void TcpDevice::close(std::ios_base::openmode /* mode */)
{
	// Don't close m_ClientSocket if you want to use Keep-Alive
	//	m_ClientSocket.close();
}

bool TcpDevice::is_open() const
{
	return false;
}

} /* palo */
