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

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "palo.h"
#include "Commitable.h"
#include "Thread/Mutex.h"
#include "Exceptions/ErrorException.h"
#include <boost/thread/tss.hpp>
#include "Engine/EngineBase.h"
#include "Engine/Cache.h"
#include "Scheduler/IoTask.h"

namespace paloLegacy {
class ECube;
}

namespace palo {

class PaloSession;
class RulesContext;

class PaloSharedMutex {
public:
	Mutex *getLock() {
		return &lock;
	}

private:
	Mutex lock;
};

class ContextStream : public std::ostringstream {
public:
	ContextStream() : std::ostringstream() {}
	ContextStream(const ContextStream&) : std::ostringstream() {}
private:
};

struct CellValueContext {
	enum JobType {
		CELL_REPLACE_JOB, CELL_REPLACE_BULK_JOB, CELL_GOAL_SEEK_JOB, CELL_COPY_JOB
	};

	struct PathAreaAndValue {
		PathAreaAndValue(const PCubeArea &cellPath, const CellValue &value, bool sepRight, CubeArea::CellType cellType) :
			cellPath(cellPath), value(value), sepRight(sepRight), cellType(cellType) {}

		PCubeArea cellPath;
		CellValue value;
		bool sepRight;
		CubeArea::CellType cellType;
	};

	struct PathVectorAndValue {
		PathVectorAndValue(IdentifiersType &cellPath, const CellValue &value, bool sepRight, CubeArea::CellType cellType) :
			cellPath(cellPath), value(value), sepRight(sepRight), cellType(cellType) {}

		IdentifiersType cellPath;
		CellValue value;
		bool sepRight;
		CubeArea::CellType cellType;
	};

	CellValueContext(PCube cube, JobType job, PDatabase db, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, int splashMode, PPaths lockedPaths, PCubeArea cellPathTo, bool useRules, double *ptrValue) :
		job(job), db(db), cube(cube), user(user), session(session), checkArea(checkArea), addValue(addValue), splashMode(splashMode), lockedCells(new LockedCells(db, cube, lockedPaths)), cellPathTo(cellPathTo), useRules(useRules), ptrValue(ptrValue), vElemTypes(0) {}

	void addPathAndValue(const PCubeArea &area, const CellValue &value, bool sepRight, CubeArea::CellType cellType) {
		pathAreaAndValue.push_back(PathAreaAndValue(area, value, sepRight, cellType));
	}

	void addPathAndValue(IdentifiersType &cellPath, const CellValue &value, bool sepRight, CubeArea::CellType cellType) {
		pathVectorAndValue.push_back(PathVectorAndValue(cellPath, value, sepRight, cellType));
	}

	enum GoalSeekType {
		GS_COMPLETE, GS_EQUAL, GS_RELATIVE
	};
	void addGoalSeekParams(PArea area, GoalSeekType type) {
		gsArea = area; gsType = type;
	}

	JobType job;

	//members used by CellReplaceJob and CellReplaceBulkJob
	PDatabase db;
	PCube cube;
	PUser user;
	boost::shared_ptr<PaloSession> session;
	bool checkArea;
	bool addValue;
	int splashMode;
	PLockedCells lockedCells;

	//additional members used by CellGoalSeekJob
	PArea gsArea;
	GoalSeekType gsType;

	//additional members used by CellCopyJob
	PCubeArea cellPathTo;
	bool useRules;
	double *ptrValue;

	//members used by CellReplaceBulkJob optimization
	vector<vector<char> > *vElemTypes;
	std::vector<PathVectorAndValue> pathVectorAndValue;

	std::vector<PathAreaAndValue> pathAreaAndValue;
};

struct ElementsContext {
	ElementsContext(PServer srv, PDatabase db, PDimension dim, IdentifiersType elements, PUser user, boost::shared_ptr<PaloSession> session, bool useDimWorker)
		: srv(srv), db(db), dim(dim), elements(elements), user(user), session(session), useDimensionWorker(useDimWorker) {}

	PServer srv;
	PDatabase db;
	PDimension dim;
	IdentifiersType elements;
	PUser user;
	boost::shared_ptr<PaloSession> session;
	bool useDimensionWorker;
};

struct MarkerListFilter {
	MarkerListFilter(IdentifierType dbId, IdentifierType cubeId) : dbId(dbId), cubeId(cubeId) {}
	IdentifierType	dbId;
	IdentifierType	cubeId;
	vector<const RuleMarker *> filter;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Thread local storage.
/// This is a thread local storage that exists for duration of job.
/// It's mainly used to keep user version of server (and hence the whole tree)
/// and parent child relations.
////////////////////////////////////////////////////////////////////////////////
class Context {
public:
	typedef pair<IdentifierType, pair<IdentifierType, IdentifierType> > RuleId; //dbID, cubeID, ruleID
	typedef set<RuleId> RuleIds;

	typedef set<ValueCache::CacheWriteProcessor *> CacheDependences;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Constructor
	/// Context shouldn't be constructed directly. See getContext()
	////////////////////////////////////////////////////////////////////////////////
	Context();
	virtual ~Context();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns server for reading.
	////////////////////////////////////////////////////////////////////////////////
	PServer getServer();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns copy of server for reading.
	////////////////////////////////////////////////////////////////////////////////
	PServer getServerCopy();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Stores parent child relationship.
	/// Used in lookup... functions to store parent child relationship like
	/// database/dimension, database/cube, etc.
	////////////////////////////////////////////////////////////////////////////////
	void saveParent(const CPCommitable& parent, const CPCommitable &child);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Gets parent of the specified object.
	/// Reads relationship that was stored via saveParent method
	////////////////////////////////////////////////////////////////////////////////
	CPCommitable getParent(const CPCommitable& child);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Clears the context
	/// Context is cleared at the end of the job or after unsuccessful merge.
	////////////////////////////////////////////////////////////////////////////////
	static void reset(bool wasNew = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions for workers support.
	////////////////////////////////////////////////////////////////////////////////
	static void setWorkersContext(Context *c) {
		c->setWorker(true);
		context.reset(c);
	}
	bool isWorker() {
		return worker;
	}
	void setWorker(bool w) {
		worker = w;
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions used by rule engine.
	////////////////////////////////////////////////////////////////////////////////
	void setEngineCube(paloLegacy::ECube* c);
	paloLegacy::ECube *getEngineCube(dbID_cubeID db_cube) const;
	void eraseEngineCube(dbID_cubeID db_cube);
	RulesContext *getRulesContext();
	void freeEngineCube();
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions used by journal.
	////////////////////////////////////////////////////////////////////////////////
	ostream &getJournalStream(PJournalFile journal);
	bool getJournalIsFirst(PJournalFile journal);
	void setJournalIsFirst(PJournalFile journal, bool isFirst);
	void deleteJournalStream(PJournalFile journal);
	void flushJournals();
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions for calculating markers.
	////////////////////////////////////////////////////////////////////////////////
	CubesWithDBs &getChangedMarkerCubes() {
		return changedMarkerCubes;
	}
	void addNewMarkerRule(IdentifierType db, IdentifierType cube, IdentifierType rule);
	void removeNewMarkerRuleJournal(IdentifierType db, IdentifierType cube, IdentifierType rule);
	void calcNewMarkerRules();
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions for storing active session.
	////////////////////////////////////////////////////////////////////////////////
	void setSession(boost::shared_ptr<PaloSession> session) {this->session = session;}
	boost::shared_ptr<PaloSession> getSession() {return session;}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Functions used by ElementDestroy and ElementDestroyBulk Jobs
	////////////////////////////////////////////////////////////////////////////////
	PElementsContext getElementsContext() {
		return elementsContext;
	}
	void setElementsContext(PElementsContext context) {
		elementsContext = context;
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Functions used by SvsRestart Job
	////////////////////////////////////////////////////////////////////////////////
	svsStatusChange getSvsChangeStatusContext() {
		return svsChangeStatusContext;
	}
	void setSvsChangeStatusContext(svsStatusChange value) {
		svsChangeStatusContext = value;
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Functions used by CellReplace(Bulk) job.
	////////////////////////////////////////////////////////////////////////////////
	PCellValueContext getCellValueContext() {
		return cellValue_context;
	}
	void setCellValueContext(PCellValueContext context) {
		cellValue_context = context;
	}

	bool makeCubeChanges(bool merge, PServer actual);
	void setPesimistic() {
		optimistic = false;
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Functions used for decisions if tokens should be updated.
	////////////////////////////////////////////////////////////////////////////////
	void setTokenUpdate(bool value) { //true - enable, false - disable token update
		updateToken = value;
	}
	bool doTokenUpdate() {
		return updateToken;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Functions used for decisions if user information should be refreshed.
	////////////////////////////////////////////////////////////////////////////////
	bool getRefreshUsers() {
		return refreshUsers;
	}
	void setRefreshUsers() {
		refreshUsers = true;
	}
	void addRenamedUser(IdentifierType id, string newName) {
		renamedUsers[id] = newName;
	}
	map<IdentifierType, string> getRenamedUsers() const {
		return renamedUsers;
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions for deleting database/cube files
	////////////////////////////////////////////////////////////////////////////////
	void deleteCubesFromDisk();
	void deleteDatabasesFromDisk();
	void addCubeToDelete(PCube cube);
	void addDatabaseToDelete(PDatabase db);
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Internal functions for faster Marker loading
	////////////////////////////////////////////////////////////////////////////////
	const MarkerListFilter *getMarkerListFilter() const {
		return filteredFromMarkers;
	};
	const MarkerListFilter *setMarkerListFilter(const MarkerListFilter *filter) {
		const MarkerListFilter *oldFilter = filteredFromMarkers;
		filteredFromMarkers = filter;
		return oldFilter;
	};
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name Cubes used in PALO.DATA rule functions
	////////////////////////////////////////////////////////////////////////////////
	int16_t savePaloDataCube(CPCube cube, CPDatabase db);
	bool getPaloDataCube(int16_t pos, CPCube &cube, CPDatabase &db);
	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Stop Job
	/// Stop Job Running within this context
	////////////////////////////////////////////////////////////////////////////////
	void stop(bool admin) {
		stopJob = admin ? ADMIN_STOP : LOGOUT_STOP;
	}

	void setIgnoreStop(bool st) {
		ignoreStopJob = st;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Check Stop signal
	/// check if Job Running within this context was signaled to stop and throw
	// the exception if so
	////////////////////////////////////////////////////////////////////////////////
	bool check(bool th = true) {
		if (!ignoreStopJob) {
			if (stopJob != NO_STOP) {
				if (th) {
					throw ErrorException(ErrorException::ERROR_STOPPED_BY_ADMIN, stopJob == ADMIN_STOP ? "job stopped by administrator" : "job stopped");
				}
				return false;
			}
			if (task && task->wasClosed()) {
				if (th) {
					throw ErrorException(ErrorException::ERROR_STOPPED_BY_ADMIN, "connection closed");
				}
				return false;
			}
		}
		return true;
	}

	void setSaveToCache(bool save) {
		saveToCache = save;
	}

	bool getSaveToCache() {
		return saveToCache;
	}


	////////////////////////////////////////////////////////////////////////////////
	/// @brief Returns context for current thread.
	////////////////////////////////////////////////////////////////////////////////
	static Context *getContext(bool *wasNew = 0, bool checkTermination = true);

	const set<EngineBase::Type> &getAvailableEngines() const {return availableEngines;}
	void disableEngine(EngineBase::Type engineType);

	boost::shared_ptr<ValueCache::QueryCache> getQueryCache(CPCube cube, size_t initSize, ValueCache &cache);
	boost::shared_ptr<ValueCache::QueryCache> getQueryCache(CPCube cube);
	void clearQueryCache() {queryCache.clear();}

	CacheDependences &getCacheDependences() {return cacheDependences;}
	void setCacheDependence(const dbID_cubeID &dbCubeId);

	void setInJournal(bool value) {
		inJournal = value;
	}

	bool getInJournal() const {
		return inJournal;
	}

	void setTask(IoTask *t) {
		task = t;
	}

private:
	enum JobStatus {
		NO_STOP, ADMIN_STOP, LOGOUT_STOP
	};

	pair<ContextStream, bool> &getJournal(PJournalFile journal);

	static boost::thread_specific_ptr<Context> context;
	std::map<CPCommitable, CPCommitable> relations;
	PServer	server;
	std::map<dbID_cubeID, paloLegacy::ECube*> engineCubes;
	std::map<PJournalFile, pair<ContextStream, bool> > journalstreams;
	PCellValueContext cellValue_context;
	PElementsContext elementsContext;
	svsStatusChange svsChangeStatusContext;
	CubesWithDBs changedMarkerCubes;
	bool updateToken;
	bool refreshUsers;
	map<IdentifierType, string> renamedUsers;
	std::list<PCube> cubesToDelete;
	std::list<PDatabase> dbsToDelete;
	RuleIds newMarkerRules;
	bool optimistic;
	bool worker;
	const MarkerListFilter *filteredFromMarkers;
	std::vector<pair<CPCube, CPDatabase> > paloDataSourceCubes;
	RulesContext *rulesContext;
	boost::shared_ptr<PaloSession> session;
	set<EngineBase::Type> availableEngines;
	map<const Cube *, boost::shared_ptr<ValueCache::QueryCache> > queryCache;
	JobStatus stopJob;
	bool ignoreStopJob;
	CacheDependences cacheDependences;
	bool inJournal;
	IoTask *task;
	bool saveToCache;
};

#define COMMITABLE_CAST(a, b) boost::dynamic_pointer_cast<a, Commitable>(b)
#define CONST_COMMITABLE_CAST(a, b) boost::dynamic_pointer_cast<const a, const Commitable>(b)

}

#endif /* CONTEXT_H_ */
