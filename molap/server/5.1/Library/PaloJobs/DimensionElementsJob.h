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

#ifndef PALO_JOBS_DIMENSION_ELEMENTS_JOB_H
#define PALO_JOBS_DIMENSION_ELEMENTS_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief dimension elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DimensionElementsJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new DimensionElementsJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	DimensionElementsJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets job type
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief start working
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		ElementsType elements;
		uint64_t hiddenCount = 0;
		uint64_t *pHiddenCount = 0;

		findDimension(false);
		dimension->checkElementAccessRight(user.get(), database, RIGHT_READ);

		if (jobRequest->parent == ALL_IDENTIFIERS) {
			if (jobRequest->mode == 1) {
				pHiddenCount = &hiddenCount;
			}
			// no parent filter - sorted by Id - old version
			elements = dimension->getElements(user, false, pHiddenCount);
		} else {
			Element *parentElem = 0;

			if (jobRequest->parent != NO_IDENTIFIER) {
				parentElem = dimension->findElement(jobRequest->parent, user.get(), false);
				if (!parentElem) {
					throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element not found", PaloRequestHandler::ID_ELEMENT, "");
				}
			}
			// parent restricted or limited count - new version
			ElementsWeightType ew = dimension->getChildren(user, parentElem);
			// convert to ElementsType
			for (ElementsWeightType::const_iterator it = ew.begin(); it != ew.end(); ++it) {
				elements.push_back(it->first);
			}
		}
		generateElementsResponse(dimension, &elements, pHiddenCount);
	}
};

}

#endif
