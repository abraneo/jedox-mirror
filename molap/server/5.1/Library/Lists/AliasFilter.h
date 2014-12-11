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

#ifndef __ALIASFILTER_H_INCL__
#define __ALIASFILTER_H_INCL__

#include <string>
#include <list>
#include <vector>

#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "Filter.h"
#include "SubSet.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {

class AliasFilter : public AliasFilterBase, public Filter {
private:
	struct FilterOperator;
	struct AliasFilterHelper;

	enum Optype {
		NO_OP = 0, OP_GT, OP_LESS, OP_GEQ, OP_LEQ, OP_EQU, OP_NE
	};

	typedef boost::tuple<Optype, double, boost::regex> filterexpression_type;
	typedef vector<filterexpression_type> filterexpression_vec_type;
	typedef multimap<IdentifierType, filterexpression_vec_type> filterexpression_map_vec_type;

public:
	AliasFilter(SubSet &sub, AliasFilterSettings &settings, FieldFilterSettings &fs);

	ElementsType apply(bool &worked);

private:
	void applySettings();
	set<int> check(const CellValue &val, IdentifierType attrId);
	AliasFilterSettings &m_settings;
	FieldFilterSettings &m_fs;
	IdentifierType m_attr1_coord;
	IdentifierType m_attr2_coord;
	IdentifiersType m_filter_coords;
	filterexpression_map_vec_type m_filterexp;
};

} //palo
#endif
