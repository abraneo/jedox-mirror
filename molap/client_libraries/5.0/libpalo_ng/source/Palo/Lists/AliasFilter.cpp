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

#include "AliasFilter.h"
#include "ListBasicException.h"
#include <string>
#include <list>
#include <utility>
#include <map>
#include <vector>

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 103500
#	include <boost/range/as_array.hpp>
#endif

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "SubSet.h"
#include "PickList.h"

using namespace jedox::palo;
using namespace boost;

namespace jedox {
namespace palo {

struct AliasFilter::AliasFilterData {

	AliasFilterData() :
		m_attr1_coord(-1), m_attr2_coord(-1)
	{
	}

	boost::scoped_ptr<std::string> m_attr1;
	long m_attr1_coord;

	boost::scoped_ptr<std::string> m_attr2;
	long m_attr2_coord;

	boost::scoped_ptr<std::string> m_display_attr;
	long m_display_attr_coord;

	// Attributes we want to search
	std::vector<std::string> m_attributes;
	coords_type m_attr_coords;

	std::vector<unsigned int> m_filterexpressions_lookup_map;
	filterexpression_vec_vec_type m_filterexpressions;
	cell_value_vec_type m_cell_vals;
	cell_value_vec_type m_picklist_cell_vals;
	std::vector<bool> m_dont_need_col;
};

struct AliasFilter::AliasFilterHelper {
	typedef std::vector<coords_type> coord_vec_type;

	static inline void getCellValues(AliasFilter& a_filter, ElementExList& elemlist, bool two_alias = false)
	{
		getCellValues(a_filter, a_filter.m_data->m_cell_vals, elemlist, two_alias);
	}

	static inline void getFilterExpressionCellValues(AliasFilter& a_filter, cell_value_vec_type& result, ElementExList& elemlist)
	{
		AliasFilterData& data = *a_filter.m_data;
		coord_vec_type coords;
		const unsigned int coords_size = 2;
		if (0 == std::min(data.m_filterexpressions[0].size(), data.m_attributes.size())) {
			return;
		} else {
			if (data.m_attr_coords.empty()) {
				data.m_attr_coords = validateAttributes(a_filter, data.m_attributes);
			}
			coords.reserve(elemlist.size());
			ElementExList::const_iterator itend = elemlist.end();
			for (size_t i = 0; i < data.m_attributes.size(); i++) {
				for (ElementExList::const_iterator it = elemlist.begin(); it != itend; ++it) {
					coords_type coordinates(coords_size);
					coordinates[0] = data.m_attr_coords[i];
					coordinates[1] = it->get_id();
					coords.push_back(coordinates);
				}
			}
			a_filter.m_subset_ref.getAttributeCube().MDXCellValues(result, coords);
		}
	}

	static inline void getCellValues(AliasFilter& a_filter, cell_value_vec_type& result, ElementExList& elemlist, bool two_alias = false)
	{
		AliasFilterData& data = *a_filter.m_data;
		coord_vec_type coords;
		const unsigned int coords_size = 2;
		if (two_alias) {
			coords.reserve(2 * elemlist.size());
			ElementExList::const_iterator itend = elemlist.end();
			for (ElementExList::const_iterator it = elemlist.begin(); it != itend; ++it) {
				coords_type coordinates(coords_size);
				coordinates[0] = data.m_attr2_coord;
				coordinates[1] = it->get_id();
				coords.push_back(coordinates);
				coords_type coordinates2(coords_size);
				coordinates2[0] = data.m_attr1_coord;
				coordinates2[1] = it->get_id();
				coords.push_back(coordinates2);
			}
		} else {
			coords.reserve(elemlist.size());
			ElementExList::const_iterator itend = elemlist.end();
			for (ElementExList::const_iterator it = elemlist.begin(); it != itend; ++it) {
				coords_type coordinates(coords_size);
				coordinates[0] = data.m_attr1_coord;
				coordinates[1] = it->get_id();
				coords.push_back(coordinates);
			}
		}
		a_filter.m_subset_ref.getAttributeCube().MDXCellValues(result, coords);
	}

	/*@brief extract an operator and its value from a string. If the string is not properly formatted,
	 return NO_OP and value 0 */
	static inline filterexpression_type getOperatorExpression(const std::string& s)
	{
		//we accept empty string as * too
		std::string new_s = s;
		if ("" == new_s) {
			new_s = "*";
		}
		std::string h = new_s.substr(0, 2);
		int op_size = 0;
		AliasFilter::Optype t_op = AliasFilter::NO_OP;
		if (h == std::string("<=")) {
			t_op = AliasFilter::OP_GEQ;
			op_size = 2;
		} else if (h == std::string(">=")) {
			t_op = AliasFilter::OP_LEQ;
			op_size = 2;
		} else if (h == std::string("<>")) {
			t_op = AliasFilter::OP_NE;
			op_size = 2;
		} else if (h[0] == '<') {
			t_op = AliasFilter::OP_GT;
			op_size = 1;
		} else if (h[0] == '>') {
			t_op = AliasFilter::OP_LESS;
			op_size = 1;
		} else if (h[0] == '=') {
			t_op = AliasFilter::OP_EQU;
			op_size = 1;
		}

		if (t_op != AliasFilter::NO_OP) {
			std::string sub = new_s.substr(op_size);
			try {
				double ret_val = util::lexicalConversion(double, std::string, sub);
				return filterexpression_type(t_op, ret_val, boost::regex());
			} catch (PaloException &) {
			}
		}
		return filterexpression_type(AliasFilter::NO_OP, 0, boost::regex(getRegex(new_s)));
	}

	static inline std::string getRegex(const std::string& ss)
	{

		string rex = ss;

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

		return rex;
	}

	static inline std::vector<long> validateAttributes(AliasFilter& a_filter, const std::vector<std::string>& attributes)
	{
		typedef std::vector<long> id_set_type;
		id_set_type tmp_list;
		tmp_list.reserve(attributes.size());
		std::vector<std::string>::const_iterator end_it(attributes.end());
		for (std::vector<std::string>::const_iterator it = attributes.begin(); it != end_it; ++it) {
			tmp_list.push_back(a_filter.m_subset_ref.validateAttribute(*it));
		}
		return tmp_list;
	}
};

struct AliasFilter::FilterOperator {
	typedef ElementExList element_info_ext_list_type;
	typedef element_info_ext_list_type::iterator iterator;
	typedef element_info_ext_list_type::const_iterator const_iterator;

	static inline void check_and_erase(AliasFilter& a_filter, const Cube& c, const std::vector<string>& attrlist, const filterexpression_vec_vec_type & filters, std::vector<CELL_VALUE>& cell_vals)
	{
		const filterexpression_vec_vec_type::size_type filters_size = filters.size();
		const filterexpression_vec_type::size_type filter_size = filters[0].size();
		cell_value_vec_type cell_vals_vec;
		element_info_ext_list_type& concrete_subset = a_filter.m_subset_ref.getConcreteSubset();
		AliasFilterHelper::getFilterExpressionCellValues(a_filter, cell_vals_vec, concrete_subset);
		iterator itbeg = concrete_subset.begin();
		const_iterator itend = concrete_subset.end();
		size_t elemcount = concrete_subset.size();
		unsigned int idx = 0;
		bool match = false;
		while (itbeg != itend) {
			for (unsigned int i = 0; i < filters_size; ++i) {
				match = true;
				for (unsigned int j = 0; j < filter_size; ++j) {
					if (!check(cell_vals_vec[j * elemcount + idx], filters[i][j])) {
						match = false;
						break;
					}
				}
				if (match) {
					break;
				}
			}
			if (match) {
				++itbeg;
			} else {
				itbeg = concrete_subset.erase(itbeg);
			}
			idx++;
		}
	}

	static inline bool check(const CELL_VALUE& cval, const filterexpression_type& f)
	{
		if (cval.type == CELL_VALUE::ERROR) {
				return false;
		} 
		Optype tp = boost::get<0>(f);
		if (tp == NO_OP) {
			if ((cval.type != CELL_VALUE::STRING) && ((boost::get<2>(f)).str() != "^.*"))
				throw ListBasicException("String expected but double/error found as cell value", "Alias Filter data input error");
			else if (!boost::regex_search(cval.val.s, boost::get<2>(f))) {
				return false;
			}
		} else {
			double val = 0.0;
			if (cval.type == CELL_VALUE::STRING) {
				try {
					val = util::lexicalConversion(double, std::string, cval.val.s);
				} catch (const PaloException &) {
				}
			} else if (cval.type == CELL_VALUE::NUMERIC) {
				val = cval.val.d;
			} else {
				throw ListBasicException("Unknown type inside Cell value", "Alias Filter data input error");
			}
			switch (tp) {
			case OP_EQU:
				return (boost::get<1>(f) == val);
				break;
			case OP_GEQ:
				return (boost::get<1>(f) >= val);
				break;
			case OP_LEQ:
				return (boost::get<1>(f) <= val);
				break;
			case OP_GT:
				return (boost::get<1>(f) > val);
				break;
			case OP_LESS:
				return (boost::get<1>(f) < val);
				break;
			case OP_NE:
				return (boost::get<1>(f) != val);
				break;
			default:
				throw ListBasicException("Unknown operator", "Alias Filter error");
			}
		}
		return true;
	}
};

AliasFilter::AliasFilter(SubSet& s, unsigned long int flags) :
	Filter(s, flags, ALIAS_FILTER_NUMFLAGS, Filter::ALIAS)
{
	if (queryFlag(SEARCH_ONE) && queryFlag(SEARCH_TWO)) {
		throw jedox::palo::ListBasicException("Alias Filter construction with both SEARCH_ONE and SEARCH_TWO impossible.", "Error using Alias-Filter");
	}
	m_data.reset(new AliasFilterData());
}

void AliasFilter::apply()
{
	AliasFilterData& data = *m_data;
	//default: search one alias if alias filter is initialized without flags
	//apply alias-filter to pick list, such that its elements can be sorted accordingly
	if (queryFlag(SEARCH_ONE)) {
		AliasFilterHelper::getCellValues(*this, m_subset_ref.getConcreteSubset());
		applyOneAlias(m_subset_ref.getConcreteSubset(), data.m_cell_vals);
		if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_ACTIVE)) {
			AliasFilterHelper::getCellValues(*this, data.m_picklist_cell_vals, m_subset_ref.getPicklist()->getElements());
			applyOneAlias(m_subset_ref.getPicklist()->getElements(), data.m_picklist_cell_vals);
		}
		m_subset_ref.setGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE);
	} else if (queryFlag(SEARCH_TWO)) {
		AliasFilterHelper::getCellValues(*this, m_subset_ref.getConcreteSubset(), true);
		applyTwoAliases(m_subset_ref.getConcreteSubset());
		if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_ACTIVE)) {
			AliasFilterHelper::getCellValues(*this, data.m_picklist_cell_vals, m_subset_ref.getPicklist()->getElements(), true);
			applyTwoAliases(m_subset_ref.getPicklist()->getElements(), data.m_picklist_cell_vals);
		}
		m_subset_ref.setGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE);
	}
	if (queryFlag(DISPLAY_ALIAS)) {
		/*get all cell-values of the attribute-cube-column that have a string cell-value != 0
		 and associate them with the original element name*/
		if (queryFlag(SEARCH_TWO)) {
			AliasFilterHelper::getCellValues(*this, m_subset_ref.getConcreteSubset(), true);
			applyTwoDisplayAliases(m_subset_ref.getConcreteSubset(), data.m_cell_vals);
			if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_ACTIVE)) {
				AliasFilterHelper::getCellValues(*this, data.m_picklist_cell_vals, m_subset_ref.getConcreteSubset(), true);
				applyTwoDisplayAliases(m_subset_ref.getPicklist()->getElements(), data.m_picklist_cell_vals);
			}
		} else {
			AliasFilterHelper::getCellValues(*this, m_subset_ref.getConcreteSubset());
			applyOneDisplayAlias(m_subset_ref.getConcreteSubset(), data.m_cell_vals);
			if (m_subset_ref.queryGlobalFlag(SubSet::PICKLIST_ACTIVE)) {
				AliasFilterHelper::getCellValues(*this, data.m_picklist_cell_vals, m_subset_ref.getConcreteSubset(), true);
				applyOneDisplayAlias(m_subset_ref.getPicklist()->getElements(), data.m_picklist_cell_vals);
			}
		}
	}
	if (queryFlag(USE_FILTEREXP) && !data.m_filterexpressions.empty()) {
		//get the attribute cube.
		//get the attribute dimension
		// for each element inside the subset,
		// iterate over the attribute dimension and check if it matches the advanced filter.
		size_t colnum = std::min(data.m_filterexpressions[0].size(), data.m_attributes.size());
		if (colnum != 0) {
			FilterOperator::check_and_erase(*this, m_subset_ref.getAttributeCube(), data.m_attributes, data.m_filterexpressions, data.m_cell_vals);
		}
	}
	if (queryFlag(USE_FILTEREXP) && data.m_filterexpressions.empty()) {
		throw ListBasicException("Tried to use advanced filter-expressions without arguments", "Wrong Alias-Filter usage.");
	}
}

void AliasFilter::setAttributes(const std::string& first, const std::string& second)
{
	//Default is SEARCH_ONE
	AliasFilterData& data = *m_data;
	if ((first != "" && queryFlag(SEARCH_ONE)) || ((!queryFlag(SEARCH_ONE) && !queryFlag(SEARCH_TWO)))) {
		setFlag(SEARCH_ONE);
		data.m_attr1_coord = m_subset_ref.validateAttribute(first);
		data.m_attr1.reset(new std::string(first));
	} else if (second != "" && first != "" && queryFlag(SEARCH_TWO)) {
		data.m_attr1_coord = m_subset_ref.validateAttribute(first);
		data.m_attr2_coord = m_subset_ref.validateAttribute(second);
		data.m_attr2.reset(new std::string(second));
		data.m_attr1.reset(new std::string(first));
	} else
		throw jedox::palo::ListBasicException("Set-Attributes parameters do not match flags", "Error using Alias-Filter");
}

/*@brief define advanced filters that remove any element that does not match any set of attribute-values
 @param columns: Each column represents valid values for an attribute. Each row is a complete filter.
 each list<string> represents a column. Not by reference because we change the columns */
void AliasFilter::setFilters(const std::vector<std::vector<std::string> >& columns)
{
	//columns must not be empty
	if (!queryFlag(USE_FILTEREXP))
		setFlag(USE_FILTEREXP);
	if (columns.empty()) {
		throw ListBasicException("No columns passed to Field Filter.", "Field Filter Data input error");
	}
	//first, get the attribute-names and validate them.
	if (columns.front().empty()) {
		throw ListBasicException("Empty attribute column passed to Field Filter.", "Field Filter Data input error");
	}
	AliasFilterData& data = *m_data;
	const size_t col_len = columns.front().size() - 1;
	BOOST_FOREACH( const std::vector<std::string>& col, columns ) {
		if ( col.empty() ) {
			throw ListBasicException( "Empty attribute column passed to Field Filter.", "Field Filter Data input error" );
		}
		if ( col.size() != col_len + 1 ) {
			throw ListBasicException( "Not all columns have the same length.", "Field Filter Data input error" );
		}
		string newstr = col.front();
		data.m_attributes.push_back( newstr );
	}
	//second, identify if the column type is operator-expression(true) or a simple string(false)
	std::vector<bool> col_types;
	//and identify if a column just uses "*" in that case we completely
	//forget about that one, since it should return true anyways
	std::vector<std::vector<std::string> >::const_iterator col(columns.begin()), end_col(columns.end());
	for (; col != end_col; ++col) {
		if (col->size() < 2) {
			throw ListBasicException("Empty column passed to Field Filter", "Field Filter Data input error");
		}
		if (boost::get<0>(AliasFilterHelper::getOperatorExpression(col->operator[](1))) == NO_OP) {
			col_types.push_back(false);
			data.m_dont_need_col.push_back(true);
		} else {
			col_types.push_back(true);
			data.m_dont_need_col.push_back(false);
		}

	}
	const size_t col_num = columns.size();
	data.m_filterexpressions.resize(col_len);
	for (unsigned int k = 0; k < col_len; k++) {
		data.m_filterexpressions[k].resize(col_num);
	}
	for (unsigned int i = 0; i < col_len; ++i) {
		for (unsigned int j = 0; j < col_num; ++j) {
			if (col_types[j]) {
				filterexpression_type t_ret = AliasFilterHelper::getOperatorExpression(columns[j][i + 1]);
				if ((boost::get<0>(t_ret)) == NO_OP) {
					throw ListBasicException("Columns passed to set_filters are not in proper format. Not all elements are of the same type.", "Alias filter input exception");
				}
				data.m_filterexpressions[i][j] = t_ret;
			} else {
				data.m_filterexpressions[i][j] = AliasFilterHelper::getOperatorExpression(columns[j][i + 1]);
				if ((boost::get<2>(data.m_filterexpressions[i][j])).str() != "^.*") {
					data.m_dont_need_col[j] = false;
				}
			}
		}
	}
	unsigned int idx = 0;
	std::vector<std::string>::iterator attr_it = data.m_attributes.begin();
	for (unsigned int j = 0; j != data.m_dont_need_col.size(); ++j) {
		if (data.m_dont_need_col[j] && attr_it != data.m_attributes.end()) {
			unsigned int tmp_idx = j + idx;
			filterexpression_vec_vec_type::iterator it = data.m_filterexpressions.begin();
			while (it != data.m_filterexpressions.end()) {
				filterexpression_vec_type::iterator erase_it = it->begin() + tmp_idx;
				it->erase(erase_it);
				++it;
			}
			attr_it = data.m_attributes.erase(attr_it);
			--idx;
		} else {
			++attr_it;

		}
	}
}

AliasFilter::~AliasFilter()
{
}

bool AliasFilter::insertAlias(const CELL_VALUE& cell_val, const ElementExList::iterator& it)
{
	if (cell_val.type != CELL_VALUE::ERROR) {
		if (cell_val.exists) {
			if (cell_val.type == CELL_VALUE::STRING) {
				it->search_alias = cell_val.val.s;
			} else {
				it->search_alias = cell_val.val.d;
			}
			return true;
		} else {
			if (cell_val.type == CELL_VALUE::NUMERIC) {
				it->search_alias = 0.0;
				return false;
			}
		}
	}
	it->search_alias = it->get_name();
	return false;
}

bool AliasFilter::insertDisplayAlias(const CELL_VALUE& cell_val, const ElementExList::iterator& it)
{
	if (cell_val.type == CELL_VALUE::STRING) {
		it->display_alias = cell_val.val.s;
		return true;
	} else {
		if (!m_data->m_display_attr) {
			throw ListBasicException(" no display attribute set! ", "Field Filter Data input error");
		}
		it->display_alias = *(m_data->m_display_attr);
		return false;
	}
}

inline void AliasFilter::setDisplayAttribute(const std::string& attr)
{
	m_data->m_display_attr_coord = m_subset_ref.validateAttribute(attr);
	m_data->m_display_attr.reset(new std::string(attr));
}
inline void AliasFilter::applyOneAlias(ElementExList& elemlist)
{
	applyOneAlias(elemlist, m_data->m_cell_vals);
}

void AliasFilter::applyOneAlias(ElementExList& elemlist, cell_value_vec_type& cell_vals)
{
	ElementExList::iterator it1 = elemlist.begin();
	ElementExList::iterator itend = elemlist.end();
	std::vector<CELL_VALUE>::const_iterator cv_it = cell_vals.begin();
	std::vector<CELL_VALUE>::const_iterator cv_end = cell_vals.end();
	while (it1 != itend && cv_it != cv_end) {
		insertAlias(*cv_it, it1);
		++it1;
		++cv_it;
	}
}

inline void AliasFilter::applyTwoAliases(ElementExList& elemlist)
{
	applyTwoAliases(elemlist, m_data->m_cell_vals);
}

void AliasFilter::applyTwoAliases(ElementExList& elemlist, cell_value_vec_type& cell_vals)
{
	ElementExList::iterator it1 = elemlist.begin();
	ElementExList::iterator itend = elemlist.end();
	std::vector<CELL_VALUE>::const_iterator cv_it = cell_vals.begin();
	std::vector<CELL_VALUE>::const_iterator cv_end = cell_vals.end();
	while (it1 != itend && cv_it != cv_end) {
		if (!insertAlias(*cv_it, it1)) {
			++cv_it;
			insertAlias(*cv_it, it1);
		} else {
			++cv_it;
		}
		++cv_it;
		++it1;
	}
}

void AliasFilter::applyOneDisplayAlias(ElementExList& elemlist, cell_value_vec_type& cell_vals)
{
	ElementExList::iterator it1 = elemlist.begin();
	ElementExList::iterator itend = elemlist.end();
	std::vector<CELL_VALUE>::const_iterator cv_it = cell_vals.begin();
	std::vector<CELL_VALUE>::const_iterator cv_end = cell_vals.end();
	while (it1 != itend && cv_it != cv_end) {
		insertDisplayAlias(*cv_it, it1);
		++it1;
		++cv_it;
	}
}

void AliasFilter::applyTwoDisplayAliases(ElementExList& elemlist, cell_value_vec_type& cell_vals)
{
	ElementExList::iterator it1 = elemlist.begin();
	ElementExList::iterator itend = elemlist.end();
	std::vector<CELL_VALUE>::const_iterator cv_it = cell_vals.begin();
	std::vector<CELL_VALUE>::const_iterator cv_end = cell_vals.end();
	while (it1 != itend && cv_it != cv_end) {
		if (!insertDisplayAlias(*cv_it, it1)) {
			++cv_it;
			insertDisplayAlias(*cv_it, it1);
		} else {
			++cv_it;
		}
		++cv_it;
		++it1;
	}
}
} //palo
} //jedox
