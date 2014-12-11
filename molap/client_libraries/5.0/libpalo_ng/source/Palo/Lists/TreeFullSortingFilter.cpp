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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

/*@File: this filter removes only those elements that have been filtered by previous filters
 children of removed elements are included.*/

#include "TreeFullSortingFilter.h"

#include <algorithm>
#include <list>

#include <boost/bind.hpp>

using namespace jedox::palo;

namespace jedox {
namespace palo {

void SDTreeFullSortingFilter::finalize(ElementExList& elemlist)
{
	ElementExList::iterator it_beg = elemlist.begin();
	ElementExList::const_iterator it_end = elemlist.end();
	while (it_beg != it_end) {
		if (!concrete_subset.checkId(it_beg->get_id())) {
			it_beg = elemlist.erase(it_beg);
		} else
			++it_beg;
	}
}

/*@brief: sort according to names, evtl. reverse if reverse is set */
void SDTreeFullSortingFilter::processChildren(ElementExList& elems)
{

	elems.sort(boost::bind(&AbstractComparison::operator(), boost::ref(m_comp), _1, _2));
	if (m_filter.m_parents_below) {
		elems.reverse();
	}
}

SDTreeFullSortingFilter::SDTreeFullSortingFilter(SubSet& concrete_subset, const SortingFilter& filter, AbstractComparison& comp) :
	concrete_subset(concrete_subset), m_filter(filter), m_comp(comp)
{
}

bool SDTreeFullSortingFilter::check(const ELEMENT_INFO_EXT& elem)
{
	return concrete_subset.checkPath(elem);
}

/*@brief: remove duplicates */
void TreeFullSortingFilter::finalize(ElementExList& elemlist)
{
	std::set<long> processed_ids;
	//remove duplicates
	for (ElementExList::iterator it = elemlist.begin(); it != elemlist.end();) {
		if (processed_ids.find(it->get_id()) == processed_ids.end()) {
			processed_ids.insert(it->get_id());
			++it;
		} else {
			it = elemlist.erase(it);
		}
	}
	//ElementExList::iterator it_source = m_given_elements.begin();
	for (ElementExList::iterator it_beg = elemlist.begin(); it_beg != elemlist.end();) {
		if (!concrete_subset.checkId(it_beg->get_id())) {
			it_beg = elemlist.erase(it_beg);
		} else
			//copy other data we computed before,
			// e.g. in data-filter

			++it_beg;
	}
}

/*@brief: sort according to names, evtl. reverse if reverse is set */
void TreeFullSortingFilter::processChildren(ElementExList& elems)
{
	elems.sort(boost::bind(&AbstractComparison::operator(), boost::ref(m_comp), _1, _2));
	if (m_filter.m_parents_below) {
		elems.reverse();
	}
}

TreeFullSortingFilter::TreeFullSortingFilter(SubSet& concrete_subset, const SortingFilter& filter, AbstractComparison& comp) :
	concrete_subset(concrete_subset), m_filter(filter), m_comp(comp)
{
}

bool TreeFullSortingFilter::check(const ELEMENT_INFO_EXT& elem)
{
	return concrete_subset.checkPath(elem);
}

} //palo
} //jedox
