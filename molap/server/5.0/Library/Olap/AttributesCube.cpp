/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#include "Olap/AttributesCube.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"

namespace palo {

AttributesCube::AttributesCube(PDatabase db, const string& name, const IdentifiersType* dimensions, bool dimension) :
	SystemCube(db, name, dimensions, Cube::ATTRIBUTES), rightObject(dimension ? User::dimensionRight : User::cubeRight), dimensionAttributes(dimension)
{
}

AttributesCube::AttributesCube(const AttributesCube& c) :
	SystemCube(c), rightObject(c.rightObject), dimensionAttributes(c.dimensionAttributes)
{
}

PCommitable AttributesCube::copy() const
{
	checkNotCheckedOut();
	PAttributesCube newd(new AttributesCube(*this));
	return newd;
}

void AttributesCube::checkCubeAccessRight(PUser user, RightsType minimumRight) const
{
	if (User::checkUser(user) && user->getRoleRight(rightObject) < minimumRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for attributes cube", "user", (int)user->getId());
	}
}

void AttributesCube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight) const
{
	if (User::checkUser(user)) {
		if (rightObject == User::dimensionRight && minimumRight == RIGHT_SPLASH) {
			// attribute cube of dimension, S is not valid right
			minimumRight = RIGHT_DELETE;
		}
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		RightsType rtMax = user->getRoleRight(rightObject);
		bool enough = rtMax >= minimumRight;
		if (enough) {
			RightsType rtMin = rtMax;
			user->computeDRight(db->getId(), cube, rtMin, rtMax);
			enough = rtMax >= minimumRight;
		}
		if (enough) {
			set<IdentifierType> userGroups;
			user->getUserGroupsCopy(userGroups);
			enough = user->checkDimsAndCells(db, cube, userGroups, area, true, false, minimumRight);
		}
		if (!enough) {
			throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)getId());
		}
	}
}

RightsType AttributesCube::getMinimumAccessRight(PUser user) const
{
	if (User::checkUser(user)) {
		RightsType rtMin = user->getRoleRight(rightObject);
		RightsType rtMax = rtMin;
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		CPCube cube = CONST_COMMITABLE_CAST(Cube, shared_from_this());
		user->computeDRight(db->getId(), cube, rtMin, rtMax);
		return rtMin;
	} else {
		return RIGHT_SPLASH;
	}
}

RightsType AttributesCube::getElementAccessRight(CPDatabase db, PUser user, CPDimension dim, IdentifierType elemId, vector<User::RoleCubeRight>& rcRights) const
{
	if (User::checkUser(user)) {
		RightsType rt1 = user->getRoleRight(rightObject);
		RightsType rt2 = Cube::getElementAccessRight(db, user, dim, elemId, rcRights);
		return min(rt1, rt2);
	} else {
		return RIGHT_SPLASH;
	}
}

PCellStream AttributesCube::calculatePropertiesArea(CPDatabase db, CPCube origCube, PCubeArea area) const
{
	return calculateArea(area, CubeArea::ALL, ALL_RULES, true, UNLIMITED_SORTED_PLAN);
}

PPlanNode AttributesCube::getCubeRightsPlan(PCubeArea area) const
{
	PPlanNode plan;
	string originalCubeName = area->getCube()->getName();
	if (originalCubeName.substr(0, SystemCube::PREFIX_CELL_PROPS_DATA.length()) == SystemCube::PREFIX_CELL_PROPS_DATA) {
		originalCubeName = originalCubeName.substr(SystemCube::PREFIX_CELL_PROPS_DATA.length());
		PCube originalCube = area->getDatabase()->lookupCubeByName(originalCubeName, false);
		if (originalCube) {
			Area originalArea(originalCube->getDimensions()->size());
			for (size_t dim = 0; dim < originalCube->getDimensions()->size(); dim++) {
				originalArea.insert(dim, area->getDim(dim));
			}
			PCubeArea originalCubeArea(new CubeArea(area->getDatabase(), originalCube, originalArea));
			plan.reset(new CellRightsPlanNode(originalCubeArea, true));
		}
	}
	//PCube sourceCube =
	return plan;
}

PPlanNode AttributesCube::createPlan(PCubeArea area, CubeArea::CellType type, RulesType paramRulesType, bool skipEmpty, uint64_t blockSize) const
{
	if (!dimensionAttributes && (paramRulesType & DIRECT_RULES)) {
		size_t cellPropDimOrdinal = area->dimCount()-1;
		CPSet cellPropDim = area->getDim(cellPropDimOrdinal);
		if (cellPropDim->size() == 1 && *cellPropDim->begin() == 0) {
			// only #_Rights queried
			return getCubeRightsPlan(area);
		} else {
			PSet complement;
			PSet rightSet(new Set());
			rightSet->insert(0); // insert #_Right element
			if (cellPropDim->intersection(rightSet.get(), 0, &complement)) {
				PCubeArea rightsArea(new CubeArea(*area));
				PCubeArea complementArea(new CubeArea(*area));
				rightsArea->insert(cellPropDimOrdinal, rightSet);
				complementArea->insert(cellPropDimOrdinal, complement);
				vector<PPlanNode> planNodes;
				planNodes.push_back(getCubeRightsPlan(area));
				planNodes.push_back(Cube::createPlan(complementArea, type, paramRulesType, skipEmpty, blockSize));
				return PPlanNode(new PlanNode(blockSize ? COMBINATION : SEQUENCE, area, 0, planNodes, 0, CPCube()));
			} else {
				// create standard plan
			}
		}
	}
	return Cube::createPlan(area, type, paramRulesType, skipEmpty, blockSize);
}

}
