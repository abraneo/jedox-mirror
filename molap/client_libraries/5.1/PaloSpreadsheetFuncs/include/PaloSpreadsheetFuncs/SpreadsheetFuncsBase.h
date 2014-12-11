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

#ifndef SPREADSHEET_FUNCS_BASE_H
#define SPREADSHEET_FUNCS_BASE_H

#include <memory>
#include <set>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Server.h>

#include "QueryCache.h"

#include <PaloSpreadsheetFuncs/QueryCache.h>

#include "StringArray.h"
#include "StringArrayArray.h"
#include "GenericCell.h"
#include "ElementListArray.h"
#include "DimensionElementType.h"
#include "ConsolidationElementArray.h"
#include "CubeInfo.h"
#include "GoalSeekType.h"
#include "AggregationTypes.h"
#include "CellValue.h"
#include "CellValueArray.h"
#include "DimensionElementInfoArray.h"
#include "DatabaseInfo.h"
#include "UserInfo.h"
#include "LockInfo.h"

#include "RuleInfo.h"

namespace Palo {
using namespace Types;

namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief SpreadsheetFuncsBase.
 *
 *  This is the main class of this project. It contains the implementations of most functions.
 *  You should prefer using the derived SpreadsheetFuncs.
 */
class SpreadsheetFuncsBase {
public:
	SpreadsheetFuncsBase();
	virtual ~SpreadsheetFuncsBase();

	void calculationBegin();
	void calculationEnd();

	std::string FPaloRegisterServer(const std::string& key, const std::string& hostname, unsigned short int port, const std::string& username, const std::string& password, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures);
	std::string FPaloRegisterServer(const std::string& key, const std::string& host, const unsigned int port, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures);
	std::string FPaloRegisterServer(const std::string& key, const std::string& hostname, unsigned short int port, const std::string& sid);
	void FPaloRemoveConnection(const std::string& key, bool force = false);

	bool FPaloPing(boost::shared_ptr<jedox::palo::Server> s);
	void FPaloSetSvs(boost::shared_ptr<jedox::palo::Server> s);
	unsigned int FPaloServerToken(boost::shared_ptr<jedox::palo::Server> s);
	ServerInfo FPaloServerInfo(boost::shared_ptr<jedox::palo::Server> s);
	LicenseInfo FPaloLicenseInfo(boost::shared_ptr<jedox::palo::Server> s);
	void FPaloChangePassword(boost::shared_ptr<jedox::palo::Server> s, const std::string& oldpassword, const std::string& newpassword);
	void FPaloChangeUserPassword(boost::shared_ptr<jedox::palo::Server> s, const std::string& userName, const std::string& newpassword);

	void FPaloCubeClear(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const ElementListArray& a);
	void FPaloCubeClear(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	void FPaloDimensionClear(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);

	void FPaloRootAddDatabase(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const jedox::palo::DATABASE_INFO::TYPE = jedox::palo::DATABASE_INFO::NORMAL, const std::string path = "");
	bool FPaloRootDeleteDatabase(boost::shared_ptr<jedox::palo::Server> s, const std::string& database);
	bool FPaloRootSaveDatabase(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string path = "", bool complete = false);
	bool FPaloRootUnloadDatabase(boost::shared_ptr<jedox::palo::Server> s, const std::string& database);
	void FPaloDatabaseAddCube(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& dimension_names);
	void FPaloDatabaseAddDimension(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	bool FPaloDatabaseDeleteCube(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	bool FPaloDatabaseDeleteDimension(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	bool FPaloDatabaseLoadCube(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	bool FPaloDatabaseUnloadCube(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);

	ConsolidationElementInfoArray FPaloElementListConsolidationElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);

	DimensionElementInfoSimpleArray FPaloElementListParents(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	DimensionElementInfoSimpleArray FPaloElementListSiblings(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	DimensionElementInfoSimpleArray FPaloElementListDescendants(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	DimensionElementInfoSimpleArray FPaloElementListAncestors(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);

	size_t FPaloDimensionTopElementsCount(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	std::unique_ptr<DimensionElementInfoSimpleArray> FPaloDimensionSimpleFlatListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoSimpleArray> FPaloDimensionSimpleTopListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoSimpleArray> FPaloDimensionSimpleChildrenListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long element_identifier, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoReducedArray> FPaloDimensionReducedFlatListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoReducedArray> FPaloDimensionReducedTopListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoReducedArray> FPaloDimensionReducedChildrenListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long element_identifier, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoArray> FPaloDimensionListElements2(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, bool filter);
	std::unique_ptr<DimensionElementInfoPermArray> FPaloDimensionListElements2Perm(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, bool filter, bool showPermission);
	DimensionElementInfoSimpleArray FPaloDimensionListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, bool filter);
	StringArray FPaloDimensionListElementsSimple(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	StringArray FPaloDimensionListCubes(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	StringArray FPaloCubeListDimensions(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	StringArray FPaloDatabaseListCubes(boost::shared_ptr<jedox::palo::Server> s, const std::string& database);
	StringArray FPaloDatabaseListCubes(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, jedox::palo::CUBE_INFO::TYPE type);
	CubeInfoArray FPaloDatabaseListCubesExt(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, jedox::palo::CUBE_INFO::TYPE type, bool showPermission);
	StringArray FPaloDatabaseListDimensions(boost::shared_ptr<jedox::palo::Server> s, const std::string& database);
	StringArray FPaloDatabaseListDimensions(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, jedox::palo::DIMENSION_INFO::TYPE type);
	DimensionInfoArray FPaloDatabaseListDimensionsExt(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, bool showNormal, bool showSystem, bool showAttribute, bool showUserInfo, bool showPermission);
	StringArray FPaloRootListDatabases(boost::shared_ptr<jedox::palo::Server> s, bool list_system_db, bool list_user_info_db, bool advanced);
	DatabaseInfoArray FPaloRootListDatabasesExt(boost::shared_ptr<jedox::palo::Server> s, bool list_system_db, bool list_user_info_db, bool showPermission);

	void FPaloDatabaseRenameDimension(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& old_name, const std::string& new_name);
	void FPaloCubeRename(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const std::string& name);
	CubeInfo FPaloCubeInfo(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, bool showPermission);
	StringArray FPaloDimensionInfo(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const bool showPermission);

	DatabaseInfo FPaloDatabaseInfo(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, bool showPermission);

	CellValue FPaloSetdata(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, const CellValue& value, jedox::palo::SPLASH_MODE splash, const StringArrayArray& LockingArea);
	bool FPaloSetdataBulk(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArrayArray& coordinates, const CellValueArray& values, jedox::palo::SPLASH_MODE splash);
	CellValueWithProperties FPaloGetdata(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, StringArray *&properties);
	CellValueWithProperties FPaloGetdataC(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, StringArray *&properties, const CellValue* * const ptr = NULL);
	CellValueArray FPaloGetdataV(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArrayArray& area);
	bool FPaloCellCopy(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& from, const StringArray& to, const CellValue& value, bool use_rules);
	bool FPaloGoalSeek(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, const double value, GoalSeekType type, StringArrayArray area);
	CellValueWithProperties FPaloGetdataAggregation(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& path, AggregationType aggregationtype, IntArray expandtypes);

	std::string FPaloGetUserForSID(boost::shared_ptr<jedox::palo::Server> s, const std::string& sid);
	UserInfo FPaloGetUserInfoForSID(boost::shared_ptr<jedox::palo::Server> s, const std::string& sid);
	std::string FPaloConnectionUser(boost::shared_ptr<jedox::palo::Server> s);
	StringArray FPaloGetGroupsForSID(boost::shared_ptr<jedox::palo::Server> s, const std::string& sid);
	StringArray FPaloGetGroups(boost::shared_ptr<jedox::palo::Server> s);

	std::string FPaloElementAdd(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, DimensionElementType type, const std::string& element, const std::string& parent, double cfactor, int clear);
	void FPaloElementCreateBulk(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const StringArray &elements, DimensionElementType type, const StringArrayArray children, const StringArrayArray weights);
	bool FPaloElementDelete(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	void FPaloElementDeleteBulk(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, std::vector<std::string> elements);
	void FPaloElementMove(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, unsigned int new_offset);
	void FPaloElementMoveBulk(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, std::vector<std::string> elements, std::vector<int> postions);
	void FPaloElementRename(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& old_name, const std::string& new_name);
	void FPaloElementUpdate(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, const DimensionElementType& type, const ConsolidationElementArray& children, bool append);
	void FPaloChildrenDelete(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, const StringArray& children);
	StringArray FPaloElementAlias(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& attribute, const std::string& alias_value, unsigned long idx);

	unsigned int FPaloElementChildcount(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloElementChildname(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, unsigned int child_offset);
	unsigned int FPaloElementCount(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	const std::string& FPaloElementFirst(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	unsigned int FPaloElementIndex(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	bool FPaloElementIsChild(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& parent, const std::string& child);
	unsigned int FPaloElementLevel(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	unsigned int FPaloElementIndent(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloElementName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, unsigned int offset);
	const std::string FPaloElementName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	CellValue FPaloElementName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, const std::string& attribute);
	unsigned int FPaloElementParentcount(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloElementParentname(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, unsigned int offset);
	const std::string& FPaloElementPrev(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloElementNext(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloElementSibling(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element, int sibling_offset);
	unsigned int FPaloDimensionMaxLevel(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	DimensionElementType FPaloElementType(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	double FPaloElementWeight(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& parent, const std::string& child);

	long FPaloGetElementId(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, const std::string& element);
	const std::string& FPaloGetElementName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long elementId);

	long FPaloGetDimensionId(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension);
	const std::string& FPaloGetDimensionName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, long dimensionId);

	long FPaloGetCubeId(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	const std::string& FPaloGetCubeName(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, long cubeId);

	void FPaloEventLockBegin(boost::shared_ptr<jedox::palo::Server> s, const std::string& source, const std::string& areaid);
	void FPaloEventLockEnd(boost::shared_ptr<jedox::palo::Server> s);

	void FPaloStartCacheCollect();
	/*! \return true if cache was not empty, data (and/or lock status) has changed */
	bool FPaloEndCacheCollect(bool check_locks = false, bool remove_obsolete = true);
	/*! \returns true if cache has been changed */
	bool FPaloCacheClearObsolete();

	RuleInfo FPaloCubeRuleCreate(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const std::string& definition, std::string extern_id = "", std::string comment = "", bool activate = true, double position = 0.0);
	RuleInfo FPaloCubeRuleModify(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, long int identifier, const std::string& definition, std::string extern_id = "", std::string comment = "", bool activate = true, double position = 0.0);
	std::string FPaloCubeRuleParse(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const std::string& definition);
	RuleInfoArray FPaloCubeRulesMove(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const IntArray &identifiers, double startPosition, double belowPosition);
	RuleInfoArray FPaloCubeRules(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube);
	bool FPaloCubeRuleDelete(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, long identifier);
	bool FPaloCubeRulesDelete(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const IntArray &identifiers);
	void FPaloCubeConvert(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, CubeInfo::CubeType cubetype);
	StringArrayArray FPaloCellDrillTrough(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, const StringArray& elements);
	void FPaloActivateLicense(boost::shared_ptr<jedox::palo::Server> s, const std::string &licenseKey, const std::string &activationCode);
	void FPaloSetClientDescription(const std::string &clientDescription);

	void FPaloSVSRestart(boost::shared_ptr<jedox::palo::Server> s, int mode);
	StringArray FPaloSVSInfo(boost::shared_ptr<jedox::palo::Server> s);

	void FPaloAuthSID(const std::string& host, unsigned short port, const std::string& sid);

	struct GetdataExportResultRow {
		GetdataExportResultRow(jedox::palo::Database& db, jedox::palo::Cube& c, const jedox::palo::CELL_VALUE_EXPORTED& cve);

		StringArray path;
		CellValue val;
	};
	typedef std::vector<GetdataExportResultRow> GetdataExportResult;

	GetdataExportResult FPaloGetdataExport(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, bool ignore_empty, bool base_only, const CellValue& lower_limit, const CellValue& upper_limit, const std::string& lower_operator, const std::string& upper_operator, const std::string& bool_operator, unsigned int max_rows, const StringArray& first_path, const ElementListArray& area, unsigned short use_rules);
	GetdataExportResult FPaloGetdataExport(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, bool ignore_empty, bool base_only, const CellValue& lower_limit, const CellValue& upper_limit, const std::string& lower_operator, const std::string& upper_operator, const std::string& bool_operator, unsigned int max_rows, const StringArray& first_path, unsigned short use_rules);
	GetdataExportResult FPaloGetdataExport(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, bool ignore_empty, bool base_only, const std::string& condition, unsigned int max_rows, const StringArray& first_path, const ElementListArray& area, unsigned short use_rules);
	GetdataExportResult FPaloGetdataExport(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& cube, bool ignore_empty, bool base_only, const std::string& condition, unsigned int max_rows, const StringArray& first_path, unsigned short use_rules);

	SubsetResults FPaloSubset(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, int indent, const jedox::palo::BasicFilterSettings& basic, const jedox::palo::TextFilterSettings& text, const jedox::palo::SortingFilterSettings& sorting, const jedox::palo::AliasFilterSettings& alias, const jedox::palo::FieldFilterSettings& field, const jedox::palo::StructuralFilterSettings& structural, const jedox::palo::DataFilterSettings& data);

	static void ChangeToNewStartIndex();
	static void InitSSL(std::string trustFile);

	std::string FPaloViewSubsetDefinition(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &dimension, int indent, const std::vector<jedox::palo::BasicFilterSettings> &basic, const jedox::palo::TextFilterSettings &text, const jedox::palo::SortingFilterSettings &sorting, const jedox::palo::AliasFilterSettings &alias, const jedox::palo::FieldFilterSettings &field, const std::vector<jedox::palo::StructuralFilterSettings> &structural, const std::vector<jedox::palo::DataFilterSettings> &data);
	std::string FPaloViewAxisDefinition(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, int axisId, const jedox::palo::AxisSubsets &as);
	std::string FPaloViewAreaDefinition(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube, const StringArray &axes, const StringArray &properties);
	SubsetResult FPaloViewAxisGet(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, int axisId, size_t subsetPos, size_t elemPos);
	jedox::palo::AxisElement FPaloViewAxisGet(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos);
	CellValueWithProperties FPaloViewAreaGet(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, const StringArray &coord, StringArray *&properties);
	size_t FPaloViewAxisGetSize(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, int axisId);
	std::vector<jedox::palo::AxisElement> FPaloViewAxisGetTop(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, size_t start, size_t limit);
	std::vector<jedox::palo::AxisElement> FPaloViewAxisGetChildren(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, const std::string &element);

	LockInfo FPaloCubeLock(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube, const std::vector<std::vector<std::string> > &coordinates, bool complete);
	std::vector<LockInfo> FPaloCubeLocks(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube);
	bool FPaloCubeRollback(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube, jedox::palo::IdentifierType lockId, long steps);
	bool FPaloCubeCommit(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube, jedox::palo::IdentifierType lockId);
	static bool isVirtualCube(const std::string &cube);

protected:
	QueryCache qc;

	typedef QueryCache::QueryCacheIndex UniqueDimensionIdx;
	std::map<UniqueDimensionIdx, bool> clearedDimensions;

	const std::string _CellValue2encString(const CellValue& cv);
	const std::string _compare_op2encString(const std::string& compare_op);
	const std::string _validate_bool_op(const std::string& compare_mode);
	double _string2Double(const std::string& s, bool& is_percentage);
	jedox::palo::CELL_VALUE _double2CELL_VALUE(double d);
	std::string permToString(jedox::palo::PERMISSION p);

private:
	static unsigned short backwards_startindex;
	static const std::string doublepoint, atsign, emptystring, prefixpropertycube;
	static const size_t lengthprefixpropertycube;

	inline static bool isPropertyCube(const std::string& cube)
	{
		return cube.compare(0, lengthprefixpropertycube, prefixpropertycube) == 0;
	}

	void FPaloElementListAncestorsHelper(std::set<DimensionElementInfoSimple>& vec, jedox::palo::Dimension& dim, const jedox::palo::ELEMENT_LIST& elements);
	void FPaloElementListDescendantsHelper(std::set<DimensionElementInfoSimple>& vec, jedox::palo::Dimension& dim, const jedox::palo::ELEMENT_LIST& elements);

	std::unique_ptr<DimensionElementInfoSimpleArray> FPaloDimensionSimpleListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long id, long start = 0, long limit = -1);
	std::unique_ptr<DimensionElementInfoReducedArray> FPaloDimensionReducedListElements(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, const std::string& dimension, long id, long start = 0, long limit = -1);

	void CellReplaceWrapper(boost::shared_ptr<jedox::palo::Server> s, const long dbid, const long cid, const StringArray& path, const jedox::palo::CELL_VALUE& value, jedox::palo::SPLASH_MODE splash, const StringArrayArray& LockingArea);
	void CellCopyWrapper(boost::shared_ptr<jedox::palo::Server> s, const long dbid, const long cid, const StringArray& from, const StringArray& to, const double* value, const StringArrayArray& LockingArea, bool useRules);
	bool parseCopyParams(const StringArray &tokens, double &d, std::string &coord_str, bool &is_like, bool &is_percentage, bool &use_rules, bool &is_add, jedox::palo::Cube::COPY_FUNCTION &func);
	void parsePath(boost::shared_ptr<jedox::palo::Server> s, const long dbid, const long cid, StringArray &result, const StringArray &path, const std::string &coord_str);
	void parseArea(boost::shared_ptr<jedox::palo::Server> s, const long dbid, const long cid, StringArrayArray &result, const StringArray &path, const std::string &coord_str);
	CubeInfo fillCubeInfo(boost::shared_ptr<jedox::palo::Server> s, const std::string& database, jedox::palo::CUBE_INFO ci);
};
}
}
#endif
