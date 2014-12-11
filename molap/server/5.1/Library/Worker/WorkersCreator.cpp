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
 * \author Vali Nitu, Yalos Solutions, Bucharest, Romania
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Worker/WorkersCreator.h"
#include "Worker/DimensionWorker.h"
#include "Worker/LoginWorker.h"

namespace palo {

void WorkersCreator::operator()()
{
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	startAll(true);

	if (NULL != sink) {
		sink(); //give a chance to the caller to update state
	}
}


void WorkersCreator::startAll(bool startup)
{
	bool had_some_workers = false;
	vector<PCube> startedWorkers;

	PServer server = Context::getContext()->getServerCopy();
	try {
		if (server->getLoginWorker()) {
			had_some_workers = true;
		}
	} catch (ErrorException &) {
		Logger::error << "could not start Login Worker, palo terminating..." << endl;
		exit(1);
	}

	try {
		PDimensionWorker dimWorker = server->getDimensionWorker();
		if (dimWorker) {
			had_some_workers = true;
			ResultStatus status = dimWorker->getDimensionsToWatch();
			if (status == RESULT_FAILED) {
				Logger::error << "could not get dimension worker definitions, palo terminating..." << endl;
				quitAll(server, startedWorkers, false);
				exit(1);
			}
		}
	} catch (ErrorException &) {
		Logger::error << "could not start Dimension Worker, palo terminating..." << endl;
		exit(1);
	}

	if (CubeWorker::useCubeWorker()) {
		boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, server->useShortSid(), false, "worker", 0, 0, 0, "worker", "");
		PCubeWorker investigatorWorker = PCubeWorker(new CubeWorker(session->getSid(), true));
		if (investigatorWorker != 0) {
			Logger::trace << "trying to start cube investigator worker" << endl;
			bool ok = investigatorWorker->start();
			if (!ok) {
				Logger::error << "could not start cube investigator worker, palo terminating..." << endl;
				exit(1);
			}
		} else {
			Logger::error << "could not create cube investigator worker, palo terminating..." << endl;
			exit(1);
		}

		PDatabaseList databases = server->getDatabaseList(true);
		server->setDatabaseList(databases);

		for (DatabaseList::Iterator i = databases->begin(); i != databases->end(); ++i) {
			PDatabase database = COMMITABLE_CAST(Database, i.getCopy());
			databases->set(database);

			PCubeList cubes = database->getCubeList(true);
			database->setCubeList(cubes);

			for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
				CPCube ccube = CONST_COMMITABLE_CAST(Cube, *j);
				if (!ccube->supportsWorker()) {
					continue;
				}

				PCube cube = COMMITABLE_CAST(Cube, j.getCopy());
				cubes->set(cube);

				vector<string> areaIds;
				vector<PArea> areas;
				ResultStatus status;
				try {
					status = investigatorWorker->getCubeAreas(database, cube, areaIds, areas);
				} catch (...) {
					status = RESULT_FAILED;
				}
				if (status == RESULT_FAILED) {
					Logger::error << "could not get cube worker areas for cube "<< cube->getName() << ", palo terminating..." << endl;
					quitAll(server, startedWorkers, false);
					investigatorWorker->notifyShutdown();
					exit(1);
				} else if (areaIds.empty() || areas.empty()) {
					cube->removeWorker();
					continue;
				}

				PCubeWorker worker = cube->createCubeWorker();
				if (worker != 0) {
					Logger::trace << "starting cube worker for cube '" << cube->getName() << "'" << endl;

					try {
						bool ok = worker->start(database, cube);
						Logger::trace << "status = " << (ok ? "ok" : "FAILED") << endl;
					} catch (ErrorException &) {
						Logger::error << "could not start Cube Worker, palo terminating..." << endl;
						quitAll(server, startedWorkers, false);
						investigatorWorker->notifyShutdown();
						exit(1);
					}

					had_some_workers = true;
					startedWorkers.push_back(cube);
				}
			}
		}

		investigatorWorker->notifyShutdown();
		investigatorWorker->releaseSession();
	}

	server->setSvsStopped(false);

	if (startup) {
		try {
			Context::getContext()->getServer()->afterLoad();
		} catch (...) {
			Logger::error << "stop request before initialization completed" << endl;
			exit(1);
		}
		if (!server->commit()) {
			Logger::warning << "Can't commit changes needed for workers." << endl;
		}
		Context::reset();
	}

	if (had_some_workers) {
		Logger::info << "All Workers initialized." << endl;
	}
}


void WorkersCreator::quitAll(PServer server, vector<PCube> &vCubes, bool restart)
{
	PLoginWorker loginWorker = server->getLoginWorker();
	if (loginWorker) {
		loginWorker->terminate(restart);
		if (!restart) {
			loginWorker->releaseSession();
			server->setLoginWorker(PLoginWorker());
		}
	}

	PDimensionWorker dimensionWorker = server->getDimensionWorker();
	if (dimensionWorker) {
		dimensionWorker->terminate(restart);
		if (!restart) {
			dimensionWorker->releaseSession();
			server->setDimensionWorker(PDimensionWorker());
		}
	}

	for (vector<PCube>::iterator it = vCubes.begin(); it != vCubes.end(); ++it) {
		PCubeWorker worker = (*it)->getCubeWorker();
		if (worker) {
			worker->terminate(restart);
			(*it)->removeWorker();
		}
	}

	server->setSvsStopped(true);
}


void WorkersCreator::prepareNewStart(PServer server)
{
	if (server->getLoginType() != WORKER_NONE || server->winAuthEnabled()) {
		boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, server->useShortSid(), false, "worker", 0, 0, 0, "worker", "");
		PLoginWorker worker(new LoginWorker(session->getSid()));
		server->setLoginWorker(worker);
	}
	if (server->isDimensionWorkerConfigured()) {
		boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, server->useShortSid(), false, "worker", 0, 0, 0, "worker", "");
		PDimensionWorker worker(new DimensionWorker(session->getSid()));
		server->setDimensionWorker(worker);
	}
}


}
