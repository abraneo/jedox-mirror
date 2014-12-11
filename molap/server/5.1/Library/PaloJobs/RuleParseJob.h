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
 * 
 *
 */

#ifndef PALO_JOBS_RULE_PARSE_JOB_H
#define PALO_JOBS_RULE_PARSE_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "Parser/RuleParserDriver.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief rule parse
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RuleParseJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new RuleParseJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	RuleParseJob(PaloJobRequest* jobRequest) :
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

		string definition = "";
		if (jobRequest->definition) {
			definition = *(jobRequest->definition);
		}

		string functions = "";
		if (jobRequest->functions) {
			functions = *(jobRequest->functions);
		}

		bool validate = functions.empty();

		RuleNode * r = 0;

		// validate against cube data
		if (validate) {
			findCube(true, false);

			RuleParserDriver driver;

			driver.parse(definition);

			r = driver.getResult();

			if (r == 0) {
				throw ParameterException(ErrorException::ERROR_PARSING_RULE, driver.getErrorMessage(), PaloRequestHandler::DEFINITION, definition);
			}

			string errorMsg;
			bool ok = r->validate(server, database, cube, errorMsg);

			if (!ok) {
				delete r;

				throw ParameterException(ErrorException::ERROR_PARSING_RULE, errorMsg, PaloRequestHandler::DEFINITION, definition);
			}
		}

		// do no validate against cube data
		else {
			set<string> functionList;

			string all = functions;
			size_t pos1 = 0;
			boost::scoped_array<char> dummyBuffer;

			while (pos1 < all.size()) {
				string buffer = StringUtils::getNextElement(all, pos1, ',', false, dummyBuffer);

				if (!buffer.empty()) {
					functionList.insert(StringUtils::tolower(buffer));
				}
			}

			RuleParserDriver driver(&functionList);

			driver.parse(definition);

			r = driver.getResult();

			if (r == 0) {
				throw ParameterException(ErrorException::ERROR_PARSING_RULE, driver.getErrorMessage(), PaloRequestHandler::DEFINITION, definition);
			}

			string errorMsg;
			bool ok = r->validate(server, PDatabase(), PCube(), errorMsg);

			if (!ok) {
				delete r;

				throw ParameterException(ErrorException::ERROR_PARSING_RULE, errorMsg, PaloRequestHandler::DEFINITION, definition);
			}
		}

		// generate output
		response = new HttpResponse(HttpResponse::OK);
		StringBuffer& body = response->getBody();

		r->appendXmlRepresentation(&body, 0, !validate);
		delete r;

	}
};

}

#endif
