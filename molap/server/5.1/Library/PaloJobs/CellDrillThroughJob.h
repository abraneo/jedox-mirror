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
 * \author Marko Stijak, Banja Luka, Bosnia and Herzegovina
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_DRILLTHROUGH_JOB_H
#define PALO_JOBS_CELL_DRILLTHROUGH_JOB_H 1

#include "palo.h"

#include "Worker/LoginWorker.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief login to a palo server
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellDrillThroughJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellDrillThroughJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellDrillThroughJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return SPECIAL_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		findPath();

		if (server->svsIsStopped()) {
			throw ErrorException(ErrorException::ERROR_WORKER_OPERATION, "SVS processes are stopped.");
		}

		if (jobRequest->mode < 1 || jobRequest->mode > 2)
			throw ParameterException(ErrorException::ERROR_INVALID_MODE, "unsupported cell/drillthrough mode", PaloRequestHandler::ID_MODE, jobRequest->mode);

		if (User::checkUser(user)) {
			if (user->getRoleRight(User::cellDataRight) < RIGHT_READ) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for path", "user", user->getId());
			} else if (user->getRoleRight(User::drillthroughRight) < RIGHT_DELETE) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for drillthrough", "user", user->getId());
			}
		}

		PLoginWorker loginWorker = server->getLoginWorker();

		if (loginWorker) {
			if (!server->isDrillThroughEnabled()) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "drillthrough not enabled");
			}
			cellDrillThrough(jobRequest->mode);
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "no login worker");
		}
	}

private:
	void cellDrillThrough(IdentifierType modeId) {
		PLoginWorker loginWorker = server->getLoginWorker();
		vector<string> result;
		ResultStatus status = loginWorker->cellDrillThrough(modeId, database->getId(), cube->getId(), cellPath, result);
//		ResultStatus status = loginWorker->cellDrillThrough(modeId, database->getId(), cube->getId(), cellPath, result, getSid());

		try {
			if (status == RESULT_FAILED) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "login worker error");
			}

			if (result.empty()) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "login worker error");
			} 
			
			vector<string>::iterator it = --result.end();
			if (it->empty()) {
				result.erase(it);
			}

			if (result.size() == 1) {
				if (result[0] == "no result") {
					// nothing returned, but ok
					result.clear();
				} else if (result[0].find(';') != string::npos) {
					// just header returned, ok
				} else {
					throw ErrorException(ErrorException::ERROR_INTERNAL, result[0]);
				}
			}
		} catch (ErrorException e) {
			response = new HttpResponse(HttpResponse::BAD);

			StringBuffer& body = response->getBody();

			body.appendCsvInteger((int32_t)e.getErrorType());
			body.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(e.getErrorType())));
			body.appendCsvString(StringUtils::escapeString(e.getMessage()));
			body.appendEol();

			return;
		}

		response = new HttpResponse(HttpResponse::OK);
		StringBuffer& sb = response->getBody();

		vector<string>::iterator i = result.begin();
		while (i != result.end()) {
			sb.appendText(i->c_str());
			++i;
			if (i != result.end()) {
				sb.appendEol();
			}
		} 

	}
};
}

#endif
