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

#include <boost/shared_ptr.hpp>

#include "TreeBuilder.h"
#include "SubSet.h"
#include "TreeHierarchyFilter.h"
#include "TreeFullSortingFilter.h"
#include "TreePartialSortingFilter.h"

namespace palo {

template<class Algorithm>
TreeBuilder<Algorithm>::TreeBuilder(Algorithm& strat, SubSet::Iterator &it_beg, SubSet& subset) :
	m_reverse_active(false), m_alg(strat), m_beg(it_beg), m_subset_ref(subset)
{
}

template<class Algorithm>
TreeBuilder<Algorithm>::~TreeBuilder()
{
}

template<class Algorithm>
void TreeBuilder<Algorithm>::setReverse(bool b)
{
	m_reverse_active = b;
}

template<class Algorithm>
void TreeBuilder<Algorithm>::build(vector<SubElem> &result, bool usePath)
{
	list<SortElem> top;
	for (; !m_beg.end(); ++m_beg) {
		top.push_back(SortElem(&m_subset_ref, m_beg.getElement(), m_beg.getIndent(), m_beg.getDepth(), m_beg.getConsOrder()));
	}
	m_alg.processChildren(top);
	for (list<SortElem>::iterator it = top.begin(); it != top.end(); ++it) {
		string path = usePath ? StringUtils::convertToString(it->el->getIdentifier()) : "";
		result.push_back(SubElem(it->el, it->ind, it->dep, path));
		process(result, *it, path);
	}
	m_alg.finalize(result);
}

template<class Algorithm>
void TreeBuilder<Algorithm>::process(vector<SubElem> &result, const SortElem &elem, const string &path)
{
	if (m_reverse_active ? elem.el->getParentsCount() : elem.el->getChildrenCount()) {
		IdentifierType indent, depth;
		indent = elem.ind + 1;
		depth = elem.dep + 1;

		list<SortElem> elems;
		for (SubSet::Iterator it = m_reverse_active ? m_subset_ref.parentsbegin(elem.el) : m_subset_ref.childrenbegin(elem.el); !it.end(); ++it) {
			if (m_alg.check(it)) {
				elems.push_back(SortElem(&m_subset_ref, it.getElement(), indent, depth, it.getConsOrder()));
			}
		}
		m_alg.processChildren(elems);

		for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
			string pathn = path.empty() ? "" : path + "/" + StringUtils::convertToString(it->el->getIdentifier());
			result.push_back(SubElem(it->el, indent, depth, pathn));
			process(result, *it, pathn);
		}
	}
}

template class TreeBuilder<TreeHierarchyFilter>;
template class TreeBuilder<SDTreeFullSortingFilter>;
template class TreeBuilder<TreeFullSortingFilter>;
template class TreeBuilder<SDTreePartialSortingFilter>;
template class TreeBuilder<TreePartialSortingFilter>;

} //palo
