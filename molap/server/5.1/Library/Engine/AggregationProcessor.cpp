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
 * 
 *
 */

#include "Engine/AggregationProcessor.h"
#include "Engine/StorageCpu.h"
#include "Logger/Logger.h"
#include "InputOutput/Condition.h"

namespace palo {

HashValueStorage::HashValueStorage(CPArea areaSP) : results(0), resultSet(0), areaSP(areaSP), area(areaSP.get()), firstIds(0), lastIds(0), offsets(0), resultSize(0)
{
	// create key to offset mapping
	dimCount = area->dimCount();
	double areaSize = area->getSize();
	if (areaSize > double(MAX_OFFSET)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage area too big!");
	}
	resultSize = size_t(areaSize);
	firstIds = new IdentifierType[dimCount];
	lastIds = new IdentifierType[dimCount];
	offsets = new OffsetType*[dimCount];
	for (size_t dim = 0; dim < area->dimCount(); dim++) {
		const Set *set = area->getDim(dim).get();
		IdentifierType firstId = *set->begin();
		IdentifierType lastId = *(--set->end());

		firstIds[dim] = firstId;
		lastIds[dim] = lastId;
		OffsetType *dimOffsets = new OffsetType[lastId-firstId+1];
		memset(dimOffsets, 0, sizeof(OffsetType)*(lastId-firstId+1));
		offsets[dim] = dimOffsets;
		areaSize /= set->size();
		OffsetType elemOffset = 0;
		OffsetType dimOffsetStep = OffsetType(areaSize);
		for (Set::Iterator elemIt = set->begin(); elemIt != set->end(); ++elemIt) {
			dimOffsets[*elemIt-firstId] = elemOffset;
			elemOffset += dimOffsetStep;
		}
	}
	// allocate array
	results = new double[resultSize];
	memset(results, 0, resultSize * sizeof(double));
	resultSet = new uint8_t[resultSize];
	memset(resultSet, 0, resultSize);
}

HashValueStorage::OffsetType HashValueStorage::offsetFromPath(const IdentifierType *path) const
{
	OffsetType offset = 0;
	const IdentifierType *elemId = path;
	IdentifierType *firstId = firstIds;

	for (size_t dim = 0; dim < dimCount; dim++, elemId++, firstId++) {
		offset += offsets[dim][*elemId-*firstId];
	}
#ifdef _DEBUG
	if (resultSize < offset) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage::offsetFromPath: offset out of range!");
	}
#endif
	return offset;
}

HashValueStorage::OffsetType HashValueStorage::offsetMoveToPath(const IdentifierType *path, bool *found) const
{
	OffsetType offset = 0;
	const IdentifierType *elemId = path;
	IdentifierType *firstId = firstIds;
	IdentifierType *lastId = lastIds;

	for (size_t dim = 0; dim < dimCount; ++dim, ++elemId, ++firstId, ++lastId) {
		if (*elemId < *firstId) {
			if (found) {
				*found = false;
			}
		} else if (*elemId > *lastId) {
			if (found) {
				*found = false;
			}
		} else {
			IdentifierType id = *elemId;
			OffsetType *poff = offsets[dim]+*elemId-*firstId;
			if (!*poff && *elemId != *firstId) {
				if (found) {
					*found = false;
				}
				while (++id <= *lastId) {
					++poff;
					if(*poff) {
						break;
					}
				}
			}
			offset += *poff;
		}
	}
#ifdef _DEBUG
	if (resultSize < offset) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage::offsetMoveToPath: offset out of range!");
	}
#endif
	return offset;
}

HashValueStorage::~HashValueStorage()
{
	delete []results; results = 0;
	delete []resultSet;
	delete []firstIds;
	delete []lastIds;
	if (offsets) {
		for (size_t dim = 0; dim < dimCount; dim++) {
			delete []offsets[dim];
		}
		delete[] offsets;
	}
}

bool HashValueStorage::set(const IdentifierType *path, const double &value)
{
	OffsetType offset = offsetFromPath(path);
	bool res = resultSet[offset] == 0;
	results[offset] = value;
	resultSet[offset] = 1;
	return res;
}

bool HashValueStorage::add(const IdentifierType *path, const double &value)
{
	OffsetType offset = offsetFromPath(path);
	bool res = resultSet[offset] == 0;
	results[offset] += value;
	resultSet[offset] = 1;
	return res;
}

bool HashValueStorage::get(const IdentifierType *path, double &value) const
{
	OffsetType offset = offsetFromPath(path);
	value = results[offset];
	return resultSet[offset] != 0;
}

PProcessorBase HashValueStorage::getValues()
{
	return PProcessorBase(new Reader(boost::dynamic_pointer_cast<HashValueStorage>(this->shared_from_this())));
}

bool HashValueStorage::Reader::next()
{
//	return move(EMPTY_KEY, 0);
	if (!started) {
		started = true;
		if (pathIt != endIt && storage.resultSet[offset]) {
			return true;
		}
	}
	while (pathIt != endIt) {
		++pathIt;
		if (pathIt != endIt && storage.resultSet[++offset]) {
			return true;
		}
	}
	return false;
}

const CellValue &HashValueStorage::Reader::getValue()
{
	value = getDouble();
	if (value.isEmpty() && storage.resultSet[offset]) {
		value.setEmpty(false);
	}
	return value;
}

double HashValueStorage::Reader::getDouble()
{
#ifdef _DEBUG
	if (storage.resultSize < offset) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage::Reader: offset out of range!");
	}
#endif
	return storage.results[offset];
}

const IdentifiersType &HashValueStorage::Reader::getKey() const
{
	if (!started) {
		return EMPTY_KEY;
	}
	return *pathIt;
}

void HashValueStorage::Reader::reset()
{
	started = false;
	offset = 0;
	pathIt = beginIt;
}

bool HashValueStorage::Reader::move(const IdentifiersType &key, bool *found)
{
	started = true;
	OffsetType newOffset = key.empty() ? offset+1 : storage.offsetMoveToPath(&key[0], 0);
	size_t moveCounter = 0;

	while (offset < storage.resultSize) {
		if (offset >= newOffset && storage.resultSet[offset]) {
			// reconstruct pathIt
			pathIt = pathIt + double(moveCounter);
			// set found flag
			if (found) {
				// compare key
				if (offset == newOffset) {
					*found = true;
				} else {
					*found = false;
				}
			}
			return true;
		}
		offset++;
		moveCounter++;
	}
	if (found) {
		*found = false;
	}
	return false;
}

FilteredReader::FilteredReader(PEngineBase engine, PCellStream stream, CPCondition condition, CPArea area) :
	ProcessorBase(true, engine), conditionSP(condition), condition(condition.get()), stream(stream), area(area), init(true), fromStream(true), streamEnd(false)
{
	generateEmpty = condition->check(CellValue());
	pathIt = area->pathBegin();
}

bool FilteredReader::next()
{
	if (generateEmpty) {
		if (pathIt != area->pathEnd()) {
			while (true) {
				if (init) {
					streamEnd = !stream->next();
					init = false;
				} else {
					if (!streamEnd && *pathIt == stream->getKey()) {
						streamEnd = !stream->next();
					}
					++pathIt;
					if (pathIt == area->pathEnd()) {
						break;
					}
				}
				if (streamEnd || *pathIt < stream->getKey()) {
					fromStream = false;
					return true;
				} else if (!streamEnd && condition->check(stream->getValue())) {
					fromStream = true;
					return true;
				}
			}
		}
	} else {
		while (stream->next()) {
			const CellValue &val = stream->getValue();
			if (condition->check(val)) {
				return true;
			}
		}
	}
	return false;
}

AggregationProcessor::AggregationProcessor(PEngineBase engine, CPPlanNode node)
	: ProcessorBase(true, engine), engine(engine), planNode(node)
{
	aggregationPlan = dynamic_cast<const AggregationPlanNode *>(planNode.get());
}

AggregationProcessor::~AggregationProcessor()
{
}

bool AggregationProcessor::next()
{
	if (!storage) {
		aggregate();
	}
	return storageReader->next();
}

const CellValue &AggregationProcessor::getValue()
{
	return storageReader->getValue();
}

const IdentifiersType &AggregationProcessor::getKey() const
{
	return storageReader ? storageReader->getKey() : EMPTY_KEY;
}

void AggregationProcessor::reset()
{
	if (storageReader) {
		storageReader->reset();
	}
}

bool AggregationProcessor::move(const IdentifiersType &key, bool *found)
{
//	return CellValueStream::move(key, found);
	if (!storage) {
		// TODO: -jj optimize aggregation from key for better performance
		aggregate();
	}
	return storageReader->move(key, found);
}

void AggregationProcessor::aggregateCell(const IdentifiersType &key, const double value)
{
	double fixedWeight;
	size_t multiDimCount;
	initParentKey(key, multiDimCount, &fixedWeight);

	// for each parent cell
	size_t changeMultiDim;
	do {
		double weight = fixedWeight;
		// add value * weight to parent
		for (size_t multiDim = 0; multiDim < multiDimCount; multiDim++) {
			weight *= currentTarget[multiDims[multiDim]].getWeight();
		}
		if (weight != 1) {
			storage->add(parentKey, weight * value);
		} else {
			storage->add(parentKey, value);
		}
		nextParentKey(multiDimCount, changeMultiDim);
	} while (changeMultiDim >= 0 && changeMultiDim < multiDimCount);
}

void AggregationProcessor::initIntern()
{
	dimCount = aggregationPlan->getArea()->dimCount();
	lastTargets.resize(dimCount);
	currentTarget.resize(dimCount);
	parentKey.resize(dimCount);
	multiDims.resize(dimCount);
	prevSourceKey = IdentifiersType(dimCount, NO_IDENTIFIER);
	lastKeyParent = prevSourceKey;

	parentMaps.resize(dimCount);
	for (size_t dim = 0; dim < dimCount; dim++) {
		parentMaps[dim] = &aggregationPlan->getAggregationMaps()->at(dim);
	}
}

PProcessorBase AggregationProcessor::getCalculatedValues()
{
	if (storage) {
		if (aggregationPlan->getMaxCount() == UNLIMITED_UNSORTED_PLAN || aggregationPlan->getMaxCount() == UNLIMITED_SORTED_PLAN) {
			return storage->getValues();
		}
	}
	return PProcessorBase();
}

void AggregationProcessor::aggregate()
{
	// not yet calculated
	double resultSize = aggregationPlan->getArea()->getSize();
	HashValueStorage *hashStorage = 0;
	if (resultSize < 1000) {
		hashStorage = new HashValueStorage(aggregationPlan->getArea());
		storage.reset(hashStorage);
	} else {
		storage = CreateDoubleCellMap(aggregationPlan->getArea()->dimCount());
		storage->setLimit(IdentifiersType(), (aggregationPlan->getMaxCount() == 0 ? 0 : aggregationPlan->getMaxCount() + 1));
	}

	initIntern();

	// process all source values
	const vector<PPlanNode> &sources = planNode->getChildren();
	for (vector<PPlanNode>::const_iterator source = sources.begin(); source != sources.end(); ++source) {
		PCellStream sourceDataSP = createProcessor(*source, false);
		CellValueStream *sourceData = sourceDataSP.get();

		StorageCpu::Processor *cpuProc = dynamic_cast<StorageCpu::Processor *>(sourceData);
		if (hashStorage && cpuProc && aggregationPlan->getAggregationType() == AggregationPlanNode::SUM) {
			cpuProc->aggregate(hashStorage, &parentMaps[0]);
		} else {
			while (sourceData->next()) {
				// process the value
				aggregateCell(sourceData->getKey(), sourceData->getDouble());
			}
		}
	}
	if (Logger::isDebug()) {
		if (resultSize > 1000) {
			Logger::debug << "Aggregated area of " << resultSize << " cells. " << storage->size() << " aggregations exists." << endl;
		} else {
			Logger::debug << "Aggregated area of " << resultSize << " cells. " << endl;
		}
	}
	// create iterator
	storageReader = storage->getValues();
}

void AggregationProcessor::initParentKey(const IdentifiersType &key, size_t &multiDimCount, double *fixedWeight)
{
	if (fixedWeight) {
		*fixedWeight = 1;
	}
	// generate all combinations of parents
	multiDimCount = 0;
	vector<AggregationMap::TargetReader>::iterator lastTarget = lastTargets.begin();
	IdentifiersType::iterator prevSourceKeyIt = prevSourceKey.begin();
	IdentifiersType::const_iterator elemId = key.begin();
	for (size_t dim = 0; dim < dimCount; dim++, ++lastTarget, ++prevSourceKeyIt, ++elemId) {
		AggregationMap::TargetReader targets;
		IdentifierType lastTargetId = NO_IDENTIFIER;

		if (*elemId != *prevSourceKeyIt) {
			*prevSourceKeyIt = *elemId;
			targets = parentMaps[dim]->getTargets(*elemId);
			*lastTarget = targets;
			if (targets.size() == 1) {
				lastTargetId = *targets;
			}
			lastKeyParent[dim] = lastTargetId;
		} else {
			//dimParents = *lastParent;
			targets = *lastTarget;
			lastTargetId = lastKeyParent[dim];
		}
		if (lastTargetId != NO_IDENTIFIER) {
			// exactly one parent with default weight
			parentKey[dim] = lastTargetId;
		}

		parentKey[dim] = *targets;
		currentTarget[dim] = targets;
		if (targets.size() > 1) {
			// multiple targets for this source
			multiDims[multiDimCount++] = (IdentifierType)dim;
		} else {
			// single target
			if (fixedWeight) {
				*fixedWeight *= targets.getWeight();
			}
		}
	}
}

void AggregationProcessor::nextParentKey(size_t multiDimCount, size_t &changeMultiDim)
{
	changeMultiDim = multiDimCount - 1;
	bool endOfIteration = false;
	while (!endOfIteration && changeMultiDim >= 0 && changeMultiDim < multiDimCount) {
		size_t currentDim = multiDims[changeMultiDim];
		AggregationMap::TargetReader &target = currentTarget[currentDim];

		++target;
		if (target.end()) {
			target.reset();
			changeMultiDim--;
		} else {
			endOfIteration = true;
		}
		parentKey[currentDim] = *target;
	}
}

AggregationFunctionProcessor::AggregationFunctionProcessor(PEngineBase engine, CPPlanNode node) : AggregationProcessor(engine, node)
{
	aggrType = aggregationPlan->getAggregationType();
	filteredDim = aggregationPlan->getDimIndex();
	isFiltered = filteredDim != NO_DFILTER;

	// supported functions
	if (isFiltered) {
		if (aggrType != AggregationPlanNode::SUM && aggrType != AggregationPlanNode::AVG && aggrType != AggregationPlanNode::MAX && aggrType != AggregationPlanNode::MIN) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid DFilter aggregation function");
		}
	} else {
		if (aggrType != AggregationPlanNode::AVG && aggrType != AggregationPlanNode::COUNT && aggrType != AggregationPlanNode::MAX && aggrType != AggregationPlanNode::MIN) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid aggregation function");
		}
	}

	cellsPerElement = aggregationPlan->getCellsPerElement();
	if (isFiltered || aggrType == AggregationPlanNode::AVG) {
		counter = CreateCellMap<uint32_t>(aggregationPlan->getArea()->dimCount());
	}
}

void AggregationFunctionProcessor::aggregate()
{
	AggregationProcessor::aggregate();
	if (isFiltered) {
		aggregateEmptyCells();
	}
}

void AggregationFunctionProcessor::aggregateCell(const IdentifiersType &key, const double value)
{
	if (isFiltered) {
		if (aggrType == AggregationPlanNode::SUM || aggrType == AggregationPlanNode::AVG) {
			AggregationProcessor::aggregateCell(key, value);
			return;
		}
	}

	size_t multiDimCount;
	initParentKey(key, multiDimCount, 0);

	// for each parent cell
	size_t changeMultiDim;
	do {
		double oldVal, newVal;
		uint32_t valueCount;
		bool setValue = false;
		bool setCounter = false;

		bool found = storage->get(parentKey, oldVal);
		if (isFiltered) { // AggregationPlanNode::MAX or AggregationPlanNode::MIN
			setCounter = true;
			valueCount = 1;

			if (!found) {
				setValue = true;
				newVal = value;
			} else {
				if (aggrType == AggregationPlanNode::MAX) {
					if (value > oldVal) {
						setValue = true;
						newVal = value;
					}
				} else { // AggregationPlanNode::MIN
					if (value < oldVal) {
						setValue = true;
						newVal = value;
					}
				}
				if (counter->get(parentKey, valueCount)) {
					valueCount++;
				} else {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid counter for DFilter aggregation function");
				}
			}
		} else {
			if (!found) {
				setValue = true;
				switch (aggrType) {
				case AggregationPlanNode::AVG:
					newVal = value;
					setCounter = true;
					valueCount = 1;
					break;
				case AggregationPlanNode::COUNT:
					newVal = 1.0;
					break;
				case AggregationPlanNode::MAX:
				case AggregationPlanNode::MIN:
					newVal = value;
					break;
				default: ;
				}
			} else {
				switch (aggrType) {
				case AggregationPlanNode::AVG:
					setValue = true;
					newVal = oldVal + value;
					setCounter = true;
					if (counter->get(parentKey, valueCount)) {
						valueCount++;
					} else {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid counter for AVG aggregation function");
					}
					break;
				case AggregationPlanNode::COUNT:
					setValue = true;
					newVal = oldVal + 1.0;
					break;
				case AggregationPlanNode::MAX:
					if (value > oldVal) {
						setValue = true;
						newVal = value;
					}
					break;
				case AggregationPlanNode::MIN:
					if (value < oldVal) {
						setValue = true;
						newVal = value;
					}
					break;
				default: ;
				}
			}
		}
		if (setValue) {
			storage->set(parentKey, newVal);
		}
		if (setCounter) {
			counter->set(parentKey, valueCount);
		}
		nextParentKey(multiDimCount, changeMultiDim);
	} while (changeMultiDim >= 0 && changeMultiDim < multiDimCount);
}

const CellValue &AggregationFunctionProcessor::getValue()
{
	const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(planNode.get());
	if (apn->getAggregationType() == AggregationPlanNode::AVG) {
		uint32_t count;
		bool found = counter->get(getKey(), count);
		if (found) {
			currentAvg = AggregationProcessor::getDouble() / count;
			return currentAvg;
		}
	}
	return AggregationProcessor::getValue();
}

void AggregationFunctionProcessor::aggregateEmptyCells()
{
	CPSet filteredSet = aggregationPlan->getArea()->getDim(filteredDim);

	IdentifiersType key(*aggregationPlan->getArea()->pathBegin());
	const Condition *cond = aggregationPlan->getCondition().get();
	bool addZero = !cond || cond->check(0.0);

	for (Set::Iterator it = filteredSet->begin(); it != filteredSet->end(); ++it) {
		key[filteredDim] = *it;

		double val;
		if (storage->get(key, val)) {
			if (aggrType == AggregationPlanNode::SUM) {
				continue;
			} else if (aggrType == AggregationPlanNode::AVG) {
				storage->set(key, val / cellsPerElement);
			} else { // AggregationPlanNode::MAX or AggregationPlanNode::MIN
				uint32_t valueCount;
				if (!counter->get(key, valueCount)) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid counter for DFilter aggregation function");
				}
				if (valueCount < cellsPerElement) { // there was at least one #N/A for element *it
					if (aggrType == AggregationPlanNode::MAX) {
						if (val < 0.0) {
							storage->set(key, 0.0);
						}
					} else { // AggregationPlanNode::MIN
						if (val > 0.0) {
							storage->set(key, 0.0);
						}
					}
				}
			}
		} else if (addZero) {
			storage->set(key, 0.0);
		}
	}
}

}
