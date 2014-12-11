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

#ifndef OLAP_USER_INFO_CUBE_H
#define OLAP_USER_INFO_CUBE_H 1

#include "palo.h"

#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief user info OLAP cube
///
/// An OLAP cube is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS UserInfoCube : public Cube {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates empty cube
	///
	/// @throws ParameterException on double or missing dimensions
	////////////////////////////////////////////////////////////////////////////////

	UserInfoCube(PDatabase db, const string& name, const IdentifiersType* dimensions);
	UserInfoCube(const UserInfoCube& c);

	virtual PCommitable copy() const;

	virtual void checkAreaAccessRight(CPDatabase db, PUser user, CPCubeArea area, User::RightSetting& rs, bool isZero, RightsType minimumRight, bool *defaultUsed) const;
	virtual RightsType getMinimumAccessRight(CPUser user) const;
	virtual void checkCubeAccessRight(PUser user, RightsType minimumRight, bool checkGroupCubeData, bool checkCubeRightObject) const;
	virtual RightsType getCubeAccessRight(CPUser user) const;
};

}

#endif
