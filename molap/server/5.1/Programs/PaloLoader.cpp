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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Programs/PaloLoader.h"

#include "Exceptions/FileFormatException.h"
#include "Exceptions/FileOpenException.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"

#ifdef STATISTICS
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#endif

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloLoader::PaloLoader(bool autoCommit, const string& dataDirectory) :
	autoCommit(autoCommit), dataDirectory(dataDirectory)
{
	lastResort = new char[1000 * 1000];
	server = Context::getContext()->getServerCopy();
}

PaloLoader::~PaloLoader()
{
	if (lastResort != 0) {
		delete[] lastResort;
	}
	Context::reset();

	//    if (autoCommit) {
	//      commitChanges();
	//    }
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

void PaloLoader::load(bool autoLoadDb, bool autoAddDb)
{

	loadServer();
	server->updateDatabaseDim(false);

	// add any new database found in the path
	if (autoLoadDb && autoAddDb) {
		addNewDatabases();
	}

	// autoload other databases
	if (autoLoadDb) {
		autoLoadDatabases();
	}
}

void PaloLoader::finalize(bool useFakeSession)
{
#ifdef STATISTICS
	timeval tv1;
	timeval tv2;
	gettimeofday(&tv1, 0);
#endif

	server->commit();

#ifdef STATISTICS
	gettimeofday(&tv2, 0);
	double d = 1000000 * (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec);
	d /= 1000000;
	cout << "PaloLoader::finalize" << "\t" << d << endl;
#endif

	server->checkOldCubes();

	// create a fake session id
	if (useFakeSession) {
		PaloSession::createSession(PaloSession::FAKE_SESSION, (IdentifierType)-1, PUser(), false, 0, false, "FAKE", 0, 0, 0, "Fake session", "");
	}
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloLoader::loadServer()
{
	bool loaded = false;

	server->physicalRenames();
	// load palo server from disk
	try {
		server->loadServer(PUser());
		loaded = true;
	} catch (const FileFormatException&) {
		Logger::error << "server file is corrupted" << endl;
		throw;
	} catch (const FileOpenException& e) {
		loaded = false;
		Logger::warning << "failed to load server '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
	} catch (const ErrorException&) {
		Logger::error << "cannot load server" << endl;
		throw;
	}

	// create a new server
	if (!loaded) {
		try {
			server->saveServer(PUser(), false);
		} catch (const FileFormatException&) {
			Logger::error << "server file is corrupted" << endl;
			throw;
		} catch (const ErrorException&) {
			Logger::error << "cannot save server file" << endl;
			throw;
		}
	}

	// load or generate system databases
	try {
		server->addSystemDatabase();
	} catch (const FileFormatException&) {
		Logger::error << Server::NAME_SYSTEM_DATABASE << " database file is corrupted" << endl;
		throw;
	} catch (const ErrorException&) {
		Logger::error << "cannot load " << Server::NAME_SYSTEM_DATABASE << " database file" << endl;
		throw;
	}
}

void PaloLoader::autoLoadDatabases()
{
	server->commit();
	server = Context::getContext()->getServerCopy();

	PDatabase systemDB = server->getSystemDatabase();
	PDatabaseList databases = server->getDatabaseList(true);
	server->setDatabaseList(databases);

	bool bConfig = false;
	for (DatabaseList::Iterator i = databases->begin(); i != databases->end(); i++) {
		PDatabase database = COMMITABLE_CAST(Database, (*i)->copy());

		if (database->getId() == systemDB->getId()) {
			continue;
		}

		try {
			Logger::info << "auto loading database '" << database->getName() << "'" << endl;
			if (database->getName() == Server::NAME_CONFIG_DATABASE) {
				bConfig = true;
			}
			server->loadDatabase(database, PUser());
		} catch (std::bad_alloc&) {
			delete[] lastResort;
			lastResort = 0;

			Logger::error << "running out of memory, please reduce the number of loaded databases" << endl;
			throw ErrorException(ErrorException::ERROR_OUT_OF_MEMORY, "");
		} catch (const ErrorException& e) {
			Logger::error << "cannot load database '" << database->getName() << "'" << endl;
			Logger::error << "message 1: " << e.getMessage() << endl;
			Logger::error << "message 2: " << e.getDetails() << endl;

			database = COMMITABLE_CAST(Database, (*i)->copy());
			database->setStatus(Database::LOADING); // error status
			databases->set(database);
		}
	}
	if (!bConfig) {
		server->addConfigDatabase();
	}
}

void PaloLoader::addNewDatabases()
{
	vector<string> files = FileUtils::listFiles(dataDirectory);

	for (vector<string>::iterator i = files.begin(); i != files.end(); ++i) {
		string dbName = *i;
		string file = dataDirectory + "/" + dbName;

		if (FileUtils::isReadable(FileName(file, "database", "csv"))) {
			if (dbName.find_first_not_of(Server::VALID_DATABASE_CHARACTERS) != string::npos) {
				Logger::warning << "directory '" << dbName << "' of database contains an illegal character, skipping" << endl;
			} else if (dbName[0] == '.') {
				Logger::warning << "directory '" << dbName << "' of database begins with a dot character, skipping" << endl;
			} else {
				if (server->lookupDatabaseByName(dbName, false) == 0) {
					Logger::info << "added new directory as database '" << dbName << "'" << endl;

					try {
						server->addDatabase(dbName, PUser(), 0, false);
					} catch (const ErrorException&) {
						Logger::error << "cannot load database '" << dbName << "'" << endl;
						throw;
					}
				}
			}
		}
	}
}

void PaloLoader::commitChanges()
{
	try {
		server->saveServer(PUser(), false);

		PDatabaseList databases = server->getDatabaseList(false);

		for (DatabaseList::Iterator i = databases->begin(); i != databases->end(); ++i) {
			PDatabase database = COMMITABLE_CAST(Database, *i);
			if (NULL == database)
				continue;

			if (database->getStatus() == Database::CHANGED) {
				Logger::info << "auto commiting changes to database '" << database->getName() << "'" << endl;
				server->saveDatabase(database, PUser(), false, NULL, false);
			}

			PCubeList cubes = database->getCubeList(false);

			for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
				PCube cube = COMMITABLE_CAST(Cube, *j);
				if (NULL == cube)
					continue;

				if (cube->getStatus() == Cube::CHANGED) {
					Logger::info << "auto commiting changes to cube '" << cube->getName() << "'" << endl;
					database->saveCube(server, cube, PUser());
				}
			}
		}
	} catch (const ErrorException& e) {
		Logger::warning << "cannot commit data file '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
	}
}
}
