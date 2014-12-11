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

#include "Olap/NormalDimension.h"

#include "InputOutput/FileWriter.h"

#include "Olap/AttributesCube.h"
#include "Olap/NormalDatabase.h"
#include "Olap/RightsCube.h"
#include "Olap/SystemDatabase.h"
#include "Olap/PaloSession.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// notification callbacks
////////////////////////////////////////////////////////////////////////////////

void NormalDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	// create attribute dimension and cube
	AttributedDimension::addDimension(server, database, this, false, attrDimId, attrCubeId, useDimWorker);
	// create dimension data rights cube
	DRCubeDimension::addCube(server, database, this, false, rightsCubeId, useDimWorker);

	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);
	if (db) {
		// add dimension to list of dimensions in #_DIMENSION_
		PDimension dimensionDimension = db->getDimensionDimension();
		if (dimensionDimension) {
			Element *e = dimensionDimension->addElement(server, database, (dimDimElemId ? *dimDimElemId : NO_IDENTIFIER), getName(), Element::STRING, PUser(), false);
			boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
			if (useDimWorker && (!session || !session->isWorker())) {
				dimensionDimension->addElementEvent(server, database, e);
			}
			if (dimDimElemId && e) {
				*dimDimElemId = e->getIdentifier();
			}
		}
	}
}

void NormalDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	// note: delete all system cubes using this dimension

	// remove attribute dimension and cube
	AttributedDimension::removeDimension(server, database, getName(), useDimWorker);
	// delete dimension data rights cube
	DRCubeDimension::removeCube(server, database, getName(), useDimWorker);

	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);
	if (db) {
		// delete dimension from list of dimensions in #_DIMENSION_
		PDimension dimensionDimension = db->getDimensionDimension();
		if (dimensionDimension) {
			Element * e = dimensionDimension->lookupElementByName(getName(), true);
			if (e) {
				dimensionDimension->deleteElement(server, database, e, PUser(), false, NULL, useDimWorker);
			}
		}
	}
}

void NormalDimension::notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker)
{
	// rename attribute dimension and cube
	AttributedDimension::renameDimension(server, database, getName(), oldName, useDimWorker);
	// rename dimension data rights cube
	DRCubeDimension::renameCube(server, database, getName(), oldName, useDimWorker);

	PNormalDatabase db = COMMITABLE_CAST(NormalDatabase, database);
	if (db) {
		// rename dimension in list of dimensions in #_DIMENSION_
		PDimension dimensionDimension = db->getDimensionDimension();
		if (dimensionDimension) {
			Element * e = dimensionDimension->lookupElementByName(oldName, true);
			if (e) {
				dimensionDimension->changeElementName(server, database, e, getName(), PUser(), false, useDimWorker);
			}
		}
	}
}

PCommitable NormalDimension::copy() const
{
	checkNotCheckedOut();
	PNormalDimension newd(new NormalDimension(*this));
	return newd;
}

}
