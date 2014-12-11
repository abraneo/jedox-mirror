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

#include "TextFilter.h"
#include <boost/regex.hpp>
#include <string>
#include "SubSet.h"
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 103500
#	include <boost/range/as_array.hpp>
#endif

#include "Olap/AttributedDimension.h"

namespace palo {


void TextFilter::findRegexInAttribute(const boost::regex& rx, const long attr_id)
{
	CPCube attrcube = AttributedDimension::getAttributesCube(m_subset_ref.getDatabase(), m_subset_ref.getDimension()->getName());
	if (!attrcube) {
		throw ErrorException(ErrorException::ERROR_CUBE_NOT_FOUND, "Attribute cube doesn't exist.");
	}
	PCubeArea area(new CubeArea(m_subset_ref.getDatabase(), attrcube, 2));
	SubSet::Iterator it = m_subset_ref.begin(false);
	PSet s(new Set());
	s->insert(attr_id);
	area->insert(0, s);
	s.reset(new Set());
	for (match_vec_type::size_type idx = 0; !it.end(); ++it, ++idx) {
		if (!m_matches[idx]) {
			s->insert(it.getId());
		}
	}
	area->insert(1, s);

	PCellStream cs = attrcube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_SORTED_PLAN);
	if (cs) {
		while (cs->next()) {
			const CellValue &val = cs->getValue();
			const IdentifiersType &key = cs->getKey();
			if (val.isString()) {
				if (boost::regex_search(val, rx)) {
					m_matches[key[1]] = true;
				}
			}
		}
	}
}

void TextFilter::findRegexInAlias(const boost::regex& rx)
{
	SubSet::Iterator it = m_subset_ref.begin(true);
	for (match_vec_type::size_type idx = 0; !it.end(); ++it, ++idx) {
		if (!m_matches[idx]) {
			if (boost::regex_search(it.getSearchAlias(true), rx)) {
				m_matches[idx] = true;
			}
		}
	}
}

void TextFilter::findRegexInBasic(const boost::regex& rx)
{
	SubSet::Iterator it = m_subset_ref.begin(true);
	for (match_vec_type::size_type idx = 0; !it.end(); ++it, ++idx) {
		if (!m_matches[idx]) {
			if (boost::regex_search(it.getName(), rx)) {
				m_matches[idx] = true;
			}
		}
	}
}

void TextFilter::validateRegex(const std::string& s) const
{
	try {
		if (queryFlag(TextFilter::EXTENDED)) {
			boost::regex rex(s);
		} else {
			boost::regex rex(s, boost::regex::basic | boost::regex::icase | boost::regex::no_char_classes | boost::regex::no_intervals);
		}

	} catch (std::exception&) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attempt to apply an invalid regular expression.");
	} catch (...) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attempt to apply an invalid regular expression.");
	}
}

void TextFilter::prepareRegex(string& rex)
{

	static const char nospecialchars[] = {'\\', '|', '.', '+', '(', ')', '{', '}', '[', ']', '^', '$'};

	size_t i = 0;

#if BOOST_VERSION >= 103500
	BOOST_FOREACH( char s, boost::as_array(nospecialchars) ) {
#else
	BOOST_FOREACH( char s, nospecialchars ) {
#endif
		i = 0;
		while ( 1 ) {
			i = rex.find_first_of( s, i );
			if ( i != string::npos ) {
				rex.insert( i, "\\" );
				i += 2;
			} else break;
		}
	}
	i = 0;

	while (i != string::npos) {
		i = rex.find_first_of('?', i);
		if (i != string::npos) {
			rex.replace(i, 1, ".");
			++i;
		} else
			break;
	}

	i = 0;

	while (i != string::npos) {
		i = rex.find_first_of('*', i);
		if (i != string::npos) {
			//rex.replace(i,"+");
			rex.insert(i, ".");
			i += 2;
		} else
			break;
	}
	// always start from the beginning
	rex.insert(0, "^");

	if (rex.length() < 2 || (*(rex.end() - 1) != '*' && *(rex.end() - 2) != '\\')) {
		rex.push_back('$');
	}

}

TextFilter::TextFilter(SubSet& s, TextFilterSettings &settings, ColumnType type, const string &attribute) :
	Filter(s, settings.flags, TEXT_FILTER_NUM_FLAGS), m_settings(settings), m_matches(match_vec_type(s.size(), false)), m_type(type), m_attr(attribute)
{
}

ElementsType TextFilter::apply()
{
	applySettings();
	std::list<internal_regex>::iterator it = regular_expressions.begin();
	std::list<internal_regex>::iterator itend = regular_expressions.end();
	if (m_subset_ref.queryGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE)) {
		for (; it != itend; ++it) {
			findRegexInAlias(it->rex);
		}
	} else {
		for (; it != itend; ++it) {
			if (it->type == ATTRIBUTE) {
				findRegexInAttribute(it->rex, it->attr_id);
			} else if (it->type == BASIC) {
				findRegexInBasic(it->rex);
			}
		}
	}

	SubSet::Iterator s_it = m_subset_ref.begin(true);
	const match_vec_type::size_type subset_size = m_matches.size();
	ElementsType ret;
	for (match_vec_type::size_type i = 0; i < subset_size && !s_it.end(); ++i, ++s_it) {
		if (m_matches[i]) {
			ret.push_back(s_it.getElement());
		}
	}
	return ret;
}

void TextFilter::applySettings()
{
	vector<string>::const_iterator begin = m_settings.regexps.begin();
	vector<string>::const_iterator end = m_settings.regexps.end();
	vector<string>::const_iterator i;

	for (i = begin; i != end; ++i) {
		if (!m_attr.empty() && (m_type != ATTRIBUTE)) {
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attribute field not empty but ColumnType != ATTRIBUTE");
		}
		if (m_attr.empty() && (m_type == ATTRIBUTE)) {
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attribute field empty but ColumnType == ATTRIBUTE");
		}
		if (!m_attr.empty() && !queryFlag(TextFilter::ADDITIONAL_FIELDS)) {
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attribute field not empty but ADDITIONAL_FIELDS == 0");
		}
		long attr_id = -1;
		if (!m_attr.empty()) {
			attr_id = m_subset_ref.validateAttribute(m_attr);
		}
		//TODO:this is workaround because we always get alias-type Regexes (Bug in XLL)
		validateRegex(*i);
		if (queryFlag(TextFilter::EXTENDED)) {
			const internal_regex irex(*i, m_type, attr_id);
			regular_expressions.push_back(irex);
		} else {
			prepareRegex(const_cast<string&> (*i));
			const internal_regex irex(*i, m_type, attr_id);
			regular_expressions.push_back(irex);
		}
	}
}
} //palo
