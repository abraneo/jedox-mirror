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
#include "Engine/Area.h"

namespace palo {

typedef map<IdentifierType, const WeightedSet * > AMBase;

const size_t AggregationMap::MAX_BASE_VECTOR_SIZE = 512*1024;

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
		maxConsId = max(maxConsId, parentId);
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

void AggregationMap::buildBaseToParentMap()
{
	if (size() == 1) {
		singleParent = begin()->first;
		anyMultiMap = false;
	}
	if (maxBaseId-minBaseId <= MAX_BASE_VECTOR_SIZE) {
		base2ParentVector.resize(maxBaseId-minBaseId+1, NO_IDENTIFIER);
		base2ParentWeights.resize(maxBaseId-minBaseId+1);
		IdentifiersType base2ParentVectorCnt(maxBaseId-minBaseId+1);
		for (AMBase::const_iterator parent = AMBase::begin(); !base2ParentVector.empty() && parent != AMBase::end(); ++parent) {
			if (!parent->second) {
				base2ParentVector[parent->first-minBaseId] = parent->first;
				base2ParentVectorCnt[parent->first-minBaseId]++;
				base2ParentWeights[parent->first-minBaseId] = 1.0;
				continue;
			}
			for (WeightedSet::range_iterator rit = parent->second->rangeBegin(); rit != parent->second->rangeEnd(); ++rit) {
				double weight = rit.weight();
				for (IdentifierType baseId = rit.low(); baseId <= rit.high(); baseId++) {
					base2ParentVector[baseId-minBaseId] = parent->first;
					base2ParentVectorCnt[baseId-minBaseId]++;
					base2ParentWeights[baseId-minBaseId] = weight;
				}
			}
		}
		size_t multiSize = 0;
		if (!base2ParentVector.empty()) {
			IdentifiersType::iterator multiit = base2ParentVector.begin();
			for (IdentifiersType::const_iterator cntit = base2ParentVectorCnt.begin(); cntit != base2ParentVectorCnt.end(); ++cntit, ++multiit) {
				if (*cntit > 1) {
					*multiit = IdentifierType(multiSize + maxConsId + 1);
					multiSize += *cntit + 1;
					if (multiSize > MAX_BASE_VECTOR_SIZE) {
						base2ParentVector.clear();
						multiSize = 0;
						newAlg = false;
						break;
					}
				}
			}
			if (!multiSize && !base2ParentVector.empty()) {
				anyMultiMap = false;
			}
			if (multiSize) {
				base2ParentVector.resize(base2ParentVector.size()+multiSize, NO_IDENTIFIER);
				if (!base2ParentWeights.empty()) {
					base2ParentWeights.resize(base2ParentWeights.size()+multiSize);
				}
				for (AMBase::const_iterator parent = AMBase::begin(); parent != AMBase::end(); ++parent) {
					if (!parent->second) {
						if (base2ParentVector[parent->first-minBaseId] > maxConsId) {
							size_t startOffset = base2ParentVector[parent->first-minBaseId]-maxConsId+maxBaseId-minBaseId;
							IdentifierType *multi = &base2ParentVector[startOffset];
							double *weights = base2ParentWeights.empty() ? 0 : &base2ParentWeights[startOffset];
							while (*multi != NO_IDENTIFIER) {
								++multi;
								if (weights) {
									weights++;
								}
							}
							*multi = parent->first;
							if (weights) {
								*weights = 1.0;
							}
						}
						continue;
					}
					for (WeightedSet::range_iterator rit = parent->second->rangeBegin(); rit != parent->second->rangeEnd(); ++rit) {
						for (IdentifierType baseId = rit.low(); baseId <= rit.high(); baseId++) {
							double weight = rit.weight();
							if (base2ParentVector[baseId-minBaseId] > maxConsId) {
								size_t startOffset = base2ParentVector[baseId-minBaseId]-maxConsId+maxBaseId-minBaseId;
								IdentifierType *multi = &base2ParentVector[startOffset];
								double *weights = base2ParentWeights.empty() ? 0 : &base2ParentWeights[startOffset];
								while (*multi != NO_IDENTIFIER) {
									++multi;
									if (weights) {
										weights++;
									}
								}
								*multi = parent->first;
								if (weights) {
									*weights = weight;
								}
							}
						}
					}
				}
			}
		}
	} else {
		newAlg = false;
	}
	if (anyWeights || base2ParentVector.empty()) {
		for (AMBase::const_iterator parent = AMBase::begin(); parent != AMBase::end(); ++parent) {
			const WeightedSet *bases = parent->second;
			if (bases) {
				buildBaseToParentMap(parent->first, bases);
			}
		}
	}
}

void AggregationMap::buildBaseToParentMap(IdentifierType parent, const WeightedSet *descElems)
{
	for (WeightedSet::const_iterator it = descElems->begin(); it != descElems->end(); ++it) {
		base2ParentMap[it.first()][parent] = it.second();
	}
}

void AggregationMap::buildBaseToParentMap(IdentifierType parent, const IdentifiersWeightType *descElems)
{
	for (IdentifiersWeightType::const_iterator it = descElems->begin(); it != descElems->end(); ++it) {
		base2ParentMap[it->first][parent] = it->second;
	}
}

const AggregationMap::ParentMapType *AggregationMap::getParentsByBase(IdentifierType baseId, const IdentifierType *&parentsBegin, const IdentifierType *&parentsEnd) const
{
	const ParentMapType *result = 0;
	parentsBegin = 0;
	parentsEnd = 0;
	if (anyWeights || base2ParentVector.empty()) {
		BaseToParentMapType::const_iterator it = base2ParentMap.find(baseId);
		if (it != base2ParentMap.end()) {
			result = &it->second;
		}
	} else {
		const IdentifierType &singleParentId = base2ParentVector[baseId-minBaseId];
		if (singleParentId <= maxConsId) {
			parentsBegin = &singleParentId;
			parentsEnd = parentsBegin+1;
		} else {
			parentsBegin = &base2ParentVector[singleParentId-maxConsId+maxBaseId-minBaseId];
			parentsEnd = parentsBegin+1;
			while (*parentsEnd != NO_IDENTIFIER) {
				parentsEnd++;
			}
		}
	}
	return result;
}

bool AggregationMap::canMergeRanges(IdentifierType lHigh, IdentifierType rLow) const
{
	map<IdentifierType, IdentifierType>::const_iterator mmIt = mergeMap.find(lHigh);

	if (mmIt != mergeMap.end() && mmIt->second == rLow) {
		return true;
	}

	return false;
}

}
