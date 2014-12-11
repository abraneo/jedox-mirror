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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Olap/User.h"

#include "Olap/SystemCube.h"
#include "Olap/SystemDatabase.h"
#include "Olap/SystemDimension.h"
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

void User::getRoleRights(RightObject object, vector<RoleCubeRight>& rcRights) const
{
	size_t size = groups.size();
	if (size) {
		if (!rcRights.size()) {
			rcRights.resize(size);
		}
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
			PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
			while (cs->next()) {
				const CellValue value = cs->getValue();

				rt = max(rt, stringToRightsType(value));
				if (rt == RIGHT_SPLASH) {
					break;
				}
			}
			rcRights[i].first = rt;
		}
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
		PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
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
	PCube groupRoleCube = sysDb->getGroupRoleCube();

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

User::MinMaxRight User::getCubeDataRight(CPDatabase db, IdentifierType cubeId) const
{
	RightMap::const_iterator it = cubeRights.find(make_pair(db->getId(), cubeId));
	return it == cubeRights.end() ? make_pair(db->getDefaultRight(), db->getDefaultRight()) : it->second;
}

void User::getCubeDataRights(CPDatabase db, CPCube cube, vector<RoleCubeRight>& rcRights, bool isComputed) const
{
	size_t size = groups.size();
	if (size) {
		if (!rcRights.size()) {
			rcRights.resize(size);
		}
		if (cube->getType() == SYSTEMTYPE) {
			for (size_t i = 0; i < size; i++) {
				rcRights[i].second = RIGHT_DELETE;
			}
		} else {
			MinMaxRight right = getCubeDataRight(db, cube->getId());
			if (isComputed && right.first == right.second) {
				for (size_t i = 0; i < size; i++) {
					rcRights[i].second = right.first;
				}
			} else {
				CPCube groupCubeDataCube = db->findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false);
				CPDimension cubeDimension = db->findDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, PUser(), false);
				Element* elem = cubeDimension->lookupElementByName(cube->getName(), false);

				if (elem) {
					PCubeArea area(new CubeArea(db, groupCubeDataCube, 2));
					PSet s(new Set);
					for (size_t i = 0; i < size; i++) {
						s->insert(groups[i]);
					}
					area->insert(0, s);
					s.reset(new Set);
					s->insert(elem->getIdentifier());
					area->insert(1, s);

					RightsType rt = RIGHT_NONE;
					size_t igr = 0;
					PCellStream cs = groupCubeDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
					while (cs->next()) {
						const CellValue value = cs->getValue();

						if (value.isEmpty()) {
							rt = db->getDefaultRight();
						} else {
							rt = stringToRightsType(value);
						}
						if (rt == RIGHT_SPLASH) {
							// maximum right is RIGHT_DELETE
							rt = RIGHT_DELETE;
						}
						rcRights[igr++].second = rt;
					}
				} else {
					for (size_t i = 0; i < size; i++) {
						rcRights[i].second = RIGHT_NONE;
					}
				}
			}
		}
	}
}

User::MinMaxRight User::getRCDDataRight(CPDatabase db, IdentifierType cubeId) const
{
	RightMap::const_iterator it = RCDRights.find(make_pair(db->getId(), cubeId));
	return it == RCDRights.end() ? make_pair(db->getDefaultRight(), db->getDefaultRight()) : it->second;
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

		PCellStream cs = groupCubeDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
		while (cs->next()) {
			const CellValue value = cs->getValue();

			RightsType rt = value.isEmpty() ? db->getDefaultRight() : stringToRightsType(value);
			if (rt < requiredRight) {
				set<IdentifierType>::iterator git = userGroups.find(cs->getKey()[0]);
				userGroups.erase(git);
			}
		}
	}
	return userGroups.size() > 0;
}

RightsType User::getSystemDimensionDataRight(CPSystemDatabase sysDb, CPDimension dim) const
{
	CPSystemDimension sysDim = CONST_COMMITABLE_CAST(SystemDimension, dim);
	if (!sysDim) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid system dimension");
	}

	RightsType result = RIGHT_NONE;
	RightObject object;

	// system rights dimension
	if (sysDim->getDimensionType() == Dimension::RIGHTS) {
		// #_USER_ dimension
		if (sysDim == sysDb->getUserDimension()) {
			object = User::userRight;
		}

		// #_GROUP_ dimension
		else if (sysDim == sysDb->getGroupDimension()) {
			object = User::groupRight;
		}

		// #_USER_PROPERTIES_ dimension (not changable)
		else if (sysDim == sysDb->getUserPropertiesDimension()) {
			object = User::passwordRight;
		}

		// #_ROLE_ dimension
		else if (sysDim == sysDb->getRoleDimension()) {
			// use rights of #_GROUP_ for #_GROUP_ROLE cube and
			// use rights of #_RIGHT_OBJECT_ for #_ROLE_RIGHTS_OBJECT cube and
			result = RIGHT_DELETE;
		}

		// #_RIGHT_OBJECT_ dimension
		else if (sysDim == sysDb->getRightsObjectDimension()) {
			object = User::rightsRight;
		}

		// #_GROUP_PROPERTIES_ dimension
		// #_ROLE_PROPERTIES_ dimension
		else {
			object = User::rightsRight;
		}
	}

	// alias dimension
	else if (sysDim->getDimensionType() == Dimension::ALIAS) {
		object = User::rightsRight;
	}

	// attribute dimension
	else if (sysDim->getDimensionType() == Dimension::ATTRIBUTES) {
		object = User::elementRight; // same as role "dimension element"
	} else if (sysDim->getDimensionType() == Dimension::CELLPROPS) {
		object = User::elementRight; // same as role "dimension element"
	}

	// #_CUBE_ dimension
	else if (sysDim->getDimensionType() == Dimension::CUBE) {
		object = User::cubeRight;
	}

	// #_CONFIGURATION_ dimension
	else if (sysDim->getDimensionType() == Dimension::CONFIG) {
		object = User::sysOpRight;
	}

	// #_DIMENSION_ dimension
	else if (sysDim->getDimensionType() == Dimension::DIMENSION) {
		object = User::dimensionRight;
	}

	// #_SUBSET_ dimension
	else if (sysDim->getDimensionType() == Dimension::SUBSETVIEW) {
		object = User::subSetViewRight;
	}
	// #_ID_ dimension
	else if (sysDim->getDimensionType() == Dimension::VIRTUAL) {
		result = RIGHT_READ;
	}

	else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid dimension subtype");
	}

	if (result == RIGHT_NONE) {
		if (isAdmin) {
			result = object == User::cellDataRight ? RIGHT_SPLASH : RIGHT_DELETE;
		} else {
			result = getRoleRight(object);
		}
	}

	return result;
}

RightsType User::getUserInfoDimensionDataRight() const
{
	return isAdmin ? RIGHT_DELETE : getRoleRight(User::userInfoRight);
}

RightsType User::getGroupDimensionDataRight(CPCube groupDimensionDataCube, CPDimension dim, IdentifierType groupId, IdentifierType elemId, RightsType defaultRight) const
{
	RightsType result;

	if (!groupDimensionDataCube->sizeFilledCells() && !groupDimensionDataCube->hasRule()) {
		result = defaultRight;
	} else {
		//TODO - read from cache
		Element* element = dim->findElement(elemId, 0, false);
		result = computeDimensionDataRight(groupDimensionDataCube, groupId, dim, element, defaultRight);
	}

	return result;
}

bool User::checkGroupDimensionDataRight(CPDatabase db, CPCube groupDimensionDataCube, CPDimension dim, IdentifierType elemId, RightsType requiredRight) const
{
	bool enough;

	if (!groupDimensionDataCube->sizeFilledCells() && !groupDimensionDataCube->hasRule()) {
		enough = db->getDefaultRight() >= requiredRight;
	} else {
		enough = false;
		for (size_t i = 0; i < groups.size(); i++) {
			if (getGroupDimensionDataRight(groupDimensionDataCube, dim, groups[i], elemId, db->getDefaultRight()) < requiredRight) {
				continue;
			}
			enough = true;
			break;
		}
	}

	return enough;
}

RightsType User::getDimensionDataRight(CPDatabase db, CPDimension dim, IdentifierType elemId, vector<RoleCubeRight>& rcRights) const
{
	RightsType result = RIGHT_NONE;
	RightsType right = RIGHT_DELETE;
	CPCube groupDimensionDataCube;

	bool hasRightsCube = dim->hasRightsCube();
	if (hasRightsCube) {
		groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
	} else if (dim->getType() == SYSTEMTYPE) {
		right = getSystemDimensionDataRight(Context::getContext()->getServer()->getSystemDatabase(), dim);
	} else { //USER_INFOTYPE
		right = getUserInfoDimensionDataRight();
	}

	for (size_t i = 0; i < groups.size(); i++) {
		RightsType rt;
		if (hasRightsCube) {
			rt = getGroupDimensionDataRight(groupDimensionDataCube, dim, groups[i], elemId, db->getDefaultRight());
		} else {
			rt = right;
		}
		rt = min(rcRights[i].first, rt);
		rt = min(rcRights[i].second, rt);
		if (rt > result) {
			result = rt;
			if (rt >= RIGHT_DELETE) {
				// no higher right can be obtained from other groups
				break;
			}
		}
	}

	return result;
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

		PCellStream cs = groupRoleCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
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

		Logger::trace << "loading group rights for user " << getName() << endl;
		for (vector<string>::const_iterator i = groupNames.begin(); i != groupNames.end(); ++i) {
			Element* e = sysDb->getGroupElement(*i);

			if (e) {
				groups.push_back(e->getIdentifier());
				if (e->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
					isAdmin = true;
				}
				Logger::trace << "group '" << *i << "' found in group dimension" << endl;
			} else {
				Logger::trace << "group '" << *i << "' not found in group dimension" << endl;
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

		PCellStream cs = userGroupCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
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

	PCellStream cs = roleRightObjectCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
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
		computeCubeDataRights(db, sysDb, sameGroups);
		computeDimensionDataRights(db, sysDb, sameGroups);
		computeRCDRights(db);

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

void User::computeCubeDataRights(CPDatabase db, CPSystemDatabase sysDb, bool sameGroups)
{
	IdentifierType dbId = db->getId();

	Cube* groupCubeDataCube = 0;
	CubeTokenMap::iterator cubeIter;
	bool sameCube = true;

	bool sys = dbId == sysDb->getId();
	if (!sys) {
		groupCubeDataCube = db->findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false).get();
		cubeIter = cubeTokens.find(dbId);
		sameCube = isSameCubeToken(cubeIter, groupCubeDataCube->getMyToken());
	}

	if (!sameGroups || !sameCube) {
		vector<CPCube> cubes = db->getCubes(PUser());
		for (vector<CPCube>::iterator cit = cubes.begin(); cit != cubes.end(); ++cit) {
			CPCube cube = *cit;
			IdentifierType cubeId = cube->getId();

			RightsType rtMin;
			RightsType rtMax;
			if (cube->getType() == SYSTEMTYPE) {
				rtMin = RIGHT_DELETE;
				rtMax = RIGHT_DELETE;
			} else {
				vector<RoleCubeRight> rcRights;
				getCubeDataRights(db, cube, rcRights, false);

				size_t size = rcRights.size();
				if (size) {
					rtMin = RIGHT_DELETE;
					rtMax = RIGHT_NONE;
					for (size_t i = 0; i < size; i++) {
						RightsType rt = rcRights[i].second;
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
			}
			updateRightMap(cubeRights, dbId, cubeId, rtMin, rtMax);
		}
		if (!sys) {
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
					dRights.minRight = db->getDefaultRight();
					dRights.maxRight = db->getDefaultRight();
				} else {
					size_t size = groups.size();
					if (size) {
						dRights.elemRights.reset(new MapElementRight);
						MapElementRight *pElemRights = dRights.elemRights.get();
						dRights.minRight = RIGHT_DELETE;
						dRights.maxRight = RIGHT_NONE;

						PCubeArea area(new CubeArea(db, groupDimensionDataCube, 2));
						PSet s(new Set());
						for (size_t i = 0; i < groups.size(); i++) {
							s->insert(groups[i]);
						}
						area->insert(0, s);

						PMapElementRight tmpRights;
						ElementsType children = dim->getElements(PUser(), true);
						while (children.size()) {
							s.reset(new Set());
							for (ElementsType::const_iterator it = children.begin(); it != children.end(); ++it) {
								s->insert((*it)->getIdentifier());
							}
							area->insert(1, s);

							tmpRights.reset(new MapElementRight);
							PCellStream cs = groupDimensionDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
							while (cs->next()) {
								IdentifierType id = cs->getKey().at(1);
								CellValue value = cs->getValue();
								RightsType rt;

								if (value.isEmpty()) {
									rt = RIGHT_NONE;
									Element *elem = dim->lookupElement(id, false);

									if (elem->getParentsCount()) {
										//take the highest parent right
										CPParents parentIds = elem->getParents();
										for (Parents::const_iterator pit = parentIds->begin(); pit != parentIds->end(); ++pit) {
											MapElementRight::iterator parentit = pElemRights->find(*pit);
											if (parentit == pElemRights->end()) {
												if (db->getDefaultRight() > rt) {
													rt = db->getDefaultRight();
												}
											} else {
												if (parentit->second > rt) {
													rt = parentit->second;
												}
											}
										}
									} else {
										rt = db->getDefaultRight();
									}
								} else {
									rt = stringToRightsType(value);
								}

								MapElementRight::iterator elemit = tmpRights->find(id);
								if (elemit == tmpRights->end()) {
									tmpRights->insert(make_pair(id, rt));
								} else {
									if (rt > elemit->second) {
										elemit->second = rt;
									}
								}
								if (dRights.minRight > rt) {
									dRights.minRight = rt;
								}
								if (dRights.maxRight < rt) {
									dRights.maxRight = rt;
								}
							}

							// copy calculated rights to dRights.elemRights
							for (MapElementRight::const_iterator it = tmpRights->begin(); it != tmpRights->end(); ++it) {
								MapElementRight::iterator elemit = pElemRights->find(it->first);
								if (elemit == pElemRights->end()) {
									if (it->second != db->getDefaultRight()) {
										// store only non-default values
										pElemRights->insert(make_pair(it->first, it->second));
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
						throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)getId());
					}
				}

				if (!sameDim) {
					dRights.dimToken = dim->getToken();
					dRights.cubeToken = groupDimensionDataCube->getMyToken();
				}
			} else {
				continue;
			}
		} else if (dim->getType() == SYSTEMTYPE) {
			dRights.minRight = dRights.maxRight = getSystemDimensionDataRight(sysDb, dim);
		} else { //USER_INFOTYPE
			dRights.minRight = dRights.maxRight = getUserInfoDimensionDataRight();
		}

		updateDimRights(dbId, dimId, dRights);
	}
}

void User::computeRCDRights(CPDatabase db)
{
	IdentifierType dbId = db->getId();
	vector<CPCube> cubes = db->getCubes(PUser());
	for (vector<CPCube>::iterator cit = cubes.begin(); cit != cubes.end(); ++cit) {
		CPCube cube = *cit;
		IdentifierType cubeId = cube->getId();

		RightsType rtMin = cellDataRights.first;
		RightsType rtMax = cellDataRights.second;

		RightMap::const_iterator it = cubeRights.find(make_pair(dbId, cubeId));
		if (it != cubeRights.end()) {
			if (rtMin > it->second.first) {
				rtMin = it->second.first;
			}
			if (rtMax < it->second.second) {
				rtMax = it->second.second;
			}
		}

		computeDRight(dbId, cube, rtMin, rtMax);
		updateRightMap(RCDRights, dbId, cubeId, rtMin, rtMax);
	}
}

void User::computeDRight(IdentifierType dbId, CPCube cube, RightsType &rtMin, RightsType &rtMax) const
{
	const IdentifiersType* dimIds = cube->getDimensions();
	for (size_t i = 0; i < dimIds->size(); i++) {
		DimRightsMap::const_iterator it = dimRights.find(make_pair(dbId, dimIds->at(i)));
		if (it != dimRights.end()) {
			if (rtMin > it->second.minRight) {
				rtMin = it->second.minRight;
			}
			if (rtMax < it->second.maxRight) {
				rtMax = it->second.maxRight;
			}
		}
	}
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
	if (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE) {
		PCube groupCellDataCube = getCellDataRightCube(db, cube);
		bool isEmpty = !groupCellDataCube || (groupCellDataCube->sizeFilledStringCells() == 0 && !groupCellDataCube->hasRule());
		if (isEmpty && db->getDefaultRight() == RIGHT_DELETE) {
			return false;
		}
	}
	return true;
}

void User::updateGlobalDatabaseToken(PServer server, PDatabase db)
{
	if (db->getStatus() == Database::LOADED || db->getStatus() == Database::CHANGED) {
		PSystemDatabase sysDb = server->getSystemDatabase();
		if (sysDb != 0) {
			PUserList users = sysDb->getUsers(true);
			sysDb->setUsers(users);
			for (UserList::Iterator it = users->begin(); it != users->end(); ++it) {
				PUser user = COMMITABLE_CAST(User, users->get((*it)->getId(), true));
				users->set(user);
				user->computeRights(db);
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
		cubeTokens.clear();
		cubeRights.clear();
		RCDRights.clear();
		dimRights.clear();

		PDatabaseList dbs = Context::getContext()->getServer()->getDatabaseList(false);
		for (DatabaseList::Iterator it = dbs->begin(); it != dbs->end(); ++it) {
			PDatabase db = COMMITABLE_CAST(Database, *it);
			if (db->getStatus() == Database::LOADED || db->getStatus() == Database::CHANGED) {
				computeRights(db);
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

void User::updateRightMap(RightMap& rightMap, IdentifierType dbId, IdentifierType objId, RightsType minRight, RightsType maxRight)
{
	RightMap::iterator rit = rightMap.find(make_pair(dbId, objId));
	if (rit == rightMap.end()) {
		rightMap.insert(make_pair(make_pair(dbId, objId), make_pair(minRight, maxRight)));
	} else {
		rit->second = make_pair(minRight, maxRight);
	}
}

bool User::checkCell(size_t dimCount, IdentifierType groupId, const IdentifiersType& grKey, vector<pair<bool, bool> >& checkElems, vector<RightsType>& rtSingle, vector<ElemRightsMap>& rtMulti, RightsType requiredRight)
{
	bool enough = true;
	for (size_t i = 0; i < dimCount; i++) {
		if (checkElems[i].first) {
			if (checkElems[i].second) {
				if (rtSingle[i] < requiredRight) {
					enough = false;
					break;
				}
			} else {
				ElemRightsMap::iterator it = rtMulti[i].find(make_pair(groupId, grKey[i + 1]));
				if (it != rtMulti[i].end() && it->second < requiredRight) {
					enough = false;
					break;
				}
			}
		}
	}
	return enough;
}

User::ElemRightsMap User::computeDimensionDataRight(CPDatabase db, CPCube groupDimensionDataCube, set<IdentifierType> &userGroups, CPDimension dim, CPSet self)
{
	ElemRightsMap result;
	PSet self_anc = Set::addAncestors(self, dim);

	PCubeArea area(new CubeArea(db, groupDimensionDataCube, 2));
	PSet s(new Set);
	for (set<IdentifierType>::iterator git = userGroups.begin(); git != userGroups.end(); ++git) {
		s->insert(*git);
	}
	area->insert(0, s);
	area->insert(1, self_anc);

	PCellStream cs = groupDimensionDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
	while (cs->next()) {
		const IdentifiersType &key = cs->getKey();
		result.insert(make_pair(make_pair(key[0], key[1]), stringToRightsType(cs->getValue())));
	}
	return result;
}

RightsType User::getDimensionDataRight(ElemRightsMap &erm, IdentifierType groupId, IdentifierType elemId, CPDimension dim, RightsType defaultRight)
{
	RightsType rt = RIGHT_NONE;
	ElemRightsMap::const_iterator it = erm.find(make_pair(groupId, elemId));
	if (it == erm.end()) {
		Element *elem = dim->lookupElement(elemId, false);
		CPParents parents = elem->getParents();

		if (!parents || !parents->size()) {
			rt = defaultRight;
		} else {
			for (Parents::const_iterator it = parents->begin(); it != parents->end(); ++it) {
				RightsType parentRT = getDimensionDataRight(erm, groupId, *it, dim, defaultRight);
				if (parentRT > rt) {
					rt = parentRT;
				}
			}
		}
		erm.insert(make_pair(make_pair(groupId, elemId), rt));
	} else {
		rt = it->second;
	}
	return rt;
}

bool User::checkDimsAndCells(CPDatabase db, CPCube cube, set<IdentifierType>& userGroups, CPCubeArea area, bool checkDims, bool checkCells, RightsType requiredRight) const
{
	const IdentifiersType* dims = cube->getDimensions();
	size_t dimCount = dims->size();
	vector<pair<bool, bool> > checkElems(dimCount); //<check at all, use rtSinge>
	vector<RightsType> rtSingle(dimCount);
	vector<ElemRightsMap> rtMulti(dimCount);

	bool enough = true;
	for (size_t i = 0; i < dimCount; i++) {
		checkElems[i] = make_pair(false, false);
		if (checkDims) {
			PDimension dim = db->lookupDimension(dims->at(i), false);

			if (dim->hasRightsCube()) {
				CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
				bool empty = !groupDimensionDataCube || (groupDimensionDataCube->sizeFilledStringCells() == 0 && !groupDimensionDataCube->hasRule());

				rtSingle[i] = RIGHT_NONE;
				if (empty) {
					RightsType rt = db->getDefaultRight();
					if (rt < requiredRight) {
						enough = false;
						break;
					} else if (checkCells && rt < RIGHT_DELETE) {
						bool b = true;
						for (Area::ConstElemIter eit = area->elemBegin(i); eit != area->elemEnd(i); ++eit) {
							for (set<IdentifierType>::const_iterator git = userGroups.begin(); git != userGroups.end(); ++git) {
								rtMulti[i].insert(make_pair(make_pair(*git, *eit), rt));
								if (b) {
									checkElems[i] = make_pair(true, false);
									b = false;
								}
							}
						}
					}
				} else {
					ElemRightsMap erm;
					erm = computeDimensionDataRight(db, groupDimensionDataCube, userGroups, dim, area->getDim(i));

					bool b = true;
					for (Area::ConstElemIter eit = area->elemBegin(i); eit != area->elemEnd(i); ++eit) {
						enough = false;
						for (set<IdentifierType>::const_iterator git = userGroups.begin(); git != userGroups.end(); ++git) {
							RightsType rt = getDimensionDataRight(erm, *git, *eit, dim, db->getDefaultRight());

							if (rt >= requiredRight) {
								enough = true;
								if (!checkCells) {
									break;
								}
							}
							if (checkCells && rt < RIGHT_DELETE) {
								rtMulti[i].insert(make_pair(make_pair(*git, *eit), rt));
								if (b) {
									checkElems[i] = make_pair(true, false);
									b = false;
								}
							}
						}
						if (!enough) {
							break;
						}
					}
					if (!enough) {
						break;
					}
				}
			} else {
				RightsType rt;
				if (dim->getType() == SYSTEMTYPE) {
					rt = getSystemDimensionDataRight(Context::getContext()->getServer()->getSystemDatabase(), dim);
				} else { //USER_INFOTYPE
					rt = getUserInfoDimensionDataRight();
				}
				if (rt < requiredRight) {
					enough = false;
					break;
				} else {
					checkElems[i] = make_pair(true, true);
					rtSingle[i] = rt;
				}
			}
		}
	}

	if (enough && checkCells && (cube->getType() == NORMALTYPE || cube->getType() == GPUTYPE)) {
		PCube groupCellDataCube = getCellDataRightCube(db, cube);

		size_t groupCount = userGroups.size();
		vector<PCellStream> streams(groupCount);
		size_t igr = 0;
		for (set<IdentifierType>::const_iterator git = userGroups.begin(); git != userGroups.end(); ++git, igr++) {
			PCubeArea garea(new CubeArea(db, groupCellDataCube, dimCount + 1)); //different copy for each group
			for (size_t i = 0; i < dimCount; i++) {
				garea->insert((IdentifierType)i + 1, area->getDim(i));
			}
			PSet s(new Set);
			s->insert(*git);
			garea->insert(0, s);

			streams[igr] = groupCellDataCube->calculateArea(garea, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
		}

		while (streams[0]->next()) {
			const IdentifiersType& grKey = streams[0]->getKey();

			for (igr = 1; igr < groupCount; igr++) {
				streams[igr]->next();
			}

			enough = false;
			igr = 0;
			for (set<IdentifierType>::const_iterator git = userGroups.begin(); git != userGroups.end(); ++git, igr++) {
				CellValue value = streams[igr]->getValue();

				RightsType rt = value.isEmpty() ? db->getDefaultRight() : stringToRightsType(value);
				if (rt >= requiredRight) {
					if (!checkDims) {
						enough = true;
						break;
					}
					enough = checkCell(dimCount, *git, grKey, checkElems, rtSingle, rtMulti, requiredRight);
				}
				if (enough) { //if enough then check next cell otherwise continue with the next group
					break;
				}
			}
			if (!enough) {
				break;
			}
		}
	}
	return enough;
}

void User::checkAreaRightsComplete(CPDatabase db, CPCube cube, CPCubeArea area, RightSetting& rs, bool isZero, RightsType requiredRight) const
{
	IdentifierType cubeId = cube->getId();

	User::MinMaxRight rtRCD = getRCDDataRight(db, cubeId);

	bool enough = rtRCD.second >= requiredRight;
	if (enough) {
		RightsType roleRight = requiredRight;
		if (requiredRight == RIGHT_SPLASH) {
			requiredRight = isZero ? RIGHT_DELETE : RIGHT_WRITE;
		}

		bool checkCellDataRight = rtRCD.first >= roleRight ? false : getRoleCellDataRight().first < roleRight;
		bool checkCube = rtRCD.first >= requiredRight ? false : getCubeDataRight(db, cubeId).first < requiredRight;
		bool checkDims = rtRCD.first < requiredRight;

		if (checkCellDataRight || checkCube || checkDims || rs.checkCells || rs.checkSepRight) {
			set<IdentifierType> userGroups;
			getUserGroupsCopy(userGroups);

			if (checkCellDataRight || rs.checkSepRight) {
				PSystemDatabase sysDb = Context::getContext()->getServer()->getSystemDatabase();
				if (enough && checkCellDataRight) {
					enough = checkRoleRight(sysDb, userGroups, cellDataRight, roleRight);
				}
				if (enough && rs.checkSepRight) {
					enough = checkRoleRight(sysDb, userGroups, eventProcessorRight, RIGHT_DELETE);
					if (!enough) {
						throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for disabling event processor", "user", (int)getId());
					}
				}
			}

			if (enough && checkCube) {
				enough = checkCubeDataRight(db, cube, userGroups, requiredRight);
			}
			if (enough && (checkDims || rs.checkCells)) {
				enough = checkDimsAndCells(db, cube, userGroups, area, checkDims, rs.checkCells, requiredRight);
			}
		}
	}
	if (!enough) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)getId());
	}
}

RightsType User::getCellRight(CPDatabase db, CPCube cube, const IdentifiersType &key, vector<RoleCubeRight>& rcRights) const
{
	const IdentifiersType* dims = cube->getDimensions();
	size_t dimCount = dims->size();

	vector<RightsType> elemRights(dimCount);
	vector<PCellStream> elemStreams(dimCount);
	for (size_t i = 0; i < dimCount; i++) {
		PDimension dim = db->lookupDimension(dims->at(i), false);

		if (dim->hasRightsCube()) {
			CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
			bool empty = !groupDimensionDataCube || (groupDimensionDataCube->sizeFilledStringCells() == 0 && !groupDimensionDataCube->hasRule());
			if (empty) {
				elemRights[i] = db->getDefaultRight();
			} else {
				PCubeArea area(new CubeArea(db, groupDimensionDataCube, 2));
				PSet s(new Set);
				for (IdentifiersType::const_iterator git = groups.begin(); git != groups.end(); ++git) {
					s->insert(*git);
				}
				area->insert(0, s);
				s.reset(new Set);
				s->insert(key[i]);
				area->insert(1, s);

				elemStreams[i] = groupDimensionDataCube->calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
			}
		} else if (dim->getType() == SYSTEMTYPE) {
			elemRights[i] = getSystemDimensionDataRight(Context::getContext()->getServer()->getSystemDatabase(), dim);
		} else { //USER_INFOTYPE
			elemRights[i] = getUserInfoDimensionDataRight();
		}
	}

	RightsType cellRight = RIGHT_NONE;
	PCellStream cellStream;
	if (cube->getType() == SYSTEMTYPE) {
		cellRight = RIGHT_DELETE;
	} else {
		PCube groupCellDataCube = getCellDataRightCube(db, cube);
		bool empty = !groupCellDataCube || (groupCellDataCube->sizeFilledStringCells() == 0 && !groupCellDataCube->hasRule());
		if (empty) {
			cellRight = db->getDefaultRight();
		} else {
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

			cellStream = groupCellDataCube->calculateArea(garea, CubeArea::ALL, ALL_RULES, false, 0);
		}
	}

	RightsType result = RIGHT_NONE;
	for (size_t i = 0; i < groups.size(); i++) {
		RightsType rtMin = min(RIGHT_DELETE, rcRights[i].first);
		rtMin = min(rtMin, rcRights[i].second);

		for (size_t j = 0; j < dimCount; j++) {
			if (elemStreams[j]) {
				elemStreams[j]->next(); //it is necessary to call next even if rtMin is RIGHT_NONE

				if (rtMin != RIGHT_NONE) {
					const CellValue value = elemStreams[j]->getValue();

					if (value.isEmpty()) {
						rtMin = min(rtMin, db->getDefaultRight());
					} else {
						rtMin = min(rtMin, stringToRightsType(value));
					}
				}
			} else {
				rtMin = min(rtMin, elemRights[j]);
			}
		}
		if (cellStream) {
			cellStream->next(); //it is necessary to call next even if rtMin is RIGHT_NONE

			if (rtMin != RIGHT_NONE) {
				const CellValue value = cellStream->getValue();

				if (value.isEmpty()) {
					rtMin = min(rtMin, db->getDefaultRight());
				} else {
					rtMin = min(rtMin, stringToRightsType(value));
				}
			}
		} else {
			rtMin = min(rtMin, cellRight);
		}

		result = max(result, rtMin);
		if (result >= RIGHT_DELETE) {
			if (rcRights[i].first == RIGHT_SPLASH) {
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
		for (map<IdentifierType, RightsType>::const_iterator it = dr.elemRights->begin(); it != dr.elemRights->end(); ++it) {
			ostr << "elemId: " << it->first << ", right: " << it->second << endl;
		}
	}
	return ostr;
}

}
