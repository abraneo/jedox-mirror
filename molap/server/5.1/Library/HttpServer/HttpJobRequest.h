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

#ifndef HTTP_SERVER_HTTP_JOB_REQUEST_H
#define HTTP_SERVER_HTTP_JOB_REQUEST_H 1

#include "palo.h"

#include "Dispatcher/JobRequest.h"

namespace palo {
class HttpResponse;

////////////////////////////////////////////////////////////////////////////////
/// @brief job request
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpJobRequest : public JobRequest {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a job with a name
	////////////////////////////////////////////////////////////////////////////////

	HttpJobRequest(const string &name) :
		JobRequest(name) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes the job
	////////////////////////////////////////////////////////////////////////////////

	virtual ~HttpJobRequest() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get the result of the job request
	////////////////////////////////////////////////////////////////////////////////

	virtual HttpResponse* getResponse() = 0;

};
}

#endif
