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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_CELL_REPLACE_JOB_H
#define PALO_JOBS_CELL_REPLACE_JOB_H 1

#include "palo.h"

#include <iostream>

#include "Exceptions/ParameterException.h"
#include "Olap/Dimension.h"
#include "Olap/Lock.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell replace
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellReplaceJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellReplaceJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellReplaceJob(PaloJobRequest* jobRequest) :
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
			findLockedPaths(0);

			cube->disableTokenUpdate();

			// get splash mode
			SplashMode mode = PaloJob::splashMode(jobRequest->splash);

			// get "add" flag, true means add value instead of set
			bool addFlag = jobRequest->add;

			if (addFlag) {
				if (mode != DISABLED && mode != DEFAULT) {
					throw ParameterException(ErrorException::ERROR_INVALID_SPLASH_MODE, "add=1 requires splash mode DEFAULT or DISABLED", PaloRequestHandler::SPLASH, (int)mode);
				}
			}

			// get event-process flag, false means to circumvent the processor (right will be checked by cube)
			bool eventProcessor = jobRequest->eventProcess;

			// try to change the cell value
			bool withinEvent = server->isBlocking();

			if (!withinEvent && session->isWorker()) {
				throw ParameterException(ErrorException::ERROR_NOT_WITHIN_EVENT, "worker cell/replace requires an event/begin", "session", session->getSid());
			}

			// do not use the supervision event processor if within event or requested by user
			bool checkArea = eventProcessor && !withinEvent;

			bool unused = false; //NULL is also an unused value
			PCellValueContext cvc = PCellValueContext(new CellValueContext(cube, CellValueContext::CELL_REPLACE_JOB, database, user, session, checkArea, addFlag, mode, lockedPaths, PCubeArea(), unused, NULL));
			context->setCellValueContext(cvc);

			CellValue value;
			CubeArea::CellType cellType = cellPath->getType(cellPath->pathBegin());
			if (cellType & CubeArea::BASE_STRING) {
				value = CellValue::NullString;
				if (jobRequest->value && !jobRequest->value->empty()) {
					value = *(jobRequest->value);
				}

				for (size_t i = 0; i < value.length(); i++) {
					if (value[i] >= 0 && value[i] < 32 && value[i] != 9 && value[i] != 10 && value[i] != 13) {
						// only \n \r \t are allowed from special characters
						throw ParameterException(ErrorException::ERROR_INVALID_STRING, "string value contains an illegal character", "value", value);
					}
				}

				cvc->addPathAndValue(cellPath, value, !eventProcessor, cellType);
			} else {
				if (jobRequest->value) {
					double dvalue = StringUtils::stringToDouble(*(jobRequest->value));
					if (dvalue != 0) {
						value = dvalue;
					}
				}

				cvc->addPathAndValue(cellPath, value, !eventProcessor, cellType);
			}
			if (!optimistic) {
				context->setPesimistic();
			}

			updateLics = true;
			ret = server->commit();

			if (ret) {
				updateLics = false;
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "CellReplaceJob failed. Internal error occured.");
		}

		server->invalidateCache(database ? database->getId() : NO_IDENTIFIER, cube ? cube->getId() : NO_IDENTIFIER);

		generateOkResponse(cube);
	}

};

}

#endif
