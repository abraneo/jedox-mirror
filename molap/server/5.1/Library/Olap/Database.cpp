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

#include "Olap/Database.h"

#include <algorithm>
#include <iostream>

#include "Exceptions/FileFormatException.h"
#include "Exceptions/FileOpenException.h"
#include "Exceptions/ParameterException.h"

#include "Collections/DeleteObject.h"
#include "Collections/StringBuffer.h"

#include "InputOutput/FileReader.h"
#include "InputOutput/FileWriter.h"
#include "InputOutput/FileUtils.h"
#include "InputOutput/JournalFileReader.h"
#include "InputOutput/StringVectorReader.h"
#include "Logger/Logger.h"
#include "InputOutput/Statistics.h"

#include "Olap/AliasDimension.h"
#include "Olap/ConfigurationCube.h"
#include "Olap/NormalDatabase.h"
#include "Olap/NormalDimension.h"
#include "Olap/Server.h"
#include "Olap/SystemCube.h"
#include "Olap/RightsCube.h"
#include "Olap/Rule.h"
#include "Olap/SystemDimension.h"
#include "Olap/SystemDatabase.h"
#include "Olap/UserInfoCube.h"
#include "Olap/UserInfoDimension.h"
#include "Olap/UserInfoDatabase.h"

#include "Thread/WriteLocker.h"

namespace palo {

const string Database::INVALID_CHARACTERS = "\\/?*:|<>";

////////////////////////////////////////////////////////////////////////////////
// constructors and destructors
////////////////////////////////////////////////////////////////////////////////

Database::Database(const string& name) :
	Commitable(name), token(rand()), deletable(true), renamable(true), extensible(true), dims(new DimensionList()), cubes(new CubeList()), cacheType(ConfigurationCube::NO_CACHE), hideElements(false), defaultRight(RIGHT_SPLASH), filelock(new PaloSharedMutex)
{
	if (!token) {
		++token;
	}
	status = CHANGED; // fileName is always 0! Check for UNLOADED later
}

Database::Database(const Database& d) :
	Commitable(d), token(d.token), fileName(d.fileName), status(d.status), journalFile(d.journalFile), journal(Server::ignoreJournal || !journalFile ? 0 : new JournalMem(journalFile.get())), deletable(d.deletable),
	renamable(d.renamable), extensible(d.extensible), dims(d.dims), cubes(d.cubes), cacheType(d.cacheType), hideElements(d.hideElements), defaultRight(d.defaultRight), filelock(d.filelock), old2NewMap(d.old2NewMap)
{
}

Database::~Database()
{
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a dimension
////////////////////////////////////////////////////////////////////////////////

bool Database::isLoadable()
{
	return fileName == 0 ? false : (FileUtils::isReadable(*fileName) || FileUtils::isReadable(FileName(*fileName, "tmp")));
}

PDatabase Database::loadDatabaseFromType(FileReader* file, PServer server, IdentifierType identifier, const string& name, int type)
{
	int isDeletable = file->getDataInteger(3, 1);
	int isRenamable = file->getDataInteger(4, 1);
	int isExtensible = file->getDataInteger(5, 1);

	PDatabase database;

	switch (type) {
	case UserInfoDatabase::DB_TYPE:
		Logger::info << "registered user info database '" << name << "'" << endl;
		if (name == Server::NAME_CONFIG_DATABASE) {
			database.reset(new ConfigDatabase(name));
		} else {
			database.reset(new UserInfoDatabase(name));
		}
		break;

	case SystemDatabase::DB_TYPE:
		Logger::info << "registered system database '" << name << "'" << endl;
		database.reset(new SystemDatabase(name));
		break;

	default:
		Logger::info << "registered database '" << name << "'" << endl;
		database.reset(new NormalDatabase(name));
		break;
	}

	database->setDeletable(isDeletable != 0);
	database->setRenamable(isRenamable != 0);
	database->setExtensible(isExtensible != 0);
	database->setID(identifier);

	return database;
}

void Database::loadDatabaseOverview(FileReader* file)
{
	if (file->isSectionLine() && file->getSection() == "DATABASE") {
		file->nextLine();

		if (file->isDataLine()) {
			file->getDataInteger(0);
			file->getDataInteger(1);
			file->nextLine();
		}
	} else {
		throw FileFormatException("section 'DATABASE' not found", file);
	}
}

uint32_t Database::loadDatabaseDimension(PServer server, FileReader* file)
{
	uint32_t identifier = file->getDataInteger(0);
	string name = file->getDataString(1);

	Dimension::SaveType type = (Dimension::SaveType)file->getDataInteger(2);
	PDimension dimension = Dimension::loadDimensionFromType(server, file, name, type);
	dimension->setID(identifier);

	dims->add(dimension, false);

	file->nextLine();
	return identifier;
}

void Database::loadDatabaseDimensions(PServer server, FileReader* file)
{
	dims.reset(new DimensionList());

	// load dimension section
	if (file->isSectionLine() && file->getSection() == "DIMENSIONS") {
		file->nextLine();

		IdentifierType maxId = 0;
		while (file->isDataLine()) {
			uint32_t dimId = loadDatabaseDimension(server, file);
			if (dimId > maxId) {
				maxId = dimId;
			}
		}
		dims->setNewIDStart(maxId + 1);
	} else {
		throw FileFormatException("section 'DIMENSIONS' not found", file);
	}

	while (file->isSectionLine()) {
		string section = file->getSection();
		if (!section.compare(0, 10, "DIMENSION ")) {
			IdentifierType dimId = StringUtils::stringToInteger(section.substr(10));
			// load dimension data into memory
			PDimension dimension = COMMITABLE_CAST(Dimension, dims->get(dimId, true));
			if (dimension != 0) {
				dimension->loadDimension(server, COMMITABLE_CAST(Database, shared_from_this()), file);
			} else {
				throw FileFormatException("Dimension '" + section + "' not found.", file);
			}
		} else {
			break;
		}
	}
}

PCube Database::loadDatabaseCube(PServer server, FileReader* file)
{
	uint32_t identifier = file->getDataInteger(0);
	string name = file->getDataString(1);

	const vector<int> dids = file->getDataIntegers(2);
	int type = file->getDataInteger(3);

#ifdef x_ENABLE_GPU_SERVER
	// TODO: -jj- test code = convert all cubes to gpu
	if (type == 2) {
		type = 7;
	}
#endif

	// if we have gpu cube, but gpu server not enabled - fall back to normal cube
	if (type == 7) {
		if (!server->isEnableGpu()) {
			// normal cube type
			type = 2;
			Logger::info << " cube with Id: '" << identifier << "' fall back to normal cube - gpu server not enabled" << endl;
		}
	}

	bool deleteable = file->getDataBool(4);
	bool renamable = file->getDataBool(5);

	IdentifiersType dims;

	for (vector<int>::const_iterator i = dids.begin(); i != dids.end(); ++i) {
		dims.push_back(*i);
	}

	// create cube
	PCube cube(Cube::loadCubeFromType(file, COMMITABLE_CAST(Database, shared_from_this()), identifier, name, &dims, type));
	cube->setID(identifier);
	cube->setDeletable(deleteable);
	cube->setRenamable(renamable);

//	cout << name << endl;
	// addCube to DB and load cell data
	addCube(server, cube, false, false, NULL, NULL, NULL, false);

	file->nextLine();

	return cube;
}

void Database::checkOldCubes()
{
	// check for old cube files
	vector<string> files = FileUtils::listFiles(fileName->path);
	string prefix = fileName->name + "_cube_";
	string postfix = "." + fileName->extension;
	string rules = "_rules";
	for (vector<string>::iterator iter = files.begin(); iter != files.end(); ++iter) {
		string name = StringUtils::tolower(*iter);
		if (name.size() >= max(prefix.size(), postfix.size()) && name.compare(0, prefix.size(), prefix) == 0 && name.compare(name.size() - postfix.size(), postfix.size(), postfix) == 0 && name.find(rules) == string::npos) {
			string num = name.substr(prefix.size(), name.size() - prefix.size() - postfix.size());
			try {
				IdentifierType id = StringUtils::stringToInteger(num);
				if (!cubes->get(id, false)) {
					FileName cubeStrange(fileName->path, fileName->name + "_CUBE_" + StringUtils::convertToString(id), fileName->extension);
					FileName cubeDeleted(fileName->path, fileName->name + "_CUBE_" + StringUtils::convertToString(id), "corrupt");
					FileUtils::remove(cubeDeleted);
					FileUtils::rename(cubeStrange, cubeDeleted);
					Logger::info << "found obsolete cube file " << cubeStrange.fullPath() << ", removing" << endl;
				}
			} catch (const ParameterException &) {
			}
		}
	}
}

void Database::loadDatabaseCubes(PServer server, FileReader *file)
{
	cubes.reset(new CubeList);
	if (file->isSectionLine() && file->getSection() == "CUBES") {
		IdentifierType maxId = 0;
		file->nextLine();
		while (file->isDataLine()) {
			PCube cube = loadDatabaseCube(server, file);
			if (cube->getStatus() == Cube::LOADED) {
				cube->setStatus(Cube::UNLOADED); // cube journal to be processed later
			}
			if (cube->getId() > maxId) {
				maxId = cube->getId();
			}
		}
		cubes->setNewIDStart(maxId + 1);
	} else {
		throw FileFormatException("section 'CUBES' not found", file);
	}
}

boost::shared_ptr<JournalFileReader> Database::initJournalProcess()
{
	checkCheckedOut();

	boost::shared_ptr<JournalFileReader> history(new JournalFileReader(FileName(*fileName, "log")));
	if (!history->openFile(false, false)) {
		history.reset();
	}

	return history;
}

vector<PCube> Database::processJournalCommand(PServer server, JournalFileReader &history, bool &changed, FileReader *file, IdentifiersType &deletedDims, IdentifierType &bulkDimId, vector<vector<string> > &elementBulkCommands)
{
	string username = history.getDataString(1);
	string event = history.getDataString(2);
	string command = history.getDataString(3);
	vector<PCube> createdCubes;
	bool inBulk = (file == NULL);

	if (command == JournalFileReader::JOURNAL_VERSION) {
		int release = history.getDataInteger(4);
		int sr = history.getDataInteger(5);
		int build = history.getDataInteger(7);
		history.setVersion(release, sr, build);

		if (history.getVersion().isOld()) {
			throw ErrorException(ErrorException::ERROR_INVALID_VERSION, "database " + StringUtils::convertToString(getId()) + " has nonempty journal file from old version");
		}
	}

	else if (history.getVersion().isUnknown()) {
		throw ErrorException(ErrorException::ERROR_INVALID_VERSION, "database " + StringUtils::convertToString(getId()) + " has nonempty journal file from old version");
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENTS_BULK_START) {
		bulkDimId = history.getDataInteger(4);
		elementBulkCommands.clear();
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENTS_BULK_STOP) {
		IdentifierType dimId = history.getDataInteger(4);
		if (dimId != bulkDimId) {
			Logger::error << "Corrupted element bulk in journal, started " << bulkDimId << ", ended with " << dimId << endl;
		} else {
			PDimension dimension = findDimension(dimId, PUser(), true);

			if (!file) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "unknown filename in journal processing");
			}

			JournalFileReader jfr(boost::shared_ptr<FileReader>(new StringVectorReader(elementBulkCommands, file->getFileName())), file->getFileName());
			jfr.setVersion(history.getVersion().release, history.getVersion().sr, history.getVersion().build);
			elementBulkCommands.clear();
			bulkDimId = NO_IDENTIFIER;
			while (jfr.isDataLine()) {
				processJournalCommand(server, jfr, changed, NULL, deletedDims, bulkDimId, elementBulkCommands);
				jfr.nextLine(false);
			}
			dimension->updateElementsInfo();
		}
	}

	else if (bulkDimId != NO_IDENTIFIER) { // we are in bulk, save command to execute it in BULK STOP
		elementBulkCommands.push_back(history.getDataStrings(NO_IDENTIFIER, ';'));
	}

	else if (command == JournalFileReader::JOURNAL_CUBE_CONVERT) {
		IdentifierType idCube = history.getDataInteger(4);
		ItemType type = (ItemType)history.getDataInteger(5);

		if (server->isEnableGpu()) {
			PCube cube = findCube(idCube, PUser(), false, true);
			cube->setStatus(Cube::CHANGED); // UNLOADED was set before journal load started, can be changed here

			changeCubeType(server, cube, type, PUser(), false);

			changed = true;
		} else {
			Logger::warning << "GPU server not enabled, cannot process convert command (cube " << idCube << ")" << endl;
		}
	}

	else if (command == JournalFileReader::JOURNAL_DIMENSION_DESTROY) {
		IdentifierType idDimension = history.getDataInteger(4);
		PDimension dimension = findDimension(idDimension, PUser(), true);

		dimension->setDeletable(true);
		deleteDimension(server, dimension, PUser(), false, false);

		deletedDims.push_back(idDimension);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_DIMENSION_RENAME) {
		IdentifierType idDimension = history.getDataInteger(4);
		const string name = history.getDataString(5);

		renameDimension(server, findDimension(idDimension, PUser(), true), name, PUser(), false, false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_CUBE_DESTROY) {
		IdentifierType idCube = history.getDataInteger(4);
		PCube cube = findCube(idCube, PUser(), false, true);

		cube->setDeletable(true);
		deleteCube(server, cube, PUser(), false, false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_CUBE_RENAME) {
		IdentifierType idCube = history.getDataInteger(4);
		const string name = history.getDataString(5);

		renameCube(server, findCube(idCube, PUser(), false, true), name, PUser(), false, false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_DIMENSION_CLEAR) {
		IdentifierType idDimension = history.getDataInteger(4);

		PDimension dim = findDimension(idDimension, PUser(), true);
		dim->clearElements(server, COMMITABLE_CAST(Database, shared_from_this()), PUser(), false, false);
		dim->updateElementsInfo();

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_MOVE) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		IdentifierType position = history.getDataInteger(6);
		PDimension dimension = findDimension(idDimension, PUser(), true);

		dimension->moveElement(server, COMMITABLE_CAST(Database, shared_from_this()), dimension->findElement(idElement, 0, true), position, PUser(), false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_MOVE_BULK) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifiersType ids = history.getDataIdentifiers(5);
		IdentifiersType positions = history.getDataIdentifiers(6);
		PDimension dimension = findDimension(idDimension, PUser(), true);

		vector<pair<Element *, PositionType> > elem_pos;
		elem_pos.resize(ids.size());
		for (size_t i = 0; i < ids.size(); i++) {
			Element *elem = dimension->findElement(ids[i], 0, true);
			elem_pos[i] = make_pair(elem, positions[i]);
		}

		dimension->moveElements(server, COMMITABLE_CAST(Database, shared_from_this()), elem_pos, PUser(), false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_CREATE) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		const string name = history.getDataString(6);
		Element::Type type = (Element::Type)history.getDataInteger(7);
		PDimension dimension;
		try {
			dimension = findDimension(idDimension, PUser(), true);
		} catch (ParameterException&) {
			if (find(deletedDims.begin(), deletedDims.end(), idDimension) != deletedDims.end()) {
				Logger::debug << "Database " << getId() << ": element " << idElement << " to be added to deleted dimension " << idDimension << endl;
			} else {
				throw;
			}
		}
		if (dimension) {
			dimension->addElement(server, COMMITABLE_CAST(Database, shared_from_this()), idElement, name, type, PUser(), false);
			if (!inBulk) {
				dimension->updateElementsInfo();
			}

			changed = true;
		}
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_RENAME) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		const string name = history.getDataString(6);
		PDimension dimension = findDimension(idDimension, PUser(), true);
		Element * element = dimension->findElement(idElement, 0, true);

		dimension->changeElementName(server, COMMITABLE_CAST(Database, shared_from_this()), element, name, PUser(), false, false);

		changed = true;
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_APPEND) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		IdentifiersType children = history.getDataIdentifiers(6);
		vector<double> weights = history.getDataDoubles(7);

		PDimension dimension;
		try {
			dimension = findDimension(idDimension, PUser(), true);
		} catch (ParameterException&) {
			if (find(deletedDims.begin(), deletedDims.end(), idDimension) != deletedDims.end()) {
				Logger::debug << "Database " << getId() << ": element " << idElement << " to be modified in deleted dimension " << idDimension << endl;
			} else {
				throw;
			}
		}

		if (dimension) {
			if (children.size() != weights.size()) {
				throw FileFormatException("children weights corrupted", file);
			}

			size_t len = children.size();
			IdentifiersWeightType iw(len);
			for (size_t i = 0; i < len; i++) {
				iw[i].first = children[i];
				iw[i].second = weights[i];
			}

			dimension->addChildren(server, COMMITABLE_CAST(Database, shared_from_this()), dimension->findElement(idElement, 0, true), &iw, PUser(), NULL, true, !inBulk, false, NULL);
			if (!inBulk) {
				dimension->updateElementsInfo(); // should do nothing
			}

			changed = true;
		}
	}

	else if (command == JournalFileReader::JOURNAL_ELEMENT_REPLACE) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		Element::Type type = (Element::Type)history.getDataInteger(6);

		PDimension dimension;
		try {
			dimension = findDimension(idDimension, PUser(), true);
		} catch (ParameterException&) {
			if (find(deletedDims.begin(), deletedDims.end(), idDimension) != deletedDims.end()) {
				Logger::debug << "Database " << getId() << ": element " << idElement << " to be modified in deleted dimension " << idDimension << endl;
			} else {
				throw;
			}
		}

		if (dimension) {
			dimension->changeElementType(server, COMMITABLE_CAST(Database, shared_from_this()), dimension->findElement(idElement, 0, true), type, PUser(), false, NULL, NULL, true);
			if (!inBulk) {
				dimension->updateElementsInfo();
			}

			changed = true;
		}
	} else if (command == JournalFileReader::JOURNAL_ELEMENT_DESTROY) {
		IdentifierType idDimension = history.getDataInteger(4);
		PDimension dimension = findDimension(idDimension, PUser(), true);
		IdentifiersType ids = history.getDataIdentifiers(5);
		dimension->deleteElements(server, COMMITABLE_CAST(Database, shared_from_this()), ids, PUser(), false, NULL, false);
		dimension->updateElementsInfo();

		changed = true;
	} else if (command == JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		PDimension dimension;
		try {
			dimension = findDimension(idDimension, PUser(), true);
		} catch (ParameterException&) {
			if (find(deletedDims.begin(), deletedDims.end(), idDimension) != deletedDims.end()) {
				Logger::debug << "Database " << getId() << ": element " << idElement << " to be modified in deleted dimension " << idDimension << endl;
			} else {
				throw;
			}
		}

		if (dimension) {
			dimension->removeChildren(server, COMMITABLE_CAST(Database, shared_from_this()), PUser(), dimension->findElement(idElement, 0, true), NULL, false, false);
			if (!inBulk) {
				dimension->updateElementsInfo();
			}

			changed = true;
		}
	} else if (command == JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN_NOT_IN) {
		IdentifierType idDimension = history.getDataInteger(4);
		IdentifierType idElement = history.getDataInteger(5);
		IdentifiersType children = history.getDataIdentifiers(6);
		PDimension dimension;
		try {
			dimension = findDimension(idDimension, PUser(), true);
		} catch (ParameterException&) {
			if (find(deletedDims.begin(), deletedDims.end(), idDimension) != deletedDims.end()) {
				Logger::debug << "Database " << getId() << ": element " << idElement << " to be modified in deleted dimension " << idDimension << endl;
			} else {
				throw;
			}
		}

		if (dimension) {
			set<IdentifierType> keep;

			for (size_t i = 0; i < children.size(); i++) {
				keep.insert(children[i]);
			}

			dimension->removeChildrenNotIn(server, COMMITABLE_CAST(Database, shared_from_this()), PUser(), dimension->findElement(idElement, 0, true), &keep, NULL, false);
			if (!inBulk) {
				dimension->updateElementsInfo();
			}

			changed = true;
		}
	} else if (command == JournalFileReader::JOURNAL_CUBE_CREATE) {
		IdentifierType idCube = history.getDataInteger(4);
		string nameCube = history.getDataString(5);
		IdentifiersType idDims = history.getDataIdentifiers(6);
		bool isUserInfo = (history.getDataInteger(7) == 3);
		IdentifierType idNewCubeDimElem = history.getDataInteger(8);
		IdentifierType idNewCellRightCube = history.getDataInteger(9);
		IdentifierType idNewCellPropsCube = history.getDataInteger(10);
		PCube c = lookupCube(idCube, false);
		if (!c) {
			PCube cube;
			if (isUserInfo) {
				cube = PCube(new UserInfoCube(COMMITABLE_CAST(Database, shared_from_this()), nameCube, &idDims));
			} else {
				cube = PCube(new Cube(COMMITABLE_CAST(Database, shared_from_this()), nameCube, &idDims, Cube::NORMAL));
			}
			cube->setID(idCube);
			addCube(server, cube, true, false, &idNewCubeDimElem, &idNewCellRightCube, &idNewCellPropsCube, false);

			createdCubes.push_back(cube);

			PCube newCube;
			if (newCube = lookupCube(idNewCellRightCube, false)) {
				createdCubes.push_back(newCube);
			}
			if (newCube = lookupCube(idNewCellPropsCube, false)) {
				createdCubes.push_back(newCube);
			}

			changed = true;
		} else {
			if (c->getName() != nameCube) {
				throw ErrorException(ErrorException::ERROR_CORRUPT_FILE, "Journal and database file are inconsistent.");
			}
		}
	} else if (command == JournalFileReader::JOURNAL_DIMENSION_CREATE) {
		IdentifierType id = history.getDataInteger(4);
		string name = history.getDataString(5);
		uint32_t type = (uint32_t)history.getDataInteger(6);
		IdentifierType attributesDim = history.getDataInteger(7, NO_IDENTIFIER);
		IdentifierType attributesCube = history.getDataInteger(8, NO_IDENTIFIER);
		IdentifierType rightsCube = history.getDataInteger(9, NO_IDENTIFIER);
		IdentifierType dimDimElem = history.getDataInteger(10, NO_IDENTIFIER);

		PDimension dim;
		if (type == 3) {
			dim = PDimension(new UserInfoDimension(name));
		} else {
			dim = PDimension(new NormalDimension(name));
		}
		dim->setID(id);
		addDimension(server, dim, true, false, &attributesDim, &attributesCube, &rightsCube, &dimDimElem, false);

		PCube newCube;
		if (newCube = lookupCube(attributesCube, false)) {
			createdCubes.push_back(newCube);
		}
		if (newCube = lookupCube(rightsCube, false)) {
			createdCubes.push_back(newCube);
		}

		changed = true;
	}

	return createdCubes;
}

static PDimensionOld2NewMap readDimensionMap(FileReader *file)
{
	PDimensionOld2NewMap result;
	ElementOld2NewMap elemMap;
	IdentifierType dimId = NO_IDENTIFIER;
	if (file->openFile(false, false)) {
		while (!file->isEndOfFile()) {
			if (file->isDataLine() && dimId != NO_IDENTIFIER) {
				IdentifierType oldId = file->getDataInteger(0, NO_IDENTIFIER);
				IdentifierType newId = file->getDataInteger(1, NO_IDENTIFIER);
				elemMap.setTranslation(oldId, newId);
			} else if (file->isSectionLine()) {
				if (dimId != NO_IDENTIFIER && elemMap.size()) {
					if (!result) {
						result.reset(new DimensionOld2NewMap);
					}
					(*result)[dimId] = elemMap;
				}
				elemMap.clear();
				string section = file->getSection();
				if (!section.compare(0, 10, "DIMENSION ")) {
					dimId = StringUtils::stringToInteger(section.substr(10));
				} else {
					break;
				}
			}
			file->nextLine();
		}
	}
	return result;
}

void Database::loadDatabase(PServer server, bool addSystemDimension)
{
	checkCheckedOut();
	if (status == LOADED) {
		return;
	}
	bool useLog = status == UNLOADED;
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database file name not set");
	}
	if (FileUtils::isReadable(FileName(fileName->path, "_loading", "lock"))) {
		throw ParameterException(ErrorException::ERROR_INTERNAL, "incompletely restored database", "db", getName());
	}
	status = LOADING;
	bool tmpExtensible = extensible;
	extensible = true;
	bool dbChanged = false;
	bool dbRemapping = false;

	{
		WriteLocker wl(filelock->getLock());
		if (journalFile != 0) {
			journalFile->closeFile();
		}
		// load database from file
		if (!FileUtils::isReadable(*fileName) && FileUtils::isReadable(FileName(*fileName, "tmp"))) {
			Logger::warning << "using temp file for database '" << getName() << "'" << endl;

			// delete journal
			JournalFile::deleteJournalFiles(FileName(*fileName, "log"), false);

			// rename temp file
			if (!FileUtils::rename(FileName(*fileName, "tmp"), *fileName)) {
				Logger::error << "cannot rename database file: '" << strerror(errno) << "'" << endl;
				Logger::error << "please check the underlying file system for errors" << endl;
				exit(1);
			}
		}
		{
			// check presence of DimensionMapping file and load it
			FileName mapFileName(fileName->path, "database_map", "csv");
			if (FileUtils::isReadable(mapFileName)) {
				boost::shared_ptr<FileReader> mapFR(FileReader::getFileReader(mapFileName));
				old2NewMap = readDimensionMap(mapFR.get());
				dbRemapping = true;
			}
		}

		{
			boost::shared_ptr<FileReader> fr(FileReader::getFileReader(*fileName));
			fr->openFile(true, false);

			loadDatabaseOverview(fr.get());

			// load dimensions from file
			loadDatabaseDimensions(server, fr.get());

			// load cubes with cells data from file
			loadDatabaseCubes(server, fr.get());

			if (addSystemDimension) {
				dbChanged = this->createSystemItems(server, false); // calls NormalDatabase::createSystemItems
			}

			// and read the journal
			if (useLog) {
				processJournalsChronologically(server, fr.get(), dbChanged);
			} else {
				// delete the journal as we have replaced the database with the saved data
				JournalFile::deleteJournalFiles(FileName(*fileName, "log"), false);
			}
		}

		if (journal) journal->flush();
		if (journalFile) journalFile->clear();
	}

	IdentifierType maxId = 0;
	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); ++i) {
		PCube cube = COMMITABLE_CAST(Cube, *i);

		if (cube != 0) {
			if (dbRemapping) {
				cube->setStatus(Cube::CHANGED);
			}
			if (cube->getStatus() == Cube::CHANGED) {
				saveCube(server, cube, PUser());
			}
			cube->setStatus(Cube::LOADED);

			if (cube->getId() > maxId) {
				maxId = cube->getId();
			}
		}
	}
	cubes->setNewIDStart(maxId + 1);

	// force a write back to disk, this will archive the journal
	if (dbChanged || dbRemapping) {
		status = CHANGED;
		saveDatabase(server);
	}
	extensible = tmpExtensible;
	// database is now loaded
	status = LOADED;
}

struct journalStruct {
	timeval tv;
	set<PCube> changedCubes;
	CPCube cpcube;
	PCube pcube;
	boost::shared_ptr<JournalFileReader> history;
	Cube::InBulkEnum inBulk;
};

void Database::processJournalsChronologically(PServer server, FileReader *file, bool &dbChanged)
{
	Context::getContext()->setInJournal(true);

	vector<journalStruct> journals;

	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());

	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); ++i) {
		PCube cube = COMMITABLE_CAST(Cube, *i);
		if (cube != 0) {
			boost::shared_ptr<JournalFileReader> history = cube->initJournalProcess();
			if (!history.get() || !history->isDataLine()) {
				continue;
			}

			journalStruct js;
			CPCube thisCube = CONST_COMMITABLE_CAST(Cube, cube->shared_from_this());
			long int sec, usec;
			history->getTimeStamp(&sec, &usec, 0);
			js.tv.tv_sec = sec;
			js.tv.tv_usec = usec;
			js.cpcube = thisCube;
			js.pcube = cube;
			js.history = history;
			js.inBulk = Cube::Not;
			journals.push_back(js);
		}
	}

	boost::shared_ptr<JournalFileReader> history = initJournalProcess();
	if (history && history->isDataLine()) {
		journalStruct js;
		long int sec, usec;
		history->getTimeStamp(&sec, &usec, 0);
		js.tv.tv_sec = sec;
		js.tv.tv_usec = usec;
		js.cpcube = CPCube();
		js.pcube = PCube();
		js.history = history;
		journals.push_back(js);
	}

	IdentifiersType deletedDims;
	vector<vector<string> > elementBulkCommands;
	IdentifierType bulkDimId = NO_IDENTIFIER;

	if (!journals.empty()) {
		Logger::info << "found non-empty journal(s) in database " << getName() << ", processing started" << endl;
	}

	while (!journals.empty()) { // while any journal contains commands
		vector<journalStruct>::iterator min = journals.begin();
		vector<journalStruct>::iterator it = min;
		++it;
		for (; it != journals.end(); ++it) {
			if (it->tv.tv_sec < min->tv.tv_sec || (it->tv.tv_sec == min->tv.tv_sec && it->tv.tv_usec < min->tv.tv_usec)) {
				min = it;
			}
		}

		vector<PCube> newCubes;

		// process journal command from "min" iterator
		if (min->pcube) {
			// cube journal
			min->pcube->processJournalCommand(server, db, min->cpcube, *min->history, min->inBulk, min->changedCubes);
		} else {
			// database journal
			newCubes = processJournalCommand(server, *min->history, dbChanged, file, deletedDims, bulkDimId, elementBulkCommands);
		}

		min->history->nextLine();
		if (min->history->isDataLine()) {
			long int sec, usec;
			min->history->getTimeStamp(&sec, &usec, 0);
			min->tv.tv_sec = sec;
			min->tv.tv_usec = usec;
		} else {
			// nothing more in this journal
			if (min->pcube) { // not a database journal
				min->pcube->commitChanges(false, PUser(), min->changedCubes, false);
			}
			journals.erase(min);
		}

		// add new cubes to journal process
		for (vector<PCube>::iterator newIt = newCubes.begin(); newIt != newCubes.end(); ++newIt) {
			PCube cube = *newIt;
			cube->setStatus(Cube::UNLOADED); // journal was not loaded yet, must be done chronologically here
			boost::shared_ptr<JournalFileReader> history = cube->initJournalProcess();
			if (!history.get() || !history->isDataLine()) {
				continue;
			}

			journalStruct js;
			CPCube thisCube = CONST_COMMITABLE_CAST(Cube, cube->shared_from_this());
			long int sec, usec;
			history->getTimeStamp(&sec, &usec, 0);
			js.tv.tv_sec = sec;
			js.tv.tv_usec = usec;
			js.cpcube = thisCube;
			js.pcube = cube;
			js.history = history;
			js.inBulk = Cube::Not;
			journals.push_back(js);
		}
	}

	Context::getContext()->setInJournal(false);
}

void Database::saveDatabaseOverview(FileWriter *file)
{
	int32_t db_type = (int32_t)(getType());
	file->appendComment("PALO DATABASE DATA");
	string versionInfo = "version: " + Server::getVersionRevision();
	file->appendComment(versionInfo);
	file->appendComment("");
	file->appendComment("Description of data:");
	file->appendComment("SIZE_DIMENSIONS;SIZE_CUBES;TYPE");
	file->appendSection("DATABASE");
	file->appendInteger((int32_t)(dims->getLastId()));
	file->appendInteger((int32_t)(cubes->getLastId()));
	//YLS: Translation between the internal, normalized types and
	// the ad-hoc database types (visible in palo.csv and database.csv)
	switch (db_type) {
	case NORMALTYPE:
		db_type = NormalDatabase::DB_TYPE;
		break;
	case SYSTEMTYPE:
		db_type = SystemDatabase::DB_TYPE;
		break;
	case USER_INFOTYPE:
		db_type = UserInfoDatabase::DB_TYPE;
		break;
	}
	file->appendInteger(db_type);
	file->nextLine();
}

void Database::saveDatabaseDimensions(FileWriter *file)
{
	file->appendComment("Description of data:");
	file->appendComment("ID;NAME;TYPE;...");
	file->appendSection("DIMENSIONS");
	for (DimensionList::Iterator i = dims->begin(); i != dims->end(); ++i) {
		PDimension dimension = COMMITABLE_CAST(Dimension, *i);
		dimension->saveDimensionType(file);
	}
	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());
	for (DimensionList::Iterator i = dims->begin(); i != dims->end(); ++i) {
		PDimension dimension = COMMITABLE_CAST(Dimension, *i);
		dimension->saveDimension(db, file);
	}
}

void Database::saveDatabaseCubes(FileWriter *file)
{
	file->appendComment("Description of data:");
	file->appendComment("ID;NAME;DIMENSIONS;TYPE;DELETABLE;RENAMABLE;...");
	file->appendSection("CUBES");
	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); i++) {
		PCube cube = COMMITABLE_CAST(Cube, *i);

		cube->saveCubeType(file);
	}
}

void Database::saveDatabase(PServer server)
{
	if (status == LOADED || status == LOADING) {
		return;
	}
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database file name not set");
	}
	// open a new temp-database file
	WriteLocker wl(filelock->getLock());
	boost::shared_ptr<FileWriter> fw(FileWriter::getFileWriter(FileName(*fileName, "tmp")));
	fw->openFile();
	// save database, dimension and cubes to disk
	saveDatabaseOverview(fw.get());
	saveDatabaseDimensions(fw.get());
	saveDatabaseCubes(fw.get());
	fw->appendComment("");
	fw->appendComment("");
	fw->appendComment("PALO DATABASE DATA END");
	fw->closeFile();
	if (journalFile != 0) {
		journalFile->closeFile();
	}
	// archive journal
	JournalFile::archiveJournalFiles(FileName(*fileName, "log"));
	// remove old database file
	if (FileUtils::isReadable(*fileName) && !FileUtils::remove(*fileName)) {
		Logger::error << "cannot remove database file: '" << strerror(errno) << "'" << endl;
		Logger::error << "please check the underlying file system for errors" << endl;
		exit(1);
	}
	// delete journal
	JournalFile::deleteJournalFiles(FileName(*fileName, "log"), false);
	// rename temp-database file
	if (!FileUtils::rename(FileName(*fileName, "tmp"), *fileName)) {
		Logger::error << "cannot rename database file: '" << strerror(errno) << "'" << endl;
		Logger::error << "please check the underlying file system for errors" << endl;
		exit(1);
	}

	{
		// rename remapping file if present
		FileName mapFileName(fileName->path, "database_map", "csv");
		if (FileUtils::isReadable(mapFileName)) {
			if (!FileUtils::rename(mapFileName, FileName(mapFileName, "archive"))) {
				Logger::error << "cannot rename remapping file: '' " << mapFileName.fullPath() << " Error: '" << strerror(errno) << "'" << endl;
				Logger::error << "please delete it manually and restart the server " << endl;
				exit(1);
			}
		}
	}

	if (journalFile != 0) {
		if (journal) journal->flush();
		journalFile->clear();
	}
	// database is now loaded
	setStatus(LOADED);
}

void Database::setDatabaseFile(PServer server, const FileName & newName)
{
	checkCheckedOut();
	// new association with a database file
	if (fileName == 0) {
		fileName.reset(new FileName(newName));

		if (FileUtils::isReadable(FileName(fileName->path, "database", "csv"))) {
			status = UNLOADED;
		} else {
			if (!FileUtils::createDirectory(fileName->path)) {
				Logger::error << "cannot create database directory: '" << strerror(errno) << "'" << endl;
				Logger::error << "please check the underlying file system for errors" << endl;
				exit(1);
			}

			saveDatabase(server);
		}

		journalFile.reset(new JournalFile(FileName(*fileName, "log"), filelock, getId(), -1));
		return;
	}
	// no change in database file
	if (newName.path == fileName->path) {
		return;
	}
	// change database file
	if (status == UNLOADED) {
		if (!FileUtils::renameDirectory(*fileName, newName)) {
			throw ParameterException(ErrorException::ERROR_RENAME_FAILED, "cannot rename database files", "name", getName());
		}
		fileName.reset(new FileName(newName));
	} else {
		saveDatabase(server);
		unloadDatabase(server);
		if (!FileUtils::renameDirectory(*fileName, newName)) {
			throw ParameterException(ErrorException::ERROR_RENAME_FAILED, "cannot rename database files", "name", getName());
		}
		fileName.reset(new FileName(newName));
		loadDatabase(server, false);
	}

	journalFile.reset(new JournalFile(FileName(*fileName, "log"), filelock, getId(), -1));
}

void Database::deleteDatabaseFiles(PServer server)
{
	checkCheckedOut();
	if (fileName == 0) {
		return;
	}
	// delete database file from disk
	if (FileUtils::isReadable(*fileName)) {
		if (status == UNLOADED) {
			loadDatabase(server, false);
		}
		FileWriter::deleteFile(*fileName);
	}

	// delete cube files of database from disk
	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); ++i) {
		PCube cube = COMMITABLE_CAST(Cube, *i);

		cube->deleteCubeFiles();
	}
	journalFile->closeFile();
	JournalFile::deleteJournalFiles(FileName(*fileName, "log"));
	// delete path
	if (!FileUtils::removeDirectory(*fileName)) {
		Logger::error << "cannot remove database directory: '" << strerror(errno) << "'" << endl;
		Logger::error << "please check the underlying file system for errors" << endl;
	}
	if (journal) journal->flush();
	journalFile->clear();
}

void Database::unloadDatabase(PServer server)
{
	checkCheckedOut();
	if (!isLoadable()) {
		throw ParameterException(ErrorException::ERROR_DATABASE_UNSAVED, "cannot unload an unsaved database, you must use delete", "", "");
	}
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database file name not set");
	}
	if (!FileUtils::isReadable(*fileName)) {
		throw ParameterException(ErrorException::ERROR_CORRUPT_FILE, "cannot load file", "file name", fileName->fullPath());
	}
	dims.reset(new DimensionList);
	cubes.reset(new CubeList);
	// database is now unloaded
	status = UNLOADED;
}

////////////////////////////////////////////////////////////////////////////////
// getter and setter
////////////////////////////////////////////////////////////////////////////////
vector<CPDimension> Database::getDimensions(PUser user) const
{
	checkDbRoleRight(user, User::dimensionRight, RIGHT_READ);
	vector<CPDimension> result;
	for (DimensionList::Iterator i = dims->begin(); i != dims->end(); ++i) {
		PDimension dimension = COMMITABLE_CAST(Dimension, *i);

		if (dimension == 0) {
			continue;
		}

		Context::getContext()->saveParent(shared_from_this(), dimension);

		bool push = false;
		if (!User::checkUser(user)) {
			push = true;
		} else if (dimension->getType() == SYSTEMTYPE) {
			switch (dimension->getDimensionType()) {
			case Dimension::ATTRIBUTES:
			case Dimension::CELLPROPS:
			case Dimension::CONFIG:
				push = true;
				break;
			default:
				push = user->getRoleRight(User::rightsRight) > RIGHT_NONE;
			}
			if (!push) {
				switch (dimension->getDimensionType()) {
				case Dimension::ALIAS:
					if (dimension->getName() == SystemDatabase::NAME_USER_DIMENSION) {
						push = user->getRoleRight(User::userRight) > RIGHT_NONE;
					} else if (dimension->getName() == SystemDatabase::NAME_GROUP_DIMENSION) {
						push = user->getRoleRight(User::groupRight) > RIGHT_NONE;
					} else {
						push = user->getRoleRight(User::elementRight) > RIGHT_NONE;
					}
					break;
				case Dimension::CUBE:
				case Dimension::DIMENSION:
				case Dimension::SUBSETVIEW:
					if (!push) {
						push = user->getRoleRight(User::subSetViewRight) > RIGHT_NONE;
					}
					break;
				default: ;
				}
			}
		} else if (dimension->getType() == USER_INFOTYPE) {
			push = user->getRoleRight(User::userInfoRight) > RIGHT_NONE;
		} else {
			push = true;
		}

		if (push) {
			result.push_back(dimension);
		}
	}
	return result;
}

vector<CPCube> Database::getCubes(PUser user) const
{
	checkDbRoleRight(user, User::cubeRight, RIGHT_READ);
	vector<CPCube> result;
	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); ++i) {
		PCube cube = COMMITABLE_CAST(Cube, *i);

		if (cube == 0) {
			continue;
		}
		Context::getContext()->saveParent(shared_from_this(), cube);

		bool push = false;
		if (!User::checkUser(user)) {
			push = true;
		} else if (cube->getType() == SYSTEMTYPE) {
			switch (cube->getCubeType()) {
				case Cube::ATTRIBUTES:
				case Cube::CONFIGURATION:
					push = true;
					break;
				case Cube::SUBSETVIEW:
					push = user->getRoleRight(User::subSetViewRight) > RIGHT_NONE;
					break;
				default:
					// only users from "admin" group are allowed to view #_GROUP_DATABASE_DATA
					if (cube->getName() != SystemDatabase::NAME_GROUP_DATABASE_CUBE) {
						push = user->getRoleRight(User::rightsRight) > RIGHT_NONE;
					}
			}
		} else if (cube->getType() == USER_INFOTYPE) {
			push = user->getRoleRight(User::userInfoRight) > RIGHT_NONE;
		} else {
			// normal and gpu cube
			push = true;
		}
		if (push) {
			result.push_back(cube);
		}
	}
	return result;
}

void Database::setHideElements(PServer server, bool hide)
{
	checkCheckedOut();
	bool changed = hideElements != hide;
	Logger::info << "setting hide elements to " << hide << endl;
	hideElements = hide;

	if (changed) {
		changeTokens(false);
	}
}

void Database::setDefaultRight(PServer server, RightsType right)
{
	checkCheckedOut();
	bool changed = defaultRight != right;
	defaultRight = right;

	if (changed) {
		Context::getContext()->setTokenUpdate(true);
		changeTokens(true);
	}
}

void Database::changeTokens(bool changeGroupCubeData)
{
	PDimensionList dimList = getDimensionList(true);
	PCubeList cubeList = getCubeList(changeGroupCubeData);
	for (CubeList::Iterator it = cubeList->begin(); it != cubeList->end(); ++it) {
		CPCube cube = COMMITABLE_CAST(Cube, *it);

		if (changeGroupCubeData && cube->getName() == SystemCube::GROUP_CUBE_DATA) {
			PCube c = lookupCube(cube->getId(), true);
			cubeList->set(c);
		}

		const IdentifiersType &dimensions = *cube->getDimensions();
		if (dimensions.size() == 2 && cube->getName() == SystemCube::PREFIX_GROUP_DIMENSION_DATA + lookupDimension(dimensions[1], false)->getName()) {
			PDimension dim = lookupDimension(dimensions.at(1), true);
			dimList->set(dim);
		}
	}
	setDimensionList(dimList);
}

// this method is used internally, so no rights checking is done
void Database::addDimension(PServer server, PDimension dimension, bool notify, bool newid, IdentifierType *attributesDim, IdentifierType *attributesCube, IdentifierType *rightsCube, IdentifierType *dimDimElem, bool useDimWorker)
{
	checkCheckedOut();
	const string & name = dimension->getName();
	// check extensible flag
	if (!extensible) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "dimension cannot be added", "dimension", dimension->getName());
	}
	if (lookupDimensionByName(name, false) != 0) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_NAME_IN_USE, "dimension name is already in use", "name", name);
	}
	IdentifierType identifier = dimension->getId();
	if (lookupDimension(identifier, false) != 0) {
		throw ParameterException(ErrorException::ERROR_INTERNAL, "dimension identifier is already in use", "identifier", (int)(identifier));
	}

	if (!dims->isCheckedOut()) {
		dims = COMMITABLE_CAST(DimensionList, dims->copy());
	}
	dims->add(dimension, newid);
	Context::getContext()->saveParent(shared_from_this(), dimension);
	// database has been changed, update token and status
	setStatus(CHANGED);

	if (!newid) {
		IdentifierType last = dims->getLastId();
		IdentifierType id = dimension->getId();
		if (last <= id) {
			dims->setNewIDStart(id + 1);
		}
	}

	// tell dimension that it has been added
	if (notify) {
		dimension->notifyAddDimension(server, COMMITABLE_CAST(Database, shared_from_this()), attributesDim, attributesCube, rightsCube, dimDimElem, useDimWorker);
	}
}

// this method is used internally, so no rights checking is done
void Database::removeDimension(PServer server, PDimension dimension, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	IdentifierType identifier = dimension->getId();
	if (0 == lookupDimension(identifier, false)) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "dimension not found", "dimension", (int)(identifier));
	}
	// is dimension used in a normal cube?
	for (CubeList::Iterator i = cubes->begin(); i != cubes->end(); ++i) {
		PCube cube = COMMITABLE_CAST(Cube, *i);

		if (cube->getType() == NORMALTYPE) {
			const IdentifiersType* dims = cube->getDimensions();

			// dimension is used by cube
			if (find(dims->begin(), dims->end(), dimension->getId()) != dims->end()) {
				throw ParameterException(ErrorException::ERROR_DIMENSION_IN_USE, "dimension is used by a cube", "dimension", (int)identifier);
			}
		}
	}
	// tell dimension that it will be removed
	if (notify) {
		// we have to delete system cubes using the dimension here
		dimension->beforeRemoveDimension(server, COMMITABLE_CAST(Database, shared_from_this()), useDimWorker);
	}
	// remove element from mapping
	if (!dims->isCheckedOut()) {
		dims = COMMITABLE_CAST(DimensionList, dims->copy());
	}
	dims->remove(identifier);
	// database has been changed, update token and status
	setStatus(CHANGED);
	// tell dimension that it has been removed
	if (notify) {
		dimension->notifyRemoveDimension(COMMITABLE_CAST(Database, shared_from_this()));
	}
}

// this method is used internally, so no rights checking is done
void Database::renameDimension(PServer server, PDimension dimension, const string & name, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	// check for double used name
	PDimension dimByName = lookupDimensionByName(name, false);
	if (dimByName != 0) {
		if (dimByName->getId() != dimension->getId()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_NAME_IN_USE, "dimension name is already in use", "name", name);
		}
		if (dimension->getName() == name) {
			// new name = old name
			return;
		}
	}

	// keep old name
	string oldName = dimension->getName();
	if (!dims->isCheckedOut()) {
		dims = COMMITABLE_CAST(DimensionList, dims->copy());
	}
	dims->rename(dimension->getId(), name);
	// database has been changed
	setStatus(CHANGED);
	// tell dimension that it has been renamed
	if (notify) {
		dimension->notifyRenameDimension(server, COMMITABLE_CAST(Database, shared_from_this()), oldName, useDimWorker);
	}
}

PDimension Database::addDimension(PServer server, const string & name, PUser user, bool isUserInfo, bool useDimWorker, bool useJournal)
{
	checkCheckedOut();
	if (isUserInfo) {
		checkDbRoleRight(user, User::userInfoRight, RIGHT_DELETE);
	} else {
		checkDbRoleRight(user, User::dimensionRight, RIGHT_WRITE);
	}
	checkDimensionName(name, isUserInfo);
	// create new dimension
	PDimension dimension;
	if (isUserInfo) {
		dimension = PDimension(new UserInfoDimension(name));
	} else {
		dimension = PDimension(new NormalDimension(name));
	}

	IdentifierType newAttributesDimension = NO_IDENTIFIER;
	IdentifierType newAttributesCube = NO_IDENTIFIER;
	IdentifierType newRightsCube = NO_IDENTIFIER;
	IdentifierType newDimDimElem = NO_IDENTIFIER;
	addDimension(server, dimension, true, true, &newAttributesDimension, &newAttributesCube, &newRightsCube, &newDimDimElem, useDimWorker);
	// log changes to journal is not necessary (see above, we already saved the database)
	// this is kept for tracking the changes to the database
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_DIMENSION_CREATE);
		journal->appendInteger(dimension->getId());
		journal->appendEscapeString(name);
		journal->appendInteger(isUserInfo ? 3 : 0);
		journal->appendInteger(newAttributesDimension);
		journal->appendInteger(newAttributesCube);
		journal->appendInteger(newRightsCube);
		journal->appendInteger(newDimDimElem);
		journal->nextLine();
	}
	// return dimension
	return dimension;
}

// this method is used internally, so no rights checking is done
PDimension Database::addAliasDimension(PServer server, const string & name, PDimension alias, bool useDimWorker)
{
	checkCheckedOut();
	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_DIMENSION_NAME, "dimension name is empty", "name", name);
	}
	// create new dimension
	PDimension dimension = PDimension(new AliasDimension(name, alias->getId(), Context::getContext()->getParent(alias)->getId()));
	// and update dimension structure
	addDimension(server, dimension, true, true, NULL, NULL, NULL, NULL, useDimWorker);
	// return dimension
	return dimension;
}

void Database::deleteDimension(PServer server, PDimension dimension, PUser user, bool useJournal, bool useDimWorker)
{
	checkCheckedOut();
	if (dimension->getType() == USER_INFOTYPE) {
		checkDbRoleRight(user, User::userInfoRight, RIGHT_DELETE);
	} else {
		checkDbRoleRight(user, User::dimensionRight, RIGHT_DELETE);
	}
	// check deletable flag
	if (!dimension->isDeletable()) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNDELETABLE, "dimension cannot be deleted", "dimension", dimension->getName());
	}
	IdentifierType id = dimension->getId();
	// remove dimension from database
	removeDimension(server, dimension, true, useDimWorker);
	// delete dimension from name mapping
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_DIMENSION_DESTROY);
		journal->appendInteger(id);
		journal->nextLine();
	}
}

void Database::renameDimension(PServer server, PDimension dimension, const string & name, PUser user, bool useDimWorker, bool useJournal)
{
	checkCheckedOut();
	if (dimension->getType() == USER_INFOTYPE) {
		checkDbRoleRight(user, User::userInfoRight, RIGHT_WRITE);
	} else {
		checkDbRoleRight(user, User::dimensionRight, RIGHT_WRITE);
	}
	checkName(name);
	// check renamable flag
	if (!dimension->isRenamable()) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNRENAMABLE, "dimension cannot be renamed", "dimension", dimension->getName());
	}
	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_DIMENSION_NAME, "dimension name is empty", "name", name);
	}
	if (dimension->getType() == USER_INFOTYPE) {
		checkDimensionName(name, true);
	} else {
		checkDimensionName(name, false);
	}
	// rename dimension in database
	renameDimension(server, dimension, name, true, useDimWorker);
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_DIMENSION_RENAME);
		journal->appendInteger(dimension->getId());
		journal->appendEscapeString(name);
		journal->nextLine();
	}
}

////////////////////////////////////////////////////////////////////////////////
// functions to administrate dimensions
////////////////////////////////////////////////////////////////////////////////
FileName Database::computeCubeFileName(const FileName & fileName, IdentifierType cubeIdentifier)
{
	return FileName(fileName.path, fileName.name + "_CUBE_" + StringUtils::convertToString(cubeIdentifier), fileName.extension);
}

// this method is used internally, so no rights checking is done; already existing cubes are loaded, new are saved (files created)
void Database::addCube(PServer server, PCube cube, bool notify, bool newid, IdentifierType *newCubeDimElem, IdentifierType *newCellRightCube, IdentifierType *newCellPropsCube, bool useDimWorker)
{
	checkCheckedOut();
	const string & name = cube->getName();
	// check extensible flag
	if (!extensible) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "cube cannot be added", "cube", cube->getName());
	}
	if (lookupCubeByName(name, false) != 0) {
		throw ParameterException(ErrorException::ERROR_CUBE_NAME_IN_USE, "cube name is already in use", "name", name);
	}
	// add cube to mapping
	if (!cubes->isCheckedOut()) {
		cubes = COMMITABLE_CAST(CubeList, cubes->copy());
	}
	cubes->add(cube, newid);
	Context::getContext()->saveParent(shared_from_this(), cube);
	// update filename
	cube->setCubeFileAndLoad(server, COMMITABLE_CAST(Database, shared_from_this()), computeCubeFileName(*fileName, cube->getId()), newid);
	// database has been changed, update token and status
	setStatus(CHANGED);

	if (!newid) {
		IdentifierType last = cubes->getLastId();
		IdentifierType idCube = cube->getId();
		if (last <= idCube) {
			cubes->setNewIDStart(idCube + 1);
		}
	}

	// tell cube that it has been added
	if (notify) {
		cube->notifyAddCube(server, COMMITABLE_CAST(Database, shared_from_this()), newCubeDimElem, newCellRightCube, newCellPropsCube, useDimWorker);
	}
}

// this method is used internally, so no rights checking is done
void Database::removeCube(PServer server, PCube cube, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	IdentifierType id = cube->getId();
	// check that cube is cube of database
	if (0 == lookupCube(id, false)) {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube not found", "cube", (int)(id));
	}
	Context::getContext()->addCubeToDelete(cube);
	if (!cubes->isCheckedOut()) {
		cubes = COMMITABLE_CAST(CubeList, cubes->copy());
	}
	PDatabase database = COMMITABLE_CAST(Database, shared_from_this());
	vector<PRule> rules = cube->getRules(PUser(), false);
	for (vector<PRule>::const_iterator ruleIt = rules.begin(); ruleIt != rules.end(); ++ruleIt) {
		cube->deleteRule(server, database, (*ruleIt)->getId(), PUser(), true);
	}

	cubes->remove(cube->getId());
	// delete storage from all engines here
	IdentifiersType storages;
	storages.push_back(cube->getNumericStorageId());
	storages.push_back(cube->getStringStorageId());
	storages.push_back(cube->getMarkerStorageId());
	server->removeStorages(storages);

	// database has been changed, update token and status
	setStatus(CHANGED);
	// tell cube that it has been removed
	if (notify) {
		cube->notifyRemoveCube(server, database, useDimWorker);
	}
}

// this method is used internally, so no rights checking is done
void Database::renameCube(PServer server, PCube cube, const string & name, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	checkName(name);
	// check for double used name
	PCube cubeByName = lookupCubeByName(name, false);
	if (cubeByName != 0) {
		if (cubeByName->getId() != cube->getId()) {
			throw ParameterException(ErrorException::ERROR_CUBE_NAME_IN_USE, "cube name is already in use", "name", name);
		}
		if (cube->getName() == name) {
			// new name == old name
			return;
		}
	}

	string oldName = cube->getName();

	if (!cubes->isCheckedOut()) {
		cubes = COMMITABLE_CAST(CubeList, cubes->copy());
	}
	cubes->rename(cube->getId(), name);
	// database has been changed
	setStatus(CHANGED);
	// tell cube that it has been renamed
	if (notify) {
		cube->notifyRenameCube(server, COMMITABLE_CAST(Database, shared_from_this()), oldName, useDimWorker);
	}
}

void Database::renameCube(PServer server, const string& oldName, const string& newName, bool useDimWorker)
{
	PCube cube = lookupCubeByName(oldName, true);

	PCubeList cubes = getCubeList(true);
	setCubeList(cubes);
	cubes->set(cube);

	if (cube) {
		renameCube(server, cube, newName, false, useDimWorker);
	}
}

PCube Database::addCube(PServer server, const string & name, IdentifiersType *dimensions, PUser user, bool isUserInfo, bool useDimWorker, bool useJournal)
{
	checkCheckedOut();
	if (isUserInfo) {
		checkDbRoleRight(user, User::userInfoRight, RIGHT_DELETE);
	} else {
		checkDbRoleRight(user, User::cubeRight, RIGHT_WRITE);
	}
	checkCubeName(name, isUserInfo);
	// create new cube and add cube to cube vector
	PCube cube;
	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());
	if (isUserInfo) {
		cube = PCube(new UserInfoCube(db, name, dimensions));
	} else {
		// NOTE: gpu cube cannot be create here
		cube = PCube(new Cube(db, name, dimensions, Cube::NORMAL));
	}

	IdentifierType newCubeDimElem = NO_IDENTIFIER;
	IdentifierType newCellRightCube = NO_IDENTIFIER;
	IdentifierType newCellPropsCube = NO_IDENTIFIER;
	addCube(server, cube, true, true, &newCubeDimElem, &newCellRightCube, &newCellPropsCube, useDimWorker);
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_CREATE);
		journal->appendInteger(cube->getId());
		journal->appendEscapeString(name);
		IdentifiersType identifiers;
		for (IdentifiersType::iterator i = dimensions->begin(); i != dimensions->end(); ++i) {
			identifiers.push_back(*i);
		}
		journal->appendIdentifiers(identifiers.begin(), identifiers.end());
		if (isUserInfo) {
			journal->appendInteger(3);
		} else {
			journal->appendInteger(0);
		}
		journal->appendIdentifier(newCubeDimElem);
		journal->appendIdentifier(newCellRightCube);
		journal->appendIdentifier(newCellPropsCube);
		journal->nextLine();
	}

	// and return the cube
	return cube;
}

void Database::deleteCube(PServer server, PCube cube, PUser user, bool useJournal, bool useDimWorker)
{
	checkCheckedOut();
	if (cube->getType() == USER_INFOTYPE) {
		checkDbRoleRight(user, User::userRight, RIGHT_DELETE);
	} else {
		checkDbRoleRight(user, User::cubeRight, RIGHT_DELETE);
	}
	// check deletable flag
	if (!cube->isDeletable()) {
		throw ParameterException(ErrorException::ERROR_CUBE_UNDELETABLE, "cube cannot be deleted", "cube", cube->getName());
	}
	if (status == UNLOADED) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "cannot delete cube of unloaded database", "database", getName());
	}
	// remove cube
	removeCube(server, cube, true, useDimWorker);
	// delete it completely
	IdentifierType id = cube->getId();
	// log changes to journal
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_DESTROY);
		journal->appendInteger(id);
		journal->nextLine();
	}
}

void Database::deleteCubeByName(PServer server, const string& name, bool useDimWorker)
{
	PCube cube = lookupCubeByName(name, true);

	if (cube) {
		cube->setDeletable(true);
		deleteCube(server, cube, PUser(), false, useDimWorker);
	}
}

void Database::renameCube(PServer server, PCube cube, const string & name, PUser user, bool useDimWorker, bool useJournal)
{
	checkCheckedOut();
	if (cube->getType() == USER_INFOTYPE) {
		checkDbRoleRight(user, User::userRight, RIGHT_WRITE);
	} else {
		checkDbRoleRight(user, User::cubeRight, RIGHT_WRITE);
	}
	// check renamable flag
	if (!cube->isRenamable()) {
		throw ParameterException(ErrorException::ERROR_CUBE_UNRENAMABLE, "cube cannot be renamed", "cube", cube->getName());
	}
	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_CUBE_NAME, "cube name is empty", "name", name);
	}
	if (cube->getType() == USER_INFOTYPE) {
		checkCubeName(name, true);
	} else {
		checkCubeName(name, false);
	}
	// rename cube
	renameCube(server, cube, name, true, useDimWorker);
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_RENAME);
		journal->appendInteger(cube->getId());
		journal->appendEscapeString(name);
		journal->nextLine();
	}
}

void Database::loadCube(PServer server, PCube cube, PUser user)
{
	checkCheckedOut();
	checkDbRoleRight(user, User::sysOpRight, RIGHT_DELETE);
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database file name not set");
	}
	if (status == UNLOADED) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "cannot load cube of unloaded database", "fileName", fileName->fullPath());
	}
	if (!cubes->isCheckedOut()) {
		cubes = COMMITABLE_CAST(CubeList, cubes->copy());
	}
	cube->loadCube(server, COMMITABLE_CAST(Database, shared_from_this()), true);
	cubes->set(cube);
}

void Database::saveCube(PServer server, PCube cube, PUser user)
{
	checkDbRoleRight(user, User::sysOpRight, RIGHT_WRITE);
	if (!isLoadable()) {
		throw ParameterException(ErrorException::ERROR_DATABASE_UNSAVED, "cannot save cube of unsaved database", "database", getName());
	}
	cube->saveCube(server, COMMITABLE_CAST(Database, shared_from_this()));
}

void Database::unloadCube(PServer server, PCube cube, PUser user)
{
	checkCheckedOut();
	checkDbRoleRight(user, User::sysOpRight, RIGHT_DELETE);
	if (status == UNLOADED) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "cannot unload cube of an unloaded database", "database", getName());
	}
	if (getType() == NORMALTYPE && (cube->getType() == GPUTYPE || cube->getType() == NORMALTYPE || cube->getType() == USER_INFOTYPE)) {
		if (!cubes->isCheckedOut()) {
			cubes = COMMITABLE_CAST(CubeList, cubes->copy());
		}
		cube->unloadCube(server, COMMITABLE_CAST(Database, shared_from_this()));
		cubes->set(cube);
	} else {
		throw ParameterException(ErrorException::ERROR_CUBE_IS_SYSTEM_CUBE, "cannot unload system or attribute cubes", "cube", cube->getName());
	}
}

////////////////////////////////////////////////////////////////////////////////
// other stuff
////////////////////////////////////////////////////////////////////////////////
void Database::checkName(const string & name) const
{
	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "name is empty", "name", name);
	}
	if (name[0] == ' ' || name[name.length() - 1] == ' ') {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "name begins or ends with a space character", "name", name);
	}
	if (name[0] == '.') {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "name begins with a dot character", "name", name);
	}
	for (size_t i = 0; i < name.length(); i++) {
		if (0 <= name[i] && name[i] < 32) {
			throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "name contains an illegal character", "name", name);
		}
	}

	if (name.find_first_of(INVALID_CHARACTERS) != string::npos) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "name contains an illegal character", "name", name);
	}
}

void Database::checkDimensionName(const string & name, bool isInfo) const
{
	try {
		checkName(name);
	} catch (ParameterException&) {
		throw ParameterException(ErrorException::ERROR_INVALID_DIMENSION_NAME, "invalid dimension name", "name", name);
	}
	if (isInfo) {
		if (name.length() < 3 || name.substr(0, 2) != "##") {
			throw ParameterException(ErrorException::ERROR_INVALID_DIMENSION_NAME, "invalid name for user info object", "name", name);
		}
	}

}

void Database::checkCubeName(const string & name, bool isInfo) const
{
	try {
		checkName(name);
	} catch (ParameterException&) {
		throw ParameterException(ErrorException::ERROR_INVALID_CUBE_NAME, "invalid cube name", "name", name);
	}
	if (isInfo) {
		if (name.length() < 3 || name.substr(0, 2) != "##") {
			throw ParameterException(ErrorException::ERROR_INVALID_CUBE_NAME, "invalid name for user info object", "name", name);
		}
	}

}

PCube Database::changeCubeType(PServer server, PCube cube, ItemType type, PUser user, bool useJournal)
{
	checkCheckedOut();

	if (!(cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE)) {
		throw ParameterException(ErrorException::ERROR_CUBE_UNDELETABLE, "cube type cannot be converted", "cube", cube->getName());
	} else {
		checkDbRoleRight(user, User::cubeRight, RIGHT_DELETE);
	}

	// check deletable flag
	if (!cube->isDeletable()) {
		throw ParameterException(ErrorException::ERROR_CUBE_UNDELETABLE, "cube type cannot be changed", "cube", cube->getName());
	}

	if (status == UNLOADED) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_LOADED, "cannot change cube type of unloaded database", "database", getName());
	}

	PEngineBase gpuEngine = server->getEngine(EngineBase::GPU, true);
	PEngineBase cpuEngine = server->getEngine(EngineBase::CPU, false);

	if (!gpuEngine) {
		throw ErrorException(ErrorException::ERROR_GPU_SERVER_NOT_ENABLED, "cube convert is not possible");
	}

	// database has been changed, update token and status
	setStatus(CHANGED);

	IdentifierType numericStorageId = cube->getNumericStorageId();
	if (numericStorageId != NO_IDENTIFIER) {
		PStorageBase numericCpuStorage = cpuEngine->getStorage(numericStorageId);
		PStorageBase numericGpuStorage = gpuEngine->getStorage(numericStorageId);
		if (numericGpuStorage) {
			// delete storage in gpu engine
			gpuEngine->deleteStorage(numericStorageId);
			Logger::info << "Gpu acceleration deactivated for cube with id: " << cube->getId() << "; name: " << cube->getName() << endl;
		}
		if (type == GPUTYPE) {
			// -jj- hotfix for 3.3 -> cube using alias dimension cannot be converted
			const IdentifiersType &cubeDimensionIds = *cube->getDimensions();
			for (uint32_t dimIdx = 0; dimIdx < cubeDimensionIds.size(); dimIdx++) {
				PDimension dim = lookupDimension(cubeDimensionIds[dimIdx], false);
				if (!dim || dim->getDimensionType() == Dimension::ALIAS) {
					throw ParameterException(ErrorException::ERROR_INVALID_TYPE, "cube with alias dimension cannot be converted", "cube", cube->getName());
				}
			}
            
			// -jj- hotfix for 3.3 -> PathTranslator is synchronized with current sizes of dimensions
            PCubeList cubelist = getCubeList(true);
            setCubeList(cubelist);
            Context*context = Context::getContext();
            PCube cubecopy = COMMITABLE_CAST(Cube, cubelist->get(cube->getId(), true));
            PDatabase database = COMMITABLE_CAST(Database, shared_from_this());
            context->saveParent(database, cubecopy);
            cubecopy->updatePathTranslator(database);
            cubelist->set(cubecopy);

			// create storage in gpu engine
			numericGpuStorage = gpuEngine->getCreateStorage(numericStorageId, cubecopy->getPathTranslator(), EngineBase::Numeric);
			if (numericGpuStorage) {
				// load numeric storage
				numericGpuStorage->setCellValue(numericCpuStorage->getCellValues(CPArea()));
			}
            cube = cubecopy;
            Logger::info << "Gpu acceleration activated for cube with id: " << cube->getId() << "; name: " << cube->getName() << endl;
		}
	} else {
		// pure string cube, no error, no conversion
	}
	cube->setType(type);

	// log changes to journal
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_CONVERT);
		journal->appendInteger(cube->getId());
		journal->appendInteger((int)type);
		journal->nextLine();
	}
	return cube;
}

void Database::deleteGpuStorage(PServer server, CPCube cube)
{
	checkCheckedOut();

	PEngineBase gpuEngine = server->getEngine(EngineBase::GPU, true);

	if (!gpuEngine) {
		throw ErrorException(ErrorException::ERROR_GPU_SERVER_NOT_ENABLED, "delete gpu storage is not possible");
	}

	IdentifierType numericStorageId = cube->getNumericStorageId();
	if (numericStorageId != NO_IDENTIFIER) {
		PStorageBase numericGpuStorage = gpuEngine->getStorage(numericStorageId);
		if (numericGpuStorage) {
			// delete storage in gpu engine
			gpuEngine->deleteStorage(numericStorageId);
			Logger::info << "Gpu acceleration was deactivated for cube: " << cube->getId() << ":" << cube->getName() << endl;
		}
	}
	// database has been changed, update token and status
	setStatus(CHANGED);
}

void Database::setCubeList(PCubeList l)
{
	checkCheckedOut();
	cubes = l;
}

void Database::setDimensionList(PDimensionList l)
{
	checkCheckedOut();
	dims = l;
}

ElementOld2NewMap *Database::getDimensionMap(IdentifierType dimId)
{
	ElementOld2NewMap *result = 0;
	if (old2NewMap) {
		DimensionOld2NewMap::iterator dimMap = old2NewMap->find(dimId);
		if (dimMap != old2NewMap->end()) {
			result = &dimMap->second;
		}
	}
	return result;
}

DatabaseList::DatabaseList(const DatabaseList& l) :
		CommitableList(l)
{
}

PCommitableList DatabaseList::createnew(const CommitableList& l) const
{
	return PCommitableList(new DatabaseList(dynamic_cast<const DatabaseList&>(l)));
}

bool Database::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);
	CPDatabase db = CONST_COMMITABLE_CAST(Database, o);
	CPDatabase olddb = CONST_COMMITABLE_CAST(Database, old);
	if (o != 0 && Context::getContext()->doTokenUpdate()) {
		token = db->token + 1;
		if (!token) {
			++token;
		}
	}
	if (old != 0) {
		if (db != 0) {
			if (status == olddb->status) {
				status = db->status;
			}
			if (deletable == olddb->deletable) {
				deletable = db->deletable;
			}
			if (renamable == olddb->renamable) {
				renamable = db->renamable;
			}
			if (extensible == olddb->extensible) {
				extensible = db->extensible;
			}
			if (cacheType == olddb->cacheType) {
				cacheType = db->cacheType;
			}
			if (hideElements == olddb->hideElements) {
				hideElements = db->hideElements;
			}
			if (defaultRight == olddb->defaultRight) {
				defaultRight = db->defaultRight;
			}
		}
	}

	if (dims->isCheckedOut()) {
		ret = dims->merge(o != 0 ? db->dims : PDimensionList(), shared_from_this());
	} else if (o != 0) {
		dims = db->dims;
	}

	if (ret) {
		if (cubes->isCheckedOut()) {
			ret = cubes->merge(o != 0 ? db->cubes : PCubeList(), shared_from_this());
		} else if (o != 0) {
			cubes = db->cubes;
		}
	}

	if (ret) {
		commitintern();
		if (Context::getContext()->doTokenUpdate() && getType() != SYSTEMTYPE) {
			User::updateGlobalDatabaseToken(COMMITABLE_CAST(Server, p), COMMITABLE_CAST(Database, shared_from_this()));
		}
	}
	return ret;
}

}
