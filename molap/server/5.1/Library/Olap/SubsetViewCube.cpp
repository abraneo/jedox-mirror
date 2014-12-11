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

#include "Olap/SubsetViewCube.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"

namespace palo {

SubsetViewCube::SubsetViewCube(PDatabase db, const string& name, const IdentifiersType* dimensions) :
	SystemCube(db, name, dimensions, Cube::SUBSETVIEW)
{
	if (name == SystemCube::NAME_SUBSET_GLOBAL_CUBE || name == SystemCube::NAME_VIEW_GLOBAL_CUBE) {
		isGlobalCube = true;
	} else {
		isGlobalCube = false;
	}
}

SubsetViewCube::SubsetViewCube(const SubsetViewCube& c) :
	SystemCube(c), isGlobalCube(c.isGlobalCube)
{
}

PCommitable SubsetViewCube::copy() const
{
	checkNotCheckedOut();
	PSubsetViewCube newd(new SubsetViewCube(*this));
	return newd;
}

bool SubsetViewCube::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	CPSubsetViewCube db = CONST_COMMITABLE_CAST(SubsetViewCube, o);
	CPSubsetViewCube olddb = CONST_COMMITABLE_CAST(SubsetViewCube, old);
	if (old != 0 && o != 0 && isGlobalCube == olddb->isGlobalCube) {
		isGlobalCube = db->isGlobalCube;
	}
	return Cube::merge(o, p);
}

void SubsetViewCube::checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const
{
	if (defaultUsed) {
		*defaultUsed = false;
	}
	if (minimumRight == RIGHT_WRITE && isGlobalCube) {
		minimumRight = RIGHT_DELETE;
	}
	if (getCubeAccessRight(user) < minimumRight) {
		throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
	}
}

RightsType SubsetViewCube::getCubeAccessRight(CPUser user) const
{
	if (User::checkUser(user)) {
		return user->getRoleDbRight(User::subSetViewRight, CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this())));
	} else {
		return RIGHT_SPLASH;
	}
}

}
