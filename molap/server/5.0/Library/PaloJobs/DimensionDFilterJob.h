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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef DIMENSIONDFILTERJOB_H_
#define DIMENSIONDFILTERJOB_H_

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {
class Condition;

////////////////////////////////////////////////////////////////////////////////
/// @brief dimension dfilter
////////////////////////////////////////////////////////////////////////////////
class CellArrayAccumulator {
protected:
	double m_accumulation;
	boost::shared_ptr<Condition> m_op;
public:
	CellArrayAccumulator() : m_accumulation(0) {}
	virtual ~CellArrayAccumulator() {}
	virtual void addVal(double d) {};
	virtual void addVal(const string &s) {};
	virtual bool finalize() = 0;
	void setOperator(boost::shared_ptr<Condition> op) {
		m_op = op;
	}
	double getAccumulation() {
		return m_accumulation;
	}

	virtual string getString() {
		return "";
	}

	virtual void reset() {
		m_accumulation = 0;
	}

	static CellArrayAccumulator *create(long flag, boost::shared_ptr<Condition> op);
};

class SERVER_CLASS DimensionDFilterJob : public AreaJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new DimensionDFilterJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	DimensionDFilterJob(PaloJobRequest* jobRequest) : AreaJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute();

private:
	virtual void appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals);

	void top(list<Element*> &subset, vector<CellValue> &vals, int top_num, IdentifiersType &elemIds);
	void upperPer(list<Element*> &subset, vector<CellValue> &vals, double u, IdentifiersType &elemIds);
	void lowerPer(list<Element*> &subset, vector<CellValue> &vals, double l, IdentifiersType &elemIds);
	void middlePer(list<Element*> &subset, vector<CellValue> &vals, double u, double l, IdentifiersType &elemIds);

	size_t pos;

	typedef map<IdentifierType, boost::shared_ptr<CellArrayAccumulator> > Accumulators;

	Accumulators accumulators;
	vector<map<IdentifierType, size_t> > factors;

	boost::shared_ptr<Condition> op;

	template<class T> class PercentageAccumulator {
	private:
		long double m_sum;
		long double m_limit;
		multimap<CellValue, pair<Element*, IdentifierType>, T> m_swaplist;

	public:
		PercentageAccumulator(DimensionDFilterJob *df, multimap<CellValue, pair<Element*, IdentifierType>, T> &sort, double percentage);
		virtual ~PercentageAccumulator() {}
		virtual bool check(typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it);
		multimap<CellValue, pair<Element*, IdentifierType>, T>& get_swaplist();
		void removeif(multimap<CellValue, pair<Element*, IdentifierType>, T> &sort);
	};

	template<class T> class PercentageNegAccumulator : public PercentageAccumulator<T> {
	public:
		PercentageNegAccumulator(DimensionDFilterJob *df, multimap<CellValue, pair<Element*, IdentifierType>, T> &sort, double percentage);
		virtual ~PercentageNegAccumulator() {}
		virtual bool check(typename multimap<CellValue, pair<Element*, IdentifierType>, T>::iterator it);
	};
};

}

#endif /* DIMENSIONDFILTERJOB_H_ */
