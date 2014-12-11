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
 * 
 *
 */

#ifndef OLAP_SYSTEM_CUBE_H
#define OLAP_SYSTEM_CUBE_H 1

#include "palo.h"

#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief system OLAP cube
///
/// An OLAP cube is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SystemCube : public Cube
{
public:
	static const string PREFIX_GROUP_DIMENSION_DATA;
	static const string PREFIX_GROUP_CELL_DATA;
	static const string PREFIX_CELL_PROPS_DATA;
	static const string GROUP_CUBE_DATA;
	static const string CONFIGURATION_DATA;
	static const string NAME_VIEW_LOCAL_CUBE;
	static const string NAME_VIEW_GLOBAL_CUBE;
	static const string NAME_SUBSET_LOCAL_CUBE;
	static const string NAME_SUBSET_GLOBAL_CUBE;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates empty cube
	///
	/// @throws ParameterException on double or missing dimensions
	////////////////////////////////////////////////////////////////////////////////

	SystemCube(PDatabase db, const string& name, const IdentifiersType* dimensions, SaveType saveType) :
		Cube(db, name, dimensions, saveType) {
	}

	SystemCube(const SystemCube& c) :
		Cube(c) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name notification callbacks
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been added to a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyAddCube(PServer server, PDatabase database, IdentifierType *newCubeDimElem, IdentifierType *newCellRightCube, IdentifierType *newCellPropsCube, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been removed from a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRemoveCube(PServer server, PDatabase database, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a cube has been renamed
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRenameCube(PServer server, PDatabase database, const string& oldName, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	virtual bool supportsAggregations() const {return true;}

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief saves data to file
	////////////////////////////////////////////////////////////////////////////////

	void saveCube(PServer server, PDatabase db);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief reads data from file
	////////////////////////////////////////////////////////////////////////////////

	void loadCube(PServer server, PDatabase db, bool processJournal);

	virtual void checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const;
	virtual RightsType getMinimumAccessRight(CPUser user) const;
	virtual void checkCubeAccessRight(PUser user, RightsType minimumRight, bool checkGroupCubeData, bool checkCubeRightObject) const;
	virtual RightsType getCubeAccessRight(CPUser user) const;

	virtual void clearCells(PServer server, PDatabase db, PUser user, bool useJournal);
	virtual void clearCells(PServer server, PDatabase db, PCubeArea areaElements, PUser user, bool useJournal);
};

}

#endif
