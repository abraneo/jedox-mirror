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

#ifndef __DATAFILTER_H_INCL__
#define __DATAFILTER_H_INCL__

#include <string>
#include <list>
#include <utility>
#include <boost/scoped_ptr.hpp>
#include <map>

#include "Filter.h"
#include "SubSet.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {

class DataFilter : public DataFilterBase, public Filter {
public:
	DataFilter(SubSet& s, DataFilterSettings &settings);
	ElementsType apply();
private:
	typedef vector<IdentifiersType> coords_vec_type;
	double m_percentage1, m_percentage2;
	unsigned int m_top_num;
	coords_vec_type m_coords;
	PCube m_source_cube;
	PCondition op;
	size_t pos;
	vector<map<IdentifierType, size_t> > factors;
	DataFilterSettings &m_settings;

	void applySettings();
	int getFuncType(bool &isAggrFunc) const;

	template<class T> class PercentageAccumulator {
	protected:
		long double m_sum;
		long double m_limit;
		ElementsType m_others;
		multimap<double, Element *, T> sort;

	public:
		PercentageAccumulator(DataFilter &df, const ElementsType &ids, double percentage);
		virtual ~PercentageAccumulator() {}
		virtual bool check(double value);
		virtual multiset<Element *> apply();
	};

	template<class T>class PercentageNegAccumulator : public PercentageAccumulator<T> {
	public:
		PercentageNegAccumulator(DataFilter &df, const ElementsType &ids, double percentage);
		virtual ~PercentageNegAccumulator() {}
		virtual bool check(double value);
	};

	ElementsType top(int top_num, const ElementsType &subset);
	ElementsType upperPer(double u, const ElementsType &subset);
	ElementsType lowerPer(double l, const ElementsType &subset);
	ElementsType middlePer(double u, double l, const ElementsType &subset);
};

} //palo
#endif
