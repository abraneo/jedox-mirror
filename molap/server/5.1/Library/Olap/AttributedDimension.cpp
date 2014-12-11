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

#include "Olap/AttributedDimension.h"
#include "Olap/AttributesDimension.h"
#include "Olap/AttributesCube.h"
#include "Olap/SystemDatabase.h"
#include "Olap/RightsCube.h"

namespace palo {

void AttributedDimension::addDimension(PServer server, PDatabase database, const Dimension *dimension, bool lookup, IdentifierType *attributesDimId, IdentifierType *attributesCubeId, bool useDimWorker)
{
	const string dimName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + dimension->getName() + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;
	PDimension attribDimension;
	if (lookup) {
		attribDimension = database->lookupDimensionByName(dimName, false);
	}
	if (!lookup || !attribDimension) {
		// create attributes dimension
		attribDimension = PAttributesDimension(new AttributesDimension(dimName));
		attribDimension->setDeletable(false);
		attribDimension->setRenamable(false);

		bool newId = true;
		if (attributesDimId && *attributesDimId != NO_IDENTIFIER) {
			attribDimension->setID(*attributesDimId);
			newId = false;
		}

		database->addDimension(server, attribDimension, true, newId, NULL, NULL, NULL, NULL, useDimWorker);

		if (attributesDimId) {
			*attributesDimId = attribDimension->getId();
		}
	}

	const string cubeName = Cube::PREFIX_ATTRIBUTE_CUBE + dimension->getName();
	if (!lookup || !database->lookupCubeByName(cubeName, false)) {
		IdentifiersType dimensions;
		dimensions.push_back(attribDimension->getId());
		dimensions.push_back(dimension->getId());

		PSystemCube attributesCube = PSystemCube(new AttributesCube(database, cubeName, &dimensions, true));
		attributesCube->setDeletable(false);
		attributesCube->setRenamable(false);

		bool newId = true;
		if (attributesCubeId && *attributesCubeId != NO_IDENTIFIER) {
			attributesCube->setID(*attributesCubeId);
			newId = false;
		}

		database->addCube(server, attributesCube, false, newId, NULL, NULL, NULL, useDimWorker);

		if (attributesCubeId) {
			*attributesCubeId = attributesCube->getId();
		}
	}
}

void AttributedDimension::removeDimension(PServer server, PDatabase database, const string &name, bool useDimWorker)
{
	// note: delete all system cubes using this dimension

	// delete attribute cube
	database->deleteCubeByName(server, Cube::PREFIX_ATTRIBUTE_CUBE + name, useDimWorker);

	// delete attribute dimension
	const string dimName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + name + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;
	PDimension attributesDimension = database->lookupDimensionByName(dimName, true);

	if (attributesDimension) {
		attributesDimension->setDeletable(true);
		database->deleteDimension(server, attributesDimension, PUser(), false, useDimWorker);
	}

}

void AttributedDimension::renameDimension(PServer server, PDatabase database, const string &newName, const string &oldName, bool useDimWorker)
{
	// rename attribute cube
	const string oldCubeName2 = Cube::PREFIX_ATTRIBUTE_CUBE + oldName;
	const string newCubeName2 = Cube::PREFIX_ATTRIBUTE_CUBE + newName;
	database->renameCube(server, oldCubeName2, newCubeName2, useDimWorker);

	// rename attribute dimension
	const string oldDimName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + oldName + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;
	const string newDimName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + newName + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;

	PDimension dim = database->lookupDimensionByName(oldDimName, true);
	PDimensionList dims = database->getDimensionList(true);
	database->setDimensionList(dims);
	dims->set(dim);

	if (dim) {
		database->renameDimension(server, dim, newDimName, true, useDimWorker);
	}

}

CPAttributesDimension AttributedDimension::getAttributesDimension(CPDatabase database, const string &name)
{
	const string str = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + name + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;

	PDimension d = database->lookupDimensionByName(str, false);
	if (d && d->getType() == SYSTEMTYPE) {
		PSystemDimension sd = COMMITABLE_CAST(SystemDimension, d);
		if (sd->getDimensionType() == Dimension::ATTRIBUTES) {
			return COMMITABLE_CAST(AttributesDimension, sd);
		}
	}
	return PAttributesDimension();
}

CPSystemCube AttributedDimension::getAttributesCube(CPDatabase database, const string &name)
{
	const string str = Cube::PREFIX_ATTRIBUTE_CUBE + name;
	PCube c = database->lookupCubeByName(str, false);
	if (c && c->getType() == SYSTEMTYPE) {
		PSystemCube sc = COMMITABLE_CAST(SystemCube, c);
		if (sc->getCubeType() == Cube::ATTRIBUTES) {
			return COMMITABLE_CAST(SystemCube, sc);
		}
	}
	return PSystemCube();
}

void DRCubeDimension::addCube(PServer server, PDatabase database, const Dimension *dimension, bool lookup, IdentifierType *rightsCubeId, bool useDimWorker)
{
	const string cubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + dimension->getName();
	if (!lookup || !database->lookupCubeByName(cubeName, false)) {
		PDimension groupDimension = database->findDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, PUser(), false);
		IdentifiersType dimensions;
		dimensions.push_back(groupDimension->getId());
		dimensions.push_back(dimension->getId());

		PRightsCube cube = PRightsCube(new RightsCube(database, cubeName,  &dimensions));
		cube->setDeletable(false);
		cube->setRenamable(false);

		bool newId = true;
		if (rightsCubeId && *rightsCubeId != NO_IDENTIFIER) {
			cube->setID(*rightsCubeId);
			newId = false;
		}

		database->addCube(server, cube, false, newId, NULL, NULL, NULL, useDimWorker);

		if (rightsCubeId) {
			*rightsCubeId = cube->getId();
		}
	}
}

void DRCubeDimension::removeCube(PServer server, PDatabase database, const string &dimName, bool useDimWorker)
{
	database->deleteCubeByName(server, SystemCube::PREFIX_GROUP_DIMENSION_DATA + dimName, useDimWorker);
}

void DRCubeDimension::renameCube(PServer server, PDatabase database, const string &newDimName, const string &oldDimName, bool useDimWorker)
{
	const string oldCubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + oldDimName;
	const string newCubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + newDimName;
	database->renameCube(server, oldCubeName, newCubeName, useDimWorker);
}

CPRightsCube DRCubeDimension::getRightsCube(CPDatabase database, const string &dimName)
{
	const string str = SystemCube::PREFIX_GROUP_DIMENSION_DATA + dimName;
	PCube c = database->lookupCubeByName(str, false);
	if (c && c->getType() == SYSTEMTYPE) {
		PSystemCube sc = COMMITABLE_CAST(SystemCube, c);
		if (sc->getCubeType() == Cube::RIGHTS) {
			return COMMITABLE_CAST(RightsCube, sc);
		}
	}
	return PRightsCube();
}

}
