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

#ifndef GENERIC_CELL_H
#define GENERIC_CELL_H

#include <string>
#include <exception>
#include <stddef.h>

#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Exception/PaloServerException.h>

#include "IntArray.h"
#include "StringArray.h"
#include "DimensionElementType.h"
#include "ElementList.h"
#include "ElementListArray.h"
#include "ConsolidationElement.h"
#include "ConsolidationElementArray.h"
#include "DimensionElementInfoArray.h"
#include "CubeInfo.h"
#include "CellValue.h"
#include "DatabaseInfo.h"
#include "DimensionInfo.h"
#include "UserInfo.h"
#include "CellValueArray.h"
#include "GenericCellIterator.h"

#include "XLError.h"
#include "GenericArrayBuilder.h"
#include "ServerInfo.h"
#include "LicenseInfo.h"
#include "RuleInfo.h"
#include "LockInfo.h"
#include "SubsetResult.h"

namespace Palo {
namespace SpreadsheetFuncs {
using namespace Types;

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief GenericCell class.
 *
 *  Provides an interface to access and store values.
 */
class GenericCell {
public:
	/*! \brief Simple typedef.
	 */
	typedef GenericCellIterator Iterator;

	/*! \brief Simple typedef.
	 */
	typedef GenericArrayBuilder ArrayBuilder;

	virtual ~GenericCell()
	{
	}

	/*! \brief Possible types of a GenericCell.
	 */
	enum Type {
		TString, TInt, TDouble, TConnection, TArray, TMatrix, TNull, TCellValue, TBool, TOther
	};

	/*! \brief Get the cells type.
	 */
	virtual Type getType() = 0;

	/*! \brief Don't pad arrays (default impl does nothing). */
	virtual GenericCell& supressPadding();

	/*! \brief Check whether the cell contents can be interpreted as a number.
	 */
	bool isNumber()
	{
		Type t = getType();
		return (t == TInt) || (t == TDouble);
	}

	/*! \brief Check whether the cell contents can be interpreted as a boolean.
	 */
	bool isBool()
	{
		Type t = getType();
		return t == TBool;
	}

	/*! \brief Argument not passed?
	 *
	 *  Check if the cell represents a function argument which
	 *  has been omitted in the function call.
	 */
	virtual bool isMissing() = 0;

	/*! \brief Argument empty?
	 *
	 *  Check if the argument is empty, e.g. contains an empty string.
	 */
	virtual bool empty(bool allelements = true) = 0;

	/*! \brief Set the argument to empty.
	 */
	virtual GenericCell& setEmpty() = 0;

	/*! \brief Retrieve an array.
	 *
	 *  Arrays are one-dimensional.
	 */
	virtual Iterator getArray() = 0;

	/*! \brief Retrieve a matrix.
	 *
	 *  Matrices are two-dimensional arrays. Your implementation should set rows and cols
	 *  to correct values.
	 */
	virtual Iterator getMatrix(size_t& rows, size_t& cols);

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual StringArray getStringArray(bool suppressEmpty = false);

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual IntArray getIntArray(bool suppressEmpty = false);

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual StringArrayArray getStringArrayArray(bool suppressEmpty = false);

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual boost::shared_ptr<jedox::palo::Server> getConnection();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual std::string getString() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual long int getSLong() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual unsigned long int getULong() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual unsigned int getUInt() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual int getSInt() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual bool getBool() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual double getDouble() = 0;

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual DimensionElementType getDimensionElementType();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual ElementList getElementList();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual ElementListArray getElementListArray();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual ConsolidationElement getConsolidationElement();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual ConsolidationElementArray getConsolidationElementArray();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual CellValue getCellValue();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual CellValueArray getCellValueArray();

	/*! \brief Retrieve an argument.
	 *
	 *  What the name suggests.
	 */
	virtual jedox::palo::SPLASH_MODE getSplashMode();

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(boost::shared_ptr<jedox::palo::Server> s) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(int i) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(unsigned int i) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(long int i) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(unsigned long int i) = 0;

#ifdef WIN64
	/*! \brief Set a value.
	 *
	 *  What the name suggests (this is required for Windows x64).
	 */
	virtual GenericCell& set( size_t n );
#endif

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(double d) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(long double d) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const std::string& s) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const char *s);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementType& t);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(bool b) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ParentElementInfo& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ChildElementInfo& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementInfoReduced& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementInfo& ei) = 0;

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementInfoPerm& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementInfoSimple& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ConsolidationElementInfo& ei);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ElementList& el);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const CellValue& v, bool set_error_desc = true);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const CellValueWithProperties& v, const std::vector<std::string> &properties, bool set_error_desc = true);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const CubeInfo& ci);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const RuleInfo& ri);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const LicenseInfo& ri);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ServerInfo& si);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const SubsetResult& sr);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const SubsetResults& srs);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ParentElementInfoArray& pia);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ChildElementInfoArray& cia);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionElementInfoSimpleArray& disa);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const ConsolidationElementInfoArray& disa);
	virtual GenericCell& set(const jedox::palo::AxisElement &ae, jedox::palo::AxisElement::MEMBERS selector);
	virtual GenericCell& set(const std::vector<jedox::palo::AxisElement> &ae, jedox::palo::AxisElement::MEMBERS selector);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DatabaseInfo& di);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const DimensionInfo& di);

	/*! \brief Set a value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& set(const UserInfo& ui);
	virtual GenericCell& set(const LockInfo& li);

	/*! \brief Set an error value.
	 *
	 *  What the name suggests. Implementation should call GenericContext::setError()
	 */
	virtual GenericCell& setError(const ErrorInfo& ei, bool set_error_desc = true) = 0;

	/*! \brief Set an error value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& setError(const jedox::palo::PaloException& e);

	/*! \brief Set an error value.
	 *
	 *  What the name suggests.
	 */
	virtual GenericCell& setNull() = 0;

	/*! \brief Set an array.
	 *
	 *  What the name suggests.
	 */
	virtual ArrayBuilder setArray(size_t length, bool pad = true) = 0;

	/*! \brief Set a matrix.
	 *
	 *  What the name suggests.
	 */
	virtual ArrayBuilder setMatrix(size_t rows, size_t cols);

	/*! \brief Set a matrix.
	 *
	 *  What the name suggests.
	 */
	virtual ArrayBuilder setMatrix(size_t rows, size_t cols, bool pad);

	/*! \brief Check if cell contains an error. Default implementation returns false.
	 *
	 *  What the name suggests. Only applied if error values are allowed (Documentation.xml)
	 */
	virtual bool isError();

	/*!
	 * \brief
	 * clone the current object.
	 *
	 * \author
	 * Florian Schaper <florian.schaper@jedox.com>
	 */
	virtual std::unique_ptr<GenericCell> clone() const = 0;

	/*!
	 * \brief
	 * create an object of the current type.
	 *
	 * \author
	 * Florian Schaper <florian.schaper@jedox.com>
	 */
	virtual std::unique_ptr<GenericCell> create() const = 0;

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the cells value to an array of T's.
	 */
	template<typename T> GenericCell& set(const std::vector<T>& a)
	{
		GenericArrayBuilder da = setArray(a.size());

		for (typename std::vector<T>::const_iterator i = a.begin(); i != a.end(); i++)
			da.append(da.createGenericCell()->set(*i));

		return *this;
	}

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the cells value to an array of T's.
	 */
	template<typename T> GenericCell& set(const std::list<T>& a)
	{
		GenericArrayBuilder da = setArray(a.size());

		for (typename std::vector<T>::const_iterator i = a.begin(); i != a.end(); i++)
			da.append(da.createGenericCell()->set(*i));

		return *this;
	}

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the char value
	 */
	GenericCell& set(const char c)
	{
		std::stringstream ss;
		std::string s;
		ss << c;
		ss >> s;
		return set(s);
	}

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the cells value to an map of T's.
	 */
	template<typename T2> GenericCell& set(const std::map<std::string, T2>& m)
	{
		ArrayBuilder a = setArray(m.size());

		for (typename std::map<std::string, T2>::const_iterator it = m.begin(); it != m.end(); ++it) {
			a.append(it->first, a.createGenericCell()->set(it->second));
		}

		return *this;
	}

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the cells value to an array of T's.
	 */
	template<typename T> GenericCell& set(const std::vector<T*>& a)
	{
		GenericArrayBuilder da = setArray(a.size());

		for (typename std::vector<T*>::const_iterator i = a.begin(); i != a.end(); i++)
			da.append(da.createGenericCell()->set(**i));

		return *this;
	}

	/*! \brief Set a generic value.
	 *
	 *  This template function is used to set the cells value to an array of T's.
	 */
	template<typename T> GenericCell& set(const std::list<T*>& a)
	{
		GenericArrayBuilder da = setArray(a.size());

		for (typename std::vector<T*>::const_iterator i = a.begin(); i != a.end(); i++)
			da.append(da.createGenericCell()->set(**i));

		return *this;
	}

	/*! \brief Used to get guess and get a values type. */
	template<typename T>
	T get();

	/*! \brief Used to get guess and get a values type */
	template<typename T>
	operator T()
	{
		return get<T>();
	}

protected:
	/*! \brief You have to define this somewhere!!! */
	static const std::string ELEMENTS_ALL;

	/*! \brief Set properties values.
	 *
	 *  What the name suggests.
	 */
	virtual void setPropValues(const std::vector<std::string>& PropValues, const std::vector<std::string> &properties)
	{
	}
	;

private:
	inline void setHelper(const CellValue& v, bool set_error_desc = true);
};
}
}
#endif
