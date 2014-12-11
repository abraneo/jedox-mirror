/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include <PaloSpreadsheetFuncs/GenericCell.h>
#include <PaloSpreadsheetFuncs/GenericCellException.h>
#include <libpalo_ng/Palo/ServerPool.h>

using namespace Palo::SpreadsheetFuncs;
using namespace Palo::Util;

GenericCell::Iterator GenericCell::getMatrix(size_t& rows, size_t& cols)
{
	Iterator i = getArray();

	rows = i->getUInt();
	++i;
	cols = i->getUInt();
	++i;

	return i;
}

StringArray GenericCell::getStringArray(bool suppressEmpty)
{
	StringArray sa;

	if (isMissing()) {
		return sa;
	}

	if (getType() != GenericCell::TArray) {
		sa.push_back(getString());
	} else {

		sa.reserve(getArray().minRemaining());

		for (GenericCell::Iterator i = getArray(); !i.end(); ++i) {
			if (suppressEmpty && (i->isMissing() || i->empty())) {
				continue;
			}
			sa.push_back(i->getString());
		}
	}

	return sa;
}

IntArray GenericCell::getIntArray(bool suppressEmpty)
{
	IntArray ia;

	if (isMissing()) {
		return ia;
	}

	if (getType() != GenericCell::TArray) {
		ia.push_back(getSInt());
	} else {

		ia.reserve(getArray().minRemaining());

		for (GenericCell::Iterator i = getArray(); !i.end(); ++i) {
			if (i->isMissing() || i->empty()) {
				continue;
			}
			ia.push_back(i->getSInt());
		}
	}

	return ia;
}

GenericCell& GenericCell::supressPadding()
{
	return *this;
}

StringArrayArray GenericCell::getStringArrayArray(bool suppressEmpty)
{
	StringArrayArray saa;

	saa.reserve(getArray().minRemaining());

	for (GenericCell::Iterator i = getArray(); !i.end(); ++i)
		saa.push_back(i->getStringArray(suppressEmpty));

	return saa;
}

DimensionElementType GenericCell::getDimensionElementType()
{
	const std::string s = jedox::util::UTF8Comparer::toUpper(getString());
	if (s == "N")
		return DimensionElementType(DimensionElementType::Numeric);
	else if (s == "C")
		return DimensionElementType(DimensionElementType::Consolidated);
	else if (s == "S")
		return DimensionElementType(DimensionElementType::String);
	else
		throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_DIMENSION_ELEMENT_TYPE);
}

ElementListArray GenericCell::getElementListArray()
{
	ElementListArray ela;

	ela.reserve(getArray().minRemaining());

	for (GenericCell::Iterator i = getArray(); !i.end(); ++i) {
		ela.push_back(i->getElementList());
	}

	return ela;
}

ConsolidationElement GenericCell::getConsolidationElement()
{
	ConsolidationElement ce;
	unsigned int cur_idx = 0;

	for (GenericCell::Iterator i = getArray(); !i.end(); ++i) {
		switch (cur_idx) {
		case 0:
			ce.name = i->getString();
			break;
		case 1:
			ce.weight = i->getDouble();
			break;
		}

		cur_idx++;
	}

	if (cur_idx != 2) {
		throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_CONSOLIDATION_ELEMENT_SPECIFICATION);
	}

	return ce;
}

ConsolidationElementArray GenericCell::getConsolidationElementArray()
{
	ConsolidationElementArray cea;

	cea.reserve(getArray().minRemaining());

	for (GenericCell::Iterator i = getArray(); !i.end(); ++i)
		cea.push_back(i->getConsolidationElement());

	return cea;
}

CellValue GenericCell::getCellValue()
{
	if (getType() == TNull) {
		return CellValue("");
	} else {
		if (getType() == TString) {
			return CellValue(getString());
		} else {
			return CellValue(getDouble());
		}
	}
}

bool GenericCell::isError()
{
	return false;
}

CellValueArray GenericCell::getCellValueArray()
{
	CellValueArray cva;

	cva.reserve(getArray().minRemaining());

	for (GenericCell::Iterator i = getArray(); !i.end(); ++i)
		cva.push_back(i->getCellValue());

	return cva;
}

jedox::palo::SPLASH_MODE GenericCell::getSplashMode()
{
    GenericCell::Type ctype = getType();

    switch(ctype) {
    case GenericCell::TString: {
            std::string s = jedox::util::UTF8Comparer::toUpper(getString());

            if (s == "SPLASH_MODE_NONE")
                return jedox::palo::MODE_SPLASH_NONE;
            else if (s == "SPLASH_MODE_DEFAULT")
                return jedox::palo::MODE_SPLASH_DEFAULT;
            else if (s == "SPLASH_MODE_SET")
                return jedox::palo::MODE_SPLASH_SET;
            else if (s == "SPLASH_MODE_ADD")
                return jedox::palo::MODE_SPLASH_ADD;
            else if (s == "TRUE")
                return jedox::palo::MODE_SPLASH_DEFAULT;
            else if (s == "FALSE")
                return jedox::palo::MODE_SPLASH_NONE;
            else
                throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_SPLASH_MODE_SPECIFICATION);
            break;
        }

    case GenericCell::TDouble:
    case GenericCell::TInt: {
            int i = 0;
			if (ctype == GenericCell::TDouble) {
				i = (int)getDouble();
			} else {
				i = getUInt();
			}
            switch (i) {
			case 0:
                return jedox::palo::MODE_SPLASH_NONE;
			case 1:
                return jedox::palo::MODE_SPLASH_DEFAULT;
			case 2:
                return jedox::palo::MODE_SPLASH_SET;
			case 3:
                return jedox::palo::MODE_SPLASH_ADD;
			default:
                throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_SPLASH_MODE_SPECIFICATION);
			}
        }

    default:
        return getBool() ? jedox::palo::MODE_SPLASH_DEFAULT : jedox::palo::MODE_SPLASH_NONE;

    }
}

GenericCell::ArrayBuilder GenericCell::setMatrix(size_t rows, size_t cols)
{
	ArrayBuilder a = setArray(rows * cols + 2);

	a.append(a.createGenericCell()->set(rows));
	a.append(a.createGenericCell()->set(cols));

	return a;
}

GenericCell::ArrayBuilder GenericCell::setMatrix(size_t rows, size_t cols, bool pad)
{
	return setMatrix(rows, cols);
}

GenericCell& GenericCell::set(const ParentElementInfo& ei)
{
	ArrayBuilder a = setArray(1);

	a.append("identifier", a.createGenericCell()->set(ei.identifier));

	return *this;
}

GenericCell& GenericCell::set(const ParentElementInfoArray& disa)
{
	return set < ParentElementInfo > (disa);
}

GenericCell& GenericCell::set(const ChildElementInfo& ei)
{
	ArrayBuilder a = setArray(2);

	a.append("identifier", a.createGenericCell()->set(ei.identifier));
	a.append("weight", a.createGenericCell()->set(ei.weight));

	return *this;
}

GenericCell& GenericCell::set(const ChildElementInfoArray& disa)
{
	return set < ChildElementInfo > (disa);
}

GenericCell& GenericCell::set(const DimensionElementInfoSimple& ei)
{
	ArrayBuilder a = setArray(3);

	a.append("name", a.createGenericCell()->set(ei.name));
	a.append("type", a.createGenericCell()->set(ei.type));
	a.append("identifier", a.createGenericCell()->set((int)ei.identifier));

	return *this;
}

GenericCell& GenericCell::set(const DimensionElementInfoSimpleArray& disa)
{
	return set < DimensionElementInfoSimple > (disa);
}

GenericCell& GenericCell::set(const ConsolidationElementInfo& ei)
{
	ArrayBuilder a = setArray(4);

	a.append("name", a.createGenericCell()->set(ei.name));
	a.append("type", a.createGenericCell()->set(ei.type));
	a.append("identifier", a.createGenericCell()->set(ei.identifier));
	a.append("weight", a.createGenericCell()->set(ei.weight));

	return *this;
}

GenericCell& GenericCell::set(const ConsolidationElementInfoArray& disa)
{
	return set < ConsolidationElementInfo > (disa);
}

GenericCell& GenericCell::set(const RuleInfo& ri)
{
	ArrayBuilder a = setArray(6);

	a.append("identifier", a.createGenericCell()->set(ri.identifier));
	a.append("definition", a.createGenericCell()->set(ri.definition));
	a.append("extern_id", a.createGenericCell()->set(ri.extern_id));
	a.append("comment", a.createGenericCell()->set(ri.comment));
	a.append("timestamp", a.createGenericCell()->set((unsigned long)ri.timestamp));
	a.append("activated", a.createGenericCell()->set(ri.activated));

	return *this;
}

GenericCell& GenericCell::set(const LicenseInfo& li)
{
	ArrayBuilder a = setArray(2);

	a.append("hw_key", a.createGenericCell()->set(li.hw_key));
	std::unique_ptr < GenericCell > in = a.createGenericCell();
	ArrayBuilder ina = in->setArray(li.licenses.size());
	for (std::vector<jedox::palo::LICENSE_INFO>::const_iterator it = li.licenses.begin(); it != li.licenses.end(); ++it) {
		std::unique_ptr < GenericCell > lic = ina.createGenericCell();
		ArrayBuilder lica = lic->setArray(li.licenses.size());
		lica.append("key", lica.createGenericCell()->set(it->key));
		lica.append("customer", lica.createGenericCell()->set(it->customer));
		lica.append("version", lica.createGenericCell()->set(it->version));
		lica.append("license_count", lica.createGenericCell()->set(it->license_count));
		lica.append("named_count", lica.createGenericCell()->set(it->named_count));
		char *time_string = ctime((time_t *)&it->start);
		lica.append("start", lica.createGenericCell()->set(time_string ? time_string : "Unlimited\n"));
		time_string = ctime((time_t *)&it->expiration);
		lica.append("expiration", lica.createGenericCell()->set(time_string ? time_string : "Unlimited\n"));
		lica.append("sharing_limit", lica.createGenericCell()->set(it->sharing_limit));
		lica.append("gpu_count", lica.createGenericCell()->set(it->gpu_count));
		lica.append("features", lica.createGenericCell()->set(it->features));
		ina.append(*lic);
	}
	a.append("licenses", *in);

	return *this;
}

GenericCell& GenericCell::set(const ServerInfo& si)
{
	ArrayBuilder a = setArray(7);

	a.append("major_version", a.createGenericCell()->set(si.major_version));
	a.append("minor_version", a.createGenericCell()->set(si.minor_version));
	a.append("bugfix_version", a.createGenericCell()->set(si.bugfix_version));
	a.append("build_number", a.createGenericCell()->set(si.build_number));
	a.append("encryption", a.createGenericCell()->set(si.encryption));
	a.append("https_port", a.createGenericCell()->set(si.httpsPort));
	a.append("data_sequence_number", a.createGenericCell()->set(si.data_sequence_number));
	a.append("sid", a.createGenericCell()->set(si.sid));
	a.append("ttl", a.createGenericCell()->set(si.ttl));

	return *this;
}

GenericCell& GenericCell::set(const CubeInfo& ci)
{
	ArrayBuilder a = setArray(8);

	a.append("identifier", a.createGenericCell()->set(ci.identifier));
	a.append("name", a.createGenericCell()->set(ci.name));
	a.append("number_dimensions", a.createGenericCell()->set(ci.number_dimensions));
	a.append("dimensions", a.createGenericCell()->set(ci.dimensions));
	a.append("number_cells", a.createGenericCell()->set(ci.number_cells));
	a.append("number_filled_cells", a.createGenericCell()->set(ci.number_filled_cells));
	a.append("status", a.createGenericCell()->set(ci.status));
	a.append("type", a.createGenericCell()->set(ci.type));

	return *this;
}

GenericCell& GenericCell::set(const SubsetResult& sr)
{
	ArrayBuilder a = setArray(3);

	a.append("name", a.createGenericCell()->set(sr.name));
	a.append("alias", a.createGenericCell()->set(sr.alias));
	a.append("indent", a.createGenericCell()->set(sr.idx));

	return *this;
}

GenericCell& GenericCell::set(const SubsetResults& srs)
{
	return set < SubsetResult > (srs);
}

GenericCell& GenericCell::set(const char *s)
{
	return set(std::string(s));
}

boost::shared_ptr<jedox::palo::Server> GenericCell::getConnection()
{
	return jedox::palo::ServerPool::getInstance().getServer(getString());
}

GenericCell& GenericCell::set(const DimensionElementType& t)
{
	std::string s;

	switch (t.type) {
	case DimensionElementType::Numeric:
		s = "numeric";
		break;
	case DimensionElementType::String:
		s = "string";
		break;
	case DimensionElementType::Consolidated:
		s = "consolidated";
		break;
	default:
		throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_DIMENSION_ELEMENT_TYPE);
	}

	set(s);

	return *this;
}

void GenericCell::setHelper(const CellValue& v, bool set_error_desc)
{
	switch (v.type) {
	case CellValue::NUMERIC:
		set(v.val.d);
		break;

	case CellValue::STRING:
		set(v.val.s);
		break;

	case CellValue::ERR:
		setError(v.val.err, set_error_desc);
		break;

	default:
		throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_UNKNOWN_CELL_VALUE_TYPE);
	}
}

GenericCell& GenericCell::set(const CellValue& v, bool set_error_desc)
{
	setHelper(v, set_error_desc);
	return *this;
}

GenericCell& GenericCell::set(const CellValueWithProperties& cv, bool set_error_desc)
{
	setHelper(cv, set_error_desc);
	if (cv.type != CellValue::ERR) {
		setPropValues(cv.PropValues);
	}

	return *this;
}

GenericCell& GenericCell::setError(const jedox::palo::PaloException& e)
{
	return setError(ErrorInfo(XLError::getError(e), e.code(), e.longDescription()));
}

GenericCell& GenericCell::set(const ElementList& el)
{
	if (el.all()) {
		set(ELEMENTS_ALL);
	} else {
		set(el.getArray());
	}

	return *this;
}

GenericCell& GenericCell::set(const DimensionElementInfoReduced& el)
{
	return *this;
}

ElementList GenericCell::getElementList()
{
	ElementList el;

	if ((getType() == GenericCell::TString) && (getString() == ELEMENTS_ALL)) {
		el.setAll();
	} else {
		el.set(getStringArray());
	}

	return el;
}

#ifdef WIN64
GenericCell& GenericCell::set( size_t n ) {
	// TODO: is this always right?
	return set(( long int )n );
}
#endif

namespace Palo {
namespace SpreadsheetFuncs {
/*! \brief Specialization. */
template<>
GenericCell::Iterator GenericCell::get<GenericCell::Iterator>()
{
	return getArray();
}

/*! \brief Specialization. */
template<>
StringArray GenericCell::get<StringArray>()
{
	return getStringArray();
}

/*! \brief Specialization. */
template<>
StringArrayArray GenericCell::get<StringArrayArray>()
{
	return getStringArrayArray();
}

/*! \brief Specialization. */
template<>
std::string GenericCell::get<std::string>()
{
	return getString();
}

/*! \brief Specialization. */
template<>
long int GenericCell::get<long int>()
{
	return getSLong();
}

/*! \brief Specialization. */
template<>
unsigned long int GenericCell::get<unsigned long int>()
{
	return getULong();
}

/*! \brief Specialization. */
template<>
int GenericCell::get<int>()
{
	return getSInt();
}

/*! \brief Specialization. */
template<>
unsigned int GenericCell::get<unsigned int>()
{
	return getUInt();
}

/*! \brief Specialization. */
template<>
bool GenericCell::get<bool>()
{
	return getBool();
}

/*! \brief Specialization. */
template<>
double GenericCell::get<double>()
{
	return getDouble();
}

/*! \brief Specialization. */
template<>
DimensionElementType GenericCell::get<DimensionElementType>()
{
	return getDimensionElementType();
}

/*! \brief Specialization. */
template<>
ElementList GenericCell::get<ElementList>()
{
	return getElementList();
}

/*! \brief Specialization. */
template<>
ElementListArray GenericCell::get<ElementListArray>()
{
	return getElementListArray();
}

/*! \brief Specialization. */
template<>
ConsolidationElement GenericCell::get<ConsolidationElement>()
{
	return getConsolidationElement();
}

/*! \brief Specialization. */
template<>
ConsolidationElementArray GenericCell::get<ConsolidationElementArray>()
{
	return getConsolidationElementArray();
}

/*! \brief Specialization. */
template<>
CellValue GenericCell::get<CellValue>()
{
	return getCellValue();
}

/*! \brief Specialization. */
template<>
CellValueArray GenericCell::get<CellValueArray>()
{
	return getCellValueArray();
}

/*! \brief Specialization. */
template<>
jedox::palo::SPLASH_MODE GenericCell::get<jedox::palo::SPLASH_MODE>()
{
	return getSplashMode();
}
}
}
