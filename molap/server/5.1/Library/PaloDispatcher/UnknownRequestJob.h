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

#ifndef PALO_DISPATCHER_UNKNOWN_REQUEST_JOB_H
#define PALO_DISPATCHER_UNKNOWN_REQUEST_JOB_H 1

#include "palo.h"

#include "Collections/StringUtils.h"
#include "Exceptions/ErrorException.h"
#include "HttpServer/HttpResponse.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server info job
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS UnknownRequestJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	UnknownRequestJob(PaloJobRequest* jobRequest) :
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
		response = new HttpResponse(HttpResponse::BAD);
		StringBuffer& body = response->getBody();

		body.appendCsvInteger((uint32_t)ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED);
		body.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED)));
		body.appendCsvString(StringUtils::escapeString("error in request"));
		body.appendEol();
	}

private:
};
}

#endif
