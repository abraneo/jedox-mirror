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

#ifndef SERVERUSERINFO_H_
#define SERVERUSERINFO_H_

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////

/// @brief server change password
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerUserInfoJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerUserInfoJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerUserInfoJob(PaloJobRequest* jobRequest) :
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
		return READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		if (user) {
			server = Context::getContext()->getServer();

			checkToken(server);
			response = new HttpResponse(HttpResponse::OK);
			setToken(server);
			StringBuffer& body = response->getBody();

			body.appendCsvInteger(user->getId());
			body.appendCsvString(StringUtils::escapeString(user->getName()));
			const IdentifiersType& groups = user->getUserGroups();
			for (IdentifiersType::const_iterator it = groups.begin(); it != groups.end(); ++it) {
				if (it != groups.begin()) {
					body.appendChar(',');
				}
				body.appendInteger(*it);
			}
			body.appendChar(';');

			PDimension groupDim = server->getSystemDatabase()->getGroupDimension();
			for (IdentifiersType::const_iterator it = groups.begin(); it != groups.end(); ++it) {
				if (it != groups.begin()) {
					body.appendChar(',');
				}

				body.appendText(StringUtils::escapeString(groupDim->findElement(*it, 0, false)->getName(groupDim->getElemNamesVector())));
			}

			body.appendChar(';');
			body.appendCsvInteger((uint64_t)session->getTtlIntervall());

			if (jobRequest->showPermission) {
				vector<RightsType> rights;
				user->getAllRoleRights(rights);
				for (size_t i = 0; i < rights.size(); i++) {
					if (i) {
						body.appendChar(',');
					}
					body.appendText(StringUtils::escapeString(SystemDatabase::ROLE[i]));
					body.appendChar(':');
					body.appendText(StringUtils::escapeString(User::rightsTypeToString(rights[i])));
				}
				body.appendChar(';');
			}
			if (jobRequest->showInfo) {
				body.appendText(StringUtils::escapeString(server->getSessionLicense(session)));
				body.appendChar(';');
			}

			body.appendEol();
		} else if (session && session->isWorker()) {
			response = new HttpResponse(HttpResponse::OK);
			StringBuffer& body = response->getBody();
			body.appendCsvInteger(0);
			body.appendCsvString("#worker");
			body.appendChar(';'); // group ids
			body.appendChar(';'); // group names
			body.appendChar(';'); // ttl
			if (jobRequest->showPermission) {
				body.appendChar(';');
			}
			if (jobRequest->showInfo) {
				body.appendChar(';');
			}
			body.appendEol();
		} else {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "No such user.", "user", 0);
		}
	}
};

}

#endif /* SERVERUSERINFO_H_ */
