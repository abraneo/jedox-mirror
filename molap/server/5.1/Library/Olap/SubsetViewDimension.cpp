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

#include "Olap/SubsetViewDimension.h"
#include "Olap/Database.h"

#include "InputOutput/FileWriter.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////


Element* SubsetViewDimension::addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal)
{
	return SystemDimension::addElement(server, db, idElement, name, Element::STRING, user, useJournal);
}

void SubsetViewDimension::changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove)
{
	return SystemDimension::changeElementType(server, db, element, Element::STRING, user, setConsolidated, disabledRules, elemsToDeleteFromCubes, doRemove);
}

////////////////////////////////////////////////////////////////////////////////
// notification callbacks
////////////////////////////////////////////////////////////////////////////////

void SubsetViewDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	// create attribute dimension and cube
	AttributedDimension::addDimension(server, database, this, false, attrDimId, attrCubeId, useDimWorker);
}

void SubsetViewDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	// note: delete all system cubes using this dimension

	// remove attribute dimension and cube
	AttributedDimension::removeDimension(server, database, getName(), useDimWorker);
}

void SubsetViewDimension::notifyRenameDimension(PServer server, PDatabase db, const string& oldName, bool useDimWorker)
{
	// rename attribute dimension and cube
	AttributedDimension::renameDimension(server, db, getName(), oldName, useDimWorker);
}

PCommitable SubsetViewDimension::copy() const
{
	checkNotCheckedOut();
	PSubsetViewDimension newd(new SubsetViewDimension(*this));
	return newd;
}

}
