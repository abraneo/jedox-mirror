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
//	updateGroupDimensionDataToken(db, user, PArea());
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
//	updateGroupDimensionDataToken(db, user, baseElements);
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

//void RightsCube::updateGroupDimensionDataToken(PDatabase db, PUser user, PArea baseElements)
//{
//	PDimension dim0 = db->lookupDimension(dimensions[0], false);
//	PDimension dim1 = db->lookupDimension(dimensions[1], false);
//	if (!db->getHideElements() || dimensions.size() != 2 || getName() != SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim1->getName()) {
//		return;
//	}
//
//	PArea cells = baseElements;
//	if (!cells) {
//		cells = PArea(new Area(dimensions.size()));
//
//		ElementsType elements1 = dim0->getElements(user);
//		ElementsType elements2 = dim1->getElements(user);
//		PSet s(new Set);
//		for (ElementsType::const_iterator it1 = elements1.begin(); it1 != elements1.end(); ++it1) {
//			s->insert((*it1)->getIdentifier());
//		}
//		cells->insert(0, s);
//		s.reset(new Set);
//		for (ElementsType::const_iterator it2 = elements2.begin(); it2 != elements2.end(); ++it2) {
//			s->insert((*it2)->getIdentifier());
//		}
//		cells->insert(1, s);
//	}
//
//	CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
//	PCubeArea areaCalc(new CubeArea(db, cube, *cells.get()));
//	PCellStream cs = cube->calculateArea(areaCalc, CubeArea::ALL, ALL_RULES, true, UNLIMITED_UNSORTED_PLAN);
//	while (cs->next()) {
//		const CellValue value = cs->getValue();
//		if (!value.isEmpty() && value.isString() && (string)value == "N") {
//			PDimension dimensionToUpdate = db->lookupDimension(dimensions[1], true);
//			PDimensionList dims = db->getDimensionList(true);
//			db->setDimensionList(dims);
//			dims->set(dimensionToUpdate);
//			break;
//		}
//	}
//}

ResultStatus RightsCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode,bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	checkCheckedOut();
	if (value.isString()) {
		size_t l = value.length();
		string newValue = value;

		PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);
		if (l > 0) {
			if (system == 0 || (system->getRoleRightObjectCube() && getId() == system->getRoleRightObjectCube()->getId())) {

				if (l > 1) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "value not allowed here", "value", value);
				}

				string okStrings = "NRWD";

				if (system) {
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
//
//				bool bOldValueWasN = false;
//				CellValue oldValue = getCellValue(cellPath);
//				if (!oldValue.isEmpty() && oldValue.isString() && (string)oldValue == "N") {
//					bOldValueWasN = true;
//				}
//
//				if ((valueChar == 'N' || bOldValueWasN) && db->getHideElements()) {
//					// we need to update dimension's token if this is #_GROUP_DIMENSION_DATA_dim cube
//					if (dimensions.size() == 2 && getName() == SystemCube::PREFIX_GROUP_DIMENSION_DATA + db->lookupDimension(dimensions[1], false)->getName()) {
//						PDimension dimensionToUpdate = db->lookupDimension(dimensions[1], true);
//						PDimensionList dims = db->getDimensionList(true);
//						db->setDimensionList(dims);
//						dims->set(dimensionToUpdate);
//					}
//				}
			}
		}

		if (system && cellPath->dimCount() == 2) {
			PDimension dim0 = db->lookupDimension(dimensions[0], false);
			PDimension dim1 = db->lookupDimension(dimensions[1], false);
			const Element *elem0 = dim0->lookupElement((*cellPath->pathBegin())[0], false);
			const Element *elem1 = dim1->lookupElement((*cellPath->pathBegin())[1], false);

			PCube roleRightObjectCube = system->getRoleRightObjectCube();
			if (user && roleRightObjectCube && roleRightObjectCube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
					throw ErrorException(ErrorException::ERROR_INVALID_PERMISSION, "Rights of user \"admin\" are firmly predetermined.");
				}
			}
			PCube userPropCube = system->getUserUserPropertiesCube();
			if (userPropCube && userPropCube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
				if (elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::LICENSES) {
					server->checkNamedLicense(elem0->getIdentifier(), value);
				}
			}
			PCube groupPropCube = system->getGroupGroupPropertiesCube();
			if (groupPropCube && groupPropCube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
			}
			PCube rolePropCube = system->getRoleRolePropertiesCube();
			if (rolePropCube && rolePropCube->getId() == getId()) {
				if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::INACTIVE) {
					throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "Can't change inactive value for admin.", "value", value);
				}
			}
		}
		value = newValue;
	}
	ResultStatus status = Cube::setCellValue(server, db, cellPath, value, lockedCells, user, session, checkArea, addValue, splashMode, bWriteToJournal, checkRights, changedCubes, possibleCommit, ct);
	updateUserRights(server, db);
	return status;
}

PRule RightsCube::createRule(PServer server, PDatabase db, PRuleNode rn, const string& definition, const string& external, const string& comment, bool activate, PUser user, bool useJournal, IdentifierType *id)
{
	if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	PRule rule = SystemCube::createRule(server, db, rn, definition, external, comment, activate, user, useJournal, id);
	if (rule) {
		updateUserRights(server, db);
	}
	return rule;
}

bool RightsCube::modifyRule(PServer server, PDatabase db, PRule rule, PRuleNode rn, const string& definition, const string& external, const string& comment, PUser user, ActivationType activation, bool useJournal)
{
	if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	bool result = SystemCube::modifyRule(server, db, rule, rn, definition, external, comment, user, activation, useJournal);
	updateUserRights(server, db);
	return result;
}

bool RightsCube::activateRules(PServer server, PDatabase db, const vector<PRule> &rules, ActivationType activation, PUser user, string* errMsg, bool bDefinitionChangedBefore, bool useJournal)
{
	if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	bool result = SystemCube::activateRules(server, db, rules, activation, user, errMsg, bDefinitionChangedBefore, useJournal);
	updateUserRights(server, db);
	return result;
}

void RightsCube::deleteRule(PServer server, PDatabase db, IdentifierType id, PUser user, bool useJournal)
{
	if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < RIGHT_DELETE) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", user ? (int)user->getId() : -1);
	}
	SystemCube::deleteRule(server, db, id, user, useJournal);
	updateUserRights(server, db);
}

bool RightsCube::deleteElement(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifierType element, bool deleteRules, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, bool completeRemove)
{
	bool del = Cube::deleteElement(server, db, user, event, dimension, element, deleteRules, disabledRules, delType, completeRemove);
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

bool RightsCube::deleteElements(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifiersType elements, bool deleteRules, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, PSet fullSet, bool completeRemove)
{
	bool del = Cube::deleteElements(server, db, user, event, dimension, elements, deleteRules, disabledRules, delType, fullSet, completeRemove);
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
		bool refresh = false;
		if (userGroupCube && groupRoleCube && roleRightObjectCube) {
			if (getId() == userGroupCube->getId() || getId() == groupRoleCube->getId() || getId() == roleRightObjectCube->getId()) {
				//refresh if this cube is a 'user right' cube
				refresh = true;
			} if (userGroupCube->hasRule() || groupRoleCube->hasRule() || roleRightObjectCube->hasRule()) {
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

void RightsCube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight) const
{
	if (user) {
		if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < minimumRight) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for rights cube", "user", (int)user->getId());
		}

		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		CPSystemDatabase system = CONST_COMMITABLE_CAST(SystemDatabase, db);
		if (system != 0) {
			PDimension dim0 = db->lookupDimension(dimensions[0], false);
			PDimension dim1 = db->lookupDimension(dimensions[1], false);
			for (Area::PathIterator it = area->pathBegin(); it != area->pathEnd(); ++it) {
				const Element *elem0 = dim0->lookupElement((*it)[0], false);
				const Element *elem1 = dim1->lookupElement((*it)[1], false);

				// special check for user and group "admin"
				if (getId() == system->getUserGroupCube()->getId() || getId() == system->getGroupRoleCube()->getId()) {
					if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && RIGHT_READ < minimumRight) {
						throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for path", "user", (int)user->getId());
					}
				}
				// special check for password
				else if (getId() == system->getRoleRightObjectCube()->getId()) {
					if (elem0->getName(dim0->getElemNamesVector()) == SystemDatabase::NAME_ADMIN && (elem1->getIdentifier() < sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0])) && RIGHT_READ < minimumRight) {
						throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for path", "user", (int)user->getId());
					}
				}
				// special check for password
				else if (getId() == system->getUserUserPropertiesCube()->getId()) {
					for (size_t i = 0; i < dimensions.size(); i++) {
						PDimension dimi = db->lookupDimension(dimensions[i], false);
						const Element *element = dimi->lookupElement((*it)[i], false);
						if (element->getName(dimi->getElemNamesVector()) == "password" && User::checkUser(user) && user->getRoleRight(User::passwordRight) < minimumRight) {
							throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for path", "user", (int)user->getId());
						}
					}
				}
			}
		} else {
			const string cubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION;
			if (getId() == db->lookupCubeByName(cubeName, false)->getId()) {
				PDimension dim1 = db->lookupDimension(dimensions[1], false);
				for (Area::PathIterator it = area->pathBegin(); it != area->pathEnd(); ++it) {
					const Element *elem1 = dim1->lookupElement((*it)[1], false);
					if (elem1->getName(dim1->getElemNamesVector()) == SystemDatabase::NAME_RIGHTS_ELEMENT && RIGHT_READ < minimumRight) {
						throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for path", "user", (int)user->getId());
					}
				}
			}
		}
	}
}

RightsType RightsCube::getMinimumAccessRight(PUser user) const
{
	return RIGHT_NONE;
}

void RightsCube::checkCubeAccessRight(PUser user, RightsType minimumRight) const
{
	if (User::checkUser(user) && user->getRoleRight(User::rightsRight) < minimumRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for cube", "user", (int)user->getId());
	}
}

RightsType RightsCube::getElementAccessRight(CPDatabase db, PUser user, CPDimension dim, IdentifierType elemId, vector<User::RoleCubeRight>& rcRights) const
{
	if (user == 0) {
		return RIGHT_SPLASH;
	}

	CPSystemDatabase system = CONST_COMMITABLE_CAST(SystemDatabase, Context::getContext()->getParent(shared_from_this()));
	if (system != 0) {
		Element* element = dim->lookupElement(elemId, false);

		// special check for user and group "admin"
		if (getId() == system->getUserGroupCube()->getId() || getId() == system->getGroupRoleCube()->getId() || getId() == system->getRoleRightObjectCube()->getId()) {
			if (element->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_ADMIN) {
				return RIGHT_READ;
			}
		}
		// special check for password
		else if (getId() == system->getUserUserPropertiesCube()->getId()) {
			if (element->getName(dim->getElemNamesVector()) == "password") {
				return User::checkUser(user) ? user->getRoleRight(User::passwordRight) : RIGHT_SPLASH;
			}
		}
	}

	return User::checkUser(user) ? user->getRoleRight(User::rightsRight) : RIGHT_SPLASH;
}

PCommitable RightsCube::copy() const
{
	checkNotCheckedOut();
	PRightsCube newd(new RightsCube(*this));
	return newd;
}

}
