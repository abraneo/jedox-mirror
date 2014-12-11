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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Olap/User.h"
#include "Olap/Cube.h"
#include "Engine/CellRightProcessor.h"

namespace palo {

CellRightProcessor::CellRightProcessor(PCubeArea cubeArea, PUser user, bool forPropertyCube) :
		ProcessorBase(true, PEngineBase()), cubeArea(cubeArea), user(user), pathIt(cubeArea->pathBegin()), pathTranslator(cubeArea->getCube()->getPathTranslator()), forPropertyCube(forPropertyCube)
{
}

bool CellRightProcessor::next()
{
	bool result = false;
	if (pathIt != cubeArea->pathEnd()) {
		if (pathIt == cubeArea->pathBegin()) {
			// first cell
			vRights.clear();
			if (User::checkUser(user)) {
				user->fillRights(vRights, User::cellDataRight, cubeArea->getDatabase(), cubeArea->getCube());
			}
		}

		RightsType r = RIGHT_SPLASH;
		if (User::checkUser(user)) {
			r = user->getCellRight(cubeArea->getDatabase(), cubeArea->getCube(), *pathIt, vRights, 0);
		}
		cellValue = User::rightsTypeToString(r);
		key = *pathIt;
		if (forPropertyCube) {
			key.push_back(0);
		}
		++pathIt;
		result = true;
	}

	return result;
}

const CellValue &CellRightProcessor::getValue()
{
	return cellValue;
}

double CellRightProcessor::getDouble()
{
	return getValue().getNumeric();
}

const IdentifiersType &CellRightProcessor::getKey() const
{
	return key;
}

const GpuBinPath &CellRightProcessor::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void CellRightProcessor::reset()
{
	key.clear();
	pathIt = cubeArea->pathBegin();
}

}
