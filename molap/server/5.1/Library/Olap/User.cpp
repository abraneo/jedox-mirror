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

#include "Olap/User.h"

#include "Olap/SystemCube.h"
#include "Olap/SystemDatabase.h"
#include "Olap/SystemDimension.h"
#include "Olap/UserInfoDimension.h"
#include "Olap/Cube.h"
#include "Olap/Server.h"
#include "Engine/EngineBase.h"

namespace palo {

User::User(const string& name, vector<string>* groups, bool isExternal) :
	Commitable(name), isExternal(isExternal), isAdmin(false)
{
	if (isExternal && groups) {
		this->groupNames = *groups;
	}
	getRolesGroups(false);
}

User::MinMaxRight User::getRoleCellDataRight() const
{
	return cellDataRights;
}

RightsType User::getRoleRight(RightObject object) const
{
	PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
	PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
	PDimension rightObjectDim = sysDb->getRightsObjectDimension();

	return getRoleRightObject(sysDb, roleRightObjectCube, roles, rightObjectDim->findElementByName(SystemDatabase::ROLE[object], 0, false));
}

void User::getAllRoleRights(vector<RightsType> &rights) const
{
	PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
	PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
	PDimension rightObjectDim = sysDb->getRightsObjectDimension();

	PCubeArea area(new CubeArea(sysDb, roleRightObjectCube, 2));
	PSet s(new Set);
	for (IdentifiersType::const_iterator i = roles.begin(); i != roles.end(); ++i) {
		s->insert(*i);
	}
	area->insert(0, s);

	for (size_t i = 0; i < sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0]); i++) {
		s.reset(new Set);
		s->insert(rightObjectDim->findElementByName(SystemDatabase::ROLE[i], 0, false)->getIdentifier());
		area->insert(1, s);

		RightsType result = RIGHT_NONE;
		PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			RightsType rt = stringToRightsType(value);
			if (result < rt) {
				result = rt;
			}
			if (result == RIGHT_SPLASH) {
				break;
			}
		}

		rights.push_back(result);
	}
}

bool User::checkRoleRight(CPSystemDatabase sysDb, set<IdentifierType>& userGroups, RightObject object, RightsType requiredRight) const
{
	PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
	PDimension rightObjectDim = sysDb->getRightsObjectDimension();
	Element* elem = rightObjectDim->findElementByName(SystemDatabase::ROLE[object], 0, false);

	PCubeArea area(new CubeArea(sysDb, roleRightObjectCube, 2));
	PSet s(new Set);
	s->insert(elem->getIdentifier());
	area->insert(1, s);

	for (set<IdentifierType>::iterator git = userGroups.begin(); git != userGroups.end();) {
		s.reset(new Set);
		size_t i;
		for (i = 0; i < groups.size(); i++) {
			if (groups[i] == *git) {
				break;
			}
		}
		for (IdentifiersType::const_iterator rit = groupRoles[i].begin(); rit != groupRoles[i].end(); ++rit) {
			s->insert(*rit);
		}
		area->insert(0, s);

		bool enough = false;
		PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			RightsType rt = stringToRightsType(value);
			if (rt >= requiredRight) {
				enough = true;
				break;
			}
		}
		if (enough) {
			++git;
		} else {
			userGroups.erase(git++);
		}
	}
	return userGroups.size() > 0;
}

void User::checkRoleRight(RightObject object, RightsType requiredRight) const
{
	PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
	PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
	PDimension rightObjectDim = sysDb->getRightsObjectDimension();

	bool enough = false;
	for (size_t i = 0; i < groups.size(); i++) {
		if (getRoleRightObject(sysDb, roleRightObjectCube, groupRoles[i], rightObjectDim->findElementByName(SystemDatabase::ROLE[object], 0, false)) < requiredRight) {
			continue;
		}
		enough = true;
		break;
	}

	if (!enough) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for " + SystemDatabase::ROLE[object], "user", (int)(getId()));
	}
}

User::MinMaxRight User::getDatabaseDataRight(IdentifierType dbId) const
{
	if (isAdmin) {
		return make_pair(RIGHT_DELETE, RIGHT_DELETE);
	}
	DbRightsMap::const_iterator it = dbRights.find(dbId);
	if (it == dbRights.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database not found in User::getDatabaseDataRight method");
	}
	return it->second;
}

void User::checkDatabaseDataRight(PUser user, IdentifierType dbId, RightsType requiredRight)
{
	if (User::checkUser(user) && user->getDatabaseDataRight(dbId).second < requiredRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "database identifier", (int)dbId);
	}
}

bool User::checkDatabaseDataRight(CPDatabase db, set<IdentifierType>& userGroups, RightsType requiredRight) const
{
	if ((db->getType() == NORMALTYPE || db->getType() == USER_INFOTYPE) && db->getName() != Server::NAME_CONFIG_DATABASE) {
		PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
		PCube groupDatabaseDataCube = sysDb->getGroupDatabaseCube();
		PDimension databaseDim = sysDb->getDatabaseDimension();
		Element* elem = databaseDim->findElementByName(db->getName(), 0, false);

		PCubeArea area(new CubeArea(sysDb, groupDatabaseDataCube, 2));
		PSet s(new Set);
		for (set<IdentifierType>::iterator git = userGroups.begin(); git != userGroups.end(); ++git) {
			s->insert(*git);
		}
		area->insert(0, s);
		s.reset(new Set);
		s->insert(elem->getIdentifier());
		area->insert(1, s);

		PCellStream cs = groupDatabaseDataCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			RightsType rt = value.isEmpty() ? Server::getDefaultDbRight() : stringToRightsType(value);
			if (rt < requiredRight) {
				set<IdentifierType>::iterator git = userGroups.find(cs->getKey()[0]);
				userGroups.erase(git);
			}
		}
	}
	return userGroups.size() > 0;
}

RightsType User::getRoleDbRight(RightObject object, CPDatabase db) const
{
	RightsType result = RIGHT_NONE;
	if (groups.size()) {
		if (db->getType() == SYSTEMTYPE || (db->getType() == USER_INFOTYPE && db->getName() == Server::NAME_CONFIG_DATABASE)) {
			result = getRoleRight(object);
		} else {
			vector<RoleDbCubeRight> vRights;
			fillRights(vRights, object, db, CPCube());
			for (size_t i = 0; i < groups.size(); i++) {
				result = max(result, min(vRights[i].roleRight, vRights[i].dbRight));
			}
		}
	}
	return result;
}

void User::checkRoleDbRight(RightObject object, CPDatabase db, RightsType requiredRight) const
{
	if (getRoleDbRight(object, db) < requiredRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)getId());
	}
}

User::MinMaxRight User::getCubeDataRight(CPDatabase db, IdentifierType cubeId) const
{
	RightMap::const_iterator it = cubeRights.find(make_pair(db->getId(), cubeId));
	if (it == cubeRights.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cube not found in User::getCubeDataRight method");
	}
	return it->second;
}

User::MinMaxRight User::getRDCDRight(IdentifierType dbId, IdentifierType cubeId) const
{
	RightMap::const_iterator it = RDCDRights.find(make_pair(dbId, cubeId));
	if (it == RDCDRights.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "database or cube not found in User::getRDCDRight method");
	}
	return it->second;
}

bool User::checkCubeDataRight(CPDatabase db, CPCube cube, set<IdentifierType>& userGroups, RightsType requiredRight) const
{
	if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
		PCube groupCubeDataCube = db->findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false);
		PDimension cubeDimension = db->findDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, PUser(), false);
		Element* elem = cubeDimension->lookupElementByName(cube->getName(), false);

		PCubeArea area(new CubeArea(db, groupCubeDataCube, 2));
		PSet s(new Set);
		for (set<IdentifierType>::iterator git = userGroups.begin(); git != userGroups.end(); ++git) {
			s->insert(*git);
		}
		area->insert(0, s);
		s.reset(new Set);
		s->insert(elem->getIdentifier());
		area->insert(1, s);

		PCellStream cs = groupCubeDataCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			if (!value.isEmpty() && stringToRightsType(value) < requiredRight) {
				set<IdentifierType>::iterator git = userGroups.find(cs->getKey()[0]);
				userGroups.erase(git);
			}
		}
	}
	return userGroups.size() > 0;
}

PCube User::getCellDataRightCube(CPDatabase db, CPCube cube)
{
	PCube groupCellDataCube;
	// get rights cube
	try {
		groupCellDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_CELL_DATA + cube->getName(), PUser(), true, false);
	} catch (const ErrorException& e) {
		if (e.getErrorType() == ErrorException::ERROR_CUBE_NOT_FOUND) {
			// no groupCubeDataCube found == no restrictions
			return PCube();
		} else {
			throw;
		}
	}
	return groupCellDataCube;
}

bool User::getRolesGroups(bool checkExists)
{
	roles.clear();
	groups.clear();

	PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
	if (!getGroups(sysDb, checkExists)) {
		return false;
	}
	size_t size = groups.size();
	groupRoles.clear();
	groupRoles.resize(size);

	PCube groupRoleCube = sysDb->getGroupRoleCube();
	ElementsType roleElements = sysDb->getRoleElements();

	PCubeArea area(new CubeArea(sysDb, groupRoleCube, 2));
	PSet s(new Set);
	for (ElementsType::iterator it = roleElements.begin(); it != roleElements.end(); ++it) {
		s->insert((*it)->getIdentifier());
	}
	area->insert(1, s);

	set<IdentifierType> r;
	set<IdentifierType> gr;

	for (size_t i = 0; i < size; i++) {
		s.reset(new Set);
		s->insert(groups[i]);
		area->insert(0, s);

		PCellStream cs = groupRoleCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			if ((string)value == "1") {
				IdentifierType id = cs->getKey()[1];

				if (r.find(id) == r.end()) {
					r.insert(id);
					roles.push_back(id);
				}
				if (gr.find(id) == gr.end()) {
					gr.insert(id);
					groupRoles[i].push_back(id);
				}
			}
		}
		gr.clear();
	}

	return true;
}

// returns false if user was already deleted and groups can't be refreshed
bool User::getGroups(PSystemDatabase sysDb, bool checkExists)
{
	PDimension dim = sysDb->getGroupDimension();
	if (isExternal) {

		// check if not deleted already - has to be removed from users list
		if (checkExists) {
			Element *user = (getId() == (IdentifierType) - 1 ? sysDb->getUserElement(getName(), NULL) : sysDb->getUserDimension()->lookupElement(getId(), false));
			if (!user) {
				Logger::debug << "user '" << getId() << "' not found in user dimension, removing from user list" << endl;
				return false;
			}
		}

		isAdmin = false;
		for (vector<string>::const_iterator i = groupNames.begin(); i != groupNames.end(); ++i) {
			Element* e = sysDb->getGroupElement(*i);

			if (e) {
				groups.push_back(e->getIdentifier());
				if (e->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
					isAdmin = true;
				}
			}
		}
	} else {
		PCube userGroupCube = sysDb->findCubeByName(SystemDatabase::NAME_USER_GROUP_CUBE, PUser(), true, false);
		ElementsType groupElements = sysDb->getGroupElements();

		Element *user = (getId() == (IdentifierType) - 1 ? sysDb->getUserElement(getName(), NULL) : sysDb->getUserDimension()->lookupElement(getId(), false));

		if (!user) {
			Logger::debug << "user '" << getId() << "' not found in user dimension, removing from user list" << endl;
			return false;
		}

		PCubeArea area(new CubeArea(sysDb, userGroupCube, 2));
		PSet s(new Set);
		s->insert(user->getIdentifier());
		area->insert(0, s);

		IdentifierType adminId = -1;
		s.reset(new Set);
		for (ElementsType::iterator i = groupElements.begin(); i != groupElements.end(); ++i) {
			IdentifierType id = (*i)->getIdentifier();
			s->insert(id);
			if ((*i)->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
				adminId = id;
			}
		}
		area->insert(1, s);

		PCellStream cs = userGroupCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
		isAdmin = false;
		while (cs->next()) {
			const CellValue value = cs->getValue();

			if ((string)value == "1") {
				IdentifierType id = cs->getKey()[1];
				groups.push_back(id);
				if (id == adminId) {
					isAdmin = true;
				}
			}
		}
	}

	return true;
}

RightsType User::getRoleRightObject(CPSystemDatabase sysDb, CPCube roleRightObjectCube, const IdentifiersType& roles, Element* rightObject) const
{
	RightsType result = RIGHT_NONE;

	PCubeArea area(new CubeArea(sysDb, roleRightObjectCube, 2));
	PSet s(new Set);
	for (IdentifiersType::const_iterator i = roles.begin(); i != roles.end(); ++i) {
		s->insert(*i);
	}
	area->insert(0, s);
	s.reset(new Set);
	s->insert(rightObject->getIdentifier());
	area->insert(1, s);

	PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
	while (cs->next()) {
		const CellValue value = cs->getValue();

		RightsType rt = stringToRightsType(value);
		if (result < rt) {
			result = rt;
		}
		if (result == RIGHT_SPLASH) {
			break;
		}
	}

	return result;
}

RightsType User::computeDimensionDataRight(CPCube groupDimensionDataCube, IdentifierType groupId, CPDimension dimension, Element* element, RightsType defaultRight)
{
	PArea path(new Area(2));
	PSet s(new Set);
	s->insert(groupId);
	path->insert(0, s);
	s.reset(new Set);
	s->insert(element->getIdentifier());
	path->insert(1, s);

	CellValue value = groupDimensionDataCube->getCellValue(path);

	if (!value.isEmpty() && (string)value != "") {
		return stringToRightsType(value);
	} else {
		CPParents parents = element->getParents();

		if (!parents || !parents->size()) {
			return defaultRight;
		}

		RightsType result = RIGHT_NONE;

		for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
			Element *parent = dimension->lookupElement(*iter, false);
			RightsType parentRT = computeDimensionDataRight(groupDimensionDataCube, groupId, dimension, parent, defaultRight);

			if (parentRT > result) {
				result = parentRT;
			}
		}

		return result;
	}
}

void User::computeRights(CPDatabase db)
{
	if (!isAdmin) {
		IdentifierType dbId = db->getId();
		CPSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
		PDimension groupDimension = sysDb->findDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, PUser(), false);
		PCube userGroupCube = sysDb->findCubeByName(SystemDatabase::NAME_USER_GROUP_CUBE, PUser(), true, false);
		DimRightsMap::iterator groupIter = dimRights.find(make_pair(dbId, NO_IDENTIFIER));
		bool sameGroups = isSameDimToken(groupIter, groupDimension->getToken(), userGroupCube->getMyToken());

		computeRoleCellDataRight(db, sysDb);
		computeDbDataRights(db, sysDb);
		computeCubeDataRights(db, sameGroups);
		computeDimensionDataRights(db, sysDb, sameGroups);
		computeRDCDRights(db);

		if (!sameGroups) {
			DimRights dRights;
			dRights.dimToken = groupDimension->getToken();
			dRights.cubeToken = userGroupCube->getMyToken();
			updateDimRights(dbId, NO_IDENTIFIER, dRights);
		}
	}
}

void User::computeRoleCellDataRight(CPDatabase db, CPSystemDatabase sysDb)
{
	RightsType rtMin;
	RightsType rtMax;
	size_t size = groups.size();
	if (size) {
		rtMin = RIGHT_DELETE;
		rtMax = RIGHT_NONE;

		PCube groupRoleCube = sysDb->getGroupRoleCube();
		PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
		PDimension rightObjectDim = sysDb->getRightsObjectDimension();
		Element *elem = rightObjectDim->findElementByName(SystemDatabase::ROLE[User::cellDataRight], 0, false);

		for (size_t i = 0; i < size; i++) {
			RightsType rt = getRoleRightObject(sysDb, roleRightObjectCube, groupRoles[i], elem);
			if (rtMin > rt) {
				rtMin = rt;
			}
			if (rtMax < rt) {
				rtMax = rt;
			}
		}
	} else {
		rtMin = RIGHT_NONE;
		rtMax = RIGHT_NONE;
	}
	cellDataRights = make_pair(rtMin, rtMax);
}

void User::computeDbDataRights(CPDatabase db, CPSystemDatabase sysDb)
{
	RightsType rtMin;
	RightsType rtMax;
	if (db->getType() == SYSTEMTYPE) {
		rtMin = rtMax = getRoleRight(User::rightsRight);
	} else if (db->getName() == Server::NAME_CONFIG_DATABASE) {
		rtMin = rtMax = RIGHT_DELETE;
	} else if (db->getType() == NORMALTYPE || db->getType() == USER_INFOTYPE) {
		PCube groupDatabaseDataCube = sysDb->getGroupDatabaseCube();
		PDimension databaseDim = sysDb->getDatabaseDimension();
		Element* elem = databaseDim->lookupElementByName(db->getName(), false);
		if (!elem) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid database in User::computeDbDataRights method");
		}

		PCubeArea area(new CubeArea(sysDb, groupDatabaseDataCube, 2));
		PSet s(new Set);
		for (IdentifiersType::const_iterator git = groups.begin(); git != groups.end(); ++git) {
			s->insert(*git);
		}
		area->insert(0, s);
		s.reset(new Set);
		s->insert(elem->getIdentifier());
		area->insert(1, s);

		bool empty = true;
		rtMin = RIGHT_DELETE;
		rtMax = RIGHT_NONE;

		PCellStream cs = groupDatabaseDataCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			RightsType rt;
			if (value.isEmpty()) {
				rt = Server::getDefaultDbRight();
			} else {
				rt = stringToRightsType(value);
				empty = false;
			}
			if (rtMin > rt) {
				rtMin = rt;
			}
			if (rtMax < rt) {
				rtMax = rt;
			}
		}
		if (empty) {
			rtMin = rtMax = Server::getDefaultDbRight();
		}
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid database type in User::computeDbDataRights method");
	}
	updateDbRights(db->getIdentifier(), rtMin, rtMax);
}

void User::computeCubeDataRights(CPDatabase db, bool sameGroups)
{
	IdentifierType dbId = db->getId();

	Cube* groupCubeDataCube = 0;
	CubeTokenMap::iterator cubeIter;
	bool sameToken_groupCubeDataCube = true;
	bool sameToken_cubeDimension = true;

	if (db->getType() != SYSTEMTYPE) {
		groupCubeDataCube = db->findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false).get();
		cubeIter = cubeTokens.find(dbId);
		sameToken_groupCubeDataCube = isSameCubeToken(cubeIter, groupCubeDataCube->getMyToken());

		PDimension cubeDimension = db->findDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, PUser(), false);
		DimRightsMap::iterator dimIter = dimRights.find(make_pair(dbId, cubeDimension->getId()));
		sameToken_cubeDimension = dimIter != dimRights.end() && dimIter->second.dimToken == cubeDimension->getToken() ;
	}

	if (!sameGroups || !sameToken_groupCubeDataCube || !sameToken_cubeDimension) {
		vector<CPCube> cubes = db->getCubes(PUser());
		for (vector<CPCube>::iterator cit = cubes.begin(); cit != cubes.end(); ++cit) {
			CPCube cube = *cit;
			IdentifierType cubeId = cube->getId();

			RightsType rtMin;
			RightsType rtMax;
			if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
				size_t size = groups.size();
				vector<RoleDbCubeRight> vRights(size);
				getCubeDataRights(vRights, db, cube, groups, false);

				if (size) {
					rtMin = RIGHT_EMPTY;
					rtMax = RIGHT_NONE;
					for (size_t i = 0; i < size; i++) {
						RightsType rt = vRights[i].cubeRight;
						if (rtMin > rt) {
							rtMin = rt;
						}
						if (rtMax < rt) {
							rtMax = rt;
						}
					}
				} else {
					rtMin = RIGHT_NONE;
					rtMax = RIGHT_NONE;
				}
			} else {
				rtMin = RIGHT_EMPTY;
				rtMax = RIGHT_EMPTY;
			}
			updateRightMap(cubeRights, dbId, cubeId, rtMin, rtMax);
		}
		if (db->getType() != SYSTEMTYPE) {
			updateCubeToken(cubeIter, dbId, groupCubeDataCube->getMyToken());
		}
	}
}

void User::computeDimensionDataRights(CPDatabase db, CPSystemDatabase sysDb, bool sameGroups)
{
	IdentifierType dbId = db->getId();
	vector<CPDimension> dimensions = db->getDimensions(PUser());
	for (vector<CPDimension>::iterator dit = dimensions.begin(); dit != dimensions.end(); ++dit) {
		CPDimension dim = *dit;
		IdentifierType dimId = dim->getId();

		DimRights dRights;
		if (dim->hasRightsCube()) {
			PCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
			DimRightsMap::iterator dimIter = dimRights.find(make_pair(dbId, dimId));
			bool sameDim = isSameDimToken(dimIter, dim->getToken(), groupDimensionDataCube->getMyToken());

			if (!sameGroups || !sameDim) {
				if (!groupDimensionDataCube->sizeFilledCells() && !groupDimensionDataCube->hasRule()) {
					dRights.minRight = RIGHT_EMPTY;
					dRights.maxRight = RIGHT_EMPTY;
				} else {
					size_t size = groups.size();
					if (size) {
						dRights.elemRights.reset(new ElemRightsMap);
						dRights.minRight = RIGHT_EMPTY;
						dRights.maxRight = RIGHT_NONE;

						PCubeArea area(new CubeArea(db, groupDimensionDataCube, 2));
						PSet s(new Set());
						for (size_t i = 0; i < groups.size(); i++) {
							s->insert(groups[i]);
						}
						area->insert(0, s);

						PElemRightsMap tmpRights;
						ElementsType children = dim->getElements(PUser(), true);
						while (children.size()) {
							s.reset(new Set());
							for (ElementsType::const_iterator it = children.begin(); it != children.end(); ++it) {
								s->insert((*it)->getIdentifier());
							}
							area->insert(1, s);

							tmpRights.reset(new ElemRightsMap);
							PCellStream cs = groupDimensionDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
							while (cs->next()) {
								IdentifierType group = cs->getKey().at(0);
								IdentifierType id = cs->getKey().at(1);
								CellValue value = cs->getValue();
								RightsType rt;

								if (value.isEmpty()) {
									Element *elem = dim->lookupElement(id, false);
									if (elem->getParentsCount()) {
										//take the highest defined parent right
										rt = RIGHT_NONE;
										CPParents parentIds = elem->getParents();
										for (Parents::const_iterator pit = parentIds->begin(); pit != parentIds->end(); ++pit) {
											ElemRightsMap::iterator parentit = dRights.elemRights->find(make_pair(group, *pit));
											if (parentit == dRights.elemRights->end()) {
												rt = RIGHT_EMPTY;
											} else if (parentit->second > rt) {
												rt = parentit->second;
											}
										}
									} else {
										rt = RIGHT_EMPTY;
									}
								} else {
									rt = stringToRightsType(value);
								}

								ElemRightsMap::iterator elemit = tmpRights->find(make_pair(group, id));
								if (elemit == tmpRights->end()) {
									tmpRights->insert(make_pair(make_pair(group, id), rt));
								} else {
									throw ErrorException(ErrorException::ERROR_INTERNAL, "calculation error in User::computeDimensionDataRights method");
								}
								if (dRights.minRight > rt) {
									dRights.minRight = rt;
								}
								if (dRights.maxRight < rt) {
									dRights.maxRight = rt;
								}
							}

							// copy calculated rights to dRights.elemRights
							for (ElemRightsMap::const_iterator it = tmpRights->begin(); it != tmpRights->end(); ++it) {
								ElemRightsMap::iterator elemit = dRights.elemRights->find(it->first);
								if (elemit == dRights.elemRights->end()) {
									if (it->second != RIGHT_EMPTY) {
										// store only non-empty values
										dRights.elemRights->insert(make_pair(it->first, it->second));
									}
								} else {
									if (it->second > elemit->second) {
										elemit->second = it->second;
									}
								}
							}

							// get next level
							ElementsType parents;
							children.swap(parents);
							set<IdentifierType> uniqueChildren;
							for (ElementsType::const_iterator pit = parents.begin(); pit != parents.end(); ++pit) {
								const IdentifiersWeightType *pChildren = (*pit)->getChildren();
								for (IdentifiersWeightType::const_iterator chit = pChildren->begin(); chit != pChildren->end(); ++chit) {
									IdentifierType id = chit->first;
									if (uniqueChildren.find(id) == uniqueChildren.end()) {
										uniqueChildren.insert(id);
										children.push_back(dim->lookupElement(id, false));
									}
								}
							}
						}
					} else {
						dRights.minRight = RIGHT_NONE;
						dRights.maxRight = RIGHT_NONE;
					}
				}

				if (!sameDim) {
					dRights.dimToken = dim->getToken();
					dRights.cubeToken = groupDimensionDataCube->getMyToken();
				}
			} else {
				continue;
			}
		} else {
			dRights.minRight = RIGHT_EMPTY;
			dRights.maxRight = RIGHT_EMPTY;
		}

		updateDimRights(dbId, dimId, dRights);
//		if (dRights.elemRights) {
//			cout << dim->getName() << ", " << dRights;
//		}
	}
}

void User::computeRDCDRights(CPDatabase db)
{
	IdentifierType dbId = db->getId();
	vector<CPCube> cubes = db->getCubes(PUser());
	for (vector<CPCube>::iterator cit = cubes.begin(); cit != cubes.end(); ++cit) {
		CPCube cube = *cit;
		IdentifierType cubeId = cube->getId();

		RightsType rtMin = cellDataRights.first;
		RightsType rtMax = cellDataRights.second;

		DbRightsMap::const_iterator dit = dbRights.find(dbId);
		if (dit != dbRights.end()) {
			if (rtMin > dit->second.first) {
				rtMin = dit->second.first;
			}
			if (rtMax < dit->second.second) {
				rtMax = dit->second.second;
			}
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "database not found in User::computeRDCDRights method");
		}

		RightMap::const_iterator it = cubeRights.find(make_pair(dbId, cubeId));
		if (it != cubeRights.end()) {
			if (rtMin > it->second.first) {
				rtMin = it->second.first;
			}
			if (rtMax < it->second.second) {
				rtMax = it->second.second;
			}
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "cube not found in User::computeRDCDRights method");
		}

		MinMaxRight rtDims = computeDRight(dbId, cube);
		if (rtMin > rtDims.first) {
			rtMin = rtDims.first;
		}
		if (rtMax < rtDims.second) {
			rtMax = rtDims.second;
		}
		updateRightMap(RDCDRights, dbId, cubeId, rtMin, rtMax);
	}
}

User::MinMaxRight User::computeDRight(IdentifierType dbId, CPCube cube) const
{
	MinMaxRight result(RIGHT_EMPTY, RIGHT_NONE);
	const IdentifiersType* dimIds = cube->getDimensions();
	if (dimIds->size()) {
		for (size_t i = 0; i < dimIds->size(); i++) {
			DimRightsMap::const_iterator it = dimRights.find(make_pair(dbId, dimIds->at(i)));
			if (it != dimRights.end()) {
				if (result.first > it->second.minRight) {
					result.first = it->second.minRight;
				}
				if (result.second < it->second.maxRight) {
					result.second = it->second.maxRight;
				}
			}
		}
	} else {
		result.second = RIGHT_EMPTY;
	}
	return result;
}

string User::rightsTypeToString(RightsType rt)
{
	switch (rt) {
	case RIGHT_SPLASH:
		return "S";

	case RIGHT_DELETE:
		return "D";

	case RIGHT_WRITE:
		return "W";

	case RIGHT_READ:
		return "R";

	default:
		return "N";
	}
}

RightsType User::stringToRightsType(const string& str)
{
	if (str == "S") {
		return RIGHT_SPLASH;
	} else if (str == "D") {
		return RIGHT_DELETE;
	} else if (str == "W") {
		return RIGHT_WRITE;
	} else if (str == "R") {
		return RIGHT_READ;
	} else {
		return RIGHT_NONE;
	}
}

bool User::checkCellDataRightCube(CPDatabase db, CPCube cube)
{
	bool res = false;
	if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
		PCube groupCellDataCube = getCellDataRightCube(db, cube);
		res = groupCellDataCube && (groupCellDataCube->sizeFilledStringCells() || groupCellDataCube->hasActiveRule());
	}
	return res;
}

void User::checkRuleDatabaseRight(const User *user, IdentifierType targDb, IdentifierType sourDb)
{
	if (sourDb != targDb) {
		Context::getContext()->setSaveToCache(false);
		if (checkUser(user) && user->getDatabaseDataRight(sourDb).second < RIGHT_READ) {
			//cout << "user: " << user->getName() << ", " << targDb << " <- " << sourDb << endl;
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
		}
	}
}

void User::updateGlobalDatabaseToken(PServer server, PDatabase db)
{
	PSystemDatabase sysDb = server->getSystemDatabase();
	if (sysDb != 0) {
		PUserList users = sysDb->getUsers(true);
		sysDb->setUsers(users);
		for (UserList::Iterator it = users->begin(); it != users->end(); ++it) {
			PUser user = COMMITABLE_CAST(User, users->get((*it)->getId(), true));
			users->set(user);
			if (db->getStatus() == Database::LOADED || db->getStatus() == Database::CHANGED) {
				user->computeRights(db);
			} else {
				user->computeDbDataRights(db, sysDb);
			}
		}
	}
}

bool User::canLogin() const
{
	return (roles.size() != 0);
}

bool User::isExternalUser() const
{
	return isExternal;
}
const IdentifiersType& User::getUserGroups() const
{
	return groups;
}

void User::getUserGroupsCopy(set<IdentifierType>& gr) const
{
	for (size_t i = 0; i < groups.size(); i++) {
		gr.insert(groups[i]);
	}
}

bool User::refreshAll()
{
	if (getRolesGroups(true)) {
		refreshRights();
		return true;
	} else {
		return false;
	}
}

void User::refreshRights()
{
	if (!isAdmin) {
		dbRights.clear();
		cubeTokens.clear();
		cubeRights.clear();
		RDCDRights.clear();
		dimRights.clear();

		PServer server = Context::getContext()->getServer();
		PDatabaseList dbs = server->getDatabaseList(false);
		for (DatabaseList::Iterator it = dbs->begin(); it != dbs->end(); ++it) {
			PDatabase db = COMMITABLE_CAST(Database, *it);
			if (db->getStatus() == Database::LOADED || db->getStatus() == Database::CHANGED) {
				computeRights(db);
			} else {
				computeDbDataRights(db, server->getSystemDatabase());
			}
		}
	}
}

bool User::merge(const CPCommitable &o, const PCommitable &p)
{
	if (o && old) {
		throw ErrorException(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED, "Users don't merge.");
	}
	commitintern();
	return true;
}

PCommitable User::copy() const
{
	checkNotCheckedOut();
	PUser newu(new User(*this));
	return newu;
}

bool User::isSameCubeToken(CubeTokenMap::iterator& it, uint32_t cubeToken) const
{
	if (it == cubeTokens.end()) {
		return false;
	} else {
		return it->second == cubeToken;
	}
}

void User::updateCubeToken(CubeTokenMap::iterator& it, IdentifierType dbId, uint32_t cubeToken)
{
	if (it == cubeTokens.end()) {
		cubeTokens.insert(make_pair(dbId, cubeToken));
	} else {
		it->second = cubeToken;
	}
}

bool User::isSameDimToken(DimRightsMap::iterator &it, uint32_t dimToken, uint32_t cubeToken) const
{
	if (it == dimRights.end()) {
		return false;
	} else {
		return it->second.dimToken == dimToken && it->second.cubeToken == cubeToken;
	}
}

void User::updateDimRights(IdentifierType dbId, IdentifierType dimId, const DimRights &dRights)
{
	DimRightsMap::iterator it = dimRights.find(make_pair(dbId, dimId));
	if (it == dimRights.end()) {
		dimRights.insert(make_pair(make_pair(dbId, dimId), dRights));
	} else {
		it->second = dRights;
	}
}

const User::DimRights *User::getDimRights(IdentifierType dbId, IdentifierType dimId) const
{
	const DimRights *result = 0;
	DimRightsMap::const_iterator it = dimRights.find(make_pair(dbId, dimId));
	if (it != dimRights.end()) {
		result = &it->second;
	}
	return result;
}

void User::updateDbRights(IdentifierType dbId, RightsType minRight, RightsType maxRight)
{
	DbRightsMap::iterator rit = dbRights.find(dbId);
	if (rit == dbRights.end()) {
		dbRights.insert(make_pair(dbId, make_pair(minRight, maxRight)));
	} else {
		rit->second = make_pair(minRight, maxRight);
	}
}

void User::updateRightMap(RightMap& rightMap, IdentifierType dbId, IdentifierType objId, RightsType minRight, RightsType maxRight)
{
	RightMap::iterator rit = rightMap.find(make_pair(dbId, objId));
	if (rit == rightMap.end()) {
		rightMap.insert(make_pair(make_pair(dbId, objId), make_pair(minRight, maxRight)));
	} else {
		rit->second = make_pair(minRight, maxRight);
	}
}

bool User::checkDimsAndCells(CPDatabase db, CPCube cube, set<IdentifierType> &userGroups, CPCubeArea area, bool checkCells, RightsType requiredRight, bool *defaultUsed) const
{
	if ((cube->getType() == SYSTEMTYPE && cube->getCubeType() != Cube::ATTRIBUTES) || (cube->getType() == ATTRIBUTETYPE && checkCells) || cube->getType() == USER_INFOTYPE) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid call of User::checkDimsAndCells() method");
	}

	const IdentifiersType* dims = cube->getDimensions();
	size_t dimCount = dims->size();
	vector<RightsType> rtSingle(dimCount);
	vector<ElemRightsMap *> erm(dimCount);

	size_t groupCount = userGroups.size();
	IdentifiersType vGroups(groupCount);
	size_t igr = 0;
	for (set<IdentifierType>::const_iterator it = userGroups.begin(); it != userGroups.end(); ++it, igr++) {
		vGroups[igr] = *it;
	}
	vector<RoleDbCubeRight> vRights(groupCount);
	getCubeDataRights(vRights, db, cube, vGroups, true);

	bool enough = true;
	for (size_t i = 0; i < dimCount; i++) {
		PDimension dim = db->lookupDimension(dims->at(i), false);

		if (dim->hasRightsCube()) {
			CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
			rtSingle[i] = RIGHT_EMPTY;
			if (groupDimensionDataCube && (groupDimensionDataCube->sizeFilledStringCells() || groupDimensionDataCube->hasActiveRule())) {
				DimRightsMap::const_iterator dit = dimRights.find(make_pair(db->getId(), dim->getId()));
				if (dit == dimRights.end()) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid dimension in User::checkDimsAndCells method");
				}
				erm[i] = dit->second.elemRights.get();
			}
		} else {
			RightsType rt = dim->getDimensionDataRight(this);
			if (rt < requiredRight) {
				enough = false;
				break;
			} else {
				rtSingle[i] = rt;
			}
		}
	}

	if (enough) {
		for (igr = 0; igr < vGroups.size(); igr++) {
			enough = true;
			IdentifierType groupId = vGroups[igr];
			bool emptyCell = vRights[igr].cubeRight == RIGHT_EMPTY;
			PArea emptyArea(new Area(dimCount));

			// check element rights
			for (size_t i = 0; i < dimCount; i++) {
				PSet sEmpty(new Set);
				bool emptyElem = false;
				CPSet s = area->getDim(i);
				if (s) {
					if (erm[i]) {
						for (Set::Iterator sit = s->begin(); sit != s->end(); ++sit) {
							ElemRightsMap::iterator it = erm[i]->find(make_pair(groupId, *sit));
							if (it != erm[i]->end()) {
								if (it->second < requiredRight) {
									enough = false;
									break;
								}
							} else {
								emptyElem = true;
								if (checkCells && emptyCell) {
									sEmpty->insert(*sit);
								}
							}
						}
					} else {
						emptyElem = rtSingle[i] == RIGHT_EMPTY;
					}
					if (!emptyElem) {
						emptyCell = false;
					}
					if (checkCells && emptyCell) {
						emptyArea->insert(i, erm[i] ? sEmpty : s);
					}
				} else {
					checkCells = false; // area is empty
				}
			}

			if (checkCells) {
				// check cell rights
				PCube groupCellDataCube = getCellDataRightCube(db, cube);
				PCubeArea garea(new CubeArea(db, groupCellDataCube, dimCount + 1));
				for (size_t i = 0; i < dimCount; i++) {
					garea->insert((IdentifierType)i + 1, area->getDim(i));
				}
				PSet s(new Set);
				s->insert(groupId);
				garea->insert(0, s);

				PCellStream cs = groupCellDataCube->calculateArea(garea, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
				while (cs->next()) {
					if (stringToRightsType(cs->getValue()) < requiredRight) {
						enough = false;
						break;
					}
				}

				// check if all cells from emptyArea contains a value in groupCellDataCube
				if (enough && emptyCell) {
					if (!emptyArea->getSize()) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid area in User::checkDimsAndCells method");
					}
					for (size_t i = 0; i < dimCount; i++) {
						garea->insert((IdentifierType)i + 1, emptyArea->getDim(i));
					}

					Area::PathIterator pit = emptyArea->pathBegin();
					PCellStream cs = groupCellDataCube->calculateArea(garea, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
					while (cs->next()) {
						const char* p1 = (char *)&cs->getKey()[0];
						const char* p2 = (char *)&(*pit)[0];
						if (memcmp(p1 + sizeof(IdentifierType), p2, dimCount * sizeof(IdentifierType))) {
							break;
						}
						++pit;
					}
					if (pit == emptyArea->pathEnd()) {
						emptyCell = false;
					}
				}
			}
			if (enough && emptyCell) {
				if (defaultUsed) {
					*defaultUsed = true;
				}
				enough = db->getDefaultRight() >= requiredRight;
			}
			if (enough) { // otherwise check the next group
				break;
			}
		}
	}
	return enough;
}

void User::checkAreaRightsComplete(CPDatabase db, CPCube cube, CPCubeArea area, RightSetting& rs, bool isZero, RightsType requiredRight, bool *defaultUsed) const
{
	RightsType roleRight = requiredRight;
	if (requiredRight == RIGHT_SPLASH) {
		requiredRight = isZero ? RIGHT_DELETE : RIGHT_WRITE;
	}

	User::MinMaxRight rtRole = getRoleCellDataRight();
	bool enough = rtRole.second >= roleRight;

	User::MinMaxRight rtDb;
	if (enough) {
		rtDb = getDatabaseDataRight(db->getId());
		enough = rtDb.second >= requiredRight;
	}

	User::MinMaxRight rtCube;
	if (enough) {
		rtCube = getCubeDataRight(db, cube->getId());
		enough = rtCube.second >= requiredRight;
	}
	User::MinMaxRight rtDims;
	if (enough) {
		rtDims = computeDRight(db->getId(), cube);
		enough = rtDims.second >= requiredRight;
	}

	if (enough && rtCube.first == RIGHT_EMPTY && rtDims.first == RIGHT_EMPTY && !rs.checkCells) {
		if (defaultUsed) {
			*defaultUsed = true;
		}
		enough = db->getDefaultRight() >= requiredRight;
	}

	if (enough) {
		bool checkRole= rtRole.first < roleRight;
		bool checkDb = rtDb.first < requiredRight;
		bool checkCube = rtCube.first < requiredRight;

		set<IdentifierType> userGroups;
		getUserGroupsCopy(userGroups);

		if (checkRole || rs.checkSepRight) {
			PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
			if (enough && checkRole) {
				enough = checkRoleRight(sysDb, userGroups, cellDataRight, roleRight);
			}
			if (enough && rs.checkSepRight) {
				enough = checkRoleRight(sysDb, userGroups, eventProcessorRight, RIGHT_DELETE);
				if (!enough) {
					throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for disabling event processor", "user", (int)getId());
				}
			}
		}

		if (enough && checkDb) {
			enough = checkDatabaseDataRight(db, userGroups, requiredRight);
		}
		if (enough && checkCube) {
			enough = checkCubeDataRight(db, cube, userGroups, requiredRight);
		}
		if (enough) {
			enough = checkDimsAndCells(db, cube, userGroups, area, rs.checkCells, requiredRight, defaultUsed);
		}
	}
	if (!enough) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)getId());
	}
}

RightsType User::getElementRight(ElemRightsMap *erm, IdentifierType groupId, IdentifierType elemId) const
{
	if (erm) {
		ElemRightsMap::const_iterator eit = erm->find(make_pair(groupId, elemId));
		if (eit != erm->end()) {
			return eit->second;
		}
	}
	return RIGHT_EMPTY;
}

bool User::checkElementRight(ElemRightsMap *erm, const IdentifiersType *userGroups, IdentifierType elemId, RightsType requiredRight) const
{
	if (!erm) {
		return true;
	}
	if (userGroups) {
		bool empty = false;
		for (IdentifiersType::const_iterator git = userGroups->begin(); git != userGroups->end(); ++git) {
			User::ElemRightsMap::const_iterator it = erm->find(make_pair(*git, elemId));
			if (it == erm->end()) {
				empty = true;
			} else if (it->second >= requiredRight) {
				return true;
			}
		}
		return empty;
	}
	return false;
}

bool User::checkElementRight(IdentifierType dbId, IdentifierType dimId, IdentifierType elemId, RightsType requiredRight) const
{
	DimRightsMap::const_iterator dit = dimRights.find(make_pair(dbId, dimId));
	if (dit == dimRights.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid dimension in User::checkElementRight method");
	}
	return checkElementRight(dit->second.elemRights.get(), &groups, elemId, requiredRight);
}

RightsType User::getElementRight(IdentifierType dbId, IdentifierType dimId, IdentifierType elemId, vector<RoleDbCubeRight> &vRights, bool checkRole) const
{
	if (isAdmin) {
		return RIGHT_DELETE;
	}
	DimRightsMap::const_iterator dit = dimRights.find(make_pair(dbId, dimId));
	if (dit == dimRights.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid dimension in User::getElementRight method");
	}
	ElemRightsMap *erm = dit->second.elemRights.get();

	RightsType result = RIGHT_NONE;
	for (size_t i = 0; i < groups.size(); i++) {
		RightsType rtMin = checkRole ? min(vRights[i].roleRight, vRights[i].dbRight) : vRights[i].dbRight;
		rtMin = min(rtMin, getElementRight(erm, groups[i], elemId));

		if (rtMin > result) {
			result = rtMin;
			if (result >= RIGHT_DELETE) {
				// no higher right can be obtained from other groups
				break;
			}
		}
	}
	return result;
}

RightsType User::getCellRight(CPDatabase db, CPCube cube, const IdentifiersType &key, vector<RoleDbCubeRight> &vRights, bool *defaultUsed) const
{
	if (defaultUsed) {
		*defaultUsed = false;
	}
	if ((cube->getType() == SYSTEMTYPE && cube->getCubeType() != Cube::ATTRIBUTES) || cube->getType() == USER_INFOTYPE) {
		return cube->getMinimumAccessRight(CONST_COMMITABLE_CAST(User, shared_from_this()));
	}

	const IdentifiersType *dims = cube->getDimensions();
	size_t dimCount = dims->size();

	vector<ElemRightsMap *> vElemRights(dimCount);
	for (size_t i = 0; i < dimCount; i++) {
		PDimension dim = db->lookupDimension(dims->at(i), false);
		DimRightsMap::const_iterator it = dimRights.find(make_pair(db->getId(), dim->getId()));
		vElemRights[i] = it == dimRights.end() ? 0 : (*it).second.elemRights.get();
	}

	PCellStream cellStream;
	if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
		PCube groupCellDataCube = getCellDataRightCube(db, cube);
		if (groupCellDataCube && (groupCellDataCube->sizeFilledStringCells() || groupCellDataCube->hasActiveRule())) {
			PCubeArea garea(new CubeArea(db, groupCellDataCube, dimCount + 1));
			PSet s(new Set);
			for (IdentifiersType::const_iterator git = groups.begin(); git != groups.end(); ++git) {
				s->insert(*git);
			}
			garea->insert(0, s);
			for (size_t i = 0; i < dimCount; i++) {
				s.reset(new Set);
				s->insert(key[i]);
				garea->insert((IdentifierType)i + 1, s);
			}

			cellStream = groupCellDataCube->calculateArea(garea, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, 0);
		}
	}

	RightsType result = RIGHT_NONE;
	for (size_t i = 0; i < groups.size(); i++) {
		bool empty = vRights[i].cubeRight == RIGHT_EMPTY;
		RightsType rtMin = min(min(vRights[i].roleRight, vRights[i].dbRight), vRights[i].cubeRight);

		for (size_t j = 0; j < dimCount; j++) {
			rtMin = min(rtMin, getElementRight(vElemRights[j], groups[i], key[j]));

			if (cellStream) {
				cellStream->next(); //it is necessary to call next even if rtMin is RIGHT_NONE

				if (rtMin != RIGHT_NONE) {
					const CellValue value = cellStream->getValue();
					if (!value.isEmpty()) {
						empty = false;
						rtMin = min(rtMin, stringToRightsType(value));
					}
				}
			}
		}
		if (empty) {
			if (defaultUsed) {
				*defaultUsed = true;
			}
			rtMin = min(rtMin, db->getDefaultRight());
		}

		result = max(result, rtMin);
		if (result >= RIGHT_DELETE) {
			if (vRights[i].roleRight == RIGHT_SPLASH) {
				result = RIGHT_SPLASH;
				break;
			} else {
				result = RIGHT_DELETE;
			}
		}
	}
	return result;
}

ostream& operator<<(ostream& ostr, const User::DimRightsMap &drm)
{
	for (User::DimRightsMap::const_iterator it = drm.begin(); it != drm.end(); ++it) {
		ostr << "database: " << it->first.first << ", dimension: " << it->first.second << ", ";
		ostr << it->second;
	}
	return ostr;
}

ostream& operator<<(ostream& ostr, const User::DimRights &dr)
{
	ostr << "minRight: " << dr.minRight << ", maxRight: " << dr.maxRight << ", dimToken: " << dr.dimToken << ", cubeToken: " << dr.cubeToken;
	if (dr.minRight > dr.maxRight) {
		ostr << " ... min-max inconsistency";
	}
	ostr << endl;
	if (dr.elemRights) {
		for (User::ElemRightsMap::const_iterator it = dr.elemRights->begin(); it != dr.elemRights->end(); ++it) {
			ostr << "groupId: " << it->first.first << ", elemId: " << it->first.second << ", right: " << it->second << endl;
		}
	}
	return ostr;
}

void User::fillRights(vector<RoleDbCubeRight> &vRights, RightObject object, CPDatabase db, CPCube cube) const
{
	if (groups.size()) {
		vRights.resize(groups.size());
		getRoleRights(vRights, object);
		getDatabaseDataRight(vRights, db);
		if (cube) {
			getCubeDataRights(vRights, db, cube, groups, true);
		}
	}
}

void User::getRoleRights(vector<RoleDbCubeRight> &vRights, RightObject object) const
{
	if (object == cellDataRight && cellDataRights.first == cellDataRights.second) {
		for (size_t i = 0; i < groups.size(); i++) {
			vRights[i].roleRight = cellDataRights.first;
		}
	} else {
		PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
		PCube roleRightObjectCube = sysDb->getRoleRightObjectCube();
		PDimension rightObjectDim = sysDb->getRightsObjectDimension();
		Element* elem = rightObjectDim->findElementByName(SystemDatabase::ROLE[object], 0, false);

		PCubeArea area(new CubeArea(sysDb, roleRightObjectCube, 2));
		PSet s(new Set);
		s->insert(elem->getIdentifier());
		area->insert(1, s);

		for (IdentifiersType::const_iterator git = groups.begin(); git != groups.end(); ++git) {
			s.reset(new Set);
			size_t i;
			for (i = 0; i < groups.size(); i++) {
				if (groups[i] == *git) {
					break;
				}
			}
			for (IdentifiersType::const_iterator rit = groupRoles[i].begin(); rit != groupRoles[i].end(); ++rit) {
				s->insert(*rit);
			}
			area->insert(0, s);

			RightsType rt = RIGHT_NONE;
			PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_UNSORTED_PLAN);
			while (cs->next()) {
				const CellValue value = cs->getValue();

				rt = max(rt, stringToRightsType(value));
				if (rt == RIGHT_SPLASH) {
					break;
				}
			}
			vRights[i].roleRight = rt;
		}
	}
}

void User::getDatabaseDataRight(vector<RoleDbCubeRight> &vRights, CPDatabase db) const
{
	size_t size = groups.size();

	if (db->getType() == SYSTEMTYPE || (db->getType() == USER_INFOTYPE && db->getName() == Server::NAME_CONFIG_DATABASE)) {
		for (size_t i = 0; i < size; i++) {
			vRights[i].dbRight = RIGHT_DELETE;
		}
	} else {
		MinMaxRight right = getDatabaseDataRight(db->getId());
		if (right.first == right.second) {
			for (size_t i = 0; i < size; i++) {
				vRights[i].dbRight = right.first;
			}
		} else {
			PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
			PCube groupDatabaseDataCube = sysDb->getGroupDatabaseCube();
			PDimension databaseDim = sysDb->getDatabaseDimension();
			Element* elem = databaseDim->findElementByName(db->getName(), 0, false);

			PCubeArea area(new CubeArea(sysDb, groupDatabaseDataCube, 2));
			PSet s(new Set);
			for (size_t i = 0; i < size; i++) {
				s->insert(groups[i]);
			}
			area->insert(0, s);
			s.reset(new Set);
			s->insert(elem->getIdentifier());
			area->insert(1, s);

			size_t i = 0;
			PCellStream cs = groupDatabaseDataCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
			while (cs->next()) {
				const CellValue value = cs->getValue();
				RightsType rt = value.isEmpty() ? Server::getDefaultDbRight() : stringToRightsType(value);
				vRights[i++].dbRight = rt;
			}
		}
	}
}

void User::getCubeDataRights(vector<RoleDbCubeRight> &vRights, CPDatabase db, CPCube cube, const IdentifiersType &vGroups, bool isComputed) const
{
	size_t size = vGroups.size();
	if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
		if (isComputed) {
			MinMaxRight right = getCubeDataRight(db, cube->getId());
			if (right.first == right.second) {
				for (size_t i = 0; i < size; i++) {
					vRights[i].cubeRight = right.first;
				}
				return;
			}
		}

		CPCube groupCubeDataCube = db->findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false);
		CPDimension cubeDimension = db->findDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, PUser(), false);
		Element* elem = cubeDimension->lookupElementByName(cube->getName(), false);

		if (elem) {
			PCubeArea area(new CubeArea(db, groupCubeDataCube, 2));
			PSet s(new Set);
			for (size_t i = 0; i < size; i++) {
				s->insert(vGroups[i]);
			}
			area->insert(0, s);
			s.reset(new Set);
			s->insert(elem->getIdentifier());
			area->insert(1, s);

			size_t i = 0;
			PCellStream cs = groupCubeDataCube->calculateArea(area, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
			while (cs->next()) {
				const CellValue value = cs->getValue();
				RightsType rt = value.isEmpty() ? RIGHT_EMPTY : stringToRightsType(value);
				if (rt == RIGHT_SPLASH) {
					// maximum is RIGHT_DELETE for cubes
					rt = RIGHT_DELETE;
				}
				vRights[i++].cubeRight = rt;
			}
		} else {
			for (size_t i = 0; i < size; i++) {
				vRights[i].cubeRight = RIGHT_NONE;
			}
		}
	} else {
		for (size_t i = 0; i < size; i++) {
			vRights[i].cubeRight = RIGHT_EMPTY;
		}
	}
}

}
