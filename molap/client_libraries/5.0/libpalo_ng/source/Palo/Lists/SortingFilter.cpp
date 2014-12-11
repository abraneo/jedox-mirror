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

#include <string>
#include <set>

#include <libpalo_ng/Palo/types.h>

#include "StructuralFilter.h"
#include "SortingFilter.h"
#include "TreeFullSortingFilter.h"
#include "TreePartialSortingFilter.h"
#include "PickList.h"
#include "SubSet.h"
#include "TreeBuilder.h"
#include "ListBasicException.h"

#define _sortingfilter_set_type_selector2_(abstract_comparison1, abstract_comparison2) \
	boost::scoped_ptr<AbstractComparison > comp;\
	switch ( m_indent ){\
	case 2:\
		comp.reset( new abstract_comparison1< abstract_comparison2, LevelSelector > ( m_level, m_element_type, m_reverse_order  ) );\
		break;\
	case 3:\
		comp.reset( new abstract_comparison1< abstract_comparison2, DepthSelector > ( m_level, m_element_type, m_reverse_order  ) );\
		break;\
	default:\
		comp.reset( new abstract_comparison1< abstract_comparison2, IndentSelector > ( m_level, m_element_type, m_reverse_order ) );\
		break;\
	}

#define _sortingfilter_set_type_selector1_(abstract_comparison)\
	boost::scoped_ptr<abstract_comparison > comp;\
	comp.reset( new abstract_comparison( m_element_type, m_reverse_order )  );

#define _sortingfilter_set_type_selector_flat_(abstract_comparison)\
	boost::scoped_ptr<abstract_comparison > comp;\
	comp.reset( new abstract_comparison( m_element_type, m_reverse_order ) );

namespace jedox {
namespace palo {

/*@brief
 If POSITIONAL --> arrange positional
 else if HIERARCHICAL --> arrange hierarchical
 Now sort,maintaining position and hierarchy,either all or leaves only.
 (later we support arbitrary levels and sorting level only. Easy to integrate)*/

class change_indent : public unary_function<ELEMENT_INFO_EXT &, void> {
public:
	void operator()(ELEMENT_INFO_EXT &e)
	{
		e.m_einf.indent = 0;
	}
};

void SortingFilter::apply()
{
	ElementExList& concrete_subset = m_subset_ref.getConcreteSubset();
	if ((!m_subset_ref.queryGlobalFlag(SubSet::DATA_FILTER_ACTIVE)) && queryFlag(NUMERIC)) {
		throw ListBasicException("Flag NUMERIC set but data filter unused.", "Wrong Sorting Filter usage");
	}
	// If we sort according to data values, we have to consider the leaves/consolidated
	//flag of the data-filter
	if (m_subset_ref.queryGlobalFlag(SubSet::DATA_ONLY_CONSOLIDATED) && queryFlag(NUMERIC)) {
		setFlag((unsigned long)CONSOLIDATED_ONLY);
	} else if (m_subset_ref.queryGlobalFlag(SubSet::DATA_ONLY_LEAVES) && queryFlag(NUMERIC)) {
		setFlag((unsigned long)LEAVES_ONLY);
	}
	if (m_subset_ref.queryGlobalFlag(SubSet::REVOLVE)) {
		m_subset_ref.setGlobalFlag(SubSet::REVERSE);
	}
	if (queryFlag(LEAVES_ONLY)) {
		m_element_type = 1;
	} else if (queryFlag(CONSOLIDATED_ONLY)) {
		m_element_type = 2;
	}
	Filter::apply();
	//insert pick list elements now if merge is set
	//merge in time O(n*log(n)) (sort)
	if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_MERGE)) {
		concrete_subset.sort();
		m_subset_ref.getPicklist()->getElements().sort();
		concrete_subset.merge(m_subset_ref.getPicklist()->getElements());
		concrete_subset.unique();
	}

	//translate the reverse flags this can be removed once they are named properly
	if (queryFlag(REVERSE_ORDER)) {
		m_parents_below = true;
	}
	if (queryFlag(REVERSE_TOTAL)) {
		m_reverse_order = true;
	}

	if (queryFlag(SORT_ONE_LEVEL)) {

		if (queryFlag(NUMERIC)) {
			sortLevelNumeric();
		} else if (queryFlag(USE_ATTRIBUTE)) {
			sortLevelAttribute();
		} else if (queryFlag(USE_ALIAS)) {
			sortLevelAlias();
		} else if (queryFlag(SortingFilterBase::TEXT)) {
			sortLevelText();
		} else if (queryFlag(CONSOLIDATION_ORDER)) {
			sortLevelConsolidation();
		} else {
			sortDefinition();
		}

	} else if (queryFlag(SORT_NOT_ONE_LEVEL)) {

		if (queryFlag(NUMERIC)) {
			sortNotLevelNumeric();
		} else if (queryFlag(USE_ATTRIBUTE)) {
			sortNotLevelAttribute();
		} else if (queryFlag(USE_ALIAS)) {
			sortNotLevelAlias();
		} else if (queryFlag(SortingFilterBase::TEXT)) {
			sortNotLevelText();
		} else if (queryFlag(CONSOLIDATION_ORDER)) {
			sortNotLevelConsolidation();
		} else {
			sortDefinition();
		}

	} else if (queryFlag(WHOLE)) {
		if (queryFlag(NUMERIC)) {
			sortFullNumeric();
		} else if (queryFlag(USE_ATTRIBUTE)) {
			sortFullAttribute();
		} else if (queryFlag(USE_ALIAS)) {
			sortFullAlias();
		} else if (queryFlag(SortingFilterBase::TEXT)) {
			sortFullText();
		} else if (queryFlag(CONSOLIDATION_ORDER)) {
			sortFullConsolidation();
		} else {
			sortDefinition();
		}
	}

	// flat hierarchy is default
	else if (queryFlag(FLAT_HIERARCHY) || !queryFlag(WHOLE)) {
		if (queryFlag(NUMERIC)) {
			sortFlatNumeric();
		} else if (queryFlag(USE_ATTRIBUTE)) {
			sortFlatAttribute();
		} else if (queryFlag(USE_ALIAS)) {
			sortFlatAlias();
		} else if (queryFlag(SortingFilterBase::TEXT)) {
			sortFlatText();
		} else {
			sortFlatDefinition();
		}
		if (!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES)) {
			concrete_subset.unique();
		}
	}
	if (m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS)) {
		m_subset_ref.applyStructuralBounds();
	}

	// first, apply picklist front / back
	if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_FRONT)) {
		concrete_subset.insert(concrete_subset.begin(), m_subset_ref.getPicklist()->getElements().begin(), m_subset_ref.getPicklist()->getElements().end());
	} else if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_BACK)) {
		concrete_subset.insert(concrete_subset.end(), m_subset_ref.getPicklist()->getElements().begin(), m_subset_ref.getPicklist()->getElements().end());
	} else if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_SUB)) {
		subtract(concrete_subset, m_subset_ref.getPicklist()->getElements());
	}

	//then append elements until revolve-count is reached.
	if (m_subset_ref.queryGlobalFlag(SubSet::REVOLVE)) {
		unsigned int count = m_subset_ref.getStrucuralFilter()->getRevolveCount();
		size_t index = concrete_subset.size();
		if (index >= count) {
			concrete_subset.resize(count);
		} else {
			std::back_insert_iterator<ElementExList> back_it(concrete_subset);
			ElementExList::const_iterator stop_it = concrete_subset.end();
			ElementExList::const_iterator repeat_it = concrete_subset.begin();
			if (repeat_it != stop_it) {
				while (index < count) {
					*back_it = *repeat_it;
					++back_it;
					++index;
					++repeat_it;
					if (repeat_it == stop_it) {
						repeat_it = concrete_subset.begin();
					}
				}
			}
		}
	}

	//XOR between those two, because reverse twice (if both are set) would be silly ;-)
	if (queryFlag(REVERSE_TOTAL_EX) ^ m_parents_below) {
		concrete_subset.reverse();
	}

	if (queryFlag(LIMIT)) {
		ElementExList::iterator start = concrete_subset.begin();
		for (unsigned int i = 0; i < m_limit_start && start != concrete_subset.end(); i++, start++);
		ElementExList::iterator end = start;
		for (unsigned int i = 0; i < m_limit_count && end != concrete_subset.end(); i++, end++);
		ElementExList res;
		res.splice(res.begin(), concrete_subset, start, end);
		concrete_subset = res;
	}

	if (queryFlag(FLAT_HIERARCHY) || !queryFlag(WHOLE)) {
		for_each(concrete_subset.begin(), concrete_subset.end(), change_indent());
	}
}

void SortingFilter::subtract(ElementExList& a, const ElementExList& b)
{
	ElementExList& concrete_subset = m_subset_ref.getConcreteSubset();
	ElementExList::iterator a_it = a.begin();
	ElementExList::iterator a_end = a.end();
	ElementExList::const_iterator b_it;// = b.begin();
	ElementExList::const_iterator b_end = b.end();
	bool found;
	while (a_it != a_end) {
		b_it = m_subset_ref.getPicklist()->getElements().begin();
		found = false;
		while (b_it != b_end && a_it != a_end) {
			if (*a_it == *b_it) {
				found = true;
				++a_it;
				b_it = m_subset_ref.getPicklist()->getElements().begin();
			} else {
				++b_it;
			}
		}
		if (false == found) {
			a_it = concrete_subset.erase(a_it);
		}
	}
}
SortingFilter::SortingFilter(SubSet& s, unsigned long int flags) :
	Filter(s, flags, SORTING_FILTER_NUM_FLAGS, Filter::SORTING), m_locale(std::locale("")), m_indent(0), m_element_type(0), m_level(-1), m_parents_below(false), m_reverse_order(false), m_limit_start(0), m_limit_count(0)
{
	/*@TODO check consistency of other flags many checks missing*/
	// only one hierarchy type possible
	if (queryFlag(WHOLE) && queryFlag(FLAT_HIERARCHY)) {
		throw ListBasicException("Conflicting filter flags WHOLE and FLAT_HIERARCHY", "Wrong Sorting Filter usage");
	}
	//Aliases don't have a position / definition number
	if (queryFlag(POSITION) && queryFlag(ALIAS)) {
		throw ListBasicException("Conflicting filter flags POSITION and ALIAS", "Wrong Sorting Filter usage");
	}
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(LEAVES_ONLY)) {
		throw ListBasicException("Cannot sort leaves in a flat hierarchy. Conflicting flags FLAT_HIERARCHY and LEAVES_ONLY", "Wrong Sorting Filter usage");
	}
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(NO_CHILDREN))
		throw ListBasicException("Conflicting filter flags FLAT_HIERARCHY and NO_CHILDREN", "Wrong Sorting Filter usage");
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(CONSOLIDATED_ONLY))
		throw ListBasicException("Conflicting filter flags FLAT_HIERARCHY and CONSOLIDATED_ONLY", "Wrong Sorting Filter usage");
	if (queryFlag(SORT_ONE_LEVEL) && !(queryFlag(WHOLE) || queryFlag(NO_CHILDREN)))
		throw ListBasicException("Sorting on one level is only possible with a hierarchical sort.", "Wrong Sorting Filter usage");
}

void SortingFilter::setAttribute(const std::string& attr)
{
	/*@TODO check attribute for validity.*/
	if (!queryFlag(USE_ATTRIBUTE))
		setFlag(USE_ATTRIBUTE);
	m_sort_attribute = m_subset_ref.validateAttribute(attr);
}

void SortingFilter::setLevelElement(const std::string& elem)
{
	m_level = m_subset_ref.getDimension()[elem].getCacheData().level;
}

void SortingFilter::setSingleLevel(unsigned int level)
{
	m_level = level;
}

void SortingFilter::setIndent(unsigned int indent)
{
	m_indent = indent;
}

void SortingFilter::setLimit(unsigned int start, unsigned int count)
{
	if (count && !queryFlag(LIMIT)) {
		setFlag(LIMIT);
	}
	m_limit_start = start;
	m_limit_count = count;
}

void SortingFilter::sortFlatNumeric()
{
	if (!m_subset_ref.queryGlobalFlag(SubSet::DATA_STRING)) {
		_sortingfilter_set_type_selector_flat_(DataComparison);
		m_subset_ref.getConcreteSubset().sort(*comp);
	} else {
		_sortingfilter_set_type_selector_flat_(StringDataComparison);
		m_subset_ref.getConcreteSubset().sort(*comp);
	}
}

void SortingFilter::sortFlatAttribute()
{
	//build the mapping !!
	computeAliases();
	_sortingfilter_set_type_selector_flat_(AttributeComparison);
	m_subset_ref.getConcreteSubset().sort(*comp);
}

void SortingFilter::sortFlatAlias()
{
	_sortingfilter_set_type_selector_flat_(AliasComparison);
	m_subset_ref.getConcreteSubset().sort(*comp);
}

void SortingFilter::sortFlatDefinition()
{
	_sortingfilter_set_type_selector_flat_(PositionalComparison);
	m_subset_ref.getConcreteSubset().sort(*comp);
}

void SortingFilter::sortFlatText()
{
	_sortingfilter_set_type_selector_flat_(NameComparison);
	m_subset_ref.getConcreteSubset().sort(*comp);
}

void SortingFilter::sortDefinition()
{
	_sortingfilter_set_type_selector1_(PositionalComparison);
	buildTreeAndSort(*comp);

}

void SortingFilter::sortFullNumeric()
{
	if (!m_subset_ref.queryGlobalFlag(SubSet::DATA_STRING)) {
		_sortingfilter_set_type_selector1_(DataComparison);
		buildTreeAndSort(*comp);
	} else {
		_sortingfilter_set_type_selector1_(StringDataComparison);
		buildTreeAndSort(*comp);
	}
}

void SortingFilter::sortFullAttribute()
{
	computeAliases();
	_sortingfilter_set_type_selector1_(AttributeComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortFullAlias()
{
	if (!(m_subset_ref.queryGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE))) {
		throw ListBasicException("Trying to sort according to alias but no alias-filter in use.", "Wrong Sorting Filter usage");
	}
	_sortingfilter_set_type_selector1_(AliasComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortFullDefinition()
{
	_sortingfilter_set_type_selector1_(PositionalComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortFullText()
{
	_sortingfilter_set_type_selector1_(NameComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortFullConsolidation()
{
	_sortingfilter_set_type_selector1_(ConsolidationComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortLevelNumeric()
{
	if (!m_subset_ref.queryGlobalFlag(SubSet::DATA_STRING)) {
		_sortingfilter_set_type_selector2_(OneLevelComparison,DataComparison);
		buildTreeAndSort(*comp);
	} else {
		_sortingfilter_set_type_selector2_(OneLevelComparison,StringDataComparison);
		buildTreeAndSort(*comp);
	}
}

void SortingFilter::sortLevelAttribute()
{
	_sortingfilter_set_type_selector2_(OneLevelComparison,AttributeComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortLevelAlias()
{
	_sortingfilter_set_type_selector2_(OneLevelComparison,AliasComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortLevelText()
{
	_sortingfilter_set_type_selector2_(OneLevelComparison,NameComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortLevelConsolidation()
{
	_sortingfilter_set_type_selector2_(OneLevelComparison,ConsolidationComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortNotLevelNumeric()
{
	if (!m_subset_ref.queryGlobalFlag(SubSet::DATA_STRING)) {
		_sortingfilter_set_type_selector2_(NotOneLevelComparison,DataComparison);
		buildTreeAndSort(*comp);

	} else {
		_sortingfilter_set_type_selector2_(NotOneLevelComparison,StringDataComparison);
		buildTreeAndSort(*comp);
	}
}

void SortingFilter::sortNotLevelAttribute()
{
	_sortingfilter_set_type_selector2_(NotOneLevelComparison,AttributeComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortNotLevelAlias()
{
	_sortingfilter_set_type_selector2_(NotOneLevelComparison,AliasComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortNotLevelText()
{
	_sortingfilter_set_type_selector2_(NotOneLevelComparison,NameComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::sortNotLevelConsolidation()
{
	_sortingfilter_set_type_selector2_(NotOneLevelComparison,ConsolidationComparison);
	buildTreeAndSort(*comp);
}

void SortingFilter::computeAliases()
{
	ElementExList& concrete_subset = m_subset_ref.getConcreteSubset();
	if (-1 == m_sort_attribute) {
		throw ListBasicException("Trying to sort according to attribute but no attribute-string is set.", "Wrong Sorting Filter usage");
	}
	CELL_VALUE cell_val;
	Cube cube = m_subset_ref.getAttributeCube();
	std::vector<std::vector<long> > coord_vec;
	std::vector<long> coordinates(2);
	coordinates[0] = m_sort_attribute;
	coord_vec.reserve(coordinates.size());
	for (ElementExList::iterator it = concrete_subset.begin(), end_it = concrete_subset.end(); it != end_it; ++it) {
		try {
			coordinates[1] = it->get_id();
			cube.MDXCellValue(cell_val, coordinates);
			if (cell_val.exists) {
				if (cell_val.type == CELL_VALUE::NUMERIC) {
					it->sorting_alias = cell_val.val.d;
				} else {
					it->sorting_alias = cell_val.val.s;
				}
			} else {
				if (cell_val.type == CELL_VALUE::NUMERIC) {
					it->sorting_alias = 0.0;
				} else {
					it->sorting_alias = it->get_name();
				}
			}
		} catch (const jedox::palo::PaloException&) {
			it->sorting_alias = it->get_name();
		}
		coord_vec.push_back(coordinates);
	}
}

void SortingFilter::buildTreeAndSort(AbstractComparison& comp)
{
	ElementExList& concrete_subset = m_subset_ref.getConcreteSubset();
	if (queryFlag(NO_CHILDREN)) {
		if (queryFlag(SHOW_DUPLICATES) && !m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES)) {
			SDTreePartialSortingFilter filter(concrete_subset, *this, comp);
			TreeBuilder<SDTreePartialSortingFilter> builder(filter, get_top_elems(true), m_subset_ref);
			m_subset_ref.swapConcreteSet(builder.build());
		} else {
			TreePartialSortingFilter filter(concrete_subset, *this, comp);
			TreeBuilder<TreePartialSortingFilter> builder(filter, get_top_elems(true), m_subset_ref);
			m_subset_ref.swapConcreteSet(builder.build());
		}
	} else {
		if (queryFlag(SHOW_DUPLICATES) && !m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES)) {
			SDTreeFullSortingFilter filter(m_subset_ref, *this, comp);
			TreeBuilder<SDTreeFullSortingFilter> builder(filter, get_top_elems(false), m_subset_ref);
			m_subset_ref.swapConcreteSet(builder.build());
		} else {
			TreeFullSortingFilter filter(m_subset_ref, *this, comp);
			TreeBuilder<TreeFullSortingFilter> builder(filter, get_top_elems(false), m_subset_ref);
			m_subset_ref.swapConcreteSet(builder.build());
		}
	}
}

const ElementExList& SortingFilter::get_top_elems(bool nochildren) const
{
	bool bound = m_subset_ref.queryGlobalFlag(SubSet::BELOW_EXCLUSIVE) || m_subset_ref.queryGlobalFlag(SubSet::BELOW_INCLUSIVE);
	m_subset_ref.updateTop(nochildren, bound);
	if (bound) {
		return m_subset_ref.getElemBound();
	}
	return m_subset_ref.getTopelements();
}
} //palo
} //jedox
