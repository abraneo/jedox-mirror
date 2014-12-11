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
 * \author Alexander Haberstroh, Jedox AG, Freiburg, Germany
 * 
 *
 */

#include "Engine/AggregationProcessor.h"
#include "Engine/AggregationMap.h"

namespace palo {

typedef map<IdentifierType, const WeightedSet * > AMBase;

const WeightedSet *AggregationMap::getBasesByParent(IdentifierType parentId) const
{
	const WeightedSet * result = NULL;
	AMBase::const_iterator it = map<IdentifierType, const WeightedSet * >::find(parentId);
	if (it != AMBase::end()) {
		result = it->second;
	}
	return result;
}

double AggregationMap::getWeightsSum(const Set *restrictedSet) const
{
	if (empty()) {
		return 1;
	}
	double dimensionWeight = 0.0;
	bool notempty = false;
	for (AMBase::const_iterator it = begin(); it != end(); ++it) {
		if (it->second) {
			notempty = true;
			for (WeightedSet::range_iterator range = it->second->rangeBegin(); range != it->second->rangeEnd(); ++range) {
				if (restrictedSet) {
					// Todo: -jj- optimized intersection size calculation of Set Range and BaseRangesWeightType
					for (IdentifierType elem = range.low(); elem <= range.high(); ++elem) {
						if (restrictedSet->find(elem) != restrictedSet->end()) {
							dimensionWeight += range.weight();
						}
					}
				} else {
					dimensionWeight += range.weight() * (range.high() - range.low() + 1);
				}
			}
		}
	}
	if (!notempty) {
		dimensionWeight = 1;
	}
	return dimensionWeight;
}

void AggregationMap::setBasesByParent(IdentifierType parentId, const WeightedSet *baseRanges)
{
	if (baseRanges) {
		anyWeights |= baseRanges->hasWeights();
	}
//	if (baseRanges) {
		AMBase::operator [](parentId) = baseRanges;
		if (!baseRanges) {
			minBaseId = min(minBaseId, parentId);
			maxBaseId = max(maxBaseId, parentId);
		} else if (baseRanges->size()) {
			minBaseId = min(minBaseId, baseRanges->rangeBegin().low());
			maxBaseId = max(maxBaseId, (--baseRanges->rangeEnd()).high());
		}
//	}
	if (!baseRanges) {
		base2ParentMap[parentId][parentId] = 1;
	}
}

bool ISWeightNonDefault (const double w) {
	return w != 1.0;
}

uint32_t AggregationMap::storeDistributionSequence(const IdentifierType startId, const IdentifierType nextStartId, const IdentifiersType &targetIds, const vector<double> &targetWeights)
{
	uint32_t distrMapIndex = uint32_t(-1);
	uint32_t distMapSequenceStart = uint32_t(-1);

	uint32_t newSize = (uint32_t)targetIds.size();
	uint32_t currentIndex = (uint32_t)distributionMap.size()-1;
	// TODO: -jj- duplicities search in AggregationMap disabled because of performance problems
	for (vector<TargetSequence>::const_reverse_iterator dm = distributionMap.rbegin();  false && dm != distributionMap.rend(); ++dm, currentIndex--) {
		if (newSize == dm->length) {
			// compare content
			if (equal(targetIds.begin(), targetIds.end(), &targetIdBuffer[dm->startOffset])) {
				//uint32_t currentIndex = dm-distributionMap.rbegin();
				bool identicalWeights = false;
				if (targetWeights.empty() && weightBuffer.size() <= dm->startOffset) {
					// none of weights buffers exist - null == null
					identicalWeights = true;
				}
				if (weightBuffer.size()) {
					double *findEndIt = &weightBuffer[0]+min(dm->startOffset+dm->length, (uint32_t)weightBuffer.size());
					if (!identicalWeights && targetWeights.empty() && find_if(&weightBuffer[dm->startOffset], findEndIt, ISWeightNonDefault) == findEndIt) {
						// check if weightBuffer contains only 1.0 - null == default_weights
						identicalWeights = true;
					}
				}
				if (!identicalWeights && targetWeights.size() && weightBuffer.size() >= dm->startOffset+targetWeights.size() && equal(targetWeights.begin(), targetWeights.end(), &weightBuffer[dm->startOffset])) {
					// both weight buffers exist - weights == weights
					identicalWeights = true;
				}
				if (identicalWeights) {
					distrMapIndex = currentIndex;
//					Logger::info << "duplicity saves: " << newSize*sizeof(targetIds[0]) << " B" << endl;
					break;
				}
			}
		}
	}

	//Logger::debug << distrMapIndex << " " << distributionMap.size() << endl;
	if (distrMapIndex == uint32_t(-1)) {
		// copy sequence to buffers
		distMapSequenceStart = (uint32_t)targetIdBuffer.size();

		if (targetWeights.size()) {
			if (weightBuffer.size() != targetIdBuffer.size()) {
				weightBuffer.resize(targetIdBuffer.size(), 1.0);
			}
			copy(targetWeights.begin(),targetWeights.end(), back_insert_iterator<vector<double> >(weightBuffer));
		}
		copy (targetIds.begin(),targetIds.end(), back_insert_iterator<IdentifiersType>(targetIdBuffer));
		// write sequence start to distributionMap
		distrMapIndex = (uint32_t)(distributionMap.size());
		distributionMap.push_back(TargetSequence(distMapSequenceStart, (uint32_t)targetIds.size()));
	}
	if (source2TargetVector.empty()) {
		// update "map"
		if (source2TargetMap.empty() || source2TargetMap.back().distrMapIndex != distrMapIndex) {
			source2TargetMap.push_back(Source2TargetMapItem(startId, distrMapIndex));
			if (source2TargetMap.size()*sizeof(source2TargetMap[0]) > (maxBaseId-minBaseId+1)*sizeof(source2TargetVector[0])) {
				// convert "map" to vector
				source2TargetVector.resize(maxBaseId-minBaseId+1);
				vector<Source2TargetMapItem>::const_iterator mapIt = source2TargetMap.begin();
				size_t seqBegin = mapIt->sourceRangeBegin-minBaseId;
				do {
					size_t seqEnd;
					uint32_t dmIndex = mapIt->distrMapIndex;
					if (++mapIt == source2TargetMap.end()) {
						seqEnd = maxBaseId-minBaseId+1;
					} else {
						seqEnd = mapIt->sourceRangeBegin-minBaseId;
					}
					fill(&source2TargetVector[seqBegin], &source2TargetVector[0]+seqEnd, dmIndex);
					seqBegin = seqEnd;
				} while (mapIt != source2TargetMap.end());
				source2TargetMap.clear();
			}
		}
	} else {
		// update vector
		size_t fillEnd = min(maxBaseId+1, nextStartId);
		fill(&source2TargetVector[startId-minBaseId], &source2TargetVector[0]+fillEnd-minBaseId, distrMapIndex);
	}
	return distrMapIndex;
}

AggregationMap::TargetReader AggregationMap::getTargets(IdentifierType sourceId) const
{
	uint32_t distrMapIndex;
//	if (sourceId < minBaseId || sourceId > maxBaseId) {
//		// sourceId out of range
//		throw ErrorException(ErrorException::ERROR_INTERNAL, "AggregationMap::getTargets(): sourceId out of range");
//	}
	if (source2TargetVector.empty()) {
		// find in base2ParentMapNG
		vector<Source2TargetMapItem>::const_iterator mit = std::upper_bound(source2TargetMap.begin(), source2TargetMap.end(), Source2TargetMapItem(sourceId, 0), DistrMapCmp);
		if (mit != source2TargetMap.begin()) {
			--mit;
		}
		distrMapIndex = mit->distrMapIndex;
	} else {
		if (sourceId < minBaseId || sourceId > maxBaseId) {
			// sourceId out of range
			throw ErrorException(ErrorException::ERROR_INTERNAL, "AggregationMap::getTargets(): sourceId out of range");
		}
		distrMapIndex = source2TargetVector[sourceId-minBaseId];
	}
	const TargetSequence &distrMap = distributionMap[distrMapIndex];
	const double *weights = weightBuffer.size() <= distrMap.startOffset ? 0 : &weightBuffer[distrMap.startOffset];
	AggregationMap::TargetReader reader(&targetIdBuffer[distrMap.startOffset], &targetIdBuffer[0]+distrMap.startOffset+distrMap.length, weights, sourceId);
	return reader;
}

bool AggregationMap::operator==(const AggregationMap &o) const
{
	return targetIdBuffer == o.targetIdBuffer && weightBuffer == o.weightBuffer && distributionMap == o.distributionMap &&
			source2TargetMap == o.source2TargetMap && source2TargetVector == o.source2TargetVector;
}

#ifndef undefined
// key: first child in current range
// data: range iterator, end range iterator, target element
struct MapTuple {
	MapTuple(WeightedSet::range_iterator crange, WeightedSet::range_iterator erange, IdentifierType target) : crange(crange), erange(erange), target(target) {}
	WeightedSet::range_iterator crange;
	WeightedSet::range_iterator erange;
	IdentifierType target;
};

void AggregationMap::buildBaseToParentMap()
{
	if (size() == 1) {
		singleTarget = begin()->first;
		anyMultiMap = false;
	}

	targetIdBuffer.clear();
	weightBuffer.clear();
	distributionMap.clear();
	source2TargetMap.clear();
	source2TargetVector.clear();

	typedef multimap<IdentifierType, MapTuple* > TmpParents;

	vector<MapTuple> mapTuples;
	TmpParents parents;
	WeightedSet baseTargets;

//	Logger::debug << "#1 " << AMBase::size() << endl;
	// init parents and generate all "only self" distributions
	for (AMBase::const_iterator parent = AMBase::begin(); parent != AMBase::end(); ++parent) {
		if (!parent->second) {
			// self only
			baseTargets.pushSorted(parent->first, 1.0);
		} else {
			mapTuples.push_back(MapTuple(parent->second->rangeBegin(), parent->second->rangeEnd(), parent->first));
		}
	}
	if (baseTargets.size()) {
		mapTuples.push_back(MapTuple(baseTargets.rangeBegin(), baseTargets.rangeEnd(), NO_IDENTIFIER));
	}
//	Logger::debug << "#2 " << " " << mapTuples.size() << " " << baseTargets.rangesCount() << endl;

	for (vector<MapTuple>::iterator tvit = mapTuples.begin(); tvit != mapTuples.end(); ++tvit) {
		parents.insert(make_pair(tvit->crange.low(), &*tvit));
	}

//	Logger::debug << "#3 " << " " << parents.size() << endl;

	IdentifierType startId = minBaseId;

	// while all parents aren't fully processed
	while (startId != NO_IDENTIFIER) {
		IdentifierType firstEnd = NO_IDENTIFIER;
		IdentifierType lastEnd = 0;
		IdentifierType firstStart = NO_IDENTIFIER;
		IdentifierType nextStartId = NO_IDENTIFIER;
		IdentifiersType targetIds;
		vector<TmpParents::iterator> currentParentIts;
		vector<double> targetWeights;

		TmpParents::iterator tpend = parents.end();

		// find active targets
		TmpParents::iterator tp = parents.begin();
		for (; tp != tpend && tp->first <= startId; ++tp) {
			MapTuple *tuple = tp->second;
			const WeightedSet::range_iterator &currentSourceRange = tuple->crange;

			// current target uses current source
			if (firstEnd > currentSourceRange.high()+1) {
				firstEnd = currentSourceRange.high()+1;
			}
			if (lastEnd < currentSourceRange.high()+1) {
				lastEnd = currentSourceRange.high()+1;
			}
			double weight = currentSourceRange.weight();
			if (targetWeights.size() || weight != 1.0) {
				if (targetWeights.size() != targetIds.size()) {
					targetWeights.resize(targetIds.size(), 1.0); // add missing weights
				}
				targetWeights.push_back(weight);
			}
			targetIds.push_back(tuple->target);
			currentParentIts.push_back(tp);
		}
		if (tp != tpend) {
			firstStart = tp->first;
		}

		// next start will be ...
		if (targetIds.size() > 1) {
			if (firstEnd != lastEnd) {
				nextStartId = min(firstEnd, firstStart);
			} else {
				// find start of new range
				for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
					WeightedSet::range_iterator wsit = (*pmapit)->second->crange;
					++wsit;
					if (wsit != (*pmapit)->second->erange) {
						if (firstStart > wsit.low()) {
							firstStart = wsit.low();
						}
					}
				}
				nextStartId = firstStart;
			}
		} else {
			double currentWeight = targetWeights.empty() ? 1.0 : targetWeights[0];
			// find start of new range with different weight
			for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
				WeightedSet::range_iterator wsit = (*pmapit)->second->crange;
				while (++wsit != (*pmapit)->second->erange) {
					if (firstStart > wsit.low()) {
						if (wsit.weight() != currentWeight) {
							firstStart = wsit.low();
							break;
						}
					} else {
						break;
					}
				}
			}
			nextStartId = firstStart;
		}

		// store targetIds sequence and update map/vector
		storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);

		// remove finished parents
		for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
			MapTuple *tuple = (*pmapit)->second;
			WeightedSet::range_iterator &wsit = tuple->crange;
			WeightedSet::range_iterator wsendit = tuple->erange;
			bool erase = false;
			bool insert = true;
			while (wsit.high() < nextStartId) {
				++wsit; // next range
				erase = true;
				if (wsit == wsendit) {
					insert = false;
					break;
				}
			}
			if (erase) {
				parents.erase(*pmapit);
				if (insert) {
					parents.insert(make_pair(wsit.low(), tuple));
				}
			}
		}
//		Logger::debug << startId << "-" << nextStartId << " " << targetIds.size() << " targets" << endl;
		startId = nextStartId;
	}

	if (parents.size()) {
		Logger::error << "AggregationMap::buildBaseToParentMapNG() not finished correctly!" << endl;
	}
	if (Logger::isTrace()) {
		size_t amSize = targetIdBuffer.size() * sizeof(targetIdBuffer[0]) + weightBuffer.size() * sizeof(weightBuffer[0]) + distributionMap.size() * sizeof(distributionMap[0]) + source2TargetMap.size() * sizeof(source2TargetMap[0]) + source2TargetVector.size() * sizeof(source2TargetVector[0]);
		Logger::trace << "AggregationMap size: " << amSize << " targetSize: " << AMBase::size() << endl;
	}
}

#else
void AggregationMap::buildBaseToParentMap()
{
	if (size() == 1) {
		singleTarget = begin()->first;
		anyMultiMap = false;
	}

	targetIdBuffer.clear();
	weightBuffer.clear();
	distributionMap.clear();
	source2TargetMap.clear();
	source2TargetVector.clear();

	typedef map<IdentifierType, pair<WeightedSet::range_iterator,WeightedSet::range_iterator> > TmpParents;
	TmpParents parents;
	WeightedSet baseTargets;

	// init parents and generate all "only self" distributions
	for (AMBase::const_iterator parent = AMBase::begin(); parent != AMBase::end(); ++parent) {
		if (!parent->second) {
			// self only
			baseTargets.pushSorted(parent->first, 1.0);
		} else {
			parents.insert(parents.end(), make_pair(parent->first, make_pair(parent->second->rangeBegin(), parent->second->rangeEnd())));
		}
	}
	if (baseTargets.size()) {
		parents.insert(parents.end(), make_pair(NO_IDENTIFIER, make_pair(baseTargets.rangeBegin(), baseTargets.rangeEnd())));
	}
	IdentifierType startId = minBaseId;

	// while all parents aren't fully processed
	while (startId != NO_IDENTIFIER) {
		IdentifierType firstEnd = NO_IDENTIFIER;
		IdentifierType lastEnd = 0;
		IdentifierType firstStart = NO_IDENTIFIER;
		IdentifierType nextStartId = NO_IDENTIFIER;
		IdentifiersType targetIds;
		vector<TmpParents::iterator> currentParentIts;
		vector<double> targetWeights;

		TmpParents::iterator tpend = parents.end();
		// find first interval
		for (TmpParents::iterator tp = parents.begin(); tp != tpend; ++tp) {
			const WeightedSet::range_iterator &currentSourceRange = tp->second.first;
			IdentifierType currLow = currentSourceRange.low();

			if (currLow <= startId) {
				// current target uses current source
				if (firstEnd > currentSourceRange.high()+1) {
					firstEnd = currentSourceRange.high()+1;
				}
				if (lastEnd < currentSourceRange.high()+1) {
					lastEnd = currentSourceRange.high()+1;
				}
				double weight = currentSourceRange.weight();
				if (targetWeights.size() || weight != 1.0) {
					if (targetWeights.size() != targetIds.size()) {
						targetWeights.resize(targetIds.size(), 1.0); // add missing weights
					}
					targetWeights.push_back(weight);
				}
				targetIds.push_back(tp->first);
				currentParentIts.push_back(tp);
			} else if (firstStart > currLow) {
				firstStart = currLow;
			}
		}
		// next start will be ...
		if (targetIds.size() > 1) {
			if (firstEnd != lastEnd) {
				nextStartId = min(firstEnd, firstStart);
			} else {
				// find start of new range
				for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
					WeightedSet::range_iterator wsit = (*pmapit)->second.first;
					++wsit;
					if (wsit != (*pmapit)->second.second) {
						if (firstStart > wsit.low()) {
							firstStart = wsit.low();
						}
					}
				}
				nextStartId = firstStart;
			}
		} else {
			double currentWeight = targetWeights.empty() ? 1.0 : targetWeights[0];
			// find start of new range with different weight
			for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
				WeightedSet::range_iterator wsit = (*pmapit)->second.first;
				while (++wsit != (*pmapit)->second.second) {
					if (firstStart > wsit.low()) {
						if (wsit.weight() != currentWeight) {
							firstStart = wsit.low();
							break;
						}
					} else {
						break;
					}
				}
			}
			nextStartId = firstStart;
		}

		// store targetIds sequence and update map/vector
		storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);

		// remove finished parents
		for (vector<TmpParents::iterator>::iterator pmapit = currentParentIts.begin(); pmapit != currentParentIts.end(); ++pmapit) {
			while ((*pmapit)->second.first.high() < nextStartId) {
				++((*pmapit)->second.first); // next range
				if ((*pmapit)->second.first == (*pmapit)->second.second) {
					// remove this parent - completely processed
					parents.erase(*pmapit);
					break;
				}
			}
		}
//		Logger::debug << startId << "-" << nextStartId << " " << targetIds.size() << " targets" << endl;
		startId = nextStartId;
	}

	if (parents.size()) {
		Logger::error << "AggregationMap::buildBaseToParentMapNG() not finished correctly!" << endl;
	}
	if (Logger::isDebug()) {
		size_t amSize = targetIdBuffer.size() * sizeof(targetIdBuffer[0]) + weightBuffer.size() * sizeof(weightBuffer[0]) + distributionMap.size() * sizeof(distributionMap[0]) + source2TargetMap.size() * sizeof(source2TargetMap[0]) + source2TargetVector.size() * sizeof(source2TargetVector[0]);
		Logger::debug << "AggregationMap size: " << amSize << " targetSize: " << AMBase::size() << endl;
	}
}
#endif

void AggregationMap::buildBaseToParentMap(IdentifierType parent, const WeightedSet *descElems)
{
	for (WeightedSet::const_iterator it = descElems->begin(); it != descElems->end(); ++it) {
		base2ParentMap[it.first()][parent] = it.second();
		minBaseId = min(minBaseId, it.first());
		maxBaseId = max(maxBaseId, it.first());
	}
}

void AggregationMap::buildBaseToParentMap(IdentifierType parent, const IdentifiersWeightType *descElems)
{
	for (IdentifiersWeightType::const_iterator it = descElems->begin(); it != descElems->end(); ++it) {
		base2ParentMap[it->first][parent] = it->second;
		minBaseId = min(minBaseId, it->first);
		maxBaseId = max(maxBaseId, it->first);
	}
}

void AggregationMap::compactSourceToTarget()
{
	targetIdBuffer.clear();
	weightBuffer.clear();
	distributionMap.clear();
	source2TargetMap.clear();
	source2TargetVector.clear();

	for (SourceToTargetMapType::iterator stit = base2ParentMap.begin(); stit != base2ParentMap.end(); ++stit) {
		IdentifiersType targetIds;
		vector<double> targetWeights;

		for(TargetMapType::const_iterator tmit = stit->second.begin(); tmit != stit->second.end(); ++tmit) {
			targetIds.push_back(tmit->first);
			targetWeights.push_back(tmit->second);
		}
		SourceToTargetMapType::iterator nit = stit;
		++nit;
		IdentifierType nextStartId = nit == base2ParentMap.end() ? stit->first+1 : nit->first;
		storeDistributionSequence(stit->first, nextStartId, targetIds, targetWeights);
	}
}

void AggregationMap::buildBaseToParentMap_AllToOne(CPSet elems, map<IdentifierType, size_t> *factors)
{
	Set::Iterator sit = elems->begin();
	if (sit == elems->end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "empty Set in AggregationMap::buildBaseToParentMap_AllToOne!");
	}

	IdentifierType startId = *sit;
	IdentifierType nextStartId;
	IdentifiersType targetIds;
	targetIds.push_back(*sit);
	vector<double> targetWeights;

	sit = elems->end();
	--sit;
	minBaseId = min(minBaseId, startId);
	maxBaseId = max(maxBaseId, *sit);
	if (factors && !factors->empty()) {
		IdentifierType factorId = NO_IDENTIFIER;
		for (map<IdentifierType, size_t>::const_iterator fit = factors->begin(); fit != factors->end(); ++fit) {
			factorId = fit->first;
			if (startId < factorId) {
				nextStartId = factorId;
				storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);
			}
			startId = factorId;
			nextStartId = factorId + 1;
			targetWeights.push_back((double)fit->second);
			storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);

			startId = nextStartId;
			targetWeights.clear();
		}

		sit = elems->find(factorId);
		++sit;
		if (sit == elems->end()) {
			return;
		} else {
			startId = *sit;
			targetWeights.clear();
		}
	}
	nextStartId = maxBaseId + 1;
	storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);
}

void AggregationMap::buildBaseToParentMap_OneByOne(CPSet elems)
{
	Set::Iterator sit = elems->begin();
	if (sit == elems->end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "empty Set in AggregationMap::buildBaseToParentMap_OneByOne!");
	}
	IdentifierType startId = *sit;

	sit = elems->end();
	--sit;
	IdentifierType nextStartId = *sit + 1;

	IdentifiersType targetIds;
	targetIds.push_back(NO_IDENTIFIER);

	vector<double> targetWeights;
	minBaseId = min(minBaseId, startId);
	maxBaseId = max(maxBaseId, nextStartId-1);
	storeDistributionSequence(startId, nextStartId, targetIds, targetWeights);
}

#ifdef ENABLE_GPU_SERVER

void AggregationMap::toGpuAggregationMap(CPPathTranslator pathTranslator, uint32_t dimIdx)
{

	for(auto targetIdIt = targetIdBuffer.begin(); targetIdIt != targetIdBuffer.end(); ++targetIdIt) {
		if(*targetIdIt != (uint32_t) -1)
			targetIdBufferGpu.push_back(pathTranslator->dimIdToBinDim(*targetIdIt, dimIdx));
		else
			targetIdBufferGpu.push_back(*targetIdIt);
	}
	binBitmask = pathTranslator->getDimensionsBitmask()[dimIdx];
	binFilter = GPU_INVALID_BIN_VALUE ^ binBitmask;
	dimPos = pathTranslator->getDimensionsPos()[dimIdx];
}

#endif

void AggregationMap::buildBaseToParentMap(IdentifierType parent) {
	base2ParentMap[parent][parent] = 1.0;
	minBaseId = min(minBaseId, parent);
	maxBaseId = max(maxBaseId, parent);
}

bool AggregationMap::canMergeRanges(IdentifierType lHigh, IdentifierType rLow) const
{
	map<IdentifierType, IdentifierType>::const_iterator mmIt = mergeMap.find(lHigh);

	if (mmIt != mergeMap.end() && mmIt->second == rLow) {
		return true;
	}

	return false;
}

AggregationMap::Iterator::Iterator(const AggregationMap::Iterator &it)
: am(it.am), mit(it.mit), vit(it.vit)
{
}

AggregationMap::Iterator::Iterator(const AggregationMap &am, bool end)
: am(&am)
{
	if (am.source2TargetMap.empty()) {
		vit = end ? am.source2TargetVector.end() : am.source2TargetVector.begin();
	} else {
		mit = end ? am.source2TargetMap.end() : am.source2TargetMap.begin();
	}
}

IdentifierType AggregationMap::Iterator::operator*() const
{
	if (am->source2TargetMap.empty()) {
		return IdentifierType(vit - am->source2TargetVector.begin() + am->minBaseId);
	} else {
		return mit->sourceRangeBegin;
	}
}

AggregationMap::TargetReader AggregationMap::Iterator::targetReader() const
{
	AggregationMap::TargetReader reader = am->getTargets(operator*());
	reader.setSelfId(NO_IDENTIFIER);
	return reader;
}

bool AggregationMap::Iterator::operator!=(const Iterator &it) const
{
	return am!=it.am || mit!=it.mit || vit!=it.vit;
}

bool AggregationMap::Iterator::operator==(const Iterator &it) const
{
	return am==it.am && mit==it.mit && vit==it.vit;
}

AggregationMap::Iterator &AggregationMap::Iterator::operator++()
{
	if (am->source2TargetMap.empty()) {
		uint32_t current = *vit;
		while (++vit != am->source2TargetVector.end()) {
			if (*vit != current) {
				break;
			}
		}
	} else {
		++mit;
	}
	return *this;
}

AggregationMap::Iterator AggregationMap::Iterator::operator++(int)
{
	AggregationMap::Iterator itCopy(*this);
	return ++itCopy;
}

AggregationMap::Iterator &AggregationMap::Iterator::operator=(const AggregationMap::Iterator &it)
{
	am = it.am;
	mit = it.mit;
	vit = it.vit;
	return *this;
}

AggregationMap::Iterator AggregationMap::beginSource() const
{
	return AggregationMap::Iterator(*this);
}

AggregationMap::Iterator AggregationMap::endSource() const
{
	return AggregationMap::Iterator(*this, true);
}

}
