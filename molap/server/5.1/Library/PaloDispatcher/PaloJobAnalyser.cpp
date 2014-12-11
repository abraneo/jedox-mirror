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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloDispatcher/PaloJobAnalyser.h"

#include "PaloDispatcher/PaloJobRequest.h"
#include "PaloDispatcher/UnknownRequestJob.h"

#include "PaloBrowser/CubeBrowserJob.h"
#include "PaloBrowser/DatabaseBrowserJob.h"
#include "PaloBrowser/DimensionBrowserJob.h"
#include "PaloBrowser/ElementBrowserJob.h"
#include "PaloBrowser/ErrorCodeBrowserJob.h"
#include "PaloBrowser/RuleBrowserJob.h"
#include "PaloBrowser/CubeAnalyzerJob.h"
#include "PaloBrowser/ServerBrowserJob.h"
#include "PaloBrowser/TimeStatisticsBrowserJob.h"
#include "PaloBrowser/LogfileBrowserJob.h"
#include "PaloBrowser/SessionBrowserJob.h"
#include "PaloBrowser/JobBrowserJob.h"

#include "PaloJobs/CellAreaJob.h"
#include "PaloJobs/CellCopyJob.h"
#include "PaloJobs/CellDrillThroughJob.h"
#include "PaloJobs/CellExportJob.h"
#include "PaloJobs/CellGoalSeekJob.h"
#include "PaloJobs/CellReplaceBulkJob.h"
#include "PaloJobs/CellReplaceJob.h"
#include "PaloJobs/CellValueJob.h"
#include "PaloJobs/CellValuesJob.h"

#include "PaloJobs/CubeClearCacheJob.h"
#include "PaloJobs/CubeClearJob.h"
#include "PaloJobs/CubeCommitJob.h"
#include "PaloJobs/CubeCreateJob.h"
#include "PaloJobs/CubeConvertJob.h"
#include "PaloJobs/CubeDestroyJob.h"
#include "PaloJobs/CubeInfoJob.h"
#include "PaloJobs/CubeLoadJob.h"

#include "PaloJobs/CubeLockJob.h"
#include "PaloJobs/CubeLocksJob.h"
#include "PaloJobs/CubeRenameJob.h"
#include "PaloJobs/CubeRollbackJob.h"
#include "PaloJobs/CubeRulesJob.h"
#include "PaloJobs/CubeSaveJob.h"
#include "PaloJobs/CubeUnloadJob.h"

#include "PaloJobs/DatabaseCreateJob.h"
#include "PaloJobs/DatabaseCubesJob.h"
#include "PaloJobs/DatabaseDestroyJob.h"
#include "PaloJobs/DatabaseDimensionsJob.h"
#include "PaloJobs/DatabaseInfoJob.h"
#include "PaloJobs/DatabaseLoadJob.h"
#include "PaloJobs/DatabaseRenameJob.h"
#include "PaloJobs/DatabaseSaveJob.h"
#include "PaloJobs/DatabaseUnloadJob.h"

#include "PaloJobs/DimensionClearJob.h"
#include "PaloJobs/DimensionCreateJob.h"
#include "PaloJobs/DimensionCubesJob.h"
#include "PaloJobs/DimensionDestroyJob.h"
#include "PaloJobs/DimensionElementJob.h"
#include "PaloJobs/DimensionElementsJob.h"
#include "PaloJobs/DimensionInfoJob.h"
#include "PaloJobs/DimensionRenameJob.h"
#include "PaloJobs/DimensionDFilterJob.h"

#include "PaloJobs/ElementAppendJob.h"
#include "PaloJobs/ElementCreateJob.h"
#include "PaloJobs/ElementCreateBulkJob.h"
#include "PaloJobs/ElementDestroyJob.h"
#include "PaloJobs/ElementDestroyBulkJob.h"
#include "PaloJobs/ElementInfoJob.h"
#include "PaloJobs/ElementMoveJob.h"
#include "PaloJobs/ElementMoveBulkJob.h"
#include "PaloJobs/ElementRenameJob.h"
#include "PaloJobs/ElementReplaceJob.h"
#include "PaloJobs/ElementReplaceBulkJob.h"

#include "PaloJobs/EventBeginJob.h"
#include "PaloJobs/EventEndJob.h"

#include "PaloJobs/RuleCreateJob.h"
#include "PaloJobs/RuleDestroyJob.h"
#include "PaloJobs/RuleFunctionsJob.h"
#include "PaloJobs/RuleInfoJob.h"
#include "PaloJobs/RuleModifyJob.h"
#include "PaloJobs/RuleParseJob.h"

#include "PaloJobs/ServerDatabasesJob.h"
#include "PaloJobs/ServerInfoJob.h"
#include "PaloJobs/ServerLicenseJob.h"
#include "PaloJobs/ServerLicensesJob.h"
#include "PaloJobs/ServerLoadJob.h"
#include "PaloJobs/ServerMarkersJob.h"
#include "PaloJobs/ServerLoginJob.h"
#include "PaloJobs/ServerLogoutJob.h"
#include "PaloJobs/ServerSaveJob.h"
#include "PaloJobs/ServerShutdownJob.h"
#include "PaloJobs/ServerChangePasswordJob.h"
#include "PaloJobs/ServerUserInfoJob.h"
#include "PaloJobs/ServerActivateLicenseJob.h"

#include "PaloBrowser/StatisticsBrowserJob.h"
#include "PaloJobs/StatisticsClearJob.h"

#include "PaloJobs/SvsInfoJob.h"
#include "PaloJobs/SvsRestartJob.h"

#include "PaloJobs/ViewCalculateJob.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloJobAnalyser::PaloJobAnalyser()
{
	creators["/api/error.html"] = ErrorCodeBrowserJob::create;

	creators["/browser"] = ServerBrowserJob::create;
	creators["/browser/cube"] = CubeBrowserJob::create;
	creators["/browser/database"] = DatabaseBrowserJob::create;
	creators["/browser/dimension"] = DimensionBrowserJob::create;
	creators["/browser/element"] = ElementBrowserJob::create;
	creators["/browser/rule"] = RuleBrowserJob::create;
	creators["/browser/analyzer"] = CubeAnalyzerJob::create;
	creators["/browser/server"] = ServerBrowserJob::create;
	creators["/browser/statistics"] = TimeStatisticsBrowserJob::create;
	creators["/browser/logfile"] = LogfileBrowserJob::create;
	creators["/browser/sessions"] = SessionBrowserJob::create;
	creators["/browser/jobs"] = JobBrowserJob::create;

	creators["/cell/area"] = CellAreaJob::create;
	creators["/cell/copy"] = CellCopyJob::create;
	creators["/cell/drillthrough"] = CellDrillThroughJob::create;
	creators["/cell/export"] = CellExportJob::create;
	creators["/cell/goalseek"] = CellGoalSeekJob::create;
	creators["/cell/replace_bulk"] = CellReplaceBulkJob::create;
	creators["/cell/replace"] = CellReplaceJob::create;
	creators["/cell/value"] = CellValueJob::create;
	creators["/cell/values"] = CellValuesJob::create;

	creators["/cube/clear_cache"] = CubeClearCacheJob::create;
	creators["/cube/clear"] = CubeClearJob::create;
	creators["/cube/commit"] = CubeCommitJob::create;
	creators["/cube/create"] = CubeCreateJob::create;
	creators["/cube/convert"] = CubeConvertJob::create;
	creators["/cube/destroy"] = CubeDestroyJob::create;
	creators["/cube/info"] = CubeInfoJob::create;
	creators["/cube/load"] = CubeLoadJob::create;
	creators["/cube/lock"] = CubeLockJob::create;
	creators["/cube/locks"] = CubeLocksJob::create;
	creators["/cube/rename"] = CubeRenameJob::create;
	creators["/cube/rollback"] = CubeRollbackJob::create;
	creators["/cube/rules"] = CubeRulesJob::create;
	creators["/cube/save"] = CubeSaveJob::create;
	creators["/cube/unload"] = CubeUnloadJob::create;

	creators["/database/create"] = DatabaseCreateJob::create;
	creators["/database/cubes"] = DatabaseCubesJob::create;
	creators["/database/destroy"] = DatabaseDestroyJob::create;
	creators["/database/dimensions"] = DatabaseDimensionsJob::create;
	creators["/database/info"] = DatabaseInfoJob::create;
	creators["/database/load"] = DatabaseLoadJob::create;
	creators["/database/rename"] = DatabaseRenameJob::create;
	creators["/database/save"] = DatabaseSaveJob::create;
	creators["/database/unload"] = DatabaseUnloadJob::create;

	creators["/dimension/clear"] = DimensionClearJob::create;
	creators["/dimension/create"] = DimensionCreateJob::create;
	creators["/dimension/cubes"] = DimensionCubesJob::create;
	creators["/dimension/destroy"] = DimensionDestroyJob::create;
	creators["/dimension/element"] = DimensionElementJob::create;
	creators["/dimension/elements"] = DimensionElementsJob::create;
	creators["/dimension/info"] = DimensionInfoJob::create;
	creators["/dimension/rename"] = DimensionRenameJob::create;
	creators["/dimension/dfilter"] = DimensionDFilterJob::create;

	creators["/element/append"] = ElementAppendJob::create;
	creators["/element/create"] = ElementCreateJob::create;
	creators["/element/create_bulk"] = ElementCreateBulkJob::create;
	creators["/element/destroy"] = ElementDestroyJob::create;
	creators["/element/destroy_bulk"] = ElementDestroyBulkJob::create;
	creators["/element/info"] = ElementInfoJob::create;
	creators["/element/move"] = ElementMoveJob::create;
	creators["/element/move_bulk"] = ElementMoveBulkJob::create;
	creators["/element/rename"] = ElementRenameJob::create;
	creators["/element/replace"] = ElementReplaceJob::create;
	creators["/element/replace_bulk"] = ElementReplaceBulkJob::create;

	creators["/event/begin"] = EventBeginJob::create;
	creators["/event/end"] = EventEndJob::create;

	creators["/rule/create"] = RuleCreateJob::create;
	creators["/rule/destroy"] = RuleDestroyJob::create;
	creators["/rule/functions"] = RuleFunctionsJob::create;
	creators["/rule/info"] = RuleInfoJob::create;
	creators["/rule/modify"] = RuleModifyJob::create;
	creators["/rule/parse"] = RuleParseJob::create;

	creators["/server/info"] = ServerInfoJob::create;
	creators["/server/login"] = ServerLoginJob::create;
	creators["/server/databases"] = ServerDatabasesJob::create;
	creators["/server/load"] = ServerLoadJob::create;
	creators["/server/logout"] = ServerLogoutJob::create;
	creators["/server/save"] = ServerSaveJob::create;
	creators["/server/shutdown"] = ServerShutdownJob::create;
	creators["/server/markers"] = ServerMarkersJob::create;
	creators["/server/license"] = ServerLicenseJob::create;
	creators["/server/licenses"] = ServerLicensesJob::create;
	creators["/server/change_password"] = ServerChangePasswordJob::create;
	creators["/server/user_info"] = ServerUserInfoJob::create;
	creators["/server/activate_license"] = ServerActivateLicenseJob::create;

	creators["/statistics/clear"] = StatisticsClearJob::create;

	creators["/svs/info"] = SvsInfoJob::create;
	creators["/svs/restart"] = SvsRestartJob::create;

	creators["/view/calculate"] = ViewCalculateJob::create;
}

// /////////////////////////////////////////////////////////////////////////////
// JobAnalyser methods
// /////////////////////////////////////////////////////////////////////////////

Job* PaloJobAnalyser::analyse(JobRequest* jobRequest)
{
	PaloJobRequest * paloJobRequest = dynamic_cast<PaloJobRequest*>(jobRequest);

	if (paloJobRequest == 0) {
		cout << "Error in PaloJobAnalyser::analyse()" << endl;
		return new UnknownRequestJob(paloJobRequest);
	}

	return analysePaloJobRequest(paloJobRequest);
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

PaloJob* PaloJobAnalyser::analysePaloJobRequest(PaloJobRequest * jobRequest)
{
	const string& path = jobRequest->getName();
	map<string, create_fptr>::iterator iter = creators.find(path);

	if (iter == creators.end()) {
		cout << "Error in PaloJobAnalyser::analysePaloJobRequest() Path = " << path << endl;
		return new UnknownRequestJob(jobRequest);
	} else {
		create_fptr func = iter->second;

		return func(jobRequest);
	}
}
}
