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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "palo.h"
#include "Engine/Area.h"
#include "Olap/SubCubeList.h"
#include "Olap/Database.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief bin structure
////////////////////////////////////////////////////////////////////////////////

struct Bin {
	vector<uint32_t> binDimensionWidth; // bit width of dimension in this bin
	vector<uint32_t> binDimensionMap;   // bit position of dimension in this bin
	uint32_t dimensionCount;            // number of dimensions in bin
	uint32_t usedBits;                  // used bits in bin

	Bin(): binDimensionWidth(0), binDimensionMap(0), dimensionCount(0), usedBits(0) {
	}

	void addDimension(uint32_t width, uint32_t pos) {
		binDimensionWidth.push_back(width);
		binDimensionMap.push_back(pos);
		dimensionCount++;
		usedBits += width;
	}
};

PathTranslator::PathTranslator(const IdentifiersType &dimSizes, bool dimensionSizesInBit)
{
	IdentifiersType bitDimSizes(dimSizes);
	multimap<IdentifierType, IdentifierType> dimExtensionSpace;
	int totalBits = 0;
	// align dimension sizes to power of 2
	for (uint32_t dimIdx = 0; dimIdx < bitDimSizes.size(); dimIdx++) {
		if(!dimensionSizesInBit){
			// +2 because eg. maxID = 16 -> [0...16] -> 5 Bits AND make sure that GPU_INVALID_BIN_VALUE really IS invalid on first dimension
			bitDimSizes[dimIdx] += 2;
			IdentifierType bits = 0;
			while (bitDimSizes[dimIdx]) {
				bits++;
				bitDimSizes[dimIdx] >>= 1;
			}
			IdentifierType extensionSpace = (1UL << bits) - dimSizes[dimIdx];
			dimExtensionSpace.insert(pair<IdentifierType, IdentifierType>(extensionSpace, dimIdx));
			bitDimSizes[dimIdx] = bits;
			totalBits += bits;
		}
	}
	// TODO -jj- distribute free bits - based on dimExtensionSpace

	vector<Bin> bins(0);

	// create first bin
	Bin b;
	bins.push_back(b);

	uint32_t nextCharge = 0;

	// loop other all dimensions
	for (uint32_t dimIdx = 0; dimIdx < bitDimSizes.size(); dimIdx++) {
		nextCharge = bins.at(bins.size() - 1).usedBits + bitDimSizes.at(dimIdx);
		if (nextCharge <= MaxBinCharge) {
			bins.at(bins.size() - 1).addDimension(bitDimSizes.at(dimIdx), dimIdx);
		} else  { // bin is full
			// create new bin
			Bin nb;
			bins.push_back(nb);
			bins.at(bins.size() - 1).addDimension(bitDimSizes.at(dimIdx), dimIdx);
		}
	}

	binDimensionsCount.clear();
	bitDimSizes.clear();
	dimensionsMap.clear();
	dimensionsPos.clear();
	dimensionsBitmask.clear();

	for (uint32_t binIdx = 0; binIdx < bins.size(); binIdx++) {
		binDimensionsCount.push_back(bins[binIdx].dimensionCount);

		uint32_t pos = MaxBinCharge;

		for (uint32_t binDimIdx = 0; binDimIdx < bins[binIdx].dimensionCount; binDimIdx++) {
			uint32_t width  = bins[binIdx].binDimensionWidth[binDimIdx];
			uint32_t id     = bins[binIdx].binDimensionMap[binDimIdx];

			pos -= width;

			gpuBinType mask = ((1ULL << width) - 1) << pos;

			bitDimSizes.push_back(width);
			dimensionsMap.push_back(id);
			dimensionsPos.push_back(pos);
			dimensionsBitmask.push_back(mask);
		}
	}
}

PathTranslator::PathTranslator(const PathTranslator &pathTranslator) : 
    binDimensionsCount(pathTranslator.binDimensionsCount), dimensionsPos(pathTranslator.dimensionsPos), dimensionsMap(pathTranslator.dimensionsMap), dimensionsBitmask(pathTranslator.dimensionsBitmask)
{
}

gpuBinType PathTranslator::dimIdToBinDim(IdentifierType dimId, uint32_t dimIdx) const
{
	return (gpuBinType)dimId << dimensionsPos[dimIdx];
}

IdentifierType PathTranslator::binDimToDimId(gpuBinType binDim, uint32_t dimIdx) const
{
	return (IdentifierType)((binDim & dimensionsBitmask[dimIdx]) >> dimensionsPos[dimIdx]);
}

void PathTranslator::pathToBinPath(const IdentifiersType& path, GpuBinPath& binPath) const
{
	uint32_t dimIdx = 0;

	binPath.resize(binDimensionsCount.size());

	// encode the path to global index
	for (uint32_t binIdx = 0; binIdx < binDimensionsCount.size(); binIdx++) {
		binPath[binIdx] = 0;

		for (uint32_t i = 0; i < binDimensionsCount[binIdx]; i++, dimIdx++) {
			binPath[binIdx] |= dimIdToBinDim(path[dimensionsMap[dimIdx]], dimIdx);
		}
	}
}

void PathTranslator::binPathToPath(const GpuBinPath& binPath, IdentifiersType& path) const
{
	uint32_t dimIdx = 0;

	path.resize(dimensionsMap.size());

	// encode the path to global index
	for (uint32_t binIdx = 0; binIdx < binDimensionsCount.size(); binIdx++) {
		gpuBinType tmpIndex = binPath[binIdx];

		for (uint32_t i = 0; i < binDimensionsCount[binIdx]; i++, dimIdx++) {
			path[dimensionsMap[dimIdx]] = binDimToDimId(tmpIndex, dimIdx);
		}
	}
}

Set::Iterator::Iterator(const Set::Iterator &it) : it(it.it), pos(it.pos), par(it.par), singleElementId(it.singleElementId), end(it.end)
{
}

Set::Iterator::Iterator() : pos(0), par(0), singleElementId(NO_IDENTIFIER), end(false)
{
}

Set::Iterator::Iterator(const SetType::iterator &it, const Set &s, bool end) : it(it), pos(end ? 0 : it->first), par(&s), singleElementId(NO_IDENTIFIER), end(end)
{
}

Set::Iterator::Iterator(const SetType::iterator &it, IdentifierType pos, const Set &s) : it(it), pos(pos), par(&s), singleElementId(NO_IDENTIFIER), end(false)
{
}

Set::Iterator::Iterator(IdentifierType singleElementId, bool end) : pos(0), par(0), singleElementId(singleElementId), end(end)
{
	if (singleElementId == NO_IDENTIFIER) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid element ID");
	}
}

IdentifierType Set::Iterator::operator*() const
{
	return singleElementId == NO_IDENTIFIER ? pos : singleElementId;
}

bool Set::Iterator::operator!=(const Set::Iterator &it) const
{
	return !(*this == it);
}

bool Set::Iterator::operator==(const Set::Iterator &it) const
{
	if (singleElementId == NO_IDENTIFIER && it.singleElementId == NO_IDENTIFIER) {
		return this->it == it.it && (this->it == par->ranges.end() ? true : pos == it.pos);
	} else if (singleElementId != NO_IDENTIFIER && it.singleElementId != NO_IDENTIFIER) {
		return end == it.end && (end ? true : singleElementId == it.singleElementId);
	} else {
		const SetType::const_iterator &it1 = singleElementId == NO_IDENTIFIER ? this->it : it.it;
		const Set *par1 = singleElementId == NO_IDENTIFIER ? par : it.par;
		IdentifierType pos1 = singleElementId == NO_IDENTIFIER ? pos : it.pos;
		IdentifierType elemId2 = singleElementId == NO_IDENTIFIER ? it.singleElementId : singleElementId;
		bool end2 = singleElementId == NO_IDENTIFIER ? it.end : end;
		if (end2) {
			return it1 == par1->ranges.end();
		} else {
			return it1 == par1->ranges.end() ? false : par1->size() == 1 && pos1 == elemId2;
		}
	}
}

Set::Iterator &Set::Iterator::operator--()
{
	if (singleElementId == NO_IDENTIFIER) {
		if (!end && pos > it->first) {
			pos--;
		} else {
			if (it != par->ranges.begin()) {
				--it;
				pos = it->second;
			}
		}
	} else {
		end = false;
	}
	return *this;
}

Set::Iterator &Set::Iterator::operator++()
{
	if (singleElementId == NO_IDENTIFIER) {
		if (!end && pos < it->second) {
			pos++;
		} else {
			if (it != par->ranges.end()) {
				++it;
				if (it != par->ranges.end()) {
					pos = it->first;
				} else {
					pos = 0;
					end = true;
				}
			}
		}
	} else {
		end = true;
	}
	return *this;
}

Set::Iterator Set::Iterator::operator++(int)
{
	Iterator ret(*this);
	++(*this);
	return ret;
}

Set::Iterator &Set::Iterator::operator=(const Iterator &it)
{
	par = it.par;
	pos = it.pos;
	this->it = it.it;
	singleElementId = it.singleElementId;
	end = it.end;
	return *this;
}

Set::Set() : siz(0)
{
}

Set::Set(bool full)
{
	if (full) {
		ranges.insert(make_pair(0, ALL_IDENTIFIERS));
		siz = (size_t)-1;
	} else {
		siz = 0;
	}
}

Set::Set(const Set &set) : ranges(set.ranges), siz(set.siz)
{
}

bool Set::insert(IdentifierType id)
{
	bool result = false;

	SetType::iterator itp = ranges.lower_bound(make_pair(id, 0));
	if (itp == ranges.end() || itp->first != id) {
		if (itp != ranges.begin()) {
			--itp;
		}
	}
	if (itp == ranges.end()) {
		ranges.insert(make_pair(id, id));
		++siz;
		result = true;
	} else {
		SetType::iterator tmp;
		if (itp->second < id) {
			if (itp->second == id - 1) {
				tmp = itp;
				--tmp;
				tmp = ranges.insert(tmp, make_pair(itp->first, id));
				ranges.erase(itp);
				itp = tmp;
				result = true;
				siz += 1;
			}
			SetType::iterator itn = itp;
			++itn;
			if (itn == ranges.end()) {
				if (!result) {
					ranges.insert(itp, make_pair(id, id));
					++siz;
					result = true;
				}
			} else {
				if (itn->first > id) {
					if (itn->first == id + 1) {
						tmp = ranges.insert(itp, make_pair(id, itn->second));
						ranges.erase(itn);
						itn = tmp;
						if (!result) {
							result = true;
							++siz;
						}
					} else {
						if (!result) {
							ranges.insert(itp, make_pair(id, id));
							++siz;
							result = true;
						}
					}
				}
				if (itp->second == itn->first) {
					ranges.insert(itp, make_pair(itp->first, itn->second));
					ranges.erase(itp);
					ranges.erase(itn);
				}
			}
		} else {
			if (itp->first > id) {
				if (id == itp->first - 1) {
					ranges.insert(make_pair(id, itp->second)).first;
					ranges.erase(itp);
				} else {
					ranges.insert(make_pair(id, id)).first;
				}
				++siz;
				result = true;
			}
		}
	}
	return result;
}

Set::Iterator Set::begin() const
{
	return Iterator(ranges.begin(), *this, ranges.begin()==ranges.end());
}

Set::Iterator Set::end() const
{
	return Iterator(ranges.end(), *this, true);
}

Set::Iterator Set::find(IdentifierType id) const
{
	pair<IdentifierType, IdentifierType> val(id, 0);
	SetType::iterator it = ranges.lower_bound(val);
	if (it != ranges.end() && it->first == id) {
		return Iterator(it, id, *this);
	}
	if (it != ranges.begin()) {
		--it;
		if (it->first <= id && it->second >= id) {
			return Iterator(it, id, *this);
		}
	}
	return end();
}

Set::Iterator Set::lowerBound(IdentifierType id) const
{
	pair<IdentifierType, IdentifierType> val(id, 0);
	SetType::iterator it = ranges.lower_bound(val);
	if (it != ranges.end() && it->first == id) {
		return Iterator(it, id, *this);
	}
	if (it != ranges.begin()) {
		SetType::iterator sit = it;
		--sit;
		if (sit->first <= id && sit->second >= id) {
			return Iterator(sit, id, *this);
		}
	}
	if (it != ranges.end()) {
		return Iterator(it, it->first, *this);
	}
	return end();
}

Set::Iterator Set::erase(const Set::Iterator &it)
{
	siz--;
	if (it.it->first == it.pos) {
		pair<IdentifierType, IdentifierType> val = *it.it;
		SetType::iterator tmp = it.it;
		++tmp;
		ranges.erase(it.it);
		if (val.second != it.pos) {
			val.first++;
			return Iterator(ranges.insert(val).first, val.first, *this);
		} else {
			return Iterator(tmp, *this, tmp==ranges.end());
		}
	} else if (it.it->second == it.pos) {
		pair<IdentifierType, IdentifierType> val = *it.it;
		ranges.erase(it.it);
		val.second--;
		return Iterator(ranges.insert(val).first, val.second, *this);
	} else {
		pair<IdentifierType, IdentifierType> val1 = *it.it;
		pair<IdentifierType, IdentifierType> val2 = *it.it;
		ranges.erase(it.it);
		val1.second = it.pos - 1;
		val2.first = it.pos + 1;
		ranges.insert(val1);
		return Iterator(ranges.insert(val2).first, val2.first, *this);
	}
}

size_t Set::size() const
{
#ifdef x_DEBUG
	size_t tstSize = 0;
	for (std::set<std::pair<IdentifierType, IdentifierType> >::iterator range = ranges.begin(); range != ranges.end(); ++range) {
		tstSize += range->second - range->first + 1;
	}
	if (siz != tstSize) {
		Logger::debug << "Set::size() cached size is invalid! " << siz << " != " << tstSize << endl;
	}
#endif
	return siz;
}

void Set::clear()
{
	ranges.clear();
	siz = 0;
}

Set::range_iterator Set::rangeLowerBound(IdentifierType id) const
{
	SetType::iterator it = ranges.lower_bound(make_pair(id, 0));
	if (it != ranges.end() && it->first == id) {
		return range_iterator(it);
	}
	if (it != ranges.begin()) {
		--it;
	}
	return range_iterator(it);
}

void Set::insertRange(IdentifierType low, IdentifierType high)
{
	siz += high - low + 1;
	SetType::iterator it = ranges.end();
	if (it != ranges.begin()) {
		--it;
#ifdef _DEBUG
		if (low == it->second) {
			Logger::debug << "Strange ranges came to intersection, please, check." << endl;
		}
#endif
	}
	ranges.insert(it, make_pair(low, high));
}

bool Set::empty() const
{
	return siz == 0;
}

bool Set::operator!=(const Set &set) const
{
	return siz != set.siz || ranges != set.ranges;
}

bool Set::operator==(const Set &set) const
{
	return siz == set.siz && ranges == set.ranges;
}

bool Set::intersection_check(const Set *set, PSet *intersection, PSet *complement) const
{
	// TODO -jj- optimize Set::intersection to operate with ranges instead of iterators
	if (complement) {
		(*complement).reset(new Set);
	}
	if (intersection) {
		(*intersection).reset(new Set);
	}
	if (!set) {
		if (intersection) {
			(*intersection).reset(new Set(*this));
		}
		return true;
	}
	Iterator it1 = begin();
	Iterator it2 = set->begin();
	Iterator end1 = end();
	Iterator end2 = set->end();
	bool hasIntersection = false;
	bool hasComplement = false;
	for (;it1 != end1 && it2 != end2;) {
		if (*it1 < *it2) {
			hasComplement = true;
			if (complement) {
				if (!(*complement)) {
					(*complement).reset(new Set());
				}
				(*complement)->insert(*it1);
			}
			++it1;
		} else if (*it1 == *it2) {
			if (intersection) {
				if (!(*intersection)) {
					(*intersection).reset(new Set());
				}
				(*intersection)->insert(*it1);
			}
			++it1;
			++it2;
			hasIntersection = true;
			if (!intersection && !complement) {
				return hasIntersection;
			}
		} else {
			++it2;
		}
	}
	if (it1 != end1) {
		hasComplement = true;
		if (complement) {
			if (!(*complement)) {
				(*complement).reset(new Set());
			}
			while (it1 != end1) {
				(*complement)->insert(*it1);
				hasComplement = true;
				++it1;
			}
		}
	}
	if (!hasComplement && intersection) {
		(*intersection).reset(new Set(*this));
	}
	if (!intersection && complement) {
		(*complement).reset(new Set(*this));
	}
	return hasIntersection;
}

bool Set::intersection(const Set *set, PSet *intersection, PSet *complement) const
{
	if (complement) {
		(*complement).reset(new Set);
	}
	if (!set) {
		if (intersection) {
			(*intersection).reset(new Set(*this));
		}
		return true;
	}
	if (intersection) {
		(*intersection).reset(new Set);
	}
	bool hasIntersection = false;

	Set::range_iterator filterIt = set->rangeBegin();
	Set::range_iterator setIt = rangeBegin();
	Set::range_iterator setEnd = rangeEnd();
	Set::range_iterator filterEnd = set->rangeEnd();

	for (; setIt != setEnd; ++setIt) {
		IdentifierType setLow = setIt.low();
		IdentifierType setHigh = setIt.high();
		if (filterIt == filterEnd) {
			if (!intersection && !complement) {
				return false;
			}
			if (complement) {
				(*complement)->insertRange(setLow, setHigh);
			} else {
				break;
			}
		} else {
			IdentifierType setCurr = setLow;
			while (filterIt != filterEnd) {
				IdentifierType filterLow = filterIt.low();
				IdentifierType filterHigh = filterIt.high();
				if (filterHigh < setLow) {
					++filterIt;
				} else if (filterLow > setHigh) {
					break;
				} else {
					hasIntersection = true;
					if (!intersection && !complement) {
						return true;
					}
					if (complement && setCurr < filterLow) {
						(*complement)->insertRange(setCurr, filterLow - 1);
					}
					setCurr = min(setHigh, filterHigh);
					if (intersection) {
						(*intersection)->insertRange(max(filterLow, setLow), setCurr);
					}
					++setCurr;
					if (filterHigh > setHigh) {
						break;
					} else {
						++filterIt;
					}
				}
			}
			if (complement && setCurr <= setHigh) {
				(*complement)->insertRange(setCurr, setHigh);
			}
		}
	}

//	PSet comp_check, inter_check;
//	bool res_check = intersection_check(set, intersection ? &inter_check : 0, complement ? &comp_check : 0);
//	if (hasIntersection != res_check) {
//		Logger::debug << "Chyba endl";
//	}
//	if (intersection) {
//		if (**intersection != *inter_check) {
//			Logger::debug << "Chyba endl";
//		}
//	}
//	if (complement) {
//		if (**complement != *comp_check) {
//			Logger::debug << "Chyba endl";
//		}
//	}

#ifdef _DEBUG
	if (intersection && *intersection) {
		if (!(*intersection)->validate()) {
			Logger::debug << *this << " X " << *set << " intersection" << endl;
		}
	}
	if (complement && *complement) {
		if (!(*complement)->validate()) {
			Logger::debug << *this << " X " << *set << " complement" << endl;
		}
	}
#endif

	return hasIntersection;
}

CPSet Set::limit(IdentifierType start, IdentifierType end) const
{
	if (size()) {
		if (ranges.begin()->first >= start && (--ranges.end())->second < end) {
			return shared_from_this();
		} else {
			PSet set(new Set());
			for (Set::range_iterator rit = rangeBegin(); rit != rangeEnd(); ++rit) {
				if (rit.high() >= start && rit.low() <= end) {
					set->insertRange(max(rit.low(), start), min(rit.high(), end));
				} else if (rit.low() >= end) {
					break;
				}
			}
			return set;
		}
	} else {
		return shared_from_this();
	}
}

bool Set::validate() const
{
	for (SetType::const_iterator r = ranges.begin(); r != ranges.end(); ++r) {
		if (r != ranges.begin()) {
			SetType::const_iterator prev = r;
			--prev;
			if (r->first == prev->second+1) {
				Logger::debug << "Set::validate(): " << *this << endl;
				return false;
			}
		}
	}
	return true;
}

PSet Set::addAncestors(CPSet set, CPDimension dim)
{
	PSet result(new Set);
	for (Set::Iterator sit = set->begin(); sit != set->end(); ++sit) {
		addAncestor(result, dim, *sit);
	}
	return result;
}

void Set::addAncestor(PSet set, CPDimension dim, IdentifierType elemId)
{
	if (set->find(elemId) == set->end()) {
		set->insert(elemId);
		Element *elem = dim->lookupElement(elemId, false);
		CPParents parents = elem->getParents();
		for (Parents::const_iterator it = parents->begin(); it != parents->end(); ++it) {
			addAncestor(set, dim, *it);
		}
	}
}

bool SetMultimaps::isTrivialMapping() const
{
	bool result = true;
	// here I should check if there is always one to one mapping
	for (SetMultimaps::const_iterator smmit = begin(); result && smmit != end(); ++smmit) {
		if (!*smmit) {
			continue;
		}
		const SetMultimap &smm = **smmit;
		if (smm.size() != 1) {
			result = false;
			break;
		}
	}
	return result;
}

Area::PathIterator::PathIterator(const Area &area, bool end, const vector<ConstElemIter> &path) : path(path), end(end), area(&area), singlePath(false)
{
	if (!end) {
		for (size_t i = 0; i < area.dimCount(); i++) {
			ids.push_back(*path[i]);
		}
	}
}

Area::PathIterator::PathIterator(const Area &area, bool end, const IdentifiersType &path) : ids(path), end(end), area(&area), singlePath(true)
{
}

Area::PathIterator::PathIterator() : end(true), area(0), singlePath(false)
{
}

Area::PathIterator::PathIterator(const Area::PathIterator &it) : path(it.path), ids(it.ids), end(it.end), area(it.area), singlePath(it.singlePath)
{
}

Area::PathIterator &Area::PathIterator::operator++()
{
	if (!end) {
		if (singlePath) {
			end = true;
		} else {
			for (size_t j = path.size(); j > 0; j--) {
				size_t i = j - 1;
				++(path[i]);
				if (path[i] != area->elemEnd(i)) {
					ids[i] = *path[i];
					break;
				} else {
					if (!i) {
						end = true;
					} else {
						path[i] = area->elemBegin(i);
						ids[i] = *path[i];
					}
				}
			}
		}
	}
	return *this;
}

const IdentifiersType &Area::PathIterator::operator*() const
{
	return ids;
}

bool Area::PathIterator::operator==(const PathIterator &it) const
{
	if (area != it.area) {
		return false;
	}
	if (end != it.end) {
		return false;
	} else {
		if (!end) {
			if (singlePath && it.singlePath) {
				return ids == it.ids;
			} else if (!singlePath && !it.singlePath) {
				for (size_t i = 0; i < path.size(); i++) {
					if (path[i] != it.path[i]) {
						return false;
					}
				}
			} else {
				const IdentifiersType &p1 = singlePath ? ids : it.ids;
				const vector<ConstElemIter> &p2 = singlePath ? it.path : path;
				for (size_t i = 0; i < p1.size(); i++) {
					if (p1[i] != *p2[i]) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool Area::PathIterator::operator!=(const PathIterator &it) const
{
	return !(it == *this);
}

bool Area::PathIterator::operator!=(const IdentifiersType &key) const
{
	return ids != key;
}

Area::PathIterator Area::PathIterator::operator+(double count) const
{
	PathIterator res(area->pathBegin());
	if (!res.end) {
		if (res.singlePath) {
			if (count > 0) {
				res.end = true;
			}
		} else {
			count += getPosition();
			if (count < res.area->getSize()) {
				for (size_t i = 0; i < res.path.size(); i++) {
					size_t steps = size_t(count / res.area->stepSizes[i]);
					for (size_t j = 0; j < steps; j++) {
						++(res.path[i]);
					}
					count -= steps * res.area->stepSizes[i];
					res.ids[i] = *res.path[i];
				}
			} else {
				res.end = true;
			}
		}
	}
	return res;
}

double Area::PathIterator::operator-(const PathIterator &it) const
{
	return getPosition() - it.getPosition();
}

string Area::PathIterator::toString() const
{
	StringBuffer sb;

	for (size_t i = 0; i < ids.size(); i++) {
		if (i > 0)
			sb.appendChar(',');
		sb.appendInteger(ids.at(i));
	}

	string result = sb.c_str();
	return result;
}

IdentifierType Area::PathIterator::at(size_t index) const
{
	if (index >= 0 && index < ids.size()) {
		return ids[index];
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "index out of range");
	}
}

double Area::PathIterator::getPosition() const
{
	double res = 0;
	if (end) {
		res = area->getSize();
	} else {
		if (!singlePath) {
			for (size_t i = 0; i < path.size(); i++) {
				res += distance(area->elemBegin(i), path[i]) * area->stepSizes[i];
			}
		}
	}
	return res;
}

Area::Area(size_t dimCount) : area(dimCount), areaSize(0), stepSizes(dimCount, 0)
{
}

Area::Area(const Area &area) : area(area.area), areaSize(area.areaSize), stepSizes(area.stepSizes), pathTranslator(area.pathTranslator)
{
}

Area::Area(const IdentifiersType &path, bool useSet) : area(useSet ? path.size() : 0), areaSize(1), stepSizes(path.size(), 0)
{
	if (useSet) {
		size_t size = path.size();
		for (size_t i = 0; i < size; i++) {
			PSet s(new Set);
			s->insert(path[i]);
			insert((IdentifierType)i, s, false);
		}
	} else {
		vpath = path;
	}
//	calcSize();
}

Area::Area(const vector<IdentifiersType> &idsArea) : area(idsArea.size()), areaSize(0), stepSizes(idsArea.size(), 0)
{
	size_t size = idsArea.size();
	for (size_t i = 0; i < size; i++) {
		PSet s(new Set());
		if (idsArea[i].size() == 1 && idsArea[i][0] == ALL_IDENTIFIERS) {
			s.reset(new Set(true));
		} else {
			for (size_t j = 0; j < idsArea[i].size(); j++) {
				s->insert(idsArea[i][j]);
			}
		}
		insert((IdentifierType)i, s, false);
	}
	calcSize();
}

Area::ConstElemIter Area::elemBegin(size_t dimOrdinal) const
{
	if (vpath.size()) {
		return Set::Iterator(vpath[dimOrdinal], false);
	} else {
		return area[dimOrdinal]->begin();
	}
}

Area::ConstElemIter Area::elemEnd(size_t dimOrdinal) const
{
	if (vpath.size()) {
		return Set::Iterator(vpath[dimOrdinal], true);
	} else {
		return area[dimOrdinal]->end();
	}
}

Area::PathIterator Area::pathBegin() const
{
	if (vpath.size()) {
		return PathIterator(*this, false, vpath);
	} else {
		vector<ConstElemIter> path;
		if (areaSize) {
			for (size_t i = 0; i < area.size(); i++) {
				path.push_back((area[i])->begin());
			}
		}
		return PathIterator(*this, !areaSize, path);
	}
}

Area::PathIterator Area::pathEnd() const
{
	vector<ConstElemIter> path;
	return PathIterator(*this, true, path);
}

Area::PathIterator Area::getIterator(CPArea path) const
{
	if (path->dimCount() != area.size()) {
		return pathEnd();
	}
	if (vpath.size()) {
		for (size_t i = 0; i < area.size(); i++) {
			if (vpath[i] != *path->elemBegin(i)) {
				return pathEnd();
			}
		}
		return pathBegin();
	} else {
		vector<ConstElemIter> p;
		for (size_t i = 0; i < area.size(); i++) {
			ConstElemIter it = area[i]->find(*path->elemBegin(i));
			if (it == area[i]->end()) {
				return pathEnd();
			}
			p.push_back(it);
		}
		return PathIterator(*this, false, p);
	}
}

void Area::insert(size_t dimOrdinal, CPSet elems, bool calc)
{
	if (dimOrdinal >= area.size()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Area::insert invalid dimOrdinal");
	}
	area[dimOrdinal] = elems;
	if (calc) {
		calcSize();
	}
}

Area::ConstElemIter Area::find(size_t dimOrdinal, IdentifierType elemId) const
{
	if (vpath.size()) {
		return Set::Iterator(vpath[dimOrdinal], vpath[dimOrdinal] != elemId);
	} else {
		return area[dimOrdinal]->find(elemId);
	}
}

Area::PathIterator Area::find(const IdentifiersType &path) const
{
	if (path.size() != area.size()) {
		return pathEnd();
	}
	if (vpath.size()) {
		return vpath == path ? pathBegin() : pathEnd();
	} else {
		vector<ConstElemIter> p;
		for (size_t i = 0; i < area.size(); i++) {
			ConstElemIter it = area[i]->find(path[i]);
			if (it == area[i]->end()) {
				return pathEnd();
			}
			p.push_back(it);
		}
		return PathIterator(*this, false, p);
	}
}

Area::PathIterator Area::lowerBound(const IdentifiersType &path) const
{
	if (path.size() != area.size()) {
		return pathEnd();
	}
	if (vpath.size()) {
		return vpath == path ? pathBegin() : pathEnd();
	} else {
		vector<ConstElemIter> p;
		for (size_t i = 0; i < area.size(); i++) {
			ConstElemIter it = area[i]->lowerBound(path[i]);
			if (it == area[i]->end()) {
				return pathEnd();
			}
			p.push_back(it);
		}
		return PathIterator(*this, false, p);
	}
}

size_t Area::dimCount() const
{
	return vpath.size() ? vpath.size() : area.size();
}

size_t Area::elemCount(size_t dimOrdinal) const
{
	if (vpath.size()) {
		return 1;
	} else {
		return (dimOrdinal < area.size() && area[dimOrdinal]) ? area[dimOrdinal]->size() : 0;
	}
}

void CubeArea::split(const IdentifiersType &start, const IdentifiersType &stop, list<PCubeArea> &areaList, size_t dim) const
{
	if (dim == dimCount()) {
		// last dim - add new PArea
		PCubeArea a = PCubeArea(new CubeArea(db, cube, dimCount()));
		for (size_t i = 0; i < dimCount(); i++) {
			a->insert((IdentifierType)i, getDim(i)->limit(start[i], stop[i]));
		}
		areaList.push_back(a);
	} else if (start[dim] == stop[dim]) {
		split(start, stop, areaList, dim + 1);
	} else {
		IdentifiersType middleStart = start;
		IdentifiersType middleStop = stop;

		size_t firstCut = dim + 1;
		for (; firstCut < dimCount(); firstCut++) {
			if (start[firstCut] != *getDim(firstCut)->begin()) {
				break;
			}
		}
		if (firstCut != dimCount()) {
			// cut beginning
			++middleStart[dim];
			IdentifiersType path = start;
			for (size_t i = dim + 1; i < dimCount(); i++) {
				path[i] = NO_IDENTIFIER;
				middleStart[i] = *getDim(i)->begin();
			}
			split(start, path, areaList, dim + 1);
		}

		size_t lastCut = dim + 1;
		for (; lastCut < dimCount(); lastCut++) {
			if (stop[lastCut] != NO_IDENTIFIER) {
				break;
			}
		}
		IdentifiersType path;
		if (lastCut != dimCount()) {
			// cut end
			--middleStop[dim];
			path = stop;
			for (size_t i = dim + 1; i < dimCount(); i++) {
				path[i] = *getDim(i)->begin();
				middleStop[i] = NO_IDENTIFIER;
			}
		}

		if (middleStart[dim] <= middleStop[dim]) {
			// cut middle
			split(middleStart, middleStop, areaList, dim + 1);
		}
		if (lastCut != dimCount()) {
			split(path, stop, areaList, dim + 1);
		}
	}
}

list<PCubeArea> CubeArea::split(PathIterator begin, size_t cellCount, bool &toTheEnd) const
{
	toTheEnd = false;
	IdentifiersType start = *begin;
	list<PCubeArea> areaList;
	if (begin != pathEnd()) {
		IdentifiersType stop(dimCount(), NO_IDENTIFIER);
		if (cellCount == 0) {
			toTheEnd = true;
		} else {
			PathIterator end = begin + (double)cellCount;
			if (end == this->pathEnd()) {
				toTheEnd = true;
			} else {
				stop = *end;
			}
		}
		split(start, stop, areaList, 0);
	} else {
		toTheEnd = true;
	}
	return areaList;
}

void CubeArea::splitbyTypes(SubCubeList &stringAreas, SubCubeList &numericAreas, SubCubeList &consolidatedAreas) const
{
	if (!cube) {
		return;
	}
	if (isSingleCell()) {
		CubeArea cubeAreaCopy(getDatabase(), getCube(), vpath, true);
		return cubeAreaCopy.splitbyTypes(stringAreas, numericAreas, consolidatedAreas);
	}

	bool supportsAggregations = cube->supportsAggregations();

	vector<PSet> stringSets(dimCount());
	vector<PSet> numericSets(dimCount());
	vector<PSet> consolidatedOrNumericSets(dimCount());
	vector<PSet> consolidatedSets(dimCount());
	size_t stringSetsCount = 0;
	size_t consolidatedSetsCount = 0;
	size_t numericSetsCount = 0;
	size_t consolidatedOrNumericSetsCount = 0;

	const IdentifiersType *dimensionIds = cube->getDimensions();
	// split each subset into string and numeric part
	for (size_t i = 0; i != dimCount(); i++) {
		// get dimension
		CPDimension dimension = db->lookupDimension(dimensionIds->at(i), false);
		// TODO -jj- optimization if dimension contains only elements of one type

		if (dimension->getDimensionType() == Dimension::VIRTUAL) {
			PSet set = boost::const_pointer_cast<Set>(getDim(i));
			consolidatedOrNumericSets[i] = set;
			consolidatedOrNumericSetsCount++;
			numericSets[i] = set;
			numericSetsCount++;
			continue;
		}
		for (Set::Iterator sit = getDim(i)->begin(); sit != getDim(i)->end(); ++sit) {
			Element *element = dimension->lookupElement(*sit, false);
			if (!element) {
				Logger::error << "CubeArea::splitbyTypes element id: " << *sit << " not found in dimension: " << dimension->getName() << endl;
				continue;
			}
			Element::Type et = element->getElementType();
			if (et == Element::STRING || element->isStringConsolidation()) {
				if (!stringSets[i]) {
					stringSets[i].reset(new Set());
					stringSetsCount++;
				}
				stringSets[i]->insert(*sit);
			} else {
				if (!consolidatedOrNumericSets[i]) {
					consolidatedOrNumericSets[i].reset(new Set());
					consolidatedOrNumericSetsCount++;
				}
				consolidatedOrNumericSets[i]->insert(*sit);

				if (et == Element::NUMERIC || !supportsAggregations) {
					if (!numericSets[i]) {
						numericSets[i].reset(new Set());
						numericSetsCount++;
					}
					numericSets[i]->insert(*sit);
				} else {
					if (!consolidatedSets[i]) {
						consolidatedSets[i].reset(new Set());
						consolidatedSetsCount++;
					}
					consolidatedSets[i]->insert(*sit);
				}
			}
		}
	}

	if (stringSetsCount) {
		if (consolidatedOrNumericSetsCount < dimCount()) {
			// pure string
			stringAreas.push_back(shared_from_this());
		} else for (size_t i = 0; i != dimCount(); i++) {
				// combine all sets
				if (stringSets[i]) {
					bool validArea = true;
					// create CubeArea
					PCubeArea newArea(new CubeArea(db, cube, dimCount()));
					for (size_t j = 0; j != dimCount(); j++) {
						// fill newArea sets
						if (j < i) {
							if (consolidatedOrNumericSets[j]) {
								newArea->insert(j, consolidatedOrNumericSets[j]);
							} else {
								validArea = false;
								break;
							}
						} else if (j == i) {
							newArea->insert(j, stringSets[j]);
						} else {
							newArea->insert(j, getDim(j));
						}
					}
					if (validArea) {
						stringAreas.push_back(newArea);
					}
				}
			}
	}
	if (consolidatedSetsCount) {
		if (numericSetsCount < dimCount()) {
			if (consolidatedOrNumericSetsCount == dimCount()) {
				// pure consolidated except strings
				PCubeArea newArea(new CubeArea(db, cube, dimCount()));
				for (size_t i = 0; i != dimCount(); i++) {
					if (stringSets[i]) {
						newArea->insert(i, consolidatedOrNumericSets[i]);
					} else {
						newArea->insert(i, getDim(i));
					}
				}
				consolidatedAreas.push_back(newArea);
			}
		} else for (size_t i = 0; i != dimCount(); i++) {
				// combine all sets
				if (consolidatedSets[i]) {
					bool validArea = true;
					// create CubeArea
					PCubeArea newArea(new CubeArea(db, cube, dimCount()));
					for (size_t j = 0; j != dimCount(); j++) {
						// fill newArea sets
						if (j < i) {
							if (numericSets[j]) {
								newArea->insert(j, numericSets[j]);
							} else {
								validArea = false;
								break;
							}
						} else if (j == i) {
							newArea->insert(j, consolidatedSets[j]);
						} else {
							if (consolidatedOrNumericSets[j]) {
								newArea->insert(j, consolidatedOrNumericSets[j]);
							} else {
								validArea = false;
								break;
							}
						}
					}
					if (validArea) {
						consolidatedAreas.push_back(newArea);
					}
				}
			}
	}
	if (stringSetsCount || consolidatedSetsCount) {
		if (numericSetsCount == dimCount()) {
			PCubeArea newArea(new CubeArea(db, cube, dimCount()));
			for (size_t dim = 0; dim != dimCount(); dim++) {
				newArea->insert(dim, numericSets[dim]);
			}
			numericAreas.push_back(newArea);
		}
	} else {
		numericAreas.push_back(shared_from_this());
	}
}

CPSet Area::getDim(size_t dimOrdinal) const
{
	if (isSingleCell()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid call of Area::getDim() method");
//		Area *t = const_cast<Area *>(this);
//		size_t size = t->vpath.size();
//		t->area.resize(size);
//		for (size_t i = 0; i < size; i++) {
//			PSet s(new Set);
//			s->insert(t->vpath[i]);
//			t->area[i] = s;
//		}
//		t->vpath.clear();
	}
	return area[dimOrdinal];
}

bool Area::isOverlapping(const Area &area) const
{
	if (dimCount() != area.dimCount()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "different key dimensionality");
	}
	bool overlap = true;
	if (!vpath.size() && !area.vpath.size()) {
		for (size_t i = 0; i < this->area.size(); i++) {
			const Set *s1 = this->area[i].get();
			const Set *s2 = area.area[i].get();
			if (s1 && s2 && s1->size() && s2->size()) {
				if (!s1->intersection(s2, 0, 0)) {
					overlap = false;
					break;
				}
			}
		}
	} else if (vpath.size() && area.vpath.size()) {
		for (size_t i = 0; i < vpath.size(); i++) {
			if (vpath[i] != area.vpath[i]) {
				overlap = false;
				break;
			}
		}
	} else {
		const IdentifiersType &path = vpath.size() ? vpath : area.vpath;
		for (size_t i = 0; i < dimCount(); i++) {
			const Set *s = vpath.size() ? area.area[i].get() : this->area[i].get();
			if (s && s->size()) {
				if (s->find(path[i]) == s->end()) {
					overlap = false;
					break;
				}
			}
		}
	}
	return overlap;
}

string Area::toString() const
{
	stringstream ss;
	ss << *this;
	return ss.str();
}

bool Area::operator==(const Area &area) const
{
	if (dimCount() != area.dimCount()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "different key dimensionality");
	}
	bool result = true;
	if (getSize() != area.getSize()) {
		result = false;
	} else if (!vpath.size() && !area.vpath.size()) {
		for (size_t i = 0; i < this->area.size(); i++) {
			const Set *s1 = this->area[i].get();
			const Set *s2 = area.area[i].get();
			if ((s1 && !s2) || (!s1 && s2)) {
				result = false;
				break;
			}
			if (s1 && s2) {
				if (*s1 != *s2) {
					result = false;
					break;
				}
			}
		}
	} else if (vpath.size() && area.vpath.size()) {
		for (size_t i = 0; i < vpath.size(); i++) {
			if (vpath[i] != area.vpath[i]) {
				result = false;
				break;
			}
		}
	} else {
		const IdentifiersType &path = vpath.size() ? vpath : area.vpath;
		for (size_t i = 0; i < dimCount(); i++) {
			const Set *s = vpath.size() ? area.area[i].get() : this->area[i].get();
			if (*s->begin() != path[i]) {
				result = false;
				break;
			}
		}
	}
	return result;
}

bool Area::validate() const
{
	bool result = true;
//	for (Area::ConstDimIter a1 = dimBegin(); a1 != dimEnd(); ++a1) {
//		result |= (*a1)->validate();
//	}
	return result;
}

bool Area::intersection(const Area &area) const
{
	size_t dims = dimCount();
	vector<PSet> intersectionSets(dims);
	vector<PSet> complementSets(dims);
	vector<PSet>::iterator interIt = intersectionSets.begin();
	vector<PSet>::iterator complIt = complementSets.begin();
	for (size_t dim = 0; dim < dimCount(); ++interIt, ++complIt, dim++) {
		CPSet s1 = getDim(dim);
		CPSet s2 = area.getDim(dim);
		if (!s1->intersection(s2.get(), 0, 0)) {
			return false;
		}
	}
	return true;
}

PArea Area::reduce(uint32_t dimOrdinal, const set<IdentifierType>& subset) const
{
	size_t dims = dimCount();
	PArea result(new Area(dims));
	for (size_t i = 0; i < dims; i++) {
		if (i == dimOrdinal) {
			const Set *set = area[dimOrdinal].get();
			PSet s(new Set);
			for (Set::Iterator it = set->begin(); it != set->end(); ++it) {
				IdentifierType id = *it;
				if (subset.find(id) == subset.end()) {
					s->insert(id);
				}
			}
			result->insert(i, s);
		} else {
			result->insert(i, getDim(i));
		}
	}
	return result;
}

bool CubeArea::intersection(const Area &area, PCubeArea *intersection, SubCubeList *complementList) const
{
	size_t dims = dimCount();
	vector<PSet> intersectionSets(dims);
	vector<PSet> complementSets(dims);
	vector<PSet>::iterator interIt = intersectionSets.begin();
	vector<PSet>::iterator complIt = complementSets.begin();
	PCubeArea intersectionArea(new CubeArea(getDatabase(), getCube(), dims));
	for (size_t dim = 0; dim < dimCount(); ++interIt, ++complIt, dim++) {
		CPSet s1 = getDim(dim);
		CPSet s2 = area.getDim(dim);
		if (!s1->intersection(s2.get(), intersection ? &*interIt : 0, complementList ? &*complIt : 0)) {
			return false;
		}
		if (intersection) {
			intersectionArea->insert(dim, *interIt);
		}
	}
	if (intersection) {
		*intersection = intersectionArea;
	}
	if (complementList) {
		for (size_t dim = 0; dim < dims; ++dim) {
			if (complementSets[dim] && complementSets[dim]->size()) {
				PCubeArea complementArea(new CubeArea(getDatabase(), getCube(), dims));
				for (size_t dim2 = 0; dim2 < dim; ++dim2) {
					complementArea->insert(dim2, intersectionSets[dim2]);
				}
				complementArea->insert(dim, complementSets[dim]);
				for (size_t dim2 = dim+1; dim2 < dims; ++dim2) {
					complementArea->insert(dim2, getDim(dim2));
				}
				complementList->insertAndMerge(complementArea);
			}
		}
	}
	return true;
}

PCubeArea CubeArea::copy() const
{
	size_t dims = dimCount();
	PCubeArea ret(new CubeArea(db, cube, dims));
	for (size_t i = 0; i < dims; ++i) {
		ret->insert(i, CPSet(getDim(i)));
	}
	return ret;
}

bool CubeArea::operator==(const CubeArea &area) const
{
	return db == area.db && cube == area.cube && Area::operator==(area);
}

void Area::calcSize()
{
	areaSize = 1;
	if (!vpath.size()) {
		for (size_t i = area.size(); i > 0; i--) {
			if (area[i - 1]) {
				stepSizes[i - 1] = areaSize;
				areaSize *= area[i - 1]->size();
			} else {
				areaSize = 0;
				break;
			}
		}
	}
}

CubeArea::CubeArea(CPDatabase db, CPCube cube, size_t dimCount) : Area(dimCount), db(db), cube(cube)
{
	if (cube) {
		pathTranslator = cube->getPathTranslator();
	}
}

CubeArea::CubeArea(CPDatabase db, CPCube cube, const Area &area) : Area(area), db(db), cube(cube)
{
	if (cube) {
		pathTranslator = cube->getPathTranslator();
	}
}

CubeArea::CubeArea(CPDatabase db, CPCube cube, const IdentifiersType &path, bool useSet) : Area(path, useSet), db(db), cube(cube)
{
	if (cube) {
		pathTranslator = cube->getPathTranslator();
	}
}

CubeArea::CubeArea(CPDatabase db, CPCube cube, const vector<IdentifiersType> &area) : Area(area), db(db), cube(cube)
{
	if (cube) {
		pathTranslator = cube->getPathTranslator();
	}
}

PCubeArea CubeArea::expandBase(AggregationMaps *aggregationMaps, uint32_t *dimOrdinal) const
{
	const IdentifiersType &dimensions = *cube->getDimensions();
	PCubeArea result(new CubeArea(db, cube, dimensions.size()));

	if (aggregationMaps) {
		aggregationMaps->resize(dimensions.size());
	}
	IdentifiersType::const_iterator didit = dimensions.begin();
	for (size_t dim = 0; dim < dimCount(); dim++, ++didit) {
		CPDimension dimension = db->lookupDimension(*didit, false);

		PSet s(new Set);
		bool singleElement = false;
		if (elemCount(dim) == 1) {
			singleElement = true;
		}
		for (ConstElemIter eit = elemBegin(dim); eit != elemEnd(dim); ++eit) {
			Element *element = dimension->lookupElement(*eit, false);
			if (!element) {
				Logger::error << "CubeArea::expandBase element id: " << *eit << " not found in dimension: " << dimension->getName() << endl;
				continue; // possible corrupted journal
			}
			if (element->getBaseElementsCount() == 0) {
				s->insert(element->getIdentifier());
				if (aggregationMaps && (!dimOrdinal || dim == *dimOrdinal)) {
					aggregationMaps->at(dim).setBasesByParent(element->getIdentifier(), NULL);
				}
			} else {
				if (aggregationMaps && (!dimOrdinal || dim == *dimOrdinal)) {
					aggregationMaps->at(dim).setBasesByParent(element->getIdentifier(), element->getBaseElements());
				}
				if (singleElement) {
					WeightedSet *baseElements = element->getBaseElements(false);
					if (baseElements) {
						baseElements->getCompact(*s);
						//*s = *baseElements;
					}
				} else {
					for (WeightedSet::const_iterator bi = element->baseElementsBegin(); bi != element->baseElementsEnd(); ++bi) {
						s->insert(bi.first());
					}
				}
			}
		}
#ifdef _DEBUG
		if (!s->validate()) {
			Logger::debug << "Gotcha! " << *s << endl;
		}
#endif
		if (aggregationMaps && (!dimOrdinal || dim == *dimOrdinal)) {
			aggregationMaps->at(dim).buildBaseToParentMap();
			aggregationMaps->at(dim).setMergeMap(dimension->getMergeMap());
#ifdef ENABLE_GPU_SERVER
			aggregationMaps->at(dim).toGpuAggregationMap(getPathTranslator(), (uint32_t)dim);
#endif
		}
		result->insert(dim, s);
	}
	return result;
}

PCubeArea CubeArea::expandAggregation(AggregationMaps &aggregationMaps, vector<uint32_t> &expandType) const
{
	const IdentifiersType &dimensions = *cube->getDimensions();
	PCubeArea result(new CubeArea(db, cube, dimensions.size()));

	aggregationMaps.resize(dimensions.size());

	IdentifiersType::const_iterator didit = dimensions.begin();
	for (size_t dim = 0; dim < dimCount(); dim++, ++didit) {
		CPDimension dimension = db->lookupDimension(*didit, false);

		PSet s(new Set);
		for (ConstElemIter eit = elemBegin(dim); eit != elemEnd(dim); ++eit) {
			Element* element = dimension->findElement(*eit, 0, false);

			switch (expandType[dim]) {
			case SELF:
				{
					s->insert(element->getIdentifier());
					IdentifierWeightType* selfIDW = new pair<IdentifierType, double>(element->getIdentifier(), 1.0);
					const IdentifiersWeightType* selfIDsW = new IdentifiersWeightType(1, *selfIDW);
					aggregationMaps[dim].buildBaseToParentMap(element->getIdentifier(), selfIDsW);
				}
				break;
			case CHILDREN:
				{
					const IdentifiersWeightType* childE = element->getChildren();
					if (!childE || childE->empty()) {
						s->insert(element->getIdentifier());
						aggregationMaps[dim].buildBaseToParentMap(element->getIdentifier());
					} else {
						aggregationMaps[dim].buildBaseToParentMap(element->getIdentifier(), childE);
						for (IdentifiersWeightType::const_iterator ci = childE->begin(); ci != childE->end(); ++ci) {
							s->insert(ci->first);
						}
					}
				}
				break;
			case LEAVES:
				{
					const WeightedSet* baseE = element->getBaseElements();
					if (!baseE || baseE->empty()) {
						s->insert(element->getIdentifier());
						aggregationMaps[dim].buildBaseToParentMap(element->getIdentifier());
					} else {
						aggregationMaps[dim].buildBaseToParentMap(element->getIdentifier(), baseE);
						s.reset(new Set(*baseE));
					}
				}
				break;
			}
		}
		// TODO: -jj- buildSourceToTargetNG
		aggregationMaps[dim].compactSourceToTarget();
#ifdef ENABLE_GPU_SERVER
		aggregationMaps[dim].toGpuAggregationMap(getPathTranslator(), (uint32_t)dim);
#endif

		result->insert(dim, s);
	}
	return result;
}

PArea CubeArea::expandStar(ExpandStarType type) const
{
	const IdentifiersType &dimensions = *cube->getDimensions();
	size_t dimCount = dimensions.size();
	PArea result(new Area(dimCount));

	for (size_t i = 0; i < dimCount; i++) {
		if (elemCount(i)) {
			result->insert(i, getDim(i));
		} else {
			CPDimension dimension = db->lookupDimension(dimensions[i], false);
			result->insert(i, dimension->getElemIds(type));
		}
	}
	return result;
}

PArea CubeArea::expandStarOptim(PSet fullSet) const
{
	const IdentifiersType &dimensions = *cube->getDimensions();
	size_t dimCount = dimensions.size();
	PArea result(new Area(dimCount));

	for (size_t i = 0; i < dimCount; i++) {
		if (elemCount(i)) {
			result->insert(i, getDim(i));
		} else {
			result->insert(i, fullSet);
		}
	}
	return result;
}

CubeArea::CellType CubeArea::getType(const PathIterator &cellPath) const
{
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));
	const IdentifiersType *dims = cube->getDimensions();
	CubeArea::CellType pt = CubeArea::BASE_NUMERIC;
	for (size_t i = 0; i < dims->size(); i++) {
		CPDimension dimension = db->lookupDimension(dims->at(i), false);
		Element::Type et;
		bool stringConsolidation = false;
		if (dimension->getDimensionType() == Dimension::VIRTUAL) {
			et = Element::NUMERIC;
		} else {
			Element *element = dimension->lookupElement((*cellPath)[i], false);
			if (!element) {
				throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with id '" + StringUtils::convertToString((*cellPath)[i]) + "' not found in dimension '" + dimension->getName() + "'", "id", (*cellPath)[i]);
			}
			et = element->getElementType();
			stringConsolidation = element->isStringConsolidation();
		}
		if (et == Element::STRING || stringConsolidation) {
			return CubeArea::BASE_STRING;
		} else if (cube->supportsAggregations() && et == Element::CONSOLIDATED) {
			pt = CubeArea::CONSOLIDATED;
		}
	}
	return pt;
}

bool CubeArea::isBase(const PathIterator &cellPath) const
{
	return getType(cellPath) != CONSOLIDATED;
}

bool CubeArea::baseOnly(CellType type)
{
	return !(type & ~CubeArea::BASE);
}

ostream& operator<<(ostream& ostr, const Area &cubeArea)
{
	ostr << '{';
	for (size_t dim = 0; dim != cubeArea.dimCount(); dim++) {
		if (dim) {
			ostr << 'x';
		}

		ostr << '(';
		CPSet s = cubeArea.getDim(dim);
		if (!s) {
			ostr << '*';
		} else {
			ostr << *s;
		}
		ostr << ')';
	}
	ostr << '}';
	return ostr;
}

ostream& operator<<(ostream& ostr, const vector<IdentifiersType> &cubePaths)
{
	ostr << '{';
	for (vector<IdentifiersType>::const_iterator path = cubePaths.begin(); path != cubePaths.end(); ++path) {
		if (path != cubePaths.begin()) {
			ostr << ',';
		}
		ostr << '(';
		for (IdentifiersType::const_iterator elem = path->begin(); elem != path->end(); ++elem) {
			if (elem != path->begin()) {
				ostr << ',';
			}
			if (*elem == NO_IDENTIFIER) {
				ostr << '*';
			} else {
				ostr << *elem;
			}
		}
		ostr << ')';
	}
	ostr << '}';
	return ostr;
}

ostream& operator<<(ostream& ostr, const Set &set)
{
	for (std::set<std::pair<IdentifierType, IdentifierType> >::iterator range = set.ranges.begin(); range != set.ranges.end(); ++range) {
		if (range != set.ranges.begin()) {
			ostr << ',';
		}
		if (range->first == range->second) {
			ostr << range->first;
		} else {
			ostr << range->first << '-' << range->second;
		}
	}
	return ostr;

}


double WeightedSet::const_iterator::second() const
{
	return parent->rangeWeight(it->first);
}


double WeightedSet::range_iterator::weight() const
{
	return parent->rangeWeight(it->first);
}


double WeightedSet::rangeWeight(IdentifierType rangeStart) const
{
	map<IdentifierType, double>::const_iterator found = weights.find(rangeStart);
	if (found == weights.end()) {
		return 1;
	} else {
		return found->second;
	}
}

void WeightedSet::pushSorted(IdentifierType low, IdentifierType high, double weight)
{
	if (empty()) {
		ranges.insert(make_pair(low, high));
		if (weight != 1) {
			weights[low] = weight;
		}
	} else {
		SetType::reverse_iterator lastRange = ranges.rbegin();
		if (lastRange->second + 1 == low && rangeWeight(lastRange->first) == weight) {
			// append
			pair<IdentifierType, IdentifierType> val = *lastRange;
			val.second = high;
			ranges.erase(--ranges.end());
			ranges.insert(val);
		} else {
			ranges.insert(make_pair(low, high));
			if (weight != 1) {
				weights[low] = weight;
			}
		}
	}
	siz += high-low+1;
}


void WeightedSet::fastAdd(IdentifierType id, double weight)
{
	pair<SetType::iterator, bool> itb = ranges.insert(make_pair(id, id));
	if (itb.second) {
		if (weight != 1) {
			weights[id] = weight;
		}
		siz++;
	} else {
		map<IdentifierType, double>::iterator it = weights.find(id);
		if (it == weights.end()) {
			weights[id] = 1 + weight;
		} else {
			weights[id] += weight;
		}

		if (weights[id] == 1) {
			weights.erase(weights.find(id));
		}
	}
}


void WeightedSet::consolidate()
{
	// consolidates WeightedSet - joins all one-id ranges into valid ranges used in WeightedSet
	for (SetType::iterator it = ranges.begin(); it != ranges.end(); ++it) {
		double weight = rangeWeight(it->first);

		while (true) {
			SetType::iterator next = it;
			++next;
			if (next == ranges.end() || it->second + 1 != next->first) {
				// no more ids to process or not continuous
				break;
			}

			double nextWeight = rangeWeight(next->first);
			if (nextWeight != weight) {
				// different weights, cannot be joined
				break;
			}

			// next will be joined into it, delete next
			if (nextWeight != 1) {
				weights.erase(weights.find(next->first));
			}
			ranges.erase(next);

			// join
			pair<IdentifierType, IdentifierType> join = *it;
			join.second++;
			ranges.erase(it);
			pair<SetType::iterator, bool> ins = ranges.insert(join);
			it = ins.first;
		}
	}
}

void WeightedSet::multiply(double factor)
{
	if (factor == 1) {
		return;
	}

	for (Set::range_iterator rit = rangeBegin(); rit != rangeEnd(); ++rit) {
		double newWeight = rangeWeight(rit.low()) * factor;
		if (newWeight != 1) {
			weights[rit.low()] = newWeight;
		} else {
			weights.erase(rit.low());
		}
	}
}

WeightedSet * WeightedSet::add(const WeightedSet &ws2, double factor) const
{
	WeightedSet::const_iterator it1 = this->begin();
	WeightedSet::const_iterator it2 = ws2.begin();
	WeightedSet *wsNew = new WeightedSet();

	while (it1 != this->end() || it2 != ws2.end()) {
		if (it1 != this->end() && it2 != ws2.end()) {
			if (it1.first() == it2.first()) {
				wsNew->pushSorted(it1.first(), it1.second() + factor * it2.second());
				++it1;
				++it2;
			} else if (it1.first() < it2.first()) {
				wsNew->pushSorted(it1.first(), it1.second());
				++it1;
			} else {
				wsNew->pushSorted(it2.first(), factor * it2.second());
				++it2;
			}
		} else if (it1 == this->end()) {
			wsNew->pushSorted(it2.first(), factor * it2.second());
			++it2;
		} else /*if (it2 == ws2.end())*/ {
			wsNew->pushSorted(it1.first(), it1.second());
			++it1;
		}
	}

	return wsNew;
}


// "this" contains ws2 with some factor already for purpose of this function
WeightedSet * WeightedSet::subtract(const WeightedSet &ws2, double factor) const
{
	WeightedSet::const_iterator it1 = this->begin();
	WeightedSet::const_iterator it2 = ws2.begin();
	WeightedSet *wsNew = new WeightedSet();

	while (it1 != this->end() || it2 != ws2.end()) {
		if (it1 != this->end() && it2 != ws2.end()) {
			if (it1.first() == it2.first()) {
				if (it1.second() - factor * it2.second() != 0) {
					wsNew->pushSorted(it1.first(), it1.second() - factor * it2.second());
				}
				++it1;
				++it2;
			} else if (it1.first() < it2.first()) {
				wsNew->pushSorted(it1.first(), it1.second());
				++it1;
			} else {
				// "this" contains ws2 with some factor (sum of factors) already for purpose of this function (even if sums of factors was 0)
				wsNew->pushSorted(it2.first(), - factor * it2.second());
				++it2;
			}
		} else if (it1 == this->end()) {
			wsNew->pushSorted(it2.first(), - factor * it2.second());
			++it2;
		} else /*if (it2 == ws2.end())*/ {
			wsNew->pushSorted(it1.first(), it1.second());
			++it1;
		}
	}

	return wsNew;
}


// optimized insert when push_back is needed
void WeightedSet::pushSorted(IdentifierType id, double weight)
{
	pushSorted(id, id, weight);
}


void WeightedSet::clear()
{
	ranges.clear();
	siz = 0;
	weights.clear();
}


WeightedSet::range_iterator WeightedSet::lastRange()
{
	if (siz > 0) {
		return range_iterator(--ranges.end(), *this);
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "set is empty");
	}
}

double WeightedSet::weight(IdentifierType id) const
{
	Set::range_iterator rit = rangeLowerBound(id);
	if (rit != rangeEnd() && rit.low() <= id && id <= rit.high()) {
		return rangeWeight(rit.low());
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "element id out of range");
	}
}

void WeightedSet::getCompact(Set &set)
{
	set.clear();
	if (weights.empty()) {
		set = *this;
	} else {
		IdentifierType lastBegin = NO_IDENTIFIER;
		IdentifierType lastEnd = NO_IDENTIFIER;
		for (Set::range_iterator rit = rangeBegin(); rit != rangeEnd(); ++rit) {
			if (lastEnd != NO_IDENTIFIER) {
				if (lastEnd+1 == rit.low()) {
					lastEnd = rit.high(); // extend last
					continue;
				} else {
					set.insertRange(lastBegin, lastEnd); // save prev
				}
			}
			lastBegin = rit.low();
			lastEnd = rit.high();
		}
		if (lastEnd != NO_IDENTIFIER) {
			set.insertRange(lastBegin, lastEnd); // save prev
		}
	}
}

vector<PWeightedSet> WeightedSet::splitByFilter(vector<const Set *> sets) const
{
	vector<PWeightedSet> result(sets.size(), PWeightedSet());
	vector<pair<Set::range_iterator, Set::range_iterator> > filterRanges;
	for (vector<const Set *>::const_iterator set = sets.begin(); set != sets.end(); ++set) {
		filterRanges.push_back(make_pair((*set)->rangeBegin(), (*set)->rangeEnd()));
	}
	// for all ranges
	for (range_iterator range = rangeBegin(); range != rangeEnd(); ++range) {
		vector<pair<Set::range_iterator, Set::range_iterator> >::iterator filterRange = filterRanges.begin();
		vector<PWeightedSet>::iterator outWSet = result.begin();
		// for all filters
		for (;filterRange != filterRanges.end(); ++filterRange, ++outWSet) {
			// end of the filter is not yet reached
			while (filterRange->first != filterRange->second) {
				IdentifierType filterLow = filterRange->first.low();
				IdentifierType filterHigh = filterRange->first.high();
				// if filter is before the current range - move to next filter range
				if (filterHigh < range.low()) {
					++filterRange->first;
				} else if (filterLow > range.high()) {
					// next filter
					break;
				} else {
					// range intersection
					if (!*outWSet) {
						*outWSet = PWeightedSet(new WeightedSet());
					}
					(*outWSet)->pushSorted(max(filterLow, range.low()), min(filterHigh, range.high()), range.weight());
					if (filterHigh > range.high()) {
						break;
					} else {
						++filterRange->first;
					}
				}
			}
		}
	}
	return result;
}

PSubCubeList LockedCells::getLockedAreas()
{
	if (!lockedAreas && lockedPaths) {
		PSubCubeList lockedAreasMerged(new SubCubeList());
		SubCubeList lockedPathMerged;
		// TODO: -jj- can be optimized like CellValuesJob::buildSubCubes
		for (vector<IdentifiersType>::const_iterator pathIt = lockedPaths->begin(); pathIt != lockedPaths->end(); ++pathIt) {
			size_t size = pathIt->size();
			PArea area(new Area(size));
			for (size_t i = 0; i < size; i++) {
				PSet s(new Set);
				s->insert(pathIt->at(i));
				area->insert(i, s);
			}
			lockedPathMerged.insertAndMerge(PCubeArea(new CubeArea(db, cube, *area)));
		}
		for (SubCubeList::iterator scIt = lockedPathMerged.begin(); scIt != lockedPathMerged.end(); ++scIt) {
			PCubeArea expandedArea = scIt->second->expandBase(0, 0);
			lockedAreasMerged->insertAndMerge(expandedArea);
		}
		lockedAreas = lockedAreasMerged;
	}
	return lockedAreas;
}

/*
vector<PWeightedSet> WeightedSet::splitByFilterMergeDebug(vector<const Set *> sets) const
{
	typedef Set::merged_range_iterator         filter_merge_iterator;
	typedef WeightedSet::merged_range_iterator range_merge_iterator;
	typedef vector<pair<filter_merge_iterator, filter_merge_iterator> > FilterVector;

	{
		Logger::LoggerStream& line = Logger::debug << "sets: ";
		for (vector<const Set *>::const_iterator set = sets.begin(); set != sets.end(); ++set) {
			line << "{";
			for (filter_merge_iterator range = (*set)->rangeBeginMerged(); range != (*set)->rangeEndMerged(); ++range) {
				line << "["<< range.low() << "," << range.high() << "],";
			}
			line << "},";
		}
		line << endl;
	}

	{
		Logger::LoggerStream& line = Logger::debug << "this: " << "{";
		for (range_merge_iterator range = rangeBeginMerged(); range != rangeEndMerged(); ++range) {
			line << "["<< range.low() << "," << range.high() << "],";
		}
		line << "}" << endl;
	}

	vector<PWeightedSet> result(sets.size(), PWeightedSet());
	FilterVector filterRanges;
	for (vector<const Set *>::const_iterator set = sets.begin(); set != sets.end(); ++set) {
		filterRanges.push_back(make_pair((*set)->rangeBeginMerged(), (*set)->rangeEndMerged()));
	}
	// for all ranges
	for (range_merge_iterator range = rangeBeginMerged(); range != rangeEndMerged(); ++range) {
		FilterVector::iterator filterRange = filterRanges.begin();
		vector<PWeightedSet>::iterator outWSet = result.begin();
		// for all filters
		for (;filterRange != filterRanges.end(); ++filterRange, ++outWSet) {
			// end of the filter is not yet reached
			while (filterRange->first != filterRange->second) {
				IdentifierType filterLow = filterRange->first.low();
				IdentifierType filterHigh = filterRange->first.high();
				// if filter is before the current range - move to next filter range
				if (filterHigh < range.low()) {
					++filterRange->first;
				} else if (filterLow > range.high()) {
					// next filter
					break;
				} else {
					// range intersection
					if (!*outWSet) {
						*outWSet = PWeightedSet(new WeightedSet());
					}
					(*outWSet)->pushSorted(max(filterLow, range.low()), min(filterHigh, range.high()), range.weight());
					if (filterHigh > range.high()) {
						break;
					} else {
						++filterRange->first;
					}
				}
			}
		}
	}

	{
		Logger::LoggerStream& line = Logger::debug << "result: ";
		for (vector<PWeightedSet>::const_iterator set = result.begin(); set != result.end(); ++set) {
			line << "{";
			for (WeightedSet::range_iterator range = (*set)->rangeBegin(); range != (*set)->rangeEnd(); ++range) {
				line << "["<< range.low() << "," << range.high() << "],";
			}
			line << "},";
		}
		line << endl;
	}

	return result;
}*/


//bool WeightedSet::intersection(const Set *set, PWeightedSet *intersection)
//{
//	PSet setIntersection;
//	if (Set::intersection(set, intersection ? &setIntersection : 0, 0)) {
//		// has intersection
//		if (*setIntersection == *this) {
//			// identical
//			*intersection = boost::dynamic_pointer_cast<WeightedSet, Set>(shared_from_this());
//		} else {
//			PWeightedSet weightedIntersection(new WeightedSet(*setIntersection));
//			// add weights and other information
////			for () {
////			}
//		}
//		return true;
//	} else {
//		return false;
//	}
//}

}
