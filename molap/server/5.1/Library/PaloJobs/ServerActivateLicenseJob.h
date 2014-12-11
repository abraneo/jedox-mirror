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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef SERVERACTIVATELICENSEJOB_H_
#define SERVERACTIVATELICENSEJOB_H_
#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////

/// @brief server info job
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerActivateLicenseJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerActivateLicenseJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerActivateLicenseJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes job
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		return PaloJob::initialize();
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
		server = Context::getContext()->getServer();

		checkToken(server);
		set<string> lic2del;
		lic2del.insert("THISI-SATRI-ALLIC-ENSEY");
		if (jobRequest->actcode && !jobRequest->actcode->empty()) {
			server->findFree(lic2del);
			server->activateLicense(jobRequest->lickey ? *jobRequest->lickey : "", jobRequest->actcode ? *jobRequest->actcode : "");
		} else {
			if (User::checkUser(user)&& user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)(user->getId()));
			}
			if (jobRequest->lickey) {
				lic2del.insert(*jobRequest->lickey);
			}
		}
		bool del = server->deleteLicense(lic2del);
		bool ret = false;
		try {
			for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; ++commitTry) {
				server = Context::getContext()->getServerCopy();
				// TODO: GPU stuff
				if (del) {
					PSystemDatabase sysdb = server->getSystemDatabaseCopy();
					PCubeList cubes = sysdb->getCubeList(true);
					sysdb->setCubeList(cubes);
					PCube userPropsCube = COMMITABLE_CAST(Cube, sysdb->getUserUserPropertiesCube()->copy());
					cubes->set(userPropsCube);
					PEngineBase engine = server->getEngine(EngineBase::CPU, true);
					IdentifierType id = userPropsCube->getStringStorageId();
					engine->getCreateStorage(id, userPropsCube->getPathTranslator(), EngineBase::String);
					server->updateLicenses(sysdb);
				}
				ret = server->commit();
				if (ret) {
					break;
				}
				clear();
			}
		} catch (ErrorException &e) {
			Logger::error << e.getMessage() << endl;
		}
		if (!ret) {
			Logger::error << "Can't update license info in user properties." << endl;
		}

		response = new HttpResponse(HttpResponse::OK);
		setToken(server);
		generateOkResponse(server);
	}
};

}
#endif /* SERVERACTIVATELICENSEJOB_H_ */
