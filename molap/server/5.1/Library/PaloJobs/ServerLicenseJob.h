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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef SERVERLICENSEJOB_H_
#define SERVERLICENSEJOB_H_

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////

/// @brief server info job
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ServerLicenseJob : public DirectPaloJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new ServerLicenseJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ServerLicenseJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes job
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		PaloJob::initialize();

		return true;
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
		server = Context::getContext()->getServer();

		checkToken(server);

		response = new HttpResponse(HttpResponse::OK);

		setToken(server);

		StringBuffer& body = response->getBody();

		body.appendCsvString("");
		body.appendCsvString("");
		body.appendCsvString("");
		body.appendCsvString("");
		body.appendEol();
	}
};
}

#endif /* SERVERLICENSEJOB_H_ */
