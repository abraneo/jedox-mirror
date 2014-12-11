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

#ifndef PALO_JOBS_CELL_REPLACE_BULK_JOB_H
#define PALO_JOBS_CELL_REPLACE_BULK_JOB_H 1

#include "palo.h"

#include "Exceptions/CommunicationException.h"
#include "Olap/Lock.h"
#include "Olap/PaloSession.h"
#include "Engine/EngineBase.h"
#include "PaloDispatcher/DirectPaloJob.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp> //include all types plus i/o

using namespace boost::posix_time;

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief cell replace bulk
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CellReplaceBulkJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new CellReplaceBulkJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CellReplaceBulkJob(PaloJobRequest* jobRequest) :
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
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS_CELL_REPLACE; commitTry++) {
//			ptime parsingStart(microsec_clock::local_time());
			Context *context = Context::getContext();

			bool optimistic = commitTry == 0;
			server = context->getServerCopy();
			findDatabase(true, true);
			findCube(true, true);
			findCellPaths(0, user.get());
			findLockedPaths(0);
			assertParameter(PaloRequestHandler::VALUES, jobRequest->values);

			cube->disableTokenUpdate();

			// get splash mode
			SplashMode mode = splashMode(jobRequest->splash);

			// true means add value instead of set
			if (jobRequest->add) {
				if (mode != DISABLED && mode != DEFAULT) {
					throw ParameterException(ErrorException::ERROR_INVALID_SPLASH_MODE, "add=1 requires splash mode DEFAULT or DISABLED", PaloRequestHandler::SPLASH, (int)mode);
				}
			}

			bool withinEvent = server->isBlocking();

			// set cell value one-by-one

			if (session->isWorker()) {
				if (!withinEvent || getSid() != server->getActiveSession()) {
					throw ParameterException(ErrorException::ERROR_NOT_WITHIN_EVENT, "worker cell/replace_bulk requires an event/begin", "session", getSid());
				}
			}

			// get event-process flag, false means to circumvent the processor (right will be checked by cube)
			bool eventProcessor = jobRequest->eventProcess;
			bool checkArea = eventProcessor && !withinEvent;

			// check size of values
			if (cellPaths->size() > jobRequest->values->size()) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing values", PaloRequestHandler::VALUES, (int)jobRequest->values->size());
			}
			const IdentifiersType *dims = cube->getDimensions();
			size_t dimCount = dims->size();

			vector<vector<char> > vElemTypes(dimCount);
			vector<Dimension *> vDims(dimCount);
			for (size_t i = 0; i < dimCount; i++) {
				vElemTypes[i].resize(cellPaths->size());
				vDims[i] = database->lookupDimension(dims->at(i), false).get();
			}

			bool unused = false; //NULL is also an unused value
			PCellValueContext cvc = PCellValueContext(new CellValueContext(cube, CellValueContext::CELL_REPLACE_BULK_JOB, database, user, session, checkArea, jobRequest->add, mode, lockedPaths, PCubeArea(), unused, NULL));
			context->setCellValueContext(cvc);

			cvc->pathVectorAndValue.reserve(cellPaths->size());

			bool hasCons = false;
			for (size_t i = 0; i < cellPaths->size(); i++) {
				IdentifiersType& path = cellPaths->at(i);
				IdentifiersType& prev = cellPaths->at(i ? i - 1 : 0);

				CubeArea::CellType cellType = CubeArea::BASE_NUMERIC;
				for (size_t j = 0; j < dimCount; j++) {
					if (vDims[j]->getDimensionType() == Dimension::VIRTUAL) {
						continue;
					}
					if (!i || path[j] != prev[j]) {
						Element *element = vDims[j]->lookupElement(path[j], false);
						if (!element) {
							throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with id '" + StringUtils::convertToString(path[j]) + "' not found in dimension '" + vDims[j]->getName() + "'", "id", path[j]);
						}
						if (element->isStringConsolidation()) {
							vElemTypes[j][i] = (char)Element::STRING;
						} else {
							vElemTypes[j][i] = (char)element->getElementType();
						}
					} else {
						vElemTypes[j][i] = vElemTypes[j][i - 1];
					}
					if (vElemTypes[j][i] == (char)Element::STRING) {
						cellType = CubeArea::BASE_STRING;
					} else if (cube->supportsAggregations() && vElemTypes[j][i] == (char)Element::CONSOLIDATED) {
						if (cellType != CubeArea::BASE_STRING) {
							cellType = CubeArea::CONSOLIDATED;
						}
					}
				}

				if (cellType == CubeArea::BASE_STRING) {
					string value = jobRequest->values->at(i);
					cvc->addPathAndValue(cellPaths->at(i), value.empty() ? CellValue::NullString : CellValue(value), !eventProcessor, cellType);
				} else {
					if (cellType == CubeArea::CONSOLIDATED) {
						hasCons = true;
					}
					double value = StringUtils::stringToDouble(jobRequest->values->at(i));
					cvc->addPathAndValue(cellPaths->at(i), value == 0 ? CellValue::NullNumeric : CellValue(value), !eventProcessor, cellType);
				}
			}
			if (hasCons) {
				cvc->vElemTypes = &vElemTypes;
			}

//			ptime parsingEnd(microsec_clock::local_time());
//			size_t parsingTime = (parsingEnd-parsingStart).total_microseconds();
//			Logger::info << "CellReplaceBulkJob preparations took " << parsingTime/1000 << " ms" << endl;

			if (!optimistic) {
				context->setPesimistic();
			}

			updateLics = true;
//			ptime indexingStart(microsec_clock::local_time());
			ret = server->commit();
			if (ret) {
//				ptime indexingEnd(microsec_clock::local_time());
//				size_t indexingTime = (indexingEnd-indexingStart).total_microseconds();
//				Logger::info << "CellReplaceBulkJob server->commit took " << indexingTime/1000 << " ms" << endl;
				updateLics = false;
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "CellReplaceBulkJob failed. Internal error occurred.");
		}

		server->invalidateCache(database ? database->getId() : NO_IDENTIFIER, cube ? cube->getId() : NO_IDENTIFIER);

		generateOkResponse(cube);
	}
};

}

#endif
