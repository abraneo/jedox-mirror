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

#ifndef OLAP_CONFIGURATION_CUBE_H
#define OLAP_CONFIGURATION_CUBE_H 1

#include "palo.h"

#include "Olap/SystemCube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief configuration OLAP cube
///
/// An OLAP cube is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ConfigurationCube : public SystemCube {
public:
	enum ClientCacheType {
		NO_CACHE, NO_CACHE_WITH_RULES, CACHE_ALL
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates empty cube
	///
	/// @throws ParameterException on double or missing dimensions
	////////////////////////////////////////////////////////////////////////////////

	ConfigurationCube(PDatabase db, const string& name, const IdentifiersType* dimensions);
	ConfigurationCube(const ConfigurationCube& c);

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions dealing with cells and cell values
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a value to a cell
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	void updateDatabase(PServer server, PDatabase database);

	virtual PCommitable copy() const;

	virtual RightsType getCubeAccessRight(CPUser user) const;

	IdentifierType checkElement(PServer server, PDatabase database, string elemName);

private:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief get value of cell path and set client cache type in parent database
	///
	/// if the value is missing the path ist set to 'N' (= NO_CACHE)
	////////////////////////////////////////////////////////////////////////////////

	void updateDatabaseClientCacheType(PServer server, PDatabase database, const CellValue &value);
	void updateDatabaseHideElements(PServer server, PDatabase database, const CellValue &value);
	void updateDatabaseDefaultRight(PServer server, PDatabase database, const CellValue &value);
};

}

#endif
