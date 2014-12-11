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

#ifndef OLAP_RIGHTS_CUBE_H
#define OLAP_RIGHTS_CUBE_H 1

#include "palo.h"

#include "Olap/SystemCube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief rights OLAP cube
///
/// An OLAP cube is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RightsCube : public SystemCube {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates empty cube
	///
	/// @throws ParameterException on double or missing dimensions
	////////////////////////////////////////////////////////////////////////////////

	RightsCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
		SystemCube(db, name, dimensions, Cube::RIGHTS) {
	}

	RightsCube(const RightsCube& c) :
		SystemCube(c) {
	}

	void saveCube(PServer server, PDatabase db);

	void loadCube(PServer server, PDatabase db, bool processJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clears all cells
	////////////////////////////////////////////////////////////////////////////////

	virtual void clearCells(PServer server, PDatabase db, PUser user, bool useJournal);

	virtual void clearCells(PServer server, PDatabase db, PCubeArea baseElements, PUser user, bool useJournal);

	virtual PRule createRule(PServer server, PDatabase db, PRuleNode, const string& definition, const string& external, const string& comment, bool activate, PUser, bool useJournal, IdentifierType *id, double position);

	virtual bool modifyRule(PServer server, PDatabase db, PRule, PRuleNode, const string& definition, const string& external, const string& comment, PUser, ActivationType activation, bool useJournal, double position);

	virtual bool activateRules(PServer server, PDatabase db, const vector<PRule> &, ActivationType activation, PUser, string* errMsg, bool bDefinitionChangedBefore, bool useJournal);

	virtual void deleteRule(PServer server, PDatabase db, IdentifierType id, PUser user, bool useJournal);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a value to a cell
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes element
	////////////////////////////////////////////////////////////////////////////////

	virtual bool deleteElement(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifierType element, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, bool completeRemove);
	virtual bool deleteElements(PServer server, PDatabase db, PUser user, const string& event, CPDimension dimension, IdentifiersType elements, CubeRulesArray* disabledRules, Dimension::DeleteCellType delType, PSet fullSet, bool completeRemove);

	virtual PCommitable copy() const;

	virtual void checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const;
	virtual RightsType getMinimumAccessRight(CPUser user) const;
	virtual RightsType getCubeAccessRight(CPUser user) const;

	virtual bool invalidateCache();
protected:
	void updateGroupDimensionDataToken(PDatabase db, PUser user, PArea baseElements);

private:
	void updateUserRights(PServer server, PDatabase db);
	bool isElementInSet(CPDatabase db, CPCubeArea area, uint32_t dimOrdinal, const string elemName) const;
};

}

#endif
