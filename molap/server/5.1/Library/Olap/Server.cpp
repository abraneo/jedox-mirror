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

#include "Olap/Server.h"

#include <algorithm>
#include <iostream>

#include "Exceptions/FileFormatException.h"
#include "Exceptions/ParameterException.h"

#include "Collections/DeleteObject.h"
#include "Collections/StringBuffer.h"

#include "InputOutput/FileReader.h"
#include "InputOutput/FileReaderTXT.h"
#include "InputOutput/FileWriter.h"
#include "InputOutput/ProgressCallback.h"
#include "InputOutput/Statistics.h"
#include "InputOutput/ZipUtils.h"

#include "Logger/Logger.h"

#include "Olap/NormalDatabase.h"
#include "Olap/UserInfoDatabase.h"
#include "Olap/RuleMarker.h"
#include "Olap/SystemDatabase.h"
#include "Olap/User.h"
#include "Olap/PaloSession.h"
#include "Olap/Rule.h"

#include "Engine/EngineCpu.h"
#include "Engine/EngineCpuMT.h"

#include "Worker/LoginWorker.h"
#include "Worker/DimensionWorker.h"
#include "Worker/WorkersCreator.h"

#include "Thread/WriteLocker.h"

#ifdef ENABLE_GPU_SERVER
#include "EngineGpu/GpuEngine.h"
#endif

#include "build.h"

namespace palo {
const string Server::NAME_SYSTEM_DATABASE = "System";
const string Server::NAME_CONFIG_DATABASE = "Config";
const string Server::VALID_DATABASE_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.";
bool Server::ignoreJournal = false;

PServer Server::readersserver;
Mutex Server::readerslock;
PServer Server::writersserver;
Mutex Server::writerslock;

RightsType Server::defaultDbRight = RIGHT_DELETE;
string Server::crossOrigin;

string LicIter::getKey()
{
	return "";
}

string LicIter::getCustomer()
{
	return "";
}

string LicIter::getFeatures()
{
	return "";
}

double LicIter::getLicenseCount()
{
	return 0.0;
}

double LicIter::getNamedCount()
{
	return 0.0;
}

string LicIter::getConcurrentSessions()
{
	return "";
}

double LicIter::getGPUCount()
{
	return 0.0;
}

double LicIter::getFreeConcurrent()
{
	return 0.0;
}

double LicIter::getFreeNamed()
{
	return 0.0;
}

int64_t LicIter::getActivationTime()
{
	return 0;
}

int64_t LicIter::getExpirationTime()
{
	return 0;
}

bool LicIter::next()
{
	return false;
}

string LicIter::validateLics(string value)
{
	return "";
}

void Server::invalidateCache(IdentifierType dbId, IdentifierType cubeId)
{
	dbID_cubeID dbCubeId(dbId, cubeId);
	Context::getContext()->clearQueryCache();
	for (DatabaseList::Iterator dbit = dbs->begin(); dbit != dbs->end(); ++dbit) {
		if (NULL == (*dbit)) {
			continue;
		}
		PDatabase db = COMMITABLE_CAST(Database, *dbit);
		for (CubeList::Iterator cbit = db->cubes->begin(); cbit != db->cubes->end(); ++cbit) {
			if (NULL != (*cbit)) {
				PCube cube = COMMITABLE_CAST(Cube, *cbit);
				if (dbId != NO_IDENTIFIER && cubeId != NO_IDENTIFIER) {
					// check if cube depends on specified cube
					ValueCache *cache = cube->getCache();
					if (cube->getId() == cubeId && (*dbit)->getId() == dbId) {
						// invalidate cache exactly of this cube
					} else if (!cache || !cache->isDepending(dbCubeId)) {
						// this cube doesn't depend on specified cube
						continue;
					} else {
						// invalidate cache of this depending cube
//						int breakHere = 1;
					}
				}
				cube->invalidateCache();
			}
		}
	}
}

void Server::updateDatabaseDim(bool useDimWorker)
{
	systemDatabase->updateDatabaseDim(COMMITABLE_CAST(Server, shared_from_this()), useDimWorker);
}

void Server::updateDatabaseDim(SystemDatabase::UpdateType type, const string &dbName, const string &dbOldName, PUser user, bool useDimWorker)
{
	getSystemDatabaseCopy();
	PServer server = COMMITABLE_CAST(Server, shared_from_this());
	systemDatabase->updateDatabaseDim(server, type, dbName, dbOldName, useDimWorker);
	if (type == SystemDatabase::ADD && user) {
		systemDatabase->setDbRight(server, user, dbName);
	}
}

void Server::checkOldCubes()
{
	WriteLocker wl(filelock->getLock());
	for (DatabaseList::Iterator dbit = dbs->begin(); dbit != dbs->end(); ++dbit) {
		if (NULL == (*dbit)) {
			continue;
		}
		PDatabase db = COMMITABLE_CAST(Database, *dbit);
		if (db->getStatus() != Database::LOADING) {
			db->checkOldCubes();
		}
	}
}

void Server::setMarkerCalculationState(bool state)
{
	checkCheckedOut();
	m_DisableMarkerCalculation = state;
}

void Server::addChangedMarkerCube(CPCube cube, bool enforceBuildMarkers)
{
	checkCheckedOut();
	if (enforceBuildMarkers) {
		buildMarkers = true;
	}
	Statistics::Timer timer("server::addChangedMarkerCube");
	CubesWithDBs& changedMarkerCubes = Context::getContext()->getChangedMarkerCubes();
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));

	// check if already know this cube, do nothing in this case
	if (changedMarkerCubes.find(make_pair(db->getId(), cube->getId())) != changedMarkerCubes.end()) {
		return;
	}

	// add cube
	changedMarkerCubes.insert(make_pair(db->getId(), cube->getId()));

	Logger::trace << "adding cube '" << cube->getName() << "' to the list of changed marker cubes" << endl;

	// and all its destinations
	const Cube::RuleMarkerSet& fromMarkers = cube->getFromMarkers();
	Cube::RuleMarkerSet::const_iterator end_it = fromMarkers.end();

	for (Cube::RuleMarkerSet::const_iterator i = fromMarkers.begin(); i != end_it; ++i) {
		PRuleMarker marker = *i;

		dbID_cubeID toDbCube = marker->getToDbCube();
		PCube toCube = Cube::addCubeToDatabase(COMMITABLE_CAST(Server, shared_from_this()), toDbCube);
		if (toCube) {
			addChangedMarkerCube(toCube, enforceBuildMarkers);
		}
	}
}

void Server::addCubeToList(PDatabase database, PCube cube)
{
	PCubeList cubes = database->getCubeList(true);
	database->setCubeList(cubes);
	cubes->set(cube);
	dbs->set(database);
}

void Server::triggerMarkerCalculation()
{
	checkCheckedOut();
	if (m_DisableMarkerCalculation) {
		return;
	}
	CubesWithDBs & changedMarkerCubes = Context::getContext()->getChangedMarkerCubes();
	if (changedMarkerCubes.empty()) {
		return;
	}
	PDatabaseList dbs = getDatabaseList(true);
	setDatabaseList(dbs);
	// first clear all markers
	for (CubesWithDBs::iterator i = changedMarkerCubes.begin(); i != changedMarkerCubes.end(); ++i) {
		PDatabase database = lookupDatabase(i->first, true);
		if (database) {
			PCube cube = database->lookupCube(i->second, true);
			if (cube && cube->getStatus() > Cube::UNLOADED && cube->hitMarkerRebuildLimit()) {
				cube->clearAllMarkers();
				addCubeToList(database, cube);
			}
		}
	}

	CubesWithDBs alreadyBuilt;
	// then rebuild
	CubesWithDBs cubes;
	changedMarkerCubes.swap(cubes);
	if (!cubes.empty()) {
		Logger::debug << "Rebuilding markers" << endl;
	}
	for (CubesWithDBs::iterator i = cubes.begin(); i != cubes.end(); ++i) {
		PDatabase database = lookupDatabase(i->first, true);
		if (!database) {
			continue;
		}
		PCube cube = database->lookupCube(i->second, true);
		if (!(cube && cube->getStatus() > Cube::UNLOADED) || cube->isLocked()) {
			continue;
		}

		const Cube::RuleMarkerSet toMarkers = cube->getToMarkers();
		Cube::RuleMarkerSet::const_iterator end_it = toMarkers.end();

		for (Cube::RuleMarkerSet::const_iterator ci = toMarkers.begin(); ci != end_it; ++ci) {
			PRuleMarker marker = *ci;
			dbID_cubeID fromDbCube = marker->getFromDbCube();
			PCube fromCube = Cube::addCubeToDatabase(COMMITABLE_CAST(Server, shared_from_this()), fromDbCube);
			if (fromCube && fromCube->getStatus() > Cube::UNLOADED && !Cube::isSameCube(cube, fromCube, database->getId(), fromDbCube.first) && !cube->isLocked()) {
				if (alreadyBuilt.find(fromDbCube) == alreadyBuilt.end() && fromCube->hitMarkerRebuildLimit()) {
					fromCube->rebuildAllMarkers();
					alreadyBuilt.insert(fromDbCube);
				}
			}
		}

		if (alreadyBuilt.find(*i) == alreadyBuilt.end() && cube->hitMarkerRebuildLimit()) {
			cube->rebuildAllMarkers();
			addCubeToList(database, cube);
			alreadyBuilt.insert(*i);
		}
	}
	buildMarkers = false;
}

PServer Server::getInstance(bool write)
{
	PServer ret;
	if (write) {
		WriteLocker wl(&writerslock);
		ret = COMMITABLE_CAST(Server, writersserver->copy());
	} else {
		WriteLocker wl(&readerslock);
		ret = readersserver;
	}
	return ret;
}

void Server::create(const FileName & fileName)
{
	PServer s = PServer(new Server(fileName));
	s->commit();
}

Server::Server(const FileName & fileName) :
		Commitable(""), token(rand()), dataToken(rand()), dbs(new DatabaseList()), fileName(fileName), svsStopped(false), dimensionWorkerConfigured(false), filelock(new PaloSharedMutex()), blocking(false), activeSessionSid(PaloSession::NO_SESSION), username(""), event(""), loginType(WORKER_NONE), defaultTtl(300), drillThroughEnabled(false), m_DisableMarkerCalculation(false), encryption(ENC_NONE), shortSid(false), enableGpu(false), engines(new EngineList), bigEndian(false), buildMarkers(false), tp(new ThreadPool)
{
	if (!token) {
		++token;
	}
#ifdef PALO_BIG_ENDIAN
	bigEndian = true;
#endif
	if (fileName.name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_SERVER_PATH, "empty file name", "file name", "");
	}
	PEngineBase cpuEngine(new EngineCpuMT());
	engines->add(PEngineBase(cpuEngine), true);
	Logger::debug << "CPU Engine created with Id " << cpuEngine->getId() << endl;
}

Server::Server(const Server & s) :
	Commitable(s), token(s.token), dataToken(s.dataToken), dbs(s.dbs), fileName(s.fileName), systemDatabase(s.systemDatabase),
	svsStopped(s.svsStopped), loginWorker(s.loginWorker), dimensionWorker(s.dimensionWorker), dimensionWorkerConfigured(s.dimensionWorkerConfigured), filelock(s.filelock), blocking(s.blocking),
	activeSessionSid(s.activeSessionSid), username(s.username), event(s.event), loginType(s.loginType), winSSOenabled(s.winSSOenabled),
	defaultTtl(s.defaultTtl), drillThroughEnabled(s.drillThroughEnabled), m_DisableMarkerCalculation(s.m_DisableMarkerCalculation), encryption(s.encryption),
	shortSid(s.shortSid), enableGpu(s.enableGpu), engines(s.engines), bigEndian(s.bigEndian), buildMarkers(s.buildMarkers), tp(s.tp)
{
}

Server::~Server()
{
}

string Server::getHWKey()
{
	throw ParameterException(ErrorException::ERROR_LICENSE_NOAPI, "api call not implemented.", "", "");
}

string Server::getLicenses()
{
	throw ParameterException(ErrorException::ERROR_LICENSE_NOAPI, "api call not implemented.", "", "");
}

void Server::activateLicense(string lickey, string actcode)
{
}

bool Server::deleteLicense(const set<string> &code)
{
	return false;
}

void Server::findFree(set<string> &code)
{
}

unsigned int Server::getNumberofGPUs()
{
	return 0;
}

void Server::reserveLicense(boost::shared_ptr<PaloSession> session, const string *machine, const string *required, string *optional)
{
	if (required && !required->empty()) {
		throw ParameterException(ErrorException::ERROR_LICENSE_NOAPI, "Parameter not supported.", "required", "");
	}
	if (optional && !optional->empty()) {
		throw ParameterException(ErrorException::ERROR_LICENSE_NOAPI, "Parameter not supported.", "optional", "");
	}
}

void Server::freeLicense(boost::shared_ptr<PaloSession> session)
{
}

void Server::moveLicense(IdentifierType id, IdentifierType position)
{
}

void Server::updateLicenses(PSystemDatabase sysdb)
{
	PCube userPropsCube = sysdb->getUserUserPropertiesCube();
	PDimension userDim = sysdb->getUserDimension();
	PDimension userPropsDim = sysdb->getUserPropertiesDimension();
	Element* elem = userPropsDim->findElementByName(SystemDatabase::LICENSES, 0, false);

	PCubeArea area(new CubeArea(sysdb, userPropsCube, 2));
	PSet s(new Set);
	s->insert(elem->getIdentifier());
	area->insert(1, s);

	s.reset(new Set);
	ElementsType users = userDim->getElements(PUser(), false);
	for (ElementsType::iterator it = users.begin(); it != users.end(); ++it) {
		s->insert((*it)->getIdentifier());
	}
	area->insert(0, s);

	PCellStream cs = userPropsCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
	PLicIter licIt = getLicenseIterator(true);
	set<PCube> c;
	bool changed = false;
	while (cs->next()) {
		const CellValue value = cs->getValue();
		if (!value.empty()) {
			string v = licIt->validateLics(value);
			if (v.size() != value.size()) {
				cs->setValue(v);
				changed = true;
			}
		}
	}
	if (changed) {
		userPropsCube->commitChanges(false, PUser(), c, false);
		userPropsCube->setStatus(Cube::CHANGED);
	}
}

void Server::checkNamedLicense(IdentifierType user, string lic)
{
}

string Server::getSessionLicense(boost::shared_ptr<PaloSession> session)
{
	return "";
}

void Server::updateGpuBins(PDimension dimension, PServer server, PaloJobRequest* jobRequest, PDatabase database, Context* context, PUser user){
#ifdef ENABLE_GPU_SERVER
	 //get GPU-accelerated cubes using this dimension
	ItemType gpuFilter = GPUTYPE;
	vector<CPCube> gCubes = dimension->getCubes(user, database, &gpuFilter);

	//if there are any GPU-accelerated cubes using this dimension at all
	if(gCubes.size() > 0){

		PEngineGpu engineGpu = COMMITABLE_CAST(EngineGpu, server->getEngine(EngineBase::GPU, true));

		//ID of this dimension
		IdentifierType dimID = jobRequest->dimension;

		for (size_t cubeIdx = 0; cubeIdx < gCubes.size(); ++cubeIdx){
			jobRequest->cube = gCubes[cubeIdx]->getId();
			database->findCube(jobRequest->cube, user, true, true);
			PCubeList cubelist = database->getCubeList(true);
			database->setCubeList(cubelist);
			PCube cubecopy = COMMITABLE_CAST(Cube, cubelist->get(jobRequest->cube, true));

			//find out the index of the dimension in the cube
			uint32_t dimIdx = 0;
			const IdentifiersType* cubeDimIds = cubecopy->getDimensions();
			for(IdentifiersType::const_iterator cIt = cubeDimIds->begin(); cIt != cubeDimIds->end(); cIt++, dimIdx++){
				if((*cIt) == dimID)
					break;
			}

			PPathTranslator pathTrans(cubecopy->getPathTranslator());
			vector<uint32_t> dimPos = pathTrans->getDimensionsPos();

			//get number of bits used to represent this dimension
			size_t numBits = pathTrans->getDimensionBitSize(dimIdx);

			//get the max identifier which has to be represented in this dimension
			size_t maxId = dimension->getMaximalIdentifier();

			if(maxId + 2 <= (size_t)((1 << numBits) - 1)) //if there is sufficient space for a new element in this dimension
				break;

			//find out the number of additional bits needed to represent maxId
			uint64_t bits = (1 << numBits) - 1;
			uint32_t numAdditionalBitsNeeded = 0;
			while(maxId + 2 > bits){
				bits <<= 1;
				++bits;
				++numAdditionalBitsNeeded;
			}

			//find out the index of the bin including the actual dimension
			//and the index of the last dimension in that bin
			uint32_t actBin = 0;
			uint32_t indexOfLastDimInActBin = 0;
			vector<pair<uint32_t, uint32_t> > binDimRanges = pathTrans->getBinDimensionRanges();
			for(vector<pair<uint32_t, uint32_t> >::iterator rangesIt = binDimRanges.begin(); rangesIt != binDimRanges.end(); rangesIt++, actBin++){
				if(rangesIt->first <= dimIdx && rangesIt->second >= dimIdx){
					indexOfLastDimInActBin = rangesIt->second;
					break;
				}
			}

			PGpuStorage storageGpu;

			if(dimPos[indexOfLastDimInActBin] >= numAdditionalBitsNeeded){ //if there's enough room in the actual bin for the additional element
				//rightshift actual bin by numAdditionalBitsNeeded
				pathTrans->enlargeDimension(dimIdx, indexOfLastDimInActBin - dimIdx + 1, numAdditionalBitsNeeded);
				cubecopy->updatePathTranslator(pathTrans);
				IdentifierType id(cubecopy->getNumericStorageId());
				storageGpu = COMMITABLE_CAST(GpuStorage, engineGpu->getCreateStorage(id, pathTrans, EngineBase::Numeric));
				Logger::trace << "Resizing dimension " << dimIdx << " in bin " << actBin << " of cube " << gCubes[cubeIdx]->getId() << endl;
				Logger::trace << "Rightshifting " << numAdditionalBitsNeeded << " bit(s) inside bin " << actBin << " of cube " << cubeIdx << endl;
				storageGpu->rightshiftBin(actBin, dimIdx ? dimPos[dimIdx-1]-1 : 63, numAdditionalBitsNeeded);
			}
			else{   //else rightshift actual bin by numAdditionalBitsNeeded and move the last dimension(s) in this bin to the next bin, check there again,
					//if there's enough room for the dimension - if not, move the last dimension(s) in this bin to the next bin
					//and so on
				uint32_t numBinsToChange = 0;
				uint32_t lastDimResizedBits = 0;
				vector<uint32_t> numEmptyBits;
				vector<uint32_t> numBitsToMove;
				numBitsToMove.push_back(numAdditionalBitsNeeded);
				bool addNewBin = false;

				uint32_t numBins = pathTrans->getNumberOfBins();
				for(uint32_t bin = actBin; bin < numBins; ++bin){
					if(dimPos[binDimRanges[bin].second] >= numBitsToMove[numBinsToChange]) //if there's enough room for moved dimension in actual bin
					   break;
					else{ //else move numDims dimensions to the next bin until the bit-size of these dimenions is greater than the amount of bits to be moved to the actual bin
						uint32_t numDims = 0;
						uint32_t bits = 0;
						if(bin == actBin + 1 && lastDimResizedBits){
							while(bits < lastDimResizedBits - dimPos[binDimRanges[bin].second]){ //dimPos[...] is the empty space at the end of a bin
								bits += pathTrans->getDimensionBitSize(binDimRanges[bin].second-numDims);
								++numDims;
							}
						}
						else{
							while(bits < numBitsToMove[numBinsToChange] - dimPos[binDimRanges[bin].second]){ //dimPos[...] is the empty space at the end of a bin
								bits += pathTrans->getDimensionBitSize(binDimRanges[bin].second-numDims);
								++numDims;
							}
						}
						if(bin == actBin && dimIdx == indexOfLastDimInActBin)
							lastDimResizedBits = bits + numAdditionalBitsNeeded;

						numBitsToMove.push_back(bits);
						numEmptyBits.push_back(dimPos[binDimRanges[bin].second]);
						++numBinsToChange;

						//add a new bin to all pages in storage if necessary
						if(bin == numBins - 1){
							addNewBin = true;
						}
					}
				}

				numBitsToMove.erase(numBitsToMove.begin()); //the "original" bin doesn't need to be moved
				IdentifiersType dimensionSizes;
				for(uint32_t i = 0; i < dimPos.size(); ++i){
					dimensionSizes.push_back(pathTrans->getDimensionBitSize(i));
				}
				dimensionSizes[dimIdx] += numAdditionalBitsNeeded;
				PPathTranslator pathTransNew(new PathTranslator(dimensionSizes, true));
				cubecopy->updatePathTranslator(pathTransNew);
				IdentifierType id(cubecopy->getNumericStorageId());
				storageGpu = COMMITABLE_CAST(GpuStorage, engineGpu->getCreateStorage(id, pathTrans, EngineBase::Numeric));
				Logger::trace << "Resizing dimension " << dimIdx << " in bin " << actBin << " of cube " << gCubes[cubeIdx]->getId() << endl;
				if(addNewBin)
					Logger::trace << "New bin has to be created for rightshifting!" << endl;
				for(size_t i = 0; i < numBitsToMove.size(); i++){
					Logger::trace << "Moving " << numBitsToMove[i] << " bits from bin " << actBin << " to " << actBin+i+1 << endl;
				}

				storageGpu->rightshiftBins(actBin, numBinsToChange, numBitsToMove, numEmptyBits, addNewBin, dimIdx ? dimPos[dimIdx-1] - 1: 63, numAdditionalBitsNeeded, lastDimResizedBits);
			}
			context->saveParent(database, cubecopy);
			cubelist->set(cubecopy);
		}
	}
#endif
}

PLicIter Server::getLicenseIterator(bool find)
{
	return PLicIter(new LicIter);
}

void Server::afterLoad()
{
}

vector<CPDatabase> Server::getAdvanced(PUser user)
{
	return vector<CPDatabase>();
}

void Server::removeAdvanced(IdentifierType id)
{
}

void Server::resetFirst()
{
}

string Server::getUserLicense(IdentifierType user)
{
	PServer server = Context::getContext()->getServer();
	PSystemDatabase db = server->getSystemDatabase();
	PCube cube = db->getUserUserPropertiesCube();
	PCubeArea path(new CubeArea(db, cube, 2));
	PSet s(new Set);
	s->insert(user);
	path->insert(0, s);
	s.reset(new Set);
	s->insert(db->findDimensionByName(SystemDatabase::NAME_USER_PROPERTIES_DIMENSION, PUser(), false)->findElementByName(SystemDatabase::LICENSES, 0, false)->getIdentifier());
	path->insert(1, s);
	CellValue val = cube->getCellValue(path);
	return val;
}

void Server::setUserLicense(IdentifierType user, string lickey)
{
	PServer server = Context::getContext()->getServer();
	PSystemDatabase db = server->getSystemDatabaseCopy();
	PCubeList cubes = db->getCubeList(true);
	db->setCubeList(cubes);
	PCube cube = db->findCube(db->getUserUserPropertiesCube()->getId(), PUser(), true, true);
	cubes->set(cube);
	PCubeArea path(new CubeArea(db, cube, 2));
	PSet s(new Set);
	s->insert(user);
	path->insert(0, s);
	s.reset(new Set);
	s->insert(db->findDimensionByName(SystemDatabase::NAME_USER_PROPERTIES_DIMENSION, PUser(), false)->findElementByName(SystemDatabase::LICENSES, 0, false)->getIdentifier());
	path->insert(1, s);

	set<PCube> changedCubes;
	cube->setCellValue(server, db, path, lickey, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, true, CubeArea::BASE_STRING);
	cube->commitChanges(false, PUser(), changedCubes, false);
}

void Server::databaseRenamed(const string &oldName, const string &newName)
{
	WriteLocker wl(filelock->getLock());
	FileWriterTXT f(FileName(fileName.path, "renamed", "dbs"));
	f.openFile(true);
	f.appendEscapeString(oldName);
	f.appendEscapeString(newName);
	f.nextLine();
}

void Server::physicalRenames()
{
	WriteLocker wl(filelock->getLock());
	physicalRenamesNoLock(fileName);
}

void Server::physicalRenamesNoLock(FileName &fileName)
{
	FileName fn(fileName.path, "renamed", "dbs");
	{
		FileReaderTXT f(fn);
		if (f.openFile(false, true)) {
			while (f.isDataLine()) {
				string oldName = f.getDataString(0);
				string newName = f.getDataString(1);
				f.nextLine();
				if (!FileUtils::rename(FileName(fileName.path, oldName, ""), FileName(fileName.path, newName, ""))) {
					Logger::error << "Can't rename database \"" << oldName << "\" to \"" << newName << "\"" << endl;
				}
			}
		}
	}
	FileUtils::remove(fn);
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a server
////////////////////////////////////////////////////////////////////////////////
FileName Server::computeDatabaseFileName(const FileName & fileName, const string & name)
{
	return FileName(fileName.path + "/" + name, "database", "csv");
}

IdentifierType Server::loadServerOverview(FileReader *file)
{
	IdentifierType sizeDatabases = 0;
	if (file->isSectionLine() && file->getSection() == "SERVER") {
		file->nextLine();
		if (file->isDataLine()) {
			sizeDatabases = file->getDataInteger(0);
			file->nextLine();
		}
	} else {
		throw FileFormatException("section 'SERVER' not found", file);
	}
	return sizeDatabases;
}

IdentifierType Server::loadServerDatabase(FileReader *file)
{
	IdentifierType identifier = (IdentifierType)(file->getDataInteger(0));
	string name = file->getDataString(1);
	int type = file->getDataInteger(2);
	FileName dbFile = computeDatabaseFileName(fileName, name);
	if (FileUtils::isReadable(dbFile)) {
		PDatabase database = Database::loadDatabaseFromType(file, COMMITABLE_CAST(Server, shared_from_this()), identifier, name, type);

		addDatabase(database, false, false, PUser(), false);
	} else {
		Logger::warning << "database file for '" << name << "' is missing, removing database" << endl;
		identifier = 0;
	}
	file->nextLine();
	return identifier;
}

void Server::loadServerDatabases(FileReader *file)
{
	dbs.reset(new DatabaseList());
	if (file->isSectionLine() && file->getSection() == "DATABASES") {
		file->nextLine();
		IdentifierType newStartId = 0;
		while (file->isDataLine()) {
			IdentifierType id = loadServerDatabase(file);
			if (id >= newStartId) {
				newStartId = id + 1;
			}
		}

		dbs->setNewIDStart(newStartId);
	} else {
		throw FileFormatException("section 'DATABASES' not found", file);
	}
}

void Server::loadServer(PUser user)
{
	checkCheckedOut();
	checkSystemOperationRight(user, RIGHT_DELETE);
	WriteLocker wl(filelock->getLock());
	if (!FileUtils::isReadable(fileName) && FileUtils::isReadable(FileName(fileName, "tmp"))) {
		Logger::warning << "using temp file for server" << endl;
		// rename temp file
		FileUtils::rename(FileName(fileName, "tmp"), fileName);
	}
	boost::shared_ptr<FileReader> fr(FileReader::getFileReader(fileName));
	fr->openFile(true, false);
	// load server from file
	loadServerOverview(fr.get());
	loadServerDatabases(fr.get());
}

void Server::saveServerOverview(FileWriter *file)
{
	file->appendComment("PALO SERVER DATA");
	file->appendComment("");
	file->appendComment("Description of data:");
	file->appendComment("NUMBER_DATABASES;");
	file->appendSection("SERVER");
	file->appendInteger(dbs->getLastId());
	file->nextLine();
}

void Server::saveServerDatabases(FileWriter *file)
{
	// write data for databases
	file->appendComment("Description of data: ");
	file->appendComment("ID;NAME;TYPE;DELETABLE;RENAMABLE;EXENTSIBLE");
	file->appendSection("DATABASES");
	for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
		PDatabase database = COMMITABLE_CAST(Database, *i);
		if (NULL != database)
			database->saveDatabaseType(file);
	}
}

void Server::saveServer(PUser user, bool complete)
{
	if (NULL != user) {
		checkSystemOperationRight(user, RIGHT_WRITE);
	}
	// open a new temp-server file
	WriteLocker wl(filelock->getLock());
	boost::shared_ptr<FileWriter> fw(FileWriter::getFileWriter(FileName(fileName, "tmp")));
	fw->openFile();
	// save server and databases to disk
	saveServerOverview(fw.get());
	saveServerDatabases(fw.get());
	fw->appendComment("");
	fw->appendComment("PALO SERVER DATA END");
	fw->closeFile();
	// remove old server file
	if (FileUtils::isReadable(fileName) && !FileUtils::remove(fileName)) {
		Logger::error << "cannot remove server file: '" << strerror(errno) << endl;
		Logger::error << "please check the underlying file system for errors" << endl;
		exit(1);
	}
	// rename temp-server file
	if (!FileUtils::rename(FileName(fileName, "tmp"), fileName)) {
		Logger::error << "cannot rename server file: '" << strerror(errno) << endl;
		Logger::error << "please check the underlying file system for errors" << endl;
		exit(1);
	}

	if (complete) {
		for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
			PDatabase database = COMMITABLE_CAST(Database, *i);
			if (NULL != database)
				saveDatabase(database, user, true, NULL, complete);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// getter and setter
////////////////////////////////////////////////////////////////////////////////
const char *Server::getRevision()
{
	return Revision;
}

const char *Server::getVersion()
{
	return Version;
}

string Server::getVersionRevision()
{
	string version = Version;
	replace(version.begin(), version.end(), ';', '.');
	return version + " (" + Revision + ")";
}

string Server::getVersionRevisionDots()
{
	string version = Version;
	replace(version.begin(), version.end(), ';', '.');
	return version + "." + Revision;
}

vector<CPDatabase> Server::getDatabases(PUser user) const
{
	checkDatabaseAccessRight(user, RIGHT_READ);
	bool showSystem = !User::checkUser(user) || user->getRoleRight(User::rightsRight) > RIGHT_NONE;
	vector<CPDatabase> result;
	for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
		PDatabase database = COMMITABLE_CAST(Database, *i);

		if (database != 0) {
			ItemType type = database->getType();
			bool showNormal = type == NORMALTYPE && (!User::checkUser(user) || user->getRoleDbRight(User::databaseRight, database) > RIGHT_NONE);
			bool showUserInfo = type == USER_INFOTYPE && (!User::checkUser(user) || user->getRoleDbRight(User::databaseRight, database) > RIGHT_NONE);
			if (showNormal || (type == SYSTEMTYPE && showSystem) || showUserInfo) {
				result.push_back(database);
			}
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// remove specified storages from all engines
////////////////////////////////////////////////////////////////////////////////
void Server::removeStorages(IdentifiersType storages)
{
	checkCheckedOut();
	engines = getEngineList(true);
	IdentifiersType engineIds;
	for (EngineList::Iterator engineIt = engines->begin(); engineIt != engines->end(); ++engineIt) {
		if (*engineIt) {
			engineIds.push_back((*engineIt)->getId());
		}
	}

	for (IdentifiersType::const_iterator engineIt = engineIds.begin(); engineIt != engineIds.end(); ++engineIt) {
		PEngineBase engine = getEngine((EngineBase::Type)*engineIt, true);
		for (IdentifiersType::const_iterator storageIt = storages.begin(); storageIt != storages.end(); ++storageIt) {
			PStorageBase storage = engine->getStorage(*storageIt);
			if (storage) {
				engine->deleteStorage(*storageIt);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// functions to administrate the databases
////////////////////////////////////////////////////////////////////////////////
void Server::beginShutdown(PUser user)
{
	checkCheckedOut();
	if (User::checkUser(user) && user->getRoleRight(User::sysOpRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)(user->getId()));
	}
	ShutdownGpuEngine(user);
	for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
		PDatabase database = COMMITABLE_CAST(Database, *i);
		PCubeList cubes = database->getCubeList(false);

		for (CubeList::Iterator ic = cubes->begin(); ic != cubes->end(); ++ic) {
			PCube cube = COMMITABLE_CAST(Cube, *ic);

			cube->executeShutdown();
		}
	}
	Context::getContext()->setTokenUpdate(false);
}

void Server::changePassword(PUser userChanging, IdentifierType userToChange, const string& new_password)
{
	if (userChanging) {
		getSystemDatabaseCopy();
		systemDatabase->changePassword(COMMITABLE_CAST(Server, shared_from_this()), userChanging, userToChange, new_password);
	}
}

void Server::ShutdownLoginWorker()
{
	if (loginWorker) {
		loginWorker->notifyShutdown();
		loginWorker.reset();
	}
}

void Server::ShutdownDimensionWorker()
{
	if (dimensionWorker) {
		dimensionWorker->notifyShutdown();
		dimensionWorker.reset();
	}
}

// workaround to ensure graceful GPUServer shutdown without relying on Server
void Server::ShutdownGpuEngine(PUser user)
{
#ifdef ENABLE_GPU_SERVER
	checkCheckedOut();
	PEngineBase gpuEngine = getEngine(EngineBase::GPU, false);
	if (gpuEngine) {
		removeEngine(gpuEngine);
	}
	enableGpu = false;
#endif
}

// this method is used internally, so no rights checking is done
void Server::addDatabase(PDatabase database, bool notify, bool newid, PUser user, bool useDimWorker)
{
	checkCheckedOut();
	const string & name = database->getName();
	if (lookupDatabaseByName(name, false) != 0) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NAME_IN_USE, "database name is already in use", "name", name);
	}
	if (!newid) {
		IdentifierType identifier = database->getIdentifier();
		if (lookupDatabase(identifier, false) != 0) {
			throw ParameterException(ErrorException::ERROR_INTERNAL, "database identifier is already in use", "identifier", (int)(identifier));
		}
	}

	if (!dbs->isCheckedOut()) {
		dbs = COMMITABLE_CAST(DatabaseList, dbs->copy());
	}
	dbs->add(PCommitable(database), newid);
	// set database file
	database->setDatabaseFile(COMMITABLE_CAST(Server, shared_from_this()), computeDatabaseFileName(fileName, name));
	// tell database that it has been added
	if (notify) {
		database->notifyAddDatabase(COMMITABLE_CAST(Server, shared_from_this()), user, useDimWorker);
	}
	if (database->getStatus() == Database::CHANGED) {
		database->saveDatabase(COMMITABLE_CAST(Server, shared_from_this()));
	}
}

// this method is used internally, so no rights checking is done
void Server::commitAndSave()
{
	try {
		saveServer(PUser(), false);

		for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
			PDatabase database = COMMITABLE_CAST(Database, *i);
			if (NULL == database)
				continue;

			if (database->getStatus() == Database::CHANGED) {
				Logger::info << "auto commiting changes to database '" << database->getName() << "'" << endl;
				saveDatabase(database, PUser(), true, NULL, false);
			}

			PCubeList cubes = database->getCubeList(false);

			for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
				PCube cube = COMMITABLE_CAST(Cube, *j);
				if (NULL == cube)
					continue;

				if (cube->getStatus() == Cube::CHANGED) {
					Logger::info << "auto commiting changes to cube '" << cube->getName() << "'" << endl;
					database->saveCube(COMMITABLE_CAST(Server, shared_from_this()), cube, PUser());
				}
			}
		}
	} catch (const ErrorException& e) {
		Logger::warning << "cannot commit data file '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
	}
}

// this method is used internally, so no rights checking is done
void Server::removeDatabase(PDatabase database, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	IdentifierType identifier = database->getIdentifier();
	if (database != lookupDatabase(identifier, false)) {
		throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database not found", "database", database->getName());
	}
	if (!dbs->isCheckedOut()) {
		dbs = COMMITABLE_CAST(DatabaseList, dbs->copy());
	}

	PServer server = COMMITABLE_CAST(Server, shared_from_this());
	// delete storage from all engines here
	IdentifiersType storages;
	vector<CPCube> cubes = database->getCubes(PUser());
	for (vector<CPCube>::iterator cbit = cubes.begin(); cbit != cubes.end(); ++cbit) {
		if (NULL != (*cbit)) {
			PCube cube = COMMITABLE_CAST(Cube, (*cbit)->copy());
			vector<PRule> rules = cube->getRules(PUser(), false);
			for (vector<PRule>::const_iterator ruleIt = rules.begin(); ruleIt != rules.end(); ++ruleIt) {
				cube->deleteRule(server, database, (*ruleIt)->getId(), PUser(), true);
			}
			storages.push_back(cube->getNumericStorageId());
			storages.push_back(cube->getStringStorageId());
			storages.push_back(cube->getMarkerStorageId());
		}
	}
	removeStorages(storages);
	dbs->remove(identifier);

	// tell database that it has been removed
	if (notify) {
		database->notifyRemoveDatabase(server, useDimWorker);
	}
}

// this method is used internally, so no rights checking is done
void Server::renameDatabase(PDatabase database, const string & name, bool notify, bool useDimWorker)
{
	checkCheckedOut();
	// check for doubly defined name
	PDatabase databaseByName = lookupDatabaseByName(name, false);
	if (databaseByName != 0) {
		if (databaseByName != database) {
			throw ParameterException(ErrorException::ERROR_DATABASE_NAME_IN_USE, "database name already in use", "name", name);
		}
		if (database->getName() == name) {
			// new name == old name
			return;
		}
	}

	string oldName = database->getName();

	dbs->remove(database->getId());
	database->setName(name);
	dbs->add(PDatabase(database), false);
	// tell database that it has been renamed
	if (notify) {
		database->notifyRenameDatabase(COMMITABLE_CAST(Server, shared_from_this()), oldName, useDimWorker);
	}
}

PDatabase Server::addDatabase(const string & realName, PUser user, IdentifierType type, bool useDimWorker)
{
	checkCheckedOut();
	checkDatabaseAccessRight(user, RIGHT_WRITE);
	IdentifierType id;
	if (realName.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "database name is empty", "name", realName);
	}
	string name = realName;
	if (name.find_first_not_of(VALID_DATABASE_CHARACTERS) != string::npos) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "database name contains an illegal character", "name", name);
	}
	if (name[0] == '.') {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "database name begins with a dot character", "name", name);
	}
	if (0 == type) {
		//retrieve the type ID from the database file
		boost::shared_ptr<FileReader> database_file(FileReader::getFileReader(computeDatabaseFileName(fileName, name)));
		database_file->openFile(true, false);
		while (!database_file->isSectionLine() && !database_file->isEndOfFile()) {
			database_file->nextLine();
			if (database_file->isSectionLine()) {
				if ("DATABASE" != database_file->getSection()) {
					database_file->nextLine();
				}
			}
		}

		if (!database_file->isSectionLine() || ("DATABASE" != database_file->getSection())) {
			//invalid db file
			throw ParameterException(ErrorException::ERROR_CORRUPT_FILE, "missing [DATABASE] section", "name", name);
		}
		database_file->nextLine();
		id = database_file->getDataInteger(2, 1);
	} else {
		id = type;
	}
	// create database and add database to database vector
	PDatabase database; // = new NormalDatabase(identifier, this, name);
	switch (id) {
	case UserInfoDatabase::DB_TYPE:
		if (name == Server::NAME_CONFIG_DATABASE) {
			database = PDatabase(new ConfigDatabase(name));
		} else {
			database = PDatabase(new UserInfoDatabase(name));
		}
		break;
	case NormalDatabase::DB_TYPE: //RIA: support for 0003372. everything other than User Info is normal
	default:
		database = PDatabase(new NormalDatabase(name));
		/*
		 throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_TYPE,
		 "invalid database type",
		 "type", id);
		 */
	}
	database->setDeletable(true);
	database->setRenamable(true);
	database->setExtensible(true);
	// and update database structure
	addDatabase(database, true, true, user, useDimWorker);
	// return database
	return database;
}

void Server::deleteDatabase(PDatabase database, PUser user, bool useDimWorker)
{
	checkCheckedOut();
	checkDatabaseAccessRight(user, RIGHT_DELETE);
	User::checkDatabaseDataRight(user, database->getId(), RIGHT_DELETE);
	// check deletable flag
	if (!database->isDeletable()) {
		throw ParameterException(ErrorException::ERROR_DATABASE_UNDELETABLE, "database cannot be deleted", "database", database->getName());
	}
	// remove database
	removeDatabase(database, true, useDimWorker);
	Context::getContext()->addDatabaseToDelete(database);
	removeAdvanced(database->getIdentifier());
}

void Server::renameDatabase(PDatabase database, const string & realName, PUser user, bool useDimWorker)
{
	checkCheckedOut();
	checkDatabaseAccessRight(user, RIGHT_WRITE);
	User::checkDatabaseDataRight(user, database->getId(), RIGHT_WRITE);
	// check renamable flag
	if (!database->isRenamable()) {
		throw ParameterException(ErrorException::ERROR_DATABASE_UNRENAMABLE, "database cannot be renamed", "database", database->getName());
	}
	if (realName.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "database name is empty", "name", realName);
	}
	if (database->getType() != NORMALTYPE) {
		throw ParameterException(ErrorException::ERROR_DATABASE_UNRENAMABLE, "system database cannot be renamed", "name", realName);
	}
	string name = realName;
	if (name.find_first_not_of(VALID_DATABASE_CHARACTERS) != string::npos) {
		throw ParameterException(ErrorException::ERROR_INVALID_DATABASE_NAME, "database name contains illegal characters", "name", name);
	}
	// change name
	renameDatabase(database, name, true, useDimWorker);
}

void Server::loadDatabase(PDatabase database, PUser user)
{
	checkCheckedOut();
	checkSystemOperationRight(user, RIGHT_DELETE);
	if (!dbs->isCheckedOut()) {
		dbs = COMMITABLE_CAST(DatabaseList, dbs->copy());
	}
	database->loadDatabase(COMMITABLE_CAST(Server, shared_from_this()), false);
	dbs->set(database);
	resetFirst();
}

PDatabase Server::restoreDatabase(const string &zipFileName, PUser user, string *dbName)
{
	checkCheckedOut();
	checkSystemOperationRight(user, RIGHT_WRITE);
	if (!dbs->isCheckedOut()) {
		dbs = COMMITABLE_CAST(DatabaseList, dbs->copy());
	}

	string newDatabaseName = "";
	ZipUtils::checkZipDbValidity(zipFileName);
	ZipUtils::extractZip(zipFileName, "", true, dbName ? *dbName : newDatabaseName, &newDatabaseName);

	if (newDatabaseName == "") {
		throw ErrorException(ErrorException::ERROR_ZIP, "database folder not found in the archive");
	}

	PDatabase database = addDatabase(newDatabaseName, PUser(), 0, true);
	FileUtils::remove(FileName(database->getPath(), "_loading", "lock"));
	database->loadDatabase(COMMITABLE_CAST(Server, shared_from_this()), false);
	dbs->set(database);
	resetFirst();
	return database;
}

void Server::saveDatabaseCubes(PDatabase database)
{
	PCubeList cubes = database->getCubeList(false);
	for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
		PCube cube = COMMITABLE_CAST(Cube, *j);
		if (NULL == cube) {
			continue;
		}
		if (cube->getStatus() == Cube::CHANGED) {
			database->saveCube(COMMITABLE_CAST(Server, shared_from_this()), cube, PUser());
		}
	}
}

void Server::saveDatabase(PDatabase database, PUser user, bool sendEvent, string *backupZipPath, bool complete)
{
	checkSystemOperationRight(user, RIGHT_WRITE);

	if (sendEvent) { // not auto commit
		if (backupZipPath) {
			Logger::info << "starting backup operation for database " << database->getName() << endl;
		} else {
			Logger::info << "starting save operation for database " << database->getName() << endl;
		}
	}

	database->saveDatabase(COMMITABLE_CAST(Server, shared_from_this()));

	if (backupZipPath) {
		if (database->getType() == SYSTEMTYPE) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "backup of system database cannot be created");
		}

		if (!systemDatabase) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "no system database to save");
		}
	}

	if (backupZipPath || complete) {
		saveDatabaseCubes(database);
	}

	if (backupZipPath) {
		systemDatabase->saveDatabase(COMMITABLE_CAST(Server, shared_from_this()));
		saveDatabaseCubes(systemDatabase);

		Logger::info << "database saved, creating backup file" << endl;

		ZipUtils::zipDirectory(database->getPath(), *backupZipPath, false);
		ZipUtils::zipDirectory(systemDatabase->getPath(), *backupZipPath, true);

	}

	if (sendEvent) { // not auto commit
		if (backupZipPath) {
			Logger::info << "finished backup operation for database " << database->getName() << endl;
		} else {
			Logger::info << "finished save operation for database " << database->getName() << endl;
		}
	}

	if (sendEvent && loginWorker && !loginWorker->isShutdownInProgress()) {
		PLoginWorker loginWorker = getLoginWorker();
		if (loginWorker) {
			loginWorker->notifyDatabaseSaved(database->getIdentifier());
		}
	}
}

void Server::unloadDatabase(PDatabase database, PUser user)
{
	checkCheckedOut();
	checkSystemOperationRight(user, RIGHT_DELETE);
	if (!dbs->isCheckedOut()) {
		dbs = COMMITABLE_CAST(DatabaseList, dbs->copy());
	}
	Logger::info << "Unloading database: " << database->getName() << endl;
	database->unloadDatabase(COMMITABLE_CAST(Server, shared_from_this()));
	dbs->set(database);
	removeAdvanced(database->getIdentifier());
}

////////////////////////////////////////////////////////////////////////////////
// functions to administrate the system databases
////////////////////////////////////////////////////////////////////////////////
void Server::addSystemDatabase()
{
	checkCheckedOut();
	addSystemDatabaseIntern(true);
}

void Server::addConfigDatabase()
{
	checkCheckedOut();
	addSystemDatabaseIntern(false);
}

void Server::addSystemDatabaseIntern(bool sys)
{
	string name = sys ? NAME_SYSTEM_DATABASE : NAME_CONFIG_DATABASE;
	bool newdb = false;

	PDatabase database = lookupDatabaseByName(name, false);
	if (database == 0) {
		Logger::info << name << " database not found" << endl;
		if (sys) {
			systemDatabase = PSystemDatabase(new SystemDatabase(name));
			database = systemDatabase;

		} else {
			database = PUserInfoDatabase(new ConfigDatabase(name));
		}
		database->setDeletable(false);
		database->setRenamable(false);

		FileName dbFile = computeDatabaseFileName(fileName, name);
		newdb = !FileUtils::isReadable(dbFile);
		addDatabase(database, newdb, true, PUser(), false);
	}
	if (database->getStatus() == Database::UNLOADED) {
		Logger::info << name << " database not yet loaded, loading now" << endl;
		loadDatabase(database, PUser());
		Logger::trace << name << " database found and loaded" << endl;
		if (sys) {
			systemDatabase = COMMITABLE_CAST(SystemDatabase, database);
		}
	}
	if (database->getType() == (sys ? SYSTEMTYPE : USER_INFOTYPE)) {
		database->createSystemItems(COMMITABLE_CAST(Server, shared_from_this()), newdb);
		if (newdb) {
			Logger::info << "created " << name << " database " << endl;
		} else {
			Logger::trace << name << " database checked" << endl;
		}
	} else {
		Logger::error << name << " database is not of type " << (sys ? "SYSTEM" : "USER_INFO") << endl;
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot recover from error");
	}
}

////////////////////////////////////////////////////////////////////////////////
// functions for workers
////////////////////////////////////////////////////////////////////////////////
bool Server::svsConfigured() const
{
	return loginType != WORKER_NONE || dimensionWorkerConfigured || CubeWorker::useCubeWorker() || winAuthEnabled();
}

bool Server::svsIsStopped() const
{
	return svsStopped;
}

void Server::setSvsStopped(bool state)
{
	svsStopped = state;
}

PLoginWorker Server::getLoginWorker() const
{
	if (!loginWorker) {
		return PLoginWorker();
	}
	Logger::trace << "starting login worker" << endl;
	bool ok = loginWorker->start();
	if (!ok) {
		throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start login worker");
	} else {
		Logger::trace << "login worker started." << endl;
	}
	return loginWorker;
}

PDimensionWorker Server::getDimensionWorker() const
{
	if (!dimensionWorker) {
		return PDimensionWorker();
	}
	Logger::trace << "starting dimension worker" << endl;
	bool ok = dimensionWorker->start();
	if (!ok) {
		throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start dimension worker");
	} else {
		Logger::trace << "dimension worker started." << endl;
	}
	return dimensionWorker;
}

bool Server::isDimensionWorkerConfigured() const
{
	return dimensionWorkerConfigured;
}

void Server::svsRestart(vector<PCube> &cubes, svsStatusChange status)
{
	switch (status) {
	case SVS_RESTART:
		Logger::info << "Restarting SVS." << endl;
		WorkersCreator::quitAll(COMMITABLE_CAST(Server, shared_from_this()), cubes, true);
		WorkersCreator::startAll(false);
		break;
	case SVS_STOP:
		Logger::info << "Stopping SVS." << endl;
		WorkersCreator::quitAll(COMMITABLE_CAST(Server, shared_from_this()), cubes, false);
		resetFirst();
		break;
	case SVS_START:
		Logger::info << "Starting SVS." << endl;
		WorkersCreator::prepareNewStart(COMMITABLE_CAST(Server, shared_from_this()));
		WorkersCreator::startAll(false);
		resetFirst();
		break;
	case SVS_NONE:
		break;
	}
}

void Server::setDatabaseList(PDatabaseList l)
{
	checkCheckedOut();
	dbs = l;
}

bool Server::commit()
{
	bool ret = true;
	checkCheckedOut();
	ret = Context::getContext()->makeCubeChanges(false, PServer());
	if (!Context::getContext()->isWorker()) {
		if (ret) {
			WriteLocker wl(&writerslock);
			ret = merge(writersserver, PCommitable());
			if (ret) {
				writersserver = COMMITABLE_CAST(Server, shared_from_this());
				WriteLocker wl(&readerslock);
				readersserver = writersserver;
			}
		}
	}
	return ret;
}

bool Server::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	mergeint(o,p);
	bool ret = true;
	if (o != 0) {
		o->checkNotCheckedOut();
	}

	Context *context = Context::getContext();
	ret = context->makeCubeChanges(true, writersserver);
	CPServer server = CONST_COMMITABLE_CAST(Server, o);
	if (ret && systemDatabase) {
		systemDatabase = COMMITABLE_CAST(SystemDatabase, dbs->get(systemDatabase->getId(), true));
		ret = systemDatabase->mergespecial(server->systemDatabase, shared_from_this(), false);
	}
	if (ret && systemDatabase && dbs->isCheckedOut()) {
		dbs->set(systemDatabase);
		if (context->getRefreshUsers()) {
			systemDatabase->refreshUsers();
		}
	}
	if (ret) {
		CPServer oldserver = CONST_COMMITABLE_CAST(Server, old);

		if (o != 0 && context->doTokenUpdate()) {
			token = server->token + 1;
			if (!token) {
				++token;
			}
		}
		if (o != 0) {
			dataToken = server->dataToken + 1;
		}
		if (old != 0) {
			if (server != 0) {
				if (blocking == oldserver->blocking) {
					blocking = server->blocking;
				}
				if (activeSessionSid == oldserver->activeSessionSid) {
					activeSessionSid = server->activeSessionSid;
				}
				if (username == oldserver->username) {
					username = server->username;
				}
				if (event == oldserver->event) {
					event = server->event;
				}
				if (m_DisableMarkerCalculation == oldserver->m_DisableMarkerCalculation) {
					m_DisableMarkerCalculation = server->m_DisableMarkerCalculation;
				}
				if (svsStopped == oldserver->svsStopped) {
					svsStopped = server->svsStopped;
				}
				if (loginWorker == oldserver->loginWorker) {
					loginWorker = server->loginWorker;
				}
				if (dimensionWorker == oldserver->dimensionWorker) {
					dimensionWorker = server->dimensionWorker;
				}
				if (dimensionWorkerConfigured == oldserver->dimensionWorkerConfigured) {
					dimensionWorkerConfigured = server->dimensionWorkerConfigured;
				}
#ifdef ENABLE_GPU_SERVER
				if (enableGpu == oldserver->enableGpu) {
					enableGpu = server->enableGpu;
				}
#endif
			}
		}

		if (dbs->isCheckedOut()) {
			ret = dbs->merge(o != 0 ? server->dbs : PDatabaseList(), shared_from_this());
		} else if (o != 0) {
			dbs = server->dbs;
		}

		if (ret) {
			if (engines->isCheckedOut()) {
				ret = engines->merge(o != 0 ? server->engines : PEngineList(), shared_from_this());
			} else if (o != 0) {
				engines = server->engines;
			}
		}
	}

	if (ret && systemDatabase) {
		ret = systemDatabase->mergespecial(server->systemDatabase, shared_from_this(), true);
	}
	if (ret) {
		commitintern();
		context->flushJournals();
		context->deleteCubesFromDisk();
		context->deleteDatabasesFromDisk();
	}
	return ret;
}

PCommitable Server::copy() const
{
	checkNotCheckedOut();
	PServer news(new Server(*this));
	return news;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check gpu devices ids from config file palo.ini and return gpu ordinals
////////////////////////////////////////////////////////////////////////////////
map<string, int32_t> Server::getGpuDeviceIds()
{
	std::map<string, int32_t> idsMap;
#ifdef ENABLE_GPU_SERVER
	int deviceCount = 0;
	cudaGetDeviceCount(&deviceCount);
	// iterate throw all device
	for (int device = 0; device < deviceCount; ++device) {
		cudaDeviceProp devProp;
		cudaGetDeviceProperties(&devProp, device);
		int ccMajor = devProp.major;
		int ccMinor = devProp.minor;
		// check gpu device Compute Capability
		if (ccMajor > 1 || (ccMajor == 1 && ccMinor >= 3)) {
			// get gpu device id
			int busID = devProp.pciBusID;
			int devID = devProp.pciDeviceID;
			ostringstream tmp;
			tmp << busID << "." << devID;
			Logger::trace << "GPU: Gpu Device "<<device<<" has following id: "<<tmp.str()<<endl;
			idsMap.insert(pair<string, int32_t>(tmp.str(), device));
		}
	}
#endif
	return idsMap;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check gpu devices ids from config file palo.ini and return gpu ordinals
////////////////////////////////////////////////////////////////////////////////
map<string, int32_t> Server::getGpuDeviceIds(vector<string> gpuDeviceIdsOptions)
{
	std::map<string, int32_t> idsMap;
#ifdef ENABLE_GPU_SERVER
	int deviceCount = 0;
	cudaGetDeviceCount(&deviceCount);
	ostringstream firstTeslaGpuName;
	// iterate over all devices
	for (int device = 0; device < deviceCount; ++device) {
		cudaDeviceProp devProp;
		cudaGetDeviceProperties(&devProp, device);
		int ccMajor = devProp.major;
		int ccMinor = devProp.minor;

		ostringstream gpuName;
		gpuName << devProp.name;

		// check if card should be used

		//check gpu device Compute Capability(>= 2) and
		if (ccMajor > 1 || (ccMajor == 1 && ccMinor >= 3)) {
			if (gpuDeviceIdsOptions.size() > 0)	// if user selected specific device id's
			{
				// get gpu device id
				int busID = devProp.pciBusID;
				int devID = devProp.pciDeviceID;
				ostringstream tmp;
				tmp << std::hex << busID << "." << std::hex << devID; // convert integers to hex as used in palo.ini and nvsmi
				if (Logger::isTrace())
					Logger::trace << "GPU: Device " << device << " has ID " << tmp.str() << endl;
				bool gpuFlag = false;	// user selected specific device id's
				for (int i = 0; i < gpuDeviceIdsOptions.size(); i++){
					if (gpuDeviceIdsOptions.at(i) == tmp.str())
						gpuFlag = true;
				}
				if (gpuFlag) {
					// check first valid gpu's name
					if (firstTeslaGpuName.str().size() == 0 && (gpuName.str().find("esla") != string::npos || gpuName.str().find("ESLA") != string::npos))
						firstTeslaGpuName << devProp.name;
					// check that only equivalent gpus are used
					if (firstTeslaGpuName.str().size() > 0 && gpuName.str().find(firstTeslaGpuName.str()) != string::npos)
						idsMap.insert(pair<string, int32_t>(tmp.str(), device));
				}
			}
			else
			{
				// remember first Tesla card
				if (firstTeslaGpuName.str().size() == 0 && (gpuName.str().find("esla") != string::npos || gpuName.str().find("ESLA") != string::npos))
					firstTeslaGpuName << devProp.name;
				// add equivalent GPU cards
				if (firstTeslaGpuName.str().size() > 0 && gpuName.str().find(firstTeslaGpuName.str()) != string::npos) {
					// get gpu device id
					int busID = devProp.pciBusID;
					int devID = devProp.pciDeviceID;
					ostringstream tmp;
					tmp << std::hex << busID << "." << std::hex << devID; // convert integers to hex as used in palo.ini and nvsmi
					if (Logger::isTrace())
						Logger::trace << "GPU: Device " << device << " has ID " << tmp.str() << endl;
					idsMap.insert(pair<string, int32_t>(tmp.str(), device));
				}
			}
		}
	}
#endif
	return idsMap;
}

vector<int32_t> Server::getGpuDeviceOrdinals(map<string, int32_t> &gpuDeviceIdsSystem, size_t numGpusLicense)
{
	// correct mapping of devices
	vector<int32_t> gpuDeviceOrdinals(0);
	if (numGpusLicense > 0) {
		// activate all gpu, count <= number of gpus from license
		for (map<string, int32_t>::iterator devIt = gpuDeviceIdsSystem.begin(); devIt != gpuDeviceIdsSystem.end(); devIt++) {
			if (gpuDeviceOrdinals.size() < numGpusLicense) {
				gpuDeviceOrdinals.push_back(devIt->second);
			}
			else {
				Logger::info << "GPU: Maximum number of licensed GPU devices reached!" << endl;
				break;
			}
		}
	}
	return gpuDeviceOrdinals;
}

vector<int32_t> Server::getGpuDeviceOrdinals(map<string, int32_t> &gpuDeviceIdsSystem, vector<string> gpuDeviceIdsOptions, size_t numGpusLicense)
{
	//correct mapping of devices
	vector<int32_t> gpuDeviceOrdinals(0);
	if (numGpusLicense > 0) {
		size_t devCount = gpuDeviceIdsOptions.size();
		if (devCount == 0) {
			// activate all gpu, count <= number of gpus from license
			for (map<string, int32_t>::iterator devIt = gpuDeviceIdsSystem.begin(); devIt != gpuDeviceIdsSystem.end(); devIt++) {
				if (gpuDeviceOrdinals.size() < numGpusLicense) {
					gpuDeviceOrdinals.push_back(devIt->second);
				} else {
					Logger::info << "GPU: Maximum number of licensed gpu devices reached!" << endl;
					break;
				}
			}
		} else {
			for (size_t i = 0; i < devCount; ++i) {
				string::size_type j = gpuDeviceIdsOptions[i].find(".");
				int busId = strtol(gpuDeviceIdsOptions[i].substr(0, j).c_str(), NULL, 16);
				int devId = strtol(gpuDeviceIdsOptions[i].substr(j + 1, gpuDeviceIdsOptions[i].size()).c_str(), NULL, 16);
				ostringstream tmp;
				tmp << busId << "." << devId;
				map<string, int32_t>::const_iterator devIt = gpuDeviceIdsSystem.find(tmp.str());
				if (devIt != gpuDeviceIdsSystem.end()) {
					if (gpuDeviceOrdinals.size() < numGpusLicense) {
						gpuDeviceOrdinals.push_back(devIt->second);
					} else {
						Logger::info << "GPU: Maximum number of licensed gpu devices reached!" << endl;
						break;
					}
				} else {
					Logger::warning << "GPU: Incorrect device ID defined in palo.ini: " << gpuDeviceIdsOptions[i] << endl;
				}
			}
		}
	}
	return gpuDeviceOrdinals;
}

bool Server::activateGpuEngine(vector<string> gpuDeviceIdsOptions)
{
	checkCheckedOut();
	bool ret = false;
#ifdef ENABLE_GPU_SERVER
	size_t numGpusLicense = getNumberofGPUs();
	vector<int32_t> gpuDeviceOrdinals(0);
	// get gpu device ordinals to start gpu engine
	if (numGpusLicense <= 0) {
		Logger::error << "GPU: Invalid or no license detected - Jedox OLAP Accelerator (Gpu) disabled." << endl;
		enableGpu = false;
	} else {
		map<string, int32_t> gpuDeviceIdsSystem = getGpuDeviceIds(gpuDeviceIdsOptions);
		gpuDeviceOrdinals = getGpuDeviceOrdinals(gpuDeviceIdsSystem, numGpusLicense);

		if (gpuDeviceOrdinals.size() > 0) {
			if (engines->size() == 1) {
				// add EngineGpu
				PEngineBase gpuEngine(new EngineGpu(gpuDeviceOrdinals));
				if (gpuEngine != NULL) {
					if (!engines->isCheckedOut()) {
						engines = COMMITABLE_CAST(EngineList, engines->copy());
					}
					engines->add(gpuEngine, true);
					Logger::debug << "GPU Engine created with Id " << gpuEngine->getId() << endl;
					enableGpu = true;
					ret = true;
				} else {
					Logger::warning << "GPU Engine initialization failed - Jedox OLAP Accelerator (GPU) disabled!" << endl;
					enableGpu = false;
					ret = false;
				}
			}
		} else {
			Logger::error << "GPU: No suitable devices detected - Jedox OLAP Accelerator (GPU) disabled!" << endl;
			enableGpu = false;
			ret = false;
		}
	}
#else
	enableGpu = false;
#endif
	return ret;
}

bool Server::optimizeGpuEngine()
{
	checkCheckedOut();
	bool reordered = false;

#ifdef ENABLE_GPU_SERVER
	if (getEngine(EngineBase::GPU, false) != NULL && getEngine(EngineBase::GPU, false)->getCopiesCount() == 0) {
		try {
			PEngineBase gpuEngine = getEngine(EngineBase::GPU, true);
			for (DatabaseList::Iterator i = dbs->begin(); i != dbs->end(); ++i) {
				PDatabase database = COMMITABLE_CAST(Database, *i);
				if (NULL == database) {
					continue;
				} else {
					PCubeList cubes = database->getCubeList(true);
					for (CubeList::Iterator j = cubes->begin(); j != cubes->end(); ++j) {
						PCube cube = COMMITABLE_CAST(Cube, *j);
						if (NULL == cube)
						continue;

						if(cube->getType() == GPUTYPE) {
							reordered = cube->optimizeNumericStorage(gpuEngine);
						}
					}
				}
			}
		} catch (const ErrorException& e) {
			Logger::warning << "cannot optimize gpu storage '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
		}
	}
#endif
	return reordered;
}

bool Server::needGpuOptimization()
{
	bool ret = false;
#ifdef ENABLE_GPU_SERVER
	PEngineGpu gpuEngine = COMMITABLE_CAST(EngineGpu, getEngine(EngineBase::GPU, false));
	if(gpuEngine != NULL) {
		ret = gpuEngine->needOptimization();
	}
#endif
	return ret;
}

void Server::setDefaultDbRight(string right)
{
	defaultDbRight = User::stringToRightsType(right);
}

RightsType Server::getDefaultDbRight()
{
	return defaultDbRight;
}

ostream& operator<<(ostream& ostr, const vector<CPDatabase> &vdb)
{
	ostr << "Databases: " << endl;
	for (vector<CPDatabase>::const_iterator it = vdb.begin(); it != vdb.end(); ++it) {
		const Database *db = it->get();
		if (db) {
			ostr << db->getName() << ", status: " << db->getStatus();
		} else {
			ostr << db;
		}
		ostr << endl;
	}
	return ostr;
}

}

