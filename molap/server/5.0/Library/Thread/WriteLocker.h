/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#ifndef THREAD_WRITE_LOCKER_H
#define THREAD_WRITE_LOCKER_H 1

#include "palo.h"

#include "Thread/Mutex.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief write lock
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS WriteLocker {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a write lock
	////////////////////////////////////////////////////////////////////////////////

	WriteLocker(Mutex* mutex) :
		mutex(mutex) {
		mutex->lock();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes write lock
	////////////////////////////////////////////////////////////////////////////////

	~WriteLocker() {
		if (mutex != 0) {
			mutex->releaseLock();
		}
	}

private:
	WriteLocker(const WriteLocker&);
	WriteLocker& operator=(const WriteLocker&);

private:
	Mutex* mutex;
};

}

#endif
