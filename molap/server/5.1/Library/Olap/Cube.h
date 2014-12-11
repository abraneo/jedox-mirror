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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_CUBE_H
#define OLAP_CUBE_H 1

#include "palo.h"

#include "InputOutput/JournalFileReader.h"
#include "InputOutput/JournalFileWriter.h"
#include "Logger/Logger.h"
#include "Olap/Dimension.h"
#include "Olap/Element.h"
#include "Worker/CubeWorker.h"
#include "Olap/RulesList.h"
#include "Olap/Context.h"
#include "Engine/EngineBase.h"
#include "Engine/Streams.h"
#include "Engine/Cache.h"

namespace palo {
class PaloSession;

class SERVER_CLASS CubeList : public CommitableList {
public:
	CubeList(const PIdHolder &newidh) : CommitableList(newidh) {}
	CubeList() {}
	CubeList(const CubeList &l);
	virtual PCommitableList createnew(const CommitableList& l) const;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP cube
///
/// An OLAP cube stores the data
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Cube : public Commitable {

public:
	static double splashLimit1; // error
	static double splashLimit2; // warning
	static double splashLimit3; // info

	static int goalseekTimeoutMiliSec;
	static int goalseekCellLimit;
	static bool saveCSV;

	static const string PREFIX_ATTRIBUTE_CUBE;

	static const string NUMERIC_SECTION;
	static const string STRING_SECTION;
	static const string ALIAS_SECTION;
	static const string GROUP_SECTION;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief status of the cube
	///
	/// UNLOADED: the cube was not loaded<br>
	/// LOADED:   the cube is loaded and not changed<br>
	/// CHANGED:  the cube is new or changed
	////////////////////////////////////////////////////////////////////////////////

	enum CubeStatus {
		UNLOADED, LOADED, CHANGED
	};

	enum SaveType {
		RIGHTS = 1, NORMAL, ATTRIBUTES, CONFIGURATION, SUBSETVIEW, USERINFO, GPU, LOG, SESSIONS, JOBS, LICENSES
	};

	typedef uint32_t CellLockInfo;

	typedef map<IdentifierType, IdentifierType> Id2IdMap;

	struct ltMarker {
		bool operator()(const PRuleMarker &m1, const PRuleMarker &m2) const;
	};
	typedef set<PRuleMarker, ltMarker> RuleMarkerSet;

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets flag for ignoring cell data (for debugging only)
	////////////////////////////////////////////////////////////////////////////////

	static void setIgnoreCellData(bool ignore) {
		ignoreCellData = ignore;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a new cube from type line
	////////////////////////////////////////////////////////////////////////////////

	static PCube loadCubeFromType(FileReader*, PDatabase, IdentifierType, const string& name, const IdentifiersType *dimensions, uint32_t type);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief set the number of base elements for caching consolidated values
	////////////////////////////////////////////////////////////////////////////////

	static void setCacheBarrier(double barrier);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get the number of base elements for caching consolidated values
	////////////////////////////////////////////////////////////////////////////////

	static double getCacheBarrier();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets timeout for goalseek operations
	////////////////////////////////////////////////////////////////////////////////
	static void setGoalseekTimeout(int milisecs);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets cell limit for goalseek operations
	////////////////////////////////////////////////////////////////////////////////
	static void setGoalseekCellLimit(int cellCount);

	static void setSaveCSV(bool save);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Creates empty cube
	////////////////////////////////////////////////////////////////////////////////

	Cube(PDatabase db, const string& name, const IdentifiersType *dimensions, SaveType saveType);
	Cube(const Cube& c);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Cube();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name notification callbacks
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been added to a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyAddCube(PServer server, PDatabase database, IdentifierType *newCubeDimElem, IdentifierType *newCellRightCube, IdentifierType *newCellPropsCube, bool useDimWorker);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been removed from a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRemoveCube(PServer server, PDatabase database, bool useDimWorker);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been renamed
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRenameCube(PServer server, PDatabase database, const string& oldName, bool useDimWorker);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions to save and load the dimension
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if cube is loadable
	////////////////////////////////////////////////////////////////////////////////

	bool isLoadable() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if cube can have worker
	////////////////////////////////////////////////////////////////////////////////

	virtual bool supportsWorker() const {return true;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if cube can aggregate numeric cells
	////////////////////////////////////////////////////////////////////////////////

	virtual bool supportsAggregations() const {return true;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief changes cube path
	////////////////////////////////////////////////////////////////////////////////

	virtual void setCubeFileAndLoad(PServer server, PDatabase db, const FileName& fileName, bool newid);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief reads data from file
	////////////////////////////////////////////////////////////////////////////////

	virtual void loadCube(PServer server, PDatabase db, bool processJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief reads journal data from file
	////////////////////////////////////////////////////////////////////////////////

	void processCubeJournal(PServer server, PDatabase db);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief saves cube name and type to file
	////////////////////////////////////////////////////////////////////////////////

	void saveCubeType(FileWriter* file);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief saves data to file
	////////////////////////////////////////////////////////////////////////////////

	virtual void saveCube(PServer server, PDatabase db);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes cube file from disk
	////////////////////////////////////////////////////////////////////////////////

	void deleteCubeFiles();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief unloads saved cubes from memory
	////////////////////////////////////////////////////////////////////////////////

	virtual void unloadCube(PServer server, PDatabase db);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name getter and setter
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns cube type
	////////////////////////////////////////////////////////////////////////////////

	ItemType getType() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns cube subtype
	////////////////////////////////////////////////////////////////////////////////

	SaveType getCubeType() const {return saveType;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets cube type
	////////////////////////////////////////////////////////////////////////////////

	void setType(ItemType type);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the token
	////////////////////////////////////////////////////////////////////////////////

	uint32_t getToken() const;

    ////////////////////////////////////////////////////////////////////////////////
    /// @brief gets the cube's token
    ////////////////////////////////////////////////////////////////////////////////

    uint32_t getMyToken() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the client cache token
	////////////////////////////////////////////////////////////////////////////////

	uint32_t getClientCacheToken() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets deletable attribute
	////////////////////////////////////////////////////////////////////////////////

	void setDeletable(bool deletable) {
		this->deletable = deletable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets deletable attribute
	////////////////////////////////////////////////////////////////////////////////

	bool isDeletable() const {
		return deletable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets renamable attribute
	////////////////////////////////////////////////////////////////////////////////

	void setRenamable(bool renamable) {
		this->renamable = renamable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets renamable attribute
	////////////////////////////////////////////////////////////////////////////////

	bool isRenamable() const {
		return renamable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets cube dimension list
	////////////////////////////////////////////////////////////////////////////////

	const IdentifiersType *getDimensions() const {
		return &dimensions;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets sizes of cube dimensions
	////////////////////////////////////////////////////////////////////////////////

	IdentifiersType getDimensionSizes(PDatabase db) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets PathTranslator instance of the cube
	////////////////////////////////////////////////////////////////////////////////

	PPathTranslator getPathTranslator() const {
		return pathTranslator;
	}

    ////////////////////////////////////////////////////////////////////////////////
    // @updates PathTranslator instance of the cube
    ////////////////////////////////////////////////////////////////////////////////
    
    void updatePathTranslator(PDatabase db);

    void updatePathTranslator(PPathTranslator pt);


	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets rule list
	////////////////////////////////////////////////////////////////////////////////

	vector<PRule> getRules(PUser, bool) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets cube status
	////////////////////////////////////////////////////////////////////////////////

	CubeStatus getStatus() const {
		return max(cellsStatus, rulesStatus);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets cube status variables
	////////////////////////////////////////////////////////////////////////////////

	void setStatus(CubeStatus status) {
		cellsStatus = status;
		rulesStatus = status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the number of filled cells
	////////////////////////////////////////////////////////////////////////////////

	size_t sizeFilledCells() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the number of filled numeric cells
	////////////////////////////////////////////////////////////////////////////////

	virtual size_t sizeFilledNumericCells() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the number of filled string cells
	////////////////////////////////////////////////////////////////////////////////

	size_t sizeFilledStringCells() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the number of filled marker cells
	////////////////////////////////////////////////////////////////////////////////

	size_t sizeFilledMarkerCells() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions to update internal structures
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a new rule
	////////////////////////////////////////////////////////////////////////////////

	virtual PRule createRule(PServer server, PDatabase db, PRuleNode, const string& definition, const string& external, const string& comment, bool activate, PUser, bool useJournal, IdentifierType *id, double position);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief modifies an existing rule
	////////////////////////////////////////////////////////////////////////////////

	virtual bool modifyRule(PServer server, PDatabase db, PRule, PRuleNode, const string& definition, const string& external, const string& comment, PUser, ActivationType activation, bool useJournal, double position);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief activate/deactivate an existing rule
	////////////////////////////////////////////////////////////////////////////////

	virtual bool activateRules(PServer server, PDatabase db, const vector<PRule> &, ActivationType activation, PUser, string* errMsg, bool bDefinitionChangedBefore, bool useJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief set position (priority) of the rule
	////////////////////////////////////////////////////////////////////////////////

	virtual bool setRulesPosition(PServer server, PDatabase db, const vector<PRule> rules, double startPosition, double belowPosition, PUser user, bool useJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes a rule
	////////////////////////////////////////////////////////////////////////////////

	virtual void deleteRule(PServer server, PDatabase db, IdentifierType id, PUser user, bool useJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds a rule
	////////////////////////////////////////////////////////////////////////////////

	PRule findRule(IdentifierType) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief increments the client cache token
	////////////////////////////////////////////////////////////////////////////////

	void updateClientCacheToken();

	void disableTokenUpdate();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes element
	////////////////////////////////////////////////////////////////////////////////

	virtual bool deleteElement(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifierType element, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, bool completeRemove);
	virtual bool deleteElements(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifiersType elements, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, PSet fullSet, bool completeRemove);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief set the worker area of the cube
	////////////////////////////////////////////////////////////////////////////////

	void setWorkerAreas(const vector<string> &areaIdentifiers, const vector<PArea> &areas);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief remove unused worker
	////////////////////////////////////////////////////////////////////////////////

	void removeWorker();

	PLock lockCube(PArea area, const string& areaString, bool whole, PUser user);

	void commitCube(long int id, PUser user);

	void rollbackCube(PServer server, PDatabase db, long int id, PUser user, size_t numSteps);

	CPLockList getCubeLocks(PUser user) const;

	bool hasLockedArea() const {
		return hasLock;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions dealing with cells and cell values
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clears all cells
	////////////////////////////////////////////////////////////////////////////////

	virtual void clearCells(PServer server, PDatabase db, PUser user, bool useJournal);

	virtual void clearCells(PServer server, PDatabase db, PCubeArea areaElements, PUser user, bool useJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a value to a cell
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct);
	void commitChanges(bool checkLocks, PUser user, set<PCube> &changedCubes, bool disjunctive)
	{
		commitChangesIntern(checkLocks, user, disjunctive);
		for (set<PCube>::iterator it = changedCubes.begin(); it != changedCubes.end(); ++it) {
			if ((*it).get() != this) {
				(*it)->commitChangesIntern(false, PUser(), false);
			}
		}
		changedCubes.clear();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a value to a cell
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus setCellValue(bool isBulk);

	bool hasRule() const {
		return rules->size() > 0;
	}

	bool hasActiveRule() const {
		return rules->hasActiveRule();
	}

	virtual CellValue getCellValue(PCubeArea cellPath, bool bUseRules) const;
	virtual CellValue getCellValue(CPArea cellPath) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies a cell value (or cell values) to an other cell
	////////////////////////////////////////////////////////////////////////////////

	void copyCellCreateMaps(PDatabase db, Element *sourceElem, Element *targetElem, CPDimension dim, PSetMultimap &setMultimap,
			Set &sourceSet, Set &targetSet, set<IdentifierType> &targetForbiddenElems, Id2IdMap &target2SourceMap, bool &simpleCase);
	bool copyCellValuesPrepare(PServer server, PDatabase db, PCubeArea cellPathFrom, PCubeArea cellPathTo, PLockedCells lockedCells, PUser user, CubeArea &areaSource,
			CubeArea &areaTarget, SetMultimaps &setMultimaps, double &factorToReturn, double *dValue, double *factorFromJournal, bool &simpleCase, bool bUseRules);
	virtual bool copyCells(PServer server, PDatabase db, PCubeArea cellPathFrom, PCubeArea cellPathTo, PLockedCells lockedCells, PUser user, bool bUseRules, double *dValue, double *factorFromJournal, bool useJournal);

	virtual bool copyCells();

	virtual PPlanNode createPlan(PCubeArea area, CubeArea::CellType type, RulesType paramRulesType, bool skipEmpty, uint64_t blockSize) const;

	PCellStream calculateArea(PCubeArea area, CubeArea::CellType type, RulesType paramRulesType, bool skipEmpty, uint64_t blockSize) const;

	virtual PProcessorBase evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const;

	virtual bool invalidateCache();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief disable all dim's rules when dim is cleared
	////////////////////////////////////////////////////////////////////////////////

	virtual void disableRules(PServer server, PDatabase db, CPDimension dim);


	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets values from the cube
	///
	/// cell goal seek
	///
	///
	////////////////////////////////////////////////////////////////////////////////

	void cellGoalSeek();

protected:
	void cellGoalSeek(PServer server, PDatabase db, CellValueContext::GoalSeekType gsType, PCubeArea cellPath, PArea gsArea, PUser user, boost::shared_ptr<PaloSession> session, const double &value, bool useJournal);
	void cellGoalSeekEqualRelative(PServer server, PDatabase db, PCubeArea cellPath, PUser user, boost::shared_ptr<PaloSession> session, double value, vector<IdentifiersWeightMap> &siblings, bool equal, User::RightSetting& rs);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:
	void executeShutdown();

	static bool isInArea(const Area *cellPath, const Area *area);

	static bool isInArea(const IdentifierType *cellPath, const Area *area);

	static PCube addCubeToDatabase(PServer server, dbID_cubeID db_cube);

	static bool isSameCube(PCube lcube, PCube rcube, IdentifierType ldb, IdentifierType rdb);

	void removeFromMarker(PRuleMarker marker);

	void removeToMarker(PServer server, PRuleMarker marker);

	void addFromMarker(PRuleMarker marker, set<PCube> *changedCubes);

	void addToMarker(PRuleMarker marker);

	bool hitMarkerRebuildLimit();

	void clearAllMarkers();

	void rebuildAllMarkers();

	const RuleMarkerSet& getFromMarkers() const;

	const RuleMarkerSet& getToMarkers() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief check the user access rights for area
	////////////////////////////////////////////////////////////////////////////////

	// throws exception, checks everything
	virtual void checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get the minimum access right for the cube
	////////////////////////////////////////////////////////////////////////////////

	virtual RightsType getMinimumAccessRight(CPUser user) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks the cube access right
	////////////////////////////////////////////////////////////////////////////////

	virtual void checkCubeAccessRight(PUser user, RightsType minimumRight, bool checkGroupCubeData, bool checkCubeRightObject) const;
	virtual RightsType getCubeAccessRight(CPUser user) const;

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;
	virtual bool checkBeforeInsertToMergedList(const CPCommitable &parent);

	bool isLocked() const {
		return wholeCubeLocked;
	}

public:

	enum InBulkEnum { Not, First, In };
	boost::shared_ptr<JournalFileReader> initJournalProcess();
	void processJournalCommand(PServer server, PDatabase db, CPCube thisCube, JournalFileReader &journalReader, InBulkEnum &replaceBulkState, set<PCube> &changedCubes);

protected:

	bool loadCubeCells(FileReader *fr, PDatabase db, bool checkAlias, bool binary, bool &diffAlias, uint32_t fileversion);

	//load of bin file
	bool loadCubeOverview(FileReader *file, timeval &tv, uint32_t &file_version, uint32_t &endianness);
	//load of csv file
	void loadCubeOverview(FileReader *file, timeval &tv);

	void loadCubeRuleInfo(FileReader*);

	void loadCubeRule(PServer server, PDatabase db, FileReader*, int version);

	void loadCubeRules(PServer server, PDatabase db);

	void saveCubeOverview(FileWriter *file, PServer server, PDatabase db, timeval &tv, bool binary);

	void saveCubeCells(FileWriter *file, PServer server, PDatabase db, bool checkAlias, bool binary);

	void saveCubeRule(FileWriter* file, CPRule rule, CPDatabase db);

	void saveCubeRules(CPDatabase db);

	void loadCubeIntern(PServer server, PDatabase db, bool processJournal, bool saveCells, bool checkAlias);
	void saveCubeIntern(PServer server, PDatabase db, bool saveCells, bool checkAlias);

private:
	static const uint32_t CUBE_FILE_VERSION;
	static const string CSV;
	static const string BIN;
	static const string CSVTMP;
	static const string BINTMP;
	static const int littleEndian;
	static const int bigEndian;
	static const uint32_t maxNewMarkerCount;
	static const uint32_t markerRebuildLimit;

	void loadFromFile(PServer server, PDatabase db, bool loadCells, bool checkAlias, bool binary);
	void saveToFile(PServer server, PDatabase db, bool saveCells, bool checkAlias, timeval &tv, bool binary);
	bool isInArea(const Area *cellPath, string &areaIdentifier) const;
	PLock lookupLockedArea(const IdentifiersType &key, PUser user);
	void setCellMarker(EngineBase *engine, MarkerStorageCpu *storage, const IdentifiersType &key, set<PCube> &changedCubes);
	void checkFromMarkers(EngineBase *engine, CPArea area, set<PCube> &changedCubes);
	void checkFromMarkers(EngineBase *engine, const IdentifiersType &key, set<PCube> &changedCubes);
	bool checkMarkerInvalidation(const Area *area, StorageBase *storageCpu) const;
	bool checkMarkerInvalidation(SubCubeList &areas) const;
	PLock findCellLock(const IdentifiersType &key) const;
	bool checkDimensions(CPDatabase db);
	void checkValueLocks(PCellStream oldvals, PUser user, StorageBase *storage);
	void commitChangesIntern(bool checkLocks, PUser user, bool disjunctive);

public:
	void checkCubeRuleRight(PUser user, RightsType minimumRight) const;
	CellLockInfo getCellLockInfo(const IdentifiersType &key, IdentifierType userId) const;

	PCubeWorker getCubeWorker() const;
	PCubeWorker createCubeWorker();

	PRuleList getRuleList(bool write) const {
		return write && !rules->isCheckedOut() ? COMMITABLE_CAST(RuleList, rules->copy()) : rules;
	}

	void setRuleList(PRuleList l) {
		checkCheckedOut();
		rules = l;
	}

	IdentifierType getNumericStorageId() const {
		return numericStorageId;
	}

	IdentifierType getStringStorageId() const {
		return stringStorageId;
	}

	IdentifierType getMarkerStorageId() const {
		return markerStorageId;
	}

	ValueCache *getCache() const {return const_cast<ValueCache *>(&cache);}

#ifdef ENABLE_GPU_SERVER
	bool optimizeNumericStorage(PEngineBase engine);
#endif

protected:
	uint32_t token; // token for changes
	PIdHolder clientCacheToken; // token for client cache changes

	IdentifiersType dimensions; // list of dimensions used for the cube

	PRuleList rules;

	bool deletable;
	bool renamable;

	PCubeWorker cubeWorker;
	bool hasArea;
	vector<string> workerAreaIdentifiers;
	vector<PArea> workerAreas;

	IdentifierType numericStorageId;	// assigned numeric storage identifier (unique per server runtime)
	IdentifierType stringStorageId;		// assigned string storage identifier (unique per server runtime)
	IdentifierType markerStorageId;		// assigned marker storage identifier (unique per server runtime)

	CubeStatus cellsStatus; // the status of the cube cells
	CubeStatus rulesStatus; // the status of the rules

	PFileName fileName; // file name of the cube
	PFileName ruleFileName; // file of the rules

	PJournalFile journalFile;
	PJournalMem journal;

	static double cacheBarrier;

	static bool ignoreCellData;

	bool hasLock;
	PLockList locks;

	RuleMarkerSet fromMarkers;
	RuleMarkerSet toMarkers;

	PSharedMutex filelock;
	PSharedMutex rulefilelock;

	SaveType saveType;

	bool wholeCubeLocked;

	PPathTranslator pathTranslator;

	ValueCache cache;

	AsyncResults pendingWrites;
	bool additiveCommit;

	uint64_t delCount;
};

}

#endif
