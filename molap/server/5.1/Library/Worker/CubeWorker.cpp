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

#include "Worker/CubeWorker.h"

#include "Collections/StringBuffer.h"
#include "Collections/StringUtils.h"

#include "Exceptions/WorkerException.h"

#include "Logger/Logger.h"

#include "Olap/Cube.h"
#include "Olap/Database.h"
#include "Olap/Server.h"

#include "PaloDispatcher/PaloJob.h"

#include "Engine/EngineBase.h"

#include "Thread/WriteLocker.h"

namespace palo {
bool CubeWorker::useWorkers = false;

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

CubeWorker::CubeWorker(const string& sessionString, bool isInvestigator) :
	Worker(sessionString), shutdownInProgress(false), dbid(-1), cubeid(-1), isInvestigator(isInvestigator)
{
}

CubeWorker::~CubeWorker()
{
	terminate(false);
}

// /////////////////////////////////////////////////////////////////////////////
// getters and setters
// /////////////////////////////////////////////////////////////////////////////

void CubeWorker::setUseCubeWorker(bool use)
{
	useWorkers = use;
}

bool CubeWorker::useCubeWorker()
{
	return useWorkers;
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

ResultStatus CubeWorker::setCellValue(const string& areaIdentifier, const string& sessionIdentifier, const IdentifiersType& path, double value, SplashMode splashMode, bool addValue)
{
	StringBuffer sb;

	sb.appendText("DOUBLE;");
	sb.appendInteger(dbid);
	sb.appendChar(';');
	sb.appendInteger(cubeid);
	sb.appendChar(';');
	sb.appendText(StringUtils::escapeString(areaIdentifier));
	sb.appendChar(';');
	sb.appendText(StringUtils::escapeString(sessionIdentifier));
	sb.appendChar(';');
	sb.appendText(buildPathString(&path));
	sb.appendChar(';');
	sb.appendDecimal(value);
	sb.appendChar(';');
	sb.appendInteger(PaloJob::splashNumber(splashMode));
	sb.appendChar(';');
	sb.appendInteger((int)addValue);

	// we need to change the job-type
	//    dispatcher->downgradeCurrentJobs();

	// send set-cell-value request to worker
	vector<string> result;

	ResultStatus status = execute(sb.c_str(), result);

	if (status != RESULT_OK) {
		return status;
	}

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	} else if (isExceptionStatus(result)) {
		throw WorkerException(result[0].substr(10), true);
	}

	numFailures = 0;
	return RESULT_OK;
}

ResultStatus CubeWorker::setCellValue(const string& areaIdentifier, const string& sessionIdentifier, const IdentifiersType& path, const string& value, SplashMode splashMode, bool addValue)
{
	StringBuffer sb;

	sb.appendText("STRING;");
	sb.appendInteger(dbid);
	sb.appendChar(';');
	sb.appendInteger(cubeid);
	sb.appendChar(';');
	sb.appendText(StringUtils::escapeString(areaIdentifier));
	sb.appendChar(';');
	sb.appendText(StringUtils::escapeString(sessionIdentifier));
	sb.appendChar(';');
	sb.appendText(buildPathString(&path));
	sb.appendChar(';');
	sb.appendText(StringUtils::escapeString(value));
	sb.appendChar(';');
	sb.appendInteger(PaloJob::splashNumber(splashMode));
	sb.appendChar(';');
	sb.appendInteger((int)addValue);

	// we need to change the job-type
	//dispatcher->downgradeCurrentJobs();

	// send set-cell-value request to worker
	vector<string> result;

	ResultStatus status = execute(sb.c_str(), result);

	if (status != RESULT_OK) {
		return status;
	}

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	} else if (isExceptionStatus(result)) {
		throw WorkerException(result[0].substr(10), true);
	}

	numFailures = 0;
	return RESULT_OK;
}

ResultStatus CubeWorker::notifyShutdown()
{
	shutdownInProgress = true;

	terminate(false);

	return RESULT_OK;
}

// /////////////////////////////////////////////////////////////////////////////
// Worker methods
// /////////////////////////////////////////////////////////////////////////////

bool CubeWorker::start(PDatabase db, PCube cube)
{
	dbid = db->getId();
	cubeid = cube->getId();;
	if (shutdownInProgress) {
		return false;
	}

	{
		WriteLocker locker(&mutex);

		if (status == WORKER_RUNNING) {
			return true;
		}
	}

	Logger::trace << "starting cube worker for cube '" << cube->getName() << "'" << endl;

	bool ok = Worker::startint();

	if (ok) {
		ResultStatus status = defineCubeAreas(cube);

		if (status == RESULT_FAILED) {
			ok = false;
		}
	} else {
		throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start worker");
	}

	return ok;
}

// investigator worker
bool CubeWorker::start()
{
	if (shutdownInProgress) {
		return false;
	}

	{
		WriteLocker locker(&mutex);

		if (status == WORKER_RUNNING) {
			return true;
		}
	}

	Logger::trace << "starting cube investigator worker" << endl;

	return Worker::startint();
}

// investigator worker
ResultStatus CubeWorker::getCubeAreas(PDatabase db, PCube cube, vector<string> &areaIds, vector<PArea> &areas)
{
	if (!isInvestigator) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "CubeWorker::getCubeAreas called for normal cube worker");
	}

	dbid = db->getId();
	cubeid = cube->getId();

	message.clear();

	vector<string> result;

	// send area request to cube worker
	ResultStatus status = execute("CUBE;" + StringUtils::convertToString(dbid) + ";" + StringUtils::convertToString(cubeid), result);

	if (status == RESULT_FAILED) {
		return status;
	}

	// check result
	if (!result.empty()) {
		if (result.at(0).substr(0, 6) == "ERROR;") {
			message = result[0].substr(6);
			return RESULT_FAILED;
		} else if (isExceptionStatus(result)) {
			message = result[0].substr(10);
			Logger::error << "SVS initialization failed: " << message << endl;
			return RESULT_FAILED;
		} else {
			status = readAreaLines(result, areaIds, areas);

			if (status == RESULT_FAILED) {
				return status;
			}
		}
	}

	if (areaIds.empty()) {
		Logger::debug << "cube '" << cube->getName() << "' has no worker areas" << endl;
		return RESULT_OK;
	}

	// check areas
	bool ok = checkAreas(areaIds, areas);

	if (ok) {
		if (Logger::isTrace()) {
			for (vector<string>::const_iterator i = result.begin(); i != result.end(); ++i) {
				Logger::trace << *i << endl;
			}
		}
	} else {
		return RESULT_FAILED;
	}

	return RESULT_OK;
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

ResultStatus CubeWorker::defineCubeAreas(PCube cube)
{
	vector<string> areaIds;
	vector<PArea> areas;

	message.clear();

	vector<string> result;

	// send area request to cube worker
	ResultStatus status = execute("CUBE;" + StringUtils::convertToString(dbid) + ";" + StringUtils::convertToString(cubeid), result);

	if (status == RESULT_FAILED) {
		return status;
	}

	// check result
	if (!result.empty()) {
		if (result.at(0).substr(0, 6) == "ERROR;") {
			message = result[0].substr(6);
			return RESULT_FAILED;
		} else {
			status = readAreaLines(result, areaIds, areas);

			if (status == RESULT_FAILED) {
				notifyShutdown();
				return status;
			}
		}
	}

	if (areaIds.empty()) {
		Logger::debug << "cube '" << cube->getName() << "' has no worker areas, shutting down worker" << endl;
		notifyShutdown();
		cube->removeWorker();
		return RESULT_OK;
	}

	// check areas
	bool ok = checkAreas(areaIds, areas);

	if (ok) {
		Logger::info << "got area for cube '" << cube->getName() << "'" << endl;

		if (Logger::isTrace()) {
			for (vector<string>::const_iterator i = result.begin(); i != result.end(); ++i) {
				Logger::trace << *i << endl;
			}
		}

		cube->setWorkerAreas(areaIds, areas);
	} else {
		notifyShutdown();
		cube->removeWorker();
		return RESULT_FAILED;
	}

	return notifyAreaBuildOk();
}

ResultStatus CubeWorker::notifyAreaBuildOk()
{
	vector<string> result;

	return execute("AREA-BUILD;OK", result);
}

void CubeWorker::notifyAreaBuildError(const string &id1, const string &id2)
{
	terminate("AREA-BUILD;ERROR;" + StringUtils::escapeString(id1) + ";" + StringUtils::escapeString(id2), 0);
}

string CubeWorker::buildPathString(const IdentifiersType *path)
{
	bool first = true;
	string pathString;

	for (IdentifiersType::const_iterator i = path->begin(); i != path->end(); ++i) {
		if (first) {
			first = false;
		} else {
			pathString += ",";
		}

		pathString += StringUtils::convertToString(*i);
	}

	return pathString;
}

ResultStatus CubeWorker::readAreaLines(const vector<string> &result, vector<string> &areaIds, vector<PArea> &areas)
{
	for (vector<string>::const_iterator i = result.begin(); i != result.end(); ++i) {
		vector<string> values;
		StringUtils::splitString(*i, &values, ';');

		// check for AREA string
		if (values[0] == "AREA" || values.size() < 6) {
			string areaId;
			PArea area;

			try {
				bool ok = computeArea(values, areaId, area);

				if (ok) {
					if (area->dimCount() > 0) {
						areaIds.push_back(areaId);
						areas.push_back(area);
					}
				} else {
					Logger::error << "error in worker response AREA: '" << *i << "'" << endl;
					return RESULT_FAILED;
				}
			} catch (...) {
				Logger::error << "error in worker response AREA: '" << *i << "'" << endl;
				return RESULT_FAILED;
			}
		} else {
			Logger::error << "error in worker response: '" << *i << "'" << endl;
			return RESULT_FAILED;
		}
	}

	return RESULT_OK;
}

bool CubeWorker::computeArea(vector<string> &answer, string &areadId, PArea &area)
{

	// has no area
	// AREA;<id_database>;<id_cube>

	// has area
	// AREA;<id_database>;<id_cube>;<id_area>;<list_elements>;<list_elements>;...;<list_elements>

	if (answer.size() < 4) {
		return false;
	}

	// we start one worker for each normal cube, so we can check the cube and databse identifier here
	IdentifierType databaseId = StringUtils::stringToInteger(answer[2]);

	if (databaseId != dbid) {
		Logger::error << "wrong database id '" << databaseId << "' in worker response" << endl;
		return false;
	}

	IdentifierType cubeId = StringUtils::stringToInteger(answer[3]);

	if (cubeId != cubeid) {
		Logger::error << "wrong cube id '" << cubeId << "' in worker response" << endl;
		return false;
	}

	area.reset(new Area(answer.size() > 4 ? answer.size() - 4 : 0));
	// get area identifier and dimension elements
	if (answer.size() > 4) {
		areadId = answer[1];

		for (int i = 4; i < (int)answer.size(); i++) {
			if (answer[i] != "*") {
				vector<string> idStrings;
				StringUtils::splitString(answer[i], &idStrings, ',');

				PSet s(new Set);
				for (vector<string>::iterator ii = idStrings.begin(); ii != idStrings.end(); ++ii) {
					s->insert(StringUtils::stringToInteger(*ii));
				}
				area->insert(i - 4, s);
			}
		}
	}

	return true;
}

bool CubeWorker::checkAreas(vector<string> &areaIds, vector<PArea> &areas)
{
	const IdentifiersType* dimensions = Context::getContext()->getServer()->lookupDatabase(dbid, false)->lookupCube(cubeid, false)->getDimensions();
	size_t numDimension = dimensions->size();

	size_t num = areas.size();

	for (size_t i = 0; i < num; i++) {
		if (areaIds[i] == "") {
			Logger::error << "name of area is empty" << endl;
			return false;
		}

		if (numDimension != areas[i]->dimCount()) {
			Logger::error << "error in size of dimension elements in AREA: '" << areaIds[i] << "'" << endl;
			return false;
		}
	}

	// check for overlapping areas
	for (size_t i = 0; i < num - 1; i++) {
		for (size_t j = i + 1; j < num; j++) {
			bool overlaps = isOverlapping(areas[i], areas[j]);

			if (overlaps) {
				Logger::error << "error overlapping area '" << areaIds[i] << "' and '" << areaIds[j] << "'" << endl;
				if (isInvestigator) {
					return false;
				} else {
					notifyAreaBuildError(areaIds[i], areaIds[j]);
					throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start worker, overlapping areas");
				}
			}
		}
	}

	return true;
}

bool CubeWorker::isOverlapping(CPArea area1, CPArea area2)
{
	return area1->isOverlapping(*area2);
}

}
