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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef SIMPLE_CELL_H
#define SIMPLE_CELL_H

#include <boost/ptr_container/ptr_vector.hpp>

#include <PaloSpreadsheetFuncs/GenericCell.h>

namespace Palo {
namespace SpreadsheetFuncs {
namespace SimpleCell {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Dummy cell.
 *
 *  This class provides a dummy implementation of GenericCell. Calling a member
 *  will result in an InvalidRequestException.
 */
class DummyCell : public GenericCell {
public:
	virtual ~DummyCell();

	virtual Type getType() = 0;

	virtual bool isMissing();

	virtual bool empty();
	virtual GenericCell& setEmpty();

	virtual Iterator getArray();
	virtual boost::shared_ptr<jedox::palo::Server> getConnection(std::string& database);
	virtual std::string getString();
	virtual unsigned int getUInt();
	virtual int getSInt();
	virtual unsigned long int getULong();
	virtual long int getSLong();
	virtual bool getBool();
	virtual double getDouble();

	virtual GenericCell& set(boost::shared_ptr<jedox::palo::Server> s);
	virtual GenericCell& set(int i);
	virtual GenericCell& set(unsigned int i);
	virtual GenericCell& set(long int i);
	virtual GenericCell& set(unsigned long int i);
	virtual GenericCell& set(double d);
	virtual GenericCell& set(long double d);
	virtual GenericCell& set(const std::string& s);
	virtual GenericCell& set(bool b);
	virtual GenericCell& set(const DimensionElementInfo& ei);
	virtual GenericCell& setError(const ErrorInfo& ei, bool set_error_desc = true);
	virtual GenericCell& setNull();

	virtual ArrayBuilder setArray(size_t length, bool pad = true);
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief String cell.
 *
 *  This class provides an implementation of GenericCell which supports strings only.
 */
class StringCell : public DummyCell {
public:
	StringCell(const std::string& s);
	~StringCell();

	std::string getString();
	bool empty(bool allelements = true);

	Type getType();

	virtual std::unique_ptr<GenericCell> clone() const;
	virtual std::unique_ptr<GenericCell> create() const;

private:
	std::string s;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Missing cell.
 *
 *  Used to represent missing arguments.
 */
class MissingCell : public DummyCell {
public:
	MissingCell();
	~MissingCell();

	bool empty(bool allelements = true);
	bool isMissing();
	Type getType();

	virtual std::unique_ptr<GenericCell> clone() const;
	virtual std::unique_ptr<GenericCell> create() const;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Connection cell.
 *
 *  This class provides an implementation of GenericCell which supports connections only.
 */
class ConnectionCell : public DummyCell {
public:
	ConnectionCell(boost::shared_ptr<jedox::palo::Server> s);
	~ConnectionCell();

	boost::shared_ptr<jedox::palo::Server> getConnection(std::string& database);
	bool empty(bool allelements = true);
	Type getType();

	virtual std::unique_ptr<GenericCell> clone() const;
	virtual std::unique_ptr<GenericCell> create() const;

private:
	boost::shared_ptr<jedox::palo::Server> s;
};

class ArrayCell;

class ArrayCellIteratorImpl : public GenericCellIteratorImpl {
public:
	ArrayCellIteratorImpl(ArrayCell& a);
	~ArrayCellIteratorImpl();

	GenericCell* operator ->();
	GenericCell* release();
	GenericCellIteratorImpl& operator ++();
	bool end() const;
	size_t minRemaining() const;

private:
	ArrayCell& a;
	size_t pos;
};

/*!
 * \brief
 * clone allocator for the ArrayCell ptr_vector.
 *
 * \author
 * Florian Schaper <florian.schaper@jedox.com>
 */
struct GenericCellCloneAllocator {
	template<class U>
	static U* allocate_clone(const U& r)
	{
		return r.clone().release();
	}

	template<class U>
	static void deallocate_clone(const U* r)
	{
		delete r;
	}
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Array cell.
 *
 *  This class provides an implementation of GenericCell which can represent an
 *  array by storing pointers to GenericCell objects.
 */
class ArrayCell : public DummyCell {
public:
	ArrayCell(size_t reserved_size = 0);
	~ArrayCell();

	/*! \warning ~ArrayCell() will call delete on gc */
	void append(GenericCell* gc);

	Iterator getArray();
	bool empty(bool allements = true);
	bool isMissing();
	Type getType();
	virtual std::unique_ptr<GenericCell> clone() const;
	virtual std::unique_ptr<GenericCell> create() const;

	friend class ArrayCellIteratorImpl;

private:
	boost::ptr_vector<GenericCell, GenericCellCloneAllocator> a;
};
}
}
}
#endif
