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

#ifndef WORKER_LOGIN_WORKER_H
#define WORKER_LOGIN_WORKER_H 1

#include "palo.h"

#include "Worker/Worker.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief login worker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS LoginWorker : public Worker {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	LoginWorker(const string& session) :
		Worker(session), shutdownInProgress(false) {
	}

	virtual ~LoginWorker();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool start();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about login
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus loginInformation(const string& username);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief authenticates user
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus authenticateUser(const string& username, const string& password, bool* canLogin);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief authorize user
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus authorizeUser(const string& username, const string& password, bool* canLogin, vector<string>* groups);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief authorize user
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus authorizeWindowsUser(const string& domain, const string& username, vector<string> &winGroups, bool* canLogin, vector<string>* groups);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about server shutdown
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyShutdown();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief worker is in shutdown sequence
	////////////////////////////////////////////////////////////////////////////////

	bool isShutdownInProgress() const {return shutdownInProgress;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about database saved
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyDatabaseSaved(IdentifierType);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief informs worker about user logout
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyLogout(const string& username);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief drills through cell
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus cellDrillThrough(IdentifierType mode, IdentifierType database, IdentifierType cube, CPArea cellPath, vector<string>& result);

private:
	ResultStatus authorizeUserGeneral(const string& idString1, const string& idString2, bool* canLogin, vector<string>* groups, bool windows, vector<string> *winGroups);

	bool shutdownInProgress;
};

}
#endif
