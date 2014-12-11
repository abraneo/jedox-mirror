/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Thread/WriteLocker.h"
#include "Olap/Server.h"
#include "Engine/Cache.h"
#include "Olap/Context.h"
#include "Logger/Logger.h"
#include "Olap/Cube.h"
#include "Olap/Database.h"

namespace palo {

class FilteredCellMapStream : public ConstantRepeater
{
public:
	FilteredCellMapStream(PCellStream input, CPArea filter, const CellValue *defaultValue) : ConstantRepeater(filter, defaultValue), input(input), end(filter->pathEnd()), endReached(false) {}
	virtual ~FilteredCellMapStream() {}
	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble() {return getValue().getNumeric();}
	virtual const IdentifiersType &getKey() const;
	virtual void reset();
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed

private:
	PCellStream input;
	CellValue val;
	Area::PathIterator end;
	bool endReached;
	IdentifiersType vkey;
};

bool FilteredCellMapStream::next()
{
	if (defaultValue) {
		if (isDefaultValue()) {
			if (nextRepetition()) {
				return true;
			} else {
				return !endReached;
			}
		} else {
			bool result = false;
			IdentifiersType prevKey(vkey);

			while (!endReached) {
				result = false;
				while (input->next()) {
					if (area->find(input->getKey()) != end) {
						vkey = input->getKey();
						val = input->getDouble();
						result = true;
						break;
					}
				}
				if (result) {
					val = input->getDouble();
					vkey = input->getKey();
					ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, &getKey());
					return true;
				} else {
					endReached = true;
				}
			}
			return 0 != ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, 0);
		}
	} else {
		while (input->next()) {
			if (area->find(input->getKey()) != end) {
				vkey = input->getKey();
				val = input->getDouble();
				return true;
			}
		}
		return false;
	}
}

const CellValue &FilteredCellMapStream::getValue()
{
	if (isDefaultValue()) {
		return ConstantRepeater::getValue();
	} else {
		return val;
	}
}

const IdentifiersType &FilteredCellMapStream::getKey() const
{
	if (isDefaultValue()) {
		return ConstantRepeater::getKey();
	} else {
		return vkey;
	}
}

void FilteredCellMapStream::reset()
{
	input->reset();
	ConstantRepeater::reset();
}

class SingleCellFilter : public CellValueStream
{
public:
	SingleCellFilter(const IdentifiersType &key, const CellValue &val, bool empty) : key(key), val(val), empty(empty), end(empty) {}
	virtual ~SingleCellFilter() {}
	virtual bool next() {if (end) return false; else {end = true; return true;}}
	virtual const CellValue &getValue() {return val;}
	virtual double getDouble() {return val.getNumeric();}
	virtual const IdentifiersType &getKey() const {return key;}
	virtual void reset() {end = empty;}
	virtual bool move(const IdentifiersType &key, bool *found) {
		if (empty) {
			if (found) {
				*found = false;
			}
			return false;
		} else {
			int keyCmp = CellValueStream::compare(key, this->key);
			if (keyCmp) {
				if (found) {
					*found = false;
				}
				return keyCmp < 0; //return true if key < this->key, false if key > this->key
			} else {
				if (found) {
					*found = true;
				}
				return true;
			}
		}
	}

private:
	IdentifiersType key;
	CellValue val;
	bool empty;
	bool end;
};

ValueCache::QueryCache::QueryCache(CPCube cube, size_t initSize, ValueCache &cache) : cube(cube), areas(new CachedAreas), initSize(initSize), cache(cache)
{
	inserted = CreateDoubleCellMap(cache.keySize);
}

ValueCache::QueryCache::~QueryCache()
{
	if (areas->size()) {
		PStorageCpu storage;
		PCachedAreas currAreas;
		CubesWithDBs currDependencies;
		{
			WriteLocker wl(cache.mutex->getLock());
			storage = cache.storage;
			currAreas = cache.areas;
			currDependencies = cache.sourceCubes;
		}
		currAreas.reset(new CachedAreas(*currAreas));
		if (inserted->size()) {
			storage = COMMITABLE_CAST(StorageCpu, storage->copy());
			storage->commitExternalChanges(false, dynamic_cast<ICellMapStream *>(inserted.get())->getValues(), inserted->size() == 1, false);
			storage->merge(CPCommitable(), PCommitable());
		}

		for (CachedAreas::iterator it = areas->begin(); it != areas->end(); ++it) {
			CachedAreas::iterator currit = currAreas->find(it->first);
			if (currit != currAreas->end()) {
				currit->second.reset(new SubCubeList(*currit->second));
				for (SubCubeList::iterator lit = it->second->begin(); lit != it->second->end(); ++lit) {
					currit->second->insertAndMerge(*lit);
				}
			} else {
				currAreas->insert(*it);
			}
		}
		currDependencies.insert(sourceCubes.begin(), sourceCubes.end());
		{
			WriteLocker wl(cache.mutex->getLock());
			cache.storage = storage;
			cache.areas = currAreas;
			cache.sourceCubes = currDependencies;
		}
	}
}

void ValueCache::QueryCache::insert(IdentifierType ruleId, CPCubeArea area, PDoubleCellMap vals, const CubesWithDBs &sourceCubes)
{
	boost::shared_ptr<SubCubeList> currArea = findArea(ruleId);
	currArea->insertAndMerge(area);
	insertVals(vals);
	this->sourceCubes.insert(sourceCubes.begin(), sourceCubes.end());
}

void ValueCache::QueryCache::insert(IdentifierType ruleId, const list<PCubeArea> &ars, PDoubleCellMap vals, const CubesWithDBs &sourceCubes)
{
	boost::shared_ptr<SubCubeList> currArea = findArea(ruleId);
	for (list<PCubeArea>::const_iterator it = ars.begin(); it != ars.end(); ++it) {
		currArea->insertAndMerge(*it);
	}
	insertVals(vals);
	this->sourceCubes.insert(sourceCubes.begin(), sourceCubes.end());
}

boost::shared_ptr<SubCubeList> ValueCache::QueryCache::findArea(IdentifierType ruleId)
{
	CachedAreas::iterator it = areas->find(ruleId);
	boost::shared_ptr<SubCubeList> currArea;
	if (it == areas->end()) {
		currArea.reset(new SubCubeList());
		areas->insert(make_pair(ruleId, currArea));
	} else {
		currArea = it->second;
	}
	return currArea;
}

PCellStream ValueCache::QueryCache::getFilteredValues(CPArea area, const CellValue *defaultValue)
{
	if (area->getSize() == 1) {
		Area::PathIterator pit = area->pathBegin();
		const IdentifiersType &key = *pit;
		double val;
		bool found = inserted->get(key, val);
		PCellStream s(new SingleCellFilter(key, found ? CellValue(val) : (defaultValue ? *defaultValue : CellValue()), !found && !defaultValue));
		return s;
	} else {
		return PCellStream(new FilteredCellMapStream(dynamic_cast<ICellMapStream *>(inserted.get())->getValues(), area, defaultValue));
	}
}

void ValueCache::QueryCache::insertVals(PDoubleCellMap vals)
{
	if (inserted->size()) {
		PCellStream str = dynamic_cast<ICellMapStream *>(vals.get())->getValues();
		while (str->next()) {
			inserted->set(str->getKey(), str->getDouble());
		}
	} else {
		inserted = vals;
	}
}


ValueCache::CacheWriteProcessor::CacheWriteProcessor(QueryCache &cache, CPCubeArea area, PCellStream input, IdentifierType ruleId, size_t initSize) : cache(cache), area(area), input(input), finished(false), nextCalled(false), ruleId(ruleId), initSize(initSize)
{
	if (cache.getBarrier()) {
		inserted = CreateDoubleCellMap(cache.getKeySize());
	}
}

ValueCache::CacheWriteProcessor::~CacheWriteProcessor()
{
	Context *context = Context::getContext(0, false);
	// remove cacheWriter from context stack
	Context::CacheDependences &cacheDependences = context->getCacheDependences();
	if (cacheDependences.find(this) == cacheDependences.end()) {
		// something wrong with the list of cache writers
		Logger::error << "Internal Error: Cache Dependences list is incomplete!" << endl;
	} else {
		if (Logger::isTrace()) {
			ostringstream ss;
			ss << "Cache dependences detected. Source Cubes: ";
			for(CubesWithDBs::const_iterator srcCubeIt = sourceCubes.begin(); srcCubeIt != sourceCubes.end(); ++srcCubeIt) {
				PDatabase db = context->getServer()->lookupDatabase(srcCubeIt->first, false);
				PCube cube = db->lookupCube(srcCubeIt->second, false);
				ss << db->getName() << "/" << cube->getName() << " ";

			}
			Logger::trace << ss.str() << endl;
		}
		cacheDependences.erase(this);
	}

	if (inserted) {
		if (nextCalled) {
			if (finished) {
//				for (Area::PathIterator pit = area->pathBegin(); pit != area->pathEnd(); ++pit) {
//					cout << "C\t" << *pit << endl;
//				}
				cache.insert(ruleId, area, inserted, sourceCubes);
			} else {
				list<PCubeArea> result;
				const IdentifiersType &endKey = input->getKey();
				if (endKey.empty()) {
//					Logger::error << "Key has zero length." << endl;
//					input->getKey();
					return;
				}
				const IdentifiersType start = *area->pathBegin();
				area->split(start, endKey, result, 0);
//				for (list<PCubeArea>::const_iterator arit = result.begin(); arit != result.end(); ++arit) {
//					for (Area::PathIterator pit = (*arit)->pathBegin(); pit != (*arit)->pathEnd(); ++pit) {
//						cout << "C\t" << *pit << endl;
//					}
//				}
				cache.insert(ruleId, result, inserted, sourceCubes);
			}
		} else {
			Logger::debug << "Processor created and was not read at all!" << endl;
		}
	}
}

bool ValueCache::CacheWriteProcessor::next()
{
	nextCalled = true;
	bool ret = input->next();
	if (ret && inserted) {
		CellValue v = input->getValue();
		if (!v.isEmpty()) {
			if (initSize + inserted->size() + 1 > cache.getBarrier()) {
				inserted.reset();
			} else {
				if (v.isNumeric()) {
					const IdentifiersType &key = input->getKey();
					inserted->set(key, v.getNumeric());
				} else {
					inserted.reset();
				}
			}
		}
	}
	finished = !ret;
//	if (!ret) {
//		input.reset();
//	}
	return ret;
}

const CellValue &ValueCache::CacheWriteProcessor::getValue()
{
	return input->getValue();
}

double ValueCache::CacheWriteProcessor::getDouble()
{
	return input->getDouble();
}

const IdentifiersType &ValueCache::CacheWriteProcessor::getKey() const
{
	return input->getKey();
}

void ValueCache::CacheWriteProcessor::reset()
{
	if (cache.getBarrier()) {
		inserted = CreateDoubleCellMap(cache.getKeySize());
	} else {
		inserted.reset();
	}
	finished = false;
	nextCalled = false;
	input->reset();
}

bool ValueCache::CacheWriteProcessor::move(const IdentifiersType &key, bool *found)
{
	if (!nextCalled && key == *area->pathBegin()) {
		// called move(begin) instead of first next
		bool result = next();
		if (found) {
			if (result && getKey() == key) {
				*found = true;
			} else {
				*found = false;
			}
		}
		return result;
	} else {
		inserted.reset();
	}
	return input->move(key, found);
}

void ValueCache::CacheWriteProcessor::addSourceCube(const dbID_cubeID &dbCubeId) {
	sourceCubes.insert(dbCubeId);
}

ValueCache::ValueCache(size_t keySize, double cacheBarrier, double cacheClearBarrierCells) : mutex(new PaloSharedMutex()), keySize(keySize), cacheBarrier(cacheBarrier), cacheClearBarrierCells(cacheClearBarrierCells)
{
	clear();
}

bool ValueCache::clear()
{
	bool result = false;
	WriteLocker w(mutex->getLock());
	storage.reset(new StorageCpu(PPathTranslator()));
	storage->merge(CPCommitable(), PCommitable());
	areas.reset(new CachedAreas());
	result = !sourceCubes.empty();
	sourceCubes.clear();
	found = 0;
	return result;
}

PCellStream ValueCache::getWriter(CPCube cube, CPCubeArea area, PCellStream input, IdentifierType ruleId)
{
	Context *context = Context::getContext();
	boost::shared_ptr<QueryCache> qc = context->getQueryCache(cube, storage->valuesCount(), *this);
	CacheWriteProcessor *cacheProc = new CacheWriteProcessor(*qc, area, input, ruleId, qc->getInitSize());
	PCellStream result(cacheProc);
	context->getCacheDependences().insert(cacheProc);
	return result;
}

double ValueCache::getAreasAndStorage(PStorageCpu &stor, PCachedAreas &ars)
{
	WriteLocker w(mutex->getLock());
	stor = storage;
	ars = areas;
	return found;
}

void ValueCache::getStatistics(PStorageCpu cacheStorage, PCachedAreas cacheAreas, double cacheFound, size_t &areasCount, size_t &valuesCount, double &cellCount, double &foundCellsCount)
{
	areasCount = 0;
	valuesCount = cacheStorage->valuesCount();
	cellCount = 0;
	foundCellsCount = cacheFound;
	for (CachedAreas::iterator ruleAreas = cacheAreas->begin(); ruleAreas != cacheAreas->end(); ++ruleAreas) {
		areasCount += ruleAreas->second->size();
		cellCount += ruleAreas->second->cellCount();
	}
}

void ValueCache::increaseFound(double f)
{
	PStorageCpu cacheStorage;
	PCachedAreas cacheAreas;
	double cacheFound = 0;
	{
		WriteLocker w(mutex->getLock());
		found += f;
		if (Logger::isDebug()) {
			cacheStorage = storage;
			cacheAreas = areas;
			cacheFound = found;
		}
	}
	if (cacheStorage && cacheAreas) {
		double totalCellCount = 0;
		size_t areasCount = 0;
		size_t valuesCount;
		double foundCellsCount;
		getStatistics(cacheStorage, cacheAreas, cacheFound, areasCount, valuesCount, totalCellCount, foundCellsCount);
		Logger::debug << "cells/areas/values in cache: " << totalCellCount << "/" << areasCount << "/" << valuesCount << " Cells found in cache: " <<  f << " Accumulated: " << foundCellsCount << endl;
	}
}

bool ValueCache::isDepending(const dbID_cubeID &cubeId) const
{
	return sourceCubes.find(cubeId) != sourceCubes.end();
}

}
