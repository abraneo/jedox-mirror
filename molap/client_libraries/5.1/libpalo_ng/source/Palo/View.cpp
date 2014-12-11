/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include <vector>

#include "View.h"
#include "libpalo_ng/Palo/ServerPool.h"
#include "libpalo_ng/Palo/Server.h"
#include "libpalo_ng/Palo/Database.h"
#include "libpalo_ng/Palo/Cube.h"
#include "libpalo_ng/Palo/Dimension.h"

#include "../Util/CsvLineDecoder.h"
#include "../Util/CsvTokenFromStream.h"

using namespace std;

namespace jedox {
namespace palo {

struct Filter {
	enum FilterType {
		PICKLIST = 0, STRUCTURAL, ALIAS, TEXT, DATA, SORTING, TOTAL_NUMBER
	};
};

static ostream &operator<<(ostream &str, const BasicFilterSettings &set)
{
	str << Filter::PICKLIST << ";";
	str << set.flags << ";";
	vector<string>::const_iterator itp = set.manual_paths.begin();
	for (vector<string>::const_iterator it = set.manual_subset.begin(); it != set.manual_subset.end(); ++it) {
		if (it != set.manual_subset.begin()) {
			str << ":";
		}
		jedox::util::URLencoder(str, util::StringUtils::escapeString(*it));
		if (itp != set.manual_paths.end()) {
			str << "," << *itp++;
		}
	}
	return str;
}

static ostream &operator<<(ostream &str, const TextFilterSettings &set)
{
	str << Filter::TEXT << ";";
	str << set.flags << ";";
	for (vector<string>::const_iterator it = set.regexps.begin(); it != set.regexps.end(); ++it) {
		if (it != set.regexps.begin()) {
			str << ":";
		}
		jedox::util::URLencoder(str, util::StringUtils::escapeString(*it));
	}
	return str;
}

static ostream &operator<<(ostream &str, const SortingFilterSettings &set)
{
	str << Filter::SORTING << ";";
	str << set.flags << ";" << set.indent << ";";
	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.attribute));
	str << ";" << set.level << ";" << set.limit_count << ";" << set.limit_start;
	return str;
}

static ostream &operator<<(ostream &str, const StructuralFilterSettings &set)
{
	str << Filter::STRUCTURAL << ";";
	str << set.flags << ";" << set.indent << ";";
	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.bound));
	str << ";";
	if (set.level) {
		str << set.level_start << ";" << set.level_end << ";";
	} else {
		str << ";;";
	}
	if (set.revolve) {
		str << set.revolve_count << ";";
		jedox::util::URLencoder(str, util::StringUtils::escapeString(set.revolve_elem));
	} else {
		str << ";";
	}
	return str;
}

static ostream &operator<<(ostream &str, const DataFilterSettings &set)
{
	str << Filter::DATA << ";";
	str << set.flags << ";";
	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.cube));

	str << ";" << (set.cmp.use_strings ? "1" : "0") << ";";
	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.cmp.op1));
	str << ";";
	if (set.cmp.use_strings) {
		jedox::util::URLencoder(str, util::StringUtils::escapeString(set.cmp.par1s));
	} else {
		stringstream tmp;
		tmp << set.cmp.par1d;
		jedox::util::URLencoder(str, tmp.str());
	}
	str << ";";

	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.cmp.op2));
	str << ";";
	if (set.cmp.use_strings) {
		jedox::util::URLencoder(str, util::StringUtils::escapeString(set.cmp.par2s));
	} else {
		stringstream tmp;
		tmp << set.cmp.par2d;
		jedox::util::URLencoder(str, tmp.str());
	}
	str << ";" << set.coords.size() << ";";

	for (DataFilterSettings::CoordsType::const_iterator it = set.coords.begin(); it != set.coords.end(); ++it) {
		if (!it->first) {
			for (vector<string>::const_iterator cit = it->second.begin(); cit != it->second.end(); ++cit) {
				if (cit != it->second.begin()) {
					str << ":";
				}
				jedox::util::URLencoder(str, util::StringUtils::escapeString(*cit));
			}
		}
		str << ";";
	}

	if (set.upper_percentage_set) {
		stringstream tmp;
		tmp << set.upper_percentage;
		jedox::util::URLencoder(str, tmp.str());
	}
	str << ";";
	if (set.lower_percentage_set) {
		stringstream tmp;
		tmp << set.lower_percentage;
		jedox::util::URLencoder(str, tmp.str());
	}
	str << ";" << set.top;
	return str;
}

static ostream &operator<<(ostream &str, const VIEW_AXIS_SUBSET &set)
{
	str << set.subsetHandle << ";";
	str << set.parent << ";";
	jedox::util::URLencoder(str, util::StringUtils::escapeString(set.calculation));
	str << ";" << (set.zeroSup ? "1" : "0");
	return str;
}

void VIEW_SUBSET::Serialize(Server *server)
{
	stringstream str;

	jedox::util::URLencoder(str, util::StringUtils::escapeString(dimension));
	for (vector<BasicFilterSettings>::iterator it = basic.begin(); it != basic.end(); ++it) {
		if (it->active) {
			str << ";" << *it;
		}
	}
	if (text.active) {
		str << ";" << text;
	}
	if (sorting.active) {
		str << ";" << sorting;
	}
	if (alias.active || field.active) {
		str << ";" << Filter::ALIAS << ";";
		unsigned int flags = 0;
		if (alias.active) {
			flags |= alias.flags;
		}
		if (field.active) {
			flags |= field.flags;
		}
		str << flags << ";";
		if (alias.active) {
			jedox::util::URLencoder(str, util::StringUtils::escapeString(alias.attribute1));
			str << ";";
			jedox::util::URLencoder(str, util::StringUtils::escapeString(alias.attribute2));
		} else {
			str << ";";
		}
		str << ";";
		if (field.active && !field.advanced.empty()) {
			size_t siz = field.advanced[0].size();
			str << siz << ";";
			for (vector<vector<string> >::const_iterator it = field.advanced.begin(); it != field.advanced.end(); ++it) {
				if (it->size() != siz) {
					string descr = "Lines with unequal length in attribute filter.";
					LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
				}
				for (vector<string>::const_iterator fit = it->begin(); fit != it->end(); ++fit) {
					if (it != field.advanced.begin() || fit != it->begin()) {
						str << ":";
					}
					jedox::util::URLencoder(str, util::StringUtils::escapeString(*fit));
				}
			}
		} else {
			str << "0";
		}
	}
	for (vector<StructuralFilterSettings>::iterator it = structural.begin(); it != structural.end(); ++it) {
		if (it->active) {
			str << ";" << *it;
		}
	}
	for (vector<DataFilterSettings>::iterator it = data.begin(); it != data.end(); ++it) {
		if (it->active) {
			size_t dims = (*server)[database].cube[it->cube].getCacheData().dimensions.size();
			if (dims != it->coords.size()) {
				it->coords.resize(dims);
			}
			str << ";" << *it;
		}
	}

	ser_str = str.str();
	str.str("");
	str << database << ";" << indent << ";" << ser_str;
	ser_ident = str.str();
}

void VIEW_AXIS::Serialize()
{
	stringstream str;
	for (vector<VIEW_AXIS_SUBSET>::iterator it = as.begin(); it != as.end(); ++it) {
		if (it != as.begin()) {
			str << ";";
		}
		str << *it;
	}

	ser_str = str.str();
	str.str("");
	str << database << ";" << ser_str;
	ser_ident = str.str();
}

void VIEW_AREA::Serialize()
{
	stringstream str;
	for (vector<string>::iterator it = properties.begin(); it != properties.end(); ++it) {
		if (it != properties.begin()) {
			str << ":";
		}
		jedox::util::URLencoder(str, util::StringUtils::escapeString(*it));
	}
	ser_str = str.str();

	str.str("");
	str << database << ";" << cube << ";";
	for (vector<pair<string, boost::shared_ptr<VIEW_AXIS> > >::iterator it = axes.begin(); it != axes.end(); ++it) {
		if (it != axes.begin()) {
			str << ":";
		}
		str << it->first;
	}
	str << ";" << ser_str;
	ser_ident = str.str();
}

View::View(Server *server) : m_Server(server)
{
}

string View::defineViewSubset(const string &database, const string &dimension, int indent, const vector<BasicFilterSettings> &basic, const TextFilterSettings &text, const SortingFilterSettings &sorting, const AliasFilterSettings &alias, const FieldFilterSettings &field, const vector<StructuralFilterSettings> &structural, const vector<DataFilterSettings> &data)
{
	boost::shared_ptr<VIEW_SUBSET> sub(new VIEW_SUBSET);
	sub->database = database;
	sub->dimension = dimension;
	sub->indent = indent;
	sub->basic = basic;
	sub->text = text;
	sub->sorting = sorting;
	sub->alias = alias;
	sub->field = field;
	sub->structural = structural;
	sub->data = data;
	sub->Serialize(m_Server);

	string subset_id;
	boost::unique_lock<boost::mutex> lock(m_Lock);
	unordered_map<string, string>::iterator it = m_SubsetsCache.find(sub->ser_ident);
	if (it != m_SubsetsCache.end()) {
		subset_id = it->second;
		m_Subsets[subset_id].first = boost::posix_time::microsec_clock::universal_time();
	} else {
		do {
			subset_id = ServerPool::getRandId();
		} while (m_Subsets.find(subset_id) != m_Subsets.end());
		m_SubsetsCache[sub->ser_ident] = subset_id;
		m_Subsets[subset_id] = make_pair(boost::posix_time::microsec_clock::universal_time(), sub);
	}
	deleteOld();
	return subset_id;
}

string View::defineViewAxis(const string &database, int axisId, const AxisSubsets &as)
{
	if (axisId < 0 || axisId > 2) {
		stringstream str;
		str << "Bad axis id: " << axisId;
		string descr = str.str();
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
	}

	boost::shared_ptr<VIEW_AXIS> ax(new VIEW_AXIS);
	ax->database = database;
	ax->axisId = axisId;
	boost::unique_lock<boost::mutex> lock(m_Lock);
	ax->as.reserve(as.size());
	for (AxisSubsets::const_iterator it = as.begin(); it != as.end(); ++it) {
		unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_SUBSET> > >::const_iterator subit = m_Subsets.find(it->subsetHandle);
		if (subit == m_Subsets.end()) {
			stringstream str;
			str << "Subset not found: " << it->subsetHandle;
			string descr = str.str();
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
		}
		ax->as.push_back(VIEW_AXIS_SUBSET(it->subsetHandle, subit->second.second, it->parent, it->calculation, it->zeroSup));
	}
	ax->Serialize();

	string axis_id;
	unordered_map<string , string>::iterator it = m_AxisCache.find(ax->ser_ident);
	if (it != m_AxisCache.end()) {
		axis_id = it->second;
		m_Axes[axis_id].first = boost::posix_time::microsec_clock::universal_time();
	} else {
		do {
			axis_id = ServerPool::getRandId();
		} while (m_Axes.find(axis_id) != m_Axes.end());
		m_AxisCache[ax->ser_ident] = axis_id;
		m_Axes[axis_id] = make_pair(boost::posix_time::microsec_clock::universal_time(), ax);
	}
	deleteOld();
	return axis_id;
}

string View::defineViewArea(const string &database, const string &cube, const vector<string> &axes, const vector<string> &properties)
{
	boost::shared_ptr<VIEW_AREA> ar(new VIEW_AREA);
	string area_id;
	ar->database = database;
	ar->cube = cube;
	ar->properties = properties;
	ar->axes.resize(3);
	{
		boost::unique_lock<boost::mutex> lock(m_Lock);
		for (vector<string>::const_iterator it = axes.begin(); it != axes.end(); ++it) {
			if (!it->empty()) {
				unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AXIS> > >::const_iterator axit = m_Axes.find(*it);
				if (axit == m_Axes.end()) {
					stringstream str;
					str << "Axis not found: " << *it;
					string descr = str.str();
					LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
				}
				ar->axes[axit->second.second->axisId] = make_pair(*it, axit->second.second);
			}
		}
		ar->Serialize();
		unordered_map<string , string>::iterator it = m_AreasCache.find(ar->ser_ident);
		if (it != m_AreasCache.end()) {
			area_id = it->second;
			if (m_Server->getDataToken() != m_Areas[area_id].second->calc->sequence) {
				area_id = "";
				m_AreasCache.erase(it);
			} else {
				m_Areas[area_id].first = boost::posix_time::microsec_clock::universal_time();
			}
		}
		deleteOld();
	}

	if (area_id.empty()) {
		calculateView(ar);
		boost::unique_lock<boost::mutex> lock(m_Lock);
		do {
			area_id = string("\t") + ServerPool::getRandId();
		} while (m_Areas.find(area_id) != m_Areas.end());
		if (!ar->calc->virt) {
			m_AreasCache[ar->ser_ident] = area_id;
		}
		m_Areas[area_id] = make_pair(boost::posix_time::microsec_clock::universal_time(), ar);
	}
	return area_id;
}

size_t View::getViewAxisSize(const string &database, const string &viewHandle, int axisId)
{
	unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator it;
	{
		boost::unique_lock<boost::mutex> lock(m_Lock);
		it = m_Areas.find(viewHandle);
		if (it == m_Areas.end()) {
			stringstream str;
			str << "Area not found: " << viewHandle;
			string descr = str.str();
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
		} else {
			it->second.first = boost::posix_time::microsec_clock::universal_time();
		}
	}

	if (axisId < 0 || axisId > 2) {
		stringstream str;
		str << "Bad axis id: " << axisId;
		string descr = str.str();
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
	}
	return it->second.second->calc->axes[axisId]->tuples.size();
}

ELEMENT_INFO_EXT View::getViewSubset(const string &database, const string &viewHandle, int axisId, size_t subsetPos, size_t elemPos, UINT &indent)
{
	unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator it;
	{
		boost::unique_lock<boost::mutex> lock(m_Lock);
		it = m_Areas.find(viewHandle);
		if (it == m_Areas.end()) {
			stringstream str;
			str << "Area not found: " << viewHandle;
			string descr = str.str();
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
		} else {
			it->second.first = boost::posix_time::microsec_clock::universal_time();
		}
	}
	if (axisId < 0 || axisId > 2 || it->second.second->axes[axisId].first.empty()) {
		stringstream str;
		str << "Bad axis id: " << axisId;
		string descr = str.str();
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
	}
	boost::shared_ptr<VIEW_CALC_AXIS> ax = it->second.second->calc->axes[axisId];
	indent = it->second.second->axes[axisId].second->as[subsetPos].sub->indent;
	if (elemPos < ax->tuples.size()) {
		if (subsetPos < ax->tuples[elemPos].size()) {
			return ax->tuples[elemPos][subsetPos];
		}
	}
	return ELEMENT_INFO_EXT();
}

AxisElement View::getViewSubset(const string &database, const string &viewHandle, const string &dim, size_t elemPos)
{
	pair<size_t, size_t> axpos;
	IdentifierType indent;
	boost::shared_ptr<VIEW_CALC_AXIS> ax = getAxisAndPos(axpos, indent, database, viewHandle, dim, elemPos);
	if (elemPos < ax->tuples.size()) {
		if (axpos.second < ax->tuples[elemPos].size()) {
			ELEMENT_INFO_EXT &el = ax->tuples[elemPos][axpos.second];
			return AxisElement(el.get_name(), el.m_einf.type, el.get_alias(), !el.childrenInSub.empty(), !el.parentsInSub.empty(), axpos.first, el.get_idx(indent), elemPos);
		}
	}
	return AxisElement(axpos.first);
}

CELL_VALUE_PROPS View::getViewArea(const string &database, const string &viewHandle, const vector<string> &coord, bool cubeException, vector<string> *&properties)
{
	boost::shared_ptr<VIEW_AREA> va;
	{
		boost::unique_lock<boost::mutex> lock(m_Lock);
		unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator it = m_Areas.find(viewHandle);
		if (it == m_Areas.end()) {
			stringstream str;
			if (cubeException) {
				str << "Couldn't resolve cube name \"" << viewHandle << "\" in database \"" << database << "\".";
				throw CubeNotFoundException(str.str());
			} else {
				str << "Area not found: " << viewHandle;
				string descr = str.str();
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
			}
		} else {
			it->second.first = boost::posix_time::microsec_clock::universal_time();
			va = it->second.second;
		}
	}
	if (coord.size() != va->calc->dimonaxis.size()) {
		string descr = "Coordinate size differs from subsets count.";
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
	}
	vector<string>::const_iterator coit;
	for (coit = coord.begin(); coit != coord.end(); ++coit) {
		if (coit->empty()) {
			break;
		}
	}
	properties = &va->properties;
	if (coit != coord.end()) {
		CELL_VALUE_PROPS res;
		res.exists = false;
		res.type = CELL_VALUE::STRING;
		res.prop_vals.resize(va->properties.size());
		return res;
	} else {
		ELEMENT_LIST key;
		key.reserve(va->calc->dimonaxis.size());
		vector<pair<size_t, size_t> >::const_iterator dimit;
		bool stringElem = false;
		for (dimit = va->calc->dimonaxis.begin(), coit = coord.begin(); dimit != va->calc->dimonaxis.end(); ++dimit, ++coit) {
			boost::shared_ptr<VIEW_CALC_AXIS> axis = va->calc->axes[dimit->first];
			unordered_map<string, pair<size_t, ELEMENT_INFO::TYPE> >::iterator nit = axis->nameIndex[dimit->second].find(*coit);
			if (nit == axis->nameIndex[dimit->second].end()) {
				stringstream str;
				str << "Element not in view: " << *coit;
				string descr = str.str();
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
			}
			key.push_back(nit->second.first);
			if (nit->second.second == ELEMENT_INFO::STRING) {
				stringElem = true;
			}
		}
		VIEW_AREA::valiter vi = va->calc->values.find(key);
		if (vi == va->calc->values.end()) {
			CELL_VALUE_PROPS res;
			res.exists = false;
			res.val.d = 0;
			if (stringElem) {
				res.type = CELL_VALUE::STRING;
			} else {
				res.type = CELL_VALUE::NUMERIC;
			}
			res.prop_vals.resize(va->properties.size());
			return res;
		} else {
			return vi->second;
		}
	}
}

vector<AxisElement> View::getViewSubsetTop(const string &database, const string &viewHandle, const string &dim, size_t elemPos, size_t start, size_t limit)
{
	vector<AxisElement> ret;
	pair<size_t, size_t> axpos;
	IdentifierType indent;
	boost::shared_ptr<VIEW_CALC_AXIS> ax = getAxisAndPos(axpos, indent, database, viewHandle, dim, elemPos);

	vector<ELEMENT_INFO_EXT> v;
	v.reserve(axpos.second);
	for (size_t j = 0; j < axpos.second; ++j) {
		v.push_back(ax->tuples[elemPos][j]);
	}
	for (size_t i = elemPos + start; i < ax->tuples.size(); ++i) {
		if (axpos.second < ax->tuples[i].size()) {
			if (!hasSameParentSubsets(v, ax->tuples[i], axpos.second) || ret.size() == limit) {
				break;
			}
			ELEMENT_INFO_EXT &el = ax->tuples[i][axpos.second];
			if (el.parentsInSub.empty() && (ret.empty() || el.get_name() != ret[ret.size() - 1].name)) {
				ret.push_back(AxisElement(el.get_name(), el.m_einf.type, el.get_alias(), !el.childrenInSub.empty(), !el.parentsInSub.empty(), axpos.first, el.get_idx(indent), i));
			}
		}
	}
	return ret;
}

vector<AxisElement> View::getViewSubsetChildren(const string &database, const string &viewHandle, const string &dim, size_t elemPos, const string &element)
{
	vector<AxisElement> ret;
	pair<size_t, size_t> axpos;
	IdentifierType indent;
	boost::shared_ptr<VIEW_CALC_AXIS> ax = getAxisAndPos(axpos, indent, database, viewHandle, dim, elemPos);

	vector<ELEMENT_INFO_EXT> v;
	v.reserve(axpos.second);
	for (size_t j = 0; j < axpos.second; ++j) {
		v.push_back(ax->tuples[elemPos][j]);
	}
	for (size_t i = elemPos; i < ax->tuples.size(); ++i) {
		if (axpos.second < ax->tuples[i].size()) {
			if (!hasSameParentSubsets(v, ax->tuples[i], axpos.second)) {
				break;
			}
			ELEMENT_INFO_EXT &el = ax->tuples[i][axpos.second];
			if (el.get_name() == element) {
				for (vector<size_t>::iterator it = el.childrenInSub.begin(); it != el.childrenInSub.end(); ++it) {
					ELEMENT_INFO_EXT &child = ax->tuples[*it][axpos.second];
					ret.push_back(AxisElement(child.get_name(), child.m_einf.type, child.get_alias(), !child.childrenInSub.empty(), !child.parentsInSub.empty(), axpos.first, child.get_idx(indent), *it));
				}
				break;
			}
		}
	}
	return ret;
}

void View::updateParentChild(boost::shared_ptr<VIEW_CALC_AXIS> curr, size_t start, size_t ind, vector<map<IdentifierType, size_t> > &knownElems)
{
	size_t tuplen = curr->tuples.size();
	for (size_t i = start; i < tuplen; ++i) {
		for (ELEMENT_LIST::iterator it = curr->tuples[i][ind].m_einf.children.begin(); it != curr->tuples[i][ind].m_einf.children.end(); ++it) {
			map<IdentifierType, size_t>::iterator kit = knownElems[ind].find(*it);
			if (kit != knownElems[ind].end()) {
				curr->tuples[i][ind].childrenInSub.push_back(kit->second);
			}
		}
		for (ELEMENT_LIST::iterator it = curr->tuples[i][ind].m_einf.parents.begin(); it != curr->tuples[i][ind].m_einf.parents.end(); ++it) {
			map<IdentifierType, size_t>::iterator kit = knownElems[ind].find(*it);
			if (kit != knownElems[ind].end()) {
				curr->tuples[i][ind].parentsInSub.push_back(kit->second);
			}
		}
	}
}

void View::calculateView(boost::shared_ptr<VIEW_AREA> view_area)
{
	stringstream subsets_str, axes_str, req_str;
	Database db = (*m_Server)[view_area->database];
	DIMENSION_LIST dims;
	UINT cubeId = (UINT)-1;
	if (!view_area->cube.empty()) {
		Cube cube = db.cube[view_area->cube];
		dims = cube.getCacheData().dimensions;
		cubeId = cube.getCacheData().cube;
	}
	view_area->calc.reset(new VIEW_CALC);
	view_area->calc->virt = false;
	view_area->calc->dimonaxis.resize(dims.size());
	view_area->calc->axes.resize(view_area->axes.size());
	{
		for (size_t axit = 0; axit < view_area->axes.size(); ++axit) {
			if (axit) {
				axes_str << "$";
			}
			if (!view_area->axes[axit].first.empty()) {
				boost::shared_ptr<VIEW_AXIS> axe = view_area->axes[axit].second;
				if (util::UTF8Comparer::compare(axe->database, view_area->database)) {
					string descr = "Databases differ.";
					LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
				}
				axes_str << axe->ser_str;
				for (size_t subit = 0; subit < axe->as.size(); ++subit) {
					boost::shared_ptr<VIEW_SUBSET> sub = axe->as[subit].sub;
					if (util::UTF8Comparer::compare(sub->database, view_area->database)) {
						string descr = "Databases differ.";
						LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
					}
					if (!subsets_str.str().empty()) {
						subsets_str << "$";
					}
					subsets_str << axe->as[subit].subsetHandle << ";" << sub->ser_str;
					Dimension d = (*m_Server)[axe->database].dimension[sub->dimension];
					size_t i;
					for (i = 0; i < dims.size(); ++i) {
						if (dims[i] == d.getCacheData().dimension) {
							view_area->calc->dimonaxis[i] = make_pair(axit, subit);
							view_area->calc->dim2axis[d.getCacheData().ndimension] = i;
							break;
						}
					}
					if (!dims.empty() && i == dims.size()) {
						stringstream str;
						str << "Dimension not in cube. Dimension: " << sub->dimension << " Cube: " << view_area->cube;
						string descr = str.str();
						LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
					}
					if (d.getCacheData().type == DIMENSION_INFO::SYSTEM_ID) {
						view_area->calc->virt = true;
					}
				}
				boost::shared_ptr<VIEW_CALC_AXIS> cl(new VIEW_CALC_AXIS);
				view_area->calc->axes[axit] = cl;
				cl->nameIndex.resize(axe->as.size());
			}
		}
	}
	req_str << "database=" << db.getCacheData().database << "&view_subsets=" << subsets_str.str() << "&view_axes=" << axes_str.str() << "&view_area=" << view_area->ser_str;
	if (!view_area->cube.empty()) {
		req_str << "&cube=" << cubeId;
	}

	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN token(sequencenumber);
	unique_ptr<istringstream> stream = m_Server->m_PaloClient->request("/view/calculate", req_str.str(), token, sequencenumber, dummy);
	view_area->calc->sequence = m_Server->getDataToken();
	boost::shared_ptr<VIEW_CALC_AXIS> curr;
	vector<size_t> startLine;
	vector<map<IdentifierType, size_t> > knownElems;
	bool area = false;
	while ((*stream).eof() == false) {
		if ((*stream).peek() == '[') {
			char line[1024];
			(*stream).getline(line, sizeof(line));
			string s(line);
			if (curr) {
				for (size_t i = 0; i < startLine.size(); ++i) {
					updateParentChild(curr, startLine[i], i, knownElems);
				}
				curr.reset();
			}
			if (s == "[Area]") {
				area = true;
			} else {
				s = s.substr(6, s.size() - 7);
				size_t pos = s.find_first_of(' ');
				if (pos != string::npos) {
					s = s.substr(0, pos);
				}
				curr = view_area->calc->axes[util::lexicalConversion(size_t, string, s)];
				startLine.clear();
				startLine.resize(curr->nameIndex.size(), 0);
				knownElems.clear();
				knownElems.resize(curr->nameIndex.size());
			}
		} else {
			if (area) {
				CELL_VALUE_PATH_PROPS val;
				CELL_VALUE_PROPS vp;
				(*stream) >> csv >> val;
				vp = val;
				view_area->calc->values[val.path] = vp;
			} else {
				vector<ELEMENT_INFO_EXT> v;
				size_t tuplen = curr->tuples.size();
				for (size_t i = 0; i < curr->nameIndex.size(); ++i) {
					ELEMENT_INFO_EXT element;
					(*stream) >> csv >> element.m_einf;

					jedox::util::CsvTokenFromStream tfs((*stream) >> csv);
					tfs.get(element.search_alias);
					tfs.get(element.path);

					if (tuplen) {
						if (!hasSameParentSubsets(v, curr->tuples[tuplen - 1], i)) {
							updateParentChild(curr, startLine[i], i, knownElems);
							knownElems[i].clear();
							startLine[i] = tuplen;
						}
					}

					knownElems[i].insert(make_pair(element.m_einf.element, curr->tuples.size()));
					v.push_back(element);
					curr->nameIndex[i][element.m_einf.nelement] = make_pair(element.m_einf.element, element.m_einf.type);
				}
				curr->tuples.push_back(v);
			}
		}
	}
	if (curr) {
		for (size_t i = 0; i < startLine.size(); ++i) {
			updateParentChild(curr, startLine[i], i, knownElems);
		}
	}
}

void View::deleteOld()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::minutes expired(OBSOLETE_OBJECT_MINUTES);

	for (unordered_map<string, string>::iterator it = m_SubsetsCache.begin(); it != m_SubsetsCache.end();) {
		unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_SUBSET> > >::iterator oit = m_Subsets.find(it->second);
		if (oit != m_Subsets.end() && (now - oit->second.first) > expired) {
			m_Subsets.erase(oit);
			it = m_SubsetsCache.erase(it);
		} else {
			++it;
		}
	}

	for (unordered_map<string, string>::iterator it = m_AxisCache.begin(); it != m_AxisCache.end();) {
		unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AXIS> > >::iterator oit = m_Axes.find(it->second);
		if (oit != m_Axes.end() && (now - oit->second.first) > expired) {
			m_Axes.erase(oit);
			it = m_AxisCache.erase(it);
		} else {
			++it;
		}
	}

	for (unordered_map<string, string>::iterator it = m_AreasCache.begin(); it != m_AreasCache.end();) {
		unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator oit = m_Areas.find(it->second);
		if (oit != m_Areas.end() && (now - oit->second.first) > expired) {
			m_Areas.erase(oit);
			it = m_AreasCache.erase(it);
		} else {
			++it;
		}
	}

	for (unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator it = m_Areas.begin(); it != m_Areas.end();) {
		if (it != m_Areas.end() && (now - it->second.first) > expired) {
			it = m_Areas.erase(it);
		} else {
			++it;
		}
	}
}

bool View::hasSameParentSubsets(const vector<ELEMENT_INFO_EXT> &v1, const vector<ELEMENT_INFO_EXT> &v2, size_t count)
{
	bool same = true;
	for (size_t i = 0; i < count; ++i) {
		if (v1[i].m_einf.element != v2[i].m_einf.element) {
			same = false;
			break;
		}
	}
	return same;
}

boost::shared_ptr<VIEW_CALC_AXIS> View::getAxisAndPos(pair<size_t, size_t> &axpos, IdentifierType &indent, const string &database, const string &viewHandle, const string &dim, size_t elemPos)
{
	unordered_map<string, pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > >::iterator it;
	{
		boost::unique_lock<boost::mutex> lock(m_Lock);
		it = m_Areas.find(viewHandle);
		if (it == m_Areas.end()) {
			stringstream str;
			str << "Area not found: " << viewHandle;
			string descr = str.str();
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
		} else {
			it->second.first = boost::posix_time::microsec_clock::universal_time();
		}
	}
	boost::shared_ptr<VIEW_AREA> view = it->second.second;
	boost::shared_ptr<VIEW_CALC> calc = view->calc;
	map<string, size_t>::iterator dimit = calc->dim2axis.find(dim);
	if (dimit == calc->dim2axis.end()) {
		stringstream str;
		str << "Dimension not in view: " << dim;
		string descr = str.str();
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
	}
	axpos = calc->dimonaxis[dimit->second];
	boost::shared_ptr<VIEW_CALC_AXIS> ax = calc->axes[axpos.first];
	indent = view->axes[axpos.first].second->as[axpos.second].sub->indent;
	return ax;
}

}
}
