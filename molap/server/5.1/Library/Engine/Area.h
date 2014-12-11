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

#ifndef OLAP_AREA_H
#define OLAP_AREA_H 1

#include "palo.h"
#include "Engine/AggregationMap.h"
#include "Olap/User.h"
#include "boost/enable_shared_from_this.hpp"

namespace palo {

class PathTranslator {
public:
	PathTranslator(const IdentifiersType &dimensionSizes, bool dimensionSizesInBit=false);
    PathTranslator(const PathTranslator &pathTranslator);
	void pathToBinPath(const IdentifiersType& path, GpuBinPath& binPath) const;
	void binPathToPath(const GpuBinPath& binPath, IdentifiersType& path) const;
	uint32_t getNumberOfBins() const {
		return (uint32_t)binDimensionsCount.size();
	}
	const vector<uint32_t> &getBinDimensionsCount() const {
		return binDimensionsCount;
	}
    void setBinDimensionsCount(uint32_t binIdx, uint32_t count){
        binDimensionsCount[binIdx] = count;
    }
	const vector<gpuBinType> &getDimensionsBitmask() const {
		return dimensionsBitmask;
	}
	const vector<uint32_t> &getDimensionsPos() const {
		return dimensionsPos;
	}
    const uint32_t getDimensionBitSize(uint32_t dimIdx) const{
    	if(!dimIdx || dimensionsPos[dimIdx-1] <= dimensionsPos[dimIdx])
    		return 64 - dimensionsPos[dimIdx];
    	else
    		return dimensionsPos[dimIdx-1] - dimensionsPos[dimIdx];
    }
    const vector<pair<uint32_t, uint32_t> > getBinDimensionRanges() const {
        uint32_t dimIdx = 0;
        vector<pair<uint32_t, uint32_t> > res;
        for(uint32_t binIdx = 0; binIdx < getNumberOfBins(); binIdx++){
            pair<uint32_t, uint32_t> range;
            range.first = dimIdx;
            dimIdx += binDimensionsCount[binIdx]-1;
            range.second = dimIdx;
            ++dimIdx;
            res.push_back(range);
        }
        return res;
    }
    void enlargeDimension(uint32_t startDim, uint32_t numDims, uint32_t numBits){
        //resize bitmask of the dimension to be enlarged
        dimensionsBitmask[startDim] |= (((1ULL<<(numBits+1))-1)<<(dimensionsPos[startDim]-numBits));
        //"rightshift" numDims dimensions right of the dimension to be enlarged
        uint32_t dimIdx = startDim;
        while(dimIdx < startDim + numDims){
            dimensionsPos[dimIdx] -= numBits;
            if(dimIdx != startDim)
                dimensionsBitmask[dimIdx] >>= numBits;
            ++dimIdx;
        }
    }
	gpuBinType dimIdToBinDim(IdentifierType dimId, uint32_t dimIdx) const;
	IdentifierType binDimToDimId(gpuBinType binDim, uint32_t dimIdx) const;
private:
	vector<uint32_t>	binDimensionsCount;		// the number of dimensions per bin
	vector<uint32_t>	dimensionsPos;			// bit positions inside bins
	vector<uint32_t>	dimensionsMap;			// dimension index mapping
	vector<gpuBinType>	dimensionsBitmask;		// stored bit masks for dimensions

	// constant for size of bin
	enum { MaxBinCharge = sizeof(gpuBinType) * 8 };
};

class Set : public boost::enable_shared_from_this<Set> {
public:
	typedef set<pair<IdentifierType, IdentifierType> > SetType;

	class Iterator  : public std::iterator<forward_iterator_tag, IdentifierType, ptrdiff_t, IdentifierType *, IdentifierType &> {
	public:
		Iterator(const Iterator &it);
		Iterator();
		Iterator(const SetType::iterator &it, const Set &s, bool end);
		Iterator(const SetType::iterator &it, IdentifierType pos, const Set &s);
		Iterator(IdentifierType elemId, bool end);
		IdentifierType operator*() const;
		bool operator!=(const Iterator &it) const;
		bool operator==(const Iterator &it) const;
		Iterator &operator--();
		Iterator &operator++();
		Iterator operator++(int);
		Iterator &operator=(const Iterator &it);
	protected:
		SetType::const_iterator it;
	private:
		//general set
		IdentifierType pos;
		const Set *par;
		friend class Set;
		//single element
		IdentifierType singleElementId;
		bool end;
	};
	class range_iterator {
	public:
		range_iterator(const SetType::const_iterator &it) : it(it) {}

		bool operator!=(const range_iterator &it2) const {return it2.it != it;}
		bool operator==(const range_iterator &it2) const {return it2.it == it;}
		range_iterator &operator++() {++it; return *this;}
		range_iterator &operator--() {--it; return *this;}
		range_iterator operator++(int) {range_iterator ret(*this); ++(*this); return ret;}

		IdentifierType low() const {return it->first;}
		IdentifierType high() const {return it->second;}
	private:
		SetType::const_iterator it;
	};

	Set();
	Set(bool full);
	virtual ~Set() {}
	Set(const Set &set);
	bool insert(IdentifierType id);
	template<typename T> void insert(T first, T last) {
		for (; first != last; ++first) {
			insert(*first);
		}
	}
	Iterator begin() const;
	Iterator end() const;
	Iterator find(IdentifierType id) const;
	Iterator lowerBound(IdentifierType id) const;
	// Use 'it = set.erase(it)' not 'set.erase(it++)' !!!
	Iterator erase(const Iterator &it);
	size_t size() const;
	size_t getRangesCount() const {return ranges.size();}
	void clear();

	range_iterator rangeBegin() const {return range_iterator(ranges.begin());}
	range_iterator rangeEnd() const {return range_iterator(ranges.end());}
	range_iterator rangeLowerBound(IdentifierType id) const;
	void insertRange(IdentifierType low, IdentifierType high);

	bool empty() const;
	bool operator!=(const Set &set) const;
	bool operator==(const Set &set) const;
	bool intersection_check(const Set *set, PSet *intersection, PSet *complement) const;
	bool intersection(const Set *set, PSet *intersection, PSet *complement) const;
	CPSet limit(IdentifierType start, IdentifierType end) const;
	bool validate() const;

	static PSet addAncestors(CPSet set, CPDimension dim);

protected:
	static void addAncestor(PSet set, CPDimension dim, IdentifierType elemId);

	SetType ranges;
	size_t siz;
	friend ostream& operator<<(ostream& ostr, const Set &set);
	friend struct GpuArea;
};

class WeightedSet : public Set {
public:

	class const_iterator : public Set::Iterator {
	public:
		const_iterator(const SetType::iterator &it, const WeightedSet &s, bool end) : Iterator(it, s, end), parent(&s) {}

		IdentifierType first() const {return this->operator *();}
		double second() const;
	private:
		const WeightedSet *parent;
	};

	class range_iterator : public Set::range_iterator {
	public:
		range_iterator(const SetType::iterator &it, const WeightedSet &s) : Set::range_iterator(it), it(it), parent(&s) {}

		bool operator==(const range_iterator &it2) const {return it2.it == it;}
		bool operator!=(const range_iterator &it2) const {return it2.it != it;}
		range_iterator &operator++() {++it; return *this;}
		range_iterator operator++(int) {range_iterator ret(*this); ++(*this); return ret;}

		IdentifierType low() const {return it->first;}
		IdentifierType high() const {return it->second;}
		double weight() const;
	private:
		SetType::const_iterator it;
		const WeightedSet *parent;
	};

	WeightedSet() : Set() {}
	virtual ~WeightedSet() {}
	WeightedSet(const Set &s) : Set(s) {}
	WeightedSet(const WeightedSet &ws) : Set(ws), weights(ws.weights) {}
	WeightedSet(const WeightedSet &ws, double factor) : Set(ws), weights(ws.weights) {multiply(factor);}

	WeightedSet * add(const WeightedSet &ws2, double factor) const;
	WeightedSet * subtract(const WeightedSet &ws2, double factor) const;
	void multiply(double factor);

	void pushSorted(IdentifierType id, double weight);
	void pushSorted(IdentifierType low, IdentifierType high, double weight);

	void fastAdd(IdentifierType id, double weight); // add ids in random order, only one by one
	void consolidate(); // optimize set after fastAdd sequence

	void clear();

	const_iterator begin() const {return const_iterator(ranges.begin(), *this, ranges.begin()==ranges.end());}
	const_iterator end() const {return const_iterator(ranges.end(), *this, true);}

	range_iterator rangeBegin() const {return range_iterator(ranges.begin(), *this);}
	range_iterator rangeEnd() const {return range_iterator(ranges.end(), *this);}
	range_iterator lastRange();

	size_t rangesCount() const {return ranges.size();}

	bool hasWeights() const {return !weights.empty();}
	double weight(IdentifierType id) const;

	void getCompact(Set &set);

	friend class const_iterator;
	friend class range_iterator;

	vector<PWeightedSet> splitByFilter(vector<const Set *> sets) const;
private:
	double rangeWeight(IdentifierType rangeStart) const;

	map<IdentifierType, double> weights; // weight = 1 if not found
};

typedef multimap<IdentifierType, IdentifierType> SetMultimap;
typedef boost::shared_ptr<SetMultimap> PSetMultimap;

class SetMultimaps : public vector<PSetMultimap>
{
public:
	SetMultimaps(size_t dimCount = 0) : vector<PSetMultimap>(dimCount, PSetMultimap()) {}
	bool isTrivialMapping() const;
};

class Area {
public:
	typedef Set::Iterator ConstElemIter;

	class PathIterator {
	public:
		PathIterator(const Area &area, bool end, const vector<ConstElemIter> &path);
		PathIterator(const Area &area, bool end, const IdentifiersType &path);
		PathIterator();
		PathIterator(const PathIterator &it);
		PathIterator &operator++();
		const IdentifiersType &operator*() const;
		bool operator==(const PathIterator &it) const;
		bool operator!=(const PathIterator &it) const;
		bool operator!=(const IdentifiersType &key) const;
		PathIterator operator+(double count) const;
		double operator-(const PathIterator &it) const;
		string toString() const;
		IdentifierType at(size_t index) const;
	private:
		double getPosition() const;
		vector<ConstElemIter> path;
		IdentifiersType ids;
		bool end;
		const Area *area;
		bool singlePath;
	};

	Area(size_t dimCount);
	Area(const Area &area);
	Area(const IdentifiersType &path, bool useSet);
	Area(const vector<IdentifiersType> &area);

	ConstElemIter elemBegin(size_t dimOrdinal) const;
	ConstElemIter elemEnd(size_t dimOrdinal) const;

	PathIterator pathBegin() const;
	PathIterator pathEnd() const;
	PathIterator getIterator(CPArea path) const;

	void insert(size_t dimOrdinal, CPSet elems, bool calc = true);
	ConstElemIter find(size_t dimOrdinal, IdentifierType elemId) const;
	PathIterator find(const IdentifiersType &path) const;
	PathIterator lowerBound(const IdentifiersType &path) const;
	size_t dimCount() const;
	size_t elemCount(size_t dimOrdinal) const;
	CPSet getDim(size_t dimOrdinal) const;
	bool isOverlapping(const Area &area) const;
	double getSize() const {
		return areaSize;
	}
	PPathTranslator getPathTranslator() const {
		return pathTranslator;
	}
	string toString() const;
	bool operator==(const Area &area) const;
	bool validate() const;
	bool intersection(const Area &area) const;
	PArea reduce(uint32_t dimOrdinal, const set<IdentifierType>& subset) const;
private:
	void calcSize();
	vector<CPSet> area; //general area
	double areaSize;
	vector<double> stepSizes;
protected:
	IdentifiersType vpath; //single cell
	bool isSingleCell() const {return !vpath.empty();}
	PPathTranslator pathTranslator;
	friend struct GpuArea;
};

class SubCubeList;

class CubeArea : public Area, public boost::enable_shared_from_this<CubeArea> {
public:
	enum CellType {
		NONE = 0, BASE_NUMERIC = 1, BASE_STRING = 2, BASE = 3, CONSOLIDATED = 4, NUMERIC = 5, ALL = 7
	};
	enum ExpandAggrType {
		SELF = 1, CHILDREN = 2, LEAVES = 4
	};
	enum ExpandStarType {
		BASE_ELEMENTS = 1, TOP_ELEMENTS = 2, ALL_ELEMENTS = 3, NUMERIC_ELEMENTS /*base and cons*/ = 4
	};

	CubeArea(CPDatabase db, CPCube cube, size_t dimCount);
	CubeArea(CPDatabase db, CPCube cube, const Area &area);
	CubeArea(CPDatabase db, CPCube cube, const IdentifiersType &path, bool useSet = true);
	CubeArea(CPDatabase db, CPCube cube, const vector<IdentifiersType> &area);

	list<PCubeArea> split(PathIterator begin, size_t cellCount, bool &toTheEnd) const;
	void split(const IdentifiersType &start, const IdentifiersType &stop, list<PCubeArea> &areaList, size_t dim) const;
	void splitbyTypes(SubCubeList &stringAreas, SubCubeList &numericAreas, SubCubeList &consolidatedAreas) const;

	PCubeArea expandBase(AggregationMaps *aggregationMaps, uint32_t *dimOrdinal) const;
	PCubeArea expandAggregation(AggregationMaps &aggregationMaps, vector<uint32_t> &expandType) const;
	PArea expandStar(ExpandStarType type) const;
	PArea expandStarOptim(PSet fullSet) const;
	CellType getType(const PathIterator &cellPath) const;
	bool isBase(const PathIterator &cellPath) const;

	static bool baseOnly(CellType type);

	CPDatabase getDatabase() const {
		return db;
	}
	CPCube getCube() const {
		return cube;
	}
	bool intersection(const Area &area, PCubeArea *intersection, SubCubeList *complementList) const;
	PCubeArea copy() const;
	bool operator==(const CubeArea &area) const;
private:
	CPDatabase 	db;
	CPCube 		cube;
};

ostream& operator<<(ostream& ostr, const Area &cubeArea);
ostream& operator<<(ostream& ostr, const vector<IdentifiersType> &cubePaths);
ostream& operator<<(ostream& ostr, const Set &set);

////////////////////////////////////////////////////////////////////////////////
/// @brief list of locked Cells and their expanded areas
////////////////////////////////////////////////////////////////////////////////

class LockedCells {
public:
	LockedCells(CPDatabase db, CPCube cube, CPPaths lockedPaths) : db(db), cube(cube), lockedPaths(lockedPaths) {}
	PSubCubeList getLockedAreas();
	CPPaths getLockedPaths() const {return lockedPaths;}
private:
	CPDatabase db;
	CPCube cube;
	CPPaths lockedPaths;
	PSubCubeList lockedAreas;
};

}

#endif
