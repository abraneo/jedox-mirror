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

#ifndef SCHEDULER_WRITE_TASK_H
#define SCHEDULER_WRITE_TASK_H 1

#include "palo.h"

#include "Scheduler/IoTask.h"

namespace palo {
struct StringBuffer;

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base class for output tasks
///
/// The class provides an abstract base class for all output tasks. In addition
/// to the callbacks and methods defined by IoTask, it defines a virtual method
/// compledWriteBuffer, which is called by handleWrite as soon as the current
/// write buffer has been sent to the client, and a member writeBuffer which
/// is used to store the data.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS WriteTask : virtual public IoTask {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new output task
	////////////////////////////////////////////////////////////////////////////////

	WriteTask(socket_t writeSocket);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes an output task
	////////////////////////////////////////////////////////////////////////////////

	~WriteTask();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool canHandleWrite();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool handleWrite();

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks for presence of an active write buffer
	////////////////////////////////////////////////////////////////////////////////

	bool hasWriteBuffer() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets an active write buffer
	////////////////////////////////////////////////////////////////////////////////

	void setWriteBuffer(StringBuffer*, bool ownBuffer = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called if write buffer has been sent
	///
	/// This called is called if the current write buffer has been sent
	/// completly to the client.
	////////////////////////////////////////////////////////////////////////////////

	virtual void completedWriteBuffer() = 0;

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the current write buffer
	////////////////////////////////////////////////////////////////////////////////

	StringBuffer * writeBuffer;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief if true, the resource writeBuffer is owned by the write task
	///
	/// If true, the writeBuffer is deleted as soon as it has been sent to the
	/// client. If false, the writeBuffer is keep alive.
	////////////////////////////////////////////////////////////////////////////////

	bool ownBuffer;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief number of bytes already written
	////////////////////////////////////////////////////////////////////////////////

	size_t writeLength;
};

}

#endif
