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

#ifndef AGGREGATION_MAP_H
#define AGGREGATION_MAP_H 1

#ifndef __CUDACC__
#include <unordered_map>
#include "Engine/Area.h"
#endif
#include "palo.h"

namespace palo {

struct TargetSequence {
	TargetSequence(uint32_t	startOffset, uint32_t length) : startOffset(startOffset), length (length) {}
	bool operator==(const TargetSequence &o) const {return startOffset == o.startOffset && length == o.length;}
	uint32_t startOffset;
	uint32_t length;
};
struct Source2TargetMapItem {
	Source2TargetMapItem(IdentifierType	sourceRangeBegin, uint32_t distrMapIndex) : sourceRangeBegin(sourceRangeBegin), distrMapIndex(distrMapIndex) {}
	bool operator!=(const Source2TargetMapItem &o) const {return sourceRangeBegin != o.sourceRangeBegin || distrMapIndex != o.distrMapIndex;}
	bool operator==(const Source2TargetMapItem &o) const {return sourceRangeBegin == o.sourceRangeBegin && distrMapIndex == o.distrMapIndex;}
	IdentifierType sourceRangeBegin;
	uint32_t distrMapIndex;
};

#ifndef __CUDACC__
class AggregationMap : private map<IdentifierType, const WeightedSet * > {
public:
	typedef map<IdentifierType, double> TargetMapType;
	typedef map<IdentifierType, TargetMapType > SourceToTargetMapType;
	static bool DistrMapCmp(const Source2TargetMapItem &i, const Source2TargetMapItem &j) {return (i.sourceRangeBegin < j.sourceRangeBegin);}

	AggregationMap() : minBaseId(NO_IDENTIFIER), maxBaseId(0), anyMultiMap(true), anyWeights(false), singleTarget(NO_IDENTIFIER) {}

	const WeightedSet *getBasesByParent(IdentifierType parentId) const;
	double getWeightsSum(const Set *restrictedSet) const;
	void setBasesByParent(IdentifierType parentId, const WeightedSet *baseRanges);
	void setMergeMap(const map<IdentifierType, IdentifierType>& mMap) {mergeMap = mMap;}
	void buildBaseToParentMap();
	void compactSourceToTarget();
	void buildBaseToParentMap(IdentifierType parent, const WeightedSet *descElems);
	void buildBaseToParentMap(IdentifierType parent, const IdentifiersWeightType *descElems);
	void buildBaseToParentMap_AllToOne(CPSet elems, map<IdentifierType, size_t> *factors);
	void buildBaseToParentMap_OneByOne(CPSet elems);
//	const ParentMapType *getParentsByBase(IdentifierType baseId, const IdentifierType *&parentsBegin, const IdentifierType *&parentsEnd) const;
	bool hasMultiMap() const {return anyMultiMap;}
	bool hasWeights() const {return anyWeights;}
	IdentifierType getSingleTarget() const {return singleTarget;}
	bool canMergeRanges(IdentifierType lHigh, IdentifierType rLow) const;
	void buildBaseToParentMap(IdentifierType parent);
#ifdef ENABLE_GPU_SERVER
	void toGpuAggregationMap(CPPathTranslator, uint32_t dimIdx);
#endif
	const IdentifierType* getMinBaseId() const {return &minBaseId;}
	const IdentifierType* getMaxBaseId() const {return &maxBaseId;}

#ifdef ENABLE_GPU_SERVER
	//getters for aggregation gpu job (GetCellsSumNGJob)
	const vector<gpuBinType>* getTargetIdBufferGpu() const {return &targetIdBufferGpu;}
	const vector<double>* getWeightBuffer() const {return &weightBuffer;}
	const vector<TargetSequence>* getDistributionMap() const {return &distributionMap;}
	const vector<Source2TargetMapItem>* getSource2TargetMap() const {return &source2TargetMap;}
	const vector<uint32_t>* getSource2TargetVector() const {return &source2TargetVector;}
	const gpuBinType* getBinBitmask() const {return &binBitmask;}
	const gpuBinType* getBinFilter() const {return &binFilter;}
	const uint32_t* getDimPos() const {return &dimPos;}
#endif

private:
	SourceToTargetMapType base2ParentMap;
	IdentifierType minBaseId;
	IdentifierType maxBaseId;
	bool anyMultiMap;
	bool anyWeights;
	IdentifierType singleTarget;
	map<IdentifierType, IdentifierType> mergeMap;

	// new Representation
	// buffer of target Ids - indexed by distributionMap
	IdentifiersType targetIdBuffer;
	//same as above but in GPU-shifted format
	vector<gpuBinType> targetIdBufferGpu;
	// buffer of weights - could be empty if no weights are used in aggregation map
	// could be shorter then targetIdBuffer - trailing sequence of 1.0s is missing
	vector<double> weightBuffer;
	// array of unique distribution maps
	// contains starting offset to targetIdBuffer/weightBuffer and length of sequence
	vector<TargetSequence> distributionMap;
	// "map" (vector sorted by starting sourceId) of source to target distribution map
	// first is staringId of base range - second is index to distributionMap
	vector<Source2TargetMapItem> source2TargetMap;
	// vector for fast parent distribution map lookup
	vector<uint32_t> source2TargetVector;
	//dimension bit position for gpu aggregation job
	gpuBinType binBitmask; // used to extract dimension from bin
	gpuBinType binFilter; // used to erase dimension from bin
	uint32_t dimPos;

	uint32_t storeDistributionSequence(const IdentifierType startId, const IdentifierType nextStartId, const IdentifiersType &targetIds, const vector<double> &targetWeights);
public:
	class TargetReader {
	public:
		TargetReader(const TargetReader &it)
		: targetBegin(it.targetBegin),targetEnd(it.targetEnd),targetsCount(it.targetsCount),weightsBegin(it.weightsBegin),
		  currentTarget(it.currentTarget), currentWeight(it.currentWeight), selfId(it.selfId) {}
		TargetReader()
		: targetBegin(0), targetEnd(0), targetsCount(0), weightsBegin(0), currentTarget(0), currentWeight(0), selfId(NO_IDENTIFIER) {}
		TargetReader(const IdentifierType *targetBegin, const IdentifierType *targetEnd, const double *weightsBegin, IdentifierType selfId)
		: targetBegin(targetBegin), targetEnd(targetEnd), targetsCount((uint32_t) (targetEnd-targetBegin)), weightsBegin(weightsBegin),
		  currentTarget(targetBegin), currentWeight(weightsBegin ? weightsBegin : 0), selfId(selfId) {}
		IdentifierType operator*() const {return *currentTarget == NO_IDENTIFIER ? selfId : *currentTarget;}
		double getWeight() const {return currentWeight ? *currentWeight : 1.0;}
		uint32_t size() const {return targetsCount;}
		bool end() const {return currentTarget == targetEnd;}
		void reset() {currentTarget = targetBegin; if (currentWeight) currentWeight = weightsBegin;}
		TargetReader &operator++() {++currentTarget; if (currentWeight) ++currentWeight;return *this;}
		TargetReader operator++(int) {TargetReader res(*this); return ++res;}
		TargetReader &operator=(const TargetReader &it) {
			targetBegin = it.targetBegin; targetEnd = it.targetEnd; targetsCount = it.targetsCount;
			weightsBegin = it.weightsBegin; currentTarget = it.currentTarget;  currentWeight = it.currentWeight;
			selfId = it.selfId; return *this;}
		void setSelfId(IdentifierType selfId) {this->selfId = selfId;}
	private:
		const IdentifierType *targetBegin;
		const IdentifierType *targetEnd;
		uint32_t targetsCount;
		const double *weightsBegin;
		const IdentifierType *currentTarget;
		const double *currentWeight;
		IdentifierType selfId;
	};
	TargetReader getTargets(IdentifierType sourceId) const;
	bool operator==(const AggregationMap &o) const;

	class Iterator  : public std::iterator<forward_iterator_tag, IdentifierType, ptrdiff_t, IdentifierType *, IdentifierType &> {
	public:
		Iterator(const Iterator &it);
		Iterator();
		Iterator(const AggregationMap &am, bool end=false);
		IdentifierType operator*() const;
		TargetReader targetReader() const;
		bool operator!=(const Iterator &it) const;
		bool operator==(const Iterator &it) const;
		Iterator &operator++();
		Iterator operator++(int);
		Iterator &operator=(const Iterator &it);
	private:
		const AggregationMap *am;
		vector<Source2TargetMapItem>::const_iterator mit;
		vector<uint32_t>::const_iterator vit;
	};

	Iterator beginSource() const;
	Iterator endSource() const;
	friend class Iterator;
};
#endif
}

#endif
