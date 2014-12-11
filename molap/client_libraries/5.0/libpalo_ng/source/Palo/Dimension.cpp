/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/Dimension.h>
#include <libpalo_ng/Palo/Cache/DatabaseCache.h>
#include <libpalo_ng/Palo/Cache/DimensionsCache.h>
#include <libpalo_ng/Palo/Cache/ElementCache.h>
#include <libpalo_ng/Palo/Cache/CubesCache.h>
#include <libpalo_ng/Palo/Cache/CubeCache.h>
#include <libpalo_ng/Palo/Network/PaloClient.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "Exception/TokenOutdatedException.h"
#include "Exception/CacheInvalidException.h"

#include "Lists/AliasFilter.h"
#include "Lists/DataFilter.h"
#include "Lists/PickList.h"
#include "Lists/SortingFilter.h"
#include "Lists/StructuralFilter.h"
#include "Lists/TextFilter.h"
#include "Lists/SubSet.h"

#include "ServerImpl.h"

namespace jedox {
namespace palo {
struct DimensionHelper {

	typedef std::vector<std::string> ELEMENT_NAME_LIST;
	typedef boost::shared_ptr<DimensionCache> shared_DimCache_type;
	typedef boost::shared_ptr<DatabaseCache> shared_DBCache_type;

	/*!
	 * \brief
	 * little helper function
	 *
	 * \author
	 * Frieder Hofmann <frieder.hofmann@jedox.com>
	 */
	static inline void setElementsAndIds(std::vector<const ElementCache *>& elements, ELEMENT_LIST& elemids, const ELEMENT_NAME_LIST& element_names, boost::shared_ptr<const SIElements> elemlist)
	{
		const ELEMENT_NAME_LIST::size_type element_names_size = element_names.size();
		elemids.reserve(element_names_size);
		elements.reserve(element_names_size);
		for (ELEMENT_NAME_LIST::size_type i = 0; i < element_names_size; i++) {
			const ElementCache *elem = &(*elemlist)[element_names[i]];
			elemids.push_back(elem->element);
			elements.push_back(elem);
		}
	}

	static inline ELEMENT_LIST getElementIds(boost::shared_ptr<const SIElements> elemlist, const ELEMENT_NAME_LIST& element_names)
	{
		ELEMENT_LIST return_vec;
		const ELEMENT_NAME_LIST::size_type element_names_size = element_names.size();
		return_vec.reserve(element_names_size);
		for (ELEMENT_NAME_LIST::size_type i = 0; i < element_names_size; i++) {
			return_vec.push_back((*elemlist)[element_names[i]].element);
		}
		return return_vec;
	}

	static inline std::vector<ELEMENT_LIST> getElementIds(boost::shared_ptr<const SIElements> elemlist, const std::vector<ELEMENT_NAME_LIST>& element_names)
	{
		std::vector<ELEMENT_LIST> return_vec;
		const std::vector<ELEMENT_NAME_LIST>::size_type element_names_size = element_names.size();
		return_vec.reserve(element_names_size);
		for (std::vector<ELEMENT_NAME_LIST>::size_type i = 0; i < element_names_size; i++) {
			return_vec.push_back(getElementIds(elemlist, element_names[i]));
		}
		return return_vec;
	}

};

Dimension::Dimension(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_Database(db), m_Dimension(dim), m_Cache(m_ServerImpl->getDimensions(m_PaloClient, db, false))
{
}

Element Dimension::operator[](const std::string& name)
{
	if (getCacheData().type == DIMENSION_INFO::SYSTEM_ID) {
		boost::shared_ptr<SIElements> cache(new SISystemIdElements(getSequenceNumberFromCache(), getCacheData().ndimension));
		return Element(m_ServerImpl, m_PaloClient, m_Database, m_Dimension, (*cache)[name].element, cache);
	} else {
		return Element(m_ServerImpl, m_PaloClient, m_Database, m_Dimension, m_ServerImpl->getElement(m_PaloClient, m_Database, m_Dimension, name).element, boost::shared_ptr<const SIElements>());
	}
}

Element Dimension::operator[](unsigned int id)
{
	if (getCacheData().type == DIMENSION_INFO::SYSTEM_ID) {
		boost::shared_ptr<SIElements> cache(new SISystemIdElements(getSequenceNumberFromCache(), getCacheData().ndimension));
		return Element(m_ServerImpl, m_PaloClient, m_Database, m_Dimension, (*cache)[id].element, cache);
	} else {
		return Element(m_ServerImpl, m_PaloClient, m_Database, m_Dimension, m_ServerImpl->getElement(m_PaloClient, m_Database, m_Dimension, id).element, boost::shared_ptr<const SIElements>());
	}
}

bool Dimension::Exists(const std::string& name)
{
	return m_ServerImpl->existElement(m_PaloClient, m_Database, m_Dimension, name);
}

bool Dimension::Exists(unsigned int id, bool cacheonly)
{
	return m_ServerImpl->existElement(m_PaloClient, m_Database, m_Dimension, id);
}

void Dimension::createElement(const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<std::string> &children, const std::vector<double> &weights)
{
	ELEMENT_INFO result;
	createElement(result, Name, type, children, weights);
}

void Dimension::createElement(ELEMENT_INFO& result, const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<std::string> &children, const std::vector<double> &weights)
{
	std::vector<long> elemids;
	std::vector<const ElementCache *> elems;

	// get the ids for the children names
	DimensionHelper::setElementsAndIds(elems, elemids, children, m_ServerImpl->getElements(m_PaloClient, m_Database, m_Dimension, false));

	createElement(result, Name,type, elemids, weights);
}

void Dimension::createElement(const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<long> &children, const std::vector<double> &weights)
{
	ELEMENT_INFO result;
	createElement(result, Name, type, children, weights);
}

void Dimension::createElement(ELEMENT_INFO& result, const std::string& Name, ELEMENT_INFO::TYPE type, const std::vector<long> &children, const std::vector<double> &weights)
{
	std::stringstream query;
	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);

	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&new_name=";
	jedox::util::URLencoder(query, Name);
	query << "&type=" << type;

	if (type == ELEMENT_INFO::CONSOLIDATED) {
		if (!children.empty()) {
			query << "&children=";
			jedox::util::TListe(query, children, ',', false);
		}

		if (!weights.empty()) {
			query << "&weights=";
			jedox::util::TListe(query, weights, ',', false);
		}
	}

	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/create", query.str(), token, sequencenumber, dummy);
		ElementCache element;
		(*stream) >> csv >> element;
		m_ServerImpl->updateElement(m_PaloClient, sequencenumber, m_Database, m_Dimension, element);
		result = element;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Dimension::bulkDeleteElements(const std::vector<std::string> &elements)
{
	bulkDeleteElements(DimensionHelper::getElementIds(m_ServerImpl->getElements(m_PaloClient, m_Database, m_Dimension, false), elements));
}

void Dimension::bulkDeleteElements(const std::vector<long> &elements)
{
	std::stringstream query;

	query << "database=" << m_Database << "&dimension=" << m_Dimension;
	query << "&elements=";
	jedox::util::TListe(query, elements, ',');
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/element/destroy_bulk", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Dimension::bulkCreateElements(const std::vector<std::string> &elements, ELEMENT_INFO::TYPE type, const std::vector<std::vector<std::string> > &children, const std::vector<std::vector<double> > &weights)
{
	size_t i, vsize = elements.size();
	std::vector<std::string> codednames(vsize);

	for (i = 0; i < vsize; i++) {
		codednames[i] = jedox::util::StringUtils::CSVencode(elements[i]);
	}

	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension;
	query << "&name_elements=";
	jedox::util::UrlEncodeTListe(query, codednames, ',', false);
	query << "&type=" << type;

	boost::shared_ptr<const SIElements> elemlist = m_ServerImpl->getElements(m_PaloClient, m_Database, m_Dimension, false);
	if (type == ELEMENT_INFO::CONSOLIDATED) {
		query << "&children=";
		jedox::util::ListeTListe(query, DimensionHelper::getElementIds(elemlist, children), ',', ':');
		if (weights.size() > 0) {
			query << "&weights=";
			jedox::util::ListeTListe(query, weights, ',', ':');
		}
	}

	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/element/create_bulk", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Dimension::bulkReplaceElements(const std::vector<std::string> &elements, ELEMENT_INFO::TYPE type, const std::vector<std::vector<std::string> > &children, const std::vector<std::vector<double> > &weights)
{
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension;
	query << "&name_elements=";
	jedox::util::UrlEncodeTListe(query, elements, ',', false);
	query << "&type=" << type;

	if (type == ELEMENT_INFO::CONSOLIDATED) {
		query << "&children=";
		jedox::util::ListeTListe(query, DimensionHelper::getElementIds(m_ServerImpl->getElements(m_PaloClient, m_Database, m_Dimension, false), children), ',', ':');
		if (weights.size() > 0) {
			query << "&weights=";
			jedox::util::ListeTListe(query, weights, ',', ':');
		}
	}

	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/element/replace_bulk", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Dimension::rename(const std::string& newName)
{
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&new_name=";
	jedox::util::URLencoder(query, newName);
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/dimension/rename", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		throw e;
	}
}

void Dimension::clear()
{
	return clear(ELEMENT_INFO::UNKNOWN);
}

void Dimension::clear(ELEMENT_INFO::TYPE type)
{
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension;
	if (type) {
		query << "&type=" << type;
	}
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/dimension/clear", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

bool Dimension::destroy()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension;

	unsigned int dummy, sequencenumber = m_ServerImpl->getDimensionsSN(m_PaloClient, m_Database);
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/dimension/destroy", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		throw e;
	}
	return result == '1';
}
jedox::palo::ElementExList Dimension::subset(Server *server, const AliasFilterSettings& alias, const FieldFilterSettings& field, const BasicFilterSettings& basic, const DataFilterSettings& data, const SortingFilterSettings& sorting, const StructuralFilterSettings& structural, const TextFilterSettings& text)
{
	ElementExList retval;
	subset(retval, server, alias, field, basic, data, sorting, structural, text);
	return retval;
}
void Dimension::subset(jedox::palo::ElementExList& retval, Server *server, const AliasFilterSettings& alias, const FieldFilterSettings& field, const BasicFilterSettings& basic, const DataFilterSettings& data, const SortingFilterSettings& sorting, const StructuralFilterSettings& structural, const TextFilterSettings& text)
{
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	DatabaseCache db = m_ServerImpl->getDatabase(m_PaloClient, m_Database);
	SubSet sub(server, db.ndatabase, dim.ndimension);

	if (basic.active) {
		sub.addFilter(Filter::PICKLIST, basic.flags);

		if (basic.manual_subset_set) {
			sub.getPicklist()->setPicklist(std::list<std::string>(basic.manual_subset.begin(), basic.manual_subset.end()));
		}
	}

	if (text.active) {
		sub.addFilter(Filter::TEXT, text.flags);
		std::vector<std::string>::const_iterator begin = text.regexps.begin();
		std::vector<std::string>::const_iterator end = text.regexps.end();
		std::vector<std::string>::const_iterator i;

		for (i = begin; i != end; i++) {
			sub.getTextFilter()->setRegex(*i, alias.active ? TextFilter::ALIAS : TextFilter::BASIC);
		}
	}

	if (alias.active || field.active) {
		sub.addFilter(Filter::ALIAS, alias.flags | field.flags);
		if (alias.active)
			sub.getAliasFilter()->setAttributes(alias.attribute1, alias.attribute2);
		if (field.active)
			sub.getAliasFilter()->setFilters(field.advanced);
	}

	if (structural.active) {
		sub.addFilter(Filter::STRUCTURAL, structural.flags);
		if (!structural.bound.empty())
			sub.getStrucuralFilter()->setBound(structural.bound);
		if (structural.level) {
			sub.getStrucuralFilter()->setLevels(structural.level_start, structural.level_end);
		}
		if (structural.revolve) {
			sub.getStrucuralFilter()->setRevolveParams(structural.revolve_elem, structural.revolve_count);
		}
		if (structural.indent > 0) {
			sub.getStrucuralFilter()->setIndent(structural.indent);
		}
	}

	if (data.active) {
		sub.addFilter(Filter::DATA, data.flags);
		sub.getDataFilter()->setSourceCube(data.cube);
		if (data.cmp.use_strings)
			sub.getDataFilter()->setOperands(data.cmp.par1s, data.cmp.op1, data.cmp.par2s, data.cmp.op2);
		else
			sub.getDataFilter()->setOperands(data.cmp.par1d, data.cmp.op1, data.cmp.par2d, data.cmp.op2);
		if (data.coords_set) {
			std::vector<std::vector<std::string> > coords;
			coords.reserve(data.coords.size());

			CubeCache c = m_ServerImpl->getCube(m_PaloClient, m_Database, data.cube);

			/* allow to omit the last dimension if this is the one to be filtered */
			if (c.number_dimensions != data.coords.size() && c.number_dimensions != data.coords.size() + 1)
				PaloExceptionFactory::raise(PaloExceptionFactory::ERROR_INVALID_COORDINATES, "Number of dimensions do not match.", "Number of dimensions do not match (passed <> cube).");

			bool got_empty = false;

			for (DataFilterSettings::CoordsType::const_iterator i = data.coords.begin(); i != data.coords.end(); ++i) {
				if (i->first) {
					// is_all
					DimensionCache tempdim = m_ServerImpl->getDim(m_PaloClient, m_Database, c.dimensions[i - data.coords.begin()]);

					std::vector<std::string> sa;
					sa.reserve(tempdim.number_elements);

					for (std::unique_ptr<jedox::palo::DimensionCache::CacheIterator> i = m_ServerImpl->getElemIterator(m_PaloClient, m_Database, tempdim.dimension); !(*i).end(); ++(*i))
						sa.push_back((*i)->nelement);

					coords.push_back(sa);
				} else {
					coords.push_back(i->second);

					if (i->second.empty())
						got_empty = true;
				}
			}

			if (!got_empty) {
				if (c.number_dimensions == data.coords.size() + 1)
					coords.resize(c.number_dimensions);
				else
					PaloExceptionFactory::raise(PaloExceptionFactory::ERROR_INVALID_COORDINATES, "Number of dimensions do not match.", "Number of dimensions do not match (passed <> cube).");
			}

			sub.getDataFilter()->setCoordinates(coords);
		}
		if (data.upper_percentage_set || data.lower_percentage_set)
			sub.getDataFilter()->setPercentage(data.upper_percentage, data.lower_percentage);

		if (data.top >= 0) {
			sub.getDataFilter()->setTop(data.top);
		}

	}

	if (sorting.active) {
		sub.addFilter(Filter::SORTING, sorting.flags);
		if (!sorting.attribute.empty())
			sub.getSortingFilter()->setAttribute(sorting.attribute);

		//if ( !sorting.level_element.empty() )
		//	sub.getSortingFilter()->setLevelElement( sorting.level_element );
		if (sorting.level >= 0) {
			sub.getSortingFilter()->setSingleLevel(sorting.level);

		}
		//set the indent used when sorting by level
		if (sorting.indent >= 0) {
			sub.getSortingFilter()->setIndent(sorting.indent);
		}
		sub.getSortingFilter()->setLimit(sorting.limit_start, sorting.limit_count);
	}
	sub.applyFilters()->swap(retval);
}

const DIMENSION_INFO& Dimension::getCacheData() const
{
	return (*m_Cache)[m_Dimension];
}

const DIMENSION_INFO Dimension::getCacheDataCopy() const
{
	return getCacheData();
}

std::unique_ptr<DimensionCache::CacheIterator> Dimension::getIterator()
{
	return m_ServerImpl->getElemIterator(m_PaloClient, m_Database, m_Dimension);
}

unsigned int Dimension::getSequenceNumberFromCache() const
{
	return m_Cache->getSequenceNumber();
}

void Dimension::dfilter(ElementExList &subset, const std::string &cube, std::vector<std::vector<std::string> > &coords, unsigned long mode, const std::string &condition, int top, double highPer, double lowPer)
{
	CubeCache cub = m_ServerImpl->getCube(m_PaloClient, m_Database, cube);
	const DIMENSION_LIST& dimlist = cub.dimensions;
	if (coords.size() != dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	size_t dimpos = 0;
	for (DIMENSION_LIST::const_iterator it = dimlist.begin(); it != dimlist.end(); it++, dimpos++) {
		if ((unsigned int)*it == m_Dimension) {
			break;
		}
	}
	if (dimpos == dimlist.size()) {
		const unsigned int errorcode = PaloExceptionFactory::ERROR_INVALID_COORDINATES;
		const std::string errormsg = WRONGDIMENSIONCOUNT;
		throw PaloServerException(errormsg, errormsg, errorcode);
	}

	vector<string> &v = coords[dimpos];
	v.clear();
	v.reserve(subset.size());
	for (ElementExList::iterator it = subset.begin(); it != subset.end(); it++) {
		v.push_back(it->get_name());
	}

	std::vector<std::vector<long> > coord(CubeHelper::getElementIDs(m_ServerImpl, m_PaloClient, m_Database, dimlist, coords));
	std::stringstream query;
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN sequenceToken(sequencenumber);

	query << "database=" << m_Database << "&dimension=" << m_Dimension;
	query << "&cube=" << cub.cube << "&area=";
	jedox::util::ListeTListe(query, coord, ':', ',', true);
	query << "&mode=" << mode << "&condition=" << condition;
	query << "&values=" << top << ":" << highPer << ":" << lowPer;
	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/dimension/dfilter", query.str(), sequenceToken, sequencenumber, dummy);

	multimap<int, ElementExList::iterator> m;
	for (ElementExList::iterator it = subset.begin(); it != subset.end(); it++) {
		m.insert(pair<int, ElementExList::iterator>(it->m_einf.element, it));
	}


	while ((*stream).eof() == false) {
		CELL_VALUE val;
		ElementCache element;
		(*stream) >> csv >> element;
		(*stream) >> val;
		if (m.empty() && getCacheData().type == DIMENSION_INFO::SYSTEM_ID) {
			subset.push_back(element);
		} else {
			multimap<int, ElementExList::iterator>::iterator it;
			if ((it = m.find(element.element)) != m.end()) {
				it->second->value = val.val.d;
				it->second->string_value = val.val.s;
				m.erase(it);
			}
		}
	}
	for (multimap<int, ElementExList::iterator>::iterator it = m.begin(); it != m.end(); it++) {
		subset.erase(it->second);
	}
}

std::list<ELEMENT_INFO> Dimension::getElements(long parentid, long start, long limit) const
{
	std::list<ELEMENT_INFO> ret;
	if (limit) {
		std::unique_ptr<std::istringstream> stream = getElementsIntern(parentid, start, limit);
		while ((*stream).eof() == false) {
			ELEMENT_INFO element;
			(*stream) >> csv >> element;
			ret.push_back(element);
		}
	}
	return ret;
}

size_t Dimension::getTopCount() const
{
	size_t ret;
	std::unique_ptr<std::istringstream> stream = getElementsIntern(-1, 0, 0);
	if ((*stream).eof() == false) {
		(*stream) >> ret;
	}
	return ret;
}

size_t Dimension::getFlatCount() const
{
	size_t ret;
	std::unique_ptr<std::istringstream> stream = getElementsIntern(-2, 0, 0);
	if ((*stream).eof() == false) {
		(*stream) >> ret;
	}
	return ret;
}

std::unique_ptr<std::istringstream> Dimension::getElementsIntern(long parentid, long start, long limit) const
{
	std::unique_ptr<std::istringstream> stream;
	std::stringstream query;
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN sequenceToken(sequencenumber);

	query << "show_lock_info=1&database=" << m_Database << "&dimension=" << m_Dimension;
	if (parentid != -2) {
		query << "&parent=";
		if (parentid != -1) {
			query << parentid;
		}
	}
	query << "&limit=" << start;
	if (limit != -1) {
		query << "," << limit;
	}

	try {
		stream = m_PaloClient->request("/dimension/elements", query.str(), sequenceToken, sequencenumber, dummy);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
	return stream;
}

void Dimension::unlock()
{
	std::stringstream query;
	query << "complete=1&database=" << m_Database << "&dimension=" << m_Dimension;
	const DimensionCacheB &dim = (*m_Cache)[m_Dimension];
	unsigned int dummy, sequencenumber = dim.getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		m_PaloClient->request("/element/unlock", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

} /* palo */
} /* jedox */
