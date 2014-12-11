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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_DATABASE_H
#define OLAP_DATABASE_H 1

#include "palo.h"

#include <unordered_map>

#include "Exceptions/ParameterException.h"
#include "InputOutput/JournalFileWriter.h"

#include "Olap/CommitableList.h"
#include "Olap/Cube.h"
#include "Olap/ConfigurationCube.h"
#include "Olap/Dimension.h"
#include "Olap/User.h"
#include "Olap/Context.h"
#include "Olap/Lock.h"

namespace palo {

class SERVER_CLASS DatabaseList : public CommitableList {
public:
	DatabaseList(const PIdHolder &newidh) :
			CommitableList(newidh)
	{
	}
	DatabaseList()
	{
	}
	DatabaseList(const DatabaseList& l);

private:
	virtual PCommitableList createnew(const CommitableList& l) const;
};

class ElementOld2NewMap : private unordered_map<IdentifierType, IdentifierType> {
public:
	IdentifierType translate(IdentifierType oldId)
	{
		IdentifierType result = NO_IDENTIFIER;
		unordered_map<IdentifierType, IdentifierType>::iterator trit = find(oldId);
		if (trit != end()) {
			result = trit->second;
		}
		return result;
	}
	void setTranslation(IdentifierType oldId, IdentifierType newId)
	{
		operator[](oldId) = newId;
	}
	size_t size()
	{
		return unordered_map < IdentifierType, IdentifierType > ::size();
	}
	void clear()
	{
		unordered_map < IdentifierType, IdentifierType > ::clear();
	}
};

typedef unordered_map<IdentifierType, ElementOld2NewMap> DimensionOld2NewMap;
typedef boost::shared_ptr<DimensionOld2NewMap> PDimensionOld2NewMap;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP database
///
/// An OLAP database consists of dimensions and cubes
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Database : public Commitable {
	friend class Server;
private:
	static const string INVALID_CHARACTERS;
public:
	enum DatabaseStatus {
		UNLOADED, LOADED, CHANGED, LOADING
	};
	static PDatabase loadDatabaseFromType(FileReader*, PServer, IdentifierType, const string & name, int type);
	Database(const string & name);
	Database(const Database & d);
	virtual ~Database();

	virtual uint32_t getDatabaseType() = 0;
	virtual void notifyAddDatabase(PServer server, PUser user, bool useDimWorker) {}
	virtual void notifyRemoveDatabase(PServer server, bool useDimWorker) {}
	virtual void notifyRenameDatabase(PServer server, const string &oldName, bool useDimWorker) {}
	virtual bool isLoadable();
	virtual void loadDatabase(PServer server, bool addSystemDimension);
	virtual void saveDatabase(PServer server);
	virtual void saveDatabaseType(FileWriter*) = 0;
	virtual void setDatabaseFile(PServer server, const FileName&);
	virtual void deleteDatabaseFiles(PServer server);
	virtual void unloadDatabase(PServer server);
	virtual ItemType getType() const = 0;
	void setStatus(DatabaseStatus status)
	{
		if (this->status != LOADING) {
			this->status = status;
		}
	}

	virtual void processJournalsChronologically(PServer server, FileReader *file, bool &dbChanged);

	DatabaseStatus getStatus() const
	{
		return status;
	}

	const IdentifierType getIdentifier() const
	{
		return getId();
	}

	void setDeletable(bool deletable)
	{
		checkCheckedOut();
		this->deletable = deletable;
	}

	bool isDeletable() const
	{
		return deletable;
	}

	void setRenamable(bool renamable)
	{
		checkCheckedOut();
		this->renamable = renamable;
	}

	bool isRenamable() const
	{
		return renamable;
	}

	void setExtensible(bool extensible)
	{
		checkCheckedOut();
		this->extensible = extensible;
	}

	bool isExtensible() const
	{
		return extensible;
	}

	uint32_t getToken() const
	{
		return token;
	}

	PJournalMem getJournal() const
	{
		return journal;
	}

	string getPath() const
	{
		if (fileName == 0) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "database file name not set");
		} else {
			return fileName->path;
		}
	}

	size_t sizeDimensions() const
	{
		return dims->size();
	}

	vector<CPDimension> getDimensions(PUser user) const;
	size_t sizeCubes() const
	{
		return cubes->size();
	}

	vector<CPCube> getCubes(PUser user) const;

	ConfigurationCube::ClientCacheType getClientCacheType() const
	{
		return cacheType;
	}

	void setClientCacheType(ConfigurationCube::ClientCacheType type)
	{
		checkCheckedOut();
		Logger::info << "setting client cache type to " << type << endl;
		cacheType = type;
	}

	bool getHideElements() const
	{
		return hideElements;
	}
	void setHideElements(PServer server, bool hide);

	RightsType getDefaultRight() const
	{
		return defaultRight;
	}
	void setDefaultRight(PServer server, RightsType right);

	void addDimension(PServer server, PDimension, bool notify, bool newid, IdentifierType *attributesDim, IdentifierType *attributesCube, IdentifierType *rightsCube, IdentifierType *dimDimElem, bool useDimWorker);
	void removeDimension(PServer server, PDimension, bool notify, bool useDimWorker);
	void renameDimension(PServer server, PDimension, const string&, bool notify, bool useDimWorker);
	PDimension addDimension(PServer server, const string & name, PUser user, bool isInfo, bool useDimWorker, bool useJournal);
	PDimension addAliasDimension(PServer server, const string & name, PDimension alias, bool useDimWorker);
	void deleteDimension(PServer server, PDimension dimension, PUser user, bool useJournal, bool useDimWorker);
	void renameDimension(PServer server, PDimension dimension, const string & name, PUser user, bool useDimWorker, bool useJournal);
	PDimension lookupDimension(IdentifierType identifier, bool write) const
	{
		PDimension dim = COMMITABLE_CAST(Dimension, dims->get(identifier, write));
		Context::getContext()->saveParent(shared_from_this(), dim);
		return dim;
	}

	PDimension lookupDimensionByName(const string & name, bool write) const
	{
		PDimension dim = COMMITABLE_CAST(Dimension, dims->get(name, write));
		Context::getContext()->saveParent(shared_from_this(), dim);
		return dim;
	}

	PDimension findDimension(IdentifierType identifier, PUser user, bool write) const
	{
		checkDbRoleRight(user, User::dimensionRight, RIGHT_READ);
		PDimension dimension = lookupDimension(identifier, write);
		if (dimension == 0) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "dimension not found", "dimension identifier", (int)(identifier));
		}
		return dimension;
	}

	PDimension findDimensionByName(const string & name, PUser user, bool write) const
	{
		checkDbRoleRight(user, User::dimensionRight, RIGHT_READ);
		PDimension dimension = lookupDimensionByName(name, write);
		if (dimension == 0) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "dimension not found", "name", name);
		}
		return dimension;
	}

	void addCube(PServer server, PCube cube, bool notify, bool newid, IdentifierType *newCubeDimElem, IdentifierType *newCellRightCube, IdentifierType *newCellPropsCube, bool useDimWorker);
	void removeCube(PServer server, PCube cube, bool notify, bool useDimWorker);
	void renameCube(PServer server, PCube cube, const string & name, bool notify, bool useDimWorker);
	void renameCube(PServer server, const string& oldName, const string& newName, bool useDimWorker);
	PCube addCube(PServer server, const string & name, IdentifiersType *dimensions, PUser user, bool isInfo, bool useDimWorker, bool useJournal);
	void deleteCube(PServer server, PCube cube, PUser user, bool useJournal, bool useDimWorker);
	void deleteCubeByName(PServer server, const string& name, bool useDimWorker);
	void renameCube(PServer server, PCube cube, const string & name, PUser user, bool useDimWorker, bool useJournal);
	PCube changeCubeType(PServer server, PCube cube, ItemType type, PUser user, bool useJournal);
	void deleteGpuStorage(PServer server, CPCube cube);
	void loadCube(PServer server, PCube cube, PUser user);
	void saveCube(PServer server, PCube cube, PUser user);
	void unloadCube(PServer server, PCube cube, PUser user);
	PCube lookupCube(IdentifierType identifier, bool write) const
	{
		PCube cube = COMMITABLE_CAST(Cube, cubes->get(identifier, write));
		Context::getContext()->saveParent(shared_from_this(), cube);
		return cube;
	}

	PCube lookupCubeByName(const string & name, bool write) const
	{
		PCube cube = COMMITABLE_CAST(Cube, cubes->get(name, write));
		Context::getContext()->saveParent(shared_from_this(), cube);
		return cube;
	}

	PCube findCube(IdentifierType identifier, PUser user, bool requireLoad, bool write) const
	{
		checkDbRoleRight(user, User::cubeRight, RIGHT_READ);
		PCube cube = lookupCube(identifier, write);
		if (cube == 0) {
			throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube not found", "cube identifier", (int)(identifier));
		}
		if (cube->getType() == SYSTEMTYPE) {
			cube->checkCubeAccessRight(user, RIGHT_READ, false, false);
		}
		if (requireLoad && cube->getStatus() == Cube::UNLOADED) {
			throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "cube identifier", (int)(identifier));
		}
		if (cube->isLocked()) {
			if (!user || CONST_COMMITABLE_CAST(Lock, *cube->getCubeLocks(PUser())->const_begin())->getUserIdentifier() != user->getId()) {
				throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to write to locked area", "user", user ? user->getName() : "");
			}
		}
		return cube;
	}

	PCube findCubeByName(const string & name, PUser user, bool requireLoad, bool write) const
	{
		checkDbRoleRight(user, User::cubeRight, RIGHT_READ);
		PCube cube = lookupCubeByName(name, write);
		if (cube == 0) {
			throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube not found", "name", name);
		}
		if (cube->getType() == SYSTEMTYPE) {
			cube->checkCubeAccessRight(user, RIGHT_READ, false, false);
		}
		if (requireLoad && cube->getStatus() == Cube::UNLOADED) {
			throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "name", name);
		}
		if (cube->isLocked()) {
			if (!user || CONST_COMMITABLE_CAST(Lock, *cube->getCubeLocks(PUser())->const_begin())->getUserIdentifier() != user->getId()) {
				throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to write to locked area", "user", user ? user->getName() : "");
			}
		}
		return cube;
	}

	PCubeList getCubeList(bool write)
	{
		return write && !cubes->isCheckedOut() ? COMMITABLE_CAST(CubeList, cubes->copy()) : cubes;
	}

	void setCubeList(PCubeList l);
	PDimensionList getDimensionList(bool write)
	{
		return write && !dims->isCheckedOut() ? COMMITABLE_CAST(DimensionList, dims->copy()) : dims;
	}

	void setDimensionList(PDimensionList l);
	ElementOld2NewMap *getDimensionMap(IdentifierType dimId);

	virtual bool createSystemItems(PServer server, bool forceCreate) {
		return false;
	}

	virtual bool merge(const CPCommitable & o, const PCommitable & p);
	virtual PCommitable copy() const = 0;

protected:
	uint32_t token;
	PFileName fileName;
	DatabaseStatus status;
	PJournalFile journalFile;
	PJournalMem journal;
	bool deletable;
	bool renamable;
	bool extensible;
private:
	static FileName computeCubeFileName(const FileName & fileName, IdentifierType identifier);

	void loadDatabaseOverview(FileReader *file);
	uint32_t loadDatabaseDimension(PServer server, FileReader *file);
	void loadDatabaseDimensions(PServer server, FileReader *file);
	PCube loadDatabaseCube(PServer server, FileReader *file);
	void loadDatabaseCubes(PServer server, FileReader *file);

	boost::shared_ptr<JournalFileReader> initJournalProcess();
	vector<PCube> processJournalCommand(PServer server, JournalFileReader &history, bool &changed, FileReader *file, IdentifiersType &deletedDims, IdentifierType &bulkDimId, vector<vector<string> > &elementBulkCommands);

	void saveDatabaseOverview(FileWriter *file);
	void saveDatabaseDimensions(FileWriter *file);
	void saveDatabaseCubes(FileWriter *file);

	void checkDbRoleRight(PUser user, User::RightObject object, RightsType minimumRight) const
	{
		if (User::checkUser(user)) {
			user->checkRoleDbRight(object, CONST_COMMITABLE_CAST(Database, shared_from_this()), minimumRight);
		}
	}

	void checkName(const string & name) const;
	void checkDimensionName(const string & name, bool isInfo) const;
	void checkCubeName(const string & name, bool isInfo) const;
	void checkOldCubes();
	void changeTokens(bool changeGroupCubeData);

private:
	PDimensionList dims;
	PCubeList cubes;
	ConfigurationCube::ClientCacheType cacheType;
	bool hideElements;
	RightsType defaultRight;
	PSharedMutex filelock;
	PDimensionOld2NewMap old2NewMap;
};

}

#endif
