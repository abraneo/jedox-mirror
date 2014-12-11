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

#include "Engine/AggregationProcessor.h"
#include "Engine/StorageCpu.h"
#include "Logger/Logger.h"

namespace palo {

HashValueStorage::HashValueStorage(CPArea areaSP) : results(0), resultSet(0), areaSP(areaSP), area(areaSP.get()), firstIds(0), offsets(0), resultSize(0)
{
	// create key to offset mapping
	dimCount = area->dimCount();
	double areaSize = area->getSize();
	if (areaSize > double(MAX_OFFSET)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "HashValueStorage area too big!");
	}
	resultSize = size_t(areaSize);
	firstIds = new IdentifierType[dimCount];
	offsets = new OffsetType*[dimCount];
	for (size_t dim = 0; dim < area->dimCount(); dim++) {
		const Set *set = area->getDim(dim).get();
		IdentifierType firstId = *set->begin();
		IdentifierType lastId = *(--set->end());

		firstIds[dim] = firstId;
		OffsetType *dimOffsets = new OffsetType[lastId-firstId+1];
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

HashValueStorage::~HashValueStorage()
{
	delete []results; results = 0;
	delete []resultSet;
	delete []firstIds;
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

PCellStream HashValueStorage::getValues()
{
	return PCellStream(new Reader(boost::dynamic_pointer_cast<HashValueStorage>(this->shared_from_this())));
}

bool HashValueStorage::Reader::next()
{
	if (!started) {
		started = true;
		if (pathIt != storage.area->pathEnd() && storage.resultSet[offset]) {
			return true;
		}
	}
	while (pathIt != storage.area->pathEnd()) {
		++pathIt;
		if (pathIt != storage.area->pathEnd() && storage.resultSet[++offset]) {
			return true;
		}
	}
	return false;
}

const CellValue &HashValueStorage::Reader::getValue()
{
	value = getDouble();
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
	pathIt = storage.area->pathBegin();
}

AggregationProcessor::AggregationProcessor(PEngineBase engine, CPPlanNode node)
	: ConstantRepeater(node->getArea(), node->getDefaultValue()), engine(engine), planNode(node), endReached(false),
	  parentMaps(0), parentsBegin(0), parentsEnd(0), currParent(0), newAlg(true)
{
	aggregationPlan = dynamic_cast<const AggregationPlanNode *>(planNode.get());
}

AggregationProcessor::~AggregationProcessor()
{
	delete []parentMaps;
	parentMaps = 0;

	delete []parentsBegin;
	parentsBegin = 0;

	delete []parentsEnd;
	parentsEnd =0;

	delete []currParent;
	currParent = 0;
}

bool AggregationProcessor::next()
{
	if (!storage) {
		aggregate();
	}
	if (defaultValue) {
		if (isDefaultValue()) {
			if (nextRepetition()) {
				// R
				return true;
			} else {
				// use current existing V
				return !endReached;
			}
		} else {
			// search for next existing value (V | R | E)
			bool result = false;
			IdentifiersType prevKey(vkey);

			while (!endReached) {
				result = storageReader->next();
				if (result) {
					currentValue = storageReader->getDouble();
					vkey = storageReader->getKey();
					ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, &getKey());
					return true;
				} else {
					endReached = true;
				}
			}
			return 0 != ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, 0);
		}
	} else {
		bool result = storageReader->next();
		if (result) {
			vkey = storageReader->getKey();
			currentValue = storageReader->getDouble();
//			cout << "A\t" << vkey << "\t" << currentValue << endl;
		}
		if (result && vkey.empty()) {
//			Logger::error << "AggregationProcessor::getKey: Key has zero length." << endl;
//			if (storage) {
//				storage->size();
//			}
		}
		return result;
	}
}

const CellValue &AggregationProcessor::getValue()
{
	if (isDefaultValue()) {
		return ConstantRepeater::getValue();
	} else {
		return currentValue;
	}
}

const IdentifiersType &AggregationProcessor::getKey() const
{
	if (isDefaultValue()) {
		return ConstantRepeater::getKey();
	} else {
//		if (vkey.empty()) {
//			// can happen when job is aborted
//			Logger::error << "AggregationProcessor::getKey: Key has zero length." << endl;
//			if (storage) {
//				storage->size();
//			}
//		}
		return vkey;
	}
}

void AggregationProcessor::reset()
{
	// TODO -jj-
	throw ErrorException(ErrorException::ERROR_INTERNAL, "AggregationProcessor::reset not yet implemented!");
}

bool AggregationProcessor::move(const IdentifiersType &key, bool *found)
{
	// TODO: -jj implement for better performance
	//throw ErrorException(ErrorException::ERROR_INTERNAL, "AggregationProcessor::move not yet implemented!");
	return CellValueStream::move(key, found);
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
			if (!parentsBegin[multiDims[multiDim]]) {
				weight *= currentParent[multiDims[multiDim]]->second;
			}
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
	lastParents.resize(dimCount);
	currentParent.resize(dimCount);
	endParent.resize(dimCount);
	parentKey.resize(dimCount);
	multiDims.resize(dimCount);
	lastKey = IdentifiersType(dimCount, NO_IDENTIFIER);
	lastKeyParent = lastKey;

	parentMaps = new const AggregationMap*[dimCount];
	for (size_t dim = 0; dim < dimCount; dim++) {
		parentMaps[dim] = &aggregationPlan->getAggregationMaps()->at(dim);
		newAlg &= parentMaps[dim]->isNewAlg();
	}
	parentsBegin = new const IdentifierType*[dimCount];
	parentsEnd = new const IdentifierType*[dimCount];
	currParent = new const IdentifierType*[dimCount];
}

PCellStream AggregationProcessor::getCalculatedValues()
{
	if (storage && aggregationPlan->getDefaultValue() == 0) {
		if (aggregationPlan->getMaxCount() == UNLIMITED_UNSORTED_PLAN || aggregationPlan->getMaxCount() == UNLIMITED_SORTED_PLAN) {
			return dynamic_cast<ICellMapStream *>(storage.get())->getValues();
		}
	}
	return PCellStream();
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
		PCellStream sourceDataSP = engine->createProcessor(*source, false);
		CellValueStream *sourceData = sourceDataSP.get();

		StorageCpu::Processor *cpuProc = dynamic_cast<StorageCpu::Processor *>(sourceData);
		if (newAlg && hashStorage && cpuProc && aggregationPlan->getAggregationType() == AggregationPlanNode::SUM) {
			cpuProc->aggregate(hashStorage, parentMaps);
		} else {
			while (sourceData->next()) {
				// process the value
				aggregateCell(sourceData->getKey(), sourceData->getValue().getNumeric());
			}
		}
	}
	if (resultSize > 1000 && (Logger::isDebug() || Logger::isTrace())) {
		Logger::debug << "Aggregated area of " << resultSize << " cells. " << storage->size() << " aggregations exists." << endl;
	}
	// create iterator
	storageReader = dynamic_cast<ICellMapStream *>(storage.get())->getValues();
}

void AggregationProcessor::initParentKey(const IdentifiersType &key, size_t &multiDimCount, double *fixedWeight)
{
	if (fixedWeight) {
		*fixedWeight = 1;
	}
	// generate all combinations of parents
	multiDimCount = 0;
	vector<const AggregationMap::ParentMapType *>::iterator lastParent = lastParents.begin();
	IdentifiersType::iterator lastElemId = lastKey.begin();
	IdentifiersType::const_iterator elemId = key.begin();
	for (size_t dim = 0; dim < dimCount; dim++, ++lastParent, ++lastElemId, ++elemId) {
		const AggregationMap::ParentMapType *dimParents = 0;
		IdentifierType lastParentId = NO_IDENTIFIER;

		if (*elemId != *lastElemId) {
			*lastElemId = *elemId;
			dimParents = parentMaps[dim]->getParentsByBase(*elemId, parentsBegin[dim], parentsEnd[dim]);
			if (parentsBegin[dim]) {
				if (parentsEnd[dim] - parentsBegin[dim] == 1) {
					lastParentId = *parentsBegin[dim];
				}
			} else {
				*lastParent = dimParents;
				if (dimParents && dimParents->size() == 1) {
					AggregationMap::ParentMapType::const_iterator pit = dimParents->begin();
					if (pit->second == 1) {
						lastParentId = pit->first;
					}
				}
			}
			lastKeyParent[dim] = lastParentId;
		} else {
			dimParents = *lastParent;
			lastParentId = lastKeyParent[dim];
		}
		if (lastParentId != NO_IDENTIFIER) {
			// exactly one parent with default weight
			parentKey[dim] = lastParentId;
		}
		if (parentsBegin[dim]) {
			parentKey[dim] = *parentsBegin[dim];
			currParent[dim] = parentsBegin[dim];
			if (parentsEnd[dim] - parentsBegin[dim] > 1) {
				multiDims[multiDimCount++] = (IdentifierType)dim;
			}
		} else if (dimParents) {
			currentParent[dim] = dimParents->begin();
			parentKey[dim] = currentParent[dim]->first;
			if (dimParents->size() == 1) {
				if (fixedWeight) {
					*fixedWeight *= currentParent[dim]->second;
				}
			} else {
				endParent[dim] = dimParents->end();
				// add dimension to set of multiparents
				multiDims[multiDimCount++] = (IdentifierType)dim;
			}
		} else {
			parentKey[dim] = *elemId;
		}
	}
}

void AggregationProcessor::nextParentKey(size_t multiDimCount, size_t &changeMultiDim)
{
	changeMultiDim = multiDimCount - 1;
	bool endOfIteration = false;
	while (!endOfIteration && changeMultiDim >= 0 && changeMultiDim < multiDimCount) {
		size_t currentDim = multiDims[changeMultiDim];
		if (parentsBegin[currentDim]) {
			++currParent[currentDim];
			if (currParent[currentDim] == parentsEnd[currentDim]) {
				currParent[currentDim] = parentsBegin[currentDim];
				parentKey[currentDim] = *parentsBegin[currentDim];
				changeMultiDim--;
			} else {
				parentKey[currentDim] = *currParent[currentDim];
				endOfIteration = true;
			}
		} else if (lastParents[currentDim]) {
			++currentParent[currentDim];
			if (currentParent[currentDim] == endParent[currentDim]) {
				currentParent[currentDim] = lastParents[currentDim]->begin();
				parentKey[currentDim] = currentParent[currentDim]->first;
				changeMultiDim--;
			} else {
				parentKey[currentDim] = currentParent[currentDim]->first;
				endOfIteration = true;
			}
		} else {
			changeMultiDim--;
		}
	}
}

AggregationFunctionProcessor::AggregationFunctionProcessor(PEngineBase engine, CPPlanNode node) :
	AggregationProcessor(engine, node)
{
}

const CellValue &AggregationFunctionProcessor::getValue()
{
	if (isDefaultValue()) {
		return ConstantRepeater::getValue();
	} else {
		const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(planNode.get());
		if (apn->getAggregationType() == AggregationPlanNode::AVG) {
			int count;
			bool found = counter->get(vkey, count);
			if (found) {
				currentAvg = currentValue.getNumeric() / count;
				return currentAvg;
			}
		}
		return currentValue;
	}
}

void AggregationFunctionProcessor::aggregate()
{
	if (aggregationPlan->getAggregationType() == AggregationPlanNode::AVG) {
		counter = CreateCellMap<int>(aggregationPlan->getArea()->dimCount());
	}
	AggregationProcessor::aggregate();
}

void AggregationFunctionProcessor::aggregateCell(const IdentifiersType &key, const double value)
{
	size_t multiDimCount;
	initParentKey(key, multiDimCount, 0);

	// for each parent cell
	size_t changeMultiDim;
	do {
		double oldVal, newVal;
		int count;
		bool setValue = false;
		bool setCounter = false;
		const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(planNode.get());

		bool found = storage->get(parentKey, oldVal);
		if (!found) {
			setValue = true;
			switch (apn->getAggregationType()) {
			case AggregationPlanNode::AVG:
				newVal = value;
				setCounter = true;
				count = 1;
				break;
			case AggregationPlanNode::COUNT:
				newVal = 1.0;
				break;
			case AggregationPlanNode::MAX:
			case AggregationPlanNode::MIN:
				newVal = value;
				break;
			default:
				throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid aggregation function");
			}
		} else {
			switch (apn->getAggregationType()) {
			case AggregationPlanNode::AVG:
				setValue = true;
				newVal = oldVal + value;
				setCounter = true;
				if (counter->get(parentKey, count)) {
					count++;
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
			default:
				throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid aggregation function");
			}
		}
		if (setValue) {
			storage->set(parentKey, newVal);
		}
		if (setCounter) {
			counter->set(parentKey, count);
		}
		nextParentKey(multiDimCount, changeMultiDim);
	} while (changeMultiDim >= 0 && changeMultiDim < multiDimCount);
}

}
