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

#ifndef THREAD_MUTEX_H
#define THREAD_MUTEX_H 1

#include "palo.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// UNIX implementation
// /////////////////////////////////////////////////////////////////////////////

#if ! defined(_MSC_VER)

////////////////////////////////////////////////////////////////////////////////
/// @brief condition variable
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Mutex {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a condition variable
	////////////////////////////////////////////////////////////////////////////////

	Mutex() {
		int rc;
		pthread_mutexattr_t mta;

		rc = pthread_mutexattr_init(&mta);

		if (0 != rc) {
		}

		rc = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

		if (0 != rc) {
		}

		rc = pthread_mutex_init(&mutex, &mta);

		if (0 != rc) {
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes the condition variable
	////////////////////////////////////////////////////////////////////////////////

	~Mutex() {
		pthread_mutex_destroy(&mutex);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief lock for writing
	////////////////////////////////////////////////////////////////////////////////

	void lock() {
		int rc = pthread_mutex_lock(&mutex);

		if (0 != rc) {
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief release lock
	////////////////////////////////////////////////////////////////////////////////

	void releaseLock() {
		int rc = pthread_mutex_unlock(&mutex);

		if (0 != rc) {
		}
	}

private:
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);

private:
	pthread_mutex_t mutex;
};

#endif

// /////////////////////////////////////////////////////////////////////////////
// WINDOWS implementation
// /////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)

////////////////////////////////////////////////////////////////////////////////
/// @brief condition variable
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Mutex {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a condition variable
	////////////////////////////////////////////////////////////////////////////////

	Mutex() {
		InitializeCriticalSection(&csMutex);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes the condition variable
	////////////////////////////////////////////////////////////////////////////////

	~Mutex() {
		DeleteCriticalSection(&csMutex);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief lock for writing
	////////////////////////////////////////////////////////////////////////////////

	void lock() {
		EnterCriticalSection(&csMutex);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief release lock
	////////////////////////////////////////////////////////////////////////////////

	void releaseLock() {
		LeaveCriticalSection(&csMutex);
	}

private:
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);

private:
	CRITICAL_SECTION csMutex;
};

#endif

}

#endif