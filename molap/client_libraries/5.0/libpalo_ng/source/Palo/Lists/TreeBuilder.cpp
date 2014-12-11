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

#include <boost/shared_ptr.hpp>

#include "TreeBuilder.h"
#include "SubSet.h"
#include "TreeHierarchyFilter.h"
#include "TreeFullSortingFilter.h"
#include "TreePartialSortingFilter.h"

using namespace jedox::palo;

namespace jedox {
namespace palo {
// Constructor implementation
template<class Algorithm>
TreeBuilder<Algorithm>::TreeBuilder(Algorithm& strat, const ElementExList& li, SubSet& subset) :
	m_reverse_active(false), m_alg(strat), m_elements(li), m_subset_ref(subset)
{
}

// Destructor implementation
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
ElementExList& TreeBuilder<Algorithm>::build()
{
	//process the top list, which might contain more than
	//one element and evtl. has to be sorted
	m_alg.processChildren(m_elements);

	ElementExList::iterator head_it = m_elements.begin();
	while (head_it != m_elements.end()) {
		process(head_it);
	}
	/**apply finalize of our Algorithm to perform arbitrary
	 * post-processing */
	m_alg.finalize(m_elements);
	return m_elements;
}

//TODO : we do not mark children for deletion
template<class Algorithm>
void TreeBuilder<Algorithm>::process(ElementExList::iterator& head)
{
	/** apply the check-function of the Algorithm -- do we keep
	 *  the element ?*/
	//indent of the following elements
	int indent, depth;
	indent = head->m_einf.indent + 1;
	depth = head->m_einf.depth + 1;
	if ("" == head->path) { //path hasn't been set for this element
		head->path = head->get_name();
	}

	if (m_reverse_active ? !head->m_einf.parents.empty() : !head->m_einf.children.empty()) {
		vector<ELEMENT_INFO_EXT> l;
		ELEMENT_LIST::iterator it_start;
		ELEMENT_LIST::iterator it_end;
		if (!m_reverse_active) {
			it_start = head->m_einf.children.begin();
			it_end = head->m_einf.children.end();
		} else {
			it_start = head->m_einf.parents.begin();
			it_end = head->m_einf.parents.end();
		}
		l.reserve(it_end - it_start);
		while (it_start != it_end) {
			try {
				ELEMENT_INFO_EXT tmp;
				const ELEMENT_INFO_EXT *e = m_subset_ref.getElem(*it_start);
				if (!e) {
					e = &tmp;
					tmp = m_subset_ref.getDimElement(*it_start);
				}
				if (m_alg.check(*e)) {
					size_t s = l.size();
					l.push_back(*e);
					ELEMENT_INFO_EXT &exinf = l[s];
					exinf.path = head->path + "\\" + exinf.get_name();

					if (!m_reverse_active) {
						exinf.m_einf.indent = indent;
						exinf.m_einf.depth = depth;
						exinf.cons_order = (int)s;
						exinf.curr_parent = head->m_einf.element;
					}
				}
				++it_start;
			} catch (const ElementNotFoundException&) {
				if (!m_reverse_active) {
					it_start = head->m_einf.children.erase(it_start);
					it_end = head->m_einf.children.end();
				} else {
					it_start = head->m_einf.parents.erase(it_start);
					it_end = head->m_einf.parents.end();
				}
			}
		}

		ElementExList t_list(l.begin(), l.end());
		ElementExList::iterator front_it = head;
		++front_it;

		//apply additional processing
		m_alg.processChildren(t_list);
		//insert into final list
		m_elements.insert(front_it, t_list.begin(), t_list.end());
	}
	++head;
}

} //palo
} //jedox


template class TreeBuilder<TreeHierarchyFilter>;
template class TreeBuilder<SDTreeFullSortingFilter>;
template class TreeBuilder<TreeFullSortingFilter>;
template class TreeBuilder<SDTreePartialSortingFilter>;
template class TreeBuilder<TreePartialSortingFilter>;
