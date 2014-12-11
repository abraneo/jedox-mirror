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

#ifndef PALO_BROWSER_SERVER_BROWSER_JOB_H
#define PALO_BROWSER_SERVER_BROWSER_JOB_H 1

#include "palo.h"

#include <iostream>

#include "PaloDispatcher/PaloBrowserJob.h"

#include "PaloDocumentation/ServerBrowserDocumentation.h"
#include "PaloDocumentation/HtmlFormatter.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server browser
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerBrowserJob : public PaloBrowserJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerBrowserJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerBrowserJob(PaloJobRequest* jobRequest) :
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
			if (action == "load" || action == "save" || action == "delete" || action == "unload" || action == "load_server" || action == "save_server" || action == "shutdown_server") {
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

		server = Context::getContext()->getServer();

		if (!action.empty()) {
			try {
				if (action == "load") {
					server = Context::getContext()->getServerCopy();
					findDatabase(false, true);
					server->loadDatabase(database, user);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "database loaded";
				} else if (action == "save") {
					findDatabase(true, false);
					server->saveServer(user, false);
					server->saveDatabase(database, user, true, NULL, false);
					message = "database saved";
				} else if (action == "delete") {
					server = Context::getContext()->getServerCopy();
					findDatabase(false, true);
					server->deleteDatabase(database, user, true);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "database deleted";
				} else if (action == "unload") {
					server = Context::getContext()->getServerCopy();
					findDatabase(true, true);
					server->unloadDatabase(database, user);
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "database unloaded";
				} else if (action == "load_server") {
					server = Context::getContext()->getServerCopy();
					server->loadServer(user);
					server->addSystemDatabase();
					if (!server->commit()) {
						throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't commit changes.");
					}
					message = "server loaded";
				} else if (action == "save_server") {
					WriteLocker wl(&Server::getSaveLock());
					Context::reset();
					server = Context::getContext()->getServer();
					server->saveServer(user, false);
					message = "server saved";
				} else if (action == "shutdown_server") {
					setShutdown(true);
					generateMessageResponse("Server was shut down.");
					return;
				}

				if (message != "") {
					message = createHtmlMessage("Info", message);
				}
			} catch (ErrorException e) {
				message = createHtmlMessage("Error", e.getMessage());
			}
		}

		Context::reset();
		server = Context::getContext()->getServer();

		ServerBrowserDocumentation sbd(server, message);
		HtmlFormatter hf(templatePath + "/browser_server.tmpl");

		generateResult(hf.getDocumentation(&sbd));
	}

private:
	string action;
	JobType jobType;
};
}

#endif
