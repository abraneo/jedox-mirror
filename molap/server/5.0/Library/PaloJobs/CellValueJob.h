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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_VALUE_JOB_H
#define PALO_JOBS_CELL_VALUE_JOB_H 1

#include "palo.h"

#include "PaloJobs/AreaJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell value job
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellValueJob : public AreaJob {
public:
	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellValueJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellValueJob(PaloJobRequest* jobRequest) :
		AreaJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

	void compute() {
		findPath();
		checkToken(cube);

		if (User::checkUser(user)) {
			User::RightSetting rs(User::checkCellDataRightCube(database, cube));
			cube->checkAreaAccessRight(database, user, cellPath, rs, false, RIGHT_READ);
		}
		checkProperties();

		PCubeArea areaCalc(new CubeArea(database, cube, *jobRequest->path));

		PCellStream cs = cube->calculateArea(areaCalc, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
		response = new HttpResponse(HttpResponse::OK);
		setToken(cube);
		vector<CellValue> prop_vals;
		CellValue value;
		if (!cs->next()) {
			value = CellValue(ErrorException::ERROR_ELEMENT_NOT_FOUND);
		} else {
			value = cs->getValue();
		}
		if (jobRequest->properties) {
			RightsType r = RIGHT_DELETE;

			vector<User::RoleCubeRight> rcRights;
			if (User::checkUser(user)) {
				user->getRoleRights(User::cellDataRight, rcRights);
				user->getCubeDataRights(database, cube, rcRights, true);
				r = user->getCellRight(database, cube, *jobRequest->path, rcRights);
			}

			PCellStream props = getCellPropsStream(database, cube, areaCalc, *jobRequest->properties);
			fillProps(prop_vals, *jobRequest->path, props, *jobRequest->properties, r);
		}
		generateCellValueResponse(*jobRequest->path, value, prop_vals);
	}
};

}

#endif
