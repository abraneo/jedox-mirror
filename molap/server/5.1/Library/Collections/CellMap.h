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
 * 
 *
 */

#ifndef CELLMAP_H
#define CELLMAP_H

#include "palo.h"
#include "Engine/Streams.h"
#include "Engine/EngineBase.h"
#include "Thread/WriteLocker.h"

using namespace palo;

namespace palo {

class CellMapBase : public boost::enable_shared_from_this<CellMapBase> {
public:
	virtual ~CellMapBase() {};
};

//##############################################################################
//##############################################################################
//##############################################################################
template <typename ValueType>
class ICellMap : public CellMapBase {
public:
	virtual ~ICellMap() {};
	virtual bool set(const IdentifierType *path, const ValueType &value) = 0;
	virtual bool set(const IdentifiersType &path, const ValueType &value) = 0;
	virtual bool add(const IdentifierType *path, const ValueType &value) = 0;
	virtual bool add(const IdentifiersType &path, const ValueType &value) = 0;
	virtual bool get(const IdentifierType *path, ValueType &value) const = 0;
	virtual bool get(const IdentifiersType &path, ValueType &value) const = 0;
	virtual size_t size() const = 0;
	virtual void setLimit(const IdentifiersType &startPath, uint64_t maxCount) = 0;
	virtual PProcessorBase getValues() = 0;
};
//##############################################################################

template <typename ValueType>
class CellMapSync : public ICellMap<ValueType> {
public:
	CellMapSync(boost::shared_ptr<ICellMap<ValueType> > pcm) : pcm(pcm) {}
	virtual ~CellMapSync() {}
	virtual bool set(const IdentifierType *path, const ValueType &value) {WriteLocker w(&m); return pcm->set(path, value);}
	virtual bool set(const IdentifiersType &path, const ValueType &value)  {WriteLocker w(&m); return pcm->set(path, value);}
	virtual bool add(const IdentifierType *path, const ValueType &value)  {WriteLocker w(&m); return pcm->add(path, value);}
	virtual bool add(const IdentifiersType &path, const ValueType &value)  {WriteLocker w(&m); return pcm->add(path, value);}
	virtual bool get(const IdentifierType *path, ValueType &value) const  {WriteLocker w(&m); return pcm->get(path, value);}
	virtual bool get(const IdentifiersType &path, ValueType &value) const  {WriteLocker w(&m); return pcm->get(path, value);}
	virtual size_t size() const  {WriteLocker w(&m); return pcm->size();}
	virtual void setLimit(const IdentifiersType &startPath, uint64_t maxCount)  {WriteLocker w(&m); pcm->setLimit(startPath, maxCount);}
	virtual PProcessorBase getValues() {WriteLocker w(&m); return pcm->getValues();}
private:
	boost::shared_ptr<ICellMap<ValueType> > pcm;
	mutable Mutex m;
};

template <typename ValueType>
class CellVectorMap : public ICellMap<ValueType> , public std::map<IdentifiersType, ValueType > {
public:
	typedef std::pair<IdentifiersType, ValueType > PairType;
	typedef std::map<IdentifiersType, ValueType > MapType;
	CellVectorMap(size_t dims) : _dims(dims) {};
	virtual ~CellVectorMap() {};
	virtual bool set(const IdentifierType *path, const ValueType &value) {
		return set(IdentifiersType(path, path + _dims), value);
	}
	virtual bool set(const IdentifiersType &path, const ValueType &value) {
		std::pair<typename MapType::iterator, bool> ret;
		ret = this->insert(PairType(path, value));
		if (!ret.second) {
			(ret.first)->second = value;
		}
		return ret.second;
	}
	virtual bool add(const IdentifierType *path, const ValueType &value) {
		return add(IdentifiersType(path, path + _dims), value);
	}
	virtual bool add(const IdentifiersType &path, const ValueType &value) {
		std::pair<typename MapType::iterator, bool> ret;
		ret = this->insert(PairType(path, value));
		if (!ret.second) {
			(ret.first)->second += value;
		}
		return ret.second;
	}
	virtual bool get(const IdentifierType *path, ValueType &value) const {
		return get(IdentifiersType(path, path + _dims), value);
	}
	virtual bool get(const IdentifiersType &path, ValueType &value) const {
		bool found = false;
		typename MapType::const_iterator it = MapType::find(path);
		if (it != MapType::end()) {
			value = it->second;
			found = true;
		}
		return found;
	}
	virtual size_t size() const {
		return MapType::size();
	}
	virtual void setLimit(const IdentifiersType &startPath, uint64_t maxCount) {
		// TODO: -jj- implement limit amount of values
	}
	class Reader : public ProcessorBase {
	public:
		Reader(CellVectorMap &cvm) : ProcessorBase(true, PEngineBase()), started(false), pcvm(cvm.shared_from_this()), cvm(cvm), iter(cvm.begin()) {}
		virtual ~Reader() {}
		virtual bool next() {
			if (iter != cvm.end()) {
				started = true;
				vkey = &iter->first;
				val = iter->second;
				++iter;
				return true;
			} else {
				return false;
			}
		}
		virtual const CellValue &getValue() {
			cellval = val;
			if (cellval.isEmpty()) {
				cellval.setEmpty(false);
			}
			return cellval;
		}
		virtual double getDouble() {
			return val;
		}
		virtual const IdentifiersType &getKey() const {
			if (!started) {
				return EMPTY_KEY;
			}
			return *vkey;
		}
		virtual void reset() {
			started = false;
			iter = cvm.begin();
		}
		//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed
	private:
		bool started;
		boost::shared_ptr<CellMapBase> pcvm;
		CellVectorMap &cvm;
		typename MapType::const_iterator iter;
		const IdentifiersType *vkey;
		ValueType val;
		CellValue cellval;
	};

	virtual PProcessorBase getValues() {
		return PProcessorBase(new Reader(*this));
	}
private:
	size_t _dims;
};

//##############################################################################
//##############################################################################
//##############################################################################

template<int _N>
class IdsNType {
private:
	IdentifierType m_ids[_N];
public:
	IdsNType() {
		memset(m_ids, 0, sizeof(m_ids));
	};
	IdsNType(const IdentifierType *other) {
		memcpy(m_ids, other, sizeof(m_ids));
	};
	IdsNType(const IdentifiersType &other) {
		memcpy(m_ids, other.begin(), sizeof(m_ids));
	};
	IdentifierType &operator[](size_t _pos) {
		return m_ids[_pos];
	};
	bool operator <(const IdsNType& other) const {
		int cnt = _N;
		const IdentifierType *p1 = m_ids, *p2 = other.m_ids;

		while (cnt--) {
			if (*p1 == *p2) {
				p1++;
				p2++;
			} else if (*p1 < *p2) {
				return true;
			} else {
				return false;
			}
		}
		return false;
	}
	operator const IdentifierType *() const {return m_ids;}
	string toString() const {
		stringstream ss;
		ss << "[";
		bool delimit = false;
		for (int i = 0; i < _N; i++) {
			if (delimit) {
				ss << ", ";
			} else {
				delimit = true;
			}
			ss << m_ids[i];
		}
		ss << "]";
		return ss.str();
	}
};

template<int _N, typename ValueType>
class CellArrayMap : public ICellMap<ValueType>, public std::map<IdsNType<_N>, ValueType > {
public:
	typedef typename std::map<IdsNType<_N>, ValueType > MapType;
	typedef typename MapType::const_iterator const_iterator;
	typedef typename MapType::iterator iterator;

	CellArrayMap() : maxCount(0), lastPath(0) {}
	virtual ~CellArrayMap() {};
	virtual bool set(const IdentifiersType &path, const ValueType &value) {
		return set(&path[0], value);
	}
	virtual bool set(const IdentifierType *path, const ValueType &value) {
		if (lastPath && compareKey(lastPath, path) < 0) {
			return false;
		}
		pair<iterator, bool> ret;
		ret = this->insert(pair<IdsNType<_N>, ValueType >(IdsNType<_N>(path), value));
		if (!ret.second) {
			// replace existing value
			(ret.first)->second = value;
		} else { // new value
			if (maxCount && size() > maxCount) {
				MapType::erase(--MapType::end());
				// delete last value and remember new last key
				lastPath = MapType::rbegin()->first;
				return false;
			}
		}
		return ret.second;
	}
	virtual bool add(const IdentifiersType &path, const ValueType &value) {
		return add(&path[0], value);
	}
	virtual bool add(const IdentifierType *path, const ValueType &value) {
		if (lastPath && compareKey(lastPath, path) < 0) {
			return false;
		}
		pair<iterator, bool> ret;
		ret = this->insert(pair<IdsNType<_N>, ValueType >(IdsNType<_N>(path), value));
		if (!ret.second) {
			(ret.first)->second += value;
		} else {
			if (maxCount && size() > maxCount) {
				MapType::erase(--MapType::end());
				// delete last value and remember new last key
				lastPath = MapType::rbegin()->first;
				return false;
			}
		}
		return ret.second;
	}
	virtual bool get(const IdentifiersType &path, ValueType &value) const {
		return get(&path[0], value);
	}
	virtual bool get(const IdentifierType *path, ValueType &value) const {
		bool found = false;
		const_iterator it = MapType::find(path);
		if (it != MapType::end()) {
			value = it->second;
			found = true;
		}
		return found;
	}
	virtual size_t size() const {
		return MapType::size();
	}
	virtual void setLimit(const IdentifiersType &startPath, uint64_t maxCount) {
		this->startPath = startPath;
		this->maxCount = maxCount;
	}

	class Reader : public ProcessorBase {
	public:
		Reader(CellArrayMap &cam) : ProcessorBase(true, PEngineBase()), started(false), pcam(cam.shared_from_this()), cam(cam), iter(cam.begin()), vkey(_N) {}
		virtual ~Reader() {}
		virtual bool next() {
			if (iter != cam.end()) {
				memcpy(&vkey[0], &iter->first, sizeof(IdsNType<_N>));
				val = iter->second;
				started = true;
				++iter;
				return true;
			} else return false;
		}
		virtual const CellValue &getValue() {
			cellval = val;
			if (cellval.isEmpty()) {
				cellval.setEmpty(false);
			}
			return cellval;
		}
		virtual double getDouble() {
			return double(val);
		}
		virtual const IdentifiersType &getKey() const {
			if (!started) {
				return EMPTY_KEY;
			}
			return vkey;
		}
		virtual void reset() {
			started = false;
			iter = cam.begin();
		}
		virtual bool move(const IdentifiersType &key, bool *found) {
			iter = cam.lower_bound(&key[0]);
			if (iter != cam.end()) {
				memcpy(&vkey[0], &iter->first, sizeof(IdsNType<_N>));
				val = iter->second;
				started = true;
				++iter;
				if (!memcmp(&vkey[0], &key[0], sizeof(IdsNType<_N>))) {
					if (found) {
						*found = true;
					}
				} else {
					if (found) {
						*found = false;
					}
				}
				return true;
			} else {
				if (found) {
					*found = false;
				}
				return false;
			}
		}
	private:
		bool started;
		boost::shared_ptr<CellMapBase> pcam;
		CellArrayMap<_N, ValueType> &cam;
		typename CellArrayMap::iterator iter;
		IdentifiersType vkey;
		ValueType val;
		CellValue cellval;
	};

	virtual PProcessorBase getValues() {
		return PProcessorBase(new Reader(*this));
	}
private:
	IdentifiersType startPath;
	uint64_t maxCount;
	const IdentifierType *lastPath;
	int compareKey(const IdentifierType *key1, const IdentifierType *key2) {
		for (int dim = 0; dim < _N; dim++) {
			if (key1[dim] < key2[dim]) {
				return -1;
			} else if (key1[dim] > key2[dim]) {
				return 1;
			}
		}
		return 0;
	}
};

#define CREATE_CELL_MAP(DIMENSIONS) case (DIMENSIONS): pmap.reset(new CellArrayMap<(DIMENSIONS), ValueType>()); break;
#define CREATE_DEFAULT_MAP() default: pmap.reset(new CellVectorMap<ValueType>(dimensions)); break;

template <typename ValueType>
boost::shared_ptr<ICellMap<ValueType> > CreateCellMap(size_t dimensions)
{
	boost::shared_ptr<ICellMap<ValueType> > pmap;

	switch (dimensions) {
		CREATE_CELL_MAP(1);
		CREATE_CELL_MAP(2);
		CREATE_CELL_MAP(3);
		CREATE_CELL_MAP(4);
		CREATE_CELL_MAP(5);
		CREATE_CELL_MAP(6);
		CREATE_CELL_MAP(7);
		CREATE_CELL_MAP(8);
		CREATE_CELL_MAP(9);
		CREATE_CELL_MAP(10);
		CREATE_CELL_MAP(11);
		CREATE_CELL_MAP(12);
		CREATE_CELL_MAP(13);
		CREATE_CELL_MAP(14);
		CREATE_CELL_MAP(15);
		CREATE_CELL_MAP(16);
		CREATE_CELL_MAP(17);
		CREATE_CELL_MAP(18);
		CREATE_CELL_MAP(19);
		CREATE_CELL_MAP(20);
		CREATE_CELL_MAP(21);
		CREATE_CELL_MAP(22);
		CREATE_CELL_MAP(23);
		CREATE_CELL_MAP(24);
		CREATE_CELL_MAP(25);
		CREATE_CELL_MAP(26);
		CREATE_CELL_MAP(27);
		CREATE_CELL_MAP(28);
		CREATE_CELL_MAP(29);
		CREATE_CELL_MAP(30);
		CREATE_CELL_MAP(31);
		CREATE_CELL_MAP(32);
		CREATE_DEFAULT_MAP();
	}
	return pmap;
}

#undef CREATE_CELL_MAP
#undef CREATE_DEFAULT_MAP

#define CREATE_CELL_MAP(DIMENSIONS) case (DIMENSIONS): pmap.reset(new CellArrayMap<(DIMENSIONS),double>()); break;
#define CREATE_DEFAULT_MAP() default: pmap.reset(new CellVectorMap<double>(dimensions)); break;

inline PDoubleCellMap CreateDoubleCellMap(size_t dimensions)
{
	PDoubleCellMap pmap;

	switch (dimensions) {
		CREATE_CELL_MAP(1);
		CREATE_CELL_MAP(2);
		CREATE_CELL_MAP(3);
		CREATE_CELL_MAP(4);
		CREATE_CELL_MAP(5);
		CREATE_CELL_MAP(6);
		CREATE_CELL_MAP(7);
		CREATE_CELL_MAP(8);
		CREATE_CELL_MAP(9);
		CREATE_CELL_MAP(10);
		CREATE_CELL_MAP(11);
		CREATE_CELL_MAP(12);
		CREATE_CELL_MAP(13);
		CREATE_CELL_MAP(14);
		CREATE_CELL_MAP(15);
		CREATE_CELL_MAP(16);
		CREATE_CELL_MAP(17);
		CREATE_CELL_MAP(18);
		CREATE_CELL_MAP(19);
		CREATE_CELL_MAP(20);
		CREATE_CELL_MAP(21);
		CREATE_CELL_MAP(22);
		CREATE_CELL_MAP(23);
		CREATE_CELL_MAP(24);
		CREATE_CELL_MAP(25);
		CREATE_CELL_MAP(26);
		CREATE_CELL_MAP(27);
		CREATE_CELL_MAP(28);
		CREATE_CELL_MAP(29);
		CREATE_CELL_MAP(30);
		CREATE_CELL_MAP(31);
		CREATE_CELL_MAP(32);
		CREATE_DEFAULT_MAP();
	}
	return pmap;
}

#undef CREATE_CELL_MAP
#undef CREATE_DEFAULT_MAP

}

#endif
