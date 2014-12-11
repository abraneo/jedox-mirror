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

#ifndef PALO_HTTP_SERVER_PALO_BROWSER_HANDLER_SID_H
#define PALO_HTTP_SERVER_PALO_BROWSER_HANDLER_SID_H 1

#include "palo.h"

#include "Exceptions/ParameterException.h"
#include "PaloHttpServer/PaloRequestHandler.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
class PaloJobRequest;

////////////////////////////////////////////////////////////////////////////////
/// @brief http request handler for palo objects requiring a sid
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloBrowserHandlerSid : public PaloRequestHandler {
public:

	////////////////////////////////////////////////////////////////////////////////
	// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloBrowserHandlerSid(const string& browserPath) :
		PaloRequestHandler(true), browserPath(browserPath) {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	HttpJobRequest * handleHttpRequest(HttpRequest* request, const HttpServerTask *task) {
		HttpJobRequest* jobRequest = generateJobRequest(request);
		PaloJobRequest* paloJobRequest = dynamic_cast<PaloJobRequest*>(jobRequest);

		if (paloJobRequest != 0 && !paloJobRequest->hasSession) {
			delete paloJobRequest;

			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "missing session identifier", SID, "");
		}

		if (paloJobRequest != 0) {
			paloJobRequest->browserPath = browserPath;
		}

		return jobRequest;
	}

private:
	const string browserPath;
};
}

#endif
