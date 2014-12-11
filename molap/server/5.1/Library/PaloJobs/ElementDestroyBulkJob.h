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

#ifndef PALO_JOBS_ELEMENT_DESTROY_BULK_JOB_H
#define PALO_JOBS_ELEMENT_DESTROY_BULK_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief element destroy bulk
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ElementDestroyBulkJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest)
	{
		return new ElementDestroyBulkJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ElementDestroyBulkJob(PaloJobRequest* jobRequest) :
			DirectPaloJob(jobRequest)
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType()
	{
		return WRITE_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute()
	{
		bool ret = false;
		IdentifiersType elements;
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS_CELL_REPLACE; commitTry++) {
			Context *context = Context::getContext();
			bool optimistic = commitTry == 0;
			elements.clear();

			server = context->getServerCopy();
			findDatabase(true, true);
			findDimension(true);

			set<IdentifierType> elemSet;
			if (jobRequest->elements) {
				for (size_t i = 0; i < jobRequest->elements->size(); i++) {
					element_push_back(jobRequest->elements->at(i), elements, elemSet);
				}
			} else if (jobRequest->elementsName) {
				for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {
					Element* e = dimension->findElementByName(jobRequest->elementsName->at(i), user.get(), true);
					element_push_back(e->getIdentifier(), elements, elemSet);
				}
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing elements", PaloRequestHandler::ID_ELEMENTS, "");
			}
			elemSet.clear();

			PElementsContext ec = PElementsContext(new ElementsContext(server, database, dimension, elements, user, session, !session->isWorker()));
			context->setElementsContext(ec);

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
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't create delete element(s).");
		}

		if (!elements.empty()) {
			server->invalidateCache();
		}
		generateOkResponse(dimension);
	}

private:
	void element_push_back(IdentifierType id, IdentifiersType &vElems, set<IdentifierType> &sElems)
	{
		if (sElems.find(id) == sElems.end()) {
			vElems.push_back(id);
			sElems.insert(id);
		}

	}
};

}

#endif
