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

#include "ZendValue.h"
#include "ZendValueException.h"

#include "php_jedox_palo.h"

#include <limits.h>

#include <php.h>

// for php 5.3.0 and above
#if PHP_VERSION_ID >= 50300
#	define ZVAL_DELREF(pz)	Z_DELREF_P(pz)
#	define ZVAL_REFCOUNT(pz)	Z_REFCOUNT_P(pz)
#endif 

using namespace Palo::Util;
using namespace Palo::PHPPalo;
using namespace std;

bool ZendValue::isMissing()
{
	return (p == NULL || getType() == TNull);
}

bool ZendValue::empty(bool allelements)
{
	switch (Z_TYPE_P( p )) {
	case IS_NULL:
		return true;
	case IS_ARRAY:
		for (Iterator i = getArray(); !i.end(); ++i) {
			if (allelements != i->empty(allelements))
				return !allelements;
		}
		return allelements;
	case IS_STRING:
		return getString().length() == 0;
	case IS_LONG:
	case IS_DOUBLE:
	case IS_BOOL:
	case IS_OBJECT:
	case IS_RESOURCE:
	case IS_CONSTANT:
	default:
		return false;
	}
}

GenericCell& ZendValue::setEmpty()
{
	return ConvertingCell::set(std::string(""));
}

void ZendValue::check_unset()
{
	if (modified)
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZENDVALUE_INTERNAL_ERROR);
}

void ZendValue::set_set()
{
	modified = true;
}

ZendValue::ZendValue(GenericContext& settings) :
		ConvertingCell(settings), p(_p), modified(false)
{
	MAKE_STD_ZVAL( p);
	_p_init = true;
}

std::unique_ptr<GenericCell> ZendValue::create() const
{
	return std::unique_ptr < GenericCell > (new ZendValue(this->settings));
}

std::unique_ptr<GenericCell> ZendValue::clone() const
{
	return std::unique_ptr < GenericCell > (new ZendValue(*this));
}

ZendValue::ZendValue(GenericContext& settings, zval*& z) :
		ConvertingCell(settings), _p_init(false), p(z), modified(false)
{
}

ZendValue::~ZendValue()
{
	if (_p_init) {
		ZVAL_DELREF(p);
		// decrease refCount, but memory stays allocated
		if (ZVAL_REFCOUNT(p) == 0) {
			TSRMLS_FETCH();zval_dtor(p);
			// free content (string)
			FREE_ZVAL(p);
			// free zval
		}
	}
}

GenericCell::Type ZendValue::getType()
{
	switch (Z_TYPE_P( p )) {
	case IS_STRING:
		return TString;
	case IS_LONG:
		return TInt;
	case IS_DOUBLE:
		return TDouble;
	case IS_ARRAY:
		return TArray;
	case IS_RESOURCE:
		return TConnection;
	case IS_NULL:
		return TNull;
	case IS_BOOL:
		return TBool;
	default:
		return TOther;
	}
}

GenericCell::Iterator ZendValue::getArray()
{
	if (Z_TYPE_P( p ) != IS_ARRAY) {
		check_unset();
		convert_to_array_ex( &p);
		set_set();
	}

	return GenericCell::Iterator(new ZendValueIteratorImpl(settings, this));
}

boost::shared_ptr<jedox::palo::Server> ZendValue::getConnection()
{
	using namespace jedox::palo;

	if (Z_TYPE_P( p ) == IS_RESOURCE) {
		TSRMLS_FETCH();
		boost::shared_ptr<jedox::palo::Server> s = *(boost::shared_ptr<jedox::palo::Server>*)zend_fetch_resource(&p TSRMLS_CC, -1, (char *)CONNECTION_RESOURCE_NAME, NULL, 1, le_connection_resource);

		if (s == NULL)
			throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZENDVALUE_INVALID_CONNECTION);
		else
			return s;
	} else {
		return GenericCell::getConnection();
	}
}

std::string ZendValue::getStringImpl()
{
	if (Z_TYPE_P( p ) != IS_STRING) {
		check_unset();
		convert_to_string_ex( &p);
		set_set();
	}

	return std::string(Z_STRVAL_P( p ));
}

unsigned int ZendValue::getUInt()
{
	return (unsigned int)getSLong();
}

int ZendValue::getSInt()
{
	return (int)getSLong();
}

unsigned long int ZendValue::getULong()
{
	return (unsigned long int)getSLong();
}

long int ZendValue::getSLong()
{
	if (Z_TYPE_P( p ) != IS_LONG) {
		check_unset();
		convert_to_long_ex( &p);
		set_set();
	}

	return Z_LVAL_P( p );
}

bool ZendValue::getBool()
{
	if (Z_TYPE_P( p ) != IS_BOOL) {
		check_unset();
		convert_to_boolean_ex( &p);
		set_set();
	}

	return Z_BVAL_P( p ) != 0;
}

double ZendValue::getDouble()
{
	if (Z_TYPE_P( p ) != IS_DOUBLE) {
		check_unset();
		convert_to_double_ex( &p);
		set_set();
	}

	return Z_DVAL_P( p );
}

GenericCell& ZendValue::set(boost::shared_ptr<jedox::palo::Server> s)
{
	check_unset();
	if (s == NULL) {
		TSRMLS_FETCH();zend_hash_index_del( &EG( regular_list ), Z_RESVAL_P( p ));
		ZVAL_NULL( p);
	} else {
		// for php 5.4.0 and above
#if PHP_VERSION_ID >= 50400
		TSRMLS_FETCH();
#endif

		boost::shared_ptr<jedox::palo::Server>* sp = new boost::shared_ptr<jedox::palo::Server>(s);
		ZEND_REGISTER_RESOURCE( p, ( void* )sp, le_connection_resource);
	}
	set_set();

	return *this;
}

GenericCell& ZendValue::set(int i)
{
	return set((long int)i);
}

GenericCell& ZendValue::set(unsigned int i)
{
	return set((unsigned long int)i);
}

GenericCell& ZendValue::set(long int i)
{
	check_unset();
	ZVAL_LONG( p, i);
	set_set();

	return *this;
}

GenericCell& ZendValue::set(unsigned long int i)
{
	if (i > LONG_MAX)
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_VALUE_INT_WOULD_TRUNCATE);

	return set((long int)i);
}

GenericCell& ZendValue::set(double d)
{
	check_unset();
	ZVAL_DOUBLE( p, d);
	set_set();

	return *this;
}

GenericCell& ZendValue::set(long double d)
{
	return set((double)d);
}

GenericCell& ZendValue::setImpl(const std::string& s)
{
	check_unset();
	ZVAL_STRING( p, ( char* )s.c_str(), 1);
	set_set();

	return *this;
}

GenericCell& ZendValue::set(bool b)
{
	check_unset();
	ZVAL_BOOL( p, ( int )b);
	set_set();

	return *this;
}

GenericCell::ArrayBuilder ZendValue::setArray(size_t length, bool pad)
{
	check_unset();
	array_init( p);
	set_set();

	return GenericCell::ArrayBuilder(new ZendArrayBuilderImpl(settings, p));
}

GenericCell& ZendValue::setError(const ErrorInfo& ei, bool set_error_desc)
{
	settings.setError(ei);

	return ConvertingCell::set(XLError::getString(ei.errcode));
}

GenericCell& ZendValue::setNull()
{
	check_unset();
	ZVAL_NULL( p);
	set_set();

	return *this;
}

template<class ElementInfoType>
GenericCell& ZendValue::setHelper(const ElementInfoType& ei)
{
	GenericCell::ArrayBuilder dest = setArray(12);

	dest.append("identifier", dest.createGenericCell()->set((int)ei.identifier));
	dest.append("name", dest.createGenericCell()->set(ei.name));
	dest.append("type", dest.createGenericCell()->set(ei.type));
	dest.append("level", dest.createGenericCell()->set(ei.level));
	dest.append("indent", dest.createGenericCell()->set(ei.indent));
	dest.append("depth", dest.createGenericCell()->set(ei.depth));
	dest.append("position", dest.createGenericCell()->set(ei.position));
	dest.append("num_children", dest.createGenericCell()->set(ei.children.size()));
	dest.append("num_parents", dest.createGenericCell()->set(ei.parents.size()));
	dest.append("children", dest.createGenericCell()->set(ei.children));
	dest.append("parents", dest.createGenericCell()->set(ei.parents));

	return *this;
}

GenericCell& ZendValue::set(const DimensionElementInfoReduced& ei)
{
	return setHelper<DimensionElementInfoReduced>(ei);
}

GenericCell& ZendValue::set(const DimensionElementInfo& ei)
{
	return setHelper<DimensionElementInfo>(ei);
}

extern "C" const char *get_ELEMENTS_ALL()
{
	return ZendValue::ELEMENTS_ALL.c_str();
}

const std::string GenericCell::ELEMENTS_ALL = "*";
