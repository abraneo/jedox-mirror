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

#ifndef PALO_JOBS_DIMENSION_CREATE_JOB_H
#define PALO_JOBS_DIMENSION_CREATE_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief dimension create
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DimensionCreateJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new DimensionCreateJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	DimensionCreateJob(PaloJobRequest* jobRequest) :
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

		bool isInfo = false;

		if (jobRequest->type == 0) {
		} else if (jobRequest->type == 3) {
			isInfo = true;
		} else {
			throw ParameterException(ErrorException::ERROR_INVALID_TYPE, "wrong dimension type", PaloRequestHandler::ID_TYPE, jobRequest->type);
		}

		if (jobRequest->newName == 0) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "parameter missing", PaloRequestHandler::NEW_NAME, "");
		}

		bool ret = false;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);

			checkToken(database);

			dimension = database->addDimension(server, *jobRequest->newName, user, isInfo, true, true);

			ret = server->commit();

			if (ret) {
				break;
			}
			dimension->beforeRemoveDimension(server, database, true);
			Context::getContext()->deleteCubesFromDisk();
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't create new dimension.");
		}

		generateDimensionResponse(dimension);
	}
};

}

#endif
