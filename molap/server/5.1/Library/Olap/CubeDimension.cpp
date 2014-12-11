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

#include "Olap/CubeDimension.h"

#include "InputOutput/FileWriter.h"

#include "Olap/SystemDatabase.h"
#include "Olap/AttributesDimension.h"
#include "Olap/AttributesCube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////


Element* CubeDimension::addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	return SystemDimension::addElement(server, db, idElement, name, Element::STRING, user, useJournal);
}

void CubeDimension::deleteElement(PServer server, PDatabase db, Element * element, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	SystemDimension::deleteElement(server, db, element, user, useJournal, disabledRules, useDimWorker);
}

void CubeDimension::changeElementName(PServer server, PDatabase db, Element * element, const string& name, PUser user, bool useJournal, bool useDimWorker)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	return SystemDimension::changeElementName(server, db, element, name, user, useJournal, useDimWorker);
}

void CubeDimension::changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	return SystemDimension::changeElementType(server, db, element, Element::STRING, user, setConsolidated, disabledRules, elemsToDeleteFromCubes, doRemove);
}

////////////////////////////////////////////////////////////////////////////////
// notification callbacks
////////////////////////////////////////////////////////////////////////////////

void CubeDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	// create attribute dimension and cube
	AttributedDimension::addDimension(server, database, this, false, attrDimId, attrCubeId, useDimWorker);
}

void CubeDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	// note: delete all system cubes using this dimension

	// remove attribute dimension and cube
	AttributedDimension::removeDimension(server, database, getName(), useDimWorker);
}

void CubeDimension::notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker)
{
	// rename attribute dimension and cube
	AttributedDimension::renameDimension(server, database, getName(), oldName, useDimWorker);
}

void CubeDimension::checkElements(PServer server, PDatabase database)
{

	ElementsType elements = getElements(PUser(), false);
	for (ElementsType::iterator i = elements.begin(); i != elements.end(); ++i) {
		PCube cube = database->lookupCubeByName((*i)->getName(getElemNamesVector()), true);
		if (!cube || (cube->getType() != NORMALTYPE && cube->getType() != GPUTYPE)) {
			deleteElement(server, database, *i, PUser(), false, 0, false);
		}
	}

	vector<CPCube> cubes = database->getCubes(PUser());

	for (vector<CPCube>::iterator i = cubes.begin(); i != cubes.end(); ++i) {
		if ((*i)->getType() == NORMALTYPE || (*i)->getType() == GPUTYPE) {
			Element* element = lookupElementByName((*i)->getName(), false);
			if (!element) {
				addElement(server, database, NO_IDENTIFIER, (*i)->getName(), Element::STRING, PUser(), false);
			}
		}

	}

}

PCommitable CubeDimension::copy() const
{
	checkNotCheckedOut();
	PCubeDimension newd(new CubeDimension(*this));
	return newd;
}

}
