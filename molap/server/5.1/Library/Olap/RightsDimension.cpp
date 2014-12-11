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
 * 
 *
 */

#include "Olap/RightsDimension.h"

#include "InputOutput/FileWriter.h"

#include "Olap/SystemDatabase.h"
#include "Olap/Server.h"
#include "Olap/Context.h"

namespace palo {

void RightsDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	if (getDimensionType() == CELLPROPS) {
		// create dimension data rights cube
		DRCubeDimension::addCube(server, database, this, false, rightsCubeId, useDimWorker);
	} else {
		SystemDimension::notifyAddDimension(server, database, attrDimId, attrCubeId, rightsCubeId, dimDimElemId, useDimWorker);
	}
}

void RightsDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	if (getDimensionType() == CELLPROPS) {
		// delete dimension data rights cube
		DRCubeDimension::removeCube(server, database, getName(), useDimWorker);
	} else {
		SystemDimension::beforeRemoveDimension(server, database, useDimWorker);
	}
}

void RightsDimension::notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker)
{
	if (getDimensionType() == CELLPROPS) {
		// rename dimension data rights cube
		DRCubeDimension::renameCube(server, database, getName(), oldName, useDimWorker);
	} else {
		SystemDimension::notifyRenameDimension(server, database, oldName, useDimWorker);
	}
}

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////


Element* RightsDimension::addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal)
{
	PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);

	if (getDimensionType() == Dimension::RIGHTS) {
		if (system != 0) {
			if (system->getUserDimension() && getId() == system->getUserDimension()->getId()) {
				elementType = Element::STRING;
			} else if (system->getGroupDimension() && getId() == system->getGroupDimension()->getId()) {
				elementType = Element::STRING;
			} else if (system->getRoleDimension() && getId() == system->getRoleDimension()->getId()) {
				elementType = Element::STRING;
			}
		}
	} else if (getDimensionType() == Dimension::CONFIG) {
		elementType = Element::STRING;
	}

	return SystemDimension::addElement(server, db, idElement, name, elementType, user, useJournal);
}

void RightsDimension::changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove)
{
	PSystemDatabase system = COMMITABLE_CAST(SystemDatabase, db);

	if (getDimensionType() == Dimension::RIGHTS) {
		if (system != 0) {
			PDimension dim = system->getUserDimension();
			if (dim && getId() == dim->getId()) {
				elementType = Element::STRING;
			} else {
				dim = system->getGroupDimension();
				if (dim && getId() == dim->getId()) {
					elementType = Element::STRING;
				} else {
					dim = system->getRoleDimension();
					if (dim && getId() == dim->getId()) {
						elementType = Element::STRING;
					}
				}
			}
		}
	} else if (getDimensionType() == Dimension::CONFIG) {
		elementType = Element::STRING;
	}

	SystemDimension::changeElementType(server, db, element, elementType, user, setConsolidated, disabledRules, elemsToDeleteFromCubes, doRemove);
}

void RightsDimension::checkElementAccessRight(const User *user, CPDatabase db, RightsType minimumRight) const
{
	RightsType rt = getElementAccessRight(user, db);
	if (rt < minimumRight && (getDimensionType() != Dimension::CONFIG || minimumRight > RIGHT_READ)) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
	}
}

RightsType RightsDimension::getElementAccessRight(const User *user, CPDatabase db) const
{
	RightsType rt = RIGHT_DELETE;
	if (User::checkUser(user)) {
		bool elemRight = true;
		if (getDimensionType() == Dimension::RIGHTS) {
			PSystemDatabase system = Context::getContext()->getServer()->getSystemDatabase();

			if (getId() == system->getUserDimension()->getId()) {
				rt = user->getRoleDbRight(User::userRight, db);
				if (rt == RIGHT_NONE && user->getRoleDbRight(User::subSetViewRight, db) > RIGHT_NONE) {
					rt = RIGHT_READ;
				}
				elemRight = false;
			} else if (getId() == system->getGroupDimension()->getId()) {
				rt = user->getRoleDbRight(User::groupRight, db);
				if (rt == RIGHT_NONE && user->getRoleDbRight(User::subSetViewRight, db) > RIGHT_NONE) {
					rt = RIGHT_READ;
				}
				elemRight = false;
			} else if (getId() == system->getRoleDimension()->getId()) {
				rt = user->getRoleDbRight(User::rightsRight, db);
				elemRight = false;
			}
		}
		if (elemRight) {
			rt = user->getRoleDbRight(User::elementRight, db);
		}
	}
	return rt;
}

RightsType RightsDimension::getDimensionDataRight(const User *user) const
{
	if (getDimensionType() == Dimension::RIGHTS) {
		return User::checkUser(user) ? user->getRoleRight(User::rightsRight) : RIGHT_DELETE;
	} else {
		return SystemDimension::getDimensionDataRight(user);
	}
}

PCommitable RightsDimension::copy() const
{
	checkNotCheckedOut();
	PRightsDimension newd(new RightsDimension(*this));
	return newd;
}

}
