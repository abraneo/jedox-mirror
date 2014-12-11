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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_SERVER_H
#define OLAP_SERVER_H 1

#include "palo.h"

#include "Collections/StringUtils.h"

#include "InputOutput/FileUtils.h"

#include "Olap/Cube.h"
#include "Olap/Database.h"
#include "Olap/SystemDatabase.h"
#include "Olap/Context.h"
#include "Olap/PaloSession.h"
#include "PaloDispatcher/PaloJobRequest.h"
#include "Thread/ThreadPool.h"

namespace palo {
class Cube;
class Rule;
class User;
class ProgressCallback;
class LoginWorker;
class DimensionWorker;


class SERVER_CLASS LicIter {
public:
	virtual ~LicIter() {};
	virtual string getKey();
	virtual string getCustomer();
	virtual string getFeatures();
	virtual double getLicenseCount();
	virtual double getNamedCount();
	virtual string getConcurrentSessions();
	virtual double getGPUCount();
	virtual double getFreeConcurrent();
	virtual double getFreeNamed();
	virtual int64_t getActivationTime();
	virtual int64_t getExpirationTime();
	virtual bool next();
	virtual string validateLics(string value);
};
typedef boost::shared_ptr<LicIter> PLicIter;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP server
///
/// An OALP server consists of OLAP databases.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Server : public Commitable {
	friend class Context;
public:
	static const string NAME_SYSTEM_DATABASE;
	static const string NAME_CONFIG_DATABASE;
	static const string VALID_DATABASE_CHARACTERS;
	static bool ignoreJournal;
public:
	static const char *getRevision();
	static const char *getVersion();
	static string getVersionRevision();
	static string getVersionRevisionDots();
	bool isBlocking() const {
		return blocking;
	}

	void setBlocking(bool blocking) {
		checkCheckedOut();
		this->blocking = blocking;
	}

	string getActiveSession() const {
		return activeSessionSid;
	}

	void setActiveSession(const string &session) {
		checkCheckedOut();
		this->activeSessionSid = session;
	}

	const string & getEvent() const {
		return event;
	}

	void setEvent(const string & event) {
		checkCheckedOut();
		this->event = event;
	}

	const string & getUsername(PUser user) const {
		static const string system = "#SYSTEM#";
		if (blocking) {
			return username;
		} else if (user == 0) {
			return system;
		} else {
			return user->getName();
		}

	}

	const string & getRealUsername() const {
		return username;
	}

	void setUsername(const string & username) {
		checkCheckedOut();
		this->username = username;
	}

	void addChangedMarkerCube(CPCube cube, bool enforceBuildMarkers);
	void triggerMarkerCalculation();
	void setMarkerCalculationState(bool state);

	WorkerLoginType getLoginType() const {
		return loginType;
	}

	void setLoginType(WorkerLoginType value) {
		checkCheckedOut();
		loginType = value;
	}

	static bool flightRecorderEnabled() {
		return Logger::warning.isActive();
	}

	bool winAuthEnabled() const {
		return winSSOenabled;
	}

	void setWinAuth(bool value) {
		checkCheckedOut();
		winSSOenabled = value;
	}

	void setDrillThroughEnabled(bool value) {
		checkCheckedOut();
		drillThroughEnabled = value;
	}

	bool isDrillThroughEnabled() const {
		return drillThroughEnabled;
	}

	time_t getDefaultTtl() const {
		return defaultTtl;
	}

	void setDefaultTtl(time_t value) {
		checkCheckedOut();
		defaultTtl = value;
	}

	Encryption_e getEncryptionType() const {
		return encryption;
	}

	void setEncryptionType(Encryption_e value) {
		checkCheckedOut();
		encryption = value;
	}

	bool useShortSid() const {
		return shortSid;
	}

	void setShortSid(bool value) {
		checkCheckedOut();
		shortSid = value;
	}

	bool isEnableGpu() const {
		return enableGpu;
	}

	void setEnableGpu(bool value) {
		enableGpu = value;
	}

	virtual bool activateGpuEngine(vector<string> gpuDeviceIdsOptions);
	virtual bool optimizeGpuEngine();
	virtual bool needGpuOptimization();

	static PServer getInstance(bool write);
	static void create(const FileName & fileName);
	static void destroy() {writersserver.reset(); readersserver.reset();}
	static Mutex &getSaveLock() {return writerslock;}
	virtual ~Server();
public:
	bool isLoadable() const {
		return FileUtils::isReadable(fileName);
	}

	void loadServer(PUser user);
	void saveServer(PUser user, bool complete);
public:
	uint32_t getToken() const {
		return token;
	}

	uint32_t getDataToken() const {
		return dataToken;
	}

	PSystemDatabase getSystemDatabase() {
		return systemDatabase;
	}

	const PSystemDatabase getSystemDatabase() const {
		return systemDatabase;
	}

	PSystemDatabase getSystemDatabaseCopy() {
		checkCheckedOut();
		dbs = getDatabaseList(true);
		systemDatabase = COMMITABLE_CAST(SystemDatabase, lookupDatabase(systemDatabase->getId(), true));
		dbs->set(systemDatabase);
		return systemDatabase;
	}

	IdentifierType sizeDatabases() const {
		return dbs->size();
	}

	;
	vector<CPDatabase> getDatabases(PUser user) const;
	PDatabaseList getDatabaseList(bool write) const {
		return write  && !dbs->isCheckedOut() ? COMMITABLE_CAST(DatabaseList, dbs->copy()) : dbs;
	}

	void setDatabaseList(PDatabaseList l);

	void setLoginWorker(PLoginWorker worker) {
		checkCheckedOut();
		loginWorker = worker;
	}

	void setDimensionWorker(PDimensionWorker worker) {
		checkCheckedOut();
		if (worker) {
			dimensionWorkerConfigured = true;
		}
		dimensionWorker = worker;
	}

	bool svsConfigured() const;
	bool svsIsStopped() const;
	void setSvsStopped(bool state);
	PLoginWorker getLoginWorker() const;
	PDimensionWorker getDimensionWorker() const;
	bool isDimensionWorkerConfigured() const;

	void svsRestart(vector<PCube> &cubes, svsStatusChange status);

	PEngineList getEngineList(bool write) {
		if (write) {
			checkCheckedOut();
			if (!engines->isCheckedOut()) {
				engines = COMMITABLE_CAST(EngineList, engines->copy());
			}
		}
		return engines;
	}

	PEngineList getEngineListCopy() const {
		return !engines->isCheckedOut() ? COMMITABLE_CAST(EngineList, engines->copy()) : engines;
	}

	void setEngineList(PEngineList l) {
		checkCheckedOut();
		engines = l;
	}

	PEngineBase getEngine(EngineBase::Type type = EngineBase::CPU, bool write = false) { //get Engine (main CPU engine for reading by default)
		PCommitable engine = getEngineList(write)->get((IdentifierType)type, write);
		if (write) {
			engines->set(engine);
		}
		return COMMITABLE_CAST(EngineBase, engine);
	}

	void removeEngine(PEngineBase engine){
		PEngineList el = getEngineList(true);
		el->remove(engine->getId());
	}

	void removeStorages(IdentifiersType storages);

	static void setCrossOrigin(string crossOrigin) {Server::crossOrigin = crossOrigin;}
	static string &getCrossOrigin() {return crossOrigin;}

public:
	void beginShutdown(PUser);
	void changePassword(PUser userChanging, IdentifierType userToChange, const string& new_password);
	void addDatabase(PDatabase, bool notify, bool newid, PUser user, bool useDimWorker);
	void removeDatabase(PDatabase, bool notify, bool useDimWorker);
	void renameDatabase(PDatabase, const string & name, bool notify, bool useDimWorker);
	PDatabase addDatabase(const string & name, PUser user, IdentifierType type, bool useDimWorker);
	void deleteDatabase(PDatabase database, PUser user, bool useDimWorker);
	void renameDatabase(PDatabase database, const string & name, PUser user, bool useDimWorker);
	void loadDatabase(PDatabase database, PUser user);
	PDatabase restoreDatabase(const string &zipFileName, PUser user, string *dbName);
	void saveDatabase(PDatabase database, PUser user, bool sendEvent, string *backupZipPath, bool complete);
	void unloadDatabase(PDatabase database, PUser user);
	PDatabase lookupDatabase(IdentifierType identifier, bool write) const {
		return COMMITABLE_CAST(Database, dbs->get(identifier, write));
	}

	PDatabase lookupDatabaseByName(const string & name, bool write) const {
		return COMMITABLE_CAST(Database, dbs->get(name, write));
	}

	PDatabase findDatabase(IdentifierType identifier, PUser user, bool requireLoad, bool write) const {
		checkDatabaseAccessRight(user, RIGHT_READ);
		PDatabase database = lookupDatabase(identifier, write);
		if (database) {
			User::checkDatabaseDataRight(user, database->getId(), RIGHT_READ);
		} else {
			throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", "database identifier", (int)(identifier));
		}
		if (requireLoad && database->getStatus() == Database::UNLOADED) {
			throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "database not loaded", "database identifier", (int)(identifier));
		}
		if (database->getType() == SYSTEMTYPE && User::checkUser(user)) {
			user->checkRoleRight(User::rightsRight, RIGHT_READ);
		}
		return database;
	}

	PDatabase findDatabaseByName(const string & name, PUser user, bool requireLoad, bool write) const {
		checkDatabaseAccessRight(user, RIGHT_READ);
		PDatabase database = lookupDatabaseByName(name, write);
		if (database) {
			User::checkDatabaseDataRight(user, database->getId(), RIGHT_READ);
		} else {
			throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", "name", name);
		}
		if (requireLoad && database->getStatus() == Database::UNLOADED) {
			throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "database not loaded", "name", name);
		}
		if (database->getType() == SYSTEMTYPE && User::checkUser(user)) {
			user->checkRoleRight(User::rightsRight, RIGHT_READ);
		}
		return database;
	}

	void commitAndSave();

	void addSystemDatabase();
	void addConfigDatabase();

	void ShutdownLoginWorker();
	void ShutdownDimensionWorker();
	void ShutdownGpuEngine(PUser user);
	void invalidateCache(IdentifierType dbId = NO_IDENTIFIER, IdentifierType cubeId = NO_IDENTIFIER);
	void updateDatabaseDim(bool useDimWorker);
	void updateDatabaseDim(SystemDatabase::UpdateType type, const string &dbName, const string &dbOldName, PUser user, bool useDimWorker);
	void checkOldCubes();

	bool isBigEndian() const {return bigEndian;}
	bool enforceBuildMarkers() const {return buildMarkers;}

	virtual bool commit();
	virtual bool merge(const CPCommitable & o, const PCommitable & p);
	virtual PCommitable copy() const;

	virtual string getHWKey();
	virtual string getLicenses();
	virtual void activateLicense(string lickey, string actcode);
	virtual bool deleteLicense(const set<string> &code);
	virtual void findFree(set<string> &code);
	virtual unsigned int getNumberofGPUs();
	virtual void reserveLicense(boost::shared_ptr<PaloSession> session, const string *machine, const string *required, string *optional);
	virtual void freeLicense(boost::shared_ptr<PaloSession> session);
	virtual void moveLicense(IdentifierType id, IdentifierType position);
	void updateLicenses(PSystemDatabase sysdb);
	virtual void checkNamedLicense(IdentifierType user, string lic);
	virtual string getSessionLicense(boost::shared_ptr<PaloSession> session);
	void updateGpuBins(PDimension dimension, PServer server, PaloJobRequest* jobRequest, PDatabase database, Context* context, PUser user);
	virtual PLicIter getLicenseIterator(bool find);
	virtual void afterLoad();
	virtual vector<CPDatabase> getAdvanced(PUser user);
	virtual void removeAdvanced(IdentifierType id);
	virtual void resetFirst();
	static string getUserLicense(IdentifierType user);
	static void setUserLicense(IdentifierType user, string lickey);

	PThreadPool getThreadPool() const
	{
		return tp;
	}

	void databaseRenamed(const string &oldName, const string &newName);
	void physicalRenames();
	static void physicalRenamesNoLock(FileName &fileName);

	static void setDefaultDbRight(string right);
	static RightsType getDefaultDbRight();

protected:
	Server(const FileName & fileName);
	Server(const Server & s);
private:
	static FileName computeDatabaseFileName(const FileName & fileName, const string & name);
	IdentifierType loadServerOverview(FileReader *file);
	IdentifierType loadServerDatabase(FileReader *file);
	void loadServerDatabases(FileReader *file);
	void saveServerOverview(FileWriter *file);
	void saveServerDatabases(FileWriter *file);
	void addCubeToList(PDatabase database, PCube cube);
	void saveDatabaseCubes(PDatabase database);

	void checkDatabaseAccessRight(PUser user, RightsType minimumRight) const {
		if (User::checkUser(user)) user->checkRoleRight(User::databaseRight, minimumRight);
	}

	void checkSystemOperationRight(PUser user, RightsType minimumRight) const {
		if (User::checkUser(user)) user->checkRoleRight(User::sysOpRight, minimumRight);
	}
	void addSystemDatabaseIntern(bool sys);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get vector with all usable gpu device (Compute Capability >= 1.3)
	////////////////////////////////////////////////////////////////////////////////
	map<string, int32_t> getGpuDeviceIds();
	map<string, int32_t> getGpuDeviceIds(vector<string> gpuDeviceIdsOptions);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief check gpu devices ids from config file palo.ini and return gpu ordinals
	////////////////////////////////////////////////////////////////////////////////
	vector<int32_t> getGpuDeviceOrdinals(map<string, int32_t> &gpuDeviceIdsSystem, size_t numGpusLicense);
	vector<int32_t> getGpuDeviceOrdinals(map<string, int32_t> &gpuDeviceIdsSystem, vector<string> gpuDeviceIdsOptions, size_t numGpusLicense);

private:
	uint32_t token;
	uint32_t dataToken;
protected:
	PDatabaseList dbs;
private:
	FileName fileName;
	PSystemDatabase systemDatabase;
	bool svsStopped;
	PLoginWorker loginWorker;
	PDimensionWorker dimensionWorker;
	bool dimensionWorkerConfigured;
	static PServer readersserver;
	static PServer writersserver;
	static Mutex readerslock;
	static Mutex writerslock;

	PSharedMutex filelock;

	bool blocking;
	string activeSessionSid;
	string username;
	string event;
	WorkerLoginType loginType;
	bool winSSOenabled;
	time_t defaultTtl;
	bool drillThroughEnabled;
	bool m_DisableMarkerCalculation;
	Encryption_e encryption;
	bool shortSid;
protected:
	bool enableGpu;
	PEngineList engines;	// Cpu (primary), Gpu, + other engines
private:
	bool bigEndian;
	bool buildMarkers;
	PThreadPool tp;
	static RightsType defaultDbRight;
	static string crossOrigin;
};

ostream& operator<<(ostream& ostr, const vector<CPDatabase> &vdb);

}

#endif
