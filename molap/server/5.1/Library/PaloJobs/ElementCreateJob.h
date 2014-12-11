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

#ifndef PALO_JOBS_ELEMENT_CREATE_JOB_H
#define PALO_JOBS_ELEMENT_CREATE_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief element create
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ElementCreateJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest)
	{
		return new ElementCreateJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ElementCreateJob(PaloJobRequest* jobRequest) :
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
		for (int commitTry = 0; commitTry < Commitable::COMMIT_REPEATS; commitTry++) {
			server = Context::getContext()->getServerCopy();
			findDatabase(true, true);
			findDimension(true);

			bool hasChildren = false;

			string name = "";

			if (jobRequest->newName != 0) {
				name = *jobRequest->newName;
			}

			Element::Type elementType = elementTypeByIdentifier(jobRequest->type);

			// create element
			element = dimension->addElement(server, database, NO_IDENTIFIER, name, elementType, user, true);

			// add children
			if (elementType == Element::CONSOLIDATED) {
				IdentifiersWeightType children;

				// get weights
				vector<double>* weights = 0;
				if (jobRequest->weights) {
					if (jobRequest->weights->size() == 1 && jobRequest->weights->at(0).size() > 0) {
						weights = &jobRequest->weights->at(0);
					}
				}

				// get children
				if (jobRequest->children) {
					if (jobRequest->children->size() != 1 || jobRequest->children->at(0).size() == 0) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
					}

					IdentifiersType& childrenIds = jobRequest->children->at(0);

					if (weights && childrenIds.size() > weights->size()) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
					}

					for (size_t i = 0; i < childrenIds.size(); i++) {
						double weight = 1.0;

						if (weights) {
							weight = weights->at(i);
						}

						children.push_back(IdentifierWeightType(childrenIds.at(i), weight));

						hasChildren = true;
					}
				} else if (jobRequest->childrenName) {
					if (jobRequest->childrenName->size() != 1 || jobRequest->childrenName->at(0).size() == 0) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::NAME_CHILDREN, "");
					}

					vector<string>& childrenNames = jobRequest->childrenName->at(0);

					if (weights && childrenNames.size() > weights->size()) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
					}

					for (size_t i = 0; i < childrenNames.size(); i++) {
						double weight = 1.0;

						Element * child = dimension->findElementByName(childrenNames.at(i), user.get(), true);

						if (weights) {
							weight = weights->at(i);
						}

						children.push_back(IdentifierWeightType(child->getIdentifier(), weight));

						hasChildren = true;
					}
				}

				if (hasChildren) {
					dimension->addChildren(server, database, element, &children, user, NULL, false, false, true, NULL);
				}
				dimension->updateElementsInfo();
			}

			if (!session->isWorker()) {
				dimension->addElementEvent(server, database, element);
			}

			server->updateGpuBins(dimension, server, jobRequest, database, Context::getContext(), user);

			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't create new element.");
		}

		generateElementResponse(dimension, element, false);

	}
}
;

}

#endif
