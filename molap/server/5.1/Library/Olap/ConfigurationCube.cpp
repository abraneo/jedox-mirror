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

#include "Olap/ConfigurationCube.h"
#include "Olap/Lock.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"
#include "Engine/EngineBase.h"

namespace palo {

ConfigurationCube::ConfigurationCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::CONFIGURATION)
{
}

ConfigurationCube::ConfigurationCube(const ConfigurationCube& c) :
	SystemCube(c)
{
}

ResultStatus ConfigurationCube::setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct)
{
	string newValue = value;
	bool updateCacheType = false;
	bool updateHideElements = false;
	bool updateDefaultRight = false;

	PDimension dim = db->lookupDimension(dimensions[0], false);
	Element *elem = dim->lookupElement(*cellPath->elemBegin(0), false);

	if (elem->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_CLIENT_CACHE_ELEMENT) {
		if (value.isEmpty()) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "cannot clear client cache cell", "value", SystemDatabase::NAME_CLIENT_CACHE_ELEMENT);
		}
		updateCacheType = true;
	} else if (elem->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT) {
		if (value.isEmpty()) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "cannot clear hide elements cell", "value", SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT);
		}
		updateHideElements = true;
	} else if (elem->getName(dim->getElemNamesVector()) == SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT) {
		if (value.isEmpty()) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "cannot clear default right cell", "value", SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT);
		}
		updateDefaultRight = true;
	}

	if (updateCacheType || updateHideElements || updateDefaultRight) {
		size_t l = value.length();

		if (l != 1) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "value not allowed here", "value", value);
		}

		string okStrings;
		if (updateDefaultRight) {
			okStrings = "NRWD";
		} else {
			okStrings = "NYE";
		}

		char valueChar = ::toupper(value[0]);
		newValue = string(1, valueChar);

		if (newValue.find_first_not_of(okStrings) != string::npos) {
			throw ParameterException(ErrorException::ERROR_INVALID_PERMISSION, "value not allowed here", "value", value);
		}
	}

	if (updateCacheType || updateHideElements || updateDefaultRight) {
		Cube::setCellValue(server, db, cellPath, newValue, lockedCells, user, session, false, false, DEFAULT, bWriteToJournal, checkRights, changedCubes, possibleCommit, ct);
		if (updateCacheType) {
			updateDatabaseClientCacheType(server, db, newValue);
		} else if (updateHideElements) {
			updateDatabaseHideElements(server, db, newValue);
		} else if (updateDefaultRight) {
			updateDatabaseDefaultRight(server, db, newValue);
		}
	} else {
		Cube::setCellValue(server, db, cellPath, newValue, lockedCells, user, session, checkArea, addValue, splashMode, bWriteToJournal, checkRights, changedCubes, possibleCommit, ct);
	}

	return RESULT_OK;
}

void ConfigurationCube::updateDatabase(PServer server, PDatabase database)
{
	IdentifierType IdClientCache = checkElement(server, database, SystemDatabase::NAME_CLIENT_CACHE_ELEMENT);
	IdentifierType IdHideElements = checkElement(server, database, SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT);
	IdentifierType IdDefaultRight = checkElement(server, database, SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT);

	PCube cube = COMMITABLE_CAST(Cube, shared_from_this());
	PCubeArea area(new CubeArea(database, cube, 1));
	PSet s(new Set);
	s->insert(IdClientCache);
	s->insert(IdHideElements);
	s->insert(IdDefaultRight);
	area->insert(0, s);

	PCellStream cs = calculateArea(area, CubeArea::ALL, ALL_RULES, false, UNLIMITED_UNSORTED_PLAN);
	while (cs->next()) {
		const IdentifiersType &key = cs->getKey();
		CellValue value = cs->getValue();

		if (key[0] == IdClientCache) {
			updateDatabaseClientCacheType(server, database, value);
		} else if (key[0] == IdHideElements) {
			updateDatabaseHideElements(server, database, value);
		} else { //IdDefaultRight
			if (value.isEmpty()) {
				set<PCube> changedCubes;
				PCubeArea path(new CubeArea(database, cube, key));
				string s("D");
				setCellValue(server, database, path, s, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
				commitChanges(false, PUser(), changedCubes, false);
			}
			updateDatabaseDefaultRight(server, database, value);
		}
	}
}

IdentifierType ConfigurationCube::checkElement(PServer server, PDatabase database, string elemName)
{
	Element *e = database->lookupDimension(dimensions.at(0), false)->lookupElementByName(elemName, false);
	if (!e) {
		PDimensionList dbs = database->getDimensionList(true);
		database->setDimensionList(dbs);
		PDimension dim = database->lookupDimension(dimensions.at(0), true);
		dbs->set(dim);
		e = dim->addElement(server, database, NO_IDENTIFIER, elemName, Element::STRING, PUser(), false);
	}
	return e->getIdentifier();
}

void ConfigurationCube::updateDatabaseClientCacheType(PServer server, PDatabase database, const CellValue &value)
{
	if (!value.isEmpty() && value.isString() && value.length() == 1) {
		char c = value[0];
		switch (c) {
		case 'N':
			database->setClientCacheType(ConfigurationCube::NO_CACHE);
			break;
		case 'Y':
			database->setClientCacheType(ConfigurationCube::NO_CACHE_WITH_RULES);
			break;
		case 'E':
			database->setClientCacheType(ConfigurationCube::CACHE_ALL);
			break;
		default:
			database->setClientCacheType(ConfigurationCube::NO_CACHE);
			break;
		}
	} else {
		database->setClientCacheType(ConfigurationCube::NO_CACHE);
	}
}

void ConfigurationCube::updateDatabaseHideElements(PServer server, PDatabase database, const CellValue &value)
{
	if (!value.isEmpty() && value.isString() && value.length() == 1) {
		char c = value[0];
		switch (c) {
		case 'Y':
			database->setHideElements(server, true);
			break;
		case 'N':
			database->setHideElements(server, false);
			break;
		default:
			database->setHideElements(server, false);
			break;
		}
	} else {
		database->setHideElements(server, false);
	}
}

void ConfigurationCube::updateDatabaseDefaultRight(PServer server, PDatabase database, const CellValue &value)
{
	if (!value.isEmpty() && value.isString() && value.length() == 1) {
		database->setDefaultRight(server, User::stringToRightsType(value));
	} else {
		database->setDefaultRight(server, RIGHT_SPLASH);
	}
}

RightsType ConfigurationCube::getCubeAccessRight(CPUser user) const
{
	if (User::checkUser(user)) {
		RightsType right = user->getRoleDbRight(User::sysOpRight, CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this())));
		return right == RIGHT_NONE ? RIGHT_READ : right;
	} else {
		return RIGHT_SPLASH;
	}
}

PCommitable ConfigurationCube::copy() const
{
	checkNotCheckedOut();
	PConfigurationCube newd(new ConfigurationCube(*this));
	return newd;
}

}
