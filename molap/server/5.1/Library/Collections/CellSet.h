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

#ifndef CELLSET_H
#define CELLSET_H

#include "palo.h"
#include "CellMap.h"

using namespace palo;

namespace palo {

class CellSetBase : public boost::enable_shared_from_this<CellSetBase> {
public:
	virtual ~CellSetBase() {}
};

class ICellSet : public CellSetBase {
public:
	virtual ~ICellSet() {};
	virtual bool set(const IdentifiersType &path) = 0;
	virtual size_t size() const = 0;
	virtual PCellStream getValues() = 0;
};

class CellVectorSet : public ICellSet, public std::set<IdentifiersType> {
public:
	typedef std::set<IdentifiersType> SetType;

	virtual ~CellVectorSet() {}

	virtual bool set(const IdentifiersType &path) {
		std::pair<SetType::iterator, bool> ret = this->insert(path);
		return ret.second;
	}

	virtual size_t size() const {
		return SetType::size();
	}

	class Reader : public CellValueStream {
	public:
		Reader(CellVectorSet &cvs) : pcvs(cvs.shared_from_this()), cvs(cvs), iter(cvs.begin()) {}
		virtual ~Reader() {}
		virtual bool next() {
			if (iter != cvs.end()) {
				vkey = &*iter;
				++iter;
				return true;
			} else {
				return false;
			}
		}
		virtual const CellValue &getValue() {throw ErrorException(ErrorException::ERROR_INTERNAL, "CellVectorSet::Reader::getValue not supported!");}
		virtual double getDouble() {throw ErrorException(ErrorException::ERROR_INTERNAL, "CellVectorSet::Reader::getDouble not supported!");}
		virtual const IdentifiersType &getKey() const {
			return *vkey;
		}
		virtual void reset() {
			iter = cvs.begin();
		}
	private:
		boost::shared_ptr<CellSetBase> pcvs;
		CellVectorSet &cvs;
		SetType::const_iterator iter;
		const IdentifiersType *vkey;
	};

	virtual PCellStream getValues() {
		return PCellStream(new Reader(*this));
	}
};

template<int _N>
class CellArraySet : public ICellSet, public std::set<IdsNType<_N> > {
	typedef typename std::set<IdsNType<_N> >::const_iterator const_iterator;
	typedef typename std::set<IdsNType<_N> >::iterator iterator;

public:
	virtual ~CellArraySet() {}

	virtual bool set(const IdentifiersType &path) {
		pair<iterator, bool> ret = this->insert(IdsNType<_N>(&path[0]));
		return ret.second;
	}

	virtual size_t size() const {
		return std::set<IdsNType<_N> >::size();
	}

	class Reader : public CellValueStream {
	public:
		Reader(CellArraySet &cas) : pcas(cas.shared_from_this()), cas(cas), iter(cas.begin()), vkey(_N) {}
		virtual ~Reader() {}
		virtual bool next() {
			if (iter != cas.end()) {
				memcpy(&vkey[0], &*iter, sizeof(IdsNType<_N>));
				++iter;
				return true;
			} else return false;
		}
		virtual const CellValue &getValue() {throw ErrorException(ErrorException::ERROR_INTERNAL, "CellArraySet::Reader::getValue not supported!");}
		virtual double getDouble() {throw ErrorException(ErrorException::ERROR_INTERNAL, "CellArraySet::Reader::getDouble not supported!");}
		virtual const IdentifiersType &getKey() const {
			return vkey;
		}
		virtual void reset() {
			iter = cas.begin();
		}
	private:
		boost::shared_ptr<CellSetBase> pcas;
		CellArraySet<_N> &cas;
		typename CellArraySet<_N>::iterator iter;
		IdentifiersType vkey;
	};

	virtual PCellStream getValues() {
		return PCellStream(new Reader(*this));
	}
};

#define CREATE_CELL_SET(DIMENSIONS) case (DIMENSIONS): pset.reset(new CellArraySet<(DIMENSIONS)>()); break;
#define CREATE_DEFAULT_SET() default: pset.reset(new CellVectorSet()); break;

inline boost::shared_ptr<ICellSet> CreateCellSet(size_t dimensions)
{
	boost::shared_ptr<ICellSet> pset;

	switch (dimensions) {
		CREATE_CELL_SET(1);
		CREATE_CELL_SET(2);
		CREATE_CELL_SET(3);
		CREATE_CELL_SET(4);
		CREATE_CELL_SET(5);
		CREATE_CELL_SET(6);
		CREATE_CELL_SET(7);
		CREATE_CELL_SET(8);
		CREATE_CELL_SET(9);
		CREATE_CELL_SET(10);
		CREATE_CELL_SET(11);
		CREATE_CELL_SET(12);
		CREATE_CELL_SET(13);
		CREATE_CELL_SET(14);
		CREATE_CELL_SET(15);
		CREATE_CELL_SET(16);
		CREATE_CELL_SET(17);
		CREATE_CELL_SET(18);
		CREATE_CELL_SET(19);
		CREATE_CELL_SET(20);
		CREATE_CELL_SET(21);
		CREATE_CELL_SET(22);
		CREATE_CELL_SET(23);
		CREATE_CELL_SET(24);
		CREATE_CELL_SET(25);
		CREATE_CELL_SET(26);
		CREATE_CELL_SET(27);
		CREATE_CELL_SET(28);
		CREATE_CELL_SET(29);
		CREATE_CELL_SET(30);
		CREATE_CELL_SET(31);
		CREATE_CELL_SET(32);
		CREATE_DEFAULT_SET();
	}
	return pset;
};

#undef CREATE_CELL_SET
#undef CREATE_DEFAULT_SET

}

#endif
