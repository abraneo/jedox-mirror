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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef SERVERCHANGEPASSWORDJOB_H_
#define SERVERCHANGEPASSWORDJOB_H_

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////

/// @brief server change password
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerChangePasswordJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerChangePasswordJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerChangePasswordJob(PaloJobRequest* jobRequest) :
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
		string* username = jobRequest->user;
		IdentifierType userToUpdate = NO_IDENTIFIER;

		if (username) {
			if (username->empty()) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "empty username", PaloRequestHandler::NAME_USER, "");
			}
			// check my rights to change password of others
			if (user && user->getRoleRight(User::passwordRight) < RIGHT_WRITE) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights to change passwords", "user", (int)user->getId());
			}
			// find the user
			server = Context::getContext()->getServer();
			PDimension userDimension = server->getSystemDatabase()->getUserDimension();
			Element *userElement = userDimension->lookupElementByName(*username, false);
			if (!userElement) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "incorrect username", PaloRequestHandler::NAME_USER, *username);
			}
			userToUpdate = userElement->getIdentifier();
		} else {
			// change password of the connected user
			userToUpdate = user->getId();
		}

		if (jobRequest->password == 0) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "missing password", PaloRequestHandler::PASSWORD, "");
		}

		if (jobRequest->password->empty()) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "empty password", PaloRequestHandler::PASSWORD, "");
		}

		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			server->changePassword(user, userToUpdate, *jobRequest->password);
			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't change password.");
		}
		generateOkResponse(server);
	}
};

}

#endif /* SERVERCHANGEPASSWORDJOB_H_ */
