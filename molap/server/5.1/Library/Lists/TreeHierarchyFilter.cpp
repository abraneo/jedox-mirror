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
 * \author Frieder Hofmann , Jedox AG, Freiburg, Germany
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include <list>
#include <set>
#include <algorithm>

#include "TreeHierarchyFilter.h"
#include "SubSet.h"

namespace palo {

bool TreeHierarchyFilter::check(const SubSet::Iterator &elem)
{
	if (m_filter.m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS) || m_found_elems.find(elem.getElement()) == m_found_elems.end()) {
		m_found_elems.insert(elem.getElement());
		return true;
	}
	return false;
}

void TreeHierarchyFilter::finalize(vector<SubElem> &result)
{
	if (m_filter.queryFlag(StructuralFilter::BELOW_EXCLUSIVE) || m_filter.queryFlag(StructuralFilter::ABOVE_EXCLUSIVE)) {
		m_found_elems.erase(m_filter.getElemBound());
	}

	vector<SubElem> tmp;
	tmp.reserve(result.size());
	for (vector<SubElem>::iterator itIds = result.begin(); itIds != result.end(); ++itIds) {
		if (m_found_elems.find(itIds->elem) != m_found_elems.end()) {
			if (m_check.do_check(itIds->elem->getLevel(), itIds->ind, itIds->dep)) {
				tmp.push_back(*itIds);
			}
		}
	}
	result.swap(tmp);
}

void TreeHierarchyFilter::processChildren(list<SortElem> &elems)
{
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		m_found_elems.insert(it->el);
	}
}

TreeHierarchyFilter::TreeHierarchyFilter(const StructuralFilter &filter, const StructuralFilter::InRange &check) :
	m_filter(filter), m_check(check)
{

}

} //palo
