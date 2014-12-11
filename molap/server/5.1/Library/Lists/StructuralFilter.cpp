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

#include "StructuralFilter.h"
#include "TreeBuilder.h"
#include "TreeHierarchyFilter.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace palo {

vector<SubElem> StructuralFilter::apply(bool &worked)
{
	applySettings();
	vector<SubElem> result;
	if (m_bound || m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS)) {
		worked = true;
		ElementsType vec;
		if (m_bound) {
			vec.push_back(m_bound);
			if (queryFlag(BELOW_EXCLUSIVE)) {
				m_subset_ref.setGlobalFlag(SubSet::BELOW_EXCLUSIVE);
				m_subset_ref.addElemBound(m_bound);
			} else if (queryFlag(BELOW_INCLUSIVE)) {
				m_subset_ref.setGlobalFlag(SubSet::BELOW_EXCLUSIVE);
				m_subset_ref.addElemBound(m_bound);
			}
		}

		SubSet::Iterator beg = !m_bound ? m_subset_ref.topbegin(false) : m_subset_ref.vectorbegin(vec);
		if (m_subset_ref.queryGlobalFlag(SubSet::LEVEL_BOUNDS)) {
			switch (m_indent) {
			case 2:
				result = CheckNotInLevelRange(beg);
				break;
			case 3:
				result = CheckNotInDepthRange(beg);
				break;
			default:
				result = CheckNotInIndentRange(beg);
			}
		} else {
			result = CheckNotInRange(beg);
		}
	}

	if (queryFlag(HIDE_CONSOLIDATED)) {
		if (worked) {
			vector<SubElem> leaves;
			leaves.reserve(result.size());
			for (vector<SubElem>::iterator it = result.begin(); it != result.end(); ++it) {
				if (it->elem->getChildrenCount() == 0) {
					leaves.push_back(*it);
				}
			}
			result.swap(leaves);
		} else {
			worked = true;
			for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
				if (it.getChildrenCount() == 0) {
					result.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), ""));
				}
			}
		}
	}

	if (queryFlag(HIDE_LEAVES)) {
		if (worked) {
			vector<SubElem> consolidated;
			consolidated.reserve(result.size());
			for (vector<SubElem>::iterator it = result.begin(); it != result.end(); ++it) {
				if (it->elem->getChildrenCount() != 0) {
					consolidated.push_back(*it);
				}
			}
			result.swap(consolidated);
		} else {
			worked = true;
			for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
				if (it.getChildrenCount() != 0) {
					result.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), ""));
				}
			}
		}
	}

	if (queryFlag(REVOLVING) || queryFlag(REVOLVE_ADD_ABOVE) || queryFlag(REVOLVE_ADD_BELOW)) {
		m_subset_ref.setGlobalFlag(SubSet::REVOLVE);
		unsigned int level = 0;
		if (m_revolve && !queryFlag(CYCLIC)) {
			level = m_revolve->getLevel();
		}
		if (worked) {
			vector<SubElem> tmp;
			tmp.reserve(result.size());
			for (vector<SubElem>::iterator it = result.begin(); it != result.end(); ++it) {
				bool erase = false;
				if (queryFlag(REVOLVING) && !queryFlag(CYCLIC)) {
					if (level != it->elem->getLevel()) {
						erase = true;
					}
				} else if (queryFlag(REVOLVE_ADD_BELOW)) {
					if (level < it->elem->getLevel()) {
						erase = true;
					}
				} else if (queryFlag(REVOLVE_ADD_ABOVE)) {
					if (level > it->elem->getLevel()) {
						erase = true;
					}
				}
				if (!erase) {
					tmp.push_back(*it);
				}
			}
			result.swap(tmp);
		} else {
			worked = true;
			for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
				if (queryFlag(REVOLVING) && !queryFlag(CYCLIC)) {
					if (level == it.getLevel()) {
						result.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), ""));
					}
				} else if (queryFlag(REVOLVE_ADD_BELOW)) {
					if (level >= it.getLevel()) {
						result.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), ""));
					}
				} else if (queryFlag(REVOLVE_ADD_ABOVE)) {
					if (level <= it.getLevel()) {
						result.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), ""));
					}
				}
			}
		}
	}
	return result;
}

void StructuralFilter::applySettings()
{
	if (!m_settings.bound.empty()) {
		if (!queryFlag(BELOW_INCLUSIVE) && !queryFlag(BELOW_EXCLUSIVE) && !queryFlag(ABOVE_EXCLUSIVE) && !queryFlag(ABOVE_INCLUSIVE)) {
			setFlag(BELOW_INCLUSIVE);
		}
		m_bound = m_subset_ref.getDimension()->findElementByName(m_settings.bound, m_subset_ref.getUser().get(), false);
	}
	if (m_settings.level) {
		if (!queryFlag(HIERARCHIAL_LEVEL) && !queryFlag(AGGREGATED_LEVEL)) {
			setFlag(AGGREGATED_LEVEL);
			m_subset_ref.setGlobalFlag(SubSet::LEVEL_BOUNDS);
		}
		m_level_begin = m_settings.level_start;
		m_level_end = m_settings.level_end;
	}
	if (m_settings.revolve) {
		if (m_settings.revolve_count <= 0)
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "The size of the revolving list is set to be zero or less");
		if (!queryFlag(REVOLVING) && !queryFlag(REVOLVE_ADD_ABOVE) && !queryFlag(REVOLVE_ADD_BELOW)) {
			setFlag(REVOLVING);
		}

		m_revolve_count = m_settings.revolve_count;
		if (!m_settings.revolve_elem.empty()) {
			m_revolve = m_subset_ref.getDimension()->findElementByName(m_settings.revolve_elem, m_subset_ref.getUser().get(), false);
		} else {
			setFlag(CYCLIC);
		}
	}
	if (m_settings.indent > 0) {
		m_indent = m_settings.indent;
	}
}

StructuralFilter::StructuralFilter(SubSet& s, StructuralFilterSettings &settings) :
	Filter(s, settings.flags, STRUCTURAL_FILTER_NUMFLAGS), m_indent(0), m_bound(0), m_revolve(0), m_revolve_count(0), m_settings(settings)
{

	if (queryFlag(BELOW_EXCLUSIVE) && queryFlag(BELOW_INCLUSIVE)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting flags BELOW_EXCLUSIVE and BELOW_INCLUSIVE set");
	}
	if (queryFlag(HIDE_LEAVES) && queryFlag(HIDE_CONSOLIDATED)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting flags HIDE_LEAVES and HIDE_CONSOLIDATED set");
	}
	if (queryFlag(HIERARCHIAL_LEVEL) && queryFlag(AGGREGATED_LEVEL)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting flags HIERARCHIAL and AGGREGATED_LEVEL set");
	}
	if (queryFlag(REVOLVE_ADD_ABOVE) && queryFlag(REVOLVE_ADD_BELOW)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Conflicting flags REVOLVE_ADD_ABOVE and REVOLVE_ADD_BELOW set");
	}
	m_subset_ref.setGlobalFlag(SubSet::STRUCTURAL_FILTER_ACTIVE);
}

vector<SubElem> StructuralFilter::CheckNotInRange(SubSet::Iterator &beg)
{
	vector<SubElem> result;
	InRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(*this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, beg, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	builder.build(result, false);
	return result;
}

vector<SubElem> StructuralFilter::CheckNotInLevelRange(SubSet::Iterator &beg)
{
	vector<SubElem> result;
	InLevelRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(*this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, beg, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	builder.build(result, false);
	return result;
}

vector<SubElem> StructuralFilter::CheckNotInDepthRange(SubSet::Iterator &beg)
{
	vector<SubElem> result;
	InDepthRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(*this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, beg, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	builder.build(result, false);
	return result;
}

vector<SubElem> StructuralFilter::CheckNotInIndentRange(SubSet::Iterator &beg)
{
	vector<SubElem> result;
	InIndentRange check(m_level_begin, m_level_end);
	TreeHierarchyFilter filter(*this, check);
	TreeBuilder<TreeHierarchyFilter> builder(filter, beg, m_subset_ref);
	if (queryFlag(ABOVE_EXCLUSIVE) || queryFlag(ABOVE_INCLUSIVE)) {
		builder.setReverse();
	}
	builder.build(result, false);
	return result;
}

} //palo
