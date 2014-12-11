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

#ifndef ZEND_VALUE_H
#define ZEND_VALUE_H

#ifdef __cplusplus

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <PaloSpreadsheetFuncs/GenericCell.h>
#include <PaloSpreadsheetFuncs/GenericContext.h>
#include <PaloSpreadsheetFuncs/ConvertingCell.h>

#include "ZendValueIteratorImpl.h"
#include "ZendArrayBuilderImpl.h"

#include <php_version.h>
#include <zend.h>
#include "fix_zend.h"

namespace Palo {
namespace PHPPalo {
using namespace SpreadsheetFuncs;
using namespace Types;

class ZendValue : public ConvertingCell {
	bool _p_init;
	zval* _p;

	zval*& p;
	bool modified;

	void check_unset();
	void set_set();

public:
	explicit ZendValue(GenericContext& settings);
	explicit ZendValue(GenericContext& settings, zval*& z);
	virtual ~ZendValue();

	friend void ZendArrayBuilderImpl::append(GenericCell& v);
	friend void ZendArrayBuilderImpl::append(const std::string key, GenericCell& v);friend ZendValueIteratorImpl::ZendValueIteratorImpl( GenericContext& settings, ZendValue* v );

	Type getType();

	bool isMissing();

	bool empty(bool allelements = true);
	GenericCell& setEmpty();

	Iterator getArray();
	boost::shared_ptr<jedox::palo::Server> getConnection();
	std::string getStringImpl();
	long int getSLong();
	unsigned long int getULong();
	unsigned int getUInt();
	int getSInt();
	bool getBool();
	double getDouble();

	/*!
	 * \brief
	 * clone the current object.
	 *
	 * \author
	 * Florian Schaper <florian.schaper@jedox.com>
	 */
	std::unique_ptr<GenericCell> clone() const;

	/*!
	 * \brief
	 * create an object of the current type.
	 *
	 * \author
	 * Florian Schaper <florian.schaper@jedox.com>
	 */
	std::unique_ptr<GenericCell> create() const;

	GenericCell& set(boost::shared_ptr<jedox::palo::Server> s);
	GenericCell& set(int i);
	GenericCell& set(unsigned int i);
	GenericCell& set(long int i);
	GenericCell& set(unsigned long int i);
	GenericCell& set(double d);
	GenericCell& set(long double d);
	GenericCell& setImpl(const std::string& s);
	GenericCell& set(bool b);
	GenericCell& set(const DimensionElementInfoReduced& ei);
	GenericCell& set(const DimensionElementInfo& ei);
	GenericCell& setError(const ErrorInfo& ei, bool set_error_desc = true);
	GenericCell& setNull();

	/* \brief see parent
	 *
	 * pad is being ignored
	 */
	GenericArrayBuilder setArray(size_t length, bool pad = false);

private:
	template<class ElementInfoType>
	GenericCell& setHelper(const ElementInfoType& ei);

public:
	using GenericCell::ELEMENTS_ALL;
};
}
}

extern "C" {
#endif
const char *get_ELEMENTS_ALL();
#ifdef __cplusplus
}
#endif
#endif
