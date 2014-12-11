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
 * \author Marko Stijak, Banja Luka, Bosnia and Herzegovina
 * 
 *
 */

#ifndef PALO_JOBS_CELL_GOALSEEK_JOB_H
#define PALO_JOBS_CELL_GOALSEEK_JOB_H 1

#include "palo.h"

#include <iostream>

#include "Exceptions/ParameterException.h"
#include "Olap/Dimension.h"
#include "Olap/PaloSession.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell goal seek
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellGoalSeekJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellGoalSeekJob(jobRequest);
	}


public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellGoalSeekJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS_CELL_REPLACE; commitTry++) {
			Context *context = Context::getContext();

			bool optimistic = commitTry == 0;
			server = context->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);
			findPath();
			assertParameter(PaloRequestHandler::VALUE, jobRequest->value);

			CellValueContext::GoalSeekType gs_type = CellValueContext::GS_COMPLETE; // jobRequest->area == NULL

			if (jobRequest->area) {

				if (jobRequest->area->size() != cellPath->dimCount()) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "Invalid size of area vectors.");
				}

				switch (jobRequest->type) {
				case 0:
					// area was defined but will not be used for type 0
					gs_type = CellValueContext::GS_COMPLETE;
					break;
				case 1:
					gs_type = CellValueContext::GS_EQUAL;
					break;
				case 2:
					gs_type = CellValueContext::GS_RELATIVE;
					break;
				default:
					throw ParameterException(ErrorException::ERROR_INVALID_TYPE, "Wrong goalseek algorithm type", "type", jobRequest->type);
				}
			} else if (jobRequest->type != 0) { // jobRequest->area == NULL
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "Invalid GoalSeek type. Area parameter not defined.", "type", "");
			}

			cube->disableTokenUpdate();

			double value = StringUtils::stringToDouble(*(jobRequest->value));

			bool unused = false; //NULL is also an unused value
			PCellValueContext cvc = PCellValueContext(new CellValueContext(cube, CellValueContext::CELL_GOAL_SEEK_JOB, database, user, session, unused, unused, (int)unused, PPaths(), PCubeArea(), unused, NULL));
			context->setCellValueContext(cvc);
			cvc->addPathAndValue(cellPath, CellValue(value), unused, CubeArea::NONE);
			cvc->addGoalSeekParams(PArea(jobRequest->area ? new Area(*jobRequest->area) : 0), gs_type);

			if (!optimistic) {
				context->setPesimistic();
			}

			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "CellGoalSeekJob failed. Internal error occured.");
		}

		server->invalidateCache();

		generateOkResponse(cube);
	}

};

}

#endif
