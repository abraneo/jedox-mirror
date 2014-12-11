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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef CUBE_CONVERT_JOB_H
#define	CUBE_CONVERT_JOB_H 1

#include "palo.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cube convert
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CubeConvertJob : DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CubeConvertJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CubeConvertJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

	void compute() {
		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);

			if (!server->isEnableGpu()) {
				throw ErrorException(ErrorException::ERROR_GPU_SERVER_NOT_ENABLED, "cube convert is not possible");
			}

			ItemType targetType = (ItemType)jobRequest->type;

            if(targetType == cube->getType()){
                Logger::info << "Conversion is not necessary. Cube is already type: " << targetType << endl;
            }
            else if ((targetType == NORMALTYPE && cube->getType() == GPUTYPE) ||
			        (targetType == GPUTYPE && cube->getType() == NORMALTYPE)) {
				server->invalidateCache();

				// switch cube type
				database->changeCubeType(server, cube, targetType, user, true);
			} else {
				throw ParameterException(ErrorException::ERROR_INVALID_TYPE, "wrong cube type or target cube type", PaloRequestHandler::ID_TYPE, targetType);
			}
			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't convert the cube.");
		}

		generateCubeResponse(cube);
	}
};
}

#endif // CUBE_CONVERT_JOB_H
