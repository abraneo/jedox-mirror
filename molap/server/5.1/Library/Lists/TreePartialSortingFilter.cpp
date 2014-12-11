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

#include "TreePartialSortingFilter.h"
#include "SortingFilter.h"
#include <algorithm>
#include <boost/ref.hpp>
#include <boost/bind.hpp>

namespace palo {

void  SDTreePartialSortingFilter::finalize(vector<SubElem> &result) {
}

void  SDTreePartialSortingFilter::processChildren(list<SortElem> &elems) {
	elems.sort( boost::bind( &AbstractComparison::operator(),boost::ref( m_comp ), _1, _2 ) );
	if ( m_filter.m_parents_below ) {
		elems.reverse();
	}
}

SDTreePartialSortingFilter::SDTreePartialSortingFilter(const SortingFilter& filter, AbstractComparison& comp)
	: m_filter(filter), m_comp(comp)
{
}


void  TreePartialSortingFilter::finalize(vector<SubElem> &result) {
	std::set<long> processed_ids;
	vector<SubElem> tmp;
	tmp.reserve(result.size());
	for (vector<SubElem>::iterator itIds = result.begin(); itIds != result.end(); ++itIds) {
		if (processed_ids.find(itIds->elem->getIdentifier()) == processed_ids.end()) {
			processed_ids.insert(itIds->elem->getIdentifier());
			tmp.push_back(*itIds);
		}
	}
	result.swap(tmp);
}

void  TreePartialSortingFilter::processChildren(list<SortElem> &elems) {
	elems.sort( boost::bind( &AbstractComparison::operator(),boost::ref( m_comp ), _1, _2 ) );
	if ( m_filter.m_parents_below ) {
		elems.reverse();
	}
}

TreePartialSortingFilter::TreePartialSortingFilter(const SortingFilter& filter, AbstractComparison& comp)
	: m_filter(filter), m_comp(comp) {
}

} //palo
