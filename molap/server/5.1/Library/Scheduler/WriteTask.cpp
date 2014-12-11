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

#include "Scheduler/WriteTask.h"

#include <iostream>

#include "Collections/StringBuffer.h"
#include "Logger/Logger.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

WriteTask::WriteTask(socket_t writeSocket) :
	IoTask(INVALID_SOCKET, writeSocket), writeBuffer(0), ownBuffer(true), writeLength(0)
{
}

WriteTask::~WriteTask()
{
	if (writeBuffer != 0) {
		if (ownBuffer) {
			delete writeBuffer;
		}
	}
}

// /////////////////////////////////////////////////////////////////////////////
// IoTask methods
// /////////////////////////////////////////////////////////////////////////////

bool WriteTask::canHandleWrite()
{
	if (writeBuffer == 0) {
		return false;
	}

	if (writeBuffer->length() <= writeLength) {
		return false;
	}

	return true;
}

bool WriteTask::handleWrite()
{
	if (writeBuffer == 0) {
		return true;
	}

	size_t total = writeBuffer->length() - writeLength;

	while (0 < total) {

		size_t len = total;
		//YLS: write maximum 1 MB chunks
		if (1024 * 1024 < len) {
			len = 1024 * 1024;
		}
		int nr = 0;

#if defined(_MSC_VER)
		nr = send(writeSocket, writeBuffer->begin() + writeLength, (int) len, 0);
#else
#if defined(__APPLE__) || defined(macintosh)
		nr = send(writeSocket, writeBuffer->begin() + writeLength, (int)len, SO_NOSIGPIPE);
#else
		nr = send(writeSocket, writeBuffer->begin() + writeLength, (int)len, MSG_NOSIGNAL);
#endif
#endif

		if (nr < 0) {
			if (errno_socket == EINTR_SOCKET) {
				return handleWrite();
			} else if (errno_socket != EWOULDBLOCK_SOCKET) {
				Logger::debug << "write failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

				return false;
			} else {
				nr = 0;
			}
		}

		total -= nr;

		if (total == 0) {
			if (ownBuffer) {
				delete writeBuffer;
			}

			writeBuffer = 0;
			completedWriteBuffer();
		} else {
			writeLength += nr;
		}

	}
	return true;
}

// /////////////////////////////////////////////////////////////////////////////
// protected methods
// /////////////////////////////////////////////////////////////////////////////

bool WriteTask::hasWriteBuffer() const
{
	return writeBuffer != 0;
}

void WriteTask::setWriteBuffer(StringBuffer* buffer, bool ownBuffer)
{
	writeLength = 0;

	if (buffer->empty()) {
		if (ownBuffer) {
			delete buffer;
		}

		writeBuffer = 0;
		completedWriteBuffer();
	} else {
		if (writeBuffer != 0) {
			if (this->ownBuffer) {
				delete writeBuffer;
			}
		}

		writeBuffer = buffer;
		this->ownBuffer = ownBuffer;
	}
}
}
