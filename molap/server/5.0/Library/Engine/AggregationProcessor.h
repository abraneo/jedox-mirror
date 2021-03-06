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
 * 
 *
 */

#ifndef OLAP_AGGREGATION_PROCESSOR_H
#define OLAP_AGGREGATION_PROCESSOR_H 1

#include "palo.h"
#include "Collections/CellMap.h"
#include "Engine/EngineCpu.h"
#include "Engine/Streams.h"
#include "Engine/StorageCpu.h"

namespace palo {

class HashValueStorage : public ICellMap<double>, public ICellMapStream
{
public:
	typedef uint16_t OffsetType;
	static const OffsetType MAX_OFFSET = OffsetType(-1);

	HashValueStorage(CPArea areaSP);
	virtual ~HashValueStorage();
	virtual bool set(const IdentifierType *path, const double &value);
	virtual bool set(const IdentifiersType &path, const double &value) {return set(&path[0], value);}
	virtual bool add(const IdentifierType *path, const double &value);
	virtual bool add(const IdentifiersType &path, const double &value) {return add(&path[0], value);}
	virtual bool get(const IdentifierType *path, double &value) const;
	virtual bool get(const IdentifiersType &path, double &value) const {return get(&path[0], value);}
	virtual size_t size() const {throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage::size() not yet implemented");}
	virtual void setLimit(const IdentifiersType &startPath, uint64_t maxCount) {}
	virtual PCellStream getValues();
public:
	friend class Reader;
	class Reader : public CellValueStream {
	public:
		Reader(const boost::shared_ptr<HashValueStorage> &storageSP) : started(false), storageSP(storageSP), storage(*storageSP), offset(0), pathIt(storage.area->pathBegin()) {}
		virtual ~Reader() {}
		virtual bool next();
		virtual const CellValue &getValue();
		virtual double getDouble();
		virtual const IdentifiersType &getKey() const;
		virtual void reset();
		//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed
	private:
		bool started;
		boost::shared_ptr<HashValueStorage> storageSP;
		HashValueStorage &storage;
		OffsetType offset;
		Area::PathIterator pathIt;
		CellValue value;
	};
private:
	OffsetType offsetFromPath(const IdentifierType *path) const;
	double *results;
	uint8_t *resultSet;
	CPArea areaSP;
	const Area *area;
	size_t dimCount;
	IdentifierType *firstIds;
	OffsetType **offsets;
	size_t resultSize;

	friend class StorageCpu::Processor;
};

class SERVER_CLASS AggregationProcessor : public ConstantRepeater {
public:
	AggregationProcessor(PEngineBase engine, CPPlanNode node);
	virtual ~AggregationProcessor();

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble() {return getValue().getNumeric();}
	virtual const IdentifiersType &getKey() const;
	virtual void reset();
	virtual bool move(const IdentifiersType &key, bool *found);

	PCellStream getCalculatedValues();
protected:
	virtual void aggregate();
	virtual void aggregateCell(const IdentifiersType &key, const double value);

	void initIntern();

	void initParentKey(const IdentifiersType &key, size_t &multiDimCount, double *fixedWeight);
	void nextParentKey(size_t multiDimCount, size_t &changeMultiDim);

	PEngineBase engine;
	const AggregationPlanNode *aggregationPlan;
	CPPlanNode planNode;
	CPPathTranslator pathTranslator;
	GpuBinPath binPath;
	PDoubleCellMap storage;
	PCellStream storageReader;
	CellValue currentValue;
	IdentifiersType vkey;
	bool endReached;
	IdentifiersType lastKey;
	IdentifiersType lastKeyParent;
	vector<const AggregationMap::ParentMapType *> lastParents;
	vector<AggregationMap::ParentMapType::const_iterator> currentParent;
	vector<AggregationMap::ParentMapType::const_iterator> endParent;
	IdentifiersType parentKey;
	size_t dimCount;
	IdentifiersType multiDims;
	const AggregationMap **parentMaps;
	const IdentifierType **parentsBegin;
	const IdentifierType **parentsEnd;
	const IdentifierType **currParent;
	bool newAlg;
};

class SERVER_CLASS AggregationFunctionProcessor : public AggregationProcessor {
public:
	AggregationFunctionProcessor(PEngineBase engine, CPPlanNode node);
	virtual ~AggregationFunctionProcessor() {};

	virtual const CellValue &getValue();

protected:
	virtual void aggregate();
	virtual void aggregateCell(const IdentifiersType &key, const double value);

private:
	typedef boost::shared_ptr<ICellMap<int> > PCounter;

	PCounter counter;
	CellValue currentAvg;
};

}

#endif
