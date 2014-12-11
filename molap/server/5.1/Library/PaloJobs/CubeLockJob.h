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

#ifndef PALO_JOBS_CUBE_LOCK_JOB_H
#define PALO_JOBS_CUBE_LOCK_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "Olap/AttributesCube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cube lock
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CubeLockJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CubeLockJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CubeLockJob(PaloJobRequest* jobRequest) :
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
		PLock lock;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);

			ItemType type = cube->getType();
			if (type != NORMALTYPE && (type != ATTRIBUTETYPE || dynamic_cast<AttributesCube *>(cube.get())->isCellPropertiesCube())) {
				throw ParameterException(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED, "Cube lock is allowed only for normal and dimension attribute cubes.", "cube", cube->getName());
			}
			if (type == GPUTYPE) {
				throw ParameterException(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED, "Cube lock is currently not supported on GPU cubes.", "cube", cube->getName());
			}
			cube->checkCubeAccessRight(user, RIGHT_WRITE, true, false);

			const IdentifiersType * dimensions = cube->getDimensions();
			vector<CPDimension> dims;
			for (IdentifiersType::const_iterator it = dimensions->begin(); it != dimensions->end(); ++it) {
				dims.push_back(database->lookupDimension(*it, false));
			}

			PArea area;
			uint32_t numResult;

			if (jobRequest->area) {
				area = PaloJob::area(database, cube, jobRequest->area, &dims, numResult, false);
				lock = cube->lockCube(area, areaToString(jobRequest->area), false, user);
			} else if (jobRequest->areaName) {
				area = PaloJob::area(database, cube, jobRequest->areaName, &dims, numResult, false);
				lock = cube->lockCube(area, areaToString(jobRequest->areaName), false, user);
			} else if (jobRequest->complete) {
				lock = cube->lockCube(PArea(), areaToString(cube->getDimensions()->size()), true, user);
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "area is empty", PaloRequestHandler::ID_AREA, "");
			}
			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't lock the cube.");
		}

		generateLockResponse(server, cube, lock);
	}
};

}

#endif
