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
 * \author Alexander Haberstroh, Jedox AG, Freiburg, Germany
 * 
 *
 */

//#define SAVE_PLANS_PATH "/home/QBICON/jjunek/temp/"

#if defined(_MSC_VER)
#pragma warning( disable : 4267 )
#endif

#include "Olap/Cube.h"

#include <math.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if defined(_MSC_VER)
#include <float.h>
#include <limits>
#endif

#include <boost/scoped_array.hpp>

#include "Exceptions/ParameterException.h"
#include "Exceptions/FileFormatException.h"
#include "Exceptions/FileOpenException.h"

#include "InputOutput/Condition.h"
#include "InputOutput/FileReader.h"
#include "InputOutput/FileWriter.h"
#include "InputOutput/FileUtils.h"
#include "InputOutput/Statistics.h"
#include "Collections/CellSet.h"
#include "Parser/RuleParserDriver.h"
#include "Olap/AttributesCube.h"
#include "Olap/NormalDatabase.h"
#include "Olap/UserInfoDatabase.h"
#include "Olap/Lock.h"
#include "Olap/PaloSession.h"
#include "Olap/RightsCube.h"
#include "Olap/RollbackStorage.h"
#include "Olap/Rule.h"
#include "Olap/RuleMarker.h"
#include "Olap/Server.h"
#include "Olap/SubsetViewCube.h"
#include "Olap/UserInfoCube.h"
#include "Olap/GoalSeekSolver.h"
#include "Olap/MarkerStorage.h"
#include "Olap/SystemDimension.h"
#include "Olap/ServerLogCube.h"
#include "Olap/SessionInfoCube.h"
#include "Olap/JobInfoCube.h"
#include "Olap/LicenseInfoCube.h"
#include "PaloJobs/AreaJob.h"
#include "Engine/CubeFileStream.h"
#include "Engine/Planner.h"
#include "Engine/StorageCpu.h"
#ifdef ENABLE_GPU_SERVER
	#include "EngineGpu/GpuStorage.h"
#endif

namespace palo {

double Cube::splashLimit1 = 1000.0; // error
double Cube::splashLimit2 = 500.0; // warning
double Cube::splashLimit3 = 100.0; // info

int Cube::goalseekCellLimit = 1000;
int Cube::goalseekTimeoutMiliSec = 10000;

bool Cube::ignoreCellData = false; // for DEBUGGING only

double Cube::cacheBarrier = 1000000.0;

const string Cube::PREFIX_ATTRIBUTE_CUBE = "#_";
const uint32_t Cube::CUBE_FILE_VERSION = 2;
const string Cube::CSV = "csv";
const string Cube::BIN = "bin";
const string Cube::CSVTMP = "tmp";
const string Cube::BINTMP = "btmp";

const string Cube::NUMERIC_SECTION = "NUMERIC";
const string Cube::STRING_SECTION = "STRING";
const string Cube::ALIAS_SECTION = "ALIAS";
const string Cube::GROUP_SECTION = "GROUP"; // until 5.0 SR1

const int Cube::littleEndian = 1;
const int Cube::bigEndian = 2;

const uint32_t Cube::maxNewMarkerCount = 1000000;
const uint32_t Cube::markerRebuildLimit = 100000;

bool Cube::saveCSV = true;


bool Cube::ltMarker::operator()(const PRuleMarker &m1, const PRuleMarker &m2) const
{
	return m1->getId() < m2->getId();
}

void Cube::setCacheBarrier(double barrier)
{
	cacheBarrier = barrier;
}

double Cube::getCacheBarrier()
{
	return cacheBarrier;
}

void Cube::setGoalseekCellLimit(int cells)
{
	goalseekCellLimit = cells;
}

void Cube::setGoalseekTimeout(int ms)
{
	goalseekTimeoutMiliSec = ms;
}

void Cube::setSaveCSV(bool save)
{
	saveCSV = save;
}

////////////////////////////////////////////////////////////////////////////////
// constructors and destructors
////////////////////////////////////////////////////////////////////////////////
Cube::Cube(PDatabase db, const string& name, const IdentifiersType *dimensions, Cube::SaveType saveType) :
	Commitable(name), token(rand()), clientCacheToken(new IdHolder), dimensions(*dimensions), rules(new RuleList()), deletable(true),
	renamable(true), numericStorageId(NO_IDENTIFIER), stringStorageId(NO_IDENTIFIER), markerStorageId(NO_IDENTIFIER), locks(new LockList()),
	filelock(new PaloSharedMutex), rulefilelock(new PaloSharedMutex), saveType(saveType), wholeCubeLocked(false),
	cache(dimensions->size(), cacheBarrier, 0), additiveCommit(false), delCount(0)
{
	if (!token) {
		++token;
	}
	cubeWorker.reset();

	if (dimensions->empty()) {
		throw ParameterException(ErrorException::ERROR_CUBE_EMPTY, "missing dimensions", "dimensions", "");
	}

	cellsStatus = CHANGED;
	rulesStatus = CHANGED;

	hasArea = false;

	hasLock = false;

	clientCacheToken->setStart(rand());

	IdentifiersType dimSizes = getDimensionSizes(db);
	pathTranslator = PPathTranslator(new PathTranslator(dimSizes));
	if (Logger::isTrace() && saveType == Cube::GPU)
		Logger::trace << "Loading cube '" << name << "' with " << pathTranslator->getNumberOfBins() << " bins to GPU." << endl;
	PServer server = Context::getContext()->getServerCopy();
	PEngineBase cpuEngine = server->getEngine(EngineBase::CPU, true);
	cpuEngine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	cpuEngine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
	cpuEngine->getCreateStorage(markerStorageId, pathTranslator, EngineBase::Marker);
}

Cube::Cube(const Cube& c) :
	Commitable(c), token(c.token), clientCacheToken(c.clientCacheToken), dimensions(c.dimensions), rules(c.rules), deletable(c.deletable),
	renamable(c.renamable), cubeWorker(c.cubeWorker), hasArea(c.hasArea), workerAreaIdentifiers(c.workerAreaIdentifiers), workerAreas(c.workerAreas),
	numericStorageId(c.numericStorageId), stringStorageId(c.stringStorageId), markerStorageId(c.markerStorageId), cellsStatus(c.cellsStatus),
	rulesStatus(c.rulesStatus), fileName(c.fileName), ruleFileName(c.ruleFileName), journalFile(c.journalFile), journal(Server::ignoreJournal || !journalFile ? 0 : new JournalMem(journalFile.get())), hasLock(c.hasLock), locks(c.locks),
	fromMarkers(c.fromMarkers), toMarkers(c.toMarkers), filelock(c.filelock), rulefilelock(c.rulefilelock), saveType(c.saveType),
	wholeCubeLocked(c.wholeCubeLocked), pathTranslator(c.pathTranslator), cache(c.dimensions.size(), cacheBarrier, c.getCache()->getGeneration()),
	additiveCommit(c.additiveCommit), delCount(c.delCount)
{
}

Cube::~Cube()
{
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a cube
////////////////////////////////////////////////////////////////////////////////

bool Cube::isLoadable() const
{
	return fileName == 0 ? false : (FileUtils::isReadable(FileName(*fileName, BIN)) || FileUtils::isReadable(FileName(*fileName, BINTMP)) || FileUtils::isReadable(FileName(*fileName, CSV)) || FileUtils::isReadable(FileName(*fileName, CSVTMP)));
}

PCube Cube::loadCubeFromType(FileReader* file, PDatabase database, IdentifierType identifier, const string& name, const IdentifiersType *dimensions, uint32_t type)
{
	if (type == Cube::GPU && !Context::getContext()->getServer()->isEnableGpu()) {
		Logger::warning << "cube '" << name << "' is a GpuCube, but PaloGPU is disabled - loading as NormalCube..." << endl;
		type = Cube::NORMAL;
	}

	switch (type) {
	case Cube::RIGHTS:
		return PCube(new RightsCube(database, name, dimensions));

	case Cube::NORMAL:
		return PCube(new Cube(database, name, dimensions, Cube::NORMAL));

	case Cube::GPU:
		return PCube(new Cube(database, name, dimensions, Cube::GPU));

	case Cube::ATTRIBUTES:
		if (!name.compare(0, SystemCube::PREFIX_CELL_PROPS_DATA.size(), SystemCube::PREFIX_CELL_PROPS_DATA)) {
			return PCube(new AttributesCube(database, name, dimensions, false));
		} else {
			return PCube(new AttributesCube(database, name, dimensions, true));
		}

	case Cube::CONFIGURATION:
		return PCube(new ConfigurationCube(database, name, dimensions));

	case Cube::SUBSETVIEW:
		return PCube(new SubsetViewCube(database, name, dimensions));

	case Cube::USERINFO:
		return PCube(new UserInfoCube(database, name, dimensions));

	case Cube::LOG:
		return PCube(new ServerLogCube(database, name, dimensions));

	case Cube::SESSIONS:
		return PCube(new SessionInfoCube(database, name, dimensions));

	case Cube::JOBS:
		return PCube(new JobInfoCube(database, name, dimensions));

	case Cube::LICENSES:
		return PCube(new LicenseInfoCube(database, name, dimensions));

	default:
		Logger::error << "cube '" << name << "' has unknown type '" << type << "'" << endl;
		throw FileFormatException("unknown cube type", file);
	}
}

bool Cube::loadCubeOverview(FileReader *file, timeval &tv, uint32_t &file_version, uint32_t &endianness)
{
	bool ret = true;
	if (file->isSectionLine() && file->getSection() == "CUBE") {
		file->nextLine();
		if (file->isDataLine()) {
			long seconds;
			long useconds;
			file->getTimeStamp(&seconds, &useconds, 0);
			tv.tv_sec = seconds;
			tv.tv_usec = useconds;

			file_version = file->getDataInteger(1);
			endianness = file->getDataInteger(2);
			file->nextLine();
		} else {
			ret = false;
		}
	} else {
		ret = false;
	}
	return ret;
}

void Cube::loadCubeOverview(FileReader *file, timeval &tv)
{
	vector<int> sizes;

	// last save time
	long seconds;
	long useconds;

	if (file->isSectionLine() && file->getSection() == "CUBE") {
		file->nextLine();

		while (file->isDataLine()) {
			file->getTimeStamp(&seconds, &useconds, 0);
			tv.tv_sec = seconds;
			tv.tv_usec = useconds;

			sizes = file->getDataIntegers(1);

			file->nextLine();
		}
	} else {
		throw FileFormatException("Section 'CUBE' not found.", file);
	}
}

bool Cube::loadCubeCells(FileReader *fr, PDatabase db, bool checkAlias, bool binary, bool &diffAlias, uint32_t fileVersion)
{
	checkCheckedOut();

	vector<Id2IdMap* > *aliasMaps = 0;
	vector<Id2IdMap* > mapping;

	bool ret = true;
	diffAlias = false;

	if (checkAlias) {
		mapping.resize(dimensions.size());
		checkAlias = false; // nothing found yet

		for (size_t i = 0; i < dimensions.size(); i++) {
			CPDimension dim = db->lookupDimension(dimensions[i], false);
			if (dim->getDimensionType() == Dimension::ALIAS) {
				mapping[i] = new Id2IdMap();
				checkAlias = true;
			} else {
				mapping[i] = NULL;
			}
		}
	}

	// load mapping table for aliases
	if (checkAlias) {
		aliasMaps = &mapping;

		for (size_t i = 0; i < dimensions.size(); i++) {
			if (!mapping[i]) {
				continue; // not alias dimension
			}

			CPDimension dim = db->lookupDimension(dimensions[i], false);

			if (fr->isSectionLine() && (fr->getSection() == GROUP_SECTION || fr->getSection() == ALIAS_SECTION)) {
				if (fr->getSection() == ALIAS_SECTION) {
					fr->nextLine();
					IdentifierType id = fr->getDataInteger(0);
					if (id != dim->getId()) {
						throw FileFormatException("wrong dimension in alias section found", fr);
					}
				}
				fr->nextLine();

				while (fr->isDataLine()) {
					IdentifierType id = fr->getDataInteger(0);
					string name = fr->getDataString(1);
					Element *aliasElement = dim->lookupElementByName(name, false);

					if (binary) {
						if (!aliasElement || id != aliasElement->getIdentifier()) {
							ret = false; //different alias element -> load csv file due to mapping
							diffAlias = true;
							break;
						}
					} else {
						if (aliasElement != 0) {
							(*mapping[i])[id] = aliasElement->getIdentifier();
						}
					}

					fr->nextLine();
				}
			} else {
				ElementsType elements = dim->getElements(PUser(), false);

				for (ElementsType::iterator iter = elements.begin(); iter != elements.end(); ++iter) {
					Element *element = *iter;

					if (!element) {
						ret = false; //unknown group element -> load csv file due to mapping
						diffAlias = true;
						break;
					} else {
						IdentifierType id = element->getIdentifier();
						(*mapping[i])[id] = id;
					}
				}
			}
		}
	}

	if (ret && fr->isSectionLine() && fr->getSection() == GROUP_SECTION) {
		// old versions had GROUP section also in System DB, not needed
		fr->nextLine();
		while (fr->isDataLine()) {
			fr->nextLine();
		}
	}

	if (ret) {
		PServer server = Context::getContext()->getServerCopy();
		PEngineBase cpuEngine = server->getEngine(EngineBase::CPU, true);
		if (binary) {
			StorageCpu *st;
			if (fr->isSectionLine() && fr->getSection() == NUMERIC_SECTION) {
				PStorageBase numericCpuStorage = cpuEngine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
				st = dynamic_cast<StorageCpu *>(numericCpuStorage.get());
				st->load(fr, fileVersion);
				fr->nextLine();

				if (getType() == GPUTYPE) {
					PEngineBase gpuEngine = server->getEngine(EngineBase::GPU, true);
					PStorageBase numericGpuStorage = gpuEngine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
					numericGpuStorage->setCellValue(st->getCellValues(CPArea()));
				}
			}
			if (ret && fr->isSectionLine() && fr->getSection() == STRING_SECTION) {
				PStorageBase stringStorage = cpuEngine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
				st = dynamic_cast<StorageCpu *>(stringStorage.get());
				st->load(fr, fileVersion);
			} else {
				ret = false;
			}
		} else {
			// get primary engine (CPU)
			PEngineBase gpuEngine;

			if (getType() == GPUTYPE) {
				gpuEngine = server->getEngine(EngineBase::GPU, true);
			}

			bool numFound = aliasMaps ? true : false; // group cubes doesn't have numeric section in old versions
			bool strFound = false;
			do {
				if (fr->isSectionLine()) {
					if (fr->getSection() == NUMERIC_SECTION) {
						numFound = true;
						// obtain and store storage Id for numbers
						PStorageBase numericStorage;
						PStorageBase numericCpuStorage = cpuEngine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
						PStorageBase numericGpuStorage;
						if (gpuEngine) {
							numericGpuStorage = gpuEngine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
							if (numericGpuStorage) {
								numericStorage = numericGpuStorage;
							}
						}
						if (!numericStorage) {
							numericStorage = numericCpuStorage;
						}
						// load numeric storage
						numericStorage->setCellValue(PCellStream(new CubeFileStream(db, COMMITABLE_CAST(Cube, shared_from_this()), fr, false, aliasMaps)));

						if (numericGpuStorage) {
							// synchronize CPU storage with GPU
							numericCpuStorage->setCellValue(numericGpuStorage->getCellValues(CPArea()));
						}

		//				// print numbers
		//				cout << "Loaded numeric values db: " << db->getName() << " cube: " << getId() << endl;
		//				PCellStream cells = numericStorage->getCellValues(CPArea(), 0);
		//				while (cells->next()) {
		//BIN//				GpuBinPath key = cells->getBinKey();
		//BIN//				for (GpuBinPath::const_iterator it = key.begin(); it != key.end(); ++it) {
		//					IdentifiersType key = cells->getKey();
		//					for (IdentifiersType::const_iterator it = key.begin(); it != key.end(); ++it) {
		//						cout << *it << '\t';
		//					}
		//					cout << cells->getValue().toString() << endl;
		//				}

						continue;
					} else if (fr->getSection() == STRING_SECTION) {
						strFound = true;
						// obtain and store storage Id for strings
						PStorageBase stringStorage = cpuEngine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
						// load string storage
						stringStorage->setCellValue(PCellStream(new CubeFileStream(db, COMMITABLE_CAST(Cube, shared_from_this()), fr, true, aliasMaps)));

		//				// print strings
		//				cout << "Loaded string values db: " << db->getName() << " cube: " << getId() << endl;
		//				PCellStream cells = stringStorage->getCellValues(CPArea(), 0);
		//				while (cells->next()) {
		//					IdentifiersType key = cells->getKey();
		//					for (IdentifiersType::const_iterator it = key.begin(); it != key.end(); ++it) {
		//						cout << *it << '\t';
		//					}
		//					cout << cells->getValue().toString() << endl;
		//				}
						continue;
					}
				}
				break;
			}  while (true);

			if (!numFound && this->getCubeType() != RIGHTS) { // old system cubes do not have NUMERIC section
				throw FileFormatException("section " + NUMERIC_SECTION + " not found", fr);
			} else if (!strFound) {
				throw FileFormatException("section " + STRING_SECTION + " not found", fr);
			}
		}
	}

	if (ret && !binary) {
		cellsStatus = CHANGED;
	}
	return ret;
}

void Cube::processJournalCommand(PServer server, PDatabase db, CPCube thisCube, JournalFileReader &history, InBulkEnum &replaceBulkState, set<PCube> &changedCubes)
{
	string username = history.getDataString(1);
	string event = history.getDataString(2);
	string command = history.getDataString(3);

	if (command == JournalFileReader::JOURNAL_VERSION) {
		int release = history.getDataInteger(4);
		int sr = history.getDataInteger(5);
		int build = history.getDataInteger(7);
		history.setVersion(release, sr, build);
	} else if (history.getVersion().isUnknown()) {
		throw ErrorException(ErrorException::ERROR_INVALID_VERSION, "cube " + StringUtils::convertToString(getId()) + " has nonempty journal file from old version");
	} else if (command == JournalFileReader::JOURNAL_RULE_MOVE) {
		IdentifierType id = history.getDataInteger(4);
		double position = history.getDataDouble(5);
		PRule rule = findRule(id);
		setRulesPosition(server, db, vector<PRule>(1, rule), position, 0, PUser(), false);
	} else if (history.getVersion().isOld()) {
		// MOVE_RULE only above is allowed also in old versions due to a bug fixed in 5720
		throw ErrorException(ErrorException::ERROR_INVALID_VERSION, "cube " + StringUtils::convertToString(getId()) + " has nonempty journal file from old version");
	} else if (command == JournalFileReader::JOURNAL_CELL_REPLACE_BULK_START) {
		replaceBulkState = Cube::First;
	} else if (command == JournalFileReader::JOURNAL_CELL_REPLACE_BULK_STOP) {
		replaceBulkState = Cube::Not;
	} else if (command == JournalFileReader::JOURNAL_CELL_REPLACE_DOUBLE) {
		IdentifiersType ids = history.getDataIdentifiers(4);
		SplashMode splashMode = (SplashMode)history.getDataInteger(5);
		double value = history.getDataDouble(6);
		bool addValue = history.getDataBool(7, false);
		PPaths lockedPaths = history.getDataPaths(8);
		PLockedCells lockedCells;
		if (lockedPaths && !lockedPaths->empty()) {
			lockedCells = PLockedCells(new LockedCells(db, thisCube, lockedPaths));
		}

		try {
			PCubeArea cellPath(new CubeArea(db, thisCube, ids));
			CubeArea::CellType cellType = cellPath->getType(cellPath->pathBegin());
			if (cellType == CubeArea::CONSOLIDATED && replaceBulkState != Cube::In) {
				commitChanges(false, PUser(), changedCubes, false); // commit previous changes, this command reads data
			}
			if (replaceBulkState == Cube::First) {
				replaceBulkState = Cube::In;
			}

			setCellValue(server, db, cellPath, value, lockedCells, PUser(), boost::shared_ptr<PaloSession>(), false, addValue, splashMode, false, 0, changedCubes, true, cellType);
		} catch (ErrorException &e) {
			Logger::debug << "journal file command: " << JournalFileReader::JOURNAL_CELL_REPLACE_DOUBLE << " - " << e.getMessage() << endl;
		}
	} else if (command == JournalFileReader::JOURNAL_CELL_REPLACE_STRING) {
		IdentifiersType ids = history.getDataIdentifiers(4);
		string value = history.getDataString(5);

		try {
			PCubeArea cellPath(new CubeArea(db, thisCube, ids));

			setCellValue(server, db, cellPath, value, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
		} catch (ErrorException &e) {
			Logger::debug << "journal file command: " << JournalFileReader::JOURNAL_CELL_REPLACE_STRING << " - " << e.getMessage() << endl;
		}

	} else if (command == JournalFileReader::JOURNAL_CELL_CLEAR) {
		IdentifiersType ids = history.getDataIdentifiers(4);

		try {
			PCubeArea cellPath(new CubeArea(db, thisCube, ids));
			CubeArea::CellType cellType = cellPath->getType(cellPath->pathBegin());
			if (cellType == CubeArea::CONSOLIDATED && replaceBulkState != Cube::In) {
				commitChanges(false, PUser(), changedCubes, false);
			}
			if (replaceBulkState == Cube::First) {
				replaceBulkState = Cube::In;
			}
			setCellValue(server, db, cellPath, CellValue(), PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, SET_BASE, false, 0, changedCubes, true, cellType);
		} catch (ErrorException &e) {
			Logger::debug << "journal file command: " << JournalFileReader::JOURNAL_CELL_CLEAR << " - " << e.getMessage() << endl;
		}
	} else if (command == JournalFileReader::JOURNAL_CUBE_CLEAR) {
		clearCells(server, db, PUser(), false);
	} else if (command == JournalFileReader::JOURNAL_CELL_GOALSEEK) {
		commitChanges(false, PUser(), changedCubes, false); // commit previous changes, this command reads data

		IdentifiersType ids = history.getDataIdentifiers(4);
		CellValueContext::GoalSeekType gsType = (CellValueContext::GoalSeekType)history.getDataInteger(5);
		PArea area = history.getDataArea(6, dimensions.size());
		double value = history.getDataDouble(7);

		PCubeArea cp(new CubeArea(db, thisCube, ids));
		cellGoalSeek(server, db, gsType, cp, area, PUser(), boost::shared_ptr<PaloSession>(), value, false);
	} else if (command == JournalFileReader::JOURNAL_CELL_COPY) {
		commitChanges(false, PUser(), changedCubes, false); // commit previous changes, this command reads data

		IdentifiersType from = history.getDataIdentifiers(4);
		IdentifiersType to = history.getDataIdentifiers(5);
		double dFactor = history.getDataDouble(6);
		bool bUseRules = history.getDataBool(7, false);
		PPaths lockedPaths = history.getDataPaths(8);
		PLockedCells lockedCells;
		if (lockedPaths && !lockedPaths->empty()) {
			lockedCells = PLockedCells(new LockedCells(db, thisCube, lockedPaths));
		}

		try {
			PCubeArea fromPath(new CubeArea(db, thisCube, from));
			PCubeArea toPath(new CubeArea(db, thisCube, to));

			copyCells(server, db, fromPath, toPath, lockedCells, PUser(), bUseRules, NULL, &dFactor, false);
		} catch (ErrorException &e) {
			Logger::debug << "journal file command: " << JournalFileReader::JOURNAL_CELL_COPY << " - " << e.getMessage() << endl;
		}
	} else if (command == JournalFileReader::JOURNAL_CUBE_AREA_CLEAR) {
		PArea area = history.getDataArea(4, dimensions.size());
		PCubeArea array(new CubeArea(db, thisCube, *area));
		clearCells(server, db, array, PUser(), false);
	} else if (command == JournalFileReader::JOURNAL_RULE_CREATE) {
		IdentifierType id = history.getDataInteger(4);
		string definition = history.getDataString(5);
		string external = history.getDataString(6);
		string comment = history.getDataString(7);
		bool activate = history.getDataBool(8);
		double position = history.getDataDouble(9);

		PRuleNode ruleNode;
		if (activate) {
			RuleParserDriver driver;
			driver.parse(definition);
			ruleNode = PRuleNode(driver.getResult());

			string errMessage;
			if (!ruleNode->validate(server, db, COMMITABLE_CAST(Cube, shared_from_this()), errMessage)) {
				Logger::error << "cannot parse rule " << id << " in cube '" << getName() << "': " << errMessage << ", rule disabled" << endl;
				activate = false;
				ruleNode.reset();
			}
		}

		createRule(server, db, ruleNode, definition, external, comment, activate, PUser(), false, &id, position);
	} else if (command == JournalFileReader::JOURNAL_RULE_MODIFY) {
		IdentifierType id = history.getDataInteger(4);
		string definition = history.getDataString(5);
		string external = history.getDataString(6);
		string comment = history.getDataString(7);
		ActivationType activate = (ActivationType)history.getDataInteger(8);
		double position = history.getDataDouble(9);

		PRule rule = findRule(id);

		PRuleNode ruleNode;
		if (activate) {
			RuleParserDriver driver;
			driver.parse(definition);
			ruleNode = PRuleNode(driver.getResult());

			string errMessage;
			if (!ruleNode->validate(server, db, COMMITABLE_CAST(Cube, shared_from_this()), errMessage)) {
				Logger::error << "cannot parse rule " << id << " in cube '" << getName() << "': " << errMessage << ", rule disabled" << endl;
				activate = INACTIVE;
				ruleNode.reset();
			}
		}

		modifyRule(server, db, rule, ruleNode, definition, external, comment, PUser(), activate, false, position);
	} else if (command == JournalFileReader::JOURNAL_RULE_DESTROY) {
		IdentifierType id = history.getDataInteger(4);
		deleteRule(server, db, id, PUser(), false);
	} else if (command == JournalFileReader::JOURNAL_RULE_ACTIVATE) {
		IdentifierType id = history.getDataInteger(4);
		ActivationType activate = (ActivationType)history.getDataInteger(5);
		PRule rule = findRule(id);
		activateRules(server, db, vector<PRule>(1, rule), activate, PUser(), NULL, false, false);
	} else {
		// unknown command
		Logger::info << "unknown cube journal file command: " << command << endl;
	}
}

boost::shared_ptr<JournalFileReader> Cube::initJournalProcess()
{
	checkCheckedOut();
	// and read the journal
	if (journalFile != 0) {
		journalFile->closeFile();
	}

	boost::shared_ptr<JournalFileReader> journalReader;

	if (getStatus() == LOADED) {
		JournalFile::deleteJournalFiles(FileName(*fileName, "log"), false);
		return journalReader;
	}

	Logger::trace << "scanning log file for cube '" << getName() << "'" << endl;

	if (fileName) {
		journalReader.reset(new JournalFileReader(FileName(*fileName, "log")));
		if (!journalReader->openFile(false, false)) {
			journalReader.reset();
		}
	}

	return journalReader;
}

void Cube::processCubeJournal(PServer server, PDatabase db)
{
	boost::shared_ptr<JournalFileReader> history = initJournalProcess();

	if (!history.get()) {
		return;
	}

	CPCube thisCube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
	InBulkEnum replaceBulkState = Cube::Not;

	Context::getContext()->setInJournal(true);

	set<PCube> changedCubes;
	while (history->isDataLine()) {
		processJournalCommand(server, db, thisCube, *history, replaceBulkState, changedCubes);
		history->nextLine();
	}

	Context::getContext()->setInJournal(false);

	commitChanges(false, PUser(), changedCubes, false);
}

void Cube::loadCubeRuleInfo(FileReader* file)
{
	checkCheckedOut();
	rules->setNewIDStart(file->getDataInteger(0) + 1); // for rules, max rule identifier is saved
}

void Cube::loadCubeRule(PServer server, PDatabase db, FileReader* file, int version)
{
	checkCheckedOut();
	IdentifierType id = file->getDataInteger(0);
	string definition = file->getDataString(1);
	string external = file->getDataString(2);
	string comment = file->getDataString(3);
	time_t timestamp = file->getDataInteger(4);
	bool isActive = file->getDataBool(5, true);
	double position = file->getDataDouble(6);

	if (version == 1) {
		replace(definition.begin(), definition.end(), '[', '{');
		replace(definition.begin(), definition.end(), ']', '}');
	}

	try {
		RuleParserDriver driver;
		driver.parse(definition);
		PRuleNode r = PRuleNode(driver.getResult());

		if (r) {
			string errorMsg;
			bool ok = r->validate(server, db, COMMITABLE_CAST(Cube, shared_from_this()), errorMsg);

			if (!ok) {
				if (isActive) {
					Logger::error << "cannot parse rule " << id << " in cube '" << getName() << "': " << errorMsg << ", rule disabled" << endl;
					isActive = false;
				}
				r.reset();
			}
		} else if (isActive) {
			Logger::error << "cannot parse rule " << id << ": " << driver.getErrorMessage() << endl;
			isActive = false;
		}

		PRule rule = PRule(new Rule(r, db, COMMITABLE_CAST(Cube, shared_from_this()), definition, external, comment, timestamp, isActive));
		rule->setID(id);
		if (!position) {
			position = double(id+1);
		}
		rule->setPosition(position);
		rules->add(rule, false);

		// the marker areas will be updated later
		if (isActive && rule->hasMarkers()) {
			Context::getContext()->addNewMarkerRule(db->getId(), getId(), rule->getId());
		}
	} catch (const ErrorException& ex) {
		Logger::error << "cannot parse rule " << id << ": " << ex.getMessage() << " (" << ex.getDetails() << ")" << endl;
	}
}

void Cube::loadCubeRules(PServer server, PDatabase db)
{
	checkCheckedOut();
	WriteLocker wl(rulefilelock->getLock());
	if (!FileUtils::isReadable(*ruleFileName)) {
		return;
	}

	boost::shared_ptr<FileReader> fr(FileReader::getFileReader(*ruleFileName));
	fr->openFile(true, false);
	int version = 1;

	if (fr->isSectionLine()) {

		if (fr->getSection() == "RULES INFO") {
			fr->nextLine();
			while (fr->isDataLine()) {
				loadCubeRuleInfo(fr.get());
				fr->nextLine();
			}

		}

		if (fr->getSection() == "RULES2") {
			fr->nextLine();
			version = 2;
		}

	}

	while (fr->isDataLine()) {
		loadCubeRule(server, db, fr.get(), version);

		fr->nextLine();
	}
}

void Cube::loadCube(PServer server, PDatabase db, bool processJournal)
{
	Logger::trace << "loading cube '" << getName() << "'. " << endl;
	loadCubeIntern(server, db, processJournal, true, true);
}

void Cube::loadCubeIntern(PServer server, PDatabase db, bool processJournal, bool checkIgnore, bool checkAlias)
{
	checkCheckedOut();
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cube file name not set");
	}

	if (getStatus() != UNLOADED) {
		return;
	}

	updateClientCacheToken();

	{
		WriteLocker wl(filelock->getLock());

		bool loadBin = FileUtils::isReadable(FileName(*fileName, BIN));
		bool hasTmp = FileUtils::isReadable(FileName(*fileName, BINTMP));
		if (loadBin) {
			if (hasTmp) {
				// BINTMP was not saved correctly
				FileUtils::remove(FileName(*fileName, BINTMP));
			}
		} else {
			if (hasTmp) {
				// old BIN had been removed but BINTMP was not renamed to BIN
				Logger::warning << "using binary temp file for cube '" << getName() << "'" << endl;
				FileUtils::rename(FileName(*fileName, BINTMP), FileName(*fileName, BIN));
				loadBin = true;
			} else {
				hasTmp = FileUtils::isReadable(FileName(*fileName, CSVTMP));
				if (FileUtils::isReadable(FileName(*fileName, CSV))) {
					if (hasTmp) {
						// CSVTMP was not saved correctly
						FileUtils::remove(FileName(*fileName, CSVTMP));
					}
				} else {
					if (hasTmp) {
						// old CSV had been removed but CSVTMP was not renamed to CSV
						Logger::warning << "using csv temp file for cube '" << getName() << "'" << endl;
						FileUtils::rename(FileName(*fileName, CSVTMP), FileName(*fileName, CSV));
					} else {
						throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "missing file", "cube", getId());
					}
				}
			}
		}

		bool loadCells = true;
		if (checkIgnore) {
			loadCells = checkIgnore ? !ignoreCellData && (getType() == GPUTYPE || getType() == NORMALTYPE || getType() == USER_INFOTYPE) : true;
		}
		loadFromFile(server, db, loadCells, checkAlias, loadBin);

		loadCubeRules(server, db);

		if (processJournal) {
			// process journal here, otherwise it will be done by database chronologically and saved after that
			processCubeJournal(server, db);

			if (getStatus() == CHANGED) {
				saveCube(server, db);
			}
		}

		if (journal) journal->flush();
		journalFile->clear();
	}

	// the cube is now loaded
	if (getStatus() != CHANGED) {
		setStatus(LOADED);
	}
}

void Cube::loadFromFile(PServer server, PDatabase db, bool loadCells, bool checkAlias, bool binary)
{
	FileName fn(*fileName, binary ? BIN : CSV);
	boost::shared_ptr<FileReader> fr(FileReader::getFileReader(fn));
	fr->openFile(true, false);

	timeval tv;
	bool err = false;
	if (binary) {
		uint32_t ver;
		uint32_t endianness;

		if (loadCubeOverview(fr.get(), tv, ver, endianness)) {
			if (!ver || ver > CUBE_FILE_VERSION) {
				Logger::error << "unknown file version of cube '" << getName() << "'" << endl;
				err = true;
			}
			if (!err) {
				uint32_t endian = server->isBigEndian() ? bigEndian : littleEndian;
				if (endian != endianness) {
					Logger::error << "different endianness of cube '" << getName() << "'" << endl;
					err = true;
				}
			}
		} else {
			Logger::error << "section 'CUBE' is invalid in cube '" << getName() << "'" << endl;
			err = true;
		}
		if (!err && loadCells) {
			if (fr->isSectionLine()) {
				bool diffAlias;
				if (!loadCubeCells(fr.get(), db, checkAlias, true, diffAlias, ver)) {
					if (diffAlias) {
						Logger::info << "alias dimension changed, loading cube '" << getName() << "' from csv file" << endl;
					} else {
						Logger::error << "invalid section in cube '" << getName() << "'" << endl;
					}
					err = true;
				}
			} else {
				Logger::error << "section line not found for cube '" << getName() << "'" << endl;
				err = true;
			}
		}
		if (err) {
			FileName fn(*fileName, CSV);
			fr.reset(FileReader::getFileReader(fn));
			fr->openFile(true, false);
		}
	}
	if (!binary || err) {
		timeval tv2;
		// load overview
		loadCubeOverview(fr.get(), tv2);

		if (binary && err && (tv.tv_sec != tv2.tv_sec || tv.tv_usec != tv2.tv_usec)) {
			Logger::warning << "different time stamp of files for cube '" << getName() << "'" << endl;
		}

		// and cell values
		if (loadCells) {
			if (fr->isSectionLine()) {
				bool diffGroup;
				loadCubeCells(fr.get(), db, checkAlias, false, diffGroup, 0);
			} else {
				Logger::warning << "section line not found for cube '" << getName() << "'" << endl;
			}
		}
	}
}

void Cube::saveCubeOverview(FileWriter *file, PServer server, PDatabase db, timeval &tv, bool binary)
{
	if (!binary) {
		file->appendComment("PALO CUBE DATA");
		file->appendComment("");

		file->appendComment("Description of data: ");
		file->appendComment("TIME_STAMP;SIZE_DIMENSIONS;");
	}
	file->appendSection("CUBE");
	file->appendTimeStamp(tv);
	if (binary) {
		//file version
		file->appendInteger(CUBE_FILE_VERSION);
		//endianness
		file->appendInteger(server->isBigEndian() ? bigEndian : littleEndian);
	} else {
		vector<int32_t> sizes;
		for (IdentifiersType::iterator it = dimensions.begin(); it != dimensions.end(); ++it) {
			sizes.push_back(db->lookupDimension(*it, false)->sizeElements());
		}

		file->appendIntegers(&sizes);
	}
	file->nextLine();
}

void Cube::saveCubeCells(FileWriter *file, PServer server, PDatabase db, bool checkAlias, bool binary)
{
	CPCube thisCube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
	PCubeArea area(new CubeArea(db, thisCube, dimensions.size()));

	bool hasAlias = false;
	for (size_t i = 0; i < dimensions.size(); i++) {
		PDimension dim = db->lookupDimension(dimensions[i], false);
		if (checkAlias && dim->getDimensionType() == Dimension::ALIAS) {
			if (!hasAlias) {
				hasAlias = true;
				if (!binary) {
					file->appendComment("Description of data: ");
					file->appendComment("ID;NAME ");
				}
			}

			PSet aliasSet(new Set);

			file->appendSection(ALIAS_SECTION);
			file->appendInteger(dimensions[i]);
			file->nextLine();

			ElementsType elements = dim->getElements(PUser(), false);
			for (ElementsType::iterator iter = elements.begin(); iter != elements.end(); ++iter) {
				Element* element = *iter;

				if (element != 0) {
					IdentifierType id = element->getIdentifier();
					aliasSet->insert(id);

					file->appendInteger(id);
					file->appendEscapeString(element->getName(dim->getElemNamesVector()));
					file->nextLine();
				}
			}
			area->insert(i, aliasSet);
		} else {
			area->insert(i, dim->getElemIds(CubeArea::ALL_ELEMENTS));
		}
	}

	if (!binary) {
		file->appendComment("Description of data: ");
		file->appendComment("PATH;VALUE ");
	}
	file->appendSection(NUMERIC_SECTION);
	if (binary) {
		PEngineBase engine = server->getEngine();
		StorageCpu *st = dynamic_cast<StorageCpu *>(engine->getStorage(numericStorageId).get());
		st->save(file);
		file->nextLine();
	} else {
		int32_t valuesCounter = 0;
		PCellStream cs = calculateArea(area, CubeArea::BASE_NUMERIC, NO_RULES, true, UNLIMITED_UNSORTED_PLAN);
		if (cs) {
			while (cs->next()) {
				valuesCounter++;
				if (saveCSV || hasAlias) {
					file->appendIdentifiers(cs->getKey().begin(), cs->getKey().end());
					file->appendDouble(cs->getValue().getNumeric());
					file->nextLine();
				}
			}
			if (!saveCSV && !hasAlias) {
				file->appendRaw("# ");
				file->appendInteger(valuesCounter);
				file->appendRaw(" values should be saved in bin file!");
				file->nextLine();
			}
		}
	}

	if (!binary) {
		file->appendComment("Description of data: ");
		file->appendComment("PATH;VALUE ");
	}
	file->appendSection(STRING_SECTION);
	if (binary) {
		PEngineBase engine = server->getEngine();
		StorageCpu *st = dynamic_cast<StorageCpu *>(engine->getStorage(stringStorageId).get());
		st->save(file);
		file->nextLine();
	} else {
		PCellStream cs = calculateArea(area, CubeArea::BASE_STRING, NO_RULES, true, 0);
		if (cs) {
			while (cs->next()) {
				file->appendIdentifiers(cs->getKey().begin(), cs->getKey().end());
				file->appendEscapeString(cs->getValue());
				file->nextLine();
			}
		}
	}
}

void Cube::saveCubeType(FileWriter* file)
{
	file->appendIdentifier(getId());
	file->appendEscapeString(getName());

	IdentifiersType identifiers;

	for (IdentifiersType::const_iterator i = dimensions.begin(); i != dimensions.end(); ++i) {
		identifiers.push_back(*i);
	}

	file->appendIdentifiers(identifiers.begin(), identifiers.end());
	file->appendInteger(saveType);
	file->appendBool(isDeletable());
	file->appendBool(isRenamable());

	file->nextLine();
}

void Cube::saveCube(PServer server, PDatabase db)
{
	saveCubeIntern(server, db, true, true);
}

void Cube::saveCubeIntern(PServer server, PDatabase db, bool checkIgnore, bool checkAlias)
{
	if (fileName == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cube file name not set");
	}

	if (getStatus() == LOADED) {
		return;
	}

	if (cellsStatus == CHANGED) {
		WriteLocker wl(filelock->getLock());
		bool saveCells = true;
		if (checkIgnore) {
			saveCells = checkIgnore ? !ignoreCellData && (getType() == GPUTYPE || getType() == NORMALTYPE || getType() == USER_INFOTYPE) : true;
		}
		if (checkAlias && db->getType() == SYSTEMTYPE) {
			checkAlias = false;
		}

		bool hasBin = FileUtils::isReadable(FileName(*fileName, BIN)); // hasBin is false when the cube was loaded from CSV or it was just created, hasBin is true if a value in the existing cube was changed
		bool hasCsv = FileUtils::isReadable(FileName(*fileName, CSV));
		timeval tv;
		gettimeofday(&tv, 0);
		saveToFile(server, db, saveCells, checkAlias, tv, true);
		if (hasBin || !hasCsv) { //don't save CSV if the cube was loaded from it
			saveToFile(server, db, saveCells, checkAlias, tv, false);
		}
	}

	if (rulesStatus == CHANGED) {
		saveCubeRules(db);
	}

	if (journalFile != 0) {
		journalFile->closeFile();
	}
	// archive journal
	JournalFile::archiveJournalFiles(FileName(*fileName, "log"));

	// delete journal
	JournalFile::deleteJournalFiles(FileName(*fileName, "log"), false);

	if (journal) journal->flush();
	journalFile->clear();

	// the cube is now loaded
	setStatus(LOADED);
}

void Cube::saveToFile(PServer server, PDatabase db, bool saveCells, bool checkAlias, timeval &tv, bool binary)
{
	FileName ftmp(*fileName, binary ? BINTMP : CSVTMP);
	FileName fn(*fileName, binary ? BIN : CSV);

	// open a new temp-cube file
	boost::shared_ptr<FileWriter> fw(FileWriter::getFileWriter(ftmp));
	fw->openFile();

	saveCubeOverview(fw.get(), server, db, tv, binary);

	if (saveCells) {
		saveCubeCells(fw.get(), server, db, checkAlias, binary);
	}

	if (!binary) {
		fw->appendComment("");
		fw->appendComment("PALO CUBE DATA END");
	}

	fw->closeFile();

	// remove old cube file
	FileUtils::remove(fn);

	// rename temp-cube file
	FileUtils::rename(ftmp, fn);
}

void Cube::saveCubeRule(FileWriter* file, CPRule rule, CPDatabase db)
{
	file->appendIdentifier(rule->getId());

	StringBuffer sb;
	rule->appendRepresentation(&sb, db, CONST_COMMITABLE_CAST(Cube, shared_from_this()));
	file->appendEscapeString(sb.c_str());

	file->appendEscapeString(rule->getExternal());
	file->appendEscapeString(rule->getComment());
	file->appendInteger(rule->getTimeStamp());
	file->appendBool(rule->isActive());
	file->appendDouble(rule->getPosition());
}

void Cube::saveCubeRules(CPDatabase db)
{
	WriteLocker wl(rulefilelock->getLock());
	boost::shared_ptr<FileWriter> fw(FileWriter::getFileWriter(FileName(*ruleFileName, CSVTMP)));
	fw->openFile();

	fw->appendComment("PALO CUBE RULES");
	fw->appendSection("RULES INFO");
	fw->appendInteger(rules->getLastId() - 1); // last rule id was saved by previous versions, rules are 1-based
	fw->nextLine();
	fw->appendSection("RULES2");

	for (RuleList::ConstIterator iter = rules->const_begin(); iter != rules->const_end(); ++iter) {
		saveCubeRule(fw.get(), CONST_COMMITABLE_CAST(Rule, *iter), db);
		fw->nextLine();
	}

	fw->appendComment("PALO CUBE RULES END");

	fw->closeFile();

	// rename temp-rule file
	FileUtils::remove(*ruleFileName);
	bool ok = FileUtils::rename(FileName(*ruleFileName, CSVTMP), *ruleFileName);

	if (!ok) {
		Logger::error << "cannot rename rule file to '" << ruleFileName->fullPath() << "' " << strerror(errno) << endl;
		throw FileOpenException("cannot rename rule file", ruleFileName->fullPath());
	}

	rulesStatus = LOADED;
}

void Cube::setCubeFileAndLoad(PServer server, PDatabase db, const FileName& fileName, bool newid)
{
	checkCheckedOut();
	this->fileName.reset(new FileName(fileName));
	this->ruleFileName.reset(new FileName(fileName.path, fileName.name + "_rules", fileName.extension));
	journalFile.reset(new JournalFile(FileName(*this->fileName, "log"), filelock, db->getId(), getId()));

	if (!newid && isLoadable()) {
		setStatus(UNLOADED);
		loadCube(server, db, false);
	} else {
		saveCube(server, db);
	}
}

void Cube::deleteCubeFiles()
{
	// delete cube file from disk
	if (FileUtils::isReadable(FileName(*fileName, BIN))) {
		FileWriter::deleteFile(FileName(*fileName, BIN));
	}
	if (FileUtils::isReadable(FileName(*fileName, CSV))) {
		FileWriter::deleteFile(FileName(*fileName, CSV));
	}

	if (FileUtils::isReadable(*ruleFileName)) {
		FileWriter::deleteFile(*ruleFileName);
	}

	if (journalFile != 0) {
		journalFile->closeFile();
		if (journal) journal->flush();
		journalFile->clear();
	}
	JournalFile::deleteJournalFiles(FileName(*fileName, "log"));
}

void Cube::unloadCube(PServer server, PDatabase db)
{
	if (!isLoadable()) {
		throw ParameterException(ErrorException::ERROR_CUBE_UNSAVED, "it is not possible to unload an unsaved cube, use delete for unsaved cubes", "", "");
	}

	if (getStatus() == UNLOADED) {
		return;
	}

	// save any outstanding changes
	saveCube(server, db);

	// update token
	updateClientCacheToken();

	if (getType() == GPUTYPE) {
		PEngineBase gpuEngine = server->getEngine(EngineBase::GPU, true);
		if (gpuEngine) {
			gpuEngine->recreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		}
	}
	PEngineBase cpuEngine = server->getEngine(EngineBase::CPU, true);
	cpuEngine->recreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	cpuEngine->recreateStorage(stringStorageId, pathTranslator, EngineBase::String);
	cpuEngine->recreateStorage(markerStorageId, pathTranslator, EngineBase::Marker);

	rules.reset(new RuleList());

	// cube is now unloaded
	setStatus(UNLOADED);
}

////////////////////////////////////////////////////////////////////////////////
// getter and setter
////////////////////////////////////////////////////////////////////////////////

IdentifiersType Cube::getDimensionSizes(PDatabase db) const
{
	IdentifiersType sizes(dimensions.size());
	for (uint32_t dimIdx = 0; dimIdx < dimensions.size(); dimIdx++) {
		PDimension dim = db->lookupDimension(dimensions[dimIdx], false);
		if (dim) {
			sizes[dimIdx] = dim->getMaximalIdentifier();
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "error in getDimensionSizes, database corrupted");
		}
	}
	return sizes;
}

void Cube::updatePathTranslator(PDatabase db){
    checkCheckedOut();
    IdentifiersType dimSizes = getDimensionSizes(db);
    PPathTranslator pt(new PathTranslator(dimSizes));
    pathTranslator = pt;
}

void Cube::updatePathTranslator(PPathTranslator pt){
	checkCheckedOut();
	pathTranslator = pt;
}

vector<PRule> Cube::getRules(PUser user, bool activeOnly) const
{
	checkCubeRuleRight(user, RIGHT_READ);

	Context *context = Context::getContext();
	CPCommitable commitableThis = shared_from_this();

	vector<PRule> result;

	for (RuleList::Iterator iter = rules->begin(); iter != rules->end(); ++iter) {
		if (*iter != 0) {
			PRule rule = COMMITABLE_CAST(Rule, *iter);
			if (!activeOnly || (activeOnly && rule->isActive())) {
				result.push_back(rule);
				if (context) {
					context->saveParent(commitableThis, *iter);
				}
			}
		}
	}

	return result;
}

ItemType Cube::getType() const
{
	switch (saveType) {
	case NORMAL:
		return NORMALTYPE;
	case GPU:
		return GPUTYPE;
	case USERINFO:
		return USER_INFOTYPE;
	case RIGHTS:
	case ATTRIBUTES:
	case CONFIGURATION:
	case SUBSETVIEW:
	case LOG:
	case SESSIONS:
	case JOBS:
	case LICENSES:
		return SYSTEMTYPE;
	}
	throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid cube type");
}

void Cube::setType(ItemType type)
{
	checkCheckedOut();
	if ((saveType == NORMAL || (saveType == GPU)) && (type == NORMALTYPE || type == GPUTYPE)) {
		if (type == NORMALTYPE) {
			saveType = NORMAL;
		} else {
			saveType = GPU;
		}
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Cube::setType - invalid cube type");
	}
}

uint32_t Cube::getToken() const
{
	// return database token in order to get changes on a dimension
	return CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()))->getToken();
}

uint32_t Cube::getMyToken() const
{
	return token;
}

uint32_t Cube::getClientCacheToken() const
{
	switch (CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()))->getClientCacheType()) {
	case ConfigurationCube::CACHE_ALL:
		return clientCacheToken->getLastId();
	case ConfigurationCube::NO_CACHE_WITH_RULES:
		if (!hasRule()) {
			return clientCacheToken->getLastId();
		}
	default:
		return clientCacheToken->getNewId();
	}
}

size_t Cube::sizeFilledCells() const
{
	return sizeFilledNumericCells() + sizeFilledStringCells();
}

size_t Cube::sizeFilledNumericCells() const
{
	size_t values = 0;
	if (numericStorageId != NO_IDENTIFIER) {
		PEngineBase engine = Context::getContext()->getServer()->getEngine();
		values = engine->getStorage(numericStorageId)->valuesCount();
	}
	return values;
}

size_t Cube::sizeFilledStringCells() const
{
	size_t values = 0;
	if (stringStorageId != NO_IDENTIFIER) {
		PEngineBase engine = Context::getContext()->getServer()->getEngine();
		values = engine->getStorage(stringStorageId)->valuesCount();
	}
	return values;
}

size_t Cube::sizeFilledMarkerCells() const
{
	size_t values = 0;
	if (markerStorageId != NO_IDENTIFIER) {
		PEngineBase engine = Context::getContext()->getServer()->getEngine();
		values = engine->getStorage(markerStorageId)->valuesCount();
	}
	return values;
}

void Cube::clearCells(PServer server, PDatabase db, PUser user, bool useJournal)
{
	checkCheckedOut();
	if (hasLock) {
		throw ErrorException(ErrorException::ERROR_CUBE_BLOCKED_BY_LOCK, "cannot clear cells because of a locked area");
	}
	if (user && ConfigDatabase::isConfigCube(db->getName(), getName())) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube contains protected cells, cannot be cleared", "user", user ? (int)user->getId() : 0);
	}

	checkCubeAccessRight(user, RIGHT_DELETE, true, true);

	bool invalidateMarkers = !fromMarkers.empty() && !wholeCubeLocked;
	if (sizeFilledNumericCells() && invalidateMarkers) {
		server->addChangedMarkerCube(COMMITABLE_CAST(Cube, shared_from_this()), true);
	}

	PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
	engine->recreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	engine->recreateStorage(stringStorageId, pathTranslator, EngineBase::String);
	engine->recreateStorage(markerStorageId, pathTranslator, EngineBase::Marker);

	if (getType() == GPUTYPE) {
		PEngineBase engineGpu = Context::getContext()->getServerCopy()->getEngine(EngineBase::GPU, true);
		engineGpu->recreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	}

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_CLEAR);
		journal->nextLine();
	}

	cellsStatus = CHANGED;
	updateClientCacheToken();
}

void Cube::clearCells(PServer server, PDatabase db, PCubeArea areaElements, PUser user, bool useJournal)
{
	checkCheckedOut();
	if (areaElements->dimCount() != dimensions.size()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "wrong cube used for base elements");
	}

	if (user && ConfigDatabase::isConfigCube(db->getName(), getName())) {
		PConfigDatabase cdb = COMMITABLE_CAST(ConfigDatabase, db);
		if (areaElements->isOverlapping(cdb->getProtectedArea())) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube contains protected cells, cannot be cleared", "user", user ? (int)user->getId() : 0);
		}
	}

	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	if (User::checkUser(user)) {
		RightsType minimumRight = RIGHT_DELETE;

		if ((getType() == SYSTEMTYPE && getCubeType() != Cube::ATTRIBUTES) || cube->getType() == USER_INFOTYPE) {
			User::RightSetting rs;
			checkAreaAccessRight(db, user, areaElements, rs, true, minimumRight, 0);
		} else {
			User::MinMaxRight rtRole = user->getRoleCellDataRight();
			bool enough = rtRole.second >= minimumRight;

			User::MinMaxRight rtDb;
			if (enough) {
				rtDb = user->getDatabaseDataRight(db->getId());
				enough = rtDb.second >= minimumRight;
			}
			User::MinMaxRight rtCube;
			if (enough) {
				rtCube = user->getCubeDataRight(db, getId());
				enough = rtCube.second >= minimumRight;
			}
			User::MinMaxRight rtDims;
			if (enough) {
				rtDims = user->computeDRight(db->getId(), cube);
				enough = rtDims.second >= minimumRight;
			}

			bool checkCells = false;
			if (enough) {
				checkCells = user->checkCellDataRightCube(db, cube);
				if (rtCube.first == RIGHT_EMPTY && rtDims.first == RIGHT_EMPTY && !checkCells) {
					enough = db->getDefaultRight() >= minimumRight;
				}
			}
			if (enough) {
				bool checkRole = rtRole.first < minimumRight;
				bool checkDb = rtDb.first < minimumRight;
				bool checkCube = rtCube.first < minimumRight;

				set<IdentifierType> userGroups;
				user->getUserGroupsCopy(userGroups);

				if (enough && checkRole) {
					enough = user->checkRoleRight(server->getSystemDatabase(), userGroups, User::cellDataRight, minimumRight);
				}
				if (enough && checkDb) {
					enough = user->checkDatabaseDataRight(db, userGroups, minimumRight);
				}
				if (enough && checkCube) {
					enough = user->checkCubeDataRight(db, cube, userGroups, minimumRight);
				}
				if (enough) {
					PCubeArea rootArea(new CubeArea(db, cube, dimensions.size())); // area of "root" elements (no ancestors in this area) to be checked for rights
					for (size_t i = 0; i < dimensions.size(); i++) {
						CPDimension dim = db->lookupDimension(dimensions[i], false);

						if (dim->hasRightsCube()) {
							CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
							PSet s(new Set);
							for (Area::ConstElemIter it = areaElements->elemBegin(i); it != areaElements->elemEnd(i); ++it) {
								Element* element = dim->findElement(*it, 0, false);

								set <Element*> ancestors = dim->ancestors(element);
								bool bHasAncestor = false;
								for (set<Element*>::iterator itAnc = ancestors.begin(); itAnc != ancestors.end(); ++itAnc) {
									if ((*itAnc)->getIdentifier() != element->getIdentifier() && areaElements->find(i, (*itAnc)->getIdentifier()) != areaElements->elemEnd(i)) {
										bHasAncestor = true;
										break;
									}
								}
								if (!bHasAncestor) {
									// element does not have its ancestor in requested area, it's root element in this area, check rights
									s->insert(element->getIdentifier());
								}
							}
							rootArea->insert(i, s);
						} else {
							rootArea->insert(i, areaElements->getDim(i));
						}
					}

					enough = user->checkDimsAndCells(db, cube, userGroups, rootArea, checkCells, minimumRight, 0);
				}
			}
			if (!enough) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
			}
		}
	}

	if (additiveCommit) {
		commitChangesIntern(true, user, false);
		additiveCommit = false;
	}

	size_t valueCount = sizeFilledNumericCells();

	PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
	PEngineBase engineGpu;
    if (getType() == GPUTYPE) {
    	engineGpu = Context::getContext()->getServerCopy()->getEngine(EngineBase::GPU, true);
    }
	SubCubeList str, num, cons;
	areaElements->splitbyTypes(str, num, cons);

	PStorageBase strStorage = engine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
	for (SubCubeList::iterator it  = str.begin(); it != str.end(); ++it) {
		strStorage->setCellValue(it->second, CellValue::NullString, StorageBase::SET);
	}

	PStorageBase numStorage = engine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	PStorageBase numStorageGpu;
    if (getType() == GPUTYPE) {
    	numStorageGpu = engineGpu->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
    }

    bool invalidateMarkers = false;
	bool checkMarkers = !fromMarkers.empty() && !wholeCubeLocked;
	if (checkMarkers) {
		invalidateMarkers = checkMarkerInvalidation(num);
	}
	for (SubCubeList::iterator it = num.begin(); it != num.end(); ++it) {
		numStorage->setCellValue(it->second, CellValue::NullNumeric, StorageBase::SET);
	    if (getType() == GPUTYPE) {
    		numStorageGpu->setCellValue(it->second, CellValue::NullNumeric, StorageBase::SET);
	    }
	}

	for (SubCubeList::iterator it = cons.begin(); it != cons.end(); ++it) {
		PCubeArea baseArea = it->second->expandBase(0, 0);
		if (checkMarkers && !invalidateMarkers) {
			invalidateMarkers = checkMarkerInvalidation(baseArea.get(), 0);
		}
		numStorage->setCellValue(baseArea, CellValue::NullNumeric, StorageBase::SET);
	    if (getType() == GPUTYPE) {
			numStorageGpu->setCellValue(baseArea, CellValue::NullNumeric, StorageBase::SET);
	    }
	}

	commitChangesIntern(true, user, false);

	if (valueCount != sizeFilledNumericCells() && invalidateMarkers) {
		server->addChangedMarkerCube(cube, false);
	}

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CUBE_AREA_CLEAR);
		journal->appendArea(areaElements);
		journal->nextLine();
	}

	cellsStatus = CHANGED;
	updateClientCacheToken();
}

void Cube::copyCellCreateMaps(PDatabase db, Element *sourceElem, Element *targetElem, CPDimension dim, PSetMultimap &setMultimap,
		Set &sourceSet, Set &targetSet, set<IdentifierType> &targetForbiddenElems, Id2IdMap &target2SourceMap, bool &simpleCase)
{
	IdentifierType sourceId = sourceElem->getIdentifier();
	IdentifierType targetId = targetElem->getIdentifier();

	if (!sourceElem) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with identifier " + StringUtils::convertToString(sourceId) + " not found", "elementIdentifier", (int)sourceId);
	}
	if (!targetElem) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with identifier " + StringUtils::convertToString(targetId) + " not found", "elementIdentifier", (int)targetId);
	}
	if (sourceElem->getElementType() == Element::STRING) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot copy from string path", "source element type", Element::STRING);
	}
	if (targetElem->getElementType() == Element::STRING) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot copy to string path", "target element type", Element::STRING);
	}

	if (sourceElem->getElementType() == Element::CONSOLIDATED && targetElem->getElementType() == Element::CONSOLIDATED) {
		size_t sourceChildrenCount = dim->getChildrenCount(sourceElem);
		size_t targetChildrenCount = dim->getChildrenCount(targetElem);
		const ElementsWeightType childrenSource = dim->getChildren(PUser(), sourceElem);
		const ElementsWeightType childrenTarget = dim->getChildren(PUser(), targetElem);
		if (sourceChildrenCount == targetChildrenCount) {
			for (size_t i = 0; i < sourceChildrenCount; i++) {
				if (!setMultimap) {
					setMultimap.reset(new SetMultimap());
				}
				copyCellCreateMaps(db, (childrenSource)[i].first, (childrenTarget)[i].first, dim, setMultimap, sourceSet, targetSet, targetForbiddenElems, target2SourceMap, simpleCase);
			}
			return; // all done
		}
	}

	if ((sourceElem->getElementType() == Element::NUMERIC && targetElem->getElementType() == Element::CONSOLIDATED) || (sourceElem->getElementType() == Element::CONSOLIDATED && targetElem->getElementType() == Element::NUMERIC)) {
		simpleCase = false;
	}

	if ((sourceElem->getElementType() == Element::NUMERIC || sourceElem->getElementType() == Element::CONSOLIDATED) && (targetElem->getElementType() == Element::NUMERIC || targetElem->getElementType() == Element::CONSOLIDATED)) {
		if (targetForbiddenElems.find(targetElem->getIdentifier()) != targetForbiddenElems.end()) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "source and target areas are not equivalent");
		}


		if (!targetSet.insert(targetId)) {
			// verification of multiple targets?
			Id2IdMap::iterator it = target2SourceMap.find(targetId);
			if (it != target2SourceMap.end()) {
				if (it->second != sourceId) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "source and target areas are not equivalent");
				} else {
					return; // this pair already found
				}
			}
		} else {
			target2SourceMap.insert(make_pair(targetId, sourceId));
		}
		sourceSet.insert(sourceId);
		if (setMultimap) {
			setMultimap->insert(SetMultimap::value_type(sourceId, targetId));
		}

		if (targetElem->getElementType() == Element::CONSOLIDATED) {
			set<Element *> descendants = dim->descendants(targetElem);
			for (set<Element *>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
				targetForbiddenElems.insert((*it)->getIdentifier());
			}
		}
		set<Element *> ancestors = dim->ancestors(targetElem);
		for (set<Element *>::iterator it = ancestors.begin(); it != ancestors.end(); ++it) {
			if ((*it)->getIdentifier() != targetElem->getIdentifier()) {
				targetForbiddenElems.insert((*it)->getIdentifier());
			}
		}
	} else {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot copy", "unknown element type", Element::UNDEFINED);
	}
}

bool Cube::copyCellValuesPrepare(PServer server, PDatabase db, PCubeArea cellPathFrom, PCubeArea cellPathTo, PLockedCells lockedCells, PUser user, CubeArea &areaSource,
	CubeArea &areaTarget, SetMultimaps &setMultimaps, double &factorToReturn, double *dValue, double *factorFromJournal, bool &simpleCase, bool bUseRules)
{
	size_t dimCount = dimensions.size();
	Area::PathIterator pathFrom = cellPathFrom->pathBegin();
	Area::PathIterator pathTo = cellPathTo->pathBegin();

	if (pathFrom == pathTo) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Source and target paths are not different.");
	}

	for (uint32_t i = 0; i < dimCount; i++) {
		if (pathFrom.at(i) == pathTo.at(i)) {
			continue;
		}

		PDimension dim = db->lookupDimension(dimensions[i], false);
		Element *elemFrom = dim->lookupElement(pathFrom.at(i), false);
		Element *elemTo = dim->lookupElement(pathTo.at(i), false);

		// Todo: -jj- faster would be to check intersection of base ranges
		set<Element *> descFrom = dim->descendants(elemFrom);
		set<Element *> descTo = dim->descendants(elemTo);

		bool bIntersect = false;
		for (set<Element *>::iterator it = descFrom.begin(); !bIntersect && it != descFrom.end(); ++it) {
			// Find will be faster than iteration
			set<Element *>::iterator it2 = descTo.find(*it);
			if (it2 != descTo.end()) {
				bIntersect = true;
				break;
			}
		}
		if (bIntersect) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot copy between overlapping areas");
		}
	}

	factorToReturn = 1.0;
	CellValue cellValue = getCellValue(cellPathFrom, bUseRules);

	if (!factorFromJournal && !dValue) {
		User::RightSetting rs(User::checkCellDataRightCube(db, CONST_COMMITABLE_CAST(Cube, shared_from_this())));
		checkAreaAccessRight(db, user, cellPathTo, rs, cellValue.isEmpty(), RIGHT_SPLASH, 0);
	}

	if (dValue && cellValue.isEmpty()) { // dValue != NULL means 'like' which is not allowed when the source cell is 0
		throw ErrorException(ErrorException::ERROR_INVALID_COPY_VALUE, "like/predict is not allowed when the source cell is 0");
	}
	bool del = dValue ? *dValue == 0.0 : cellValue.isEmpty();
	if (del) {
		set<PCube> changedCubes;
		setCellValue(server, db, cellPathTo, CellValue::NullNumeric, lockedCells, PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, false, CubeArea::NONE);
		commitChanges(true, user, changedCubes, false);
		return false;
	} else {
		if (dValue) {
			factorToReturn = *dValue / cellValue.getNumeric();
		} else if (factorFromJournal) {
			factorToReturn = *factorFromJournal;
		}

		vector<set<IdentifierType> > targetForbiddenElems;
		targetForbiddenElems.resize(dimCount);

		for (uint32_t i = 0; i < dimCount; i++) {
			PDimension dim = db->lookupDimension(dimensions[i], false);
			Element *sourceElement = dim->lookupElement(pathFrom.at(i), false);
			Element *targetElement = dim->lookupElement(pathTo.at(i), false);
			if (sourceElement == targetElement) {
				const WeightedSet *ws = sourceElement->getBaseElements();
				if (ws) {
					areaSource.insert(i, PSet(new Set(*ws)));
					areaTarget.insert(i, PSet(new Set(*ws)));
				} else {
					PSet s(new Set());
					s->insert(sourceElement->getIdentifier());
					areaSource.insert(i, s);
					areaTarget.insert(i, s);
				}
			} else {
				PSet targetSet(new Set());
				PSet sourceSet(new Set());
				Id2IdMap target2SourceMap;
				copyCellCreateMaps(db, sourceElement, targetElement, dim, setMultimaps[i], *sourceSet, *targetSet, targetForbiddenElems[i], target2SourceMap, simpleCase);
				areaSource.insert(i, sourceSet);
				areaTarget.insert(i, targetSet);
			}
		}
	}

	return true;
}

bool Cube::copyCells(PServer server, PDatabase db, PCubeArea cellPathFrom, PCubeArea cellPathTo, PLockedCells lockedCells, PUser user, bool bUseRules, double *dValue, double *factorFromJournal, bool useJournal)
{
	checkCheckedOut();
	if (cellPathFrom->getType(cellPathFrom->pathBegin()) == CubeArea::BASE_STRING) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot copy from string path", "source element type", CubeArea::BASE_STRING);
	}

	if (cellPathTo->getType(cellPathTo->pathBegin()) == CubeArea::BASE_STRING) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot copy to string path", "destination element type", CubeArea::BASE_STRING);
	}

	size_t dimCount = dimensions.size();

	double factor = 1.0;
	PLock lock = lookupLockedArea(*cellPathTo->pathBegin(), user);

	if (dValue) {
		if (Logger::isTrace()) {
			Logger::trace << "copy/like cell from = <" << cellPathFrom->pathBegin().toString() << "> to = <" << cellPathTo->pathBegin().toString() << ">" << " value = " << *dValue << endl;
		}
	} else if (!factorFromJournal) { // not called from journal
		if (Logger::isTrace()) {
			Logger::trace << "copy cell from = <" << cellPathFrom->pathBegin().toString() << "> to = <" << cellPathTo->pathBegin().toString() << ">" << endl;
		}
	}

	SetMultimaps setMultimaps(dimCount);
	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	PCubeArea areaSrc(new CubeArea(db, cube, dimCount));
	PCubeArea areaTarget(new CubeArea(db, cube, dimCount));

	bool simpleCase = true;
	if (!copyCellValuesPrepare(server, db, cellPathFrom, cellPathTo, lockedCells, user, *areaSrc, *areaTarget, setMultimaps, factor, dValue, factorFromJournal, simpleCase, bUseRules)) {
		// nothing prepared to be computed, all done
		return true;
	}

	PPlanNode sourcePlan = createPlan(areaSrc, CubeArea::NUMERIC, bUseRules ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES, true, UNLIMITED_UNSORTED_PLAN);
	if (User::checkUser(user)) {
		User::RightSetting rs(User::checkCellDataRightCube(db, cube));
		checkAreaAccessRight(db, user, areaSrc, rs, false, RIGHT_READ, 0);
	}

	simpleCase &= sourcePlan->getType() == SOURCE;
	if (lockedCells && lockedCells->getLockedPaths()) {
		simpleCase = false; // if any cells are locked go the complicated way - can be optimized if needed
	}

	// insert transformation on top of the plan
	PPlanNode targetPlan = PPlanNode(new TransformationPlanNode(areaTarget, sourcePlan, setMultimaps, factor, vector<uint32_t>()));

	set<PCube> changedCubes;
	if (simpleCase) { // Todo: -jj- additional cases has to be eliminated (source or target is consolidation, multidimensional rights cube)
		PEngineBase engineCpu = server->getEngine(EngineBase::CPU, true);
		PStorageBase storageCpu = engineCpu->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		PEngineBase engineGpu;
		PStorageBase storageGpu;
		if (getType() == GPUTYPE) {
			engineGpu = server->getEngine(EngineBase::GPU, true);
			if (engineGpu) {
				storageGpu = engineGpu->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
			}
		}

		if (storageGpu) {
            if(dValue){
			    pendingWrites.push_back(storageGpu->setCellValueBegin(targetPlan, engineGpu, targetPlan->getArea(), CellValue(factor), StorageBase::MULTIPLY_EXISTING));
            }
            else{
    			pendingWrites.push_back(storageGpu->setCellValueBegin(targetPlan, engineGpu));
            }
		}
		// synchronous call to Cpu storage
		bool delValue = storageCpu->setCellValue(targetPlan, engineCpu);
		if (delValue) {
			bool checkMarkers = !fromMarkers.empty() && !wholeCubeLocked;
			if (checkMarkers)
				if (checkMarkerInvalidation(targetPlan->getArea().get(), 0)) {
					server->addChangedMarkerCube(cube, false);
				}
		} else {
			if (!wholeCubeLocked && !fromMarkers.empty()) {
				EngineBase *engine = engineCpu.get();
				PProcessorBase targetData = engine->createProcessor(targetPlan, true);
				while (targetData->next()) {
					const CellValue &val = targetData->getValue();
					if (val.isNumeric() && !val.isEmpty() && val.getNumeric() != 0) {
						checkFromMarkers(engine, targetData->getKey(), changedCubes);
					}
				}
			}
		}
	} else { // complicated cases
		// cell by cell implementation
		PDoubleCellMap spFinalTargetBatch(CreateDoubleCellMap(dimCount));
		ICellMap<double> *finalTargetBatch = spFinalTargetBatch.get();
		// fill finalTargetBatch with zeros for current existing target data - will be erased when no replacement is found
		PCellStream csTarget = calculateArea(areaTarget, CubeArea::NUMERIC, NO_RULES, true, UNLIMITED_UNSORTED_PLAN);
		while (csTarget->next()) {
			finalTargetBatch->set(csTarget->getKey(), 0);
		}

		PCellStream csMappedSource = evaluatePlan(targetPlan, EngineBase::ANY, true);
		while (csMappedSource->next()) {
			const CellValue &sourceValue = csMappedSource->getValue();
			if (sourceValue.isNumeric()) {
				finalTargetBatch->set(csMappedSource->getKey(), sourceValue.getNumeric());
			} else if (sourceValue.isEmpty()) {
				// not sure if it can go here
			}
		}

		PCellStream changedValues = spFinalTargetBatch->getValues();
		size_t counter = 0;
		Context *con = Context::getContext();
		while (changedValues->next()) {
			if (!(++counter % 10000)) {
				con->check();
			}
			setCellValue(server, db, PCubeArea(new CubeArea(db, cube, changedValues->getKey())), CellValue(changedValues->getDouble()), lockedCells, PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
		}
	}

	commitChanges(true, user, changedCubes, false);

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CELL_COPY);
		Area::PathIterator it = cellPathFrom->pathBegin();
		journal->appendIdentifiers((*it).begin(), (*it).end());
		it = cellPathTo->pathBegin();
		journal->appendIdentifiers((*it).begin(), (*it).end());
		journal->appendDouble(factor); // backward compatibility
		journal->appendBool(bUseRules);
		journal->appendPaths(lockedCells ? lockedCells->getLockedPaths() : CPPaths());
		journal->nextLine();
	}

	cellsStatus = CHANGED;
	updateClientCacheToken();

	return true;
}

bool Cube::copyCells()
{
	checkCheckedOut();
	Context *context = Context::getContext();
	CPCellValueContext cvc = context->getCellValueContext();
	CellValueContext::PathAreaAndValue pav = *cvc->pathAreaAndValue.begin();

	return copyCells(context->getServer(), cvc->db, pav.cellPath, cvc->cellPathTo, cvc->lockedCells, cvc->user, cvc->useRules, cvc->ptrValue, NULL, true);
}

void Cube::disableRules(PServer server, PDatabase db, CPDimension dim)
{
	if (!rules->isCheckedOut()) {
		rules = COMMITABLE_CAST(RuleList, rules->copy());
	}
	for (RuleList::Iterator iter = rules->begin(); iter != rules->end(); ++iter) {
		PRule rule = COMMITABLE_CAST(Rule, *iter);

		if (rule->isActive() && rule->hasElement(dim, ALL_IDENTIFIERS)) {
			string errMsg;
			activateRules(server, db, vector<PRule>(1, rule), INACTIVE, PUser(), NULL, false, false);
		}
	}
}

ResultStatus Cube::setCellValue(PServer server, PDatabase db, PCubeArea spCellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	checkCheckedOut();
	CubeArea *cellPath = spCellPath.get();
	Area::PathIterator pathBegin = cellPath->pathBegin();

	if (user && ConfigDatabase::isConfigCube(db->getName(), getName())) {
		ConfigDatabase *cdb = dynamic_cast<ConfigDatabase *>(db.get());
		if (cdb->isProtected(*pathBegin)) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, pathBegin.toString(), "is protected", "cube", getName());
		}
	}

	CellValue origValue = value;
	CubeArea::CellType cellType = ct == CubeArea::NONE ? cellPath->getType(pathBegin) : ct;
	PPlanNode cellPlan;
	bool isAggregation = false;
	bool isBase = true;
	bool isString = false;
	bool addComm = false;

	if (value.isEmpty() && splashMode == DEFAULT) {
		splashMode = SET_BASE; // do not care about old value (multiplication factor) or sum of weights
	}

	if (cellType == CubeArea::CONSOLIDATED) {
		addComm = splashMode == ADD_BASE;
		// create calculation Plan - ignore rules
		cellPlan = createPlan(spCellPath, CubeArea::ALL, NO_RULES, true, UNLIMITED_UNSORTED_PLAN);

		if (!cellPlan) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "invalid cell path: ", pathBegin.toString(), "");
		}
		isAggregation = cellPlan->getType() == AGGREGATION;
		isBase = cellPlan->getType() == SOURCE;
		isString = false;
		if (isBase) {
			const SourcePlanNode *pn = static_cast<const SourcePlanNode *>(cellPlan.get());
			if (pn && pn->getStorageId() == getStringStorageId()) {
				isString = true;
			}
		}
	} else if (cellType == CubeArea::BASE_STRING) {
		addComm = additiveCommit; // to avoid commitChangesIntern
		isString = true;
		isBase = true;
	} else if (cellType == CubeArea::BASE_NUMERIC) {
		addComm = addValue;
		isString = false;
		isBase = true;
	}

	// check privs
	if (isAggregation) {
		// is splashing disabled?
		if (splashMode == DISABLED) {
			throw ParameterException(ErrorException::ERROR_SPLASH_DISABLED, pathBegin.toString(), "is consolidated, but splashing is disabled", "splashMode", splashMode);
		}
	}

	if (possibleCommit && (addComm != additiveCommit || (isAggregation && splashMode != ADD_BASE))) { //this condition can be improved, e.g. collect all DEFAULT until value is 0.0
		commitChangesIntern(true, user, false);
	}

	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	if (checkRights && user) {
		if (checkArea && cubeWorker != 0) {
			checkRights->checkSepRight = false;
		}
		bool isEmpty = value.isEmpty();
		checkAreaAccessRight(db, user, spCellPath, *checkRights, isEmpty, isAggregation ? RIGHT_SPLASH : (isEmpty ? RIGHT_DELETE : RIGHT_WRITE), 0);
	}

	// use the supervision event processor (SEP)
	if (checkArea && cubeWorker != 0) {
		bool ok = cubeWorker->start(db, cube);
		if (!ok) {
			throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start worker");
		}

		string strArea;
		if (isInArea(cellPath, strArea)) {
			if (value.isNumeric()) {
				return cubeWorker->setCellValue(strArea, session ? session->getSid() : "", *pathBegin, value.getNumeric(), splashMode, addValue);
			} else {
				return cubeWorker->setCellValue(strArea, session ? session->getSid() : "", *pathBegin, value, splashMode, addValue);
			}
		}
	}

	PServer serverCopy = Context::getContext()->getServerCopy();
	PEngineBase engineCpu = serverCopy->getEngine(EngineBase::CPU, true);
	PStorageBase storageCpu = engineCpu->getCreateStorage(value.isString() ? stringStorageId : numericStorageId, pathTranslator, value.isString() ? EngineBase::String : EngineBase::Numeric);

	PStorageBase storageGpu;
    bool useGpu = false;
	if (getType() == GPUTYPE && !value.isString()) {
		PEngineBase engineGpu = serverCopy->getEngine(EngineBase::GPU, true);
		if (engineGpu) {
            useGpu = true;
			storageGpu = engineGpu->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		}
	}

	bool invalidateMarkers = false;
	bool isZero = value.isNumeric() && (value.isEmpty() || value.getNumeric() == 0);
	bool checkMarkers = isZero && !fromMarkers.empty() && !wholeCubeLocked;
	if (isAggregation) {
		double factor = 1.0;
		bool equal = false;
		SubCubeList reducedAreas;

		PCubeArea originalBaseArea(new CubeArea(db, cellPath->getCube(), *cellPlan->getChildren().at(0)->getArea()));
		reducedAreas.push_back(originalBaseArea);

		bool anyActiveRestriction = false;
		// TODO -jj- cell locks
		PSubCubeList lockedSubcubes = lockedCells ? lockedCells->getLockedAreas() : PSubCubeList();
		if (lockedSubcubes) {
			for (SubCubeList::const_iterator it = lockedSubcubes->begin(); it != lockedSubcubes->end(); ++it){
				SubCubeList nextAreas;
				for (SubCubeList::const_iterator it2 = reducedAreas.begin(); it2 != reducedAreas.end(); ++it2){
					if (it2->second->isOverlapping(*(it->second))) {
						PCubeArea intersectionArea;
						it2->second->intersection(*(it->second), &intersectionArea, &nextAreas);
						anyActiveRestriction = true;
					} else {
						nextAreas.push_back(*it2);
					}
				}
				reducedAreas = nextAreas;
				if (reducedAreas.empty()) {
					break;
				}
			}
			if (reducedAreas.empty()) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "restricted base area is empty (by restricted cells)", "path", cellPath->toString());
			}
		}

		if (splashMode == DEFAULT) {
			PCellStream cs = evaluatePlan(cellPlan, EngineBase::ANY, true);
			cs->next();
			double prevConsValue = cs->getValue().getNumeric();

			if (addValue) {
				value = value.getNumeric() + prevConsValue;
			}
			if (anyActiveRestriction) {
				// generate new Plan of nonlocked base cells
				vector<PPlanNode> newChildrenNodes;
				SourcePlanNode *originalBaseSource = dynamic_cast<SourcePlanNode *>(cellPlan->getChildren().at(0).get());
				for (SubCubeList::const_iterator areaIt = reducedAreas.begin(); areaIt != reducedAreas.end(); ++areaIt) {
					PPlanNode newBase(new SourcePlanNode(*originalBaseSource));
					newBase->setArea(areaIt->second);
					newChildrenNodes.push_back(newBase);
				}
				cellPlan->setChildren(newChildrenNodes);

				// calculate sum on reduced area
				cs = evaluatePlan(cellPlan, EngineBase::ANY, true);
				cs->next();
				double restrictedConsValue = cs->getValue().getNumeric();
				if (!restrictedConsValue || (restrictedConsValue == prevConsValue && value.isEmpty())) {
					equal = true;
					value = value.getNumeric() - prevConsValue;
				} else {
					factor = (value.getNumeric() - prevConsValue + restrictedConsValue) / restrictedConsValue;
				}
			} else {
				if (prevConsValue && !value.isEmpty()) {
					factor = value.getNumeric() / prevConsValue;
				} else {
					equal = true;
				}
			}
			if (equal) {
				const AggregationPlanNode *apn = static_cast<const AggregationPlanNode *>(cellPlan.get());
				const AggregationMaps &ams = *apn->getAggregationMaps().get();

				SubCubeList nextAreas;
				double totalSumWeights = 0;
				for (SubCubeList::const_iterator areaIt = reducedAreas.begin(); areaIt != reducedAreas.end(); ++areaIt) {
					size_t dimOrdinal;
					double sumWeights = 1;
					for (dimOrdinal = 0; dimOrdinal < areaIt->second->dimCount(); dimOrdinal++) {
						double dimensionWeight = ams[dimOrdinal].getWeightsSum(anyActiveRestriction ? areaIt->second->getDim(dimOrdinal).get() : 0);
						sumWeights *= dimensionWeight;
						if (sumWeights == 0.0) {
							break;
						}
					}
					if (dimOrdinal == areaIt->second->dimCount()) {
						nextAreas.push_back(areaIt->second);
						totalSumWeights += sumWeights;
					}
				}
				if (nextAreas.empty()) {
					throw ParameterException(ErrorException::ERROR_SPLASH_NOT_POSSIBLE, "sum of weights is 0.0, cannot splash", "splashMode", splashMode);
				}
				value = value.getNumeric() / totalSumWeights;
				splashMode = SET_BASE;
			}
		}

		switch (splashMode) {
			case DEFAULT:
			{
				for (SubCubeList::const_iterator areaIt = reducedAreas.begin(); areaIt != reducedAreas.end(); ++areaIt) {
					if (useGpu) {
						pendingWrites.push_back(storageGpu->setCellValueBegin(areaIt->second, CellValue(factor), StorageBase::MULTIPLY_EXISTING));
					}
					storageCpu->setCellValue(areaIt->second, CellValue(factor), StorageBase::MULTIPLY_EXISTING);
				}
				break;
			}
			case ADD_BASE:
			case SET_BASE:
			{
				if (value.getNumeric() != 0) {
					double numCells = 0;
					for (SubCubeList::const_iterator area = reducedAreas.begin(); area != reducedAreas.end(); ++area) {
						numCells += area->second->getSize();
					}
					Logger::trace << "splashing: setting '" << numCells << "' cells" << endl;

					double megaBytes = 16 * numCells; // values in numeric storage
					if (fromMarkers.size()) {
//						megaBytes += ? // values in marker storage
						megaBytes += min(numCells, (double)maxNewMarkerCount) * (sizeof(double) + dimensions.size() * sizeof(IdentifierType)); // values in changedCells of marker storage
					}
					megaBytes /= 1024 * 1024;

					if (megaBytes > splashLimit1) {
						Logger::error << "palo will need about '" << megaBytes << "' mega-bytes for splashing" << endl;
						throw ParameterException(ErrorException::ERROR_SPLASH_NOT_POSSIBLE, "too many cells", "splashMode", splashMode);
					} else if (megaBytes > splashLimit2) {
						Logger::warning << "palo will need about '" << megaBytes << "' mega-bytes for splashing" << endl;
					} else if (megaBytes > splashLimit3) {
						Logger::info << "palo will need about '" << megaBytes << "' mega-bytes for splashing" << endl;
					} else {
						Logger::trace << "palo will need about '" << megaBytes << "' mega-bytes for splashing" << endl;
					}
				}

				if (checkMarkers && splashMode == SET_BASE) { //only for SET_BASE, the probability that ADD_BASE deletes a value is very small
					invalidateMarkers = checkMarkerInvalidation(reducedAreas);
				}
				for (SubCubeList::const_iterator areaIt = reducedAreas.begin(); areaIt != reducedAreas.end(); ++areaIt) {
					if (splashMode == ADD_BASE) {
						if (useGpu) {
							pendingWrites.push_back(storageGpu->setCellValueBegin(areaIt->second, value, StorageBase::ADD_ALL));
						}
						storageCpu->setCellValue(areaIt->second, value, StorageBase::ADD_ALL);
						if (!invalidateMarkers && !isZero) {
							checkFromMarkers(engineCpu.get(), areaIt->second, changedCubes);
						}
					} else {
						if (useGpu) {
							pendingWrites.push_back(storageGpu->setCellValueBegin(areaIt->second, value, StorageBase::SET));
						}
						storageCpu->setCellValue(areaIt->second, value, StorageBase::SET);
						if (!invalidateMarkers && !isZero) {
							checkFromMarkers(engineCpu.get(), areaIt->second, changedCubes);
						}
					}
				}
				break;
			}
			default:
				break;
		}
	} else {
		PSubCubeList lockedSubcubes = lockedCells ? lockedCells->getLockedAreas() : PSubCubeList();
		if (lockedSubcubes) {
			// check if cell is not inside any locked area
			for (SubCubeList::const_iterator area = lockedSubcubes->begin(); area != lockedSubcubes->end(); ++area) {
				if (area->second->isOverlapping(*(spCellPath))) {
					throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "restricted base area is empty (by restricted cells)", "path", cellPath->toString());
				}
			}
		}

		// base cell
		if (checkMarkers) {
			invalidateMarkers = checkMarkerInvalidation(cellPath, storageCpu.get());
		}
		if (useGpu) {
			pendingWrites.push_back(storageGpu->setCellValueBegin(spCellPath, value, isString || !addValue ? StorageBase::SET : StorageBase::ADD_ALL));
		}
		storageCpu->setCellValue(spCellPath, value, isString || !addValue ? StorageBase::SET : StorageBase::ADD_ALL);
		if (!invalidateMarkers && !isZero) {
			checkFromMarkers(engineCpu.get(), spCellPath, changedCubes);
		}
	}
	if (invalidateMarkers) {
		server->addChangedMarkerCube(cube, false);
	}

	// write back to journal
	if (journal && bWriteToJournal) {
		Area::PathIterator it = pathBegin;
		if (isString) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CELL_REPLACE_STRING);
			journal->appendIdentifiers((*it).begin(), (*it).end());
			journal->appendEscapeString(value);
		} else {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CELL_REPLACE_DOUBLE);
			journal->appendIdentifiers((*it).begin(), (*it).end());
			journal->appendInteger(splashMode);
			if (addValue && (splashMode == DISABLED || splashMode == DEFAULT)) {
				journal->appendDouble(origValue.getNumeric());
			} else {
				journal->appendDouble(value.getNumeric());
			}
			journal->appendBool(addValue);
			journal->appendPaths(lockedCells ? lockedCells->getLockedPaths() : CPPaths());
		}
		journal->nextLine();
	}

	// remember that this cube has changed since last load
	cellsStatus = CHANGED;
	additiveCommit = addComm;

	// invalidate client cache
	updateClientCacheToken();

	return RESULT_OK;
}

void Cube::commitChangesIntern(bool checkLocks, PUser user, bool disjunctive)
{
	Context *context = Context::getContext();
	PEngineBase engine = context->getServerCopy()->getEngine(EngineBase::CPU, true);
	PStorageBase storage = engine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
	PCellStream roll;
	roll = storage->commitChanges(checkLocks && hasLock && !wholeCubeLocked, false, false);
	checkValueLocks(roll, user, storage.get());
	storage = engine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
	roll = storage->commitChanges(checkLocks && hasLock && !wholeCubeLocked, additiveCommit, disjunctive);
	checkValueLocks(roll, user, storage.get());
	storage = engine->getCreateStorage(markerStorageId, pathTranslator, EngineBase::Marker);
	storage->commitChanges(false, false, false);
	// wait for all asynchronous operations to finish - can throw an exception
	try {
		for (AsyncResults::iterator asyncOp = pendingWrites.begin(); asyncOp != pendingWrites.end(); ++asyncOp) {
			(*asyncOp)->wait();
		}
	}
	catch (ErrorException &e){
		throw e;
	}
	catch (...)
	{
		saveType = NORMAL;
		PEngineBase gpuEngine = context->getServerCopy()->getEngine(EngineBase::GPU, true);
		if (gpuEngine) {
			gpuEngine->deleteStorage(numericStorageId);
		}
		Logger::info << "Exception! Gpu acceleration was deactivated for cube: " << getName() << endl;
	}
}

void Cube::checkValueLocks(PCellStream oldvals, PUser user, StorageBase *storage)
{
	if (oldvals) {
		StorageCpu *st = dynamic_cast<StorageCpu *>(storage);
		map<Lock *, vector<pair<IdentifiersType, CellValue> > > usedLocks;
		while (oldvals->next()) {
			PLock lock = lookupLockedArea(oldvals->getKey(), user);
			if (lock) {
				map<Lock *, vector<pair<IdentifiersType, CellValue> > >::iterator it = usedLocks.find(lock.get());
				if (it == usedLocks.end()) {
					it = usedLocks.insert(make_pair(lock.get(), vector<pair<IdentifiersType, CellValue> >())).first;
				}
				CellValue value;
				st->convertToCellValue(value, oldvals->getDouble());
				it->second.push_back(make_pair(oldvals->getKey(), value));
			}
		}
		for (map<Lock *, vector<pair<IdentifiersType, CellValue> > >::iterator it = usedLocks.begin(); it != usedLocks.end(); ++it) {
			PRollbackStorage rstorage = it->first->getStorage();
			if (rstorage) {
				rstorage->addCellValue(it->second);
			}
		}
	}
}

static ostream& operator<<(ostream& ostr, const IdentifiersType& v)
{
	bool first = true;
	ostr << dec;
	for (vector<IdentifierType>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if (!first) {
			ostr << ",";
		}
		if (*it == NO_IDENTIFIER) {
			ostr << "*";
		} else {
			ostr << *it;
		}
		first = false;
	}
	return ostr;
}

ResultStatus Cube::setCellValue(bool isBulk)
{
	checkCheckedOut();
	Context *context = Context::getContext();
	CPCellValueContext cvcSP = context->getCellValueContext();
	const CellValueContext *cvc = cvcSP.get();
	CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
	size_t dimCount = cube->getDimensions()->size();
	User::RightSetting rs(User::checkUser(cvc->user) ? User::checkCellDataRightCube(cvc->db, cube) : false);
	set<PCube> changedCubes;
	ResultStatus status = RESULT_FAILED;

	if (isBulk && journal) {
		journal->appendCommand(context->getServer()->getUsername(cvc->user), context->getServer()->getEvent(), JournalFileReader::JOURNAL_CELL_REPLACE_BULK_START);
		journal->nextLine();
	}

	bool disjunctive = isBulk && cvc->vElemTypes /*&& cvc->addValue*/;
	if (disjunctive) {
		const vector<vector<char> > &vElemTypes = *cvc->vElemTypes;
		map<IdentifiersType, pair<double, bool> > paths;
		CubeArea::CellType cellGroup = CubeArea::NONE;
		bool zeroGroup = false; // currently this group can contain only one cell

		size_t pos = 0;
		while (pos < cvc->pathVectorAndValue.size()) {
			bool multiple = false;
			size_t i;
			for (i = pos; i < cvc->pathVectorAndValue.size(); i++) {
				const CellValueContext::PathVectorAndValue &pvv = cvc->pathVectorAndValue[i];

				if (pvv.cellType != CubeArea::CONSOLIDATED) { // CubeArea::BASE_NUMERIC || CubeArea::BASE_STRING
					if (cellGroup == CubeArea::CONSOLIDATED) {
						cellGroup = pvv.cellType;
						break;
					}
				} else { // CubeArea::CONSOLIDATED
					if (cellGroup != CubeArea::CONSOLIDATED) {
						cellGroup = CubeArea::CONSOLIDATED;
						if (pos) {
							break;
						}
					}

					bool addToMap = true;
					if (i == pos) {
						zeroGroup = pvv.value.isEmpty();
					} else {
						if (!cvc->addValue && zeroGroup && (SplashMode)cvc->splashMode == DEFAULT) {
							break;
						}
						bool isZero = pvv.value.isEmpty();
						if (!cvc->addValue && isZero && (SplashMode)cvc->splashMode == DEFAULT) {
							break;
						}

						const IdentifiersType &prevPath = cvc->pathVectorAndValue[i - 1].cellPath;
						size_t j;
						for (j = 0; j < dimCount; j++) {
							if (pvv.cellPath[j] != prevPath[j] && (vElemTypes[j][i] != (char)Element::NUMERIC || vElemTypes[j][i - 1] != (char)Element::NUMERIC)) {
//								Logger::debug << "disjunct failed dim: " << j <<  " " << pvv.cellPath[j] << " " << prevPath[j] << " " << int(vElemTypes[j][i]) << " " << int(vElemTypes[j][i - 1]) << " key: " << pvv.cellPath << endl;
								break;
							}
						}
						if (j < dimCount) {
							addToMap = false;
						}
					}

					if (addToMap) {
						map<IdentifiersType, pair<double, bool> >::iterator it = paths.find(pvv.cellPath);
						if (it == paths.end()) {
							paths.insert(make_pair(pvv.cellPath, make_pair(pvv.value.getNumeric(), true)));
						} else {
							multiple = true;
							if (cvc->addValue) {
								it->second.first += pvv.value.getNumeric();
							} else {
								it->second.first = pvv.value.getNumeric(); // the last is the winner
							}
						}
					} else {
						break;
					}
				}
			}

			if (pos) {
				commitChangesIntern(true, cvc->user, true);
			}

			//values from <pos, i) are prepared
			for (size_t k = pos; k < i; k++) {
				const CellValueContext::PathVectorAndValue &pvv = cvc->pathVectorAndValue[k];
				rs.checkSepRight = pvv.sepRight;

				CellValue val;
				if (multiple) {
					map<IdentifiersType, pair<double, bool> >::iterator it = paths.find(pvv.cellPath);
					if (it == paths.end()) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid iterator in Cube::setCellValue!");
					}
					if (it->second.second) {
						val = it->second.first;
						it->second.second = false;
					} else {
						continue;
					}
				} else {
					val = pvv.value;
				}
				if (cvc->addValue && val.getNumeric() == 0.0) {
					status = RESULT_OK;
				} else {
					PCubeArea cp(new CubeArea(cvc->db, cube, pvv.cellPath));
					status = setCellValue(context->getServer(), cvc->db, cp, val, cvc->lockedCells, cvc->user, cvc->session, cvc->checkArea, cvc->addValue, (SplashMode)cvc->splashMode, true, &rs, changedCubes, false, pvv.cellType);
					if (status != RESULT_OK) {
						break;
					}
				}
			}
			if (status != RESULT_OK) {
				break;
			}
			pos = i;
		}
	} else {
		if (cvc->pathVectorAndValue.size()) {
			for (size_t i = 0; i < cvc->pathVectorAndValue.size(); i++) {
				const CellValueContext::PathVectorAndValue &pvv = cvc->pathVectorAndValue[i];
				rs.checkSepRight = pvv.sepRight;

				PCubeArea cp(new CubeArea(cvc->db, cube, pvv.cellPath));
				status = setCellValue(context->getServer(), cvc->db, cp, pvv.value, cvc->lockedCells, cvc->user, cvc->session, cvc->checkArea, cvc->addValue, (SplashMode)cvc->splashMode, true, &rs, changedCubes, true, pvv.cellType);
				if (status != RESULT_OK) {
					break;
				}
			}
		} else {
			for (vector<CellValueContext::PathAreaAndValue>::const_iterator it = cvc->pathAreaAndValue.begin(); it != cvc->pathAreaAndValue.end(); ++it) {
				CellValueContext::PathAreaAndValue pav = *it;
				rs.checkSepRight = pav.sepRight;

				status = setCellValue(context->getServer(), cvc->db, pav.cellPath, pav.value, cvc->lockedCells, cvc->user, cvc->session, cvc->checkArea, cvc->addValue, (SplashMode)cvc->splashMode, true, &rs, changedCubes, true, pav.cellType);
				if (status != RESULT_OK) {
					break;
				}
			}
		}
	}

	if (isBulk && journal) {
		journal->appendCommand(context->getServer()->getUsername(cvc->user), context->getServer()->getEvent(), JournalFileReader::JOURNAL_CELL_REPLACE_BULK_STOP);
		journal->nextLine();
	}

	if (status == RESULT_OK) {
		if (!wholeCubeLocked) {
			commitChanges(true, cvc->user, changedCubes, disjunctive);
		}
	}
	return status;
}

CellValue Cube::getCellValue(CPArea cellPath) const
{
	CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));
	PCubeArea cubeCellPath(new CubeArea(db, cube, *cellPath));
	PCellStream cs = calculateArea(cubeCellPath, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
	cs->next();
	return cs->getValue();
}

CellValue Cube::getCellValue(PCubeArea cellPath, bool bUseRules) const
{
	PCellStream cs = calculateArea(cellPath, CubeArea::ALL, bUseRules ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES, false, UNLIMITED_UNSORTED_PLAN);
	cs->next();
	return cs->getValue();
}

#ifdef SAVE_PLANS_PATH
	FileWriterTXT planWritter(FileName(SAVE_PLANS_PATH,"plans","txt"));
	bool planWritterOpen = false;
#endif

PPlanNode Cube::createPlan(PCubeArea area, CubeArea::CellType type, RulesType paramRulesType, bool skipEmpty, uint64_t blockSize) const
{
	Planner planner(CONST_COMMITABLE_CAST(Cube, shared_from_this()), area);
	PPlanNode pn = planner.createPlan(type, paramRulesType, skipEmpty, blockSize);
#ifdef SAVE_PLANS_PATH
	if (pn) {
		if (!planWritterOpen) {
			planWritter.openFile(true);
			planWritterOpen = true;
		}
		planWritter.appendPlan(pn);
		planWritter.nextLine();
	}
#endif
	if (pn && Logger::isTrace() /*&& pn->getChildren().size()*/) {
		Logger::trace << "Created Plan: " << pn->toXML() << endl;
	}
	return pn;
}

PProcessorBase Cube::evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const
{
	PEngineBase engine;
	if (engineType == EngineBase::ANY) {
		engine = ProcessorBase::selectEngine(plan);
	} else {
		Context *context = Context::getContext();
		PServer server = context->getServer();
		engine = server->getEngine(engineType);
	}
	if (!engine) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "no engine found!");
	}
	PProcessorBase result = engine->createProcessor(plan, sortedOutput);
	return result;

#ifdef undefined
	bool useGpu = (getType() == GPUTYPE && (engineType == EngineBase::ANY || engineType == EngineBase::GPU)) && context->canUseGPU();
//		original criteria from GPU server 3.2:
//        if ( currAreaSize  > 1000 || numberOfFilledCells < 100000 || (currAreaSize  > 1000 && numberOfFilledCells < 1000000)) {
//            fallBackToCpu = true;
//        }
	if (useGpu && (plan->getType() == AGGREGATION || plan->getType() == UNION)) {
		engine = server->getEngine(EngineBase::GPU);
		if (!engine || !engine->isPlanSupported(plan)) {
			useGpu = false; // Use Cpu
		}
	} else {
		useGpu = false; // Use Cpu
	}

	if (!engine || !useGpu) {
		engine = server->getEngine(EngineBase::CPU);
	}
	PProcessorBase result = engine->createProcessor(plan, sortedOutput);
	return result;
#endif
}

PCellStream Cube::calculateArea(PCubeArea area, CubeArea::CellType type, RulesType paramRulesType, bool skipEmpty, uint64_t blockSize) const
{
	PPlanNode plan = createPlan(area, type, paramRulesType, skipEmpty, blockSize);
	PCellStream result;
	if (plan) {
		result = evaluatePlan(plan, EngineBase::ANY, blockSize != UNLIMITED_UNSORTED_PLAN);
	}
	return result;
}

bool Cube::invalidateCache()
{
	bool res = cache.clear();
	if (res) {
		Logger::debug << "Cache of cube: " << getName() << " has been deleted!" << endl;
	}
	return res;
}

void Cube::cellGoalSeek(PServer server, PDatabase db, CellValueContext::GoalSeekType gsType, PCubeArea cellPath, PArea gsArea, PUser user, boost::shared_ptr<PaloSession> session, const double &value, bool useJournal)
{
	checkCheckedOut();
	Area::PathIterator pit = cellPath->pathBegin();
	CPCube thisCube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
	CubeArea::CellType type = cellPath->getType(pit);
	if (!(type & CubeArea::NUMERIC)) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "cannot goal seek string path", "destination element type", type);
	}

	vector<int> gsDim;
	ElementsType gsParent;
	vector<ElementsWeightType> gsElements;
	vector<vector<int> > gsElementIndex;
	vector<map<int, int> > gsElementIdentiferIndex;
	PCubeArea area(new CubeArea(db, thisCube, dimensions.size()));
	vector<IdentifiersWeightMap> siblings(dimensions.size());

	bool splash = false;
	for (size_t i = 0; i < dimensions.size(); i++) {
		PSet s(new Set);
		PDimension dim = db->lookupDimension(dimensions[i], false);
		Element *elem = dim->lookupElement((*pit)[i], false);
		const ElementsType p = dim->getParents(elem);
		// error if more than one parent
		if (p.size() > 1) {
			string parentsString;
			for (size_t j = 0; j < p.size(); j++) {
				if (j > 0) {
					parentsString += ", ";
				}
				parentsString += "'" + p[j]->getName(dim->getElemNamesVector()) + "'";
			}
			throw ErrorException(ErrorException::ERROR_GOALSEEK, "element '" + elem->getName(dim->getElemNamesVector()) + "' has multiple parents (" + parentsString + ")");
		} else if (p.size() == 1) { // one parent - check siblings
			const ElementsWeightType c = dim->getChildren(PUser(), p[0]);
			ElementsWeightType vc;
			vector<int> iv;
			map<int, int> iim;
			for (size_t j = 0; j < c.size(); j++) {
				if (c[j].first->getElementType() != Element::NUMERIC && c[j].first->getElementType() != Element::CONSOLIDATED) {
					continue;
				}
				if (c[j].first->getElementType() == Element::CONSOLIDATED) {
					splash = true;
				}

				//check for multiple parents
				const ElementsType cp = dim->getParents(c[j].first);
				if (cp.size() > 1) { // siblings cannot have multiple parents
					string parentsString;
					for (size_t k = 0; k < cp.size(); k++) {
						if (k > 0) {
							parentsString += ", ";
						}
						parentsString += "'" + cp[k]->getName(dim->getElemNamesVector()) + "'";
					}
					throw ErrorException(ErrorException::ERROR_GOALSEEK, "element '" + c[j].first->getName(dim->getElemNamesVector()) + "' has multiple parents (" + parentsString + ")");
				}

				if (gsType == CellValueContext::GS_COMPLETE) {
					vc.push_back(c[j]);
					iv.push_back(j);
					iim[c[j].first->getIdentifier()] = j;
					s->insert(c[j].first->getIdentifier());
				} else { // gsType == GS_EQUAL || gsType == GS_RELATIVE
					if (c[j].first->getIdentifier() == (*pit)[i]) {
						// path element itself, just add to area to compute original values
						s->insert(c[j].first->getIdentifier());
					} else if (!gsArea->getDim(i) || gsArea->getDim(i)->size() == 0) {
						// all siblings will be used for allocation
						siblings[i].insert(make_pair(c[j].first->getIdentifier(), c[j].second));
						s->insert(c[j].first->getIdentifier());
					} else if (gsArea->elemCount(i) == 0 || gsArea->find(i, c[j].first->getIdentifier()) != gsArea->elemEnd(i)) {
						siblings[i].insert(make_pair(c[j].first->getIdentifier(), c[j].second));
						s->insert(c[j].first->getIdentifier());
						// possible to throw error somewhere here if non-sibling identifiers in jobRequest->area found
					}
				}
			}
			if (gsType == CellValueContext::GS_COMPLETE && s->size() == 0) {
				throw ErrorException(ErrorException::ERROR_GOALSEEK, "dimension element has no numeric children");
			}
			gsElements.push_back(vc);
			gsElementIndex.push_back(iv);
			gsElementIdentiferIndex.push_back(iim);
			gsDim.push_back((int)i);
		} else { // no parents
			s->insert((*pit)[i]);
			splash |= (elem->getElementType() == Element::CONSOLIDATED);
		}
		area->insert(i, s);
	}

	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	User::RightSetting rs(useJournal ? User::checkCellDataRightCube(db, cube) : false);

	if (!gsDim.empty()) {
		if (gsType == CellValueContext::GS_COMPLETE) {
			// originals values have to be computed for old and equal algorithms
			if (area->getSize() > (uint32_t)goalseekCellLimit) {
				throw ErrorException(ErrorException::ERROR_GOALSEEK, "slice to big (" + StringUtils::convertToString(area->getSize()) + ", max allowed: " + StringUtils::convertToString(goalseekCellLimit) + ")");
			}

			// old "mathematical" goal seek algorithm - complete

			//define problem
			goalseeksolver::Problem p;
			vector<int> d;
			for (size_t i = 0; i < gsElements.size(); i++) {
				vector<double> w;
				for (size_t j = 0; j < gsElements[i].size(); j++)
					w.push_back(gsElements[i][j].second);
				p.dimensionElementWeight.push_back(w);
				d.push_back((int)w.size());
			}

			vector<int> coord;
			coord.resize(gsDim.size());
			for (size_t gsd = 0; gsd < gsDim.size(); gsd++) {
				int ind = gsElementIdentiferIndex[gsd][(*pit)[gsDim[gsd]]];
				coord[gsd] = ind;
			}

			p.fixedValue = value;
			p.fixedCoord = coord;

			p.cellValue = goalseeksolver::MDM<double>(d);

			PCellStream cs = calculateArea(area, CubeArea::NUMERIC, NO_RULES, true, UNLIMITED_UNSORTED_PLAN);
			while (cs->next()) {
				for (size_t gsd = 0; gsd < gsDim.size(); gsd++) {
					int ind = gsElementIdentiferIndex[gsd][cs->getKey()[gsDim[gsd]]];
					coord[gsd] = ind;
				}
				p.cellValue[coord] = cs->getValue().getNumeric();
			}

			goalseeksolver::Result res;
			try {
				res = goalseeksolver::solve(p, goalseekTimeoutMiliSec);
			} catch (goalseeksolver::CalculationTimeoutException&) {
				throw ErrorException(ErrorException::ERROR_GOALSEEK, "calculation takes too long");
			}

			if (res.valid) {
				// ignore rules, skip empty cells
				checkAreaAccessRight(db, user, area, rs, false, RIGHT_SPLASH, 0);
				cs = calculateArea(area, CubeArea::NUMERIC, NO_RULES, false, 0);

				set<PCube> changedCubes;
				while (cs->next()) {
					for (size_t gsd = 0; gsd < gsDim.size(); gsd++) {
						int ind = gsElementIdentiferIndex[gsd][cs->getKey()[gsDim[gsd]]];
						coord[gsd] = ind;
					}
					CellValue newValue(res.cellValue[coord]);
					setCellValue(server, db, PCubeArea(new CubeArea(db, cube, cs->getKey())), newValue, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
				}
				commitChanges(true, user, changedCubes, false);
			} else {
				throw ErrorException(ErrorException::ERROR_GOALSEEK, "could not find valid solution");
			}
		} else {
			// new goal-seek algorithm - equal or relative allocation
			cellGoalSeekEqualRelative(server, db, cellPath, user, session, value, siblings, gsType == CellValueContext::GS_EQUAL, rs);
		}

		if (journal != 0 && useJournal) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_CELL_GOALSEEK);
			journal->appendIdentifiers((*pit).begin(), (*pit).end());
			journal->appendInteger(gsType);
			journal->appendArea(gsArea);
			journal->appendDouble(value);
			journal->nextLine();
		}
	} else {
		set<PCube> changedCubes;
		setCellValue(server, db, cellPath, value, PLockedCells(), user, session, false, false, DEFAULT, true, &rs, changedCubes, false, CubeArea::NONE);
		commitChanges(true, user, changedCubes, false);
	}
}

void Cube::cellGoalSeekEqualRelative(PServer server, PDatabase db, PCubeArea cellPath, PUser user, boost::shared_ptr<PaloSession> session, double value, vector<IdentifiersWeightMap> &siblings, bool equal, User::RightSetting& rs)
{
	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	Area::PathIterator origPath = cellPath->pathBegin();
	PCellStream origValue = calculateArea(cellPath, CubeArea::NUMERIC, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
	origValue->next();
	CellValue storageValue = origValue->getValue();
	if (storageValue.isError()) {
		throw ErrorException(ErrorException::ERROR_GOALSEEK, ErrorException::getDescriptionErrorType(storageValue.getError()));
	}

	double dValueToAllocate = storageValue.getNumeric() - value;

	set<PCube> changedCubes;
	setCellValue(server, db, cellPath, value, PLockedCells(), user, session, false, false, DEFAULT, false, &rs, changedCubes, false, CubeArea::NONE);

	for (size_t i = 0; i < (*origPath).size(); i++) {
		if (siblings[i].size() == 0) {
			// no reallocation in this dimension
			continue;
		}

		double sum = 0;
		PCubeArea area(new CubeArea(*cellPath));
		PSet s(new Set);
		for (IdentifiersWeightMap::iterator sib = siblings[i].begin(); sib != siblings[i].end(); ++sib) {
			s->insert(sib->first);
			if (equal) {
				sum += sib->second;
			}
		}
		area->insert(i, s);

		checkAreaAccessRight(db, user, area, rs, false, RIGHT_SPLASH, 0);

		PPlanNode plan = createPlan(area, CubeArea::ALL, NO_RULES, false, UNLIMITED_UNSORTED_PLAN);
		PCellStream cs = evaluatePlan(plan, EngineBase::ANY, true);
		if (!equal) {
			while (cs->next()) {
				const CellValue &value = cs->getValue();
				IdentifiersWeightMap::iterator sib = siblings[i].find(cs->getKey()[i]);
				sum += value.getNumeric() * sib->second;
			}
			// Todo: -jj- temporary replacement for cs->reset
			cs = evaluatePlan(plan, EngineBase::ANY, true);
//			cs->reset();
		}
		if (sum == 0) {
			throw ErrorException(ErrorException::ERROR_GOALSEEK, "Sum of is zero, goal-seek aborted.");
		}

		PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
		PStorageBase storage = engine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		if (equal) {
			while (cs->next()) {
				double value = cs->getValue().getNumeric();
				CellValue newValue(value + dValueToAllocate / sum);
				setCellValue(server, db, PCubeArea(new CubeArea(db, cube, cs->getKey())), newValue, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
			}
		} else {
			while (cs->next()) {
				double value = cs->getValue().getNumeric();
				CellValue newValue(value + dValueToAllocate * value / sum);
				setCellValue(server, db, PCubeArea(new CubeArea(db, cube, cs->getKey())), newValue, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
			}
		}
	}
	commitChanges(true, user, changedCubes, false);
}

void Cube::cellGoalSeek()
{
	checkCheckedOut();
	Context *context = Context::getContext();
	CPCellValueContext cvc = context->getCellValueContext();
	CellValueContext::PathAreaAndValue pav = *cvc->pathAreaAndValue.begin();

	cellGoalSeek(context->getServer(), cvc->db, cvc->gsType, pav.cellPath, cvc->gsArea, cvc->user, cvc->session, pav.value.getNumeric(), true);
}

void Cube::updateClientCacheToken()
{
	clientCacheToken->getNewId();
}

void Cube::disableTokenUpdate()
{
	if (saveType != RIGHTS || dimensions.size() != 2 ) {
		Context::getContext()->setTokenUpdate(false);
	}
}

PRule Cube::createRule(PServer server, PDatabase db, PRuleNode node, const string& definition, const string& external, const string& comment, bool activate, PUser user, bool useJournal, IdentifierType *id, double position)
{
	checkCheckedOut();

	if (getStatus() == UNLOADED && useJournal) {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "cube", getId());
	}

	checkCubeRuleRight(user, RIGHT_WRITE);

	PRule rule = PRule(new Rule(node, db, COMMITABLE_CAST(Cube, shared_from_this()), definition, external, comment, ::time(NULL), activate));

	bool newId = true;
	if (id && *id != NO_IDENTIFIER) {
		rule->setID(*id);
		newId = false;
	}
	if (!rules->isCheckedOut()) {
		rules = COMMITABLE_CAST(RuleList, rules->copy());
	}
	rules->add(rule, newId);

	Context *context = Context::getContext();
	context->saveParent(shared_from_this(), rule);

	if (!newId) {
		IdentifierType last = rules->getLastId();
		IdentifierType idRule = rule->getId();
		if (last <= idRule) {
			rules->setNewIDStart(idRule + 1);
		}
	}
	if (!position) {
		position = rules->highestPosition()+1;
	}
	rule->setPosition(position);

	Logger::debug << "created new rule on cube " << this->getId() << " with id " << rule->getId() << ": "
	        << rule->getTextRepresentation(db, CONST_COMMITABLE_CAST(Cube, shared_from_this())) << ", (" << (rule->isActive() ? "enabled" : "disabled") << ")" << endl;

	updateClientCacheToken();

	// the marker areas will be updated later
	if (rule->isActive() && rule->hasMarkers()) {
		context->addNewMarkerRule(db->getId(), getId(), rule->getId());
	}
	context->eraseEngineCube(make_pair(db->getId(), getId()));

	if (useJournal && journal != 0) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_RULE_CREATE);
		journal->appendInteger(rule->getId());
		journal->appendEscapeString(definition);
		journal->appendEscapeString(external);
		journal->appendEscapeString(comment);
		journal->appendBool(activate);
		journal->nextLine();
	}

	rulesStatus = CHANGED;
	return rule;
}

bool Cube::modifyRule(PServer server, PDatabase db, PRule rule, PRuleNode node, const string& definition, const string& external, const string& comment, PUser user, ActivationType activation, bool useJournal, double position)
{
	checkCheckedOut();
	if (getStatus() == UNLOADED && useJournal) {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "cube", getId());
	}

	checkCubeRuleRight(user, RIGHT_WRITE);

	bool wasActive = rule->isActive();
	bool hadMarkers = rule->hasMarkers();
	bool defChanged = false;
	if (rule->getDefinition() != definition) {
		defChanged = true;
		rule->setDefinition(node, definition, db, COMMITABLE_CAST(Cube, shared_from_this()));
	}
	rule->setExternal(external);
	rule->setComment(comment);

	activateRules(server, db, vector<PRule>(1, rule), activation, user, NULL, true, false);

	Context *context = Context::getContext();
	if ((defChanged && (hadMarkers || rule->hasMarkers())) || (wasActive && activation!=ACTIVE && hadMarkers)) {
		// the marker areas will be updated later
		server->addChangedMarkerCube(COMMITABLE_CAST(Cube, shared_from_this()), true);
		context->addNewMarkerRule(db->getId(), getId(), rule->getId());
	}
	context->eraseEngineCube(make_pair(db->getId(), getId()));

	rulesStatus = CHANGED;

	if (useJournal && journal != 0) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_RULE_MODIFY);
		journal->appendInteger(rule->getId());
		journal->appendEscapeString(definition);
		journal->appendEscapeString(external);
		journal->appendEscapeString(comment);
		journal->appendInteger(activation);
		journal->nextLine();
	}

	return true;
}

bool Cube::activateRules(PServer server, PDatabase db, const vector<PRule> &rules, ActivationType activation, PUser user, string* errMsg, bool bDefinitionChangedBefore, bool useJournal)
{
	checkCheckedOut();
	if (getStatus() == UNLOADED && useJournal) {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "cube", getId());
	}

	checkCubeRuleRight(user, RIGHT_WRITE);

	if (!this->rules->isCheckedOut()) {
		this->rules = COMMITABLE_CAST(RuleList, this->rules->copy());
	}
	for (vector<PRule>::const_iterator ruleIt = rules.begin(); ruleIt != rules.end(); ++ruleIt) {
		PRule rule = *ruleIt;

		if ((activation == INACTIVE && !rule->isActive()) || (activation == ACTIVE && rule->isActive())) {
			if (bDefinitionChangedBefore) {
				Logger::debug << "updated rule on cube " << this->getId() << " with id " << rule->getId() << ": "
						<< rule->getTextRepresentation(db, CONST_COMMITABLE_CAST(Cube, shared_from_this())) << ", (" << (rule->isActive() ? "enabled" : "disabled") << ")" << endl;
			}
			continue; // nothing to do
		}

		if (!(*ruleIt)->isCheckedOut()) {
			rule = COMMITABLE_CAST(Rule, (*ruleIt)->copy());
			this->rules->set(rule);
		}

		bool newstate = activation == TOGGLE ? !rule->isActive() : activation == ACTIVE;

		rule->setActive(newstate, server, db, COMMITABLE_CAST(Cube, shared_from_this()), errMsg);

		Logger::debug << "updated rule on cube " << this->getId() << " with id " << rule->getId() << ": "
				<< rule->getTextRepresentation(db, CONST_COMMITABLE_CAST(Cube, shared_from_this())) << ", (" << (newstate ? "enabled" : "disabled") << ")" << endl;

		// the marker areas will be updated later
		Context *context = Context::getContext();
		if (newstate) {
			if (rule->hasMarkers()) {
				context->addNewMarkerRule(db->getId(), getId(), rule->getId());
			}
		} else {
			// if the rule had markers than addNewMarkerRule() has to be called, otherwise it doesn't matter
			context->addNewMarkerRule(db->getId(), getId(), rule->getId());
		}
		context->eraseEngineCube(make_pair(db->getId(), getId()));

		rulesStatus = CHANGED;

		if (useJournal && journal != 0) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_RULE_ACTIVATE);
			journal->appendInteger(rule->getId());
			journal->appendInteger(activation);
			journal->nextLine();
		}
	}

	return true;
}

bool Cube::setRulesPosition(PServer server, PDatabase db, const vector<PRule> movedRules, double startPosition, double belowPosition, PUser user, bool useJournal)
{
	if (movedRules.empty()) {
		return false;
	}

	checkCheckedOut();

	if (getStatus() == UNLOADED && useJournal) {
		throw ParameterException(ErrorException::ERROR_CUBE_NOT_LOADED, "cube not loaded", "cube", getId());
	}

	checkCubeRuleRight(user, RIGHT_WRITE);
	if (!this->rules->isCheckedOut()) {
		this->rules = COMMITABLE_CAST(RuleList, this->rules->copy());
	}

	double positionStep = 1;
	if (!startPosition) {
		startPosition = 1;
	}
	if (belowPosition) {
		positionStep = (belowPosition - startPosition)/movedRules.size();
	}
	double rulePosition = startPosition;
	for (vector<PRule>::const_iterator rit = movedRules.begin(); rit != movedRules.end(); ++rit, rulePosition+=positionStep) {
		PRule rule;
		if ((*rit)->isCheckedOut()) {
			rule = *rit;
		} else {
			rule = COMMITABLE_CAST(Rule, (*rit)->copy());
			this->rules->set(rule);
		}

		rule->setPosition(rulePosition);

		rulesStatus = CHANGED;

		if (useJournal && journal != 0) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_RULE_MOVE);
			journal->appendInteger((*rit)->getId());
			journal->appendDouble(rulePosition);
			journal->nextLine();
		}
	}
	return true;
}

void Cube::deleteRule(PServer server, PDatabase db, IdentifierType id, PUser user, bool useJournal)
{
	checkCheckedOut();
	checkCubeRuleRight(user, RIGHT_DELETE);

	if (!rules->isCheckedOut()) {
		rules = COMMITABLE_CAST(RuleList, rules->copy());
	}
	PRule rule = COMMITABLE_CAST(Rule, rules->get(id, true));

	if (NULL != rule) {

		rules->remove(rule->getId());

		StringBuffer *sb = new StringBuffer();
		rule->appendRepresentation(sb, db, CONST_COMMITABLE_CAST(Cube, shared_from_this()), true);
		Logger::info << "deleted rule on cube " << this->getId() << " with id " << rule->getId() << ": "
		        << sb->c_str() << ", (" << (rule->isActive() ? "enabled" : "disabled") << ")" << endl;
		delete sb;

		rulesStatus = CHANGED;

		updateClientCacheToken();

		if (rule->hasMarkers()) {
			rule->removeMarkers();
		}

	} else {
		throw ParameterException(ErrorException::ERROR_RULE_NOT_FOUND, "rule not found in cube", "cube", (int)getId(), "rule", (int)id);
	}
	Context::getContext()->eraseEngineCube(make_pair(db->getId(), getId()));

	if (!useJournal) {
		// we come from journal, no markers built yet, rule can be removed to prevent future error messages in commit marker calculation
		Context::getContext()->removeNewMarkerRuleJournal(db->getId(), getId(), rule->getId());
	}

	if (useJournal && journal != 0) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_RULE_DESTROY);
		journal->appendInteger(rule->getId());
		journal->nextLine();
	}
}

PRule Cube::findRule(IdentifierType id) const
{
	PRule rule = COMMITABLE_CAST(Rule, rules->get(id, false));

	if (NULL != rule) {
		Context::getContext()->saveParent(shared_from_this(), rule);
		return rule;
	} else {
		throw ParameterException(ErrorException::ERROR_RULE_NOT_FOUND, "rule not found in cube", "cube", (int)getId(), "rule", (int)id);
	}
}

////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////


bool Cube::deleteElement(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifierType element, CubeRulesArray *disabledRules, Dimension::DeleteCellType delType, bool completeRemove)
{
	PSet fullSet(new Set(true));
	return deleteElements(server, db, user, event, dimension, IdentifiersType(1, element), disabledRules, delType, fullSet, completeRemove);
}

bool Cube::deleteElements(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifiersType elements, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, PSet fullSet, bool completeRemove)
{
	checkCheckedOut();
	size_t dim = (size_t) - 1;
	size_t dimCount = dimensions.size();
	for (size_t i = 0; i != dimCount; i++) {
		if (dimensions[i] == dimension->getId()) {
			dim = i;
			break;
		}
	}
    bool dimFound = dim != (size_t) - 1;

	if (dimFound && elements.size() && (getStatus() != UNLOADED || Context::getContext()->getInJournal())) {
		updateClientCacheToken();

		if (additiveCommit) {
			commitChangesIntern(true, user, false);
			additiveCommit = false;
		}

		PSet strElemsToDelete = PSet(new Set());
		PSet numElemsToDelete = PSet(new Set());
		for (size_t i = 0; i < elements.size(); i++) {
			IdentifierType elemId = elements[i];

			if (elemId == (IdentifierType) - 1) {
				continue;
			}

			if (delType & Dimension::DEL_STR) {
				strElemsToDelete->insert(elemId);
			}
			if (delType & Dimension::DEL_NUM) {
				Element *element = dimension->findElement(elemId, 0, false);
				if (!Dimension::isStringElement(element->getElementType(), element->isStringConsolidation())) {
					numElemsToDelete->insert(elemId);
				}
			}

			// delete rules
			vector<PRule> vDisabledRules;
			if (!rules->isCheckedOut()) {
				rules = COMMITABLE_CAST(RuleList, rules->copy());
			}
			for (RuleList::Iterator iter = rules->begin(); iter != rules->end(); ++iter) {
				PRule rule = COMMITABLE_CAST(Rule, *iter);

				if (rule->isActive() && rule->hasElement(dimension, elemId)) {
					string errMsg;
					activateRules(server, db, vector<PRule>(1,rule), INACTIVE, PUser(), NULL, false, false);
					vDisabledRules.push_back(COMMITABLE_CAST(Rule, rules->get(rule->getId(), true)));
				}
			}
			if (disabledRules && vDisabledRules.size()) {
				disabledRules->push_back(pair<PCube, vector<PRule> >(COMMITABLE_CAST(Cube, shared_from_this()), vDisabledRules));
			}
		}

		PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
		PStorageBase numStorage = engine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		PStorageBase strStorage = engine->getCreateStorage(stringStorageId, pathTranslator, EngineBase::String);
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());

		// delete string values
		if (strElemsToDelete->size()) {
			PCubeArea area(new CubeArea(db, cube, dimCount));
			area->insert(dim, strElemsToDelete);

//			cout << "cube: " << getName() << ", strArea: " << *area << endl;
			if (delType == Dimension::DEL_STR && !completeRemove) {
				area.reset(new CubeArea(db, cube, *area->expandStar(CubeArea::NUMERIC_ELEMENTS)));
			} else { // Dimension::DEL_ALL || completeRemove
				area.reset(new CubeArea(db, cube, *area->expandStarOptim(fullSet)));
			}
//			cout << "cube: " << getName() << ", strArea: " << *area << endl;

			if (area->getSize()) {
				strStorage->setCellValue(area, CellValue::NullString, StorageBase::SET);
			}
		}

		// delete numeric values
		if (numElemsToDelete->size()) {
			PCubeArea area(new CubeArea(db, cube, dimCount));
			area->insert(dim, numElemsToDelete);

//			cout << "cube: " << getName() << ", numArea: " << *area << endl;
			area.reset(new CubeArea(db, cube, *area->expandStarOptim(fullSet)));
//			cout << "cube: " << getName() << ", numArea: " << *area << endl;

			if (area->getSize()) {
				numStorage->setCellValue(area, CellValue::NullNumeric, StorageBase::SET);
			}
		}

		commitChangesIntern(true, user, false);
		setStatus(CHANGED);
	}
	return dimFound;
}

void Cube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const
{
	if (defaultUsed) {
		*defaultUsed = false;
	}
	if (User::checkUser(user)) {
		user->checkAreaRightsComplete(db, CONST_COMMITABLE_CAST(Cube, shared_from_this()), area, rs, isZero, minimumRight, defaultUsed);
	}
}

RightsType Cube::getMinimumAccessRight(CPUser user) const
{
	RightsType ret = RIGHT_EMPTY;
	if (User::checkUser(user)) {
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		ret = user->getRDCDRight(db->getId(), getId()).first;
	}
	return ret;
}

void Cube::checkCubeAccessRight(PUser user, RightsType minimumRight, bool checkGroupCubeData, bool checkCubeRightObject) const
{
	if (User::checkUser(user)) {
		Context* context = Context::getContext();
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		CPDatabase db = CONST_COMMITABLE_CAST(Database, context->getParent(cube));

		User::MinMaxRight rtRole = user->getRoleCellDataRight();
		User::MinMaxRight rtDb;
		User::MinMaxRight rtCube;
		bool enough = rtRole.second >= minimumRight;
		if (enough && checkCubeRightObject) {
			enough = enough && user->getRoleRight(User::cubeRight) >= minimumRight;
		}
		if (enough) {
			rtDb = user->getDatabaseDataRight(db->getId());
			enough = enough && rtDb.second >= minimumRight;
		}
		if (enough && checkGroupCubeData) {
			rtCube = user->getCubeDataRight(db, getId());
			enough = enough && rtCube.second >= minimumRight;
		}
		if (enough) {
			bool checkRole = rtRole.first < minimumRight;
			bool checkDb = rtDb.first < minimumRight;
			bool checkCube = checkGroupCubeData ? rtCube.first < minimumRight : false;
			if (checkRole || checkCubeRightObject || checkDb || checkCube) {
				set<IdentifierType> userGroups;
				user->getUserGroupsCopy(userGroups);

				if (enough && checkRole) {
					enough = user->checkRoleRight(context->getServer()->getSystemDatabase(), userGroups, User::cellDataRight, minimumRight);
				}
				if (enough && checkCubeRightObject) {
					enough = user->checkRoleRight(context->getServer()->getSystemDatabase(), userGroups, User::cubeRight, minimumRight);
				}
				if (enough && checkDb) {
					enough = user->checkDatabaseDataRight(db, userGroups, minimumRight);
				}
				if (enough && checkCube) {
					enough = user->checkCubeDataRight(db, cube, userGroups, minimumRight);
				}
			}
		}
		if (!enough) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for cube", "user", (int)user->getId());
		}
	}
}

RightsType Cube::getCubeAccessRight(CPUser user) const
{
	RightsType res = RIGHT_SPLASH;
	if (User::checkUser(user)) {
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));
		vector<User::RoleDbCubeRight> vRights;
		user->fillRights(vRights, User::cellDataRight, db, cube);
		res = RIGHT_NONE;
		for (size_t i = 0; i < vRights.size(); i++) {
			RightsType rt;
			if (vRights[i].roleRight == RIGHT_SPLASH && vRights[i].dbRight >= RIGHT_WRITE && vRights[i].cubeRight >= RIGHT_WRITE) {
				rt = RIGHT_SPLASH;
			} else {
				rt = min(vRights[i].roleRight, min(vRights[i].dbRight, vRights[i].cubeRight));
			}
			if (rt > res) {
				res = rt;
				if (res == RIGHT_SPLASH) {
					break;
				}
			}
		}
	}
	return res;
}

void Cube::setWorkerAreas(const vector<string> &areaIdentifiers, const vector<PArea> &areas)
{
	checkCheckedOut();
	if (areas.size() == 0) {
		removeWorker();
		return;
	}

	if (areas.size() != areaIdentifiers.size()) {
		removeWorker();
		Logger::error << "Size of identifiers not equal to size of areas." << endl;
		return;
	}

	hasArea = true;
	workerAreas.clear();
	workerAreaIdentifiers = areaIdentifiers;

	workerAreas = areas;
}

bool Cube::isInArea(const Area *cellPath, string &areaIdentifier) const
{
	if (!hasArea) {
		return false;
	}

	vector<string>::const_iterator areaId = workerAreaIdentifiers.begin();
	vector<PArea>::const_iterator areaIter = workerAreas.begin();

	for (; areaIter != workerAreas.end(); ++areaIter, ++areaId) {
		bool in = isInArea(cellPath, (*areaIter).get());
		if (in) {
			Logger::trace << "request is in area '" << *areaId << "'" << endl;
			areaIdentifier = *areaId;
			return true;
		}
	}

	return false;
}

bool Cube::isInArea(const Area *cellPath, const Area *area)
{
	return isInArea(&(*cellPath->pathBegin())[0], area);
}

bool Cube::isInArea(const IdentifierType *cellPath, const Area *area)
{
	for (size_t i = 0; i != area->dimCount(); ++cellPath, i++) {
		CPSet s = area->getDim(i);
		if (s && s->size() > 0 && *cellPath != NO_IDENTIFIER) {

			// set is not empty (empty set means all elements are in the area)
			Set::Iterator element = s->find(*cellPath);

			// element not found in set so the cellPath is not in area
			if (element == s->end()) {
				return false;
			}
		}
	}

	return true;
}

PCube Cube::addCubeToDatabase(PServer server, dbID_cubeID db_cube)
{
	PDatabase db = server->lookupDatabase(db_cube.first, true);
	if (!db) {
		return PCube();
	}
	PDatabaseList dbs = server->getDatabaseList(true);
	server->setDatabaseList(dbs);
	dbs->set(db);

	PCube cube = db->lookupCube(db_cube.second, true);
	if (cube) {
		PCubeList cubes = db->getCubeList(true);
		db->setCubeList(cubes);
		cubes->set(cube);
	}

	return cube;
}

bool Cube::isSameCube(PCube lcube, PCube rcube, IdentifierType ldb, IdentifierType rdb)
{
	return lcube->getId() == rcube->getId() && ldb == rdb;
}

void Cube::executeShutdown()
{
	if (cubeWorker) {
		cubeWorker->notifyShutdown();
		cubeWorker.reset();
	}
}

void Cube::removeWorker()
{
	checkCheckedOut();
	if (cubeWorker) {
		Logger::trace << "removing worker of cube '" << getName() << "'" << endl;
		cubeWorker->releaseSession();
		cubeWorker.reset();
	}
}

PLock Cube::lockCube(PArea area, const string& areaString, bool whole, PUser user)
{
	checkCheckedOut();
	IdentifierType idUser = 0;
	if (user) {
		idUser = user->getId();
	}

	PLock lock = PLock(new Lock(COMMITABLE_CAST(Cube, shared_from_this()), area, areaString, whole, idUser));

	for (LockList::Iterator l = locks->begin(); l != locks->end(); ++l) {
		if (COMMITABLE_CAST(Lock, *l)->overlaps(lock->getContainsArea())) {
			throw ErrorException(ErrorException::ERROR_CUBE_WRONG_LOCK, "overlapping lock area");
		}
	}

	wholeCubeLocked = whole;
	locks = COMMITABLE_CAST(LockList, locks->copy());
	locks->add(lock, true);
	if (!wholeCubeLocked) {
		lock->createStorage(COMMITABLE_CAST(Cube, shared_from_this()), fileName);
	}

	hasLock = true;
	updateClientCacheToken();

	return lock;
}

void Cube::commitCube(long int id, PUser user)
{
	checkCheckedOut();
	IdentifierType idUser = 0;

	if (user) {
		idUser = user->getId();
	}

	PLock lock = COMMITABLE_CAST(Lock, locks->get(id, false));
	if (!lock) {
		throw ParameterException(ErrorException::ERROR_CUBE_LOCK_NOT_FOUND, "lock not found", "id", (int)id);
	}

	if (User::checkUser(user) && lock->getUserIdentifier() != idUser) {
		if (user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
			throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to unlock cube", "user", user->getName());
		}
	}

	locks = COMMITABLE_CAST(LockList, locks->copy());
	locks->remove(id);
	if (locks->size() == 0) {
		hasLock = false;
	}

	if (wholeCubeLocked) {
		set<PCube> changedCubes;
		commitChanges(false, user, changedCubes, false);
		wholeCubeLocked = false;
		Context::getContext()->getServerCopy()->addChangedMarkerCube(CONST_COMMITABLE_CAST(Cube, shared_from_this()), true);
	}

	updateClientCacheToken();
}

void Cube::rollbackCube(PServer server, PDatabase db, long int id, PUser user, size_t numSteps)
{
	checkCheckedOut();

	if (wholeCubeLocked) {
		throw ErrorException(ErrorException::ERROR_CUBE_BLOCKED_BY_LOCK, "Exclusive lock can't be rolled back.");
	}

	IdentifierType idUser = 0;

	if (user) {
		idUser = user->getId();
	}

	PLock lock = COMMITABLE_CAST(Lock, locks->get(id, false));
	if (!lock) {
		throw ParameterException(ErrorException::ERROR_CUBE_LOCK_NOT_FOUND, "lock not found", "id", (int)id);
	}

	if (User::checkUser(user) && lock->getUserIdentifier() != idUser) {
		if (user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
			throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to unlock cube", "user", user->getName());
		}
	}

	PRollbackStorage rstorage = lock->getStorage();

	// rollback
	if (numSteps == 0) {
		rstorage->rollback(server, db, COMMITABLE_CAST(Cube, shared_from_this()), rstorage->getNumberSteps(), user);

		locks = COMMITABLE_CAST(LockList, locks->copy());
		locks->remove(id);
		if (locks->size() == 0) {
			hasLock = false;
		}
	} else {
		rstorage->rollback(server, db, COMMITABLE_CAST(Cube, shared_from_this()), numSteps, user);
	}

	updateClientCacheToken();
}

CPLockList Cube::getCubeLocks(PUser user) const
{
	return locks;
}

PLock Cube::lookupLockedArea(const IdentifiersType &key, PUser user)
{
	if (!hasLock) {
		return PLock();
	}

	IdentifierType idUser = 0;
	if (user) {
		idUser = user->getId();
	}

	for (LockList::Iterator l = locks->begin(); l != locks->end(); ++l) {
		PLock lock = COMMITABLE_CAST(Lock, *l);
		if (lock->contains(key)) {

			if (lock->getUserIdentifier() != idUser) {
				// wrong user for lock
				if (user) {
					throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to write to locked area", "user", user->getName());
				} else {
					throw ParameterException(ErrorException::ERROR_CUBE_WRONG_USER, "wrong user to write to locked area", "user", "");
				}
			}

			if (lock->isWhole()) {
				lock.reset();
			}

			return lock;
		} else if (lock->blocks(key)) {
			throw ErrorException(ErrorException::ERROR_CUBE_BLOCKED_BY_LOCK, "cannot splash because of a locked area");
		}
	}

	return PLock();
}

PLock Cube::findCellLock(const IdentifiersType &key) const
{
	if (!hasLock) {
		return PLock();
	}
	for (LockList::Iterator l = locks->begin(); l != locks->end(); ++l) {
		PLock lock = COMMITABLE_CAST(Lock, *l);
		if (lock->contains(key)) {
			return lock;
		}
	}
	return PLock();
}

Cube::CellLockInfo Cube::getCellLockInfo(const IdentifiersType &key, IdentifierType userId) const
{
	CPLock l = findCellLock(key);
	if (l != 0) {
		if (l->getUserIdentifier() == userId)
			return 1;
		else
			return 2;
	}
	return 0;
}

PCubeWorker Cube::getCubeWorker() const
{
	return cubeWorker;
}

PCubeWorker Cube::createCubeWorker()
{
	if (CubeWorker::useCubeWorker()) {
		boost::shared_ptr<PaloSession> session = PaloSession::createSession(PUser(), true, 0, Context::getContext()->getServer()->useShortSid(), false, "worker", 0, 0, 0, getName()+" worker", "");
		cubeWorker = PCubeWorker(new CubeWorker(session->getSid(), false));
	}

	return cubeWorker;
}

void Cube::checkCubeRuleRight(PUser user, RightsType minimumRight) const
{
	if (User::checkUser(user)) {
		Context* context = Context::getContext();
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		CPDatabase db = CONST_COMMITABLE_CAST(Database, context->getParent(cube));

		User::MinMaxRight rtDb = user->getDatabaseDataRight(db->getId());
		bool enough = rtDb.second >= minimumRight;
		if (enough) {
			bool checkDb = rtDb.first < minimumRight;
			set<IdentifierType> userGroups;
			user->getUserGroupsCopy(userGroups);

			if (enough) {
				enough = user->checkRoleRight(context->getServer()->getSystemDatabase(), userGroups, User::ruleRight, minimumRight);
			}
			if (enough && checkDb) {
				enough = user->checkDatabaseDataRight(db, userGroups, minimumRight);
			}
		}
		if (!enough) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
		}
	}
}

CubeList::CubeList(const CubeList &l) :
	CommitableList(l)
{
}

PCommitableList CubeList::createnew(const CommitableList& l) const
{
	return PCommitableList(new CubeList(dynamic_cast<const CubeList &>(l)));
}

bool Cube::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);

	CPCube cube = CONST_COMMITABLE_CAST(Cube, o);
	CPCube oldcube = CONST_COMMITABLE_CAST(Cube, old);
	CPDatabase db = (CONST_COMMITABLE_CAST(Database, p));

	if (cube != 0) {
		if (cube->wholeCubeLocked && !oldcube->wholeCubeLocked) {
			ret = false;
		}
		if (wholeCubeLocked && locks != cube->locks && cube->locks->size()) {
			ret = false;
		}
	}

	Context *context = Context::getContext();
	if (cube != 0 && context->doTokenUpdate()) {
		token = cube->token + 1;
		if (!token) {
			++token;
		}
	}

	if (ret && !oldcube) {
		CPDatabase db = (CONST_COMMITABLE_CAST(Database, p));
		ret = checkDimensions(db);
	}

	if (ret) {
		if (locks->isCheckedOut()) {
			ret = locks->merge(o != 0 ? cube->locks : PLockList(), shared_from_this());
		} else {
			if (cube) {
				locks = cube->locks;
			}
		}
	}

	if (ret) {
		if (rules->isCheckedOut()) {
			ret = rules->merge(o != 0 ? cube->rules : PRuleList(), shared_from_this());
		} else {
			if (cube) {
				rules = cube->rules;
			}
		}
	}

	if (ret && oldcube != 0 && cube != 0) {
		if (deletable == oldcube->deletable) {
			deletable = cube->deletable;
		}
		if (renamable == oldcube->renamable) {
			renamable = cube->renamable;
		}
		if (cubeWorker == oldcube->cubeWorker) {
			cubeWorker = cube->cubeWorker;
		}
		if (hasArea == oldcube->hasArea) {
			hasArea = cube->hasArea;
		}
		if (workerAreaIdentifiers == oldcube->workerAreaIdentifiers) {
			workerAreaIdentifiers = cube->workerAreaIdentifiers;
		}
		if (workerAreas == oldcube->workerAreas) {
			workerAreas = cube->workerAreas;
		}
		if (workerAreas == oldcube->workerAreas) {
			workerAreas = cube->workerAreas;
		}
		if (cellsStatus == oldcube->cellsStatus) {
			cellsStatus = cube->cellsStatus;
		}
		if (rulesStatus == oldcube->rulesStatus) {
			rulesStatus = cube->rulesStatus;
		}
		if (fileName == oldcube->fileName) {
			fileName = cube->fileName;
		}
		if (ruleFileName == oldcube->ruleFileName) {
			ruleFileName = cube->ruleFileName;
		}
		if (journalFile == oldcube->journalFile) {
			journalFile = cube->journalFile;
		}
		if (hasLock == oldcube->hasLock) {
			hasLock = cube->hasLock;
		}
		if (fromMarkers == oldcube->fromMarkers) {
			fromMarkers = cube->fromMarkers;
		}
		if (toMarkers == oldcube->toMarkers) {
			toMarkers = cube->toMarkers;
		}
		if (additiveCommit == oldcube->additiveCommit) {
			additiveCommit = cube->additiveCommit;
		}
		if (delCount == oldcube->delCount) {
			delCount = cube->delCount;
		}
	}
	if (ret && cube != 0 && context->doTokenUpdate()) {
		CPDatabase db = CONST_COMMITABLE_CAST(Database, p);
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		// call onCubeChange on all rules
		for (RuleList::Iterator it = rules->begin(); it != rules->end(); ++it) {
			PRule rule = COMMITABLE_CAST(Rule, *it);
			rule->onCubeChange(db, cube);
		}
	}
	if (ret) {
		commitintern();
	}
	return ret;
}

bool Cube::checkBeforeInsertToMergedList(const CPCommitable &parent)
{
	CPDatabase db = (CONST_COMMITABLE_CAST(Database, parent));
	return checkDimensions(db);
}

bool Cube::checkDimensions(CPDatabase db)
{
	bool ret = true;
	for (IdentifiersType::iterator it = dimensions.begin(); it != dimensions.end(); ++it) {
		if (!db->lookupDimension(*it, false)) {
			ret = false;
			break;
		}
	}
	return ret;
}

PCommitable Cube::copy() const
{
	checkNotCheckedOut();
	PCube newd(new Cube(*this));
	return newd;
}

void Cube::notifyAddCube(PServer server, PDatabase database, IdentifierType *newCubeDimElem, IdentifierType *newCellRightCube, IdentifierType *newCellPCube, bool useDimWorker)
{
	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);

	if (db == 0) {
		return;
	}

	// add cube to list of cubes in #_CUBE_
	PDimension cubeDimension = db->getCubeDimension();
	Element *cubeDimElem = cubeDimension->addElement(server, database, (newCubeDimElem ? *newCubeDimElem : NO_IDENTIFIER), getName(), Element::STRING, PUser(), false);
	boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
	if (useDimWorker && (!session || !session->isWorker())) {
		cubeDimension->addElementEvent(server, database, cubeDimElem);
	}

	// create #_GROUP_CELL_DATE_<cube> cube
	PNormalDatabase pndb = COMMITABLE_CAST(NormalDatabase, database);
	string cubeName = SystemCube::PREFIX_GROUP_CELL_DATA + getName();
	IdentifiersType vDims;
	PDimension groupDimension = database->findDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, PUser(), false);
	vDims.push_back(groupDimension->getId());
	for (IdentifiersType::const_iterator i = dimensions.begin(); i != dimensions.end(); ++i) {
		vDims.push_back(*i);
	}

	PRightsCube rightsCube = PRightsCube(new RightsCube(database, cubeName, &vDims));
	rightsCube->setDeletable(false);
	rightsCube->setRenamable(false);
	bool newId = true;
	if (newCellRightCube && *newCellRightCube != NO_IDENTIFIER) {
		rightsCube->setID(*newCellRightCube);
		newId = false;
	}

	pndb->addCube(server, rightsCube, false, newId, NULL, NULL, NULL, useDimWorker);

	cubeName = SystemCube::PREFIX_CELL_PROPS_DATA + getName();
	vDims.clear();
	for (IdentifiersType::const_iterator i = dimensions.begin(); i != dimensions.end(); ++i) {
		vDims.push_back(*i);
	}
	PDimension cellPropDim = database->findDimensionByName(SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION, PUser(), false);
	vDims.push_back(cellPropDim->getId());
	PAttributesCube cellPropsAttrCube = PAttributesCube(new AttributesCube(db, cubeName, &vDims, false));
	cellPropsAttrCube->setDeletable(false);
	cellPropsAttrCube->setRenamable(false);
	newId = true;
	if (newCellPCube && *newCellPCube != NO_IDENTIFIER) {
		cellPropsAttrCube->setID(*newCellPCube);
		newId = false;
	}
	pndb->addCube(server, cellPropsAttrCube, false, newId, NULL, NULL, NULL, useDimWorker);

	if (newCubeDimElem) {
		*newCubeDimElem = cubeDimElem->getIdentifier();
	}
	if (newCellRightCube) {
		*newCellRightCube = rightsCube->getId();
	}
	if (newCellPCube) {
		*newCellPCube = cellPropsAttrCube->getId();
	}
}

void Cube::notifyRemoveCube(PServer server, PDatabase database, bool useDimWorker)
{
	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);

	if (db == 0) {
		return;
	}

	// delete #_GROUP_CELL_DATE_<cube> cube
	PCube cube = database->lookupCubeByName(SystemCube::PREFIX_GROUP_CELL_DATA + getName(), true);
	if (cube) {
		cube->setDeletable(true);
		database->deleteCube(server, cube, PUser(), false, useDimWorker);
	}
	cube = database->lookupCubeByName(SystemCube::PREFIX_CELL_PROPS_DATA + getName(), true);
	if (cube) {
		cube->setDeletable(true);
		database->deleteCube(server, cube, PUser(), false, useDimWorker);
	}

	PDimension cubeDimension = db->getCubeDimension();
	Element* element = cubeDimension->lookupElementByName(getName(), true);

	if (element != 0) {
		cubeDimension->deleteElement(server, database, element, PUser(), false, NULL, useDimWorker);
	}
}

void Cube::notifyRenameCube(PServer server, PDatabase database, const string& oldName, bool useDimWorker)
{
	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);

	if (db == 0) {
		Logger::error << "normal database is abnormal" << endl;
		return;
	}

	// rename #_GROUP_CELL_DATE_<cube> cube
	string oldCubeName = SystemCube::PREFIX_GROUP_CELL_DATA + oldName;
	string newCubeName = SystemCube::PREFIX_GROUP_CELL_DATA + getName();
	PCubeList cl = database->getCubeList(true);
	database->setCubeList(cl);
	PCube cube = database->lookupCubeByName(oldCubeName, true);
	if (cube) {
		cl->set(cube);
		database->renameCube(server, cube, newCubeName, false, useDimWorker);
	}
	oldCubeName = SystemCube::PREFIX_CELL_PROPS_DATA + oldName;
	newCubeName = SystemCube::PREFIX_CELL_PROPS_DATA + getName();
	cube = database->lookupCubeByName(oldCubeName, true);
	if (cube) {
		cl->set(cube);
		database->renameCube(server, cube, newCubeName, false, useDimWorker);
	}

	PDimension cubeDimension = db->getCubeDimension();
	Element* element = cubeDimension->lookupElementByName(oldName, true);

	if (element != 0) {
		cubeDimension->changeElementName(server, database, element, getName(), PUser(), false, useDimWorker);
	}
}

void Cube::removeFromMarker(PRuleMarker marker)
{
	checkCheckedOut();
	Logger::trace << "removing from " << *marker << " from cube '" << getName() << "'" << endl;

	fromMarkers.erase(marker);
}

void Cube::removeToMarker(PServer server, PRuleMarker marker)
{
	checkCheckedOut();
	Logger::trace << "removing to " << *marker << " from cube '" << getName() << "'" << endl;

	toMarkers.erase(marker);

	// we have to clear the markers and rebuild them
	server->addChangedMarkerCube(COMMITABLE_CAST(Cube, shared_from_this()), true);
}

void Cube::addFromMarker(PRuleMarker marker, set<PCube> *changedCubes)
{
	checkCheckedOut();
	if (changedCubes) {
		Logger::trace << "adding from " << *marker << " to cube '" << getName() << "'" << endl;

		Context* context = Context::getContext();
		PServer server = context->getServerCopy();
		dbID_cubeID toDbCube = marker->getToDbCube();
		PDatabase toDB = server->lookupDatabase(toDbCube.first, false);
		PCube toCube = toDB->lookupCube(toDbCube.second, false);

		vector<Dimension*> dimensions;

		if (marker->isMultiplicating()) {
			const IdentifiersType* dimIds = toCube->getDimensions();
			const int16_t* perms = marker->getPermutations();
			for(IdentifiersType::const_iterator idsIt = dimIds->begin(); idsIt != dimIds->end(); ++idsIt, perms++) {
				if (*perms == MarkerStorage::ALL_ELEMENTS) {
					dimensions.push_back(toDB->lookupDimension(*idsIt, false).get());
				} else {
					dimensions.push_back((Dimension*)0);
				}
			}
		}

		MarkerStorage ms(marker, toCube->getDimensions()->size(), marker->isMultiplicating() ? &dimensions : 0);
		ms.generateMarkers(COMMITABLE_CAST(Cube, shared_from_this()), marker->getFromBase().get());

		const MarkerStorage::PMarkerSet markers = ms.getMarkers();
		Logger::debug << "found " << markers->size() << " marked cells from cube: " << getName() << " to cube: " << toCube->getName() << endl;

		// and set these values as new cells
		if (markers->size()) {
			const MarkerListFilter *oldFilter = 0;
			MarkerListFilter markerFilter(toDbCube.first, toDbCube.second);

			toCube = addCubeToDatabase(server, toDbCube);

			bool prefilteredMarkers = false;
			// filter possible intersections of marker target with fromMarkers and store their pointers in context for better performance
			if (markers->size() > 100 && toCube->fromMarkers.size()) {
				markerFilter.filter.reserve(toCube->fromMarkers.size());

				for (RuleMarkerSet::iterator i = toCube->fromMarkers.begin(); i != toCube->fromMarkers.end(); ++i) {
					const RuleMarker *fromMarker = i->get();

					if (fromMarker != marker.get() && isInArea(marker->getFixed(), fromMarker->getFromBase().get())) {
						markerFilter.filter.push_back(fromMarker);
					}
				}
	//			Logger::debug << "addFromMarker filtering: " << markerFilter.filter.size() << " of total " << fromMarkers.size() << " will be used" << endl;
				prefilteredMarkers = true;
				oldFilter = context->setMarkerListFilter(&markerFilter);
			}

			PEngineBase engine = server->getEngine(EngineBase::CPU, true);
			PStorageBase storage = engine->getCreateStorage(toCube->markerStorageId, toCube->pathTranslator, EngineBase::Marker);
			MarkerStorageCpu *st = dynamic_cast<MarkerStorageCpu*>(storage.get());

			PCellStream cs = markers->getValues();
			while (cs->next()) {
				toCube->setCellMarker(engine.get(), st, cs->getKey(), *changedCubes);
			}

			if (prefilteredMarkers) {
				// remove filtered collection of fromMarkers from context
				context->setMarkerListFilter(oldFilter);
			}
		}
	}

	fromMarkers.insert(marker);
}

void Cube::addToMarker(PRuleMarker marker)
{
	checkCheckedOut();
	Logger::trace << "adding to " << *marker << " to cube '" << getName() << "'" << endl;

	// keep a list of active "to" markers
	toMarkers.insert(marker);
}

void Cube::checkFromMarkers(EngineBase *engine, const IdentifiersType &key, set<PCube> &changedCubes)
{
	if (fromMarkers.empty()) {
		return;
	}

	Context* context = Context::getContext();
	PServer server = context->getServer();
	PCube fromCube = COMMITABLE_CAST(Cube, shared_from_this());
	CPDatabase db = CONST_COMMITABLE_CAST(Database, context->getParent(shared_from_this()));

	RuleMarkerSet::iterator i;
	vector<const RuleMarker*>::const_iterator fi;

	const MarkerListFilter* filteredList = context->getMarkerListFilter();
	if (filteredList && (filteredList->dbId != db->getId() || filteredList->cubeId != fromCube->getId())) {
		filteredList = 0;
	}
	if (filteredList) {
		fi = filteredList->filter.begin();
	} else {
		i = fromMarkers.begin();
	}

	while (filteredList ? fi != filteredList->filter.end() : i != fromMarkers.end()) {
		const RuleMarker* marker = filteredList ? *fi : i->get();
		if (filteredList) {
			++fi;
		} else {
			++i;
		}

		if (isInArea(&key[0], marker->getFromBase().get())) {
			dbID_cubeID toDbCube = marker->getToDbCube();
			PDatabase toDB = server->lookupDatabase(toDbCube.first, false);
			PCube toCube = toDB->lookupCube(toDbCube.second, false);
			size_t nd = toCube->getDimensions()->size();
			IdentifiersType b(nd);

			vector<Dimension*> dimensions;
			const vector<Dimension*>* pDimensions = 0;

			if (marker->isMultiplicating()) {
				const IdentifiersType* dimIds = toCube->getDimensions();
				const int16_t* perms = marker->getPermutations();
				for(IdentifiersType::const_iterator idsIt = dimIds->begin(); idsIt != dimIds->end(); ++idsIt, perms++) {
					if (*perms == MarkerStorage::ALL_ELEMENTS) {
						dimensions.push_back(toDB->lookupDimension(*idsIt, false).get());
					} else {
						dimensions.push_back((Dimension*)0);
					}
				}
				pDimensions = &dimensions;
			}

			const int16_t* perm = marker->getPermutations();
			const uint32_t* fix = marker->getFixed();
			const RuleMarker::PMappingType* maps = marker->getMapping();

			uint32_t* path = (uint32_t*)(&key[0]);

			MarkerMappingIterator mmi(nd, perm, maps, path, pDimensions);
			int combinations = 0;

			uint32_t* ptr = (uint32_t*)(&b[0]);
			uint32_t* end = ptr + nd;

			do {
				if (!maps && !pDimensions) {
					for (; ptr < end; ptr++, perm++, fix++) {
						if (*perm != MarkerStorage::FIXED_ELEMENT) {
							*ptr = path[*perm];
						} else {
							*ptr = *fix;
						}
					}
				} else {
					if (!mmi.init()) {
						// no combination -> continue with next marker
						continue;
					}

					// generate all combinations
					for (size_t targetDim = 0; targetDim < nd; targetDim++) {
						int16_t sourceDim = perm[targetDim];
						if (sourceDim != MarkerStorage::FIXED_ELEMENT) {
							ptr[targetDim] = mmi[targetDim];
						} else {
							ptr[targetDim] = fix[targetDim];
						}
					}

					++mmi; // next combination
				}
				toCube = addCubeToDatabase(server, toDbCube);
				PStorageBase storage = engine->getCreateStorage(toCube->markerStorageId, toCube->pathTranslator, EngineBase::Marker);
				MarkerStorageCpu *st = dynamic_cast<MarkerStorageCpu*>(storage.get());

				const MarkerListFilter* oldFilteredList = context->setMarkerListFilter(0);
				toCube->setCellMarker(engine, st, b, changedCubes);
				context->setMarkerListFilter(oldFilteredList);
			} while (!mmi.isEndOfCombinations());
			if (combinations > 1) {
				//int breakHere = 1;
				//cout << "combinations = " << combinations << endl;
			}
		}
	}
}

bool Cube::hitMarkerRebuildLimit()
{
	checkCheckedOut();
	PServer server = Context::getContext()->getServerCopy();
	if (server->enforceBuildMarkers()) {
		delCount = 0;
		return true;
	} else {
		StorageCpu *st = dynamic_cast<StorageCpu *>(server->getEngine()->getStorage(numericStorageId).get());
		delCount += st->getLastDeletionCount();
		return delCount >= markerRebuildLimit;
	}
}

void Cube::clearAllMarkers()
{
	checkCheckedOut();
	PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
	engine->recreateStorage(markerStorageId, pathTranslator, EngineBase::Marker);
}

void Cube::rebuildAllMarkers()
{
	checkCheckedOut();
	Logger::trace << "rebuild all markers for cube '" << getName() << "'" << endl;

	RuleMarkerSet markers;
	markers.swap(fromMarkers);

	set<PCube> changedCubes;
	for (RuleMarkerSet::iterator i = markers.begin(); i != markers.end(); ++i) {
		addFromMarker(*i, &changedCubes);
	}
	delCount = 0;

	commitChanges(false, PUser(), changedCubes, false);
}

const Cube::RuleMarkerSet& Cube::getFromMarkers() const
{
	return fromMarkers;
}

const Cube::RuleMarkerSet& Cube::getToMarkers() const
{
	return toMarkers;
}

void Cube::setCellMarker(EngineBase *engine, MarkerStorageCpu *storage, const IdentifiersType &key, set<PCube> &changedCubes)
{
	if (storage->setMarker(key)) {
		PDoubleCellMap changedCells = storage->getChangedCells();
		if (changedCells && changedCells->size() >= maxNewMarkerCount) {
			Logger::trace << "commitChanges started,  storage: " << storage->valuesCount() << ", changed cells: " << changedCells->size() << endl;
			storage->commitChanges(false, false, false);
			Logger::trace << "commitChanges finished, storage: " << storage->valuesCount() << endl;
		}

		changedCubes.insert(COMMITABLE_CAST(Cube, shared_from_this()));
		checkFromMarkers(engine, key, changedCubes);
	}
}

void Cube::checkFromMarkers(EngineBase *engine, CPArea area, set<PCube> &changedCubes)
{
	if (!wholeCubeLocked && !fromMarkers.empty()) {
		for (Area::PathIterator it = area->pathBegin(); it != area->pathEnd(); ++it) {
			checkFromMarkers(engine, *it, changedCubes);
		}
	}
}

bool Cube::checkMarkerInvalidation(const Area *area, StorageBase *storageCpu) const
{
	bool result = false;
	size_t size = area->getSize();
	for (RuleMarkerSet::iterator it = fromMarkers.begin(); it != fromMarkers.end(); ++it) {
		const RuleMarker *fromMarker = it->get();
		if (size == 1) {
			if (isInArea(area, fromMarker->getFromBase().get())) {
				result = true;
				break;
			}
		} else {
			if (fromMarker->getFromBase()->isOverlapping(*area)) {
				result = true;
				break;
			}
		}
	}
	if (result && storageCpu) {
		CellValue val = storageCpu->getCellValue(*area->pathBegin());
		result = !val.isEmpty();
	}
	return result;
}

bool Cube::checkMarkerInvalidation(SubCubeList &areas) const
{
	for (SubCubeList::const_iterator it = areas.begin(); it != areas.end(); ++it) {
		if (checkMarkerInvalidation(it->second.get(), 0)) {
			return true;
		}
	}
	return false;
}

#ifdef ENABLE_GPU_SERVER
bool Cube::optimizeNumericStorage(PEngineBase engine)
{
	if(getType() == GPUTYPE){
		PStorageBase storageBase = engine->getCreateStorage(numericStorageId, pathTranslator, EngineBase::Numeric);
		PGpuStorage storageGpu = COMMITABLE_CAST(GpuStorage, storageBase);
		return storageGpu->optimize();
	}
	return false;
}
#endif

}
