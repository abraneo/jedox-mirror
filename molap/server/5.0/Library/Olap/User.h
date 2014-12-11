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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
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

	typedef pair<RightsType, RightsType> RoleCubeRight;
	typedef pair<RightsType, RightsType> MinMaxRight;
	typedef map<IdentifierType, RightsType> MapElementRight;
	typedef boost::shared_ptr<MapElementRight> PMapElementRight;

	struct DimRights {
		DimRights(): minRight(RIGHT_NONE), maxRight(RIGHT_NONE), dimToken(0), cubeToken(0) {}
		DimRights(const DimRights &dr) : elemRights(dr.elemRights), minRight(dr.minRight), maxRight(dr.maxRight), dimToken(dr.dimToken), cubeToken(dr.cubeToken) {}
		PMapElementRight elemRights;
		RightsType minRight; // minimum from elemRights
		RightsType maxRight; // maximum from elemRights
		uint32_t dimToken;
		uint32_t cubeToken; // token of the dimension's #_GROUP_DIMENSION_DATA_ cube
	};

	typedef map<pair<IdentifierType, IdentifierType>, DimRights> DimRightsMap;  //<dbID, dimID>, DimRights

	User(const string& name, vector<string>* groups, bool isExternal);

	MinMaxRight getRCDDataRight(CPDatabase db, IdentifierType cubeId) const;

	MinMaxRight getRoleCellDataRight() const;
	RightsType getRoleRight(RightObject object) const;
	void getRoleRights(RightObject object, vector<RoleCubeRight>& rcRights) const;
	bool checkRoleRight(CPSystemDatabase sysDb, set<IdentifierType>& userGroups, RightObject object, RightsType requiredRight) const;
	void checkRoleRight(RightObject object, RightsType requiredRight) const;

	MinMaxRight getCubeDataRight(CPDatabase db, IdentifierType cubeId) const;
	void getCubeDataRights(CPDatabase db, CPCube cube, vector<RoleCubeRight>& rcRights, bool isComputed) const;
	bool checkCubeDataRight(CPDatabase db, CPCube cube, set<IdentifierType>& userGroups, RightsType requiredRight) const;

	bool checkGroupDimensionDataRight(CPDatabase db, CPCube groupDimensionDataCube, CPDimension dim, IdentifierType elemId, RightsType requiredRight) const;

	RightsType getDimensionDataRight(CPDatabase db, CPDimension dim, IdentifierType elemId, vector<RoleCubeRight>& rcRights) const;
	void computeDRight(IdentifierType dbId, CPCube cube, RightsType &rtMin, RightsType &rtMax) const;
	const DimRights *getDimRights(IdentifierType dbId, IdentifierType dimId) const;

	bool checkDimsAndCells(CPDatabase db, CPCube cube, set<IdentifierType>& userGroups, CPCubeArea area, bool checkDims, bool checkCells, RightsType requiredRight) const;
	void checkAreaRightsComplete(CPDatabase db, CPCube cube, CPCubeArea area, RightSetting& rs, bool isZero, RightsType requiredRight) const;
	RightsType getCellRight(CPDatabase db, CPCube cube, const IdentifiersType &key, vector<RoleCubeRight>& rcRights) const;

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
	static bool checkUser(User *user) {return user ? !user->isAdmin : false;}
	static bool checkUser(PUser user) {return checkUser(user.get());}

private:
	bool getRolesGroups(bool checkExists);
	bool getGroups(PSystemDatabase sysDb, bool checkExists);
	RightsType getRoleRightObject(CPSystemDatabase sysDb, CPCube roleRightObjectCube, const IdentifiersType& roles, Element* rightObject) const;

	RightsType getSystemDimensionDataRight(CPSystemDatabase sysDb, CPDimension dim) const;
	RightsType getUserInfoDimensionDataRight() const;
	RightsType getGroupDimensionDataRight(CPCube groupDimensionDataCube, CPDimension dim, IdentifierType groupId, IdentifierType elemId, RightsType defaultRight) const;

	void computeRights(CPDatabase db);
	void computeRoleCellDataRight(CPDatabase db, CPSystemDatabase sysDb);
	void computeCubeDataRights(CPDatabase db, CPSystemDatabase sysDb, bool sameGroups);
	void computeDimensionDataRights(CPDatabase db, CPSystemDatabase sysDb, bool sameGroups);
	void computeRCDRights(CPDatabase db);

	typedef map<pair<IdentifierType, IdentifierType>, MinMaxRight> RightMap;  //<dbID, cubeID>, <minRight, maxRight>
	typedef map<IdentifierType, uint32_t> CubeTokenMap; //dbID, cubeToken
	typedef map<pair<IdentifierType, IdentifierType>, RightsType> ElemRightsMap; //<groupId, elemId>, RightsType

	bool isSameCubeToken(CubeTokenMap::iterator& it, uint32_t cubeToken) const;
	void updateCubeToken(CubeTokenMap::iterator& it, IdentifierType dbId, uint32_t cubeToken);

	bool isSameDimToken(DimRightsMap::iterator &it, uint32_t dimToken, uint32_t cubeToken) const;
	void updateDimRights(IdentifierType dbId, IdentifierType dimId, const DimRights &dRights);

	static PCube getCellDataRightCube(CPDatabase db, CPCube cube);
	static RightsType computeDimensionDataRight(CPCube groupDimensionDataCube, IdentifierType groupId, CPDimension dimension, Element* element, RightsType defaultRight);
	static ElemRightsMap computeDimensionDataRight(CPDatabase db, CPCube groupDimensionDataCube, set<IdentifierType> &userGroups, CPDimension dim, CPSet self);
	static RightsType getDimensionDataRight(ElemRightsMap &erm, IdentifierType groupId, IdentifierType elemId, CPDimension dim, RightsType defaultRight);
	static void updateRightMap(RightMap& rightMap, IdentifierType dbId, IdentifierType objId, RightsType minRight, RightsType maxRight);
	static bool checkCell(size_t dimCount, IdentifierType groupId, const IdentifiersType &grKey, vector<pair<bool, bool> >& checkElems, vector<RightsType>& rtSingle, vector<ElemRightsMap>& rtMulti, RightsType requiredRight);

	IdentifiersType groups;
	IdentifiersType roles;
	vector<IdentifiersType> groupRoles;

	MinMaxRight cellDataRights;

	CubeTokenMap cubeTokens; //token of #_GROUP_CUBE_DATA cube
	RightMap cubeRights; //<dbID, cubeID>, <minRight, maxRight>, rights from #_GROUP_CUBE_DATA cube
	RightMap RCDRights; //<dbID, cubeID>, <minRights, maxRights>

	DimRightsMap dimRights;

	// for external users:
	vector<string> groupNames;	// even non-existing groups names are saved, so rights are assigned in refreshAll - getGroups if group is created later
	bool isExternal;

	bool isAdmin;
};

ostream& operator<<(ostream& ostr, const User::DimRightsMap &drm);
ostream& operator<<(ostream& ostr, const User::DimRights &dr);

}

#endif
