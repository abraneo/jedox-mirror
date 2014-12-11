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
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * 
 *
 */

#include <list>
#include <set>
#include <algorithm>

#include <libpalo_ng/Palo/types.h>

#include "TreeHierarchyFilter.h"
#include "SubSet.h"

namespace jedox {
namespace palo {

class TreeHierarchyFilter::CopyToMultiSet {
public:
	CopyToMultiSet(SubSet::id_set_type& found_elements) :
		m_found_elems(found_elements)
	{
		;
	}
	void operator()(const ELEMENT_INFO_EXT& einf)
	{
		m_found_elems.insert(einf.get_id());
	}
	SubSet::id_set_type& m_found_elems;
};

bool TreeHierarchyFilter::check(const ELEMENT_INFO_EXT& elem)
{
	if (m_filter.m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS) || m_found_elems.find(elem.get_id()) == m_found_elems.end()) {
		m_found_elems.insert(elem.get_id());
		return true;
	}
	return false;
}

void TreeHierarchyFilter::finalize(ElementExList& elemlist)
{
	if (m_filter.queryFlag(StructuralFilter::BELOW_EXCLUSIVE) || m_filter.queryFlag(StructuralFilter::ABOVE_EXCLUSIVE)) {
		m_found_elems.erase(m_filter.m_elem_bound);
	}

	ElementExList::iterator it_beg = elemlist.begin();
	ElementExList::iterator it_end = elemlist.end();
	// erase all elements not in the set, which means they are not below the chosen element
	while (it_beg != it_end) {
		if (false == m_check.do_check(*it_beg)) {
			it_beg = elemlist.erase(it_beg);
		} else if (m_found_elems.find(it_beg->get_id()) == m_found_elems.end()) {
			it_beg = elemlist.erase(it_beg);
		} else {
			++it_beg;
		}
	}
}

void TreeHierarchyFilter::processChildren(ElementExList& elems)
{
	for (ElementExList::iterator it = elems.begin(); it != elems.end(); ++it) {
		m_found_elems.insert(it->get_id());
	}
}

TreeHierarchyFilter::TreeHierarchyFilter(ElementExList& given_elements, const StructuralFilter& filter, const StructuralFilter::InRange& check) :
	m_given_elements(given_elements), m_filter(filter), m_check(check)
{
	std::for_each(m_given_elements.begin(), m_given_elements.end(), CopyToMultiSet(m_given_names));

}

} //palo
} //jedox
