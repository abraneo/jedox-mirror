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

#ifndef __SORTINGFILTER_H_INCL__
#define __SORTINGFILTER_H_INCL__

#include <string>
#include <map>
#include <locale>
#include <string.h>

#include <libpalo_ng/config_ng.h>

#include "Filter.h"
#include "SubSet.h"

#include <libpalo_ng/Util/StringUtils.h>

namespace jedox {
namespace palo {

/**@brief AbstractSelector, small class that returns true if the
 * @param ELEMENT_INFO_EXT& e
 * is the right kind, i.e. Base Element for the BaseSelector,
 * consolidated for ConsolidatedSelector ...
 */
class AbstractSelector {
public:

	virtual bool operator()(const ELEMENT_INFO_EXT& e) = 0;
};

class AllSelector : public AbstractSelector {
public:
	inline bool operator()(const ELEMENT_INFO_EXT& e)
	{
		return true;
	}
};

class BaseSelector : public AbstractSelector {
public:
	inline bool operator()(const ELEMENT_INFO_EXT& e)
	{
		return e.m_einf.children.size() == 0;
	}
};
class ConsolidatedSelector : public AbstractSelector {
public:
	inline bool operator()(const ELEMENT_INFO_EXT& e)
	{
		return e.m_einf.children.size() > 0;
	}
};

class AbstractNegator {
public:
	virtual bool operator()(bool b) = 0;
};
class Negator : public AbstractNegator {
public:
	inline bool operator()(bool b)
	{
		return !b;
	}
};
class NotNegator : public AbstractNegator {
public:
	inline bool operator()(bool b)
	{
		return b;
	}
};

/**@brief AbstractEnumerationSelector, small class that returns the
 * right kind of "level" of
 * @param ELEMENT_INFO_EXT& e
 *  i.e. "depth" for the DepthSelector ...
 */
class AbstractEnumerationSelector {
public:
	virtual inline unsigned int operator()(const ELEMENT_INFO_EXT& e) = 0;
};

class IndentSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const ELEMENT_INFO_EXT& e)
	{
		return e.m_einf.indent;
	}
};
class LevelSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const ELEMENT_INFO_EXT& e)
	{
		return e.get_level();
	}
};
class DepthSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const ELEMENT_INFO_EXT& e)
	{
		return e.m_einf.depth;
	}
};

class AbstractComparison {
public:
	AbstractComparison(unsigned int element_type = 0, bool reverse = false)
	{
		switch (element_type) {
		case 0:
			m_type.reset(new AllSelector());
			break;
		case 1:
			m_type.reset(new BaseSelector());
			break;
		case 2:
			m_type.reset(new ConsolidatedSelector());
			break;
		}
		if (reverse) {
			m_neg.reset(new Negator());
		} else {
			m_neg.reset(new NotNegator());
		}
	}
	virtual bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2) = 0;
protected:
	boost::shared_ptr<AbstractSelector> m_type;
	boost::shared_ptr<AbstractNegator> m_neg;

};

class NameComparison : public AbstractComparison {
public:
	NameComparison(unsigned int element_type = 0, bool reverse = false) :
		AbstractComparison(element_type, reverse)
	{
	}

	bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return (*m_neg)(util::UTF8Comparer::compare(e1.get_name(), e2.get_name()) < 0);
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

class PositionalComparison : public AbstractComparison {
public:
	PositionalComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return (*m_neg)(e1.get_position() < e2.get_position());
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

class AliasComparison : public AbstractComparison {
	jedox::util::UTF8Comparer cmp;
public:
	AliasComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return (*m_neg)(e1.search_alias < e2.search_alias);
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

class DataComparison : public AbstractComparison {
public:
	DataComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return memcmp(&e1.value, &e2.value, sizeof(e1.value)) ? (*m_neg)(e1.value < e2.value) : e1.get_position() < e2.get_position();
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

class StringDataComparison : public AbstractComparison {
	jedox::util::UTF8Comparer cmp;
public:
	StringDataComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return (*m_neg)(util::UTF8Comparer::compare(e1.string_value, e2.string_value) < 0);
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

//NOTE: we assume every element contained in the map
class AttributeComparison : public AbstractComparison {
	jedox::util::UTF8Comparer cmp;
public:
	AttributeComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if ((*m_type)(e1) && (*m_type)(e2)) {
			return (*m_neg)(e1.sorting_alias < e2.sorting_alias);
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

class ConsolidationComparison : public AbstractComparison {
public:
	ConsolidationComparison(unsigned int element_type = 0, bool reverse = false) :
		AbstractComparison(element_type, reverse)
	{
	}

	bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		if (e1.cons_order != -1 && e2.cons_order != -1 && e1.curr_parent == e2.curr_parent) {
			return e1.cons_order < e2.cons_order;
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
};

template<class abstract_comparison, class IndentSelect = IndentSelector>
class OneLevelComparison : public AbstractComparison {
public:
	OneLevelComparison(unsigned int level, unsigned int element_type, bool reverse) :
		m_sorter(element_type, reverse)
	{
		//TODO maybe later this will work with other kinds of indention,
		//right now the indent of an element is used for comparison.
		m_level = level;
		//m_sorter = s;
	}
	virtual inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		//assert( m_indent(e1) == m_indent(e1) );
		if (m_indent(e1) == m_level) {
			return m_sorter(e1, e2);
		} else {
			return e1.get_position() < e2.get_position();
		}
		/*assert( e1.get_level() == e2.get_level() );
		 if ( e1.get_level() == m_level ) {
		 return m_sorter( e1, e2 );
		 } else {
		 return e1.get_position() < e2.get_position();
		 }*/
	}
private:
	unsigned int m_level;
	abstract_comparison m_sorter;
	IndentSelect m_indent;
};

template<class abstract_comparison, class IndentSelect = IndentSelector>
class NotOneLevelComparison : public AbstractComparison {
public:
	NotOneLevelComparison(unsigned int level, unsigned int element_type, bool reverse) :
		m_sorter(element_type, reverse)
	{
		m_level = level;
		//m_sorter = s;
	}
	virtual inline bool operator()(const ELEMENT_INFO_EXT& e1, const ELEMENT_INFO_EXT& e2)
	{
		assert( e1.m_einf.indent == e2.m_einf.indent );
		if (m_indent(e1) != m_level) {
			return m_sorter(e1, e2);
		} else {
			return e1.get_position() < e2.get_position();
		}
	}
private:
	unsigned int m_level;
	abstract_comparison m_sorter;
	IndentSelect m_indent;
};

// SORTING_FILTER_NUMFLAGS is defined int types.h
// SortingFilterBase is defined in types.h

/**@brief The sorting filter is responsible for the basic order of
 the list (hierarchical or positional) as well as for additional
 changes like sorting elements on a certain level alphabetically */
class SortingFilter : public SortingFilterBase, public Filter {

	long m_sort_attribute;
	//std::string m_sort_elem;
	std::locale m_locale;
	//the "kind" of level we use (i.e. 1 == indent, 2 == level, 3 == depth)
	unsigned int m_indent;
	//the node type used when sorting (i.e. 0 == all elements,
	//1 == only base elements will be sorted,
	//2 == only consolidated elements will be sorted)
	unsigned int m_element_type;
	//the level we use when sorting
	int m_level;
	//sort parents below their children
	bool m_parents_below;
	//reverse the sorting of elements
	bool m_reverse_order;
	unsigned int m_limit_start;
	unsigned int m_limit_count;

	virtual void apply();
	virtual ~SortingFilter() {}
	void computeAliases();
	SortingFilter(SubSet& s, unsigned long int flags);
	void buildTreeAndSort(AbstractComparison& comp);

	void sortFullNumeric();
	void sortFullAttribute();
	void sortFullAlias();
	void sortFullDefinition();
	void sortFullText();
	void sortFullConsolidation();

	void sortDefinition();

	void sortFlatNumeric();
	void sortFlatAttribute();
	void sortFlatAlias();
	void sortFlatDefinition();
	void sortFlatText();

	void sortLevelNumeric();
	void sortLevelAttribute();
	void sortLevelAlias();
	void sortLevelText();
	void sortLevelConsolidation();

	void sortNotLevelNumeric();
	void sortNotLevelAttribute();
	void sortNotLevelAlias();
	void sortNotLevelText();
	void sortNotLevelConsolidation();

	const ElementExList& get_top_elems(bool nochildren) const;
	//build subtract of lists a and b, that is, remove all elements from a that are not in b.
	void subtract(ElementExList& a, const ElementExList& b);

public:
	friend class SubSet;
	friend class SDTreeFullSortingFilter;
	friend class TreeFullSortingFilter;
	friend class SDTreePartialSortingFilter;
	friend class TreePartialSortingFilter;

	/*@brief Set the attribute according to which the elements are sorted.
	 Applies only if USE_ATTRIBUTE is set.*/
	void setAttribute(const std::string& attr);

	/**@brief Set the element that defines the level we use for
	 sorting. We sort all elements on this level and below.
	 (use level field) */
	void setLevelElement(const std::string& elem);
	/**@brief Set the level for sort-one-level. */
	void setSingleLevel(unsigned int level);
	/**@brief Set the indent for sort-one-level / sort-not-one-level */
	void setIndent(unsigned int indent);
	void setLimit(unsigned int start, unsigned int count);
};

} //palo
} //jedox
#endif
