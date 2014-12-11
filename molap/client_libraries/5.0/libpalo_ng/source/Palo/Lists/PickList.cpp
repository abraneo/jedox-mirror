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
 *
 */

#include <string>

#include <boost/bind.hpp>

#include "ListBasicException.h"
#include "PickList.h"
#include "SubSet.h"

using namespace jedox::palo;

namespace jedox {
namespace palo {

class PickList::CheckElementsInStrings {
	std::list<std::string> elements;
public:
	bool operator()(ELEMENT_INFO_EXT) const;
	CheckElementsInStrings(const std::list<std::string>& elems);
};

bool PickList::CheckElementsInStrings::operator()(ELEMENT_INFO_EXT elem) const
{
	std::list<std::string>::const_iterator pos = this->elements.begin();
	std::list<std::string>::const_iterator end = this->elements.end();
	while (pos != end) {
		if (pos->compare(elem.get_name()) == 0) {
			return false;
		}
		++pos;
	}
	return true;
}

PickList::CheckElementsInStrings::CheckElementsInStrings(const std::list<std::string>& elems)
{
	this->elements = elems;
}

struct PickList::PickListImpl {
	std::list<std::string> picklist;
	ElementExList elemlist;
};

PickList::PickList(SubSet& s, unsigned long int flags) :
	Filter(s, flags, PICKLIST_NUM_FLAGS, Filter::PICKLIST), m_Impl(new PickListImpl)
{
	;
}

PickList::~PickList()
{
}

void PickList::apply()
{
	Filter::apply();

	if (!m_Impl->picklist.empty()) {
		buildElemlist();
		if (queryFlag(INSERT_BACK)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_BACK);
		} else if (queryFlag(MERGE)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_MERGE);
		} else if (queryFlag(SUB)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_SUB);
			CheckElementsInStrings check(m_Impl->picklist);
			m_subset_ref.getConcreteSubset().remove_if(check);
		} else {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_FRONT);
		}
	}
	//if a picklist is applied and empty, nothing happens
	else {
		throw ListBasicException("Application of Picklist without a list of elements.", "Wrong Picklist usage");
	}
	m_subset_ref.setGlobalFlag(SubSet::PICKLIST_ACTIVE);
}

void PickList::setPicklist(const std::list<std::string>& elems)
{
	if (!queryFlag(INSERT_BACK) && !queryFlag(INSERT_FRONT) && !queryFlag(MERGE) && !queryFlag(SUB))
		setFlag(INSERT_FRONT);
	m_Impl->picklist = elems;
}

void PickList::buildElemlist(void)
{
	Dimension& dim = m_subset_ref.getDimension();
	try {
		std::list<std::string>::iterator beg = m_Impl->picklist.begin();
		std::list<std::string>::iterator end = m_Impl->picklist.end();
		for (; beg != end; ++beg) {
			m_Impl->elemlist.push_back(dim[*beg].getCacheData());
		}
	} catch (jedox::palo::PaloException e) {
		throw ListBasicException("Could not retrieve Elements from picklist-Strings. " + e.longDescription(), e.shortDescription());
	}
}

ElementExList& PickList::getElements()
{
	return m_Impl->elemlist;
}

} //palo
} //jedox
