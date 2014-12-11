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
 * 
 *
 */

#ifndef PALO_JOBS_CUBE_CLEAR_JOB_H
#define PALO_JOBS_CUBE_CLEAR_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cube clear
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CubeClearJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CubeClearJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CubeClearJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			Context *context = Context::getContext();

			server = context->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);

			cube->disableTokenUpdate();

			if (jobRequest->complete) {
				if (Logger::isTrace())
					Logger::trace << "clear all cube cells" << endl;

				cube->clearCells(server, database, user, true);
			} else if (cube->sizeFilledCells() > 0) {
				uint32_t numResult = 0;
				PCubeArea paths;

				const IdentifiersType* dims = cube->getDimensions();
				vector<CPDimension> dimensions;
				for (IdentifiersType::const_iterator it = dims->begin(); it != dims->end(); ++it) {
					dimensions.push_back(database->lookupDimension(*it, false));
				}

				if (jobRequest->area) {
					paths = area(database, cube, jobRequest->area, &dimensions, numResult, false);
				} else if (jobRequest->areaName) {
					paths = area(database, cube, jobRequest->areaName, &dimensions, numResult, false);
				} else {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "area is empty", PaloRequestHandler::ID_AREA, "");
				}

				if (0 < numResult) {
					if (Logger::isTrace())
						Logger::trace << "clear cube area of " << numResult << " cells" << endl;

					cube->clearCells(server, database, paths, user, true);
				}
			}
			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Couldn't clear cells in cube. Somebody else is to the changes in same area.");
		}
		server->invalidateCache();

		generateCubeResponse(cube);
	}
};

}

#endif
