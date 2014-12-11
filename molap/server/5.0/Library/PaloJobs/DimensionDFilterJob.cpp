/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloJobs/CellAreaJob.h"
#include "PaloJobs/DimensionDFilterJob.h"
#include "InputOutput/Condition.h"

#undef max

namespace palo {

enum DataFilterFlag {
	DATA_MIN = 0x1, DATA_MAX = 0x2, DATA_SUM = 0x4, DATA_AVERAGE = 0x8, DATA_ANY = 0x10, DATA_ALL = 0x20, DATA_STRING = 0x40, ONLY_CONSOLIDATED = 0x80, ONLY_LEAVES = 0x100, UPPER_PERCENTAGE = 0x200, LOWER_PERCENTAGE = 0x400, MID_PERCENTAGE = 0x800, TOP = 0x1000, NORULES = 0x2000
};

bool queryFlag(long a, long b)
{
	return (a & b) != 0;
}

class SingleCellAccumulator : public CellArrayAccumulator {
public:
	virtual ~SingleCellAccumulator() {}

	virtual void addVal(double d) {
		m_accumulation = d;
	}

	virtual bool finalize() {
		return m_op->check(m_accumulation);
	}
};

class SumAccumulator : public CellArrayAccumulator {
public:
	virtual ~SumAccumulator() {}

	virtual void addVal(double d) {
		m_accumulation += d;
	}

	virtual bool finalize() {
		return m_op->check(m_accumulation);
	}
};

class MinAccumulator : public CellArrayAccumulator {
public:
	MinAccumulator() {
		m_accumulation = std::numeric_limits<double>::max();
	}

	virtual ~MinAccumulator() {}

	virtual void addVal(double d) {
		if (d < m_accumulation) {
			m_accumulation = d;
		}
	}

	virtual bool finalize() {
		return m_op->check(m_accumulation);
	}

	virtual void reset() {
		m_accumulation = std::numeric_limits<double>::max();
	}
};

class MaxAccumulator : public CellArrayAccumulator {
public:
	MaxAccumulator() {
		m_accumulation = -std::numeric_limits<double>::max();
	}

	virtual ~MaxAccumulator() {}

	virtual void addVal(double d) {
		if (d > m_accumulation)
			m_accumulation = d;
	}
	virtual bool finalize() {
		return m_op->check(m_accumulation);
	}

	virtual void reset() {
		m_accumulation = -std::numeric_limits<double>::max();
	}
};

class AverageAccumulator : public CellArrayAccumulator {
public:
	AverageAccumulator() : counter(0) {}

	virtual ~AverageAccumulator() {}

	virtual void addVal(double d) {
		m_accumulation += d;
		counter++;
	}

	virtual bool finalize() {
		m_accumulation /= counter;
		return m_op->check(m_accumulation);
	}

	virtual void reset() {
		this->CellArrayAccumulator::reset();
		counter = 0;
	}

private:
	int counter;
};

class AnyAccumulator : public CellArrayAccumulator {
public:
	AnyAccumulator() : m_found(false) {}

	virtual ~AnyAccumulator() {}

	virtual void addVal(double d) {
		if (m_op->check(d)) {
			m_found = true;
			m_accumulation = d;
		}
	}

	virtual bool finalize() {
		return m_found;
	}

	virtual void reset() {
		this->CellArrayAccumulator::reset();
		m_found = false;
	}

private:
	bool m_found;
};

class AllAccumulator : public CellArrayAccumulator {
public:
	AllAccumulator() : m_all_true(true) {}

	virtual ~AllAccumulator() {}

	virtual void addVal(double d) {
		if (!m_op->check(d)) {
			m_all_true = false;
		} else {
			m_accumulation = d;
		}
	}

	virtual void addVal(const string &s) {
		m_all_true = false;
	}

	virtual bool finalize() {
		return m_all_true;
	}

	virtual void reset() {
		this->CellArrayAccumulator::reset();
		m_all_true = true;
	}

private:
	bool m_all_true;
};

class StringAccumulator : public CellArrayAccumulator {
public:
	StringAccumulator() : m_found(false) {}

	virtual ~StringAccumulator() {}

	virtual void addVal(const string &s) {
		if (m_op->check(s)) {
			m_found = true;
			str = s;
		}
	}

	virtual bool finalize() {
		return m_found;
	}

	virtual void reset() {
		this->CellArrayAccumulator::reset();
		m_found = false;
	}

	virtual string getString() {
		return str;
	}

private:
	bool m_found;
	string str;
};

CellArrayAccumulator *CellArrayAccumulator::create(long flag, boost::shared_ptr<Condition> op)
{
	CellArrayAccumulator *ret = 0;
	if (queryFlag(flag, DATA_SUM)) {
		ret = new SumAccumulator;
	} else if (queryFlag(flag, DATA_MIN)) {
		ret = new MinAccumulator;
	} else if (queryFlag(flag, DATA_MAX)) {
		ret = new MaxAccumulator;
	} else if (queryFlag(flag, DATA_AVERAGE)) {
		ret = new AverageAccumulator;
	} else if (queryFlag(flag, DATA_ANY)) {
		ret = new AnyAccumulator;
	} else if (queryFlag(flag, DATA_ALL)) {
		ret = new AllAccumulator;
	} else if (queryFlag(flag, DATA_STRING)) {
		ret = new StringAccumulator;
	} else {
		ret = new SingleCellAccumulator;
	}
	ret->setOperator(op);
	return ret;
}

void DimensionDFilterJob::compute()
{
	if ((queryFlag(jobRequest->mode, UPPER_PERCENTAGE) && queryFlag(jobRequest->mode, DATA_STRING)) || (queryFlag(jobRequest->mode, LOWER_PERCENTAGE) && queryFlag(jobRequest->mode, DATA_STRING)) || (queryFlag(jobRequest->mode, MID_PERCENTAGE) && queryFlag(jobRequest->mode, DATA_STRING))) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "It is not possible to use percentage parameters with string data", PaloRequestHandler::ID_MODE, "");
	}
	if (!jobRequest->area) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "No area defined", PaloRequestHandler::ID_AREA, "");
	}

	op.reset(Condition::parseCondition(*jobRequest->condition));
	findDimension(false);
	findCube(true, false);
	const IdentifiersType *dims = cube->getDimensions();
	size_t dimCount = jobRequest->area->size();
	if (dims->size() != dimCount) {
		throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of area elements", PaloRequestHandler::ID_AREA, "");
	}
	pos = 0;
	for (IdentifiersType::const_iterator it = dims->begin(); it != dims->end(); ++it, ++pos) {
		if ((*it) == dimension->getId()) {
			break;
		}
	}
	if (pos == dimCount) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "Dimension not in cube.", PaloRequestHandler::ID_DIMENSION, "");
	}
	for (uint32_t i = 0; i < dimCount; ++i) {
		if (i != pos) {
			CPDimension dim = database->lookupDimension((*dims)[i], false);
			if (dim->getDimensionType() == Dimension::VIRTUAL) {
				continue;
			}
			for (uint32_t j = 0; j < jobRequest->area->at(i).size(); ++j) {
				dim->findElement(jobRequest->area->at(i).at(j), 0, false);
			}
		}
	}

	factors.resize(dimCount);
	for (size_t i = 0; i < dimCount; i++) {
		for (IdentifiersType::iterator it = jobRequest->area->at(i).begin(); it != jobRequest->area->at(i).end(); ++it) {
			map<IdentifierType, size_t>::iterator mit = factors[i].find(*it);
			if (mit != factors[i].end()) {
				mit->second++;
			} else {
				factors[i].insert(make_pair(*it, 1));
			}
		}
	}
	for (size_t i = 0; i < dimCount; i++) {
		for (map<IdentifierType, size_t>::iterator mit = factors[i].begin(); mit != factors[i].end();) {
			if (mit->second == 1) {
				factors[i].erase(mit++);
			} else {
				++mit;
			}
		}
	}

	vector<User::RoleCubeRight> rcRights;
	if (User::checkUser(user)) {
		user->getRoleRights(User::cellDataRight, rcRights);
		user->getCubeDataRights(database, cube, rcRights, true);
	}
	bool checkPermissions = cube->getMinimumAccessRight(user) == RIGHT_NONE;
	fillEmptyDim(rcRights, checkPermissions);

	list<Element*> subset;
	if (dimension->getDimensionType() == Dimension::VIRTUAL) {
		if (queryFlag(jobRequest->mode, ONLY_LEAVES)) {
			jobRequest->area->at(pos).clear(); // empty means all elements
		}
	} else {
		for (IdentifiersType::iterator it = jobRequest->area->at(pos).begin(); it != jobRequest->area->at(pos).end(); ++it) {
			subset.push_back(dimension->findElement(*it, user.get(), false));
		}

		if (queryFlag(jobRequest->mode, ONLY_LEAVES) && queryFlag(jobRequest->mode, ONLY_CONSOLIDATED)) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "Bad flag combination.", PaloRequestHandler::ID_MODE, "");	\
		} else if (queryFlag(jobRequest->mode, ONLY_LEAVES)) {
			jobRequest->area->at(pos).clear();
			for (list<Element*>::iterator it = subset.begin(); it != subset.end(); ++it) {
				if (!(*it)->getChildrenCount()) {
					jobRequest->area->at(pos).push_back((*it)->getIdentifier());
				}
			}
		} else if (queryFlag(jobRequest->mode, ONLY_CONSOLIDATED)) {
			jobRequest->area->at(pos).clear();
			for (list<Element*>::iterator it = subset.begin(); it != subset.end(); ++it) {
				if ((*it)->getChildrenCount()) {
					jobRequest->area->at(pos).push_back((*it)->getIdentifier());
				}
			}
		}
	}

	for (IdentifiersType::iterator it = jobRequest->area->at(pos).begin(); it != jobRequest->area->at(pos).end(); ++it) {
		accumulators.insert(pair<IdentifierType, boost::shared_ptr<CellArrayAccumulator> >(*it, boost::shared_ptr<CellArrayAccumulator>(CellArrayAccumulator::create(jobRequest->mode, op))));
	}

	PArea area(new Area(*jobRequest->area));
	PCubeArea calcArea = checkRights(rcRights, checkPermissions, area, 0);

	PCellStream cs;
	if (calcArea->getSize()) {
		cs = cube->calculateArea(calcArea, CubeArea::ALL, queryFlag(jobRequest->mode, NORULES) ? NO_RULES : ALL_RULES, queryFlag(jobRequest->mode, DATA_SUM), 1000000);
	}
	loop(area, calcArea, cs, NULL, false, PCellStream(), true, rcRights, 0);

	IdentifiersType elementIDs;
	vector<CellValue> vals;

	if (subset.empty()) {
		// virtual dimension - iterate over accumulators
		for (Accumulators::iterator acit = accumulators.begin(); acit != accumulators.end(); ++acit) {
			if (acit->second->finalize()) {
				elementIDs.push_back(acit->first);
				CellValue v;
				if (queryFlag(jobRequest->mode, DATA_STRING)) {
					v = acit->second->getString();
				} else {
					v = acit->second->getAccumulation();
				}
				vals.push_back(v);
			}
		}
	} else for (list<Element*>::iterator it = subset.begin(); it != subset.end();) {
		map<IdentifierType, boost::shared_ptr<CellArrayAccumulator> >::iterator acc_it = accumulators.find((*it)->getIdentifier());
		if (acc_it != accumulators.end()) {
			boost::shared_ptr<CellArrayAccumulator> acc = acc_it->second;
			if (!acc->finalize()) {
				it = subset.erase(it);
			} else {
				++it;
				CellValue v;
				if (queryFlag(jobRequest->mode, DATA_STRING)) {
					v = acc->getString();
				} else {
					v = acc->getAccumulation();
				}
				vals.push_back(v);
			}
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "accumulator not found");
		}
	}

	int top_num = 0;
	double u = 0, l = 0;
	if (jobRequest->values) {
		if (jobRequest->values->size()) {
			top_num = StringUtils::stringToInteger(jobRequest->values->at(0));
			if (jobRequest->values->size() > 1) {
				u = StringUtils::stringToDouble(jobRequest->values->at(1));
				if (jobRequest->values->size() > 2) {
					l = StringUtils::stringToDouble(jobRequest->values->at(2));
				}
			}
		}
	}

	if (queryFlag(jobRequest->mode, TOP) && (top_num > 0)) {
		top(subset, vals, top_num, elementIDs);
	} else if (queryFlag(jobRequest->mode, UPPER_PERCENTAGE)) {
		upperPer(subset, vals, u, elementIDs);
	} else if (queryFlag(jobRequest->mode, LOWER_PERCENTAGE)) {
		lowerPer(subset, vals, l, elementIDs);
	} else if (queryFlag(jobRequest->mode, MID_PERCENTAGE)) {
		middlePer(subset, vals, u, l, elementIDs);
	}

	generateElementsValuesResponse(dimension, &subset, elementIDs, &vals);
}

void DimensionDFilterJob::appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals)
{
	if (!value.isError()) {
		Accumulators::iterator accumIt = accumulators.find(key[pos]);
		if (accumIt == accumulators.end()) {
			pair <Accumulators::iterator, bool> insertRes;
			insertRes = accumulators.insert(pair<IdentifierType, boost::shared_ptr<CellArrayAccumulator> >(key[pos], boost::shared_ptr<CellArrayAccumulator>(CellArrayAccumulator::create(jobRequest->mode, op))));
			accumIt = insertRes.first;
		}
		if (value.isNumeric()) {
			size_t factor = 1;
			for (size_t i = 0; i < key.size(); i++) {
				map<IdentifierType, size_t>::iterator mit = factors[i].find(key[i]);
				if (mit != factors[i].end()) {
					factor *= mit->second;
				}
			}
			for (size_t i = 0; i < factor; i++) {
				accumIt->second->addVal(value.getNumeric());
			}
		} else {
			accumIt->second->addVal(value);
		}
	}
}

struct greater {
	bool operator()(const CellValue& lhs, const CellValue& rhs) const {
		return lhs.getNumeric() > rhs.getNumeric();
	}
};

struct less {
	bool operator()(const CellValue& lhs, const CellValue& rhs) const {
		return lhs.getNumeric() < rhs.getNumeric();
	}
};

template<class T> void subs2map(const list<Element*> &subset, const IdentifiersType &elementIds, const vector<CellValue> &vals, multimap<CellValue, pair<Element*, IdentifierType>, T> &sort)
{
	vector<CellValue>::const_iterator vit = vals.begin();
	if (subset.empty()) {
		IdentifiersType::const_iterator eidit;
		for (eidit = elementIds.begin(); eidit != elementIds.end(); ++eidit, ++vit) {
			sort.insert(pair<CellValue, pair<Element*, IdentifierType> >(*vit, make_pair((Element*)0, *eidit)));
		}
	} else {
		list<Element*>::const_iterator eit;
		for (eit = subset.begin(); eit != subset.end(); ++eit, ++vit) {
			sort.insert(pair<CellValue, pair<Element*, IdentifierType> >(*vit, make_pair(*eit, IdentifierType(0))));
		}
	}
}

template<class T> void map2subs(list<Element*> &subset, IdentifiersType &elementIds, vector<CellValue> &vals, const multimap<CellValue, pair<Element*, IdentifierType>, T> &sort, int top_num)
{
	int i = 0;
	for (typename multimap<CellValue, pair<Element*, IdentifierType>, T>::const_iterator it = sort.begin(); i < top_num && it != sort.end(); i++, it++) {
		if (it->second.first) {
			subset.push_back(it->second.first);
		} else {
			elementIds.push_back(it->second.second);
		}
		vals.push_back(it->first);
	}
}

void DimensionDFilterJob::top(list<Element*> &subset, vector<CellValue> &vals, int top_num, IdentifiersType &elemIds)
{
	multimap<CellValue, pair<Element*, IdentifierType>, greater> sort;
	subs2map<greater>(subset, elemIds, vals, sort);
	subset.clear();
	elemIds.clear();
	vals.clear();
	map2subs<greater>(subset, elemIds, vals, sort, top_num);
}

template<class T> DimensionDFilterJob::PercentageAccumulator<T>::PercentageAccumulator(DimensionDFilterJob *df, multimap<CellValue, pair<Element*, IdentifierType>, T> &sort, double percentage) :
	m_sum(0), m_limit(0)
{
	double total = 0;

	for (typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it_beg = sort.begin(); it_beg != sort.end();) {
		if (queryFlag(df->jobRequest->mode, ONLY_LEAVES)) {
			if (it_beg->second.first && it_beg->second.first->getChildrenCount()) {
				m_swaplist.insert(*it_beg);
				sort.erase(it_beg++);
			} else {
				total += (it_beg->first.getNumeric() * (((double)percentage) / 100.0));
				++it_beg;

			}
		} else if (queryFlag(df->jobRequest->mode, ONLY_CONSOLIDATED)) {
			if (it_beg->second.first && !it_beg->second.first->getChildrenCount()) {
				m_swaplist.insert(*it_beg);
				sort.erase(it_beg++);
			} else {
				total += (it_beg->first.getNumeric() * (((double)percentage) / 100.0));
				++it_beg;
			}
		} else {
			total += (it_beg->first.getNumeric() * (((double)percentage) / 100.0));
			++it_beg;
		}
	}
	m_limit = total;
}

template<class T> bool DimensionDFilterJob::PercentageAccumulator<T>::check(typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it)
{
	if (m_sum < m_limit) {
		if (it->first.getNumeric() < ((m_limit - m_sum) * 2.0)) {
			m_sum += it->first.getNumeric();
			return false;
		} else {
			m_sum = m_limit;
			return true;
		}
	} else {
		return true;
	}
}

template<class T> multimap<CellValue, pair<Element*, IdentifierType>, T>& DimensionDFilterJob::PercentageAccumulator<T>::get_swaplist()
{
	return m_swaplist;
}

template<class T> DimensionDFilterJob::PercentageNegAccumulator<T>::PercentageNegAccumulator(DimensionDFilterJob *df, multimap<CellValue, pair<Element*, IdentifierType>, T> &sort, double percentage) :
	PercentageAccumulator<T>(df, sort, percentage)
{
}

template<class T> bool DimensionDFilterJob::PercentageNegAccumulator<T>::check(typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it)
{
	return !PercentageAccumulator<T>::check(it);
}

template<class T> void DimensionDFilterJob::PercentageAccumulator<T>::removeif(multimap<CellValue, pair<Element*, IdentifierType>, T> &sort)
{
	for (typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it = sort.begin(); it != sort.end();) {
		if (check(it)) {
			sort.erase(it++);
		} else {
			++it;
		}
	}
}

void DimensionDFilterJob::upperPer(list<Element*> &subset, vector<CellValue> &vals, double u, IdentifiersType &elemIds)
{
	multimap<CellValue, pair<Element*, IdentifierType>, greater> sort;
	subs2map<greater>(subset, elemIds, vals, sort);

	PercentageAccumulator<greater> p(this, sort, u);
	p.removeif(sort);
	subset.clear();
	elemIds.clear();
	vals.clear();
	map2subs<greater>(subset, elemIds, vals, sort, numeric_limits<int>::max());
	map2subs<greater>(subset, elemIds, vals, p.get_swaplist(), numeric_limits<int>::max());
}

void DimensionDFilterJob::lowerPer(list<Element*> &subset, vector<CellValue> &vals, double l, IdentifiersType &elemIds)
{
	multimap<CellValue, pair<Element*, IdentifierType>, less> sort;
	subs2map<less>(subset, elemIds, vals, sort);

	PercentageAccumulator<less> p(this, sort, l);
	p.removeif(sort);
	subset.clear();
	elemIds.clear();
	vals.clear();
	map2subs<less>(subset, elemIds, vals, sort, numeric_limits<int>::max());
	map2subs<less>(subset, elemIds, vals, p.get_swaplist(), numeric_limits<int>::max());
}

void DimensionDFilterJob::middlePer(list<Element*> &subset, vector<CellValue> &vals, double u, double l, IdentifiersType &elemIds)
{
	multimap<CellValue, pair<Element*, IdentifierType>, greater> sort1;
	subs2map<greater>(subset, elemIds, vals, sort1);
	PercentageNegAccumulator<greater> p1(this, sort1, u);
	p1.removeif(sort1);

	multimap<CellValue, pair<Element*, IdentifierType>, less> sort2;
	subs2map<less>(subset, elemIds, vals, sort2);
	PercentageNegAccumulator<less> p2(this, sort2, l);
	p2.removeif(sort2);

	map<pair<Element*, IdentifierType>, pair<int, CellValue> > tmp;
	map<pair<Element*, IdentifierType>, pair<int, CellValue> > res;
	for (multimap<CellValue, pair<Element*, IdentifierType>, greater>::iterator it = sort1.begin(); it != sort1.end(); ++it) {
		map<pair<Element*, IdentifierType>, pair<int, CellValue> >::iterator iti = tmp.find(it->second);
		if (iti == tmp.end()) {
			tmp.insert(pair<pair<Element*, IdentifierType>, pair<int, CellValue> >(it->second, pair<int, CellValue>(1, it->first)));
		} else {
			iti->second.first++;
		}
	}
	for (multimap<CellValue, pair<Element*, IdentifierType>, less>::iterator it = sort2.begin(); it != sort2.end(); ++it) {
		map<pair<Element*, IdentifierType>, pair<int, CellValue> >::iterator iti = res.find(it->second);
		if (iti == res.end()) {
			res.insert(pair<pair<Element*, IdentifierType>, pair<int, CellValue> >(it->second, pair<int, CellValue>(1, it->first)));
		} else {
			iti->second.first++;
		}
	}

	subset.clear();
	elemIds.clear();
	vals.clear();
	for (map<pair<Element*, IdentifierType>, pair<int, CellValue> >::iterator it = res.begin(); it != res.end(); ++it) {
		map<pair<Element*, IdentifierType>, pair<int, CellValue> >::iterator iti = tmp.find(it->first);
		if (iti != tmp.end()) {
			int c = iti->second.first > it->second.first ? it->second.first : iti->second.first;
			for (int i = 0; i < c; i++) {
				if (it->first.first) {
					subset.push_back(it->first.first);
				} else {
					elemIds.push_back(it->first.second);
				}
				vals.push_back(it->second.second);
			}
		}
	}

	map2subs<greater>(subset, elemIds, vals, p1.get_swaplist(), numeric_limits<int>::max());
	map2subs<less>(subset, elemIds, vals, p2.get_swaplist(), numeric_limits<int>::max());
}

}
