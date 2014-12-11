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

#include "TextFilter.h"
#include "ListBasicException.h"
#include <boost/regex.hpp>
#include <string>
#include "SubSet.h"
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 103500
#	include <boost/range/as_array.hpp>
#endif

namespace jedox {
namespace palo {

struct TextFilter::TextFilterImpl {
	typedef long coord_type;
	typedef std::vector<coord_type> coords_type;
	typedef std::vector<coords_type> coord_vec_type;
	typedef std::vector<CELL_VALUE> cell_value_vec_type;
	typedef std::vector<bool> match_vec_type;

	TextFilterImpl(SubSet& s) :
		m_subset_ref(s), m_matches(match_vec_type(s.getConcreteSubset().size(), false))
	{
	}

	class internal_regex {
	public:
		ColumnType type;
		long attr_id;
		boost::regex rex;

		internal_regex(std::string descr, ColumnType t, const long attr_id) :
			type(t), attr_id(attr_id), rex(boost::regex(descr))
		{
		}
	};

	/**@brief mark all elements of our subset that match the regular expression -- use elements attribute*/
	inline void findRegexInAttribute(const boost::regex& rx, const long attr_id, const bool is_first)
	{
		cell_value_vec_type attribute_values;
		coord_vec_type coords;

		ElementExList& subset_ref = m_subset_ref.getConcreteSubset();
		ElementExList::iterator it = subset_ref.begin();
		coords.reserve(subset_ref.size());
		if (is_first) {
			for (; it != subset_ref.end(); ++it) {
				coords_type coordinates(2);
				coordinates[0] = attr_id;
				coordinates[1] = it->get_id();
				coords.push_back(coordinates);
			}

			m_subset_ref.getAttributeCube().MDXCellValues(attribute_values, coords);
			cell_value_vec_type::const_iterator attr_it(attribute_values.begin()), attr_end(attribute_values.end());
			for (match_vec_type::size_type idx = 0; attr_it != attr_end; ++attr_it, ++idx) {
				if (attr_it->type == CELL_VALUE::STRING) {
					if (boost::regex_search(attr_it->val.s, rx)) {
						m_matches[idx] = true;
					}
				}
			}
		} else {
			for (match_vec_type::size_type idx = 0; it != subset_ref.end(); ++it, ++idx) {
				if (!m_matches[idx]) {
					coords_type coordinates(2);
					coordinates[0] = attr_id;
					coordinates[1] = it->get_id();
					coords.push_back(coordinates);
				}
			}

			m_subset_ref.getAttributeCube().MDXCellValues(attribute_values, coords);
			cell_value_vec_type::const_iterator attr_it(attribute_values.begin()), attr_end(attribute_values.end());
			for (match_vec_type::size_type idx = 0; attr_it != attr_end; ++attr_it, ++idx) {
				while (m_matches[idx]) {
					++idx;
				}
				if (attr_it->type == CELL_VALUE::STRING) {
					if (boost::regex_search(attr_it->val.s, rx)) {
						m_matches[idx] = true;
					}
				}
			}

		}
	}

	/**@brief mark all elements of our subset that match the regular expression -- use aliases from Alias filter*/
	inline void findRegexInAlias(const boost::regex& rx, const bool is_first)
	{
		ElementExList& subset_ref = m_subset_ref.getConcreteSubset();
		ElementExList::const_iterator it = subset_ref.begin(), itend = subset_ref.end();
		if (is_first) {
			for (match_vec_type::size_type idx = 0; it != itend; ++it, ++idx) {
				if (boost::regex_search(it->search_alias, rx)) {
					m_matches[idx] = true;
				}
			}
		} else {
			for (match_vec_type::size_type idx = 0; it != itend; ++it, ++idx) {
				if (!m_matches[idx]) {
					if (boost::regex_search(it->search_alias, rx)) {
						m_matches[idx] = true;
					}
				}
			}
		}
	}

	/**@brief mark all elements of our subset that match the regular expression */
	inline void findRegexInBasic(const boost::regex& rx, const bool is_first)
	{
		ElementExList& subset_ref = m_subset_ref.getConcreteSubset();
		ElementExList::const_iterator it = subset_ref.begin(), itend = subset_ref.end();
		if (is_first) {
			for (match_vec_type::size_type idx = 0; it != itend; ++it, ++idx) {
				if (boost::regex_search(it->get_name(), rx)) {
					m_matches[idx] = true;
				}
			}
		} else {
			for (match_vec_type::size_type idx = 0; it != itend; ++it, ++idx) {
				if (!m_matches[idx]) {
					if (boost::regex_search(it->get_name(), rx)) {
						m_matches[idx] = true;
					}
				}
			}
		}
	}

	inline void apply()
	{
		std::list<internal_regex>::iterator it = regular_expressions.begin();
		std::list<internal_regex>::iterator itend = regular_expressions.end();
		bool is_first = true;
		// check if alias filter is used by using myset->get_filter->checkflag
		// if alias filter unused but ColumnType ALIAS, ignore this expression
		if (m_subset_ref.queryGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE)) {
			for (; it != itend; ++it) {
				findRegexInAlias(it->rex, is_first);
				is_first = false;
			}
		} else {
			for (; it != itend; ++it) {
				if (it->type == ATTRIBUTE) { //check ATTRIBUTE and apply filter
					findRegexInAttribute(it->rex, it->attr_id, is_first);
					is_first = false;
				} else if (it->type == BASIC) {//column type basic -- search the names of elements
					findRegexInBasic(it->rex, is_first);
					is_first = false;
				}
			}
		}

		ElementExList& subset_ref = m_subset_ref.getConcreteSubset();
		ElementExList::iterator s_it = subset_ref.begin();
		ElementExList::const_iterator s_end = subset_ref.end();
		const match_vec_type::size_type subset_size = m_matches.size();
		for (match_vec_type::size_type i = 0; i < subset_size && s_it != s_end; ++i) {
			if (m_matches[i]) {
				++s_it;
			} else {
				s_it = subset_ref.erase(s_it);
			}
		}
	}

	inline void setRegex(const TextFilter& tfilter, const std::string& s, ColumnType t, const std::string& attr)
	{

		if (!attr.empty() && (t != ATTRIBUTE)) {
			throw ListBasicException("Attribute field not empty but ColumnType != ATTRIBUTE", "Invalid attribute usage");
		}
		if (attr.empty() && (t == ATTRIBUTE)) {
			throw ListBasicException("Attribute field empty but ColumnType == ATTRIBUTE", "Invalid attribute usage");
		}
		if (!attr.empty() && !tfilter.queryFlag(TextFilter::ADDITIONAL_FIELDS)) {
			throw ListBasicException("Attribute field not empty but ADDITIONAL_FIELDS == 0", "Invalid attribute usage");
		}
		long attr_id = -1;
		if (!attr.empty()) {
			attr_id = m_subset_ref.validateAttribute(attr);
		}
		//TODO:this is workaround because we always get alias-type Regexes (Bug in XLL)
		validateRegex(tfilter, s);
		if (tfilter.queryFlag(TextFilter::EXTENDED)) {
			const internal_regex irex(s, t, attr_id);
			regular_expressions.push_back(irex);
		} else {
			prepareRegex(const_cast<string&> (s));
			const internal_regex irex(s, t, attr_id);
			regular_expressions.push_back(irex);
		}
	}

	/**@brief validate the regular expression and throw if it is not valid*/
	inline void validateRegex(const TextFilter& tfilter, const std::string& s) const
	{
		try {
			if (tfilter.queryFlag(TextFilter::EXTENDED)) {
				boost::regex rex(s);
			} else {
				boost::regex rex(s, boost::regex::basic | boost::regex::icase | boost::regex::no_char_classes | boost::regex::no_intervals);
			}

		} catch (std::exception&) {
			throw ListBasicException("Attempt to apply an invalid regular expression.", "Invalid Text Filter usage.");
#ifndef _DEBUG
		} catch (...) {
			throw ListBasicException("Attempt to apply an invalid regular expression.", "Invalid Text Filter usage.");
#endif
		}
	}
	/**@brief Transform all characters X that we do not which to be treated as special characters to \x
	 Transform ? to "." and "*" to ".*". This approximates what users are used to from the DOS commandline */
	inline void prepareRegex(string& rex)
	{

		//static char specialchars[]   = { '?' , '*' };
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

	list<internal_regex> regular_expressions;
	SubSet& m_subset_ref;
	match_vec_type m_matches;
};

TextFilter::TextFilter(SubSet& s, unsigned long int flags) :
	Filter(s, flags, TEXT_FILTER_NUM_FLAGS, Filter::TEXT), m_Impl(new TextFilterImpl(s))
{
}

TextFilter::~TextFilter()
{
}

void TextFilter::apply()
{
	Filter::apply();
	m_Impl->apply();
}

void TextFilter::setRegex(const std::string &s, jedox::palo::TextFilter::ColumnType t, const std::string &attribute)
{
	m_Impl->setRegex(*this, s, t, attribute);
}
} //palo
} //jedox
