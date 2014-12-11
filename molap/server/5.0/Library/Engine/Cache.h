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

#ifndef OLAP_ENGINE_CACHE_H_
#define OLAP_ENGINE_CACHE_H_

#include "palo.h"
#include "Engine/Streams.h"
#include "Engine/StorageCpu.h"
#include "Olap/SubCubeList.h"

namespace palo {

class SERVER_CLASS ValueCache {
public:
	typedef map<IdentifierType, boost::shared_ptr<SubCubeList> > CachedAreas;
	typedef boost::shared_ptr<CachedAreas> PCachedAreas;
	typedef boost::shared_ptr<const CachedAreas> CPCachedAreas;

	class QueryCache {
	public:
		QueryCache(CPCube cube, size_t initSize, ValueCache &cache);
		~QueryCache();

		void insert(IdentifierType ruleId, CPCubeArea area, PDoubleCellMap vals, const CubesWithDBs &sourceCubes);
		void insert(IdentifierType ruleId, const list<PCubeArea> &ars, PDoubleCellMap vals, const CubesWithDBs &sourceCubes);

		size_t getKeySize() {return cache.keySize;}
		double getBarrier() {return cache.cacheBarrier;}
		size_t getInitSize() {return initSize + inserted->size();}
		CPCachedAreas getAreas() {return areas;}

		PCellStream getFilteredValues(CPArea area, const CellValue *defaultValue);

	private:
		boost::shared_ptr<SubCubeList> findArea(IdentifierType ruleId);
		void insertVals(PDoubleCellMap vals);

		CPCube cube;
		PDoubleCellMap inserted;
		PCachedAreas areas;
		size_t initSize;
		ValueCache &cache;
		CubesWithDBs sourceCubes;
	};

	class CacheWriteProcessor : public CellValueStream {
		friend class ValueCache;
	public:
		virtual ~CacheWriteProcessor();
		virtual bool next();
		virtual const CellValue &getValue();
		virtual double getDouble();
		virtual const IdentifiersType &getKey() const;
		virtual void reset();
		virtual bool move(const IdentifiersType &key, bool *found);
		void addSourceCube(const dbID_cubeID &dbCubeId);
		CPCubeArea getCubeArea() const {return area;}
		PCellStream getInputProcessor() {return input;}
	private:
		CacheWriteProcessor(QueryCache &cache, CPCubeArea area, PCellStream input, IdentifierType ruleId, size_t initSize);
		QueryCache &cache;
		CPCubeArea area;
		PCellStream input;
		bool finished;
		bool nextCalled;
		IdentifierType ruleId;
		PDoubleCellMap inserted;
		size_t initSize;
		CubesWithDBs sourceCubes;
	};

	ValueCache(size_t keySize, double cacheBarrier, double cacheClearBarrierCells);
	~ValueCache() {}
	bool clear();
	PCellStream getWriter(CPCube cube, CPCubeArea area, PCellStream input, IdentifierType ruleId);
	double getAreasAndStorage(PStorageCpu &stor, PCachedAreas &ars);
	static void getStatistics(PStorageCpu cacheStorage, PCachedAreas cacheAreas, double cacheFound, size_t &areasCount, size_t &valuesCount, double &cellCount, double &foundCellsCount);
	double getBarrier() const {return cacheBarrier;}
	void increaseFound(double f);
	bool isDepending(const dbID_cubeID &cubeId) const;
private:
	ValueCache(const ValueCache &);

	PSharedMutex mutex;
	PStorageCpu storage;
	PCachedAreas areas;
	size_t keySize;
	double cacheBarrier;
	double cacheClearBarrierCells;
	double found;
	CubesWithDBs sourceCubes;
};

class SERVER_CLASS RuleCacheProcessor : public CellValueStream {
public:
	RuleCacheProcessor(PCellStream pSource, IdentifierType ruleId) : pSource(pSource), source(pSource.get()), ruleId(ruleId) {}
	virtual ~RuleCacheProcessor() {}
	virtual bool next() {return source->next();}
	virtual const CellValue &getValue() {value = source->getValue(); value.setRuleId(ruleId); return value;}
	virtual double getDouble() {return value.getNumeric();}
	virtual const IdentifiersType &getKey() const {return source->getKey();}
	virtual void reset() {source->reset();}
	virtual bool move(const IdentifiersType &key, bool *found) {return source->move(key, found);}
private:
	PCellStream pSource;
	CellValueStream *source;
	IdentifierType ruleId;
	CellValue value;
};

}

#endif /* OLAP_ENGINE_CACHE_H_ */
