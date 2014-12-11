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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_SERVER_LOGIN_JOB_H
#define PALO_JOBS_SERVER_LOGIN_JOB_H 1

#include "palo.h"

#include "Olap/PaloSession.h"
#include "Olap/Password.h"
#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloDispatcher/PaloJobRequest.h"
#include "PaloHttpServer/PaloRequestHandler.h"
#include "PaloHttpServer/PaloSSORequestHandler.h"
#include "Worker/LoginWorker.h"
#include "Worker/Worker.h"
#include "Scheduler/IoTask.h"
#include "HttpServer/HttpServerTask.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server login
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerLoginJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerLoginJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerLoginJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest), ssoLogin(false) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes job
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		bool ok = PaloJob::initialize();

		if (!ok) {
			return false;
		}

		try {
			server = Context::getContext()->getServer();

			if (server->winAuthEnabled() && !jobRequest->user) { // ssologin
				ssoLogin = true;
				if (!PaloSSORequestHandler::getAuthenticationInfo(jobRequest->negotiationId, winUsername, winDomain, winGroups)) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot execute windows authentication");
				}
			}

			if (server->getLoginType() == WORKER_NONE && !ssoLogin) {
				worker.reset();
				type = READ_JOB;
			} else {
				worker = server->getLoginWorker();
				if (worker == 0) { // internal auth to be used, SVS turned off
					type = READ_JOB;
				} else {
					checkToken(server);
					type = SPECIAL_JOB;
				}
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
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return type;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		session.reset();
		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS_CELL_REPLACE; commitTry++) {
			Context *context = Context::getContext();
			bool optimistic = commitTry == 0;

			server = Context::getContext()->getServerCopy();
			checkToken(server);
			bool useMd5 = true;

			machine = jobRequest->machineString ? *jobRequest->machineString : "";
			required = jobRequest->requiredFeatures ? *jobRequest->requiredFeatures : "";
			optional = jobRequest->optionalFeatures ? *jobRequest->optionalFeatures : "";
			description = jobRequest->newName ? *jobRequest->newName : "";
			string sudoName = "";

			if (!ssoLogin || worker == 0) { // username required by auth type or ssoLogin active, but SVS turned off
				// extract and check username
				username = jobRequest->user;

				if (username == 0) {
					throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "missing username, win-sso is disabled", PaloRequestHandler::NAME_USER, "");
				}

				size_t tabChar = username->find('\t');
				if (tabChar != string::npos) {
					if (tabChar != username->size() - 1) {
						// get sudoName if it is available after special character
						sudoName = username->substr(tabChar + 1, string::npos);
					}
					*username = username->substr(0, tabChar);

					Logger::debug << "User " << *username << " logging as " << sudoName << endl;

					if (sudoName.empty()) {
						throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "no username to impersonate", PaloRequestHandler::NAME_USER, "");
					}
				}

				if (username->empty()) {
					throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "empty username", PaloRequestHandler::NAME_USER, "");
				}

				// extract and check password
				password = jobRequest->password;
			}

			if (worker == 0) {
				if (password == 0 && jobRequest->externPassword != 0) {
					password = jobRequest->externPassword;
					useMd5 = false;
				}
			}

			if (!ssoLogin) {
				if (password == 0) {
					throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "missing password", PaloRequestHandler::PASSWORD, "");
				}

				if (password->empty()) {
					throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "empty password", PaloRequestHandler::PASSWORD, "");
				}
			}

			// login using username and password
			if (worker == 0) {
				session = loginInternal(useMd5, sudoName);
			} else {
				try {
					if (ssoLogin) {
						session = loginWindowsAuthentication(winDomain, winUsername, winGroups);
					} else if (server->getLoginType() == WORKER_INFORMATION) {
						session = loginExternalInformation(sudoName);
					} else if (server->getLoginType() == WORKER_AUTHENTICATION) {
						session = loginExternalAuthentication(sudoName);
					} else if (server->getLoginType() == WORKER_AUTHORIZATION) {
						session = loginExternalAuthorization(sudoName);
					} else {
						throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "unknown login type", "login type", server->getLoginType());
					}
				} catch (ErrorException &origWorkerException) {
					if (!ssoLogin && server->getLoginType() != WORKER_INFORMATION && (*username == SystemDatabase::NAME_ADMIN || *username == SystemDatabase::NAME_IPS)) {
						Logger::debug << "external authentication declined for " << *username << ", logging internally" << endl;
						try {
							session = loginInternal(useMd5, sudoName);
						} catch (...) {
							throw origWorkerException; // to allow libpalo_ng to try again without hash also for admin/ips if http only is active
						}
					} else {
						throw;
					}
				}
			}

			if (!optimistic) {
				context->setPesimistic();
			}

			ret = server->commit();

			if (ret) {
				break;
			}
			clear();

		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "ServerLoginJob failed. Internal error occured.");
		}

		generateLoginResponse(session, optional);
	}

private:

	PSystemDatabase checkCreateUser(bool external, string& username, vector<string>* groups, IdentifierType *createdElement = NULL) {
		PSystemDatabase sd = server->getSystemDatabase();
		if (!sd->userExist(username, external) || (external && (sd->groupsUpdateRequired(server, username, groups)))) {
			sd = server->getSystemDatabaseCopy();
			if (external) {
				sd->createExternalUser(COMMITABLE_CAST(Server, server), username, groups, createdElement);
			} else {
				sd->createUser(username);
			}
		}
		return sd;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief login internally
	////////////////////////////////////////////////////////////////////////////////

	boost::shared_ptr<PaloSession> loginInternal(bool useMd5, string &sudoName) {
		PSystemDatabase sd = checkCreateUser(false, *username, 0);
		if (!sudoName.empty()) {
			sd = checkCreateUser(false, sudoName, 0);
		}

		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}

		bool inactive = false;
		PUser user = sd->getUser(*username, *password, useMd5, &inactive);
		checkUser(user, false, inactive, *username);

		if (!sudoName.empty()) {
			// check user for sudo right
			if (user->getRoleRight(User::sysOpRight) < RIGHT_WRITE) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "user not authorized to impersonate", "user", user->getName());
			}

			// make sudo
			user = sd->getUser(sudoName, &inactive);
			checkUser(user, false, inactive, sudoName);
		}

		IoTask *ioTask = getIoTask();
		string address;
		if (ioTask) {
			address = ioTask->getPeerName();
		}

		return PaloSession::createSession(user, false, server->getDefaultTtl(), server->useShortSid(), jobRequest->type == 1, address, &machine, &required, &optional, description, jobRequest->externalIdentifier ? (*jobRequest->externalIdentifier) : "");
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief login using external worker for information
	////////////////////////////////////////////////////////////////////////////////

	boost::shared_ptr<PaloSession> loginExternalInformation(string &sudoName) {
		PSystemDatabase sd = checkCreateUser(false, *username, 0);
		if (!sudoName.empty()) {
			sd = checkCreateUser(false, sudoName, 0);
		}

		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}

		PUser user = sd->getUser(*username, *password, true, NULL);
		checkUser(user, true, false, *username);

		if (!sudoName.empty()) {
			// check user for sudo right
			if (user->getRoleRight(User::sysOpRight) < RIGHT_WRITE) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "user not authorized to impersonate", "user", user->getName());
			}

			// make sudo
			user = sd->getUser(sudoName, NULL);
			checkUser(user, true, false, sudoName);
		}

		string loggedUser = (sudoName.empty() ? *username : sudoName);
		ResultStatus status = worker->loginInformation(loggedUser);
		if (status != RESULT_OK) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "worker failed", "username", loggedUser);
		}


		return PaloSession::createSession(user, false, server->getDefaultTtl(), server->useShortSid(), jobRequest->type == 1, getIoTask() ? getIoTask()->getPeerName() : string(), &machine, &required, &optional, description, jobRequest->externalIdentifier ? (*jobRequest->externalIdentifier) : "");
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief login using external worker for authentication
	////////////////////////////////////////////////////////////////////////////////

	boost::shared_ptr<PaloSession> loginExternalAuthentication(string &sudoName) {
		PSystemDatabase sd = checkCreateUser(false, *username, 0);
		if (!sudoName.empty()) {
			sd = checkCreateUser(false, sudoName, 0);
		}

		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}

		bool canLogin;

		ResultStatus status = worker->authenticateUser(*username, (jobRequest->externPassword ? *jobRequest->externPassword : *password), &canLogin);

		if (status != RESULT_OK) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "worker failed", "username", *username);
		}
		if (!canLogin) {
			checkEmptyUser(PUser(), true, *username);
		}

		PUser user = sd->getUser(*username, NULL);
		checkUser(user, true, false, *username);
		if (!sudoName.empty()) {
			// check user for sudo right
			if (user->getRoleRight(User::sysOpRight) < RIGHT_WRITE) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "user not authorized to impersonate", "user", user->getName());
			}

			// make sudo
			user = sd->getUser(sudoName, NULL);
			checkUser(user, true, false, sudoName);
		}

		return PaloSession::createSession(user, false, server->getDefaultTtl(), server->useShortSid(), jobRequest->type == 1, getIoTask() ? getIoTask()->getPeerName() : string(), &machine, &required, &optional, description, jobRequest->externalIdentifier ? (*jobRequest->externalIdentifier) : "");
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief login using windows authentication
	////////////////////////////////////////////////////////////////////////////////

	boost::shared_ptr<PaloSession> loginWindowsAuthentication(string domain, string username, vector<string> &winGroups) {
		bool canLogin;
		vector<string> groups;
		string fullName = domain + "\\" + username;

		ResultStatus status = worker->authorizeWindowsUser(domain, username, winGroups, &canLogin, &groups);

		if (status != RESULT_OK) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "worker failed", "username", fullName);
		}
		if (!canLogin) {
			throw ParameterException(ErrorException::ERROR_WORKER_AUTHORIZATION_FAILED, "windows user not authorized", PaloRequestHandler::NAME_USER, fullName);
		}

		PSystemDatabase sd = server->getSystemDatabase();
		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}
		if (sd->existingGroupsCount(groups) == 0) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "user is not assigned to any active group", PaloRequestHandler::NAME_USER, username);
		}

		IdentifierType createdElement = NO_IDENTIFIER;
		sd = checkCreateUser(true, fullName, &groups, &createdElement); // possible checkout of sd
		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}
		
		PUser user = sd->getExternalUser(fullName);
		if (user && user->getGroupCount() == 0) {
			checkCanLogin(false, fullName);
		}
		checkEmptyUser(user, true, *this->username);


		boost::shared_ptr<PaloSession> session = PaloSession::createSession(user, false, server->getDefaultTtl(), server->useShortSid(), jobRequest->type == 1, getIoTask() ? getIoTask()->getPeerName() : string(), &machine, &required, &optional, description, jobRequest->externalIdentifier ? (*jobRequest->externalIdentifier) : "");

		if (createdElement != NO_IDENTIFIER) {
			sd->getUserDimension()->addElementEvent(server, sd, createdElement, session->getSid());
		}

		return session;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief login using external worker for authorization
	////////////////////////////////////////////////////////////////////////////////

	boost::shared_ptr<PaloSession> loginExternalAuthorization(string &sudoName) {
		bool canLogin;
		vector<string> groups;

		ResultStatus status = worker->authorizeUser(*username, (jobRequest->externPassword ? *jobRequest->externPassword : *password), &canLogin, &groups);

		if (status != RESULT_OK) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "worker failed", "username", *username);
		}
		if (!canLogin) {
			checkEmptyUser(PUser(), true, *username);
		}

		if (groups.empty()) {
			checkCanLogin(false, *username);
		}

		IdentifierType createdElement = NO_IDENTIFIER;
		PSystemDatabase sd = checkCreateUser(true, *username, &groups, &createdElement);
		if (!sudoName.empty()) {
			sd = checkCreateUser(false, sudoName, 0);
		}
		if (sd == 0) {
			throw ErrorException(ErrorException::ERROR_DATABASE_NOT_FOUND, "system database not found");
		}

		PUser user = sd->getExternalUser(*username);
		checkUser(user, true, false, *username);
		if (!sudoName.empty()) {
			// check user for sudo right
			if (user->getRoleRight(User::sysOpRight) < RIGHT_WRITE) {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "user not authorized to impersonate", "user", user->getName());
			}

			// make sudo
			bool inactive = false;
			user = sd->getUser(sudoName, &inactive);
			checkUser(user, true, inactive, sudoName);
		}

		boost::shared_ptr<PaloSession> session = PaloSession::createSession(user, false, server->getDefaultTtl(), server->useShortSid(), jobRequest->type == 1, getIoTask() ? getIoTask()->getPeerName() : string(), &machine, &required, &optional, description, jobRequest->externalIdentifier ? (*jobRequest->externalIdentifier) : "");

		if (createdElement != NO_IDENTIFIER) {
			sd->getUserDimension()->addElementEvent(server, sd, createdElement, session->getSid());
		}

		return session;
	}

private:
	void checkUser(PUser user, bool worker, bool inactive, const string &name) const {
		if (inactive) {
			throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "user account disabled", PaloRequestHandler::NAME_USER, name);
		}

		checkEmptyUser(user, worker, name);
		if (User::checkUser(user)) {
			checkCanLogin(user->canLogin(), name);
		}
	}

	void checkEmptyUser(PUser user, bool worker, const string &name) const {
		if (user == 0) {
			if (worker) {
				throw ParameterException(ErrorException::ERROR_WORKER_AUTHORIZATION_FAILED, "incorrect username or password", PaloRequestHandler::NAME_USER, name);
			} else {
				throw ParameterException(ErrorException::ERROR_AUTHORIZATION_FAILED, "incorrect username or password", PaloRequestHandler::NAME_USER, name);
			}
		}
	}

	void checkCanLogin(bool canLogin, const string &username) const {
		if (!canLogin) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "user is not assigned to any active group", PaloRequestHandler::NAME_USER, username);
		}
	}

	PLoginWorker worker;
	JobType type;
	string *username;
	string *password;
	string machine;
	string required;
	string optional;
	string description;
	
	bool ssoLogin;
	string winUsername;
	string winDomain;
	vector<string> winGroups;
};
}

#endif
