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

#include "PaloDispatcher/PaloJob.h"

#include "Collections/StringBuffer.h"
#include "Olap/AttributesCube.h"
#include "Olap/CubeDimension.h"
#include "Olap/Element.h"
#include "Olap/Lock.h"
#include "Olap/NormalDimension.h"
#include "Olap/PaloSession.h"
#include "Olap/RightsCube.h"
#include "Olap/Rule.h"
#include "Olap/SubsetViewDimension.h"
#include "Olap/SystemDimension.h"
#include "Olap/UserInfoDimension.h"
#include "Olap/Context.h"
#include "Olap/NormalDatabase.h"
#include "Engine/EngineBase.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
map<IdentifierType, PaloJob*> PaloJob::jobs;

Mutex PaloJob::m_main_Lock;
IdentifierType PaloJob::lastId = 0;

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloJob::PaloJob(PaloJobRequest* jobRequest) :
		Job(jobRequest), response(0), jobRequest(jobRequest), element(0), updateLics(false)
{
	startTime = boost::posix_time::microsec_clock::universal_time();
	WriteLocker paloSessionLock(&m_main_Lock);
	id = lastId++;
	jobs[id] = this;
}

PaloJob::~PaloJob()
{
	{
		WriteLocker paloSessionLock(&m_main_Lock);
		jobs.erase(id);
	}
	clear(false);
}

double PaloJob::getDuration() const
{
	boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::universal_time();
	double duration = ((currentTime-startTime).total_microseconds())/1000000.0;
	return duration;
}

time_t PaloJob::getStartTime() const
{
	boost::posix_time::ptime epochTime = boost::posix_time::from_time_t(0);
	return (time_t)(startTime-epochTime).total_seconds();
}

string PaloJob::getRequest() const
{
	return jobRequest->httpRequest;
}

void PaloJob::clear(bool setSession)
{
	updateLicenses();
	server.reset();
	database.reset();
	cube.reset();
	dimension.reset();
	element = 0;
	rules.clear();
	cellPaths.reset();
	lockedPaths.reset();
	Context::reset();
	if (setSession) {
		Context::getContext()->setSession(session);
		Context::getContext()->setTask(ioTask);
	}
}

// /////////////////////////////////////////////////////////////////////////////
// Job methods
// /////////////////////////////////////////////////////////////////////////////

bool PaloJob::initialize()
{
	response = 0;

	if (jobRequest->hasSession) {
		try {
			session = PaloSession::findSession(jobRequest->sid, true);
			context = Context::getContext();
			context->setSession(session);

			if (session) {
				user = session->getUser();
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
	} else {
		user.reset();
	}

	return true;
}

// /////////////////////////////////////////////////////////////////////////////
// check token methods
// /////////////////////////////////////////////////////////////////////////////

void PaloJob::checkToken(CPServer server)
{
	if (jobRequest->serverToken == 0) {
		return;
	}

	if (server->getToken() != *jobRequest->serverToken) {
		throw ParameterException(ErrorException::ERROR_SERVER_TOKEN_OUTDATED, "server token outdated", "server", "palo server");
	}
}

void PaloJob::checkToken(CPDatabase database)
{
	if (jobRequest->databaseToken == 0) {
		return;
	}

	if (database->getToken() != *jobRequest->databaseToken) {
		throw ParameterException(ErrorException::ERROR_DATABASE_TOKEN_OUTDATED, "database token outdated", "database", database->getName());
	}
}

void PaloJob::checkToken(CPDimension dimension)
{
	if (jobRequest->dimensionToken == 0) {
		return;
	}

	if (dimension->getToken() != *jobRequest->dimensionToken) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_TOKEN_OUTDATED, "dimension token outdated", "dimension", dimension->getName());
	}
}

void PaloJob::checkToken(CPCube cube)
{
	if (jobRequest->cubeToken == 0) {
		return;
	}

	if (cube->getToken() != *jobRequest->cubeToken) {
		throw ParameterException(ErrorException::ERROR_CUBE_TOKEN_OUTDATED, "cube token outdated", "cube", cube->getName());
	}
}

// /////////////////////////////////////////////////////////////////////////////
// generate response
// /////////////////////////////////////////////////////////////////////////////

void PaloJob::generateLoginResponse(boost::shared_ptr<PaloSession> session, const string &opt)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	// append session identifier
	body.appendCsvString(session->getSid());

	// append time to live
	body.appendCsvInteger((uint32_t)session->getTtlIntervall());

	// append optional features
	body.appendCsvString(opt);

	body.appendEol();

	setToken(server);
}

void PaloJob::generateOkResponse()
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	body.appendText("1;");
	body.appendEol();
}

void PaloJob::generateMessageResponse(const string &message)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	body.appendText(message);
	body.appendEol();
}

void PaloJob::generateCellValueResponse(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals)
{
	// check for locks
	Cube::CellLockInfo lockInfo = 0;

	if (jobRequest->showLockInfo) {
		IdentifierType userId = user ? user->getId() : 0;
		lockInfo = cube->getCellLockInfo(key, userId);
	}

	StringBuffer &body = response->getBody();
	if (value.isError()) {
		appendError(body, value.getError(), value.getRuleId(), jobRequest->showRule, jobRequest->showLockInfo, prop_vals);
	} else {
		appendCell(body, value, jobRequest->showRule, jobRequest->showLockInfo, lockInfo, prop_vals);
	}
}

void PaloJob::generateDatabaseResponse(CPDatabase database)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(database);

	appendDatabase(&body, database);
}

void PaloJob::generateDatabasesResponse(CPServer server, vector<CPDatabase>* databases)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(server);

	for (vector<CPDatabase>::iterator i = databases->begin(); i != databases->end(); ++i) {
		CPDatabase database = (*i);

		bool normal = (database->getType() == NORMALTYPE) && jobRequest->showNormal;
		bool system = (database->getType() == SYSTEMTYPE) && jobRequest->showSystem;
		bool userinfo = (database->getType() == USER_INFOTYPE) && jobRequest->showUserInfo;

		if (normal || system || userinfo) {
			appendDatabase(&body, database);
		}
	}
}

void PaloJob::generateCubeResponse(CPCube cube)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(cube);
	setSecondToken(cube);

	appendCube(&body, cube);
}

void PaloJob::generateCubesResponse(CPDatabase database, vector<CPCube>* cubes, bool showNormal, bool showSystem, bool showAttribute, bool showInfo, bool showGputype)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(database);

	for (vector<CPCube>::iterator i = cubes->begin(); i != cubes->end(); ++i) {
		CPCube cube = (*i);

		ItemType it = cube->getType();
		bool normal = false;
		bool gputype = false;
		bool system = false;
		bool attribute = false;
		bool userinfo = false;

		switch (it) {
		case NORMALTYPE:
			normal = showNormal;
			break;
		case GPUTYPE:
			gputype = showGputype;
			break;
		case USER_INFOTYPE:
			userinfo = showInfo;
			break;
		case SYSTEMTYPE: {
			CPSystemCube systemCube = CONST_COMMITABLE_CAST(SystemCube, cube);
			Cube::SaveType sdt = systemCube->getCubeType();

			system = (sdt != Cube::ATTRIBUTES) && showSystem;
			attribute = (sdt == Cube::ATTRIBUTES) && showAttribute;
			break;
		}
		default:
			break;
		}

		if (normal || system || attribute || userinfo || gputype) {
			appendCube(&body, cube);
		}
	}
}

void PaloJob::generateDimensionResponse(CPDimension dimension)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(dimension);

	RightsType rt = RIGHT_NONE;
	if (jobRequest->showPermission && user) {
		rt = user->getRoleDbRight(User::dimensionRight, database);
	}
	appendDimension(&body, dimension, rt);
}

void PaloJob::generateDimensionsResponse(CPDatabase database, vector<CPDimension>* dimensions, bool showNormal, bool showSystem, bool showAttribute, bool showInfo)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(database);

	RightsType rt = RIGHT_NONE;
	if (jobRequest->showPermission && user) {
		rt = user->getRoleDbRight(User::dimensionRight, database);
	}

	for (vector<CPDimension>::iterator i = dimensions->begin(); i != dimensions->end(); ++i) {
		CPDimension dimension = (*i);

		ItemType it = dimension->getType();
		bool normal = false;
		bool system = false;
		bool attribute = false;
		bool userInfo = false;

		if (it == NORMALTYPE) {
			normal = showNormal;
		} else if (it == USER_INFOTYPE) {
			userInfo = showInfo;
		} else if (it == SYSTEMTYPE) {
			CPSystemDimension systemDimension = CONST_COMMITABLE_CAST(SystemDimension, dimension);
			Dimension::SaveType dt = systemDimension->getDimensionType();

			system = (dt != Dimension::ATTRIBUTES && dt != Dimension::CELLPROPS) && showSystem;
			attribute = (dt == Dimension::ATTRIBUTES || dt == Dimension::CELLPROPS) && showAttribute;
		}

		if (normal || system || attribute || userInfo) {
			appendDimension(&body, dimension, rt);
		}
	}
}

void PaloJob::generateElementResponse(CPDimension dimension, Element* element, bool showPermission)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(dimension);

	vector<User::RoleDbCubeRight> vRights;
	if (jobRequest->showPermission && user) {
		vRights.resize(user->getGroupCount());
		user->getDatabaseDataRight(vRights, database);
	}
	appendElement(&body, dimension, element, 0, showPermission, vRights);
}

void PaloJob::generateElementsResponse(CPDimension dimension, ElementsType* elements, uint64_t *pHiddenCount)
{
	IdentifierType skipCount = jobRequest->limitStart;
	IdentifierType responseCount = jobRequest->limitCount;
	IdentifierType elem = jobRequest->element;

	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(dimension);

	if (!responseCount) {
		// return just number of elements in selection
		body.appendInteger(elements->size());
		body.appendEol();
		return;
	}
	if (pHiddenCount) {
		body.appendCsvInteger(*pHiddenCount);
		body.appendEol();
	}
	if (elem != NO_IDENTIFIER) {
		size_t siz = elements->size();
		size_t i = skipCount;
		for (; i < siz; ++i) {
			if (elements->at(i)->getIdentifier() == elem) {
				break;
			}
		}
		if (i == siz) {
			throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with identifier " + StringUtils::convertToString((uint32_t)elem) + " not found in dimension '" + dimension->getName() + "'", "elementIdentifier", (int)elem);
		}
		skipCount = IdentifierType(((i/responseCount) * responseCount));
		body.appendCsvInteger(skipCount);
		body.appendEol();
	}

	vector<User::RoleDbCubeRight> vRights;
	if (jobRequest->showPermission && user) {
		vRights.resize(user->getGroupCount());
		user->getDatabaseDataRight(vRights, database);
	}
	for (ElementsType::iterator i = elements->begin(); i != elements->end(); ++i) {
		if (skipCount) {
			skipCount--;
			continue;
		}
		Element* element = (*i);

		appendElement(&body, dimension, element, 0, jobRequest->showPermission, vRights);

		if (!--responseCount) {
			break;
		}
	}
}

void PaloJob::generateRuleResponse(CPCube cube, CPRule rule, bool useIdentifier)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(cube);

	appendRule(&body, rule, cube, useIdentifier);
}

void PaloJob::generateRulesResponse(CPCube cube, const vector<PRule>* rules, bool useIdentifier)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(cube);

	for (vector<PRule>::const_iterator iter = rules->begin(); iter != rules->end(); ++iter) {
		appendRule(&body, *iter, cube, useIdentifier);
	}
}

void PaloJob::generateLocksResponse(CPServer server, CPCube cube, PUser user, bool completeContainsArea)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(cube);

	CPLockList locks = cube->getCubeLocks(user);
	for (LockList::ConstIterator i = locks->const_begin(); i != locks->const_end(); ++i) {
		appendLock(server, &body, CONST_COMMITABLE_CAST(Lock, *i), completeContainsArea);
	}
}

void PaloJob::generateLockResponse(CPServer server, CPCube cube, PLock lock, bool completeContainsArea)
{
	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(cube);

	if (lock) {
		appendLock(server, &body, lock, completeContainsArea);
	}
}

// /////////////////////////////////////////////////////////////////////////////
// find palo objects
// /////////////////////////////////////////////////////////////////////////////

void PaloJob::findDatabase(bool requireLoad, bool write)
{
	if (server == 0) {
		server = Context::getContext()->getServer();
	}

	if (database) {
		return;
	}

	IdentifierType id = jobRequest->database;

	if (id != NO_IDENTIFIER) {
		database = server->findDatabase(id, user, requireLoad, write);
	} else if (jobRequest->databaseName) {
		string name = *(jobRequest->databaseName);
		database = server->findDatabaseByName(name, user, requireLoad, write);
	} else {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", PaloRequestHandler::ID_DATABASE, "");
	}

	if (write && server->isCheckedOut()) {
		PDatabaseList dbs = server->getDatabaseList(true);
		server->setDatabaseList(dbs);
		dbs->set(database);
	}

	checkToken(database);
}

void PaloJob::findCube(bool requireLoad, bool write)
{
	findDatabase(true, false);

	if (cube) {
		return;
	}

	IdentifierType id = jobRequest->cube;

	if (id != NO_IDENTIFIER) {
		cube = database->findCube(id, user, requireLoad, write);
	} else if (jobRequest->cubeName) {
		string name = *(jobRequest->cubeName);
		cube = database->findCubeByName(name, user, requireLoad, write);
	} else {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube not found", PaloRequestHandler::ID_CUBE, "");
	}

	if (write && database->isCheckedOut()) {
		PCubeList cubes = database->getCubeList(true);
		database->setCubeList(cubes);
		cubes->set(cube);
	}

	checkToken(cube);
}

void PaloJob::findPath()
{
	findCube(true, false);

	const IdentifiersType * dimensions = cube->getDimensions();
	size_t numDimensions = dimensions->size();

	if (jobRequest->path) {
		if (jobRequest->path->size() != numDimensions) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::ID_PATH, "");
		}

		cellPath.reset(new CubeArea(database, cube, numDimensions));

		for (size_t i = 0; i < numDimensions; i++) {
			CPDimension d = database->lookupDimension(dimensions->at(i), false);
			PSet s(new Set);
			if (d->getDimensionType() == Dimension::VIRTUAL) {
				s->insert(jobRequest->path->at(i));
			} else {
				IdentifierType id = d->findElement(jobRequest->path->at(i), 0, false)->getIdentifier();
				s->insert(id);
			}
			cellPath->insert((IdentifierType)i, s);
		}
	} else if (jobRequest->pathName) {
		if (jobRequest->pathName->size() != numDimensions) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::NAME_PATH, "");
		}

		cellPath.reset(new CubeArea(database, cube, numDimensions));

		for (size_t i = 0; i < numDimensions; i++) {
			CPDimension d = database->lookupDimension(dimensions->at(i), false);
			IdentifierType id = d->findElementByName(jobRequest->pathName->at(i), 0, false)->getIdentifier();
			PSet s(new Set);
			s->insert(id);
			cellPath->insert((IdentifierType)i, s);
		}
	} else {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "path is empty, list of element identifiers is missing", PaloRequestHandler::ID_PATH, "");
	}
}

void PaloJob::findPathTo()
{
	findCube(true, false);

	const IdentifiersType * dimensions = cube->getDimensions();
	size_t numDimensions = dimensions->size();

	if (jobRequest->pathTo) {
		if (jobRequest->pathTo->size() != numDimensions) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::ID_PATH_TO, "");
		}

		cellPathTo.reset(new CubeArea(database, cube, numDimensions));

		for (size_t i = 0; i < numDimensions; i++) {
			CPDimension d = database->lookupDimension(dimensions->at(i), false);
			IdentifierType id = d->findElement(jobRequest->pathTo->at(i), 0, false)->getIdentifier();
			PSet s(new Set);
			s->insert(id);
			cellPathTo->insert((IdentifierType)i, s);
		}
	} else if (jobRequest->pathToName) {
		if (jobRequest->pathToName->size() != numDimensions) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::NAME_PATH_TO, "");
		}

		cellPathTo.reset(new CubeArea(database, cube, numDimensions));

		for (size_t i = 0; i < numDimensions; i++) {
			CPDimension d = database->lookupDimension(dimensions->at(i), false);
			IdentifierType id = d->findElementByName(jobRequest->pathToName->at(i), 0, false)->getIdentifier();
			PSet s(new Set);
			s->insert(id);
			cellPathTo->insert((IdentifierType)i, s);
		}
	} else {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "path to is empty, list of element identifiers is missing", PaloRequestHandler::ID_PATH_TO, "");
	}
}

void PaloJob::findDimension(bool write)
{
	findDatabase(true, false);

	if (dimension) {
		return;
	}

	IdentifierType id = jobRequest->dimension;

	if (id != NO_IDENTIFIER) {
		dimension = database->findDimension(id, user, write);
	} else if (jobRequest->dimensionName) {
		string name = *(jobRequest->dimensionName);
		dimension = database->findDimensionByName(name, user, write);
	} else {
		throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "dimension not found", PaloRequestHandler::ID_DIMENSION, "");
	}

	if (write && database->isCheckedOut()) {
		PDimensionList dims = database->getDimensionList(true);
		database->setDimensionList(dims);
		dims->set(dimension);
	}

	checkToken(dimension);
}

void PaloJob::findElement(bool write)
{
	findDimension(false);

	if (element) {
		return;
	}

	IdentifierType id = jobRequest->element;

	if (id != NO_IDENTIFIER) {
		element = dimension->findElement(id, user.get(), write);
	} else if (jobRequest->elementName) {
		string name = *(jobRequest->elementName);
		element = dimension->findElementByName(name, user.get(), write);
	} else {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element not found", PaloRequestHandler::ID_ELEMENT, "");
	}
}

void PaloJob::findRules(bool write)
{
	findCube(true, false);

	IdentifierType *rulesBegin = (jobRequest->rules && jobRequest->rules->size()) ? &jobRequest->rules->at(0) : &jobRequest->rule;
	IdentifierType *rulesEnd = (jobRequest->rules && jobRequest->rules->size()) ? &jobRequest->rules->at(0)+jobRequest->rules->size() : &jobRequest->rule + 1;

	rules.reserve(rulesEnd-rulesBegin);

	for (IdentifierType *rulesIt = rulesBegin; rulesIt != rulesEnd; ++rulesIt) {
		PRule rule = cube->findRule(*rulesIt);
		if (write && cube->isCheckedOut()) {
			PRuleList ruleList = cube->getRuleList(true);
			cube->setRuleList(ruleList);
			if (rule && !rule->isCheckedOut()) {
				rule = COMMITABLE_CAST(Rule, rule->copy());
			}
			ruleList->set(rule);
		}
		rules.push_back(rule);
	}

}

void PaloJob::findPathsIntern(vector<IdentifiersType> *sourcePaths, PPaths &targetPaths, set<size_t> *invalidPaths, User *user)
{
	findCube(true, false);

	const IdentifiersType * dimensions = cube->getDimensions();
	size_t numDimensions = dimensions->size();

	vector<PDimension> dims(numDimensions);
	for (size_t i = 0; i < numDimensions; i++) {
		dims[i] = database->lookupDimension(dimensions->at(i), false);
	}

	IdentifiersType path(numDimensions);

	targetPaths.reset(new vector<IdentifiersType>());
	targetPaths->reserve(sourcePaths->size());

	for (size_t j = 0; j < sourcePaths->size(); j++) {
		IdentifiersType& it = sourcePaths->at(j);

		if (it.size() != numDimensions) {
			if (invalidPaths) {
				invalidPaths->insert(j);
			}
			for (size_t i = 0; i < numDimensions; i++) {
				path[i] = -1;
			}
		} else {
			for (size_t i = 0; i < numDimensions; i++) {
				try {
					if (dims[i]->getDimensionType() == Dimension::VIRTUAL) {
						path[i] = it.at(i);
					} else {
						path[i] = dims[i]->findElement(it.at(i), user, false)->getIdentifier();
					}
				} catch (ErrorException &e) {
					if (invalidPaths && (e.getErrorType() == ErrorException::ERROR_ELEMENT_NOT_FOUND)) {
						invalidPaths->insert(j);
						path[i] = it.at(i);
					} else {
						throw;
					}
				}
			}
		}

		targetPaths->push_back(path);
	}
}

void PaloJob::updateLicenses()
{
	if (updateLics) {
		context->setIgnoreStop(true);
		if (database && database->getType() == SYSTEMTYPE) {
			PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, database);
			PCube userPropCube = system->getUserUserPropertiesCube();
			if (userPropCube && cube && userPropCube->getId() == cube->getId()) {
				server->updateLicenses(system);
			}
		}
		updateLics = false;
		context->setIgnoreStop(false);
	}
}

void PaloJob::findLockedPaths(set<size_t> *invalidPaths)
{
	if (jobRequest->lockedPaths) {
		findPathsIntern(jobRequest->lockedPaths, lockedPaths, invalidPaths, user.get());
	} else {
		// optional argument, ok if missing
		//throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "path is empty, list of element identifiers is missing", PaloRequestHandler::ID_LOCKED_PATHS, "");
	}
}

void PaloJob::findCellPaths(set<size_t> *invalidPaths, User *user)
{
	if (jobRequest->paths) {
		findPathsIntern(jobRequest->paths, cellPaths, invalidPaths, user);
	} else if (jobRequest->pathsName) {
		findCube(true, false);

		const IdentifiersType * dimensions = cube->getDimensions();
		size_t numDimensions = dimensions->size();

		vector<PDimension> dims(numDimensions);
		for (size_t i = 0; i < numDimensions; i++) {
			dims[i] = database->lookupDimension(dimensions->at(i), false);
		}

		IdentifiersType path(numDimensions);

		cellPaths.reset(new vector<IdentifiersType>());

		for (size_t j = 0; j < jobRequest->pathsName->size(); j++) {
			vector<string>& names = jobRequest->pathsName->at(j);

			if (names.size() != numDimensions) {
				if (invalidPaths) {
					invalidPaths->insert(j);
				}
				for (size_t i = 0; i < numDimensions; i++) {
					path[i] = -1;
				}
			} else {
				for (size_t i = 0; i < numDimensions; i++) {
					try {
						path[i] = dims[i]->findElementByName(names.at(i), user, false)->getIdentifier();
					} catch (ErrorException &e) {
						if (invalidPaths && (e.getErrorType() == ErrorException::ERROR_ELEMENT_NOT_FOUND)) {
							invalidPaths->insert(j);
							path[i] = -1;
						} else {
							throw;
						}
					}
				}
			}

			cellPaths->push_back(path);
		}

	} else {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "path is empty, list of element identifiers is missing", PaloRequestHandler::ID_PATHS, "");
	}
}

void PaloJob::checkProperties()
{
	if (jobRequest->properties && jobRequest->properties->size()) {
		CPNormalDatabase nd = COMMITABLE_CAST(NormalDatabase, database);
		if (!nd) {
			IdentifiersType::iterator it = jobRequest->properties->begin();
			throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "property element with identifier " + StringUtils::convertToString((uint32_t)*it) + " not found", "elementIdentifier", (int)*it);
		}
		CPDimension d = nd->getCellPropertiesDimension();
		for (IdentifiersType::iterator it = jobRequest->properties->begin(); it != jobRequest->properties->end(); ++it) {
			d->findElement(*it, 0, false);
		}
	}
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloJob::appendDatabase(StringBuffer* sb, CPDatabase database)
{
	sb->appendCsvInteger((int32_t)database->getIdentifier());
	sb->appendCsvString(StringUtils::escapeString(database->getName()));
	sb->appendCsvInteger((int32_t)database->sizeDimensions());
	sb->appendCsvInteger((int32_t)database->sizeCubes());
	Database::DatabaseStatus status = database->getStatus();
	sb->appendCsvInteger((int32_t)(status == Database::LOADING ? Database::UNLOADED : status));
	sb->appendCsvInteger((int32_t)database->getType());
	sb->appendCsvInteger((int32_t)database->getToken());
	if (jobRequest->showPermission) {
		if (user) {
			RightsType rt = user->getRoleDbRight(User::databaseRight, database);
			sb->appendCsvString(StringUtils::escapeString(User::rightsTypeToString(rt)));
		} else {
			sb->appendChar(';');
		}
	}
	sb->appendEol();
}

void PaloJob::appendDimension(StringBuffer* sb, CPDimension dimension, RightsType dimRight)
{
	sb->appendCsvInteger((int32_t)dimension->getId());
	sb->appendCsvString(StringUtils::escapeString(dimension->getName()));
	sb->appendCsvInteger((int32_t)dimension->sizeElements());
	sb->appendCsvInteger((int32_t)dimension->getLevel());
	sb->appendCsvInteger((int32_t)dimension->getIndent());
	sb->appendCsvInteger((int32_t)dimension->getDepth());

	int32_t dimType;
	switch (dimension->getDimensionType()) {
	case Dimension::NORMAL:
		dimType = 0;
		break;
	case Dimension::ATTRIBUTES:
	case Dimension::CELLPROPS:
		dimType = 2;
		break;
	case Dimension::USERINFO:
		dimType = 3;
		break;
	case Dimension::VIRTUAL:
		dimType = 4;
		break;
	default:
		dimType = 1;
		break;
	}
	sb->appendCsvInteger(dimType);

	IdentifierType id = NO_IDENTIFIER;
	if (dimension->isAttributed()) {
		CPAttributesDimension ad = AttributedDimension::getAttributesDimension(database, dimension->getName());
		if (ad) {
			id = (int32_t)ad->getId();
		}
		appendIdentifier(sb, id);

		id = NO_IDENTIFIER;
		if (dimension->isAttributed()) {
			CPSystemCube ac = AttributedDimension::getAttributesCube(database, dimension->getName());
			if (ac) {
				id = (int32_t)ac->getId();
			}
		}
		appendIdentifier(sb, id);
	} else {
		if (dimension->getDimensionType() == Dimension::ATTRIBUTES) {
			CPNormalDimension nd = CONST_COMMITABLE_CAST(AttributesDimension, dimension)->getNormalDimension();
			if (nd) {
				id = nd->getId();
			}
		}
		appendIdentifier(sb, id);
		sb->appendChar(';');
	}

	id = NO_IDENTIFIER;
	if (dimension->hasRightsCube()) {
		CPRightsCube rc = DRCubeDimension::getRightsCube(database, dimension->getName());
		if (rc) {
			id = rc->getId();
		}
	}
	appendIdentifier(sb, id);

	sb->appendCsvInteger((int32_t)dimension->getToken());

	if (jobRequest->showPermission) {
		if (user) {
			// appends user->getRoleDbRight(User::dimensionRight, database)
			// this is analogous to cubes where user->getRoleDbRight(User::cubeRight, database) is used
			sb->appendCsvString(StringUtils::escapeString(User::rightsTypeToString(dimRight)));
		} else {
			sb->appendChar(';');
		}
	}

	sb->appendEol();
}

void PaloJob::appendElement(StringBuffer* sb, CPDimension dimension, const Element* element, IdentifierType elemId, bool showPermission, vector<User::RoleDbCubeRight> &vRights, IdentifierType depth, IdentifierType indent)
{
	if (element) {
		sb->appendCsvInteger((int32_t)element->getIdentifier());
		sb->appendCsvString(StringUtils::escapeString(element->getName(dimension->getElemNamesVector())));
		sb->appendCsvInteger((int32_t)element->getPosition());
		sb->appendCsvInteger((int32_t)element->getLevel());
		sb->appendCsvInteger((int32_t)(indent == NO_IDENTIFIER ? element->getIndent() : indent));
		sb->appendCsvInteger((int32_t)(depth == NO_IDENTIFIER ? element->getDepth() : depth));
		sb->appendCsvInteger((int32_t)element->getElementType());

		CPParents parents = element->getParents();

		sb->appendCsvInteger((int32_t)(parents ? parents->size() : 0));

		// append parent identifier
		bool b = false;

		if (parents) {
			for (Parents::const_iterator pi = parents->begin(); pi != parents->end(); ++pi) {
				if (b) {
					sb->appendChar(',');
				}

				sb->appendInteger((uint32_t)*pi);
				b = true;
			}
		}

		sb->appendChar(';');

		// append children identifier
		const IdentifiersWeightType *children = element->getChildren();

		sb->appendCsvInteger((int32_t)(children ? children->size() : 0));

		b = false;

		if (children) {
			for (IdentifiersWeightType::const_iterator ci = children->begin(); ci != children->end(); ++ci) {
				if (b) {
					sb->appendChar(',');
				}

				sb->appendInteger((uint32_t)(*ci).first);
				b = true;
			}
		}

		sb->appendChar(';');

		// append children weight
		b = false;

		if (children) {
			for (IdentifiersWeightType::const_iterator cw = children->begin(); cw != children->end(); ++cw) {
				if (b) {
					sb->appendChar(',');
				}

				sb->appendDecimal((*cw).second);
				b = true;
			}
		}

		sb->appendChar(';');

		if (showPermission) {
			if (user) {
				RightsType rtr = dimension->getElementAccessRight(user.get(), database);
				RightsType rt = user->getElementRight(database->getId(), dimension->getId(), element->getIdentifier(), vRights, false);
				rt = min(rtr, rt);
				sb->appendCsvString(StringUtils::escapeString(User::rightsTypeToString(rt)));
			} else {
				sb->appendChar(';');
			}
		}
	} else {
		sb->appendCsvInteger(elemId);
		sb->appendCsvString(StringUtils::convertToString(elemId));
		sb->appendCsvInteger(elemId);
		sb->appendCsvInteger(0);
		sb->appendCsvInteger(0);
		sb->appendCsvInteger(0);
		sb->appendCsvInteger(Element::NUMERIC);
		sb->appendCsvInteger(0);
		sb->appendChar(';');
		sb->appendCsvInteger(0);
		sb->appendChar(';');
		sb->appendChar(';');
		if (showPermission) {
			sb->appendChar(';');
		}
	}
	sb->appendEol();
}

void PaloJob::appendCube(StringBuffer* sb, CPCube cube)
{
	sb->appendCsvInteger((int32_t)cube->getId());
	sb->appendCsvString(StringUtils::escapeString(cube->getName()));

	const IdentifiersType* dimensions = cube->getDimensions();

	sb->appendCsvInteger((int32_t)dimensions->size());

	bool b = false;
	double sizeCube = 1;

	for (IdentifiersType::const_iterator pi = dimensions->begin(); pi != dimensions->end(); ++pi) {
		CPDimension dimension = database->lookupDimension(*pi, false);

		if (b) {
			sb->appendChar(',');
		}

		sb->appendInteger(*pi);
		b = true;

		sizeCube *= dimension->sizeElements();
	}

	sb->appendChar(';');

	sb->appendCsvDouble(sizeCube);
	sb->appendCsvInteger(cube->sizeFilledCells());

	sb->appendCsvInteger((int32_t)cube->getStatus());

	ItemType it = cube->getType();

	switch (it) {
	case USER_INFOTYPE:
		sb->appendCsvInteger((int32_t)3);
		break;
	case GPUTYPE:
		sb->appendCsvInteger((int32_t)4);
		break;
	case NORMALTYPE:
		sb->appendCsvInteger((int32_t)0);
		break;
	case SYSTEMTYPE: {
		CPSystemCube systemCube = CONST_COMMITABLE_CAST(SystemCube, cube);

		switch (systemCube->getCubeType()) {
			case Cube::ATTRIBUTES:
				sb->appendCsvInteger((uint32_t)2);
				break;
			default:
				sb->appendCsvInteger((uint32_t)1);
			}
	}
		break;
	default:
		break;
	}

	sb->appendCsvInteger((int32_t)cube->getToken());

	if (jobRequest->showPermission) {
		if (user) {
			RightsType rtr = user->getRoleDbRight(User::cubeRight, database);
			RightsType rt = cube->getCubeAccessRight(user);
			rt = min(rtr, rt);
			sb->appendCsvString(StringUtils::escapeString(User::rightsTypeToString(rt)));
		} else {
			sb->appendChar(';');
		}
	}

	sb->appendEol();
}

void PaloJob::appendRule(StringBuffer *sb, CPRule rule, CPCube cube, bool useIdentifier)
{
	sb->appendCsvInteger((int32_t)rule->getId());

	StringBuffer sb2;
	rule->appendRepresentation(&sb2, database, cube, !useIdentifier);
	sb->appendCsvString(StringUtils::escapeString(sb2.c_str()));

	sb->appendCsvString(StringUtils::escapeString(rule->getExternal()));
	sb->appendCsvString(StringUtils::escapeString(rule->getComment()));
	sb->appendCsvInteger((int32_t)rule->getTimeStamp());

	if (rule->isActive()) {
		sb->appendCsvString("1");
	} else {
		sb->appendCsvString("0");
	}
	sb->appendCsvDouble(rule->getPosition());

	sb->appendEol();
}

void PaloJob::appendLock(CPServer server, StringBuffer* sb, CPLock lock, bool completeContainsArea)
{
	sb->appendCsvInteger((int32_t)lock->getId());
	if (completeContainsArea)
		appendArea(sb, lock->getContainsArea());
	else
		sb->appendCsvString(lock->getAreaString());

	// we need a system database
	PSystemDatabase sd = server->getSystemDatabase();

	PUser user = sd->getUser(lock->getUserIdentifier());
	if (user) {
		sb->appendCsvString(user->getName());
	} else {
		sb->appendCsvString("");
	}
	sb->appendCsvInteger((int32_t)(lock->isWhole() ? 0 : lock->getStorage()->getNumberSteps()));

	sb->appendEol();
}

void PaloJob::appendArea(StringBuffer* sb, CPArea area)
{
	//1:2:3,4:5:6,...
	for (size_t dit = 0; dit < area->dimCount(); dit++) {
		if (dit) {
			sb->appendChar(',');
		}
		for (Area::ConstElemIter it = area->elemBegin(dit); it != area->elemEnd(dit); ++it) {
			if (it != area->elemBegin(dit)) {
				sb->appendChar(':');
			}
			sb->appendInteger(*it);
		}
	}
	sb->appendChar(';');
}

void PaloJob::appendArea(StringBuffer* sb, const vector<IdentifiersType> &area)
{
	//1:2:3,4:5:6,...
	for (vector<IdentifiersType>::const_iterator dit = area.begin(); dit != area.end(); ++dit) {
		if (dit != area.begin())
			sb->appendChar(',');
		for (IdentifiersType::const_iterator it = dit->begin(); it != dit->end(); ++it) {
			if (it != dit->begin())
				sb->appendChar(':');
			sb->appendInteger(*it);
		}
	}
	sb->appendChar(';');
}

void PaloJob::appendCell(StringBuffer& body, const CellValue& value, bool showRule, bool showLockInfo, Cube::CellLockInfo lockInfo, const vector<CellValue> &prop_vals)
{
	body.appendCsvInteger((int32_t)(value.isString() ? Element::STRING : Element::NUMERIC));

	if (!value.isEmpty()) {
		body.appendCsvString("1");
		body.appendCsvString(value.toString());
	} else {
		body.appendCsvString("0;");
	}

	if (showRule) {
		if (value.getRuleId() != NO_RULE) {
			body.appendCsvInteger((uint32_t)value.getRuleId());
		} else {
			body.appendChar(';');
		}
	}
	if (showLockInfo) {
		body.appendCsvInteger(lockInfo);
	}
	if (!prop_vals.empty()) {
		for (vector<CellValue>::const_iterator it = prop_vals.begin(); it != prop_vals.end(); ++it) {
			if (it != prop_vals.begin()) {
				body.appendChar(',');
			}
			if (!it->isEmpty()) {
				body.appendText(it->toString());
			}
		}
		body.appendChar(';');
	}

	body.appendEol();
}

///////////////////////////////////////////////////////////////////////////////
// helper methods
///////////////////////////////////////////////////////////////////////////////

Element::Type PaloJob::elementTypeByIdentifier(uint32_t type)
{
	Element::Type elementType;

	if (type == 1) {
		elementType = Element::NUMERIC;
	} else if (type == 2) {
		elementType = Element::STRING;
	} else if (type == 4) {
		elementType = Element::CONSOLIDATED;
	} else {
		elementType = Element::UNDEFINED;
	}

	if (elementType == Element::UNDEFINED) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "wrong value for element type", PaloRequestHandler::ID_TYPE, type);
	}

	return elementType;
}

PCubeArea PaloJob::area(PDatabase database, PCube cube, vector<IdentifiersType>* paths, const vector<CPDimension> * dimensions, uint32_t& numResult, bool useBaseOnly)
{
	if (dimensions->size() != paths->size()) {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
	}

	size_t numDimensions = dimensions->size();

	PCubeArea area(new CubeArea(database, cube, numDimensions));

	numResult = 1;

	for (uint32_t i = 0; i < numDimensions; i++) {
		PSet s(new Set);
		if (paths->at(i).size() == 0) {
			CPDimension dim = dimensions->at(i);
			ElementsType elements = dim->getElements(PUser(), false);

			if (dim->getDimensionType() == Dimension::VIRTUAL) {
				s.reset(new Set(true));
			} else {
				for (ElementsType::iterator j = elements.begin(); j != elements.end(); ++j) {
					Element * e = *j;

					if (!(useBaseOnly && e->getElementType() == Element::CONSOLIDATED)) {
						s->insert(e->getIdentifier());
					}
				}
			}
		} else {
			for (size_t j = 0; j < paths->at(i).size(); j++) {
				if ((*dimensions)[i]->getDimensionType() == Dimension::VIRTUAL) {
					s->insert(paths->at(i).at(j));
				} else {
					IdentifierType id = (*dimensions)[i]->findElement(paths->at(i).at(j), 0, false)->getIdentifier();
					s->insert(id);
				}
			}
		}
		area->insert(i, s);

		numResult *= (uint32_t)area->elemCount(i);
	}

	return area;
}

PCubeArea PaloJob::area(PDatabase database, PCube cube, vector<vector<string> >* paths, const vector<CPDimension> * dimensions, uint32_t& numResult, bool useBaseOnly)
{
	if (dimensions->size() != paths->size()) {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
	}

	size_t numDimensions = dimensions->size();

	PCubeArea area(new CubeArea(database, cube, numDimensions));

	numResult = 1;

	for (uint32_t i = 0; i < numDimensions; i++) {
		PSet s(new Set);
		if (paths->at(i).size() == 0) {
			ElementsType elements = dimensions->at(i)->getElements(PUser(), false);

			for (ElementsType::iterator j = elements.begin(); j != elements.end(); ++j) {
				Element * e = *j;

				if (!(useBaseOnly && e->getElementType() == Element::CONSOLIDATED)) {
					s->insert(e->getIdentifier());
				}
			}
		} else {
			for (size_t j = 0; j < paths->at(i).size(); j++) {
				IdentifierType id = (*dimensions)[i]->findElementByName(paths->at(i).at(j), 0, false)->getIdentifier();
				s->insert(id);
			}
		}
		area->insert(i, s);

		numResult *= (uint32_t)area->elemCount(i);
	}

	return area;
}

string PaloJob::areaToString(vector<vector<string> >* paths)
{
	string result = "";
	StringBuffer sb;

	for (uint32_t i = 0; i < paths->size(); i++) {
		if (i > 0) {
			sb.appendChar(',');
		}

		IdentifiersType path;

		if (paths->at(i).size() == 0) {
			sb.appendChar('*');
		} else {
			for (size_t j = 0; j < paths->at(i).size(); j++) {
				if (j > 0)
					sb.appendChar(':');
				sb.appendText(paths->at(i).at(j));
			}
		}
	}

	result = sb.c_str();
	return result;
}

string PaloJob::areaToString(vector<IdentifiersType>* paths)
{
	string result = "";
	StringBuffer sb;

	for (uint32_t i = 0; i < paths->size(); i++) {
		if (i > 0) {
			sb.appendChar(',');
		}

		IdentifiersType path;

		if (paths->at(i).size() == 0) {
			sb.appendChar('*');
		} else {
			for (size_t j = 0; j < paths->at(i).size(); j++) {
				if (j > 0)
					sb.appendChar(':');
				IdentifierType id = paths->at(i).at(j);
				sb.appendInteger(id);
			}
		}
	}

	result = sb.c_str();
	return result;
}

string PaloJob::areaToString(size_t dims)
{
	string result = "";
	StringBuffer sb;

	for (uint32_t i = 0; i < dims; i++) {
		if (i > 0) {
			sb.appendChar(',');
		}
		sb.appendChar('*');
	}

	result = sb.c_str();
	return result;
}

SplashMode PaloJob::splashMode(uint32_t splash)
{
	switch (splash) {
	case 0:
		return DISABLED;
	case 1:
		return DEFAULT;
	case 2:
		return ADD_BASE;
	case 3:
		return SET_BASE;
	default:
		throw ParameterException(ErrorException::ERROR_INVALID_SPLASH_MODE, "wrong value for splash mode", PaloRequestHandler::SPLASH, splash);
	}

}

uint32_t PaloJob::splashNumber(SplashMode splash)
{
	switch (splash) {
	case DISABLED:
		return 0;
	case DEFAULT:
		return 1;
	case ADD_BASE:
		return 2;
	case SET_BASE:
		return 3;
	default:
		throw ParameterException(ErrorException::ERROR_INVALID_SPLASH_MODE, "wrong value for splash mode", PaloRequestHandler::SPLASH, splash);
	}

}

void PaloJob::appendError(StringBuffer &sb, ErrorException::ErrorType type, IdentifierType idRule, bool showRule, bool showLockInfo, const vector<CellValue> &prop_vals)
{
	sb.appendCsvInteger((int32_t)99);
	sb.appendCsvInteger((int32_t)type);
	sb.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(type)));

	if (showRule) {
		if (idRule != NO_RULE) {
			sb.appendCsvInteger((uint32_t)idRule);
		} else {
			sb.appendChar(';');
		}
	}
	if (showLockInfo) {
		sb.appendCsvInteger(0);
	}
	if (!prop_vals.empty()) {
		for (vector<CellValue>::const_iterator it = prop_vals.begin(); it != prop_vals.end(); ++it) {
			if (it != prop_vals.begin()) {
				sb.appendChar(',');
			}
			if (!it->isEmpty()) {
				sb.appendText(it->toString());
			}
		}
		sb.appendChar(';');
	}

	sb.appendEol();
}

void PaloJob::appendIdentifier(StringBuffer* sb, IdentifierType id)
{
	if (id == NO_IDENTIFIER) {
		sb->appendChar(';');
	} else {
		sb->appendCsvInteger((int32_t)id);
	}
}

}
