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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_SVS_RESTART_JOB_H
#define PALO_JOBS_SVS_RESTART_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief super vision server info
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SvsRestartJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new SvsRestartJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	SvsRestartJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		if (User::checkUser(user) != 0 && user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)(user->getId()));
		}

		Context *context = Context::getContext();

		server = context->getServerCopy();
		svsStatusChange st = server->svsIsStopped() ? SVS_START : SVS_RESTART;

		if (jobRequest->mode == 1) {
			// stop required
			if (server->svsIsStopped() || !server->svsConfigured()) {
				// already stopped or not configured to start at all
				generateOkResponse(server);
				return;
			} else {
				st = SVS_STOP;
			}
		} else if (!server->svsConfigured()) {
			throw ErrorException(ErrorException::ERROR_WORKER_OPERATION, "No SVS processes configured to start.");
		}

		context->setSvsChangeStatusContext(st);
		context->setPesimistic();

		server = context->getServerCopy();
		bool ret = server->commit();
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "SvsRestartJob failed. Internal error occured.");
		}

		generateOkResponse(server);
	}
};

}

#endif
