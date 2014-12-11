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

#ifndef PALO_BROWSER_DATABASE_BROWSER_JOB_H
#define PALO_BROWSER_DATABASE_BROWSER_JOB_H 1

#include "palo.h"

#include <iostream>

#include "PaloDispatcher/PaloBrowserJob.h"
#include "PaloDocumentation/HtmlFormatter.h"
#include "PaloDocumentation/DatabaseBrowserDocumentation.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief database browser
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DatabaseBrowserJob : public PaloBrowserJob {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new DatabaseBrowserJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	DatabaseBrowserJob(PaloJobRequest* jobRequest) :
		PaloBrowserJob(jobRequest) {
		jobType = READ_JOB;
		action = "";
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return jobType;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		bool ok = PaloJob::initialize();

		if (!ok) {
			return false;
		}

		if (jobRequest->action) {
			action = *(jobRequest->action);
		}

		if (!action.empty()) {
			if (action == "load" || action == "save" || action == "delete" || action == "unload" || action == "load_database" || action == "save_database" || action == "delete_dimension") {
				jobType = WRITE_JOB;
			}
		}
		return true;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		string message;

		findDatabase(true, false);

		if (!action.empty()) {
			try {
				if (action == "load") {
					getNewDatabase(true);
					findCube(false, true);
					database->loadCube(server, cube, user);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "cube loaded";
				} else if (action == "save") {
					WriteLocker wl(&Server::getSaveLock());
					getNewDatabase(false);
					findCube(true, false);
					server->saveServer(PUser(), false);
					server->saveDatabase(database, PUser(), true, NULL, false);
					database->saveCube(server, cube, PUser());
					message = "cube saved";
				} else if (action == "delete") {
					getNewDatabase(true);
					findCube(true, true);
					database->deleteCube(server, cube, PUser(), true, true);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "cube deleted";
				} else if (action == "unload") {
					getNewDatabase(true);
					findCube(true, true);
					database->unloadCube(server, cube, PUser());
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "cube unloaded";
				} else if (action == "load_database") {
					getNewDatabase(true);
					server->loadDatabase(database, PUser());
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "database loaded";
				} else if (action == "save_database") {
					WriteLocker wl(&Server::getSaveLock());
					getNewDatabase(false);
					server->saveServer(PUser(), false);
					server->saveDatabase(database, PUser(), true, NULL, false);
					message = "database saved";
				} else if (action == "delete_dimension") {
					getNewDatabase(true);
					findDimension(true);
					database->deleteDimension(server, dimension, PUser(), true, true);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "dimension deleted";
				}

				if (!message.empty()) {
					message = createHtmlMessage("Info", message);
				}
			} catch (ErrorException e) {
				message = createHtmlMessage("Error", e.getMessage());
			}
		}

		DatabaseBrowserDocumentation sbd(database, message);
		HtmlFormatter hf(templatePath + "/browser_database.tmpl");

		generateResult(hf.getDocumentation(&sbd));
	}

private:
	void getNewDatabase(bool write) {
		clear();
		if (write) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);
		} else {
			server = Context::getContext()->getServer();
			findDatabase(true, false);
		}
	}

	string action;
	JobType jobType;
};

}

#endif
