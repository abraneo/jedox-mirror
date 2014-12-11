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

#ifndef AGGREGATION_MAP_H
#define AGGREGATION_MAP_H 1

#include <unordered_map>
#include "palo.h"

namespace palo {

class AggregationMap : private map<IdentifierType, const WeightedSet * > {
public:
	typedef map<IdentifierType, double> ParentMapType;
	typedef unordered_map<IdentifierType, ParentMapType > BaseToParentMapType;

	AggregationMap() : minBaseId(0), maxBaseId(0), maxConsId(0), anyMultiMap(true), anyWeights(false), singleParent(NO_IDENTIFIER), newAlg(true) {}

	const WeightedSet *getBasesByParent(IdentifierType parentId) const;
	double getWeightsSum(const Set *restrictedSet) const;
	void setBasesByParent(IdentifierType parentId, const WeightedSet *baseRanges);
	void setMergeMap(const map<IdentifierType, IdentifierType>& mMap) {mergeMap = mMap;}
	void buildBaseToParentMap();
	void buildBaseToParentMap(IdentifierType parent, const WeightedSet *descElems);
	void buildBaseToParentMap(IdentifierType parent, const IdentifiersWeightType *descElems);
	const ParentMapType *getParentsByBase(IdentifierType baseId, const IdentifierType *&parentsBegin, const IdentifierType *&parentsEnd) const;
	bool hasMultiMap() const {return anyMultiMap;}
	bool hasWeights() const {return anyWeights;}
	bool isNewAlg() const {return newAlg;}
	IdentifierType getSingleParent() const {return singleParent;}
	void getBaseParams(IdentifierType &bMinId, IdentifierType &bMaxId, IdentifierType &cMaxId, const IdentifierType *&b2PArray, const double *&b2PWeightsArray) const {bMinId = minBaseId; bMaxId = maxBaseId; cMaxId = maxConsId; b2PArray = &base2ParentVector[0]; b2PWeightsArray = &base2ParentWeights[0];}
	bool canMergeRanges(IdentifierType lHigh, IdentifierType rLow) const;
private:
	static const size_t MAX_BASE_VECTOR_SIZE;
	BaseToParentMapType base2ParentMap;
	IdentifierType minBaseId;
	IdentifierType maxBaseId;
	IdentifierType maxConsId;
	IdentifiersType base2ParentVector;
	bool anyMultiMap;
	bool anyWeights;
	IdentifierType singleParent;
	vector<double> base2ParentWeights;
	bool newAlg;
	map<IdentifierType, IdentifierType> mergeMap;
};

typedef vector<AggregationMap> AggregationMaps;
typedef boost::shared_ptr<AggregationMaps> PAggregationMaps;
typedef boost::shared_ptr<const AggregationMaps> CPAggregationMaps;

}

#endif
