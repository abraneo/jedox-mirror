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

#include "Olap/AttributesDimension.h"
#include "Olap/NormalDimension.h"
#include "Olap/AttributedDimension.h"
#include "Olap/Database.h"
#include "Olap/SystemDatabase.h"
#include "Olap/RightsCube.h"
#include "InputOutput/FileWriter.h"


namespace palo {
const string AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION = "#_";
const string AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION = "_";

void AttributesDimension::notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker)
{
	// create dimension data rights cube
	DRCubeDimension::addCube(server, database, this, false, rightsCubeId, useDimWorker);
}

void AttributesDimension::beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker)
{
	// delete dimension data rights cube
	DRCubeDimension::removeCube(server, database, getName(), useDimWorker);
}

void AttributesDimension::notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker)
{
	// rename dimension data rights cube
	DRCubeDimension::renameCube(server, database, getName(), oldName, useDimWorker);
}

CPNormalDimension AttributesDimension::getNormalDimension() const
{
	const string str = getName().substr(PREFIX_ATTRIBUTE_DIMENSION.length(), getName().length() - PREFIX_ATTRIBUTE_DIMENSION.length() - SUFFIX_ATTRIBUTE_DIMENSION.length());

	PDimension d = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()))->lookupDimensionByName(str, false);
	if (d && d->getType() == NORMALTYPE) {
		return COMMITABLE_CAST(NormalDimension, d);
	}

	return PNormalDimension();
}

PCommitable AttributesDimension::copy() const
{
	checkNotCheckedOut();
	PAttributesDimension newd(new AttributesDimension(*this));
	return newd;
}

}

