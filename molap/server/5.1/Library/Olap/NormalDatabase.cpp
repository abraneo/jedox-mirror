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

#include "Olap/NormalDatabase.h"
#include "Olap/ConfigurationCube.h"

#include <iostream>

#include "Exceptions/FileFormatException.h"

#include "Olap/CubeDimension.h"
#include "Olap/DimensionDimension.h"
#include "Olap/RightsCube.h"
#include "Olap/AttributesCube.h"
#include "Olap/RightsDimension.h"
#include "Olap/AttributedDimension.h"
#include "Olap/Server.h"
#include "Olap/SubsetViewCube.h"
#include "Olap/SubsetViewDimension.h"
#include "Olap/User.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// constructors and destructors
////////////////////////////////////////////////////////////////////////////////

NormalDatabase::NormalDatabase(const string& name) :
	Database(name), cubeDimensionId(NO_IDENTIFIER), dimensionDimensionId(NO_IDENTIFIER), cellPropsDimensionId(NO_IDENTIFIER)
{
}

NormalDatabase::NormalDatabase(const NormalDatabase& d) :
	Database(d), cubeDimension(d.cubeDimension), dimensionDimension(d.dimensionDimension), cellPropsDimension(d.cellPropsDimension),
	cubeDimensionId(d.cubeDimensionId), dimensionDimensionId(d.dimensionDimensionId), cellPropsDimensionId(d.cellPropsDimensionId)
{
}

////////////////////////////////////////////////////////////////////////////////
// notification callbacks
////////////////////////////////////////////////////////////////////////////////

void NormalDatabase::notifyAddDatabase(PServer server, PUser user, bool useDimWorker)
{
	server->updateDatabaseDim(SystemDatabase::ADD, getName(), getName(), user, useDimWorker);
	if (status != UNLOADED) {
		createSystemItems(server, false);
	}
}

void NormalDatabase::notifyRemoveDatabase(PServer server, bool useDimWorker)
{
	server->updateDatabaseDim(SystemDatabase::REMOVE, getName(), getName(), PUser(), useDimWorker);
}

void NormalDatabase::notifyRenameDatabase(PServer server, const string& oldName, bool useDimWorker)
{
	server->updateDatabaseDim(SystemDatabase::RENAME, getName(), oldName, PUser(), useDimWorker);
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a dimension
////////////////////////////////////////////////////////////////////////////////

void NormalDatabase::saveDatabaseType(FileWriter* file)
{
	file->appendInteger(getId());
	file->appendEscapeString(getName());
	file->appendInteger(getDatabaseType());
	file->appendInteger(deletable ? 1 : 0);
	file->appendInteger(renamable ? 1 : 0);
	file->appendInteger(extensible ? 1 : 0);
	file->nextLine();
}

void NormalDatabase::loadDatabase(PServer server, bool addSystemDimension)
{
	if (status == LOADED) {
		return;
	}

	Database::loadDatabase(server, true);

	findDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, PUser(), false);
	findDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, PUser(), false);
	findCubeByName(SystemCube::GROUP_CUBE_DATA, PUser(), true, false);

	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());
	if (cubeDimension) {
		// in write mode
		PCubeDimension cd = COMMITABLE_CAST(CubeDimension, cubeDimension);
		if (cd) {
			cd->checkElements(server, db);
		}

		PDimensionDimension dd = COMMITABLE_CAST(DimensionDimension, dimensionDimension);
		if (dd) {
			dd->checkElements(server, db);
		}
	} else {
		Logger::info << "Cubes dimension for database '" << getName() << "' not found." << endl;
	}
}

////////////////////////////////////////////////////////////////////////////////
// other stuff
////////////////////////////////////////////////////////////////////////////////

bool NormalDatabase::createSystemItems(PServer server, bool forceCreate) //forceCreate is not used
{
	checkCheckedOut();
	bool dbChanged = false;

	PDatabase systemDB = server->findDatabaseByName(Server::NAME_SYSTEM_DATABASE, PUser(), true, false);
	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());

	// create alias dimension for #_USER_
	PDimension sourceDimension = systemDB->findDimensionByName(SystemDatabase::NAME_USER_DIMENSION, PUser(), false);
	PDimension userDimension = lookupDimensionByName(SystemDatabase::NAME_USER_DIMENSION, false);

	if (userDimension == 0) {
		userDimension = addAliasDimension(server, SystemDatabase::NAME_USER_DIMENSION, sourceDimension, false);
		userDimension->setDeletable(false);
		userDimension->setRenamable(false);
		userDimension->setChangable(false);
		dbChanged = true;
	} else if (userDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("alias dimension '" + SystemDatabase::NAME_USER_DIMENSION + "' corrupted", 0);
	}

	// create alias dimension for #_GROUP_
	sourceDimension = systemDB->findDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, PUser(), false);
	PDimension groupDimension = lookupDimensionByName(SystemDatabase::NAME_GROUP_DIMENSION, false);

	if (groupDimension == 0) {
		groupDimension = addAliasDimension(server, SystemDatabase::NAME_GROUP_DIMENSION, sourceDimension, false);
		groupDimension->setDeletable(false);
		groupDimension->setRenamable(false);
		groupDimension->setChangable(false);
		dbChanged = true;
	} else if (groupDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("alias dimension '" + SystemDatabase::NAME_GROUP_DIMENSION + "' corrupted", 0);
	}

	// create alias dimension for #_CUBE_PROPERTIES_
	sourceDimension = systemDB->findDimensionByName(SystemDatabase::NAME_CUBE_PROPERTIES_DIMENSION, PUser(), false);
	PDimension cubePropsDimension = lookupDimensionByName(SystemDatabase::NAME_CUBE_PROPERTIES_DIMENSION, false);

	if (cubePropsDimension == 0) {
		cubePropsDimension = addAliasDimension(server, SystemDatabase::NAME_CUBE_PROPERTIES_DIMENSION, sourceDimension, false);
		cubePropsDimension->setDeletable(false);
		cubePropsDimension->setRenamable(false);
		cubePropsDimension->setChangable(false);
		dbChanged = true;
	} else if (cubePropsDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("alias dimension '" + SystemDatabase::NAME_CUBE_PROPERTIES_DIMENSION + "' corrupted", 0);
	}

	// create cube dimension for #_CUBE_
	cubeDimension = lookupDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, false);

	if (cubeDimension == 0) {
		cubeDimension = PDimension(new CubeDimension(SystemDatabase::NAME_CUBE_DIMENSION));
		cubeDimension->setDeletable(false);
		cubeDimension->setRenamable(false);
		cubeDimension->setChangable(false);
		addDimension(server, cubeDimension, true, true, NULL, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (cubeDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("cube dimension '" + SystemDatabase::NAME_CUBE_DIMENSION + "' corrupted", 0);
	}

	// create cube #_GROUP_CUBE_DATA
	PCube groupCube = lookupCubeByName(SystemCube::GROUP_CUBE_DATA, false);

	if (groupCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(groupDimension->getId());
		dimensions.push_back(cubeDimension->getId());

		groupCube = PCube(new RightsCube(db, SystemCube::GROUP_CUBE_DATA, &dimensions));
		groupCube->setDeletable(false);
		groupCube->setRenamable(false);

		addCube(server, groupCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (groupCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("group cube '" + SystemCube::GROUP_CUBE_DATA + "' corrupted", 0);
	}

	// create configuration dimension #_CONFIGURATION_ with client cache element
	PDimension configurationDimension = lookupDimensionByName(SystemDatabase::NAME_CONFIGURATION_DIMENSION, false);

	if (configurationDimension == 0) {
		configurationDimension = PDimension(new RightsDimension(SystemDatabase::NAME_CONFIGURATION_DIMENSION, Dimension::CONFIG));
		configurationDimension->setDeletable(false);
		configurationDimension->setRenamable(false);
		configurationDimension->setChangable(true);
		addDimension(server, configurationDimension, true, true, NULL, NULL, NULL, NULL, false);

		configurationDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_CLIENT_CACHE_ELEMENT, Element::STRING, PUser(), false);
		configurationDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT, Element::STRING, PUser(), false);
		configurationDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT, Element::STRING, PUser(), false);
		dbChanged = true;
	} else if (configurationDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("configuration dimension '" + SystemDatabase::NAME_CONFIGURATION_DIMENSION + "' corrupted", 0);
	}

	// create cube #_CONFIGURATION
	PCube configurationCube = lookupCubeByName(SystemCube::CONFIGURATION_DATA, false);

	if (configurationCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(configurationDimension->getId());

		configurationCube = PCube(new ConfigurationCube(db, SystemCube::CONFIGURATION_DATA, &dimensions));
		configurationCube->setDeletable(false);
		configurationCube->setRenamable(false);

		addCube(server, configurationCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (configurationCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("configuration cube '" + SystemCube::CONFIGURATION_DATA + "' corrupted", 0);
	}
	PConfigurationCube cc = COMMITABLE_CAST(ConfigurationCube, configurationCube);
	cc->updateDatabase(server, db);

	// create dimension dimension #_DIMENSION_
	dimensionDimension = lookupDimensionByName(SystemDatabase::NAME_DIMENSION_DIMENSION, false);

	if (dimensionDimension == 0) {
		dimensionDimension = PDimension(new DimensionDimension(SystemDatabase::NAME_DIMENSION_DIMENSION));
		dimensionDimension->setDeletable(false);
		dimensionDimension->setRenamable(false);
		dimensionDimension->setChangable(false);
		addDimension(server, dimensionDimension, true, true, NULL, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (dimensionDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("dimension dimension '" + SystemDatabase::NAME_DIMENSION_DIMENSION + "' corrupted", 0);
	}

	// create subset dimension #_SUBSET_
	PDimension subsetDimension = lookupDimensionByName(SystemDatabase::NAME_SUBSET_DIMENSION, false);

	if (subsetDimension == 0) {
		subsetDimension = PDimension(new SubsetViewDimension(SystemDatabase::NAME_SUBSET_DIMENSION));
		subsetDimension->setDeletable(false);
		subsetDimension->setRenamable(false);
		addDimension(server, subsetDimension, true, true, NULL, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (subsetDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("subset dimension '" + SystemDatabase::NAME_SUBSET_DIMENSION + "' corrupted", 0);
	}

	// create view dimension #_VIEW_
	PDimension viewDimension = lookupDimensionByName(SystemDatabase::NAME_VIEW_DIMENSION, false);

	if (viewDimension == 0) {
		viewDimension = PDimension(new SubsetViewDimension(SystemDatabase::NAME_VIEW_DIMENSION));
		viewDimension->setDeletable(false);
		viewDimension->setRenamable(false);
		addDimension(server, viewDimension, true, true, NULL, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (viewDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("view dimension '" + SystemDatabase::NAME_VIEW_DIMENSION + "' corrupted", 0);
	}

	cellPropsDimension = lookupDimensionByName(SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION, false);

	if (cellPropsDimension == 0) {
		cellPropsDimension = PDimension(new RightsDimension(SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION, Dimension::CELLPROPS));
		cellPropsDimension->setDeletable(false);
		cellPropsDimension->setRenamable(false);
		cellPropsDimension->setChangable(true);
		addDimension(server, cellPropsDimension, true, true, NULL, NULL, NULL, NULL, false);
		cellPropsDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_RIGHTS_ELEMENT, Element::STRING, PUser(), false);
		cellPropsDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_FORMAT_STRING_ELEMENT, Element::STRING, PUser(), false);
		cellPropsDimension->addElement(server, db, NO_IDENTIFIER, SystemDatabase::NAME_CELL_NOTE_ELEMENT, Element::STRING, PUser(), false);
		dbChanged = true;
	} else if (cellPropsDimension->getType() != SYSTEMTYPE) {
		throw FileFormatException("cell properties  dimension '" + SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION + "' corrupted", 0);
	}

	// create cube #_SUBSET_LOCAL
	PCube subsetLocalCube = lookupCubeByName(SystemCube::NAME_SUBSET_LOCAL_CUBE, false);

	if (subsetLocalCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(dimensionDimension->getId());
		dimensions.push_back(userDimension->getId());
		dimensions.push_back(subsetDimension->getId());

		subsetLocalCube = PCube(new SubsetViewCube(db, SystemCube::NAME_SUBSET_LOCAL_CUBE, &dimensions));
		subsetLocalCube->setDeletable(false);
		subsetLocalCube->setRenamable(false);

		addCube(server, subsetLocalCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (subsetLocalCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("subset local cube '" + SystemCube::NAME_SUBSET_LOCAL_CUBE + "' corrupted", 0);
	}

	// create cube #_SUBSET_GLOBAL
	PCube subsetGlobalCube = lookupCubeByName(SystemCube::NAME_SUBSET_GLOBAL_CUBE, false);

	if (subsetGlobalCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(dimensionDimension->getId());
		dimensions.push_back(subsetDimension->getId());

		subsetGlobalCube = PCube(new SubsetViewCube(db, SystemCube::NAME_SUBSET_GLOBAL_CUBE, &dimensions));
		subsetGlobalCube->setDeletable(false);
		subsetGlobalCube->setRenamable(false);

		addCube(server, subsetGlobalCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (subsetLocalCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("subset global cube '" + SystemCube::NAME_SUBSET_GLOBAL_CUBE + "' corrupted", 0);
	}

	// create cube #_VIEW_LOCAL
	PCube viewLocalCube = lookupCubeByName(SystemCube::NAME_VIEW_LOCAL_CUBE, false);

	if (viewLocalCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(cubeDimension->getId());
		dimensions.push_back(userDimension->getId());
		dimensions.push_back(viewDimension->getId());

		viewLocalCube = PCube(new SubsetViewCube(db, SystemCube::NAME_VIEW_LOCAL_CUBE, &dimensions));
		viewLocalCube->setDeletable(false);
		viewLocalCube->setRenamable(false);

		addCube(server, viewLocalCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (viewLocalCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("view local cube '" + SystemCube::NAME_VIEW_LOCAL_CUBE + "' corrupted", 0);
	}

	// create cube #_VIEW_GLOBAL
	PCube viewGlobalCube = lookupCubeByName(SystemCube::NAME_VIEW_GLOBAL_CUBE, false);

	if (viewGlobalCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(cubeDimension->getId());
		dimensions.push_back(viewDimension->getId());

		viewGlobalCube = PCube(new SubsetViewCube(db, SystemCube::NAME_VIEW_GLOBAL_CUBE, &dimensions));
		viewGlobalCube->setDeletable(false);
		viewGlobalCube->setRenamable(false);

		addCube(server, viewGlobalCube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else if (viewGlobalCube->getType() != SYSTEMTYPE) {
		throw FileFormatException("view global cube '" + SystemCube::NAME_VIEW_GLOBAL_CUBE + "' corrupted", 0);
	}

	// check the existence of attribute dimensions, attribute cubes and dimension data rights cubes
	vector<CPDimension> dims = getDimensions(PUser());
	for (vector<CPDimension>::const_iterator it = dims.begin(); it != dims.end(); ++it) {
		CPDimension dim = *it;
		if (dim->isAttributed()) {
			AttributedDimension::addDimension(server, db, dim.get(), true, NULL, NULL, false);
			dbChanged = true;
		}
		if (dim->hasRightsCube()) {
			DRCubeDimension::addCube(server, db, dim.get(), true, NULL, false);
			if (dim->getName() == SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION) {
				fillRightProperty(server);
			}
			dbChanged = true;
		}
	}

	// check existence of cell data rights cube
	vector<CPCube> cubes = getCubes(PUser());
	for (vector<CPCube>::const_iterator i = cubes.begin(); i != cubes.end(); ++i) {
		CPCube cube = CONST_COMMITABLE_CAST(Cube, *i);

		if (cube != 0 && cube->getType() == NORMALTYPE) {
			string cubeName = SystemCube::PREFIX_GROUP_CELL_DATA + cube->getName();
			PCube groupCubeDataCube = lookupCubeByName(cubeName, false);
			if (!groupCubeDataCube) {
				IdentifiersType vDims;
				vDims.push_back(groupDimension->getId());
				const IdentifiersType* cubeDims = cube->getDimensions();
				for (IdentifiersType::const_iterator i = cubeDims->begin(); i != cubeDims->end(); ++i) {
					vDims.push_back(*i);
				}

				PRightsCube rightsCube = PRightsCube(new RightsCube(db, cubeName, &vDims));
				rightsCube->setDeletable(false);
				rightsCube->setRenamable(false);

				addCube(server, rightsCube, false, true, NULL, NULL, NULL, false);
				dbChanged = true;
			}

			cubeName = SystemCube::PREFIX_CELL_PROPS_DATA + cube->getName();
			PCube cellPropsCube = lookupCubeByName(cubeName, false);
			if (!cellPropsCube) {
				IdentifiersType vDims;
				const IdentifiersType* cubeDims = cube->getDimensions();
				for (IdentifiersType::const_iterator i = cubeDims->begin(); i != cubeDims->end(); ++i) {
					vDims.push_back(*i);
				}
				vDims.push_back(cellPropsDimension->getId());

				PAttributesCube cellPropsAttrCube = PAttributesCube(new AttributesCube(db, cubeName, &vDims, false));
				cellPropsAttrCube->setDeletable(false);
				cellPropsAttrCube->setRenamable(false);

				addCube(server, cellPropsAttrCube, false, true, NULL, NULL, NULL, false);
				dbChanged = true;
			}
		}
	}
	return dbChanged;
}

void NormalDatabase::fillRightProperty(PServer server)
{
	const string cubeName = SystemCube::PREFIX_GROUP_DIMENSION_DATA + SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION;
	PCube cube = lookupCubeByName(cubeName, true);
	PDimension dim0 = lookupDimension((*cube->getDimensions())[0], false);
	PDimension dim1 = lookupDimension((*cube->getDimensions())[1], false);

	ElementsType elements = dim0->getElements(PUser(), false);
	Element *rightElem = dim1->lookupElementByName(SystemDatabase::NAME_RIGHTS_ELEMENT, false);

	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());
	PCubeArea area(new CubeArea(db, cube, 2));
	PSet s(new Set);
	for (ElementsType::const_iterator it = elements.begin(); it != elements.end(); ++it) {
		s->insert((*it)->getIdentifier());
	}
	area->insert(0, s);
	s.reset(new Set);
	s->insert(rightElem->getIdentifier());
	area->insert(1, s);

	set<PCube> changedCubes;
	PCellStream cs = cube->calculateArea(area, CubeArea::BASE_STRING, NO_RULES, false, 0);
	string r("R");
	while (cs->next()) {
		CellValue value = cs->getValue();
		if ((string)value != r) {
			PCubeArea path(new CubeArea(db, cube, cs->getKey()));
			cube->setCellValue(server, db, path, r, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::BASE_STRING);
		}
	}
	cube->commitChanges(false, PUser(), changedCubes, false);
}

PCommitable NormalDatabase::copy() const
{
	checkNotCheckedOut();
	PNormalDatabase newd(new NormalDatabase(*this));
	return newd;
}

bool NormalDatabase::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = Database::merge(o, p);

	if (ret) {
		if (cubeDimensionId == NO_IDENTIFIER) {
			cubeDimension = lookupDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, false);
			if (cubeDimension) {
				cubeDimensionId = cubeDimension->getId();
			}
		} else {
			cubeDimension = lookupDimension(cubeDimensionId, false);
		}
		if (dimensionDimensionId == NO_IDENTIFIER) {
			dimensionDimension = lookupDimensionByName(SystemDatabase::NAME_DIMENSION_DIMENSION, false);
			if (dimensionDimension) {
				dimensionDimensionId = dimensionDimension->getId();
			}
		} else {
			dimensionDimension = lookupDimension(dimensionDimensionId, false);
		}
		if (cellPropsDimensionId == NO_IDENTIFIER) {
			cellPropsDimension = lookupDimensionByName(SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION, false);
			if (cellPropsDimension) {
				cellPropsDimensionId = cellPropsDimension->getId();
			}
		} else {
			cellPropsDimension = lookupDimension(cellPropsDimensionId, false);
		}
	}

	return ret;
}

PDimension NormalDatabase::getDimensionDimension()
{
	if (!dimensionDimension && isCheckedOut()) {
		dimensionDimension = lookupDimensionByName(SystemDatabase::NAME_DIMENSION_DIMENSION, false);
	}
	if (!dimensionDimension->isCheckedOut()) {
		checkCheckedOut();

		PDimensionList dimList = getDimensionList(true);
		setDimensionList(dimList);
		dimensionDimension = lookupDimensionByName(SystemDatabase::NAME_DIMENSION_DIMENSION, true);
		if (dimensionDimension) {
			dimList->set(dimensionDimension);
		}
	}
	return dimensionDimension;
}

PDimension NormalDatabase::getCubeDimension()
{
	if (!cubeDimension && isCheckedOut()) {
		cubeDimension = lookupDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, false);
	}
	if (!cubeDimension->isCheckedOut()) {
		checkCheckedOut();

		PDimensionList dimList = getDimensionList(true);
		setDimensionList(dimList);
		cubeDimension = lookupDimensionByName(SystemDatabase::NAME_CUBE_DIMENSION, true);
		if (cubeDimension) {
			dimList->set(cubeDimension);
		}
	}
	return cubeDimension;
}

}
