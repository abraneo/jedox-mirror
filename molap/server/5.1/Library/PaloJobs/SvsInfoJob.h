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
 * 
 *
 */

#ifndef PALO_JOBS_SVS_INFO_JOB_H
#define PALO_JOBS_SVS_INFO_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "Worker/LoginWorker.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief super vision server info
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SvsInfoJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new SvsInfoJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	SvsInfoJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		server = Context::getContext()->getServer();
		PLoginWorker loginWorker = server->getLoginWorker();
		PDimensionWorker dimensionWorker = server->getDimensionWorker();

		int svsActive = server->svsIsStopped() ? 0 :
				(server->getLoginType() != WORKER_NONE || server->winAuthEnabled() || CubeWorker::useCubeWorker() || server->isDimensionWorkerConfigured());
		int loginMode = server->getLoginType();
		int cubeWorker = CubeWorker::useCubeWorker() ? 1 : 0;
		int drillThroughEnabled = server->isDrillThroughEnabled() && server->getLoginType() != WORKER_NONE;
		int dimWorkerActive = server->isDimensionWorkerConfigured();
		int winSSOActive = server->winAuthEnabled() ? 1 : 0;

		response = new HttpResponse(HttpResponse::OK);
		StringBuffer& body = response->getBody();

		body.appendCsvInteger(svsActive);
		body.appendCsvInteger(loginMode);
		body.appendCsvInteger(cubeWorker);
		body.appendCsvInteger(drillThroughEnabled);
		body.appendCsvInteger(dimWorkerActive);
		body.appendCsvInteger(winSSOActive);
		body.appendEol();
	}
};

}

#endif
