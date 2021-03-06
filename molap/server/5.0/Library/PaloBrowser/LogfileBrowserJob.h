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
 * 
 *
 */

#ifndef PALO_BROWSER_LOGFILE_BROWSER_JOB_H
#define PALO_BROWSER_LOGFILE_BROWSER_JOB_H 1

#include "palo.h"

#include <iostream>

#include "PaloDispatcher/PaloBrowserJob.h"

#include "PaloDocumentation/LogfileDocumentation.h"
#include "PaloDocumentation/HtmlFormatter.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server browser
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS LogfileBrowserJob : public PaloBrowserJob {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief factory method
	////////////////////////////////////////////////////////////////////////////////

	static PaloJob* create(PaloJobRequest* jobRequest) {
		return new LogfileBrowserJob(jobRequest);
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	LogfileBrowserJob(PaloJobRequest *jobRequest) :
		PaloBrowserJob(jobRequest) {
		jobType = READ_JOB;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobType getType() {
		return jobType;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool initialize() {
		bool ok = PaloJob::initialize();

		if (!ok) {
			return false;
		}
		return true;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void compute() {
		string message;

		server = Context::getContext()->getServer();

		IdentifierType startLine = 0;

		server = Context::getContext()->getServer();
		database = server->getSystemDatabase();
		cube = database->findCubeByName(SystemDatabase::NAME_SERVER_LOG_CUBE, PUser(), true, false);

		const IdentifiersType* cubeDimensions = cube->getDimensions();
		vector<CPDimension> dimensions;
		for (IdentifiersType::const_iterator it = cubeDimensions->begin(); it != cubeDimensions->end(); ++it) {
			dimensions.push_back(database->lookupDimension(*it, false));
		}

		PCubeArea cubeArea(new CubeArea(database, cube, cubeDimensions->size()));
		PSet linesSet(new Set());
		linesSet->insertRange(startLine, startLine+999);
		cubeArea->insert(0, linesSet);

		PSet columnsSet(new Set());
		columnsSet->insertRange(0, sizeof(SystemDatabase::MESSAGE_ITEMS)/sizeof(SystemDatabase::MESSAGE_ITEMS[0])-1);
		cubeArea->insert(1, columnsSet);


		PCellStream logData = cube->calculateArea(cubeArea, CubeArea::BASE, NO_RULES, false, UNLIMITED_SORTED_PLAN);

		LogFileBrowserDocumentation lbd(logData, message);
		HtmlFormatter hf(templatePath + "/browser_logfile.tmpl");

		generateResult(hf.getDocumentation(&lbd));
	}

private:
	JobType jobType;
};
}

#endif
