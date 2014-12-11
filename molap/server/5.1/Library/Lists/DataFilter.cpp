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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Susanne Eichel, Albert-Ludwigs-Universitaet Freiburg, Germany
 * 
 *
 */

#include <string>
#include <vector>
#include <functional>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/limits.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "DataFilter.h"
#include "SubSet.h"
#include "PickList.h"
#include "Filter.h"

#include "PaloHttpServer/PaloRequestHandler.h"
#include "InputOutput/Condition.h"
#include "PaloJobs/AreaJob.h"

namespace palo {

static string CSVencode(const string& val, const char delimeter = '\"')
{
	size_t vsize = val.size();
	std::stringstream idstr;

	idstr << delimeter;

	for (size_t i = 0; i < vsize; i++) {
		idstr << val[i];
		if (val[i] == delimeter) {
			idstr << delimeter;
		}
	}

	idstr << delimeter;

	return idstr.str();

}

DataFilter::DataFilter(SubSet& s, DataFilterSettings &settings) :
	Filter(s, settings.flags, DATA_FILTER_NUMFLAGS), m_percentage1(0), m_percentage2(0), m_top_num(0), op(Condition::parseCondition("")), pos(0), m_settings(settings)
{
	if (queryFlag(ONLY_LEAVES) && queryFlag(ONLY_CONSOLIDATED)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Wrong number of dimensions passed to set_coordinates");
	} else if (queryFlag(ONLY_LEAVES)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_ONLY_LEAVES);
	} else if (queryFlag(ONLY_CONSOLIDATED)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_ONLY_CONSOLIDATED);
	}
	m_subset_ref.setGlobalFlag(SubSet::DATA_FILTER_ACTIVE);
	if (queryFlag(UPPER_PERCENTAGE) || queryFlag(LOWER_PERCENTAGE) || queryFlag(MID_PERCENTAGE) || queryFlag(TOP)) {
		m_subset_ref.setGlobalFlag(SubSet::DONT_SHOW_DUPLICATES);
	}
	if (queryFlag(DATA_STRING)) {
		m_subset_ref.setGlobalFlag(SubSet::DATA_STRING);
	}
}

ElementsType DataFilter::apply()
{
	applySettings();
	if ((queryFlag(UPPER_PERCENTAGE) && queryFlag(DATA_STRING)) || (queryFlag(LOWER_PERCENTAGE) && queryFlag(DATA_STRING)) || (queryFlag(MID_PERCENTAGE) && queryFlag(DATA_STRING))) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "It is not possible to use percentage parameters with string data", PaloRequestHandler::ID_MODE, "");
	}

	const IdentifiersType *dims = m_source_cube->getDimensions();
	size_t dimCount = m_coords.size();
	if (dims->size() != dimCount) {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
	}
	pos = 0;
	PDimension dimension = m_subset_ref.getDimension();
	PDatabase database = m_subset_ref.getDatabase();
	for (IdentifiersType::const_iterator it = dims->begin(); it != dims->end(); ++it, ++pos) {
		if ((*it) == dimension->getId()) {
			break;
		}
	}
	if (pos == dimCount) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "Dimension not in cube.", PaloRequestHandler::ID_DIMENSION, "");
	}

	bool isAggrFunc;
	int funcType = getFuncType(isAggrFunc);
	bool isVirtual = false;
	for (uint32_t i = 0; i < dimCount; ++i) {
		CPDimension dim = database->lookupDimension((*dims)[i], false);
		if (dim->getDimensionType() == Dimension::VIRTUAL) {
			isVirtual = true;
			break;
		}
	}
	if (isVirtual && isAggrFunc) {
		throw ParameterException(ErrorException::ERROR_INVALID_MODE, "aggregation on a virtual cube is not allowed", PaloRequestHandler::ID_MODE, (unsigned int)filter_flags);
	}

	bool clearFactors = true;
	if (queryFlag(DATA_SUM) || queryFlag(DATA_AVERAGE)) {
		factors.resize(dimCount);
		for (size_t i = 0; i < dimCount; i++) {
			if (i != pos) {
				for (IdentifiersType::iterator it = m_coords[i].begin(); it != m_coords[i].end(); ++it) {
					map<IdentifierType, size_t>::iterator mit = factors[i].find(*it);
					if (mit != factors[i].end()) {
						mit->second++;
						clearFactors = false;
					} else {
						factors[i].insert(make_pair(*it, 1));
					}
				}
			}
		}
		if (clearFactors) {
			factors.clear();
		} else {
			for (size_t i = 0; i < dimCount; i++) {
				for (map<IdentifierType, size_t>::iterator mit = factors[i].begin(); mit != factors[i].end();) {
					if (mit->second == 1) {
						factors[i].erase(mit++);
					} else {
						++mit;
					}
				}
			}
		}
	}

	m_coords[pos].reserve(m_subset_ref.size());
	for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
		m_coords[pos].push_back(it.getId());
	}

	PUser user = m_subset_ref.getUser();
	vector<User::RoleDbCubeRight> vRights;
	if (User::checkUser(user)) {
		user->fillRights(vRights, User::cellDataRight, database, m_source_cube);
	}
	bool checkPermissions = m_source_cube->getMinimumAccessRight(user) == RIGHT_NONE;
	AreaJob::fillEmptyDim(vRights, checkPermissions, m_coords, m_source_cube, database, user);

	if (dimension->getDimensionType() == Dimension::VIRTUAL) {
		if (queryFlag(ONLY_LEAVES)) {
			m_coords.at(pos).clear();
		}
	} else {
		if (queryFlag(ONLY_LEAVES) && queryFlag(ONLY_CONSOLIDATED)) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "Bad flag combination.", PaloRequestHandler::ID_MODE, "");
		} else if (queryFlag(ONLY_LEAVES)) {
			m_coords[pos].clear();
			for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
				if (!it.getChildrenCount()) {
					m_coords[pos].push_back(it.getId());
				}
			}
		} else if (queryFlag(ONLY_CONSOLIDATED)) {
			m_coords[pos].clear();
			for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
				if (it.getChildrenCount()) {
					m_coords[pos].push_back(it.getId());
				}
			}
		}
	}

	PArea area(new Area(m_coords));
	PArea noPermission;
	bool isNoPermission;
	vector<CPDimension> cdims;
	PCubeArea calcArea = AreaJob::checkRights(vRights, checkPermissions, area, 0, m_source_cube, database, user, true, noPermission, isNoPermission, cdims);
	if (!calcArea->getSize()) {
		return ElementsType();
	}

	// create plan
	bool calcRules = !queryFlag(NORULES) && m_source_cube->hasActiveRule();
	RulesType rulesType = calcRules ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES;
	PPlanNode plan = m_source_cube->createPlan(calcArea, CubeArea::ALL, rulesType, true, UNLIMITED_SORTED_PLAN);
	if (!plan) {
		return ElementsType();
	}

	vector<PPlanNode> children;
	if (plan->getType() == UNION) {
		children = plan->getChildren();
	} else {
		children.push_back(plan);
	}

	double cellsPerElement = 1;
	if (!isVirtual) {
		for (size_t i = 0; i < dimCount; i++) {
			if (i != pos) {
				cellsPerElement *= calcArea->getDim(i)->size();
			}
		}
	}

	PCubeArea numericArea;
	if (!isVirtual && !isAggrFunc && (QuantificationPlanNode::QuantificationType)funcType != QuantificationPlanNode::EXISTENCE) {
		// build numericArea for ALL, ANY_NUM and ANY_STR
		numericArea.reset(new CubeArea(database, m_source_cube, dimCount));
		for (uint32_t i = 0; i < dimCount; ++i) {
			CPDimension dim = database->lookupDimension((*dims)[i], false);
			PSet numSet(new Set);
			CPSet calcSet = calcArea->getDim(i);
			for (Set::Iterator it = calcSet->begin(); it != calcSet->end(); ++it) {
				Element *elem = dim->findElement(*it, 0, false);
				if (elem->getElementType() == Element::NUMERIC || elem->getElementType() == Element::CONSOLIDATED) {
					numSet->insert(*it);
				}
			}
			numericArea->insert(i, numSet);
		}
	}

	if (isAggrFunc) {
		// create targArea
		PArea targArea(new CubeArea(database, m_source_cube, dimCount));
		for (size_t i = 0; i < dimCount; i++) {
			CPSet srcSet = calcArea->getDim(i);
			if (i == pos || srcSet->size() == 1) {
				targArea->insert(i, srcSet);
			} else {
				PSet s(new Set);
				s->insert(*srcSet->begin());
				targArea->insert(i, s);
			}
		}
		// build aggregationMaps
		PAggregationMaps aggregationMaps(new AggregationMaps());
		AggregationMaps &maps = *aggregationMaps.get();
		maps.resize(dimCount);
		for (size_t i = 0; i < dimCount; i++) {
			CPSet s = calcArea->getDim(i);
			if (i == pos) {
				maps[i].buildBaseToParentMap_OneByOne(s);
			} else {
				maps[i].buildBaseToParentMap_AllToOne(s, clearFactors ? 0 : &factors[i]);
			}
#ifdef ENABLE_GPU_SERVER
			maps[i].toGpuAggregationMap(calcArea->getPathTranslator(), (uint32_t)i);
#endif
		}
		AggregationPlanNode::AggregationType type = (AggregationPlanNode::AggregationType)funcType;
		plan.reset(new AggregationPlanNode(targArea, children, aggregationMaps, 0, CPCube(), type, 0, op, (int)pos, cellsPerElement));
	} else {
		QuantificationPlanNode::QuantificationType type = (QuantificationPlanNode::QuantificationType)funcType;
		plan.reset(new QuantificationPlanNode(calcArea, numericArea, children, type, op, (int)pos, isVirtual, cellsPerElement, calcRules, 0));
	}

//	Logger::debug << "DataFilter Plan: " << plan->toXML() << endl;

	PCellStream cs = m_source_cube->evaluatePlan(plan, EngineBase::ANY, true);
	set<IdentifierType> valid;
	if (cs) {
		while (cs->next()) {
			IdentifierType id = cs->getKey()[pos];
			valid.insert(id);
			m_subset_ref.setValue(id, cs->getValue());
		}
	}

	ElementsType ret;
	if (dimension->getDimensionType() == Dimension::VIRTUAL) {
		for (set<IdentifierType>::iterator it = valid.begin(); it != valid.end(); ++it) {
			ret.push_back((Element *)(size_t)*it);
		}
	} else {
		for (SubSet::Iterator it = m_subset_ref.begin(true); !it.end(); ++it) {
			if (valid.find(it.getId()) != valid.end()) {
				ret.push_back(it.getElement());
			}
		}
	}
	if (queryFlag(TOP) && (m_top_num > 0)) {
		ret = top(m_top_num, ret);
	} else if (queryFlag(UPPER_PERCENTAGE)) {
		ret = upperPer(m_percentage1, ret);
	} else if (queryFlag(LOWER_PERCENTAGE)) {
		ret = lowerPer(m_percentage2, ret);
	} else if (queryFlag(MID_PERCENTAGE)) {
		ret = middlePer(m_percentage1, m_percentage2, ret);
	}
	return ret;
}

void DataFilter::applySettings()
{
	m_source_cube = m_subset_ref.getDatabase()->findCubeByName(m_settings.cube, m_subset_ref.getUser(), true, false);
	if (m_settings.cmp.use_strings) {
		if (!m_settings.cmp.op1.empty() || m_settings.cmp.force) {
			string str_par1, str_par2;
			str_par1 = m_settings.cmp.force ? string() : CSVencode(m_settings.cmp.par1s);
			if (!m_settings.cmp.op2.empty()) {
				str_par2 = CSVencode(m_settings.cmp.par2s);
				op.reset(Condition::parseCondition(m_settings.cmp.op1 + str_par1 + "and" + m_settings.cmp.op2 + str_par2));
			} else {
				op.reset(Condition::parseCondition(m_settings.cmp.op1 + str_par1));
			}
		}
	} else {
		if (!m_settings.cmp.op1.empty()) {
			string str_par1, str_par2;
			str_par1 = StringUtils::convertToString(m_settings.cmp.par1d);
			if (!m_settings.cmp.op2.empty()) {
				str_par2 = StringUtils::convertToString(m_settings.cmp.par2d);
				op.reset(Condition::parseCondition(m_settings.cmp.op1 + str_par1 + "and" + m_settings.cmp.op2 + str_par2));
			} else {
				op.reset(Condition::parseCondition(m_settings.cmp.op1 + str_par1));
			}
		}
	}
	m_coords.reserve(m_settings.coords.size());
	if (m_source_cube->getDimensions()->size() != m_settings.coords.size() && m_source_cube->getDimensions()->size() != m_settings.coords.size() + 1)
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Number of dimensions do not match (passed <> cube).");

	for (DataFilterSettings::CoordsType::const_iterator i = m_settings.coords.begin(); i != m_settings.coords.end(); ++i) {
			PDimension tempdim = m_subset_ref.getDatabase()->findDimension(m_source_cube->getDimensions()->at(i - m_settings.coords.begin()), m_subset_ref.getUser(), false);

			IdentifiersType sa;
			sa.reserve(i->size());

			for (vector<string>::const_iterator it = i->begin(); it != i->end(); ++it) {
				if (tempdim->getDimensionType() == Dimension::VIRTUAL) {
					sa.push_back(StringUtils::stringToUnsignedInteger(*it));
				} else {
					Element *el = tempdim->findElementByName(*it, m_subset_ref.getUser().get(), false);
					sa.push_back(el->getIdentifier());
				}
			}
			m_coords.push_back(sa);
	}
	if (m_settings.upper_percentage_set || m_settings.lower_percentage_set) {
		if (!queryFlag(UPPER_PERCENTAGE | LOWER_PERCENTAGE | MID_PERCENTAGE)) {
			if (m_settings.upper_percentage != 0 && m_settings.lower_percentage != 0) {
				setFlag(MID_PERCENTAGE);
			} else if (m_settings.upper_percentage != 0) {
				setFlag(UPPER_PERCENTAGE);
			} else if (m_settings.lower_percentage != 0) {
				setFlag(LOWER_PERCENTAGE);
			}
		}

		if (m_settings.upper_percentage < 0 || m_settings.upper_percentage > 100) {
			m_settings.upper_percentage = 0;
			resetFlag(MID_PERCENTAGE | UPPER_PERCENTAGE);
		}
		if (m_settings.lower_percentage < 0 || m_settings.lower_percentage > 100) {
			m_settings.lower_percentage = 0;
			resetFlag(MID_PERCENTAGE | LOWER_PERCENTAGE);
		}

		m_percentage1 = m_settings.upper_percentage;
		m_percentage2 = m_settings.lower_percentage;
	}

	if (m_settings.top >= 0) {
		if (!queryFlag(TOP))
			setFlag(TOP);
		m_top_num = m_settings.top;
	}
}

int DataFilter::getFuncType(bool &isAggrFunc) const
{
	int result = 0;
	isAggrFunc = true;
	if (queryFlag(DATA_MIN)) {
		result = AggregationPlanNode::MIN;
	} else if (queryFlag(DATA_MAX)) {
		result = AggregationPlanNode::MAX;
	} else if (queryFlag(DATA_SUM)) {
		result = AggregationPlanNode::SUM;
	} else if (queryFlag(DATA_AVERAGE)) {
		result = AggregationPlanNode::AVG;
	} else {
		isAggrFunc = false;
		if (queryFlag(DATA_ANY)) {
			result = QuantificationPlanNode::ANY_NUM;
		} else if (queryFlag(DATA_ALL)) {
			result = QuantificationPlanNode::ALL;
		} else if (queryFlag(DATA_STRING)) {
			result = QuantificationPlanNode::ANY_STR;
		}
	}
	return result;
}

struct greater {
	bool operator()(double x, double y) const {return y < x;}
};

struct less {
	bool operator()(double x, double y) const {return x < y;}
};

ElementsType DataFilter::top(int top_num, const ElementsType &subset)
{
	multimap<double, Element *, greater> sort;
	for (ElementsType::const_iterator it = subset.begin(); it != subset.end(); ++it) {
		if (m_subset_ref.getDimension()->getDimensionType() == Dimension::VIRTUAL) {
			sort.insert(make_pair(m_subset_ref.getValue((IdentifierType)(size_t)*it).getNumeric(), *it));
		} else {
			sort.insert(make_pair(m_subset_ref.getValue((*it)->getIdentifier()).getNumeric(), *it));
		}
	}
	multimap<double, Element *, greater>::iterator it = sort.begin();
	ElementsType ret;
	for (int i = 0; it != sort.end() && i < top_num; ++i, ++it) {
		ret.push_back(it->second);
	}
	return ret;
}

template<class T> DataFilter::PercentageAccumulator<T>::PercentageAccumulator(DataFilter &df, const ElementsType &ids, double percentage) :
	m_sum(0), m_limit(0)
{
	double total = 0;

	for (ElementsType::const_iterator it_beg = ids.begin(); it_beg != ids.end(); ++it_beg) {
		IdentifierType children = (IdentifierType)(*it_beg)->getChildrenCount();
		if ((df.queryFlag(DataFilterBase::ONLY_LEAVES) && !children) || (df.queryFlag(DataFilterBase::ONLY_CONSOLIDATED) && children)) {
			m_others.push_back(*it_beg);
		} else {
			double val;
			if (df.m_subset_ref.getDimension()->getDimensionType() == Dimension::VIRTUAL) {
				val = df.m_subset_ref.getValue((IdentifierType)(size_t)*it_beg).getNumeric();
			} else {
				val = df.m_subset_ref.getValue((*it_beg)->getIdentifier()).getNumeric();
			}
			sort.insert(make_pair(val, *it_beg));
			total += (val * (((double)percentage) / 100.0));
		}
	}
	m_limit = total;
}

template<class T> bool DataFilter::PercentageAccumulator<T>::check(double value)
{
	if (m_sum < m_limit) {
		if (value < ((m_limit - m_sum) * 2.0)) {
			m_sum += value;
			return false;
		} else {
			m_sum = m_limit;
			return true;
		}
	}
	return true;
}

template<class T> multiset<Element *> DataFilter::PercentageAccumulator<T>::apply()
{
	multiset<Element *> ret;
	for (typename multimap<double, Element *, T>::iterator it = sort.begin(); it != sort.end(); ++it) {
		if (!check(it->first)) {
			ret.insert(it->second);
		}
	}
	ret.insert(m_others.begin(), m_others.end());
	return ret;
}


template<class T> DataFilter::PercentageNegAccumulator<T>::PercentageNegAccumulator(DataFilter &df, const ElementsType &ids, double percentage) :
	PercentageAccumulator<T>(df, ids, percentage)
{
}

template<class T> bool DataFilter::PercentageNegAccumulator<T>::check(double value)
{
	return !PercentageAccumulator<T>::check(value);
}

ElementsType DataFilter::upperPer(double u, const ElementsType &subset)
{
	PercentageAccumulator<greater> p(*this, subset, u);
	multiset<Element *> filtered = p.apply();
	ElementsType ret;
	ret.insert(ret.begin(), filtered.begin(), filtered.end());
	return ret;
}

ElementsType DataFilter::lowerPer(double l, const ElementsType &subset)
{
	PercentageAccumulator<less> p(*this, subset, l);
	multiset<Element *> filtered = p.apply();
	ElementsType ret;
	ret.insert(ret.begin(), filtered.begin(), filtered.end());
	return ret;
}

ElementsType DataFilter::middlePer(double u, double l, const ElementsType &subset)
{
	PercentageNegAccumulator<greater> p1(*this, subset, u);
	multiset<Element *> set1 = p1.apply();
	PercentageNegAccumulator<less> p2(*this, subset, l);
	multiset<Element *> set2 = p2.apply();
	ElementsType ret(max(set1.size(), set2.size()));
	ElementsType::iterator it = set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), ret.begin());
	ret.resize(it - ret.begin());
	return ret;
}

} //palo
