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

#ifndef PALO_JOBS_SERVER_LOGOUT_JOB_H
#define PALO_JOBS_SERVER_LOGOUT_JOB_H 1

#include "palo.h"

#include "Olap/PaloSession.h"
#include "PaloDispatcher/DirectPaloJob.h"
#include "Worker/LoginWorker.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server logout
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerLogoutJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerLogoutJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerLogoutJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return type;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes job
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		bool ok = PaloJob::initialize();

		if (!ok) {
			return false;
		}

		server = Context::getContext()->getServer();

		try {
			worker = server->getLoginWorker();

			if (worker == 0) {
				type = WRITE_JOB;
			} else {
				checkToken(server);
				type = SPECIAL_JOB;
			}
		} catch (const ErrorException& e) {
			response = new HttpResponse(HttpResponse::BAD);

			StringBuffer& body = response->getBody();

			body.appendCsvInteger((int32_t)e.getErrorType());
			body.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(e.getErrorType())));
			body.appendCsvString(StringUtils::escapeString(e.getMessage()));
			body.appendEol();

			Logger::warning << "error code: " << (int32_t)e.getErrorType() << " description: " << ErrorException::getDescriptionErrorType(e.getErrorType()) << " message: " << e.getMessage() << endl;

			return false;
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		server = Context::getContext()->getServerCopy(); // to increase server token
		checkToken(server);

		if (server->isBlocking()) {
			if (session->getSid() == server->getActiveSession()) {
				server->setUsername("");
				server->setEvent("");
				server->setBlocking(false);
				server->setActiveSession(PaloSession::NO_SESSION);
			}
		}

		if (jobRequest->type == 1) {
			// terminate all session jobs
			WriteLocker wl(&session->thisLock);
			for (set<const PaloJob *>::iterator jit = session->activeJobs.begin(); jit != session->activeJobs.end(); ++jit) {
				if (*jit && (*jit)->getId() != getId()) {
					Context *jobContext = const_cast<Context *>((*jit)->getContext());
					if (jobContext) {
						jobContext->stop(false);
					}
				}
			}
		}

		session->moveJobToFinished(this);
		PaloSession::deleteSession(session, false);

		session.reset();

		if (worker) {
			if (user) {
				worker->notifyLogout(user->getName());
			} else {
				worker->notifyLogout("<unknown>");
			}
		}

		server->commit();

		generateOkResponse(server);
	}

private:
	PLoginWorker worker;
	JobType type;

};
}

#endif
