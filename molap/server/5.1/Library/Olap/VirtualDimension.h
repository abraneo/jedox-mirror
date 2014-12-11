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

#ifndef OLAP_VIRTUAL_DIMENSION_H
#define OLAP_VIRTUAL_DIMENSION_H 1

#include "palo.h"

#include "Olap/SystemDimension.h"
#include "Olap/Server.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief virtual OLAP dimension
///
/// An OLAP dimension is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS VirtualDimension : public SystemDimension {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new dimension with given identifier
	////////////////////////////////////////////////////////////////////////////////
	VirtualDimension(const string& name) : SystemDimension(name, Dimension::VIRTUAL)
	{
		status = LOADED;
		setDeletable(false);
		setRenamable(false);
		setChangable(false);
	}
	virtual ~VirtualDimension() {}

	void loadDimension(PServer server, PDatabase db, FileReader* file) {
		return;
	}

	void saveDimension(PDatabase db, FileWriter* file) {
		return;
	}

	PCommitable copy() const
	{
		return const_cast<VirtualDimension *>(this)->shared_from_this();
	}

	virtual void checkElementAccessRight(const User *user, CPDatabase db, RightsType minimumRight) const {
		if (User::checkUser(user)) {
			RightsType rt = user->getRoleDbRight(User::sysOpRight, db);
			if (rt < minimumRight) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
			}
		}

	}

	virtual RightsType getElementAccessRight(const User *user, CPDatabase db) const {
		return User::checkUser(user) ? user->getRoleDbRight(User::sysOpRight, db) : RIGHT_DELETE;
	}

	virtual RightsType getDimensionDataRight(const User *user) const {
		throw ErrorException(ErrorException::ERROR_INVALID_PERMISSION, "virtual dimension cannot be used in 'normal' cubes");
	}
};

}

#endif
