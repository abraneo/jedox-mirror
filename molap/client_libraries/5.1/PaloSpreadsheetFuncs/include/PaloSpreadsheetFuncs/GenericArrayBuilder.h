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

#ifndef GENERIC_ARRAY_BUILDER_H
#define GENERIC_ARRAY_BUILDER_H

#include <memory>
#include <string>

#include "GenericArrayBuilder.h"

namespace Palo {
namespace SpreadsheetFuncs {
class GenericCell;

class GenericArrayBuilder;

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief What the name suggests.
 *
 *  Required by GenericCell. Used by GenericArrayBuilder.
 *  Provides reference count management.
 *  If you want to implement the interface provided by this class
 *  you must at least provide implementations for all documented members
 *  and the destructor.
 */
class GenericArrayBuilderImpl {
	unsigned int ref_count;

public:
	GenericArrayBuilderImpl() :
			ref_count(0)
	{
	}

	virtual ~GenericArrayBuilderImpl()
	{
	}

	/*! \brief "Instantiate" a GenericCell.
	 *
	 *  This function creates an instance of a class derived from GenericCell and
	 *  returns a reference to it.
	 */
	virtual std::unique_ptr<GenericCell> createGenericCell() = 0;

	/*! \brief Append a GenericCell to the array.
	 *
	 *  Append the referenced class derived from GenericCell to the array.
	 */
	virtual void append(GenericCell& v) = 0;

	/*! \brief Append a GenericCell to the array.
	 *
	 *  Append the referenced object of a class derived from GenericCell to the array.
	 *  This function allows you to provide a key for the array entry.
	 */
	virtual void append(const std::string key, GenericCell& v) = 0;

	/*! \brief transposes result
	 *
	 *  Default implementation ignores call.
	 */
	virtual void transpose()
	{
	}

	void addRef()
	{
		ref_count++;
	}

	void delRef()
	{
		if (ref_count-- <= 1)
			delete this;
	}
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief What the name suggests.
 *
 *  Provides an interface to create an array of objects of a class derived
 *  from GenericCell. You need to provide an implementation of GenericArrayBuilderImpl.
 */
class GenericArrayBuilder {
	GenericArrayBuilderImpl* real_builder;

public:
	GenericArrayBuilder(GenericArrayBuilderImpl* builder) :
			real_builder(builder)
	{
		real_builder->addRef();
	}

	GenericArrayBuilder(const GenericArrayBuilder& other) :
			real_builder(other.real_builder)
	{
		real_builder->addRef();
	}

	~GenericArrayBuilder()
	{
		real_builder->delRef();
	}

	std::unique_ptr<GenericCell> createGenericCell()
	{
		return real_builder->createGenericCell();
	}

	void append(GenericCell& v)
	{
		real_builder->append(v);
	}

	void append(const std::string key, GenericCell& v)
	{
		real_builder->append(key, v);
	}

	void transpose()
	{
		real_builder->transpose();
	}
};
}
}
#endif
