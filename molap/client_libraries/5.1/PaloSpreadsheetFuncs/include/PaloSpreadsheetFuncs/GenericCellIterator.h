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

#ifndef GENERIC_CELL_ITERATOR_H
#define GENERIC_CELL_ITERATOR_H

#include <string>

namespace Palo {
namespace SpreadsheetFuncs {
class GenericCell;

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief GenericCell iterator implementation interface.
 *
 *  Provides an interface to access arrays of GenericCells. The class implements
 *  reference-count management. It is used by GenericCellIterator and therefore
 *  required by GenericCell.
 */
class GenericCellIteratorImpl {
	unsigned int ref_count;

public:
	GenericCellIteratorImpl() :
			ref_count(0)
	{
	}

	virtual ~GenericCellIteratorImpl()
	{
	}

	/*! \brief Dereference operator.
	 *
	 *  Retrieve a pointer to the GenericCell the iterator is currently pointing to.
	 */
	virtual GenericCell* operator ->() = 0;

	void addRef()
	{
		ref_count++;
	}

	void delRef()
	{
		if (ref_count-- <= 1)
			delete this;
	}

	/*! \brief Increment operator (prefix)
	 *
	 *  Move one step forward (you have to assure that there are elements left!).
	 */
	virtual GenericCellIteratorImpl& operator ++() = 0;

	/*! \brief Release pointer.
	 *
	 *  Return a pointer to a GenericCell that remains valid after the iterator
	 *  modification. You should call delete on the return value when you don't need it
	 *  anymore.
	 */
	virtual GenericCell* release() = 0;

	/*! \brief End reached?
	 */
	virtual bool end() const = 0;

	/*! \brief Minimal number of elements remaining.
	 *
	 *  Tell how many element are left at least.
	 */
	virtual size_t minRemaining() const = 0;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief GenericCell iterator interface.
 *
 *  Access arrays of GenericCells. Used by GenericCell.
 *  You need to provide an implementation of GenericCellIteratorImpl.
 */
class GenericCellIterator {
	GenericCellIteratorImpl* real_iter;

public:
	GenericCellIterator(GenericCellIteratorImpl* iter) :
			real_iter(iter)
	{
		real_iter->addRef();
	}

	GenericCellIterator(const GenericCellIterator& other) :
			real_iter(other.real_iter)
	{
		real_iter->addRef();
	}

	~GenericCellIterator()
	{
		real_iter->delRef();
	}

	GenericCell* operator ->()
	{
		return real_iter->operator ->();
	}

	GenericCell* release()
	{
		return real_iter->release();
	}

	GenericCell& operator *()
	{
		return *real_iter->operator ->();
	}

	GenericCellIterator operator ++()
	{
		++(*real_iter);

		return GenericCellIterator(real_iter);
	}

	bool end() const
	{
		return real_iter->end();
	}

	size_t minRemaining() const
	{
		return real_iter->minRemaining();
	}
};
}
}
#endif
