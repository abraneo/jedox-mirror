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

#include "Scheduler/ReadTask.h"

#include <iostream>

#include "Logger/Logger.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

ReadTask::ReadTask(socket_t readSocket) :
	IoTask(readSocket, INVALID_SOCKET)
{
}

ReadTask::~ReadTask()
{
}

// /////////////////////////////////////////////////////////////////////////////
// protected methods
// /////////////////////////////////////////////////////////////////////////////

bool ReadTask::fillReadBuffer()
{
	char buffer[100000];

#if defined(_MSC_VER)
	int nr = recv(readSocket, buffer, sizeof(buffer), 0);
#else
#if defined(__APPLE__) || defined(macintosh)
	int nr = recv(readSocket, buffer, sizeof(buffer), SO_NOSIGPIPE);
#else
	int nr = recv(readSocket, buffer, sizeof(buffer), MSG_NOSIGNAL);
#endif
#endif

	if (nr > 0) {
		readBuffer.appendText(buffer, nr);
	} else if (nr == 0 || errno_socket == ECONNRESET_SOCKET) {
		return false;
	} else if (errno_socket == EINTR_SOCKET) {
		return fillReadBuffer();
	} else if (errno_socket != EWOULDBLOCK_SOCKET) {
		Logger::debug << "read failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

		return false;
	}

	return true;
}
}
