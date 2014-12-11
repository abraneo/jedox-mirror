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
 * 
 *
 */

#ifndef PALO_JOBS_ELEMENT_MOVE_BULK_JOB_H
#define PALO_JOBS_ELEMENT_MOVE_BULK_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief element move
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ElementMoveBulkJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ElementMoveBulkJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ElementMoveBulkJob(PaloJobRequest* jobRequest) :
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
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);
			findDimension(true);
			size_t dimSize = dimension->sizeElements();
			if (dimension->getDimensionType() == Dimension::VIRTUAL && dimension->getName() == SystemDatabase::NAME_LICENSE_DIMENSION) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "/element/move_bulk for licences is not supported");
			} else {
				map<IdentifierType, PositionType> elemIds;
				map<PositionType, IdentifierType> positions;
				vector<pair<Element *, PositionType> > elem_pos;
				if (!jobRequest->elements && !jobRequest->elementsName) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing elements", PaloRequestHandler::ID_ELEMENTS, "");
				}
				if (!jobRequest->positions) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing positions", PaloRequestHandler::POSITIONS, "");
				}

				size_t elemSize = jobRequest->elements ? jobRequest->elements->size() : jobRequest->elementsName->size();
				if (elemSize != jobRequest->positions->size()) {
					throw ErrorException(ErrorException::ERROR_PARAMETER_MISSING, "different number of elements and positions!");
				}

				elem_pos.reserve(elemSize);
				for (size_t i = 0; i < elemSize; i++) {
					Element *elem;
					if (jobRequest->elements) {
						elem = dimension->findElement(jobRequest->elements->at(i), user.get(), true);
					} else {
						elem = dimension->findElementByName(jobRequest->elementsName->at(i), user.get(), true);
					}
					IdentifierType id = elem->getIdentifier();

					PositionType pos = jobRequest->positions->at(i);
					if (pos >= dimSize) {
						throw ParameterException(ErrorException::ERROR_INVALID_POSITION, "position " + StringUtils::convertToString((uint32_t)pos) + " is out of range", "position", (int)pos);
					}

					bool isNew = true;
					map<PositionType, IdentifierType>::iterator pit = positions.find(pos);
					if (pit == positions.end()) {
						positions.insert(make_pair(pos, id));
					} else if (pit->second == id) {
						isNew = false;
					} else {
						throw ParameterException(ErrorException::ERROR_INVALID_POSITION, "multiple elements have position " + StringUtils::convertToString((uint32_t)pos), "position", (int)pos);
					}

					map<IdentifierType, PositionType>::iterator eit = elemIds.find(id);
					if (eit == elemIds.end()) {
						elemIds.insert(make_pair(id, pos));
					} else if (eit->second == pos) {
						isNew = false;
					} else {
						throw ParameterException(ErrorException::ERROR_INVALID_POSITION, "element " + StringUtils::convertToString(id) + " has multiple positions", "element", (int)id);
					}

					if (isNew) {
						elem_pos.push_back(make_pair(elem, pos));
					}
				}

				dimension->moveElements(server, database, elem_pos, user, true);

				ret = server->commit();

				if (ret) {
					break;
				}
				clear();
			}
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't move element");
		}
		server->invalidateCache();

		generateOkResponse(dimension);
	}
};

}

#endif
