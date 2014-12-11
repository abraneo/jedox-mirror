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

#include <string>
#include <set>

#include "StructuralFilter.h"
#include "SortingFilter.h"
#include "TreeFullSortingFilter.h"
#include "TreePartialSortingFilter.h"
#include "PickList.h"
#include "SubSet.h"
#include "TreeBuilder.h"
#include "Olap/AttributedDimension.h"

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
	boost::scoped_ptr<abstract_comparison > comp( new abstract_comparison( m_element_type, m_reverse_order )  );

#define _sortingfilter_set_type_selector_flat_(abstract_comparison)\
	boost::scoped_ptr<abstract_comparison> comp( new abstract_comparison( m_element_type, m_reverse_order ) );

namespace palo {


vector<SubElem> SortingFilter::apply()
{
	applySettings();
	if ((!m_subset_ref.queryGlobalFlag(SubSet::DATA_FILTER_ACTIVE)) && queryFlag(NUMERIC)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Flag NUMERIC set but data filter unused.");
	}
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
	} else if (queryFlag(FLAT_HIERARCHY) || !queryFlag(WHOLE)) {
		if (queryFlag(NUMERIC)) {
			sortFlatNumeric(!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES));
		} else if (queryFlag(USE_ATTRIBUTE)) {
			sortFlatAttribute(!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES));
		} else if (queryFlag(USE_ALIAS)) {
			sortFlatAlias(!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES));
		} else if (queryFlag(SortingFilterBase::TEXT)) {
			sortFlatText(!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES));
		} else {
			sortFlatDefinition(!queryFlag(SHOW_DUPLICATES) || m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES));
		}
	}
	return result;
}

SortingFilter::SortingFilter(SubSet& s, SortingFilterSettings &settings) :
	Filter(s, settings.flags, SORTING_FILTER_NUM_FLAGS), m_locale(std::locale("")), m_indent(0), m_element_type(0), m_level(-1), m_parents_below(false), m_reverse_order(false), m_limit_start(0), m_limit_count(0), m_settings(settings)
{
	if (queryFlag(WHOLE) && queryFlag(FLAT_HIERARCHY)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting filter flags WHOLE and FLAT_HIERARCHY");
	}
	if (queryFlag(POSITION) && queryFlag(USE_ALIAS)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting filter flags POSITION and ALIAS");
	}
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(LEAVES_ONLY)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Cannot sort leaves in a flat hierarchy. Conflicting flags FLAT_HIERARCHY and LEAVES_ONLY");
	}
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(NO_CHILDREN))
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting filter flags FLAT_HIERARCHY and NO_CHILDREN");
	if (queryFlag(FLAT_HIERARCHY) && queryFlag(CONSOLIDATED_ONLY))
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting filter flags FLAT_HIERARCHY and CONSOLIDATED_ONLY");
	if (queryFlag(SORT_ONE_LEVEL) && !(queryFlag(WHOLE) || queryFlag(NO_CHILDREN)))
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Sorting on one level is only possible with a hierarchical sort.");
}

void SortingFilter::applySettings()
{
	if (!m_settings.attribute.empty()) {
		if (!queryFlag(USE_ATTRIBUTE))
			setFlag(USE_ATTRIBUTE);
		m_sort_attribute = m_subset_ref.validateAttribute(m_settings.attribute);
	}
	if (m_settings.level >= 0) {
		m_level = m_settings.level;
	}
	if (m_settings.indent >= 0) {
		m_indent = m_settings.indent;
	}
	if (m_settings.limit_count && !queryFlag(LIMIT)) {
		setFlag(LIMIT);
	}
	m_limit_start = m_settings.limit_start;
	m_limit_count = m_settings.limit_count;
}

void SortingFilter::sortFlatNumeric(bool showDuplicates)
{
	list<SortElem> elems;
	for (SubSet::Iterator it = m_subset_ref.begin(showDuplicates); !it.end(); ++it) {
		elems.push_back(SortElem(&m_subset_ref, it.getElement(), it.getIndent(), it.getDepth(), NO_IDENTIFIER));
	}
	if (!m_subset_ref.queryGlobalFlag(SubSet::DATA_STRING)) {
		_sortingfilter_set_type_selector_flat_(DataComparison);
		elems.sort(*comp);
	} else {
		_sortingfilter_set_type_selector_flat_(StringDataComparison);
		elems.sort(*comp);
	}
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		result.push_back(SubElem(it->el, it->ind, it->dep, queryFlag(PATH) ? StringUtils::convertToString(it->el->getIdentifier()) : ""));
	}
}

void SortingFilter::sortFlatAttribute(bool showDuplicates)
{
	computeAliases();
	list<SortElem> elems;
	for (SubSet::Iterator it = m_subset_ref.begin(showDuplicates); !it.end(); ++it) {
		elems.push_back(SortElem(&m_subset_ref, it.getElement(), it.getIndent(), it.getDepth(), NO_IDENTIFIER));
	}
	_sortingfilter_set_type_selector_flat_(AttributeComparison);
	elems.sort(*comp);
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		result.push_back(SubElem(it->el, it->ind, it->dep, queryFlag(PATH) ? StringUtils::convertToString(it->el->getIdentifier()) : ""));
	}
}

void SortingFilter::sortFlatAlias(bool showDuplicates)
{
	list<SortElem> elems;
	for (SubSet::Iterator it = m_subset_ref.begin(showDuplicates); !it.end(); ++it) {
		elems.push_back(SortElem(&m_subset_ref, it.getElement(), it.getIndent(), it.getDepth(), NO_IDENTIFIER));
	}
	_sortingfilter_set_type_selector_flat_(AliasComparison);
	elems.sort(*comp);
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		result.push_back(SubElem(it->el, it->ind, it->dep, queryFlag(PATH) ? StringUtils::convertToString(it->el->getIdentifier()) : ""));
	}
}

void SortingFilter::sortFlatDefinition(bool showDuplicates)
{
	list<SortElem> elems;
	for (SubSet::Iterator it = m_subset_ref.begin(showDuplicates); !it.end(); ++it) {
		elems.push_back(SortElem(&m_subset_ref, it.getElement(), it.getIndent(), it.getDepth(), NO_IDENTIFIER));
	}
	_sortingfilter_set_type_selector_flat_(PositionalComparison);
	elems.sort(*comp);
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		result.push_back(SubElem(it->el, it->ind, it->dep, queryFlag(PATH) ? StringUtils::convertToString(it->el->getIdentifier()) : ""));
	}
}

void SortingFilter::sortFlatText(bool showDuplicates)
{
	list<SortElem> elems;
	for (SubSet::Iterator it = m_subset_ref.begin(showDuplicates); !it.end(); ++it) {
		elems.push_back(SortElem(&m_subset_ref, it.getElement(), it.getIndent(), it.getDepth(), NO_IDENTIFIER));
	}
	_sortingfilter_set_type_selector_flat_(NameComparison);
	elems.sort(*comp);
	for (list<SortElem>::iterator it = elems.begin(); it != elems.end(); ++it) {
		result.push_back(SubElem(it->el, it->ind, it->dep, queryFlag(PATH) ? StringUtils::convertToString(it->el->getIdentifier()) : ""));
	}
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
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Trying to sort according to alias but no alias-filter in use.");
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
	if ((IdentifierType)-1 == m_sort_attribute) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Trying to sort according to attribute but no attribute-string is set.");
	}
	CPCube attrcube = AttributedDimension::getAttributesCube(m_subset_ref.getDatabase(), m_subset_ref.getDimension()->getName());
	if (!attrcube) {
		throw ErrorException(ErrorException::ERROR_CUBE_NOT_FOUND, "Attribute cube doesn't exist.");
	}
	PCubeArea area(new CubeArea(m_subset_ref.getDatabase(), attrcube, 2));

	PSet s(new Set());
	s->insert(m_sort_attribute);
	area->insert(0, s);
	area->insert(1, m_subset_ref.getSet(false));

	CellValue def;
	PCellStream cs = attrcube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_SORTED_PLAN);
	if (cs) {
		while (cs->next()) {
			const CellValue &val = cs->getValue();
			const IdentifiersType &key = cs->getKey();
			if (!(val.isError())) {
				m_subset_ref.setSortingAlias(key[1], val);
			}
		}
	}
}

void SortingFilter::buildTreeAndSort(AbstractComparison& comp)
{
	if (queryFlag(NO_CHILDREN)) {
		SubSet::Iterator beg = m_subset_ref.topbegin(true);
		if (queryFlag(SHOW_DUPLICATES) && !m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES)) {
			SDTreePartialSortingFilter filter(*this, comp);
			TreeBuilder<SDTreePartialSortingFilter> builder(filter, beg, m_subset_ref);
			builder.build(result, queryFlag(PATH));
		} else {
			TreePartialSortingFilter filter(*this, comp);
			TreeBuilder<TreePartialSortingFilter> builder(filter, beg, m_subset_ref);
			builder.build(result, queryFlag(PATH));
		}
	} else {
		SubSet::Iterator beg = m_subset_ref.topbegin(false);
		if (queryFlag(SHOW_DUPLICATES) && !m_subset_ref.queryGlobalFlag(SubSet::DONT_SHOW_DUPLICATES)) {
			SDTreeFullSortingFilter filter(*this, comp);
			TreeBuilder<SDTreeFullSortingFilter> builder(filter, beg, m_subset_ref);
			builder.build(result, queryFlag(PATH));
		} else {
			TreeFullSortingFilter filter(*this, comp);
			TreeBuilder<TreeFullSortingFilter> builder(filter, beg, m_subset_ref);
			builder.build(result, queryFlag(PATH));
		}
	}
}

} //palo
