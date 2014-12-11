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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_EVENT_BEGIN_JOB_H
#define PALO_JOBS_EVENT_BEGIN_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "Olap/PaloSession.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief database info
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS EventBeginJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new EventBeginJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	EventBeginJob(PaloJobRequest* jobRequest) :
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
		if (jobRequest->event == 0) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "event identifier missing", PaloRequestHandler::EVENT, getSid());
		}

		if (jobRequest->source == 0) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "source identifier missing", PaloRequestHandler::SOURCE, getSid());
		}

		// MJ this function is called only from workers, thats why it doesn't do commit, it's done by the caller.
		server = Context::getContext()->getServerCopy();
		if (server->isBlocking()) {
			if (getSid() == server->getActiveSession()) {
				throw ParameterException(ErrorException::ERROR_WITHIN_EVENT, "session already within event block", PaloRequestHandler::SID, getSid());
			} else {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "reached EventBeginJob without valid event lock");
			}
		} else {
			const string& source = *jobRequest->source;
			boost::shared_ptr<PaloSession>  sourceSession = PaloSession::findSession(source, false);
			PUser sourceUser = sourceSession->getUser();

			server->setUsername(sourceUser == 0 ? "" : sourceUser->getName());
			server->setEvent(*jobRequest->event);
			server->setBlocking(true);
			server->setActiveSession(getSid());

			Logger::info << "starting transaction for user " << server->getRealUsername() << endl;
		}
		generateOkResponse();
	}
};

}

#endif
