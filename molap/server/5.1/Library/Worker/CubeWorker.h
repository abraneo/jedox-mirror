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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef WORKER_CUBE_WORKER_H
#define WORKER_CUBE_WORKER_H 1

#include "palo.h"

#include "Worker/Worker.h"
#include "Engine/EngineBase.h"

namespace palo {
class Cube;

////////////////////////////////////////////////////////////////////////////////
/// @brief cube worker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS CubeWorker : public Worker {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get for use-flag
	////////////////////////////////////////////////////////////////////////////////

	static bool useCubeWorker();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief setter for use-flag
	////////////////////////////////////////////////////////////////////////////////

	static void setUseCubeWorker(bool);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	CubeWorker(const string&, bool isInvestigator);

	virtual ~CubeWorker();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a double value
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus setCellValue(const string& area, const string& session, const IdentifiersType& path, double value, SplashMode splashMode, bool addValue);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a double value
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus setCellValue(const string& area, const string& session, const IdentifiersType& path, const string& value, SplashMode splashMode, bool addValue);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief notifies worker about shutdown
	////////////////////////////////////////////////////////////////////////////////

	ResultStatus notifyShutdown();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool start(PDatabase db, PCube cube);
	bool start(); // for investigator worker
	ResultStatus getCubeAreas(PDatabase db, PCube cube, vector<string> &areaIds, vector<PArea> &areas);

private:
	ResultStatus defineCubeAreas(PCube cube);
	ResultStatus notifyAreaBuildOk();
	void notifyAreaBuildError(const string &id1, const string &id2);
	string buildPathString(const IdentifiersType *path);
	ResultStatus readAreaLines(const vector<string> &result, vector<string> &areaIds, vector<PArea> &areas);
	bool computeArea(vector<string> &answer, string &areaId, PArea &area);
	bool checkAreas(vector<string> &areaIds, vector<PArea> &areas);
	bool isOverlapping(CPArea area1, CPArea area2);

private:
	static bool useWorkers;

private:
	bool shutdownInProgress;

	string message;
	IdentifierType dbid;
	IdentifierType cubeid;
	bool isInvestigator; // special worker to check active event areas for all cubes
};

}

#endif
