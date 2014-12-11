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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Server.h"

#include "Context.h"

#include "Engine/Legacy/Engine.h"
#include "Olap/Rule.h"

namespace palo {

Context::Context() : svsChangeStatusContext(SVS_NONE), updateToken(true), refreshUsers(false), optimistic(true), worker(false),
	filteredFromMarkers(0), rulesContext(0), stopJob(NO_STOP), ignoreStopJob(false), inJournal(false), task(0), saveToCache(true)
{
	if (getServer()) {
		PEngineList engineList = getServer()->getEngineList(false);
		for (EngineList::Iterator engineIt = engineList->begin(); engineIt != engineList->end(); ++engineIt) {
			availableEngines.insert(EngineBase::Type((*engineIt)->getId()));
			//push_back(COMMITABLE_CAST(EngineBase, *engineIt));
		}
	}
}

Context::~Context()
{
	freeEngineCube();
	if (rulesContext) {
		delete rulesContext;
	}
}

boost::thread_specific_ptr<Context> Context::context;

PServer Context::getServer()
{
	if (server == 0) {
		server = Server::getInstance(false);
	}
	return server;
}

PServer Context::getServerCopy()
{
	if (server == 0 || !server->isCheckedOut()) {
		server = Server::getInstance(true);
	}
	return server;
}

Context *Context::getContext(bool *wasNew, bool checkTermination)
{
	if (wasNew) {
		*wasNew = false;
	}
	if (!context.get()) {
		context.reset(new Context()); // initialize the thread's storage
		if (wasNew) {
			*wasNew = true;
		}
	} else if (checkTermination) {
		context->check();
	}
	return context.get();
}

void Context::saveParent(const CPCommitable& parent, const CPCommitable& child)
{
	if (child) {
		map<CPCommitable, CPCommitable>::iterator it = relations.find(child);
		if (it == relations.end()) {
			relations.insert(make_pair(child, parent));
		} else {
			it->second = parent;
		}
	}
}

CPCommitable Context::getParent(const CPCommitable &child)
{
	map<CPCommitable, CPCommitable>::iterator it = relations.find(child);
	if (it == relations.end()) {
		return CPCommitable(); // throw exception here?
	} else {
		return it->second;
	}
}

void Context::setEngineCube(paloLegacy::ECube* c)
{
	const Cube *cube = c->acube;
	CPCube cpCube = CONST_COMMITABLE_CAST(Cube, cube->shared_from_this());
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cpCube));

	engineCubes.insert(make_pair(make_pair(db->getId(), cube->getId()), c));
}

paloLegacy::ECube *Context::getEngineCube(dbID_cubeID db_cube) const
{
	std::map<dbID_cubeID, paloLegacy::ECube*>::const_iterator it = engineCubes.find(db_cube);
	if (it != engineCubes.end()) {
		return it->second;
	}
	return 0;
}

void Context::eraseEngineCube(dbID_cubeID db_cube)
{
	std::map<dbID_cubeID, paloLegacy::ECube*>::iterator it = engineCubes.find(db_cube);
	if (it != engineCubes.end()) {
		delete it->second;
		engineCubes.erase(it);
	}
}

RulesContext *Context::getRulesContext()
{
	if (!rulesContext) {
		rulesContext = new RulesContext();
	}
	return rulesContext;
}

void Context::freeEngineCube()
{
	for (std::map<dbID_cubeID, paloLegacy::ECube*>::iterator it = engineCubes.begin(); it != engineCubes.end(); ++it) {
		delete it->second;
	}
	engineCubes.clear();
}

pair<ContextStream, bool> &Context::getJournal(PJournalFile journal)
{
	std::map<PJournalFile, pair<ContextStream, bool> >::iterator it = journalstreams.find(journal);
	if (it == journalstreams.end()) {
		ContextStream s;
		it = journalstreams.insert(make_pair(journal, make_pair(s, true))).first;
	}
	return it->second;
}

ostream &Context::getJournalStream(PJournalFile journal)
{
	return getJournal(journal).first;
}

bool Context::getJournalIsFirst(PJournalFile journal)
{
	return getJournal(journal).second;
}

void Context::setJournalIsFirst(PJournalFile journal, bool isFirst)
{
	getJournal(journal).second = isFirst;
}

void Context::flushJournals()
{
	for (std::map<PJournalFile, pair<ContextStream, bool> >::iterator it = journalstreams.begin(); it != journalstreams.end(); ++it) {
		it->first->flush(it->second.first);
	}
}

void Context::deleteJournalStream(PJournalFile journal)
{
	journalstreams.erase(journal);
}

void Context::deleteCubesFromDisk()
{
	// TODO -jj- delete storage from all engines here?
	for (list<PCube>::iterator it = cubesToDelete.begin(); it != cubesToDelete.end(); ++it) {
		(*it)->deleteCubeFiles();
	}
}
void Context::deleteDatabasesFromDisk()
{
	for (list<PDatabase>::iterator it = dbsToDelete.begin(); it != dbsToDelete.end(); ++it) {
		(*it)->deleteDatabaseFiles(server);
	}
}

void Context::addCubeToDelete(PCube cube)
{
	cubesToDelete.push_back(cube);
}

void Context::addDatabaseToDelete(PDatabase db)
{
	dbsToDelete.push_back(db);
}

void Context::addNewMarkerRule(IdentifierType db, IdentifierType cube, IdentifierType rule)
{
	newMarkerRules.insert(make_pair(db, make_pair(cube, rule)));
}

void Context::removeNewMarkerRuleJournal(IdentifierType db, IdentifierType cube, IdentifierType rule)
{
	RuleId ri = make_pair(db, make_pair(cube, rule));
	RuleIds::iterator it = newMarkerRules.find(ri);
	if (it != newMarkerRules.end()) {
		newMarkerRules.erase(it);
	}
}

void Context::calcNewMarkerRules()
{
	if (newMarkerRules.empty()) {
		return;
	}

	for (RuleIds::iterator i = newMarkerRules.begin(); i != newMarkerRules.end(); ++i) {
		PCube cube = Context::getContext()->getServer()->lookupDatabase(i->first, false)->lookupCube(i->second.first, false);
		if (!cube) {
			Logger::trace << "calcNewMarkerRules: cube already deleted" << endl;
			continue;
		}
		PRule rule;
		try {
			rule = cube->findRule(i->second.second);
		} catch (const ErrorException &ex) {
			Logger::error << "rule not found, " << ex.getMessage() << " (" << ex.getDetails() << ")" << endl;
			continue;
		}

		try {
			rule->computeMarkers();
		} catch (const ErrorException &ex) {
			Logger::error << "cannot compute markers for rule, got " << ex.getMessage() << " (" << ex.getDetails() << ")" << endl;
			throw;
		}
	}

	newMarkerRules.clear();
}

bool Context::makeCubeChanges(bool merge, PServer actual)
{
	bool ret = true;
	if (merge != optimistic) {
		PCellValueContext cvc = context->getCellValueContext();
		if (cvc) {
			if (merge) {
				PDatabaseList dbs = actual->getDatabaseList(true);
				server->setDatabaseList(dbs);
				PEngineList engines = actual->getEngineListCopy();
				server->setEngineList(engines);
				cvc->db = server->lookupDatabase(cvc->db->getId(), true);
				if (!cvc->db) {
					throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", "database", "");
				}
				dbs->set(cvc->db);
				PCubeList cubes = cvc->db->getCubeList(true);
				cvc->db->setCubeList(cubes);
				cvc->cube = cvc->db->lookupCube(cvc->cube->getId(), true);
				if (!cvc->cube) {
					throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube not found", "cube", "");
				}
				cubes->set(cvc->cube);
			}
			switch (cvc->job) {
			case CellValueContext::CELL_REPLACE_JOB:
			case CellValueContext::CELL_REPLACE_BULK_JOB:
				ret = cvc->cube->setCellValue(cvc->job == CellValueContext::CELL_REPLACE_BULK_JOB) == RESULT_OK;
				break;
			case CellValueContext::CELL_GOAL_SEEK_JOB:
				cvc->cube->cellGoalSeek();
				ret = true;
				break;
			case CellValueContext::CELL_COPY_JOB:
				ret = cvc->cube->copyCells();
				break;
			}
			setCellValueContext(PCellValueContext());
		}

		PElementsContext ec = context->getElementsContext();
		if (ret && ec) {
			if (merge) {
				PDatabaseList dbs = actual->getDatabaseList(true);
				server->setDatabaseList(dbs);
				ec->db = server->lookupDatabase(ec->db->getId(), true);
				if (!ec->db) {
					throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", "database", "");
				}
				dbs->set(ec->db);
				PDimensionList dimensions = ec->db->getDimensionList(true);
				ec->db->setDimensionList(dimensions);
				ec->dim = ec->db->lookupDimension(ec->dim->getId(), true);
				if (!ec->dim) {
					throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "dimension not found", "dimension", "");
				}
				dimensions->set(ec->dim);
			}
			ec->dim->deleteElements(ec->srv, ec->db, ec->elements, ec->user, true, NULL, ec->useDimensionWorker);
			ec->dim->updateElementsInfo();
			setElementsContext(PElementsContext());
		}

		if (ret && context->getSvsChangeStatusContext() != SVS_NONE) {
			if (merge) {
				PDatabaseList dbs = actual->getDatabaseList(true);
				server->setDatabaseList(dbs);

				vector<PCube> cubesToStop;
				for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
					PDatabase db = COMMITABLE_CAST(Database, (*i)->copy());
					dbs->set(db);

					PCubeList cubes = db->getCubeList(true);
					db->setCubeList(cubes);

					for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
						CPCube ccube = CONST_COMMITABLE_CAST(Cube, (*j));
						if (ccube->supportsWorker()) {
							PCube cube = COMMITABLE_CAST(Cube, (*j)->copy());
							cubes->set(cube);

							if (cube->getCubeWorker()) {
								cubesToStop.push_back(cube);
							}
						}
					}
				}

				server->svsRestart(cubesToStop, context->getSvsChangeStatusContext());
			}
			setSvsChangeStatusContext(SVS_NONE);
		}

		if (server) {
			Logger::trace << "Rebuilding markers" << endl;
			calcNewMarkerRules();
			server->triggerMarkerCalculation();
		}
	}
	return ret;
}

void Context::reset(bool wasNew)
{
	if (context.get() && context->isWorker()) {
		context->setTask(NULL);
		context.release()->setWorker(false);
	} else {
		if (wasNew) {
			context.reset();
		}
	}
}

int16_t Context::savePaloDataCube(CPCube cube, CPDatabase db)
{
	size_t pos = 0;
	for (std::vector<pair<CPCube, CPDatabase> >::const_iterator it = paloDataSourceCubes.begin(); it != paloDataSourceCubes.end() && pos < 32767 ; ++it, pos++) {
		if (it->first == cube && it->second == db) {
			return (int16_t)pos;
		}
	}
	if (pos >= 32766) { // too many cubes
		return -1;
	}
	paloDataSourceCubes.push_back(make_pair(cube, db));
	return (int16_t)pos;
}

bool Context::getPaloDataCube(int16_t pos, CPCube &cube, CPDatabase &db)
{
	if (pos != -1 && (size_t)pos < paloDataSourceCubes.size()) {
		cube = paloDataSourceCubes[pos].first;
		db = paloDataSourceCubes[pos].second;
		return true;
	}
	return false;
}

void Context::disableEngine(EngineBase::Type engineType) {
	availableEngines.erase(engineType);
}

boost::shared_ptr<ValueCache::QueryCache> Context::getQueryCache(CPCube cube, size_t initSize, ValueCache &cache)
{
	boost::shared_ptr<ValueCache::QueryCache> res = getQueryCache(cube);
	if (!res) {
		res.reset(new ValueCache::QueryCache(cube, initSize, cache));
		queryCache.insert(make_pair(cube.get(), res));
	}
	return res;
}

boost::shared_ptr<ValueCache::QueryCache> Context::getQueryCache(CPCube cube)
{
	if (!cube) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Trying to cache invalid cube");
	}
	map<const Cube *, boost::shared_ptr<ValueCache::QueryCache> >::iterator it = queryCache.find(cube.get());
	if (it == queryCache.end()) {
		return boost::shared_ptr<ValueCache::QueryCache>();
	}
	return it->second;
}

void Context::setCacheDependence(const dbID_cubeID &dbCubeId)
{
	for (Context::CacheDependences::iterator cdit = cacheDependences.begin(); cdit != cacheDependences.end(); ++cdit) {
		(*cdit)->addSourceCube(dbCubeId);
	}
}

}
