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

#ifndef PALO_DISPATCHER_PALO_BROWSER_JOB_H
#define PALO_DISPATCHER_PALO_BROWSER_JOB_H 1

#include "palo.h"

#include "PaloDispatcher/DirectPaloJob.h"
#include "PaloJobs/AreaJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base class for palo server browser
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloBrowserJob : public AreaJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloBrowserJob(PaloJobRequest* jobRequest) :
		AreaJob(jobRequest), templatePath(jobRequest->browserPath), contentType("text/html;charset=utf-8") {
	}

	PaloBrowserJob(PaloJobRequest* jobRequest, string contentType) :
		AreaJob(jobRequest), templatePath(jobRequest->browserPath), contentType(contentType) {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return WRITE_JOB;
	}

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates html message
	////////////////////////////////////////////////////////////////////////////////

	string createHtmlMessage(const string& type, const string& message);

	void generateResult(const StringBuffer&);

protected:
	string templatePath;
	string contentType;

};
}

#endif
