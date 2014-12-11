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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Worker/LoginWorker.h"

#include "Collections/StringUtils.h"
#include "Logger/Logger.h"
#include "Engine/EngineBase.h"

#include "Exceptions/WorkerException.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// Worker methods
// /////////////////////////////////////////////////////////////////////////////

LoginWorker::~LoginWorker()
{
	terminate(false);
}

bool LoginWorker::start()
{
	if (shutdownInProgress) {
		return false;
	}

	Logger::trace << "starting login worker" << endl;

	return Worker::startint();
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

ResultStatus LoginWorker::loginInformation(const string& username)
{
	vector<string> result;

	ResultStatus status = execute("LOGIN;" + StringUtils::escapeString(username), result);

	if (isExceptionStatus(result)) {
		Logger::error << "SVS OnUserLogin failed" << endl;
	}

	return status;
}

ResultStatus LoginWorker::authenticateUser(const string& username, const string& password, bool* canLogin)
{
	vector<string> result;

	ResultStatus status = execute("AUTHENTICATION;" + StringUtils::escapeString(username) + ";" + StringUtils::escapeString(password), result);

	if (isExceptionStatus(result)) {
		throw ErrorException(ErrorException::ERROR_WORKER_AUTHORIZATION_FAILED, "SVS OnUserAuthenticate failed");
	}

	if (status != RESULT_OK) {
		return status;
	}

	if (result.size() != 1) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer of login worker");
	}

	vector<string> answer;
	string line = result[0];
	StringUtils::splitString(line, &answer, ';');

	if (answer.size() != 2 && answer.size() != 4) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	if (answer[0] != "LOGIN") {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	if (answer[1] == "TRUE") {
		*canLogin = true;
	} else if (answer[1] == "FALSE") {
		*canLogin = false;
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	return RESULT_OK;
}

ResultStatus LoginWorker::authorizeUserGeneral(const string& idString1, const string& idString2, bool* canLogin, vector<string>* groups, bool windows, vector<string> *winGroups)
{
	vector<string> result;

	ResultStatus status;
	
	if (windows) {
		string strWinGroups;
		for (vector<string>::const_iterator it = winGroups->begin(); it != winGroups->end(); ++it) {
			if (it != winGroups->begin()) {
				strWinGroups += ",";
			}
			strWinGroups += *it;
		}
		status = execute("WINDOWS AUTHORIZATION;" + StringUtils::escapeString(idString1) + ";" + StringUtils::escapeString(idString2) + ";" + strWinGroups, result);
	} else {
		status = execute("AUTHORIZATION;" + StringUtils::escapeString(idString1) + ";" + StringUtils::escapeString(idString2), result);
	}

	if (isExceptionStatus(result)) {
		throw ErrorException(ErrorException::ERROR_WORKER_AUTHORIZATION_FAILED, windows ? "SVS OnWindowsUserAuthorize failed" : "SVS OnUserAuthorize failed");
	}

	if (status != RESULT_OK) {
		return status;
	}

	if (result.size() != 1 && result.size() != 2) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer of login worker");
	}

	vector<string> answer;
	string line = result[0];
	StringUtils::splitString(line, &answer, ';');

	if (answer.size() != 2 && answer.size() != 4) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	if (answer[0] != "LOGIN") {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	if (answer[1] == "TRUE") {
		*canLogin = true;
	} else if (answer[1] == "FALSE") {
		*canLogin = false;

		return RESULT_OK;
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker");
	}

	if (result.size() != 2) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer of login worker");
	}

	answer.clear();
	line = result[1];
	StringUtils::splitString(line, &answer, ';');

	if (answer.size() < 2) {
		throw ErrorException(ErrorException::ERROR_INVALID_WORKER_REPLY, "error in answer '" + line + "' of login worker, no groups received");
	}

	groups->clear();

	for (vector<string>::iterator i = answer.begin() + 1; i != answer.end(); ++i) {
		groups->push_back(*i);
	}

	return RESULT_OK;
}

ResultStatus LoginWorker::authorizeUser(const string& username, const string& password, bool* canLogin, vector<string>* groups)
{
	return authorizeUserGeneral(username, password, canLogin, groups, false, NULL);
}

ResultStatus LoginWorker::authorizeWindowsUser(const string& domain, const string& username, vector<string> &winGroups, bool* canLogin, vector<string>* groups)
{
	return authorizeUserGeneral(domain, username, canLogin, groups, true, &winGroups);
}

ResultStatus LoginWorker::notifyShutdown()
{
	shutdownInProgress = true;

	vector<string> result;

	ResultStatus status = execute("SERVER SHUTDOWN", result, WORKER_TIMEOUT_MSEC);

	if (isExceptionStatus(result)) {
		Logger::error << "SVS OnServerShutdown failed" << endl;
	}

	return status;
}

ResultStatus LoginWorker::notifyDatabaseSaved(IdentifierType id)
{
	vector<string> result;

	ResultStatus status = execute("DATABASE SAVED;" + StringUtils::convertToString(id), result, WORKER_TIMEOUT_MSEC);

	if (isExceptionStatus(result)) {
		Logger::error << "SVS OnDatabaseSaved failed" << endl;
	}

	return status;
}

ResultStatus LoginWorker::notifyLogout(const string& username)
{
	vector<string> result;

	ResultStatus status = execute("USER LOGOUT;" + StringUtils::escapeString(username), result, WORKER_TIMEOUT_MSEC);

	if (isExceptionStatus(result)) {
		Logger::error << "SVS OnUserLogout failed" << endl;
	}

	return status;
}

ResultStatus LoginWorker::cellDrillThrough(IdentifierType mode, IdentifierType database, IdentifierType cube, CPArea cellPath, vector<string>& result)
{
	ResultStatus status = execute("CELL DRILLTHROUGH;" + StringUtils::convertToString(mode) + ";" + StringUtils::convertToString(database) + ";" + StringUtils::convertToString(cube) + ";" + cellPath->pathBegin().toString(), result);

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	}

	return status;
}
}
