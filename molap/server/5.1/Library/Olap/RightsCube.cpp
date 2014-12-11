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
 * 
 *
 */

#include "Olap/RightsCube.h"

#include "Exceptions/FileFormatException.h"

#include "InputOutput/FileReader.h"

#include "Olap/Context.h"
#include "Olap/Server.h"
#include "Engine/EngineBase.h"

#include "Thread/WriteLocker.h"

namespace palo {
void RightsCube::clearCells(PServer server, PDatabase db, PUser user, bool useJournal)
{
	if (getName() == SystemDatabase::NAME_USER_GROUP_CUBE || getName() == SystemDatabase::NAME_GROUP_ROLE || getName() == SystemDatabase::NAME_ROLE_RIGHT_OBJECT_CUBE || getName() == SystemDatabase::NAME_USER_USER_PROPERTIERS_CUBE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube cannot be cleared", "user", user ? (int)user->getId() : 0);
	}
	Cube::clearCells(server, db, user, useJournal);
	updateUserRights(server, db);
	PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
	if (system) {
		PCube userPropCube = system->getUserUserPropertiesCube();
		if (userPropCube && userPropCube->getId() == getId()) {
			server->updateLicenses(system);
		}
	}
}

void RightsCube::clearCells(PServer server, PDatabase db, PCubeArea baseElements, PUser user, bool useJournal)
{
	if (getName() == SystemDatabase::NAME_USER_GROUP_CUBE || getName() == SystemDatabase::NAME_GROUP_ROLE || getName() == SystemDatabase::NAME_ROLE_RIGHT_OBJECT_CUBE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube cannot be cleared", "user", user ? (int)user->getId() : 0);
	}
	Cube::clearCells(server, db, baseElements, user, useJournal);
	updateUserRights(server, db);
	PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
	if (system) {
		PCube userPropCube = system->getUserUserPropertiesCube();
		if (userPropCube && userPropCube->getId() == getId()) {
			server->updateLicenses(system);
		}
	}
}

ResultStatus RightsCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode,bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	checkCheckedOut();
	if (value.isString()) {
		size_t l = value.length();
		string newValue = value;

		PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
		if (l > 0) {
			if ( system == 0 || (system->getRoleRightObjectCube() && getId() == system->getRoleRightObjectCube()->getId()) || (system->getGroupDatabaseCube() && getId() == system->getGroupDatabaseCube()->getId())
			   ) {

				if (l > 1) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "value not allowed here", "value", value);
				}

				string okStrings = "NRWD";

				if (system && system->getRoleRightObjectCube() && getId() == system->getRoleRightObjectCube()->getId()) {
					PDimension dim = db->lookupDimension(dimensions[1], false);
					const Element *elem = dim->lookupElement((*cellPath->pathBegin())[1], false);

					if (SystemDatabase::ROLE[User::cellDataRight] == elem->getName(dim->getElemNamesVector())) {
						okStrings = "NRWDS";
					}
				}

				char valueChar = ::toupper(value[0]);
				newValue = string(1, valueChar);

				if (newValue.find_first_not_of(okStrings) != string::npos) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "value not allowed here", "value", value);
				}
			}
		}

		if (system && cellPath->dimCount() == 2) {
			PDimension dim0 = db->lookupDimension(dimensions[0], false);
			PDimension dim1 = db->lookupDimension(dimensions[1], false);
			const Element *elem0 = dim0->lookupElement((*cellPath->pathBegin())[0], false);
			const Element *elem1 = dim1->lookupElement((*cellPath->pathBegin())[1], false);

			Cube *cube = system->getRoleRightObjectCube().get();
			if (user && cube && cube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
					throw ErrorException(ErrorException::ERROR_INVALID_PERMISSION, "Rights of user \"admin\" are firmly predetermined.");
				}
			}
			cube = system->getUserUserPropertiesCube().get();
			if (cube && cube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
				if (elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::LICENSES) {
					server->checkNamedLicense(elem0->getIdentifier(), value);
				}
			}
			cube = system->getGroupGroupPropertiesCube().get();
			if (cube && cube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
			}
			cube = system->getRoleRolePropertiesCube().get();
			if (cube && cube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
			}
			cube = system->getUserGroupCube().get();
			if (cube && cube->getId() == getId()) {
				if (User::checkUser(user) && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
					//only user from admin group can change admin group
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't modify \"admin\" group.", "user", user->getId());
				}
			}
			cube = system->getGroupDatabaseCube().get();
			if (cube && cube->getId() == getId() && User::checkUser(user)) {
				//only user from admin group can change #_GROUP_DATABASE_DATA cube
				throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't modify " + SystemDatabase::NAME_GROUP_DATABASE_CUBE + " cube.", "user", user->getId());
			}
		}
		value = newValue;
	}
	ResultStatus status = Cube::setCellValue(server, db, cellPath, value, lockedCells, user, session, checkArea, addValue, splashMode, bWriteToJournal, checkRights, changedCubes, possibleCommit, ct);
	updateUserRights(server, db);
	return status;
}

PRule RightsCube::createRule(PServer server, PDatabase db, PRuleNode rn, const string& definition, const string& external, const string& comment, bool activate, PUser user, bool useJournal, IdentifierType *id, double position)
{
	if (User::checkUser(user) && user->getRoleDbRight(User::rightsRight, db) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	PRule rule = SystemCube::createRule(server, db, rn, definition, external, comment, activate, user, useJournal, id, position);
	if (rule) {
		updateUserRights(server, db);
	}
	return rule;
}

bool RightsCube::modifyRule(PServer server, PDatabase db, PRule rule, PRuleNode rn, const string& definition, const string& external, const string& comment, PUser user, ActivationType activation, bool useJournal, double position)
{
	if (User::checkUser(user) && user->getRoleDbRight(User::rightsRight, db) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	bool result = SystemCube::modifyRule(server, db, rule, rn, definition, external, comment, user, activation, useJournal, position);
	updateUserRights(server, db);
	return result;
}

bool RightsCube::activateRules(PServer server, PDatabase db, const vector<PRule> &rules, ActivationType activation, PUser user, string* errMsg, bool bDefinitionChangedBefore, bool useJournal)
{
	if (User::checkUser(user) && user->getRoleDbRight(User::rightsRight, db) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	bool result = SystemCube::activateRules(server, db, rules, activation, user, errMsg, bDefinitionChangedBefore, useJournal);
	updateUserRights(server, db);
	return result;
}

void RightsCube::deleteRule(PServer server, PDatabase db, IdentifierType id, PUser user, bool useJournal)
{
	if (User::checkUser(user) && user->getRoleDbRight(User::rightsRight, db) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	SystemCube::deleteRule(server, db, id, user, useJournal);
	updateUserRights(server, db);
}

bool RightsCube::deleteElement(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifierType element, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, bool completeRemove)
{
	bool del = Cube::deleteElement(server, db, user, event, dimension, element, disabledRules, delType, completeRemove);
	if (del) {
		updateUserRights(server, db);
		PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
		if (system) {
			PCube userPropCube = system->getUserUserPropertiesCube();
			if (userPropCube && userPropCube->getId() == getId()) {
				server->updateLicenses(system);
			}
		}
	}
	return del;
}

bool RightsCube::deleteElements(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifiersType elements, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, PSet fullSet, bool completeRemove)
{
	bool del = Cube::deleteElements(server, db, user, event, dimension, elements, disabledRules, delType, fullSet, completeRemove);
	if (del) {
		updateUserRights(server, db);
		PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
		if (system) {
			PCube userPropCube = system->getUserUserPropertiesCube();
			if (userPropCube && userPropCube->getId() == getId()) {
				server->updateLicenses(system);
			}
		}
	}
	return del;
}

void RightsCube::updateUserRights(PServer server, PDatabase db)
{
	// is called in write mode
	PSystemDatabase system = server->getSystemDatabase();

	// database is the system database, refresh all users
	if (system && system->getId() == db->getId()) {
		PCube userGroupCube = system->getUserGroupCube();
		PCube groupRoleCube = system->getGroupRoleCube();
		PCube roleRightObjectCube = system->getRoleRightObjectCube();
		PCube groupDatabaseCube = system->getGroupDatabaseCube();
		bool refresh = false;
		if (userGroupCube && groupRoleCube && roleRightObjectCube && groupDatabaseCube) {
			if (getId() == userGroupCube->getId() || getId() == groupRoleCube->getId() || getId() == roleRightObjectCube->getId() || getId() == groupDatabaseCube->getId()) {
				//refresh if this cube is a 'user right' cube
				refresh = true;
			} if (userGroupCube->hasRule() || groupRoleCube->hasRule() || roleRightObjectCube->hasRule() || groupDatabaseCube->hasRule()) {
				//refresh if a 'user right' cube could read values from this cube
				refresh = true;
			}
		}
		if (refresh) {
			Context::getContext()->setRefreshUsers();
		}
	} else {
		if (dimensions.size() == 2 && getName() == SystemCube::PREFIX_GROUP_DIMENSION_DATA + db->lookupDimension(dimensions[1], false)->getName()) {
			PDimensionList dbs = db->getDimensionList(true);
			db->setDimensionList(dbs);
			PDimension dim = db->lookupDimension(dimensions.at(1), true);
			dbs->set(dim);
		}
	}
}

void RightsCube::saveCube(PServer server, PDatabase db)
{
	saveCubeIntern(server, db, false, true);
}

void RightsCube::loadCube(PServer server, PDatabase db, bool processJournal)
{
	Logger::trace << "loading rights cube '" << getName() << "'. " << endl;
	loadCubeIntern(server, db, processJournal, false, true);
}

void RightsCube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const
{
	if (defaultUsed) {
		*defaultUsed = false;
	}
	if (User::checkUser(user)) {
		bool enough = getName() != SystemDatabase::NAME_GROUP_DATABASE_CUBE; //only user from admin group can use #_GROUP_DATABASE_DATA cube
		if (enough && getCubeAccessRight(user) < minimumRight) {
			enough = false;
		}
		if (enough) {
			CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
			CPSystemDatabase system = CONST_COMMITABLE_CAST(SystemDatabase, db);
			if (system) {
				// special check for "admin" user, group and role
				if (minimumRight > RIGHT_READ && (getId() == system->getUserGroupCube()->getId() || getId() == system->getGroupRoleCube()->getId())) {
					if (isElementInSet(db, area, 0, SystemDatabase::NAME_ADMIN) || isElementInSet(db, area, 1, SystemDatabase::NAME_ADMIN)) {
						enough = false;
					}
				}
				// special check for "admin" role
				else if (minimumRight > RIGHT_READ && getId() == system->getRoleRightObjectCube()->getId()) {
					if (isElementInSet(db, area, 0, SystemDatabase::NAME_ADMIN)) {
						enough = false;
					}
				}
				// special check for "password"
				else if (getId() == system->getUserUserPropertiesCube()->getId()) {
					if (isElementInSet(db, area, 1, SystemDatabase::PASSWORD) && User::checkUser(user) && user->getRoleRight(User::passwordRight) < minimumRight) {
						enough = false;
					}
				}
			} else {
				// special check for "#_Rights"
				const string cubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION;
				if (minimumRight > RIGHT_READ && getId() == db->lookupCubeByName(cubeName, false)->getId()) {
					if (isElementInSet(db, area, 1, SystemDatabase::NAME_RIGHTS_ELEMENT)) {
						enough = false;
					}
				}
			}
		}
		if (!enough) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
		}
	}
}

RightsType RightsCube::getMinimumAccessRight(CPUser user) const
{
	if (User::checkUser(user)) {
		RightsType rtCube = getCubeAccessRight(user);
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		CPSystemDatabase system = CONST_COMMITABLE_CAST(SystemDatabase, db);
		if (system && getId() == system->getUserUserPropertiesCube()->getId()) {
			RightsType rtRole =  user->getRoleRight(User::passwordRight);
			rtCube = min(rtCube, rtRole);
		}
		return rtCube;
	} else {
		return RIGHT_SPLASH;
	}
}

RightsType RightsCube::getCubeAccessRight(CPUser user) const
{
	if (User::checkUser(user)) {
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		if (db->getType() == NORMALTYPE || db->getType() == GPUTYPE) {
			return user->getRoleDbRight(User::rightsRight, db);
		} else {
			return user->getRoleRight(User::rightsRight);
		}
	} else {
		return RIGHT_SPLASH;
	}
}

PCommitable RightsCube::copy() const
{
	checkNotCheckedOut();
	PRightsCube newd(new RightsCube(*this));
	return newd;
}

bool RightsCube::invalidateCache()
{
	bool res = Cube::invalidateCache();
	if (res) {
		Context *context = Context::getContext();
		PServer server = context->getServerCopy();
//		CPDatabase db = COMMITABLE_CAST(Database, context->getParent(shared_from_this()));
//		updateUserRights(server, db);
	}
	return res;
}

bool RightsCube::isElementInSet(CPDatabase db, CPCubeArea area, uint32_t dimOrdinal, const string elemName) const
{
	CPSet s = area->getDim(dimOrdinal);
	PDimension dim = db->lookupDimension(dimensions[dimOrdinal], false);
	const Element *elem = dim->lookupElementByName(elemName, false);
	return s->find(elem->getIdentifier()) != s->end();
}

}
