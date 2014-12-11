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

#include "Olap/DimensionDimension.h"
#include "Olap/AttributesDimension.h"
#include "Olap/AttributesCube.h"
#include "Olap/Database.h"

#include "InputOutput/FileWriter.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////


Element* DimensionDimension::addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	return SystemDimension::addElement(server, db, idElement, name, Element::STRING, user, useJournal);
}

void DimensionDimension::deleteElement(PServer server, PDatabase db, Element * element, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	return SystemDimension::deleteElement(server, db, element, user, useJournal, disabledRules, useDimWorker);
}

void DimensionDimension::changeElementName(PServer server, PDatabase db, Element * element, const string& name, PUser user, bool useJournal, bool useDimWorker)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	SystemDimension::changeElementName(server, db, element, name, user, useJournal, useDimWorker);
}

void DimensionDimension::changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove)
{
	if (user) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension is not changable", "user", (int)user->getId());
	}
	SystemDimension::changeElementType(server, db, element, Element::STRING, user, setConsolidated, disabledRules, elemsToDeleteFromCubes, doRemove);
}

////////////////////////////////////////////////////////////////////////////////
// notification callbacks
////////////////////////////////////////////////////////////////////////////////

void DimensionDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	// create attribute dimension and cube
	AttributedDimension::addDimension(server, database, this, false, attrDimId, attrCubeId, useDimWorker);
}

void DimensionDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	// note: delete all system cubes using this dimension

	// remove attribute dimension and cube
	AttributedDimension::removeDimension(server, database, getName(), useDimWorker);
}

void DimensionDimension::notifyRenameDimension(PServer server, PDatabase db, const string& oldName, bool useDimWorker)
{
	// rename attribute dimension and cube
	AttributedDimension::renameDimension(server, db, getName(), oldName, useDimWorker);
}

void DimensionDimension::checkElements(PServer server, PDatabase db)
{
	ElementsType elements = getElements(PUser(), false);

	for (ElementsType::iterator i = elements.begin(); i != elements.end(); ++i) {
		PDimension dimension = db->lookupDimensionByName((*i)->getName(getElemNamesVector()), true);
		if (!dimension || dimension->getType() != NORMALTYPE) {
			deleteElement(server, db, *i, PUser(), false, 0, false);
		}
	}

	vector<CPDimension> dimensions = db->getDimensions(PUser());

	for (vector<CPDimension>::iterator i = dimensions.begin(); i != dimensions.end(); ++i) {
		if ((*i)->getType() == NORMALTYPE) {
			Element* element = lookupElementByName((*i)->getName(), false);
			if (!element) {
				addElement(server, db, NO_IDENTIFIER, (*i)->getName(), Element::STRING, PUser(), false);
			}
		}

	}

}

PCommitable DimensionDimension::copy() const
{
	checkNotCheckedOut();
	PDimensionDimension newd(new DimensionDimension(*this));
	return newd;
}

}
