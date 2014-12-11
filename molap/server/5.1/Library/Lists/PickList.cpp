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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include <string>

#include <boost/bind.hpp>

#include "PickList.h"
#include "SubSet.h"

namespace palo {

PickList::PickList(SubSet& s, BasicFilterSettings &settings) :
	Filter(s, settings.flags, PICKLIST_NUM_FLAGS), m_settings(settings)
{
}

ElementsType PickList::apply()
{
	if (!queryFlag(INSERT_BACK) && !queryFlag(INSERT_FRONT) && !queryFlag(MERGE) && !queryFlag(SUB) && !queryFlag(DFILTER))
		setFlag(INSERT_FRONT);

	ElementsType ret;
	if (!m_settings.manual_subset.empty()) {
		buildElemlist(ret);
		if (queryFlag(INSERT_BACK)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_BACK);
		} else if (queryFlag(MERGE)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_MERGE);
		} else if (queryFlag(SUB)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_SUB);
		} else if (queryFlag(DFILTER)) {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_DFILTER);
		} else {
			m_subset_ref.setGlobalFlag(SubSet::PICKLIST_FRONT);
		}
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Application of Picklist without a list of elements.");
	}
	return ret;
}

void PickList::buildElemlist(ElementsType &result)
{
	PDimension dim = m_subset_ref.getDimension();
	PUser user = m_subset_ref.getUser();
	for (vector<vector<string> >::iterator it = m_settings.manual_subset.begin(); it != m_settings.manual_subset.end(); ++it) {
		//TODOMJ paths
		if (dim->getDimensionType() == Dimension::VIRTUAL) {
			result.push_back((Element *)(size_t)StringUtils::stringToUnsignedInteger((*it)[0]));
		} else {
			Element *el = dim->findElementByName((*it)[0], user.get(), false);
			result.push_back(el);
		}
	}
}

} //palo
