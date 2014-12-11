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

#ifndef __SORTINGFILTER_H_INCL__
#define __SORTINGFILTER_H_INCL__

#include <string>
#include <map>
#include <locale>
#include <string.h>

#include "Filter.h"
#include "SubSet.h"
#include "PaloDispatcher/PaloJobRequest.h"

#include "Collections/StringUtils.h"

namespace palo {

class AbstractSelector {
public:

	virtual bool operator()(size_t chc) = 0;
};

class AllSelector : public AbstractSelector {
public:
	inline bool operator()(size_t chc)
	{
		return true;
	}
	virtual ~AllSelector() {}
};

class BaseSelector : public AbstractSelector {
public:
	inline bool operator()(size_t chc)
	{
		return chc == 0;
	}
	virtual ~BaseSelector() {}
};

class ConsolidatedSelector : public AbstractSelector {
public:
	inline bool operator()(size_t chc)
	{
		return chc > 0;
	}
	virtual ~ConsolidatedSelector() {}
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
	virtual ~Negator() {}
};

class NotNegator : public AbstractNegator {
public:
	inline bool operator()(bool b)
	{
		return b;
	}
	virtual ~NotNegator() {}
};

class AbstractEnumerationSelector {
public:
	virtual inline unsigned int operator()(const SortElem &e) const = 0;
};

class IndentSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const SortElem &e) const
	{
		return e.ind;
	}
};
class LevelSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const SortElem &e) const
	{
		return e.el->getLevel();
	}
};
class DepthSelector : public AbstractEnumerationSelector {
public:
	inline unsigned int operator()(const SortElem &e) const
	{
		return e.dep;
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
	virtual ~AbstractComparison() {}
	virtual bool operator()(const SortElem &e1, const SortElem &e2) const = 0;
protected:
	boost::shared_ptr<AbstractSelector> m_type;
	boost::shared_ptr<AbstractNegator> m_neg;

};

class NameComparison : public AbstractComparison {
public:
	NameComparison(unsigned int element_type = 0, bool reverse = false) :
		AbstractComparison(element_type, reverse), u8(Context::getContext()->getSession()->getLocale())
	{
	}
	virtual ~NameComparison() {}
	bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e2.el->getChildrenCount())) {
			return (*m_neg)(u8(e1.getUName(), e2.getUName()));
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
private:
	UTF8Comparer u8;
};

class PositionalComparison : public AbstractComparison {
public:
	PositionalComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	virtual ~PositionalComparison() {}
	inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e1.el->getChildrenCount())) {
			return (*m_neg)(e1.el->getPosition() < e2.el->getPosition());
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
};

class AliasComparison : public AbstractComparison {
public:
	AliasComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse), u8(Context::getContext()->getSession()->getLocale())
	{
	}
	virtual ~AliasComparison() {}
	inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e2.el->getChildrenCount())) {
			const CellValue &v = e1.getSearchAlias();
			if (v.isString()) {
				return (*m_neg)(u8(v, e2.getSearchAlias()));
			} else {
				return (*m_neg)(v < e2.getSearchAlias());
			}
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
private:
	UTF8Comparer u8;
};

class DataComparison : public AbstractComparison {
public:
	DataComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse)
	{
	}
	virtual ~DataComparison() {}
	inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e2.el->getChildrenCount())) {
			return e1.getValue() != e2.getValue() ? (*m_neg)(e1.getValue() < e2.getValue()) : e1.el->getPosition() < e2.el->getPosition();
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
};

class StringDataComparison : public AbstractComparison {
public:
	StringDataComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse), u8(Context::getContext()->getSession()->getLocale())
	{
	}
	virtual ~StringDataComparison() {}
	inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e2.el->getChildrenCount())) {
			return (*m_neg)(u8(e1.getValue(), e2.getValue()));
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
private:
	UTF8Comparer u8;
};

class AttributeComparison : public AbstractComparison {
public:
	AttributeComparison(unsigned int element_type, bool reverse) :
		AbstractComparison(element_type, reverse), u8(Context::getContext()->getSession()->getLocale())
	{
	}
	virtual ~AttributeComparison() {}
	inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if ((*m_type)(e1.el->getChildrenCount()) && (*m_type)(e2.el->getChildrenCount())) {
			const CellValue &v = e1.getSortingAlias();
			if (v.isString()) {
				return (*m_neg)(u8(v, e2.getSortingAlias()));
			} else {
				return (*m_neg)(v < e2.getSortingAlias());
			}
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
private:
	UTF8Comparer u8;
};

class ConsolidationComparison : public AbstractComparison {
public:
	ConsolidationComparison(unsigned int element_type = 0, bool reverse = false) :
		AbstractComparison(element_type, reverse)
	{
	}
	virtual ~ConsolidationComparison() {}
	bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if (e1.cons != NO_IDENTIFIER && e2.cons != NO_IDENTIFIER) {
			return e1.cons < e2.cons;
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
};

template<class abstract_comparison, class IndentSelect = IndentSelector>
class OneLevelComparison : public AbstractComparison {
public:
	OneLevelComparison(unsigned int level, unsigned int element_type, bool reverse) :
		m_sorter(element_type, reverse)
	{
		m_level = level;
	}
	virtual ~OneLevelComparison() {}
	virtual inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if (m_indent(e1) == m_level) {
			return m_sorter(e1, e2);
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
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
	}
	virtual inline bool operator()(const SortElem &e1, const SortElem &e2) const
	{
		if (m_indent(e1) != m_level) {
			return m_sorter(e1, e2);
		} else {
			return e1.el->getPosition() < e2.el->getPosition();
		}
	}
private:
	unsigned int m_level;
	abstract_comparison m_sorter;
	IndentSelect m_indent;
};

class SortingFilter : public SortingFilterBase, public Filter {

	IdentifierType m_sort_attribute;
	std::locale m_locale;
	//the "kind" of level we use (i.e. 1 == indent, 2 == level, 3 == depth)
	unsigned int m_indent;
	//the node type used when sorting (i.e. 0 == all elements,
	//1 == only base elements will be sorted,
	//2 == only consolidated elements will be sorted)
	unsigned int m_element_type;
	int m_level;
	bool m_parents_below;
	bool m_reverse_order;
	unsigned int m_limit_start;
	unsigned int m_limit_count;
	SortingFilterSettings &m_settings;

	vector<SubElem> result;

	void computeAliases();
	void buildTreeAndSort(AbstractComparison& comp);

	void sortFullNumeric();
	void sortFullAttribute();
	void sortFullAlias();
	void sortFullDefinition();
	void sortFullText();
	void sortFullConsolidation();

	void sortDefinition();

	void sortFlatNumeric(bool showDuplicates);
	void sortFlatAttribute(bool showDuplicates);
	void sortFlatAlias(bool showDuplicates);
	void sortFlatDefinition(bool showDuplicates);
	void sortFlatText(bool showDuplicates);

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

	void applySettings();

public:
	friend class SubSet;
	friend class SDTreeFullSortingFilter;
	friend class TreeFullSortingFilter;
	friend class SDTreePartialSortingFilter;
	friend class TreePartialSortingFilter;

	SortingFilter(SubSet& s, SortingFilterSettings &settings);
	~SortingFilter() {}
	vector<SubElem> apply();
};

} //palo
#endif
