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

			PJournalMem journal = database->getJournal();
			if (journal) {
				journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENTS_BULK_START);
				journal->appendInteger(dimension->getId());
				journal->nextLine();
			}

			size_t elemcount = 0;
			if (jobRequest->elements) {
				elemcount = jobRequest->elements->size();
			} else if (jobRequest->elementsName) {
				elemcount = jobRequest->elementsName->size();
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing elements", PaloRequestHandler::ID_ELEMENTS, "");
			}

			Element::Type elementType = jobRequest->types ? Element::UNDEFINED : elementTypeByIdentifier(jobRequest->type);
			bool err = false;
			if (jobRequest->types) {
				if (jobRequest->types->size() != elemcount) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing types", PaloRequestHandler::ID_TYPES, "expecting " + StringUtils::convertToString(elemcount));
				}
				err = jobRequest->children && jobRequest->children->size() != elemcount;
			} else {
				err = elementType == Element::CONSOLIDATED && (!jobRequest->children || jobRequest->children->size() != elemcount);
			}
			if (err) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
			}

			if (jobRequest->types) {
				err = jobRequest->weights && jobRequest->weights->size() != elemcount;
			} else {
				err = elementType == Element::CONSOLIDATED && (jobRequest->weights ? jobRequest->weights->size() != elemcount : false);
			}
			if (err) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weights", PaloRequestHandler::WEIGHTS, "");
			}

			vector<Element::Type> elementTypes;
			if (jobRequest->types) {
				for (size_t i = 0; i < jobRequest->types->size(); i++) {
					Element::Type eType = elementTypeByIdentifier(jobRequest->types->at(i));
					elementTypes.push_back(eType);
					if (eType == Element::CONSOLIDATED && jobRequest->children && jobRequest->weights) {
						if (jobRequest->children->at(i).size() > jobRequest->weights->at(i).size()) {
							err = false;
							break;
						}
					}
				}
			} else if (elementType == Element::CONSOLIDATED && jobRequest->children && jobRequest->weights) {
				for (size_t i = 0; i < jobRequest->children->size(); i++) {
					if (jobRequest->children->at(i).size() > jobRequest->weights->at(i).size()) {
						err = false;
						break;
					}
				}
			}
			if (err) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weights", PaloRequestHandler::WEIGHTS, "");
			}

			vector<pair<Element *, bool> > elemsToReplace;
			CubeRulesArray disabledRules;

			if (jobRequest->elements) {
				for (IdentifiersType::const_iterator it = jobRequest->elements->begin(); it != jobRequest->elements->end(); ++it) {
					elemsToReplace.push_back(pair<Element *, bool> (dimension->findElement(*it, user.get(), true), false));
				}
			} else {
				size_t i = 0;
				for (vector<string>::const_iterator it = jobRequest->elementsName->begin(); it != jobRequest->elementsName->end(); ++it, i++) {
					Element::Type eType = jobRequest->types ? elementTypes[i] : elementType;
					Element *e;
					bool created = false;
					try {
						e = dimension->findElementByName(*it, user.get(), true);
					} catch (ParameterException &) {
						e = dimension->addElement(server, database, NO_IDENTIFIER, *it, eType, user, true);
						created = true;
					}
					elemsToReplace.push_back(pair<Element *, bool> (e, created));
				}
			}

			IdentifiersType elemsToDeleteFromCubes;
			size_t i = 0;
			for (vector<pair<Element *, bool> >::iterator it = elemsToReplace.begin(); it != elemsToReplace.end(); it++, i++) {
				Element::Type eType = jobRequest->types ? elementTypes[i] : elementType;
				Element *e = it->first;
				Element::Type et = e->getElementType();
				if (!it->second) {
					if (et != eType) {
						dimension->changeElementType(server, database, e, eType, user, false, &disabledRules, &elemsToDeleteFromCubes, true);
					}
				}
				if (eType == Element::CONSOLIDATED) {
					set<IdentifierType> keep;
					IdentifiersWeightType add;

					const IdentifiersType &children = jobRequest->children->at(i);
					for (size_t j = 0; j < children.size(); j++) {
						if (!it->second) {
							keep.insert(children[j]);
						}
						if (jobRequest->weights) {
							add.push_back(IdentifierWeightType(children[j], jobRequest->weights->at(i).at(j)));
						} else {
							add.push_back(IdentifierWeightType(children[j], 1.0));
						}
					}

					if (!it->second) {
						dimension->removeChildrenNotIn(server, database, user, e, &keep, &disabledRules, true);
					}
					dimension->addChildren(server, database, e, &add, user, &disabledRules, false, false, true, &elemsToDeleteFromCubes);
				}
			}

			if (!elemsToDeleteFromCubes.empty()) {
				dimension->removeElementsFromCubes(server, database, user, elemsToDeleteFromCubes, &disabledRules, Dimension::DEL_NUM, true);
			}
			dimension->updateElementsInfo();

			if (journal) {
				journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENTS_BULK_STOP);
				journal->appendInteger(dimension->getId());
				journal->nextLine();
			}

			for (CubeRulesArray::iterator it = disabledRules.begin(); it != disabledRules.end(); it++) {
				if (it->first) {
					it->first->activateRules(server, database, it->second, ACTIVE, PUser(), NULL, false, false);
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
