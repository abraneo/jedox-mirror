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

#ifndef SCHEDULER_JOB_TASK_H
#define SCHEDULER_JOB_TASK_H 1

#include "palo.h"

#include "Logger/Logger.h"
#include "Scheduler/Task.h"

namespace palo {

class JobRequest;

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS JobTask : virtual public Task {
public:
	JobTask() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called by scheduler to check if the task is finished
	///
	/// This function is used by the scheduler to check if the task is finished.
	////////////////////////////////////////////////////////////////////////////////

	virtual bool isJobDone() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief
	///
	////////////////////////////////////////////////////////////////////////////////

	virtual JobRequest* getJobRequest() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief
	///
	////////////////////////////////////////////////////////////////////////////////

	virtual void handleDone() = 0;

};

}

#endif
