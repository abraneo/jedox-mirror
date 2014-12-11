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
 * 
 *
 */

#include "InputOutput/FileReader.h"
#include "InputOutput/FileWriter.h"

#include "Olap/SystemCube.h"
#include "Olap/Server.h"

#include "Thread/WriteLocker.h"

namespace palo {
const string SystemCube::PREFIX_GROUP_DIMENSION_DATA = "#_GROUP_DIMENSION_DATA_";
const string SystemCube::PREFIX_GROUP_CELL_DATA = "#_GROUP_CELL_DATA_";
const string SystemCube::PREFIX_CELL_PROPS_DATA = "#_CELL_PROPERTIES_";
const string SystemCube::GROUP_CUBE_DATA = "#_GROUP_CUBE_DATA";
const string SystemCube::CONFIGURATION_DATA = "#_CONFIGURATION";
const string SystemCube::NAME_VIEW_LOCAL_CUBE = "#_VIEW_LOCAL";
const string SystemCube::NAME_VIEW_GLOBAL_CUBE = "#_VIEW_GLOBAL";
const string SystemCube::NAME_SUBSET_LOCAL_CUBE = "#_SUBSET_LOCAL";
const string SystemCube::NAME_SUBSET_GLOBAL_CUBE = "#_SUBSET_GLOBAL";

void SystemCube::saveCube(PServer server, PDatabase db)
{
	saveCubeIntern(server, db, false, true);
}

void SystemCube::loadCube(PServer server, PDatabase db, bool processJournal)
{
	Logger::trace << "loading system cube '" << getName() << "'. " << endl;
	loadCubeIntern(server, db, processJournal, false, true);
}

void SystemCube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const
{
	if (defaultUsed) {
		*defaultUsed = false;
	}
	if (getCubeAccessRight(user) < minimumRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
	}
}

RightsType SystemCube::getMinimumAccessRight(CPUser user) const
{
	return getCubeAccessRight(user);
}

void SystemCube::checkCubeAccessRight(PUser user, RightsType minimumRight, bool checkGroupCubeData, bool checkCubeRightObject) const
{
	if (getCubeAccessRight(user) < minimumRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for cube", "user", (int)user->getId());
	}
}

RightsType SystemCube::getCubeAccessRight(CPUser user) const
{
	return User::checkUser(user) ? user->getRoleRight(User::sysOpRight) : RIGHT_SPLASH;
}

void SystemCube::clearCells(PServer server, PDatabase db, PUser user, bool useJournal)
{
	switch (saveType) {
	case CONFIGURATION:
	case JOBS:
	case LICENSES:
	case LOG:
	case SESSIONS:
		if (user) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube cannot be cleared", "user", user ? (int)user->getId() : 0);
		}
	default:
		Cube::clearCells(server, db, user, useJournal);
	}
}

void SystemCube::clearCells(PServer server, PDatabase db, PCubeArea areaElements, PUser user, bool useJournal)
{
	switch (saveType) {
	case CONFIGURATION:
	case JOBS:
	case LICENSES:
	case LOG:
	case SESSIONS:
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, getName() + " cube cannot be cleared", "user", user ? (int)user->getId() : 0);
	default:
		Cube::clearCells(server, db, areaElements, user, useJournal);
	}
}

}
