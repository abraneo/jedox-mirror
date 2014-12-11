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

#ifndef DISPATCHER_JOB_REQUEST_H
#define DISPATCHER_JOB_REQUEST_H 1

#include "palo.h"

#include "InputOutput/Statistics.h"
#include "Logger/Logger.h"
#include "Thread/WriteLocker.h"

namespace palo {
class Job;

////////////////////////////////////////////////////////////////////////////////
/// @brief job request
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS JobRequest {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a job
	////////////////////////////////////////////////////////////////////////////////

	JobRequest() :
		name("unknown"), timer("unknown", "JobRequest") {
		done = false;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a job with a name
	////////////////////////////////////////////////////////////////////////////////

	JobRequest(const string &name) :
		name(name), timer(name, "JobRequest") {
		done = false;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes the job
	////////////////////////////////////////////////////////////////////////////////

	virtual ~JobRequest() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the name of the job request
	////////////////////////////////////////////////////////////////////////////////

	const string& getName() const {
		return name;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if request is done
	////////////////////////////////////////////////////////////////////////////////

	bool isDone() {
		return done;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets the done state
	////////////////////////////////////////////////////////////////////////////////

	virtual void handleDone(Job*) = 0;

protected:
	const string name;

	bool done;

	Statistics::Timer timer;
};
}

#endif
