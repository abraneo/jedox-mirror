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

#include "AliasFilter.h"
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

#include "SubSet.h"
#include "PickList.h"

#ifdef _MSC_VER
#undef max
#undef min
#endif

#include "Olap/AttributedDimension.h"

namespace palo {

struct AliasFilter::AliasFilterHelper {
	static inline filterexpression_type getOperatorExpression(const string& s)
	{
		//we accept empty string as * too
		string new_s = s;
		if ("" == new_s) {
			new_s = "*";
		}
		string h = new_s.substr(0, 2);
		int op_size = 0;
		AliasFilter::Optype t_op = AliasFilter::NO_OP;
		if (h == string("<=")) {
			t_op = AliasFilter::OP_GEQ;
			op_size = 2;
		} else if (h == string(">=")) {
			t_op = AliasFilter::OP_LEQ;
			op_size = 2;
		} else if (h == string("<>")) {
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
			string sub = new_s.substr(op_size);
			double ret_val = StringUtils::stringToDouble(sub);
			return filterexpression_type(t_op, ret_val, boost::regex());
		}
		return filterexpression_type(AliasFilter::NO_OP, 0, boost::regex(getRegex(new_s)));
	}

	static inline string getRegex(const string& ss)
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
};

struct AliasFilter::FilterOperator {
	static inline bool check(const CellValue& cval, const filterexpression_type& f)
	{
		if (cval.isError()) {
			return false;
		} 
		Optype tp = boost::get<0>(f);
		if (tp == NO_OP) {
			if (!cval.isString() && !cval.empty() && ((boost::get<2>(f)).str() != "^.*")) {
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "String expected but double/error found as cell value");
			} else {
				if (!boost::regex_search(cval, boost::get<2>(f))) {
					return false;
				}
			}
		} else {
			double val = 0.0;
			if (cval.isString()) {
				try {
					val = StringUtils::stringToDouble(cval);
				} catch (ParameterException &) {
				}
			} else if (cval.isNumeric()) {
				val = cval.getNumeric();
			} else {
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Unknown type inside Cell value");
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
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Unknown operator");
			}
		}
		return true;
	}
};

AliasFilter::AliasFilter(SubSet& s, AliasFilterSettings &settings, FieldFilterSettings &fs) :
	Filter(s, 0, ALIAS_FILTER_NUMFLAGS), m_settings(settings), m_fs(fs)
{
	if (m_settings.active) {
		filter_flags |= m_settings.flags;
	}
	if (m_fs.active) {
		filter_flags |= m_fs.flags;
	}

	if (queryFlag(SEARCH_ONE) && queryFlag(SEARCH_TWO)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Alias Filter construction with both SEARCH_ONE and SEARCH_TWO impossible.");
	}
}

ElementsType AliasFilter::apply(bool &worked)
{
	applySettings();
	CPCube attrcube = AttributedDimension::getAttributesCube(m_subset_ref.getDatabase(), m_subset_ref.getDimension()->getName());
	if (!attrcube) {
		throw ErrorException(ErrorException::ERROR_CUBE_NOT_FOUND, "Attribute cube doesn't exist.");
	}
	PCubeArea area(new CubeArea(m_subset_ref.getDatabase(), attrcube, 2));

	PSet s(new Set());
	if (queryFlag(SEARCH_ONE) || queryFlag(SEARCH_TWO)) {
		s->insert(m_attr1_coord);
		m_subset_ref.setGlobalFlag(SubSet::ALIAS_FILTER_ACTIVE);
	}
	if (queryFlag(SEARCH_TWO)) {
		s->insert(m_attr2_coord);
	}
	if (queryFlag(USE_FILTEREXP) && !m_filterexp.empty()) {
		for (IdentifiersType::iterator it = m_filter_coords.begin(); it != m_filter_coords.end(); ++it) {
			s->insert(*it);
		}
	}
	set<IdentifierType> flt(m_filter_coords.begin(), m_filter_coords.end());
	area->insert(0, s);
	area->insert(1, m_subset_ref.getSet(true));

	// This is a bit complicated
	// first map is element ids
	// second map is attribute ids
	// set contains condition numbers that were valid for that attribute
	// bool is for information if the value was processed in calculateArea, others have to be checked against empty value
	// element is valid if there is at least one condition, that was valid for all attributes
	map<IdentifierType, map<IdentifierType, pair<set<int>, bool> > > valid;
	for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
		for (IdentifiersType::iterator fit = m_filter_coords.begin(); fit != m_filter_coords.end(); ++fit) {
			valid[it.getId()][*fit] = make_pair(set<int>(), false);
		}
	}
	CellValue def;
	PCellStream cs = attrcube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
	if (cs) {
		while (cs->next()) {
			const CellValue &val = cs->getValue();
			const IdentifiersType &key = cs->getKey();
			if ((queryFlag(SEARCH_ONE) || queryFlag(SEARCH_TWO)) && m_attr1_coord == key[0]) {
				if (!val.isError() && m_subset_ref.getSearchAlias(key[1], def).isEmpty()) {
					m_subset_ref.setSearchAlias(key[1], val);
				}
			}
			if (queryFlag(SEARCH_TWO) && m_attr2_coord == key[0]) {
				if (!val.isError()) {
					m_subset_ref.setSearchAlias(key[1], val);
				}
			}
			if (queryFlag(USE_FILTEREXP) && !m_filterexp.empty() && flt.find(key[0]) != flt.end()) {
				pair<set<int>, bool> &v = valid[key[1]][key[0]];
				v.second = true;
				v.first =check(val, key[0]);
			}
		}
	}
	ElementsType ret;
	if (queryFlag(USE_FILTEREXP) && !m_filterexp.empty()) {
		worked = true;
		for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
			map<IdentifierType, pair<set<int>, bool> > &v = valid[it.getId()];
			bool validid = true;
			set<int> curr;
			for (map<IdentifierType, pair<set<int>, bool> >::iterator vit = v.begin(); vit != v.end(); ++vit) {
				if (!vit->second.second) {
					vit->second.first = check(CellValue::NullNumeric, vit->first);
				}
				if (vit == v.begin()) {
					curr = vit->second.first;
				} else {
					set<int> tmp;
					set_intersection(curr.begin(), curr.end(), vit->second.first.begin(), vit->second.first.end(), insert_iterator<set<int > >(tmp, tmp.end()));
					curr.swap(tmp);
				}
				if (curr.empty()) {
					validid = false;
					break;
				}
			}
			if (validid) {
				ret.push_back(it.getElement());
			}
		}
	}
	return ret;
}

void AliasFilter::applySettings()
{
	if (m_settings.active) {
		if ((m_settings.attribute1 != "" && queryFlag(SEARCH_ONE)) || ((!queryFlag(SEARCH_ONE) && !queryFlag(SEARCH_TWO)))) {
			setFlag(SEARCH_ONE);
			m_attr1_coord = m_subset_ref.validateAttribute(m_settings.attribute1);
		} else if (m_settings.attribute2 != "" && m_settings.attribute1 != "" && queryFlag(SEARCH_TWO)) {
			m_attr1_coord = m_subset_ref.validateAttribute(m_settings.attribute1);
			m_attr2_coord = m_subset_ref.validateAttribute(m_settings.attribute2);
		} else
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Set-Attributes parameters do not match flags");
	}

	if (m_fs.active) {
		vector<bool> dont_need_col;
		if (!queryFlag(USE_FILTEREXP))
			setFlag(USE_FILTEREXP);
		if (m_fs.advanced.empty()) {
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "No columns passed to Field Filter.");
		}
		if (m_fs.advanced.front().empty()) {
			throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Empty attribute column passed to Field Filter.");
		}
		const size_t col_len = m_fs.advanced.front().size() - 1;
		vector<string> attributes;
		BOOST_FOREACH( const vector<string>& col, m_fs.advanced ) {
			if ( col.empty() ) {
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Empty attribute column passed to Field Filter.");
			}
			if ( col.size() != col_len + 1 ) {
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Not all columns have the same length.");
			}
			string newstr = col.front();
			attributes.push_back( newstr );
		}
		vector<bool> col_types;
		vector<vector<string> >::const_iterator col(m_fs.advanced.begin()), end_col(m_fs.advanced.end());
		for (; col != end_col; ++col) {
			if (col->size() < 2) {
				throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Empty column passed to Field Filter");
			}
			if (boost::get<0>(AliasFilterHelper::getOperatorExpression(col->operator[](1))) == NO_OP) {
				col_types.push_back(false);
				dont_need_col.push_back(true);
			} else {
				col_types.push_back(true);
				dont_need_col.push_back(false);
			}

		}

		const size_t col_num = m_fs.advanced.size();
		for (unsigned int i = 0; i < col_num; ++i) {
			filterexpression_vec_type fe;
			fe.resize(col_len);
			for (unsigned int j = 0; j < col_len; ++j) {
				if (col_types[i]) {
					fe[j] = AliasFilterHelper::getOperatorExpression(m_fs.advanced[i][j + 1]);
					if ((boost::get<0>(fe[j])) == NO_OP && (boost::get<2>(fe[j])).str() != "^.*") {
						throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Columns passed to set_filters are not in proper format. Not all elements are of the same type.");
					}
				} else {
					fe[j] = AliasFilterHelper::getOperatorExpression(m_fs.advanced[i][j + 1]);
					if ((boost::get<2>(fe[j])).str() != "^.*") {
						dont_need_col[i] = false;
					}
				}
			}
			if (!dont_need_col[i]) {
				IdentifierType attr = m_subset_ref.validateAttribute(attributes[i]);
				m_filterexp.insert(make_pair(attr, fe));
				m_filter_coords.push_back(attr);
			}
		}
	}
}

set<int> AliasFilter::check(const CellValue &val, IdentifierType attrId)
{
	set<int> ret;

	bool first = true;
	for (filterexpression_map_vec_type::iterator ait = m_filterexp.find(attrId); ait != m_filterexp.end() && ait->first == attrId; ++ait) {
		int i = 0;
		for (filterexpression_vec_type::iterator fit = ait->second.begin(); fit != ait->second.end(); ++fit, ++i) {
			if (FilterOperator::check(val, *fit)) {
				if (first) {
					ret.insert(i);
				}
			} else {
				if (!first) {
					ret.erase(i);
				}
			}
		}
		if (ret.empty()) {
			break;
		}
		first = false;
	}
	return ret;
}

} //palo
