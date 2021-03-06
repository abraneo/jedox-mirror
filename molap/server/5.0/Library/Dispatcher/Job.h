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

#ifndef DISPATCHER_JOB_H
#define DISPATCHER_JOB_H 1

#include "palo.h"

#include "Dispatcher/JobRequest.h"

namespace palo {
class IoTask;

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base class for jobs
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Job {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a job
	////////////////////////////////////////////////////////////////////////////////

	Job(JobRequest* jobRequest) :
		jobRequest(jobRequest), name(jobRequest->getName()), bShutdown(false), ioTask(0)  {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes the job
	///
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Job() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes the job
	///
	/// Note that initialize is called, when no write job is running. It is however
	/// possible that there are running read jobs.
	////////////////////////////////////////////////////////////////////////////////

	virtual bool initialize() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the type of the job
	////////////////////////////////////////////////////////////////////////////////

	virtual JobType getType() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the session internal identifier
	////////////////////////////////////////////////////////////////////////////////

	virtual IdentifierType getSessionInternalId() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the session identifier
	////////////////////////////////////////////////////////////////////////////////

	virtual string getSid() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief starts working
	////////////////////////////////////////////////////////////////////////////////

	virtual void work() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief cleans up after work and delete
	////////////////////////////////////////////////////////////////////////////////

	virtual void cleanup() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets the IoTask
	////////////////////////////////////////////////////////////////////////////////
	virtual void setTask(IoTask *ioTask) {this->ioTask = ioTask;}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief getter for the job request
	////////////////////////////////////////////////////////////////////////////////

	JobRequest* getJobRequest() {
		return jobRequest;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief getter for the name
	////////////////////////////////////////////////////////////////////////////////

	const string& getName() const {
		return name;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets shutdown (currently for /server/shutdown request)
	////////////////////////////////////////////////////////////////////////////////

	void setShutdown(bool shutdown) {
		bShutdown = shutdown;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets shutdown
	////////////////////////////////////////////////////////////////////////////////

	bool getShutdown() const {
		return bShutdown;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets ioTask object
	////////////////////////////////////////////////////////////////////////////////

	IoTask *getIoTask() {return ioTask;}

private:
	JobRequest* jobRequest;
	const string& name;
	bool bShutdown;
protected:
	IoTask *ioTask;
};

}

#endif
