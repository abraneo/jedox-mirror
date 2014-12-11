/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_ELEMENT_REPLACE_BULK_JOB_H
#define PALO_JOBS_ELEMENT_REPLACE_BULK_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief element bulk replace
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ElementReplaceBulkJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ElementReplaceBulkJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ElementReplaceBulkJob(PaloJobRequest* jobRequest) :
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
			Element::Type elementType = elementTypeByIdentifier(jobRequest->type);

			size_t elemcount = 0;
			if (jobRequest->elements) {
				elemcount = jobRequest->elements->size();
			} else if (jobRequest->elementsName) {
				elemcount = jobRequest->elementsName->size();
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing elements", PaloRequestHandler::ID_ELEMENTS, "");
			}
			if (elementType == Element::CONSOLIDATED) {
				if (!jobRequest->children || jobRequest->children->size() != elemcount) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
				}
				if (!jobRequest->weights || jobRequest->weights->size() != elemcount) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weights", PaloRequestHandler::WEIGHTS, "");
				}
				vector<IdentifiersType>::const_iterator itc;
				vector<vector<double> >::const_iterator itw;
				for (itc = jobRequest->children->begin(), itw = jobRequest->weights->begin(); itc != jobRequest->children->end(); itc++, itw++) {
					if (itc->size() > itw->size()) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weights", PaloRequestHandler::WEIGHTS, "");
					}
				}
			}

			vector<pair<Element *, bool> > elemsToReplace;
			CubeRulesArray disabledRules;

			if (jobRequest->elements) {
				for (IdentifiersType::const_iterator it = jobRequest->elements->begin(); it != jobRequest->elements->end(); it++) {
					elemsToReplace.push_back(pair<Element *, bool> (dimension->findElement(*it, user.get(), true), false));
				}
			} else {
				for (vector<string>::const_iterator it = jobRequest->elementsName->begin(); it != jobRequest->elementsName->end(); it++) {
					Element *e;
					bool created = false;
					try {
						e = dimension->findElementByName(*it, user.get(), true);
					} catch (ParameterException ex) {
						e = dimension->addElement(server, database, NO_IDENTIFIER, *it, elementType, user, true);
						created = true;
					}
					elemsToReplace.push_back(pair<Element *, bool> (e, created));
				}
			}

			IdentifiersType elemsToDeleteFromCubes;
			size_t i = 0;
			for (vector<pair<Element *, bool> >::iterator it = elemsToReplace.begin(); it != elemsToReplace.end(); it++, i++) {
				Element *e = it->first;
				Element::Type et = e->getElementType();
				IdentifiersWeightType tmp;
				if (!it->second) {
					if (et != elementType) {
						dimension->changeElementType(server, database, e, elementType, user, false, &disabledRules, &elemsToDeleteFromCubes, true);
					}
				}
				if (elementType == Element::CONSOLIDATED) {
					set<IdentifierType> keep;
					IdentifiersWeightType add;
					IdentifiersType::const_iterator itc;
					vector<double>::const_iterator itw;
					for (itc = jobRequest->children->at(i).begin(), itw = jobRequest->weights->at(i).begin(); itc != jobRequest->children->at(i).end(); itc++, itw++) {
						if (!it->second) {
							keep.insert(*itc);
						}
						add.push_back(IdentifierWeightType(*itc, *itw));
					}

					if (!it->second) {
						dimension->removeChildrenNotIn(server, database, user, e, &keep, &disabledRules, true);
					}
					dimension->addChildren(server, database, e, &add, user, &disabledRules, false, false, true, &elemsToDeleteFromCubes);
				}
			}

			if (!elemsToDeleteFromCubes.empty()) {
				dimension->removeElementsFromCubes(server, database, user, elemsToDeleteFromCubes, true, &disabledRules, Dimension::DEL_NUM, true);
			}
			dimension->updateElementsInfo();

			for (CubeRulesArray::iterator it = disabledRules.begin(); it != disabledRules.end(); it++) {
				if (it->first) {
					it->first->activateRules(server, database, it->second, ACTIVE, PUser(), NULL, false, false);
//					for (vector<PRule>::iterator ruleIt = it->second.begin(); ruleIt != it->second.end(); ruleIt++) {
//						if (!(*ruleIt)->isActive()) {
//							it->first->activateRule(server, database, *ruleIt, true, PUser(), NULL, false, false);
//						}
//					}
				}
			}

			if (!session->isWorker()) {
				for (vector<pair<Element *, bool> >::iterator it = elemsToReplace.begin(); it != elemsToReplace.end(); ++it) {
					if (it->second) {
						dimension->addElementEvent(server, database, it->first);
					}
				}
			}

			server->updateGpuBins(dimension, server, jobRequest, database, Context::getContext(), user);

			ret = server->commit();

			if (ret) {
				break;
			}
			clear();
		}
		if (!ret) {
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't replace element(s).");
		}

		server->invalidateCache();
		generateOkResponse(dimension);
	}
};

}

#endif
