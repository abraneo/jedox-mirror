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

#ifndef __TREEHIERARCHY_FILTER_H_INCL__
#define __TREEHIERARCHY_FILTER_H_INCL__

#include <list>
#include <set>
#include "StructuralFilter.h"
#include "SubSet.h"

namespace palo {

class TreeHierarchyFilter {
public:

	bool check(const SubSet::Iterator &elem);
	void finalize(vector<SubElem> &result);
	void processChildren(list<SortElem> &elems);
	TreeHierarchyFilter(const StructuralFilter &filter, const StructuralFilter::InRange &check);

private:
	const StructuralFilter &m_filter;
	const StructuralFilter::InRange &m_check;
	set<Element *> m_found_elems;
};

} //palo
#endif
