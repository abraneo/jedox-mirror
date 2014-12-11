/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
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
 * 
 *
 */

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/Element.h>
#include <libpalo_ng/Palo/Cache/DatabaseCache.h>
#include <libpalo_ng/Palo/Cache/DimensionsCache.h>
#include <libpalo_ng/Palo/Cache/DimensionCache.h>
#include <libpalo_ng/Palo/Cache/ElementCache.h>
#include <libpalo_ng/Palo/Network/PaloClient.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "Exception/TokenOutdatedException.h"
#include "Exception/CacheInvalidException.h"

#include "ServerImpl.h"

namespace jedox {
namespace palo {

struct ElementHelper {

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
			const ElementCache& elem = (*elemlist)[element_names[i]];
			elemids.push_back(elem.element);
			elements.push_back(&elem);
		}
	}

};

Element::Element(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db, unsigned int dim, unsigned int element, boost::shared_ptr<const SIElements> cache) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_Database(db), m_Dimension(dim), m_Element(element), m_Cache(!cache ? serverImpl->getElements(m_PaloClient, db, dim) : cache)
{
}

void Element::append(const std::vector<std::string>& children, const std::vector<double>& weights)
{
	ELEMENT_LIST elemids;
	std::vector<const ElementCache *> elems;

	// get the ids for the children names
	ElementHelper::setElementsAndIds(elems, elemids, children, m_Cache);

	append(elemids, weights);
}

void Element::append(const ELEMENT_LIST& children, const std::vector<double>& weights)
{
	std::stringstream query;
	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);
	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&element=" << m_Element;

	if (!children.empty()) {
		query << "&children=";
		jedox::util::TListe(query, children, ',', false);
	}

	if (!weights.empty()) {
		query << "&weights=";
		jedox::util::TListe(query, weights, ',', false);
	}

	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/append", query.str(), token, sequencenumber, dummy);
		ElementCache element;
		(*stream) >> csv >> element;
		m_Cache = m_ServerImpl->updateElement(m_PaloClient, sequencenumber, m_Database, m_Dimension, element);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Element::replace(ELEMENT_INFO::TYPE type, const std::vector<std::string>& children, const std::vector<double>& weights)
{
	ELEMENT_LIST elemids;
	std::vector<const ElementCache *> elems;

	// get the ids for the children names
	ElementHelper::setElementsAndIds(elems, elemids, children, m_Cache);

	std::stringstream query;
	query.setf(std::ios_base::fixed, std::ios_base::floatfield);
	query.precision(PRECISION);

	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&element=" << m_Element << "&type=" << type;

	if (type == ELEMENT_INFO::CONSOLIDATED) {
		if (!elemids.empty()) {
			query << "&children=";
			jedox::util::TListe(query, elemids, ',', false);
		}

		if (!weights.empty()) {
			query << "&weights=";
			jedox::util::TListe(query, weights, ',', false);
		}
	}

	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/replace", query.str(), token, sequencenumber, dummy);
		ElementCache element;
		(*stream) >> csv >> element;
		m_Cache = m_ServerImpl->updateElement(m_PaloClient, sequencenumber, m_Database, m_Dimension, element);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Element::rename(const std::string& newName)
{
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&element=" << m_Element << "&new_name=";
	jedox::util::URLencoder(query, newName);

	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/rename", query.str(), token, sequencenumber, dummy);
		ElementCache element;
		(*stream) >> csv >> element;
		m_Cache = m_ServerImpl->updateElement(m_PaloClient, sequencenumber, m_Database, m_Dimension, element);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

void Element::move(unsigned int position)
{
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&element=" << m_Element << "&position=" << position;

	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/move", query.str(), token, sequencenumber, dummy);
		ElementCache element;
		(*stream) >> csv >> element;
		m_Cache = m_ServerImpl->updateElement(m_PaloClient, sequencenumber, m_Database, m_Dimension, element);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
}

bool Element::destroy()
{
	char result = 0;
	std::stringstream query;
	query << "database=" << m_Database << "&dimension=" << m_Dimension << "&element=" << m_Element;
	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	DIMENSION_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/element/destroy", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateElements(sequencenumber, m_Database, m_Dimension);
		throw e;
	}
	return result == '1';
}

const ELEMENT_INFO& Element::getCacheData() const
{
	return (*m_Cache)[m_Element];
}

const ELEMENT_INFO Element::getCacheDataCopy() const
{
	return getCacheData();
}

Element::~Element()
{
}

} /* palo */
} /* jedox */
