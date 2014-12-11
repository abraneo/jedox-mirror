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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloJobs/CellAreaJob.h"
#include "PaloJobs/DimensionDFilterJob.h"
#include "PaloJobs/ViewCalculateJob.h"
#include "InputOutput/Condition.h"
#include "Engine/DFilterProcessor.h"
#include "Lists/Filter.h"
#include "Lists/SubSet.h"

#undef max

namespace palo {

void DimensionDFilterJob::compute()
{
	if (!jobRequest->area) {
		throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "No area defined", PaloRequestHandler::ID_AREA, "");
	}

	findDimension(false);
	findCube(true, false);
	PSubSet sub;
	ViewSubset subdef;
	subdef.data.resize(1);
	subdef.basic.resize(1);

	subdef.data[0].active = true;
	subdef.data[0].flags = jobRequest->mode;
	subdef.data[0].cube = cube->getName();
	const IdentifiersType *dims = cube->getDimensions();
	subdef.data[0].coords.resize(dims->size());

	subdef.basic[0].active = true;
	subdef.basic[0].flags = PickListBase::DFILTER;

	subdef.sorting.active = true;
	subdef.sorting.flags = SortingFilterBase::NOSORT;

	for (size_t i = 0; i < dims->size(); ++i) {
		CPDimension dim = database->lookupDimension(dims->at(i), false);
		if (dim->getId() == dimension->getId()) {
			if (jobRequest->area->at(i).empty()) {
				subdef.basic[0].active = false;
			} else {
				subdef.basic[0].manual_subset.resize(jobRequest->area->at(i).size());
				IdentifiersType::iterator it = jobRequest->area->at(i).begin();
				size_t j = 0;
				for (; it != jobRequest->area->at(i).end(); ++it, ++j) {
					if (dim->getDimensionType() == Dimension::VIRTUAL) {
						subdef.basic[0].manual_subset[j].push_back(StringUtils::convertToString(*it));
					} else {
						subdef.basic[0].manual_subset[j].push_back(dim->findElement(*it, user.get(), false)->getName(dim->getElemNamesVector()));
					}
				}
			}
		} else {
			subdef.data[0].coords[i].reserve(jobRequest->area->at(i).size());
			for (IdentifiersType::iterator it = jobRequest->area->at(i).begin(); it != jobRequest->area->at(i).end(); ++it) {
				if (dim->getDimensionType() == Dimension::VIRTUAL) {
					subdef.data[0].coords[i].push_back(StringUtils::convertToString(*it));
				} else {
					subdef.data[0].coords[i].push_back(dim->findElement(*it, user.get(), false)->getName(dim->getElemNamesVector()));
				}
			}
		}
	}
	if (jobRequest->condition && !jobRequest->condition->empty()) {
		subdef.data[0].cmp.use_strings = true;
		subdef.data[0].cmp.force = true;
		subdef.data[0].cmp.op1 = *jobRequest->condition;
	}
	subdef.data[0].top = -1;
	subdef.data[0].lower_percentage_set = false;
	subdef.data[0].upper_percentage_set = false;
	if (jobRequest->values) {
		if (jobRequest->values->size()) {
			if (jobRequest->values->at(0) != "0") {
				subdef.data[0].top = StringUtils::stringToInteger(jobRequest->values->at(0));
			}
			if (jobRequest->values->size() > 1) {
				if (jobRequest->values->at(1) != "0") {
					subdef.data[0].upper_percentage_set = true;
					subdef.data[0].upper_percentage = StringUtils::stringToDouble(jobRequest->values->at(1));
				}
				if (jobRequest->values->size() > 2) {
					if (jobRequest->values->at(2) != "0") {
						subdef.data[0].lower_percentage_set = true;
						subdef.data[0].lower_percentage = StringUtils::stringToDouble(jobRequest->values->at(2));
					}
				}
			}
		}
	}
	sub = ViewCalculateJob::subset(database, user, dimension, subdef);

	response = new HttpResponse(HttpResponse::OK);
	StringBuffer& body = response->getBody();

	setToken(dimension);

	vector<CellValue> prop_vals;
	for (SubSet::Iterator it = sub->begin(true); !it.end(); ++it) {
		vector<User::RoleDbCubeRight> vRights;
		if (dimension->getDimensionType() == Dimension::VIRTUAL) {
			appendElement(&body, dimension, 0, it.getId(), false, vRights);
		} else {
			appendElement(&body, dimension, it.getElement(), 0, false, vRights);
		}
		appendCell(body, it.getValue(), false, false, false, prop_vals);
	}
}

}
