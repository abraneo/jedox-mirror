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

#include "StructuralFilter.h"
#include "SubSet.h"
#include "ListBasicException.h"
#include "TreeBuilder.h"
#include "TreeHierarchyFilter.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace jedox {
namespace palo {

void StructuralFilter::apply()
{
	/*@brief algorithm:
	 -remove elements below/above the bounding element
	 -remove elements not within level-range
	 -hide leaves / aggregations
	 -apply revolve parameters */
	ElementExList& concrete_subset = m_subset_ref.getConcreteSubset();

	if ((m_elem_bound != -1) || m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS)) {

		ElementExList elem_list;

		if ((m_elem_bound != -1)) {
			const ELEMENT_INFO_EXT& elem_inf = m_subset_ref.getDimension()[m_elem_bound].getCacheData();
			elem_list.push_back(elem_inf);
			if (queryFlag(BELOW_EXCLUSIVE)) {
				m_subset_ref.setGlobalFlag(SubSet::BELOW_EXCLUSIVE);
				m_subset_ref.addElemBound(elem_inf);
			} else if (queryFlag(BELOW_INCLUSIVE)) {
				m_subset_ref.setGlobalFlag(SubSet::BELOW_EXCLUSIVE);
				m_subset_ref.addElemBound(elem_inf);
			}
		}

		if (m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS)) {
			switch (m_indent) {
			case 2:
				CheckNotInLevelRange(m_elem_bound == -1 ? m_subset_ref.getTopelements() : elem_list);
				break;
			case 3:
				CheckNotInDepthRange(m_elem_bound == -1 ? m_subset_ref.getTopelements() : elem_list);
				break;
			default:
				CheckNotInIndentRange(m_elem_bound == -1 ? m_subset_ref.getTopelements() : elem_list);
			}
		} else {
			CheckNotInRange(m_elem_bound == -1 ? m_subset_ref.getTopelements() : elem_list);
		}
	}

	if (queryFlag(HIDE_CONSOLIDATED)) {
		ElementExList::iterator it_beg = concrete_subset.begin();
		ElementExList::iterator it_end = concrete_subset.end();
		// erase all elements not in the set, which means they are not below the choosen element
		while (it_beg != it_end) {
			if (it_beg->m_einf.children.size() > 0) {
				it_beg = concrete_subset.erase(it_beg);
			} else {
				++it_beg;
			}
		}
	}

	if (queryFlag(HIDE_LEAVES)) {
		ElementExList::iterator it_beg = concrete_subset.begin();
		ElementExList::iterator it_end = concrete_subset.end();
		// erase all elements not in the set, which means they are not below the chosen element
		while (it_beg != it_end) {
			if (it_beg->m_einf.children.size() == 0) {
				it_beg = concrete_subset.erase(it_beg);
			} else {
				++it_beg;
			}
		}
	}

	if (queryFlag(REVOLVING) || queryFlag(REVOLVE_ADD_ABOVE) || queryFlag(REVOLVE_ADD_BELOW)) {

		m_subset_ref.setGlobalFlag(SubSet::REVOLVE);

		unsigned int level = 0;
		if (!queryFlag(CYCLIC)) {
			level = m_subset_ref.getDimension()[m_elem_revolve].getCacheData().level;
		}

		ElementExList::iterator beg = concrete_subset.begin();
		ElementExList::iterator end = concrete_subset.end();

		if (queryFlag(REVOLVING) && !queryFlag(CYCLIC)) {
			ElementExList::iterator newend = std::remove_if(beg, end, boost::bind(std::not_equal_to<unsigned int>(), level, boost::bind(&ELEMENT_INFO_EXT::get_level, _1)));
			concrete_subset.erase(newend, end);
		}
		// TODO: Do the following: Build a subset for every element on the level with TreeBuilder
		// and then filter the elements.
		else if (queryFlag(REVOLVE_ADD_BELOW)) {
			//add all elements with level <= elem_revolve
			ElementExList::iterator newend = std::remove_if(beg, end, boost::bind(std::less<unsigned int>(), level, boost::bind(&ELEMENT_INFO_EXT::get_level, _1)));
			concrete_subset.erase(newend, end);
		} else if (queryFlag(REVOLVE_ADD_ABOVE)) {
			ElementExList::iterator newend = std::remove_if(beg, end, boost::bind(std::greater<unsigned int>(), level, boost::bind(&ELEMENT_INFO_EXT::get_level, _1)));
			concrete_subset.erase(newend, end);
		}
	}
}

void StructuralFilter::setBound(const std::string& element_name)
{
	if (!queryFlag(BELOW_INCLUSIVE) && !queryFlag(BELOW_EXCLUSIVE) && !queryFlag(ABOVE_EXCLUSIVE) && !queryFlag(ABOVE_INCLUSIVE))
		setFlag(BELOW_INCLUSIVE);
	m_elem_bound = m_subset_ref.getDimension()[element_name].getCacheData().element;
}

void StructuralFilter::setLevels(const string& start, const string& end)
{
	// if no flag has been specified, use AGGREGATED as the default
	if (!queryFlag(HIERARCHIAL_LEVEL) && !queryFlag(AGGREGATED_LEVEL)) {
		setFlag(AGGREGATED_LEVEL);
		m_subset_ref.setGlobalFlag(SubSet::LEVEL_BOUNDS);
	}
	const ELEMENT_INFO& einf_start = m_subset_ref.getDimension()[start].getCacheData();
	const ELEMENT_INFO& einf2_end = m_subset_ref.getDimension()[end].getCacheData();
	switch (m_indent) {
	case 2:
		m_level_begin = einf_start.level;
		m_level_end = einf2_end.level;
		break;
	case 3:
		m_level_begin = einf_start.depth;
		m_level_end = einf2_end.depth;
		break;
	default:
		m_level_begin = einf_start.indent;
		m_level_end = einf2_end.indent;
	}
}

void StructuralFilter::setLevels(const unsigned int& start, const unsigned int& end)
{
	// if no flag has been specified, use AGGREGATED as the default
	if (!queryFlag(HIERARCHIAL_LEVEL) && !queryFlag(AGGREGATED_LEVEL)) {
		setFlag(AGGREGATED_LEVEL);
		m_subset_ref.setGlobalFlag(SubSet::LEVEL_BOUNDS);
	}
	m_level_begin = start;
	m_level_end = end;
}

void StructuralFilter::setRevolveParams(const std::string& elemname, const unsigned int& count)
{
	if (count <= 0)
		throw ListBasicException("The size of the revolving list is set to be zero or less", "Revolving Filter usage error");
	// if no flag has been set, use REVOLVING as the default flag.
	if (!queryFlag(REVOLVING) && !queryFlag(REVOLVE_ADD_ABOVE) && !queryFlag(REVOLVE_ADD_BELOW)) {
		setFlag(REVOLVING);
	}

	m_revolve_count = count;
	if (!elemname.empty())
		m_elem_revolve = m_subset_ref.getDimension()[elemname].getCacheData().element;
	else
		// if the element name is empty but the count is not 0 or less, we do a simple repeat of the list
		// without removing further elements
		setFlag(CYCLIC);
}

StructuralFilter::StructuralFilter(SubSet& s, unsigned long int flags) :
	Filter(s, flags, STRUCTURAL_FILTER_NUMFLAGS, Filter::STRUCTURAL), m_elem_bound(-1L), m_elem_revolve(-1), m_revolve_count(0), m_indent(0)
{

	if (queryFlag(BELOW_EXCLUSIVE) && queryFlag(BELOW_INCLUSIVE)) {
		throw jedox::palo::ListBasicException("Conflicting flags BELOW_EXCLUSIVE and BELOW_INCLUSIVE set", "Wrong strucural filter usage");
	}
	if (queryFlag(HIDE_LEAVES) && queryFlag(HIDE_CONSOLIDATED)) {
		throw jedox::palo::ListBasicException("Conflicting flags HIDE_LEAVES and HIDE_CONSOLIDATED set", "Wrong stuctural filter usage");
	}
	if (queryFlag(HIERARCHIAL_LEVEL) && queryFlag(AGGREGATED_LEVEL)) {
		throw jedox::palo::ListBasicException("Conflicting flags HIERARCHIAL and AGGREGATED_LEVEL set", "Wrong stuctural filter usage");
	}
	if (queryFlag(REVOLVE_ADD_ABOVE) && queryFlag(REVOLVE_ADD_BELOW)) {
		throw jedox::palo::ListBasicException("Conflicting flags REVOLVE_ADD_ABOVE and REVOLVE_ADD_BELOW set", "Wrong strucural filter usage");
	}
	m_subset_ref.setGlobalFlag(SubSet::STRUCTURAL_FILTER_ACTIVE);
}

StructuralFilter::~StructuralFilter()
{
}

unsigned int StructuralFilter::getRevolveCount()
{
	return m_revolve_count;
}

void StructuralFilter::setIndent(const unsigned int indent)
{
	m_indent = indent;
}
void StructuralFilter::CheckNotInRange(const ElementExList& elem_list)
{
	InRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(m_subset_ref.getConcreteSubset(), *this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, elem_list, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	m_subset_ref.swapConcreteSet(builder.build());
}

void StructuralFilter::CheckNotInLevelRange(const ElementExList& elem_list)
{
	InLevelRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(m_subset_ref.getConcreteSubset(), *this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, elem_list, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	m_subset_ref.swapConcreteSet(builder.build());
}
void StructuralFilter::CheckNotInDepthRange(const ElementExList& elem_list)
{
	InDepthRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(m_subset_ref.getConcreteSubset(), *this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, elem_list, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	m_subset_ref.swapConcreteSet(builder.build());

}

void StructuralFilter::CheckNotInIndentRange(const ElementExList& elem_list)
{
	InIndentRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(m_subset_ref.getConcreteSubset(), *this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, elem_list, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	m_subset_ref.swapConcreteSet(builder.build());
}

} //palo
} //jedox
