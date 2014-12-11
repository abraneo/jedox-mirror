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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_EXPORT_JOB_H
#define PALO_JOBS_CELL_EXPORT_JOB_H 1

#include "palo.h"

#include "InputOutput/Condition.h"
#include "Olap/Cube.h"
#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloJobs/AreaJob.h"
#include "Engine/EngineBase.h"

#undef max

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell export
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellExportJob : public AreaJob {
private:
	static const uint32_t progressMax = 1000;
	boost::scoped_ptr<Condition> condition;
	uint64_t linesPrinted;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob * create(PaloJobRequest *jobRequest) {
		return new CellExportJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellExportJob(PaloJobRequest *jobRequest) :
		AreaJob(jobRequest), linesPrinted(0) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		findCube(true, false);
		checkProperties();

		// condition
		if (jobRequest->condition) {
			condition.reset(Condition::parseCondition(*(jobRequest->condition)));
		}

		// setup dimensions
		const IdentifiersType *dimensions = cube->getDimensions();
		size_t dimCount = dimensions->size();

		if (jobRequest->path || jobRequest->pathName) {
			findPath();
		}

		if (!jobRequest->area) {
			if (jobRequest->areaName && jobRequest->areaName->size() != dimCount) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::NAME_AREA, "");
			}
			// whole cube will be exported or areaName used, prepare vector for ids
			jobRequest->area = new vector<IdentifiersType>(dimensions->size());
			if (jobRequest->areaName) {
				jobRequest->area->resize(jobRequest->areaName->size());
				for (uint32_t i = 0; i < jobRequest->areaName->size(); i++) {
					CPDimension dim = database->lookupDimension((*dimensions)[i], false);
					for (uint32_t j = 0; j < jobRequest->areaName->at(i).size(); j++) {
						IdentifierType id = dim->findElementByName(jobRequest->areaName->at(i).at(j), 0, false)->getIdentifier();
						jobRequest->area->at(i).push_back(id);
					}
				}
			}
		} else {
			if (jobRequest->area->size() != dimCount) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", PaloRequestHandler::ID_AREA, "");
			}
			for (uint32_t i = 0; i < jobRequest->area->size(); ++i) {
				CPDimension dim = database->lookupDimension((*dimensions)[i], false);
				if (dim->getDimensionType() != Dimension::VIRTUAL) {
					for (uint32_t j = 0; j < jobRequest->area->at(i).size(); ++j) {
						dim->findElement(jobRequest->area->at(i).at(j), 0, false);
					}
				}
			}
		}

		vector<User::RoleDbCubeRight> vRights;
		if (User::checkUser(user)) {
			user->fillRights(vRights, User::cellDataRight, database, cube);
		}
		bool checkPermissions = cube->getMinimumAccessRight(user) == RIGHT_NONE;

		fillEmptyDim(vRights, checkPermissions, *jobRequest->area, cube, database, user);

		CubeArea::CellType type = CubeArea::ALL;
		if (jobRequest->baseOnly) {
			type = (CubeArea::CellType)(type & ~CubeArea::CONSOLIDATED);
		}
		if (jobRequest->type == 1) {
			type = (CubeArea::CellType)(type & ~CubeArea::BASE_STRING);
		} else if (jobRequest->type == 2) {
			type = CubeArea::BASE_STRING;
		}

		response = new HttpResponse(HttpResponse::OK);
		setToken(cube);

		IdentifiersType lastKey(cube->getDimensions()->size());
		PCubeArea area(new CubeArea(database, cube, *jobRequest->area));
		uint64_t freeCount = jobRequest->blockSize;
		bool endOfArea = false;
		bool mySkipEmpty = jobRequest->skipEmpty;
		if (!mySkipEmpty && condition && !checkCondition(CellValue::NullNumeric)) {
			mySkipEmpty = true;
		}

		CPArea startPath;
		if (jobRequest->path || jobRequest->pathName) {
			startPath = cellPath;
		}

		do {
			Area::PathIterator pit;
			if (startPath) {
				pit = area->getIterator(startPath);
				++pit;
			} else {
				pit = area->pathBegin();
			}

			bool splitToTheEnd = false;
			bool moreValues = false;
			std::list<PCubeArea> larea = area->split(pit, mySkipEmpty || jobRequest->baseOnly ? 0 : jobRequest->blockSize, splitToTheEnd);
			std::list<PCubeArea>::iterator it;
			for (it = larea.begin(); it != larea.end(); ++it) {
				PCubeArea calcArea = checkRights(vRights, checkPermissions, *it, 0, cube, database, user, false, noPermission, isNoPermission, this->dims);

				PCellStream cs;
				PCellStream props;
				if (calcArea->getSize()) {
					RulesType rulesType = jobRequest->useRules ? ALL_RULES : NO_RULES;
					if (jobRequest->useRules && !jobRequest->showRule) {
						rulesType = RulesType(rulesType | NO_RULE_IDS);
					}
					cs = cube->calculateArea(calcArea, type, rulesType, mySkipEmpty, freeCount ? freeCount : UNLIMITED_SORTED_PLAN);
					if (jobRequest->properties) {
						props = getCellPropsStream(database, cube, calcArea, *jobRequest->properties);
					}
				}
				if (cs) { // empty plan for whatever reason
					moreValues = loop(*it, calcArea, cs, &freeCount, props, vRights, &lastKey, false);
					if (!freeCount) {
						++it;
						break;
					}
				}
			}
			if (it == larea.end() && splitToTheEnd && !moreValues) {
				endOfArea = true;
			}
			startPath = CPArea(new Area(lastKey, true));
		} while (freeCount && !endOfArea);

		uint32_t progressOfExport;
		if (endOfArea) {
			progressOfExport = progressMax;
		} else {
			Area::PathIterator pit = area->getIterator(startPath);
			progressOfExport = (uint32_t)((pit - area->pathBegin()) / area->getSize() * progressMax);
		}

		StringBuffer *body = &response->getBody();
		body->appendCsvInteger(progressOfExport);
		body->appendCsvInteger(progressMax);
		body->appendEol();
	}

	virtual bool checkCondition(const CellValue &value) const
	{
		bool result = true;
		if (condition.get()) {
			if (!value.isError()) {
				result = condition->check(value);
			}
		}
		return result;
	}
};

}

#endif
