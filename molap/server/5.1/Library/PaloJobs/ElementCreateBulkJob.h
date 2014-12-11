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
 * \author Marko Stijak, Banja Luka, Bosnia and Herzegovina
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_JOBS_ELEMENT_CREATE_BULK_JOB_H
#define PALO_JOBS_ELEMENT_CREATE_BULK_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief element create bulk
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ElementCreateBulkJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest)
	{
		return new ElementCreateBulkJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ElementCreateBulkJob(PaloJobRequest* jobRequest) :
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

			PJournalMem journal = database->getJournal();
			if (journal) {
				journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENTS_BULK_START);
				journal->appendInteger(dimension->getId());
				journal->nextLine();
			}

			ElementsType createdElems;
			if (jobRequest->types != 0) {
				computeMultipleType(createdElems);
			} else {
				computeSingleType(createdElems);
			}
			dimension->updateElementsInfo();

			if (journal) {
				journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENTS_BULK_STOP);
				journal->appendInteger(dimension->getId());
				journal->nextLine();
			}

			if (!session->isWorker()) {
				for (ElementsType::iterator it = createdElems.begin(); it != createdElems.end(); ++it) {
					dimension->addElementEvent(server, database, *it);
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
			throw CommitException(ErrorException::ERROR_COMMIT_CANTCOMMIT, "Can't create new elements.");
		}
		generateOkResponse(dimension);
	}

private:

	// /////////////////////////////////////////////////////////////////////////////
	// create elements of same type
	// /////////////////////////////////////////////////////////////////////////////

	void computeSingleType(ElementsType &createdElems)
	{

		// example url:
		// element/create_bulk?sid=NvUl&database=3&dimension=11&name_elements=abc,def&type=4&name_children=a,b,c:d,e,f,r&weights=1,1,2:1,2,1
		Element::Type elementType = elementTypeByIdentifier(jobRequest->type);

		if (!jobRequest->elementsName) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing element names", PaloRequestHandler::NAME_ELEMENTS, "");
		}

		vector<IdentifiersWeightType> children;

		if (elementType == Element::CONSOLIDATED) {
			bool hasWeight = false;

			if (jobRequest->weights && jobRequest->weights->size() > 0) {
				hasWeight = true;
			}

			vector<IdentifiersType> childrenElements;

			if (jobRequest->children) {
				if (jobRequest->children->size() < jobRequest->elementsName->size()) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
				}
				for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {
					IdentifiersType ch;
					for (size_t j = 0; j < jobRequest->children->at(i).size(); j++) {
						Element* e = dimension->findElement(jobRequest->children->at(i).at(j), user.get(), true);
						ch.push_back(e->getIdentifier());
					}
					childrenElements.push_back(ch);
				}
			} else if (jobRequest->childrenName) {
				if (jobRequest->childrenName->size() < jobRequest->elementsName->size()) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
				}
				for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {
					IdentifiersType ch;
					for (size_t j = 0; j < jobRequest->childrenName->at(i).size(); j++) {
						Element* e = dimension->findElementByName(jobRequest->childrenName->at(i).at(j), user.get(), true);
						ch.push_back(e->getIdentifier());
					}
					childrenElements.push_back(ch);
				}
			} else {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
			}

			if (hasWeight) {
				if (childrenElements.size() > jobRequest->weights->size()) {
					throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
				}
			}

			for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {

				if (hasWeight) {
					if (childrenElements.at(i).size() > jobRequest->weights->at(i).size()) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
					}
				}

				IdentifiersWeightType elementsWeightTypes;

				for (size_t j = 0; j < childrenElements.at(i).size(); j++) {
					if (hasWeight) {
						elementsWeightTypes.push_back(IdentifierWeightType(childrenElements.at(i).at(j), jobRequest->weights->at(i).at(j)));
					} else {
						elementsWeightTypes.push_back(IdentifierWeightType(childrenElements.at(i).at(j), 1.0));
					}
				}

				children.push_back(elementsWeightTypes);
			}

		}

		//everything seems OK, now create elements
		for (size_t i = 0; i < jobRequest->elementsName->size(); ++i) {
			Element* element = dimension->addElement(server, database, NO_IDENTIFIER, jobRequest->elementsName->at(i), elementType, user, true);
			createdElems.push_back(element);
			if (elementType == Element::CONSOLIDATED) {
				dimension->addChildren(server, database, element, &children[i], user, NULL, false, false, true, NULL);
			}
		}
	}

	// /////////////////////////////////////////////////////////////////////////////
	// create elements of different types
	// /////////////////////////////////////////////////////////////////////////////

	void computeMultipleType(ElementsType &createdElems)
	{

		// example url:
		// element/create_bulk?sid=NvUl&database=3&dimension=11&name_elements=abc,xyz,def&types=4,1,4&name_children=a,b,c::d,e,f&weights=1,1,2::1,2,1

		if (!jobRequest->elementsName) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing element names", PaloRequestHandler::NAME_ELEMENTS, "");
		}

		if (jobRequest->elementsName->size() != jobRequest->types->size()) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing types", PaloRequestHandler::ID_TYPES, "expecting " + StringUtils::convertToString((uint32_t)jobRequest->elementsName->size()));
		}

		vector<Element::Type> elementTypes;

		for (vector<uint32_t>::const_iterator i = jobRequest->types->begin(); i != jobRequest->types->end(); ++i) {
			Element::Type elementType = elementTypeByIdentifier(*i);

			elementTypes.push_back(elementType);
		}

		// check for children and weights
		bool hasWeight = jobRequest->weights && jobRequest->weights->size();

		vector<vector<string> > childrenElements;

		// children by identifier
		if (jobRequest->children) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "names of children are required", PaloRequestHandler::NAME_CHILDREN, "");
		} else if (jobRequest->childrenName) {
			// children by name
			if (jobRequest->childrenName->size() < jobRequest->elementsName->size()) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
			}

			for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {
				vector<string> ch;

				for (size_t j = 0; j < jobRequest->childrenName->at(i).size(); j++) {
					ch.push_back(jobRequest->childrenName->at(i).at(j));
				}

				childrenElements.push_back(ch);
			}
		} else {
			// no children
			if (hasWeight) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing children", PaloRequestHandler::ID_CHILDREN, "");
			}
		}

		// check weights
		if (hasWeight) {
			if (childrenElements.size() > jobRequest->weights->size()) {
				throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
			}
		}

		// everything seems ok, now create elements
		createdElems.reserve(jobRequest->elementsName->size());

		for (size_t i = 0; i < jobRequest->elementsName->size(); ++i) {
			Element::Type elementType = elementTypes.at(i);
			Element* element = dimension->addElement(server, database, NO_IDENTIFIER, jobRequest->elementsName->at(i), elementType, user, true);

			createdElems.push_back(element); // save to store pointer to newly created element
		}

		// now create the children
		for (size_t i = 0; i < jobRequest->elementsName->size(); i++) {
			if (elementTypes[i] == Element::CONSOLIDATED) {
				// check weight list
				if (hasWeight) {
					if (childrenElements.at(i).size() > jobRequest->weights->at(i).size()) {
						throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing weight", PaloRequestHandler::WEIGHTS, "");
					}
				}

				// construct elements and weights of children
				IdentifiersWeightType elementsWeightTypes;

				for (size_t j = 0; j < childrenElements.at(i).size(); j++) {
					Element* e = dimension->findElementByName(childrenElements.at(i).at(j), user.get(), true);

					if (hasWeight) {
						elementsWeightTypes.push_back(IdentifierWeightType(e->getIdentifier(), jobRequest->weights->at(i).at(j)));
					} else {
						elementsWeightTypes.push_back(IdentifierWeightType(e->getIdentifier(), 1.0));
					}
				}

				// and add children
				dimension->addChildren(server, database, createdElems.at(i), &elementsWeightTypes, user, NULL, false, false, true, NULL);
			} else if (childrenElements.size() && (childrenElements.at(i).size() != 1 || childrenElements.at(i).at(0).size())) {
				// childrenElements.at(i) contains an empty string when for the i-th element is 'name_children' empty, i.e. '::'
				throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_TYPE, "expecting no children for non-consolidated element", "element", createdElems.at(i)->getName(dimension->getElemNamesVector()));
			}
		}
	}
};

}

#endif
