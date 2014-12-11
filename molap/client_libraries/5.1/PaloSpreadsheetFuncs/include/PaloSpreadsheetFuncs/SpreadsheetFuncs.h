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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#ifndef SPREADSHEET_FUNCS_H
#define SPREADSHEET_FUNCS_H

#include <PaloSpreadsheetFuncs/SpreadsheetFuncsBase.h>
#include <PaloSpreadsheetFuncs/GenericCell.h>
#include <PaloSpreadsheetFuncs/GenericContext.h>

#include "GenericArgumentArray.h"

namespace Palo {
namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief SpreadsheetFuncs.
 *
 *  This is the main class of this project.
 */
class SpreadsheetFuncs : public SpreadsheetFuncsBase {
private:
	explicit SpreadsheetFuncs(SpreadsheetFuncs& other);
protected:
	SpreadsheetFuncs();
public:
	virtual ~SpreadsheetFuncs();

	void FPaloInit(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDisconnect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRegisterServer(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloPing(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetSvs(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloServerInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloLicenseInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloChangePassword(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloChangeUserPassword(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloCubeClear(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionClear(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloRootAddDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRootDeleteDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRootSaveDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRootUnloadDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseAddCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseAddDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseDeleteCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseDeleteDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseLoadCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseUnloadCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloDimensionTopElementsCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementListConsolidationElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionSimpleFlatListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionSimpleTopListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionSimpleChildrenListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionReducedFlatListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionReducedTopListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionReducedChildrenListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionListElements2(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionListCubes(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeListDimensions(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseListCubes(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseListDimensions(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDatabaseListDimensionsExt(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRootListDatabases(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloRootListDatabasesExt(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloDatabaseInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloDatabaseRenameDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRename(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloSetdataA(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetdataAIf(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetdata(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetdataIf(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetdataBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataA(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAV(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAT(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataATC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdata(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataT(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataTC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataV(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCellCopy(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGoalSeek(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAggregationSum(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAggregationAvg(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAggregationCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAggregationMax(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetdataAggregationMin(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloElementCreateBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementAdd(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementDeleteBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementMove(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementMoveBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementRename(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementUpdate(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloElementChildcount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementChildname(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementFirst(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementIndex(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementIsChild(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementLevel(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementIndent(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementParentcount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementParentname(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementPrev(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementNext(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementSibling(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloDimensionMaxLevel(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementType(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementWeight(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementListParents(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementListDescendants(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementListAncestors(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementListSiblings(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloElementAlias(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloEventLockBegin(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloEventLockEnd(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloStartCacheCollect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloEndCacheCollect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloCubeRuleCreate(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRuleModify(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRuleParse(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRulesMove(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRuleDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRulesDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRules(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloCubeConvert(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCellDrillTrough(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloActivateLicense(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSetClientDescription(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloSVSRestart(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSVSInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloGetdataExport(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloSubset(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetSize(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloSubsetBasicFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetTextFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetSortingFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetAliasFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetStructuralFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloSubsetDataFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloSubcube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloRemoveConnection(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloGetElementId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetElementName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetDimensionId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetDimensionName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetCubeId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetCubeName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetUserForSID(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloConnectionUser(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetGroupsForSID(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloGetGroups(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	inline void FPaloExpandTypeSelf(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		retval.set(ExpandSelf);
	}

	inline void FPaloExpandTypeChildren(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		retval.set(ExpandChildren);
	}

	inline void FPaloExpandTypeLeaves(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		retval.set(ExpandLeaves);
	}

	inline void FPaloCoordinatesToArray(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		arg.collapseToArray(0);
		retval.set(arg[0].getStringArray());
	}

	inline void FPaloExpandTypesToArray(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		arg.collapseToArray(0);
		retval.set(arg[0].getIntArray());
	}

	void FPaloViewSubsetDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAxisDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAreaDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAxisGet(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAxisGetIndex(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAreaGet(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewAxisGetSize(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloViewDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

	void FPaloCubeLock(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeLocks(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeRollback(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	void FPaloCubeCommit(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);

protected:
	SubsetResults FPaloSubset(GenericContext& opts, GenericArgumentArray& arg);

	/*! You may override this function if you have better hash-keys than the PALO coordinate strings
	 */
	virtual CellValue _FPaloGetdataC(GenericCell& server, GenericCell& database, GenericCell& cube, GenericCell& path, const CellValue* * const ptr = 0);

private:
	class GetdataHelper;
	void FPaloGetdataAggregation(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg, AggregationType aggregationtype);
	void FPaloParseSubsetParams(size_t start, GenericArgumentArray& arg, std::vector<jedox::palo::BasicFilterSettings> &basics, jedox::palo::TextFilterSettings &text, jedox::palo::SortingFilterSettings &sorting, jedox::palo::AliasFilterSettings &alias, jedox::palo::FieldFilterSettings &field, std::vector<jedox::palo::StructuralFilterSettings> &structurals, std::vector<jedox::palo::DataFilterSettings> &datas);
	void FPaloGetdataACTIntern(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg, bool collapse, bool T);
};
}
}
#endif
