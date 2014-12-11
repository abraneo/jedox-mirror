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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_USER_H
#define OLAP_USER_H 1

#include "palo.h"

#include "Thread/Mutex.h"
#include "Olap/CommitableList.h"

namespace palo {

class SERVER_CLASS UserList : public CommitableList {
public:
	UserList(const PIdHolder &newidh) : CommitableList(newidh) {}
	UserList() {}
	UserList(const UserList &l) : CommitableList(l) {}
	virtual PCommitableList createnew(const CommitableList& l) const {
		return PCommitableList(new UserList(dynamic_cast<const UserList&>(l)));
	}
};

class SERVER_CLASS User : public Commitable {
public:

	enum RightObject { //same order as SystemDatabase::ROLE
		userRight = 0,
		passwordRight = 1,
		groupRight = 2,
		databaseRight = 3,
		cubeRight = 4,
		dimensionRight = 5,
		elementRight = 6,
		cellDataRight = 7,
		rightsRight = 8,
		sysOpRight = 9,
		eventProcessorRight = 10,
		subSetViewRight = 11,
		userInfoRight = 12,
		ruleRight = 13,
		drillthroughRight = 20
	};

	struct RightSetting {
		RightSetting() : checkSepRight(false), checkCells(false) {}
		RightSetting(bool checkCells) : checkSepRight(false), checkCells(checkCells) {}

		bool checkSepRight;
		bool checkCells;
	};

	struct RoleDbCubeRight {
		RightsType roleRight;
		RightsType dbRight;
		RightsType cubeRight;
	};

	typedef pair<RightsType, RightsType> MinMaxRight;
	typedef map<IdentifierType, MinMaxRight> DbRightsMap;  // dbID, <minRight, maxRight>
	typedef map<pair<IdentifierType, IdentifierType>, MinMaxRight> RightMap;  // <dbID, cubeID>, <minRight, maxRight>
	typedef map<IdentifierType, uint32_t> CubeTokenMap; // dbID, cubeToken
	typedef map<pair<IdentifierType, IdentifierType>, RightsType> ElemRightsMap; // <groupId, elemId>, RightsType
	typedef boost::shared_ptr<ElemRightsMap> PElemRightsMap;

	struct DimRights {
		DimRights(): minRight(RIGHT_NONE), maxRight(RIGHT_NONE), dimToken(0), cubeToken(0) {}
		DimRights(const DimRights &dr) : elemRights(dr.elemRights), minRight(dr.minRight), maxRight(dr.maxRight), dimToken(dr.dimToken), cubeToken(dr.cubeToken) {}
		PElemRightsMap elemRights; // non-empty calculated rights from #_GROUP_DIMENSION_DATA_ cube
		RightsType minRight; // minimum from elemRights
		RightsType maxRight; // maximum from elemRights
		uint32_t dimToken;
		uint32_t cubeToken; // token of the dimension's #_GROUP_DIMENSION_DATA_ cube
	};
	typedef map<pair<IdentifierType, IdentifierType>, DimRights> DimRightsMap;  // <dbID, dimID>, DimRights

	User(const string& name, vector<string>* groups, bool isExternal);

	MinMaxRight getRDCDRight(IdentifierType dbId, IdentifierType cubeId) const;

	MinMaxRight getRoleCellDataRight() const;
	RightsType getRoleRight(RightObject object) const;
	void getAllRoleRights(vector<RightsType> &rights) const;
	bool checkRoleRight(CPSystemDatabase sysDb, set<IdentifierType>& userGroups, RightObject object, RightsType requiredRight) const;
	void checkRoleRight(RightObject object, RightsType requiredRight) const;

	MinMaxRight getDatabaseDataRight(IdentifierType dbId) const;
	bool checkDatabaseDataRight(CPDatabase db, set<IdentifierType>& userGroups, RightsType requiredRight) const;

	RightsType getRoleDbRight(RightObject object, CPDatabase db) const;
	void checkRoleDbRight(RightObject object, CPDatabase db, RightsType requiredRight) const;

	MinMaxRight getCubeDataRight(CPDatabase db, IdentifierType cubeId) const;
	bool checkCubeDataRight(CPDatabase db, CPCube cube, set<IdentifierType>& userGroups, RightsType requiredRight) const;

	MinMaxRight computeDRight(IdentifierType dbId, CPCube cube) const;
	const DimRights *getDimRights(IdentifierType dbId, IdentifierType dimId) const;

	bool checkDimsAndCells(CPDatabase db, CPCube cube, set<IdentifierType> &userGroups, CPCubeArea area, bool checkCells, RightsType requiredRight, bool *defaultUsed) const;
	void checkAreaRightsComplete(CPDatabase db, CPCube cube, CPCubeArea area, RightSetting& rs, bool isZero, RightsType requiredRight, bool *defaultUsed) const;

	bool checkElementRight(ElemRightsMap *erm, const IdentifiersType *userGroups, IdentifierType elemId, RightsType requiredRight) const;
	bool checkElementRight(IdentifierType dbId, IdentifierType dimId, IdentifierType elemId, RightsType requiredRight) const;

	void fillRights(vector<RoleDbCubeRight> &vRights, RightObject object, CPDatabase db, CPCube cube) const;
	void getDatabaseDataRight(vector<RoleDbCubeRight> &vRights, CPDatabase db) const;
	RightsType getElementRight(IdentifierType dbId, IdentifierType dimId, IdentifierType elemId, vector<RoleDbCubeRight> &vRights, bool checkRole) const;
	RightsType getCellRight(CPDatabase db, CPCube cube, const IdentifiersType &key, vector<RoleDbCubeRight> &vRights, bool *defaultUsed) const;

	bool canLogin() const;
	bool isExternalUser() const;
	const IdentifiersType& getUserGroups() const;
	void getUserGroupsCopy(set<IdentifierType>& gr) const;
	size_t getGroupCount() const {return groups.size();}
	bool refreshAll();
	void refreshRights();

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	static void updateGlobalDatabaseToken(PServer server, PDatabase db);
	static string rightsTypeToString(RightsType rt);
	static RightsType stringToRightsType(const string& str);
	static bool checkCellDataRightCube(CPDatabase db, CPCube cube);
	static bool checkUser(const User *user) {return user ? !user->isAdmin : false;}
	static bool checkUser(CPUser user) {return checkUser(user.get());}
	static void checkRuleDatabaseRight(const User *user, IdentifierType targDb, IdentifierType sourDb);
	static void checkDatabaseDataRight(PUser user, IdentifierType dbId, RightsType requiredRight);

private:
	bool getRolesGroups(bool checkExists);
	bool getGroups(PSystemDatabase sysDb, bool checkExists);
	RightsType getRoleRightObject(CPSystemDatabase sysDb, CPCube roleRightObjectCube, const IdentifiersType& roles, Element* rightObject) const;

	void getRoleRights(vector<RoleDbCubeRight> &vRights, RightObject object) const;
	void getCubeDataRights(vector<RoleDbCubeRight> &vRights, CPDatabase db, CPCube cube, const IdentifiersType &vGroups, bool isComputed) const;

	void computeRights(CPDatabase db);
	void computeRoleCellDataRight(CPDatabase db, CPSystemDatabase sysDb);
	void computeDbDataRights(CPDatabase db, CPSystemDatabase sysDb);
	void computeCubeDataRights(CPDatabase db, bool sameGroups);
	void computeDimensionDataRights(CPDatabase db, CPSystemDatabase sysDb, bool sameGroups);
	void computeRDCDRights(CPDatabase db);

	bool isSameCubeToken(CubeTokenMap::iterator& it, uint32_t cubeToken) const;
	void updateCubeToken(CubeTokenMap::iterator& it, IdentifierType dbId, uint32_t cubeToken);

	bool isSameDimToken(DimRightsMap::iterator &it, uint32_t dimToken, uint32_t cubeToken) const;
	void updateDimRights(IdentifierType dbId, IdentifierType dimId, const DimRights &dRights);
	void updateDbRights(IdentifierType dbId, RightsType minRight, RightsType maxRight);
	RightsType getElementRight(ElemRightsMap *erm, IdentifierType groupId, IdentifierType elemId) const;

	static PCube getCellDataRightCube(CPDatabase db, CPCube cube);
	static RightsType computeDimensionDataRight(CPCube groupDimensionDataCube, IdentifierType groupId, CPDimension dimension, Element* element, RightsType defaultRight);
	static void updateRightMap(RightMap& rightMap, IdentifierType dbId, IdentifierType objId, RightsType minRight, RightsType maxRight);

	IdentifiersType groups;
	IdentifiersType roles;
	vector<IdentifiersType> groupRoles;

	MinMaxRight cellDataRights;	// aggregated max and min "cellDataRight" role rights of users groups/roles
	DbRightsMap dbRights; // dbID, <minRight, maxRight>, values from #_GROUP_DATABASE_DATA cube, otherwise Server::defaultDbRight
	CubeTokenMap cubeTokens; // token of #_GROUP_CUBE_DATA cube
	RightMap cubeRights; // <dbID, cubeID>, <minRight, maxRight>, values from #_GROUP_CUBE_DATA cube, otherwise RIGHT_EMPTY
	DimRightsMap dimRights; // <dbID, dimID>, DimRights
	RightMap RDCDRights; // <dbID, cubeID>, <minRights, maxRights>, RDCD means RoleDatabaseCubeDimensions

	// for external users:
	vector<string> groupNames;	// even non-existing groups names are saved, so rights are assigned in refreshAll - getGroups if group is created later
	bool isExternal;

	bool isAdmin;
};

ostream& operator<<(ostream& ostr, const User::DimRightsMap &drm);
ostream& operator<<(ostream& ostr, const User::DimRights &dr);

}

#endif
