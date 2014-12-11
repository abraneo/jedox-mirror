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

#ifndef OLAP_RIGHTS_DIMENSION_H
#define OLAP_RIGHTS_DIMENSION_H 1

#include "palo.h"

#include "Olap/SystemDimension.h"
#include "Olap/SystemDatabase.h"
#include "Olap/AttributedDimension.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief rights OLAP dimension
///
/// An OLAP dimension is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RightsDimension : public SystemDimension, public DRCubeDimension {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new dimension with given identifier
	////////////////////////////////////////////////////////////////////////////////

	RightsDimension(const string& name, Dimension::SaveType type) : SystemDimension(name, type)
	{
		if (type == Dimension::RIGHTS) {
			if (name == SystemDatabase::NAME_ROLE_DIMENSION || name == SystemDatabase::NAME_GROUP_DIMENSION || name == SystemDatabase::NAME_USER_DIMENSION) {
				protectedElems.push_back(SystemDatabase::NAME_ADMIN);
				protectedElems.push_back(SystemDatabase::NAME_IPS);
			} else if (name == SystemDatabase::NAME_USER_PROPERTIES_DIMENSION) {
				protectedElems.push_back(SystemDatabase::PASSWORD);
				protectedElems.push_back(SystemDatabase::INACTIVE);
				protectedElems.push_back(SystemDatabase::LICENSES);
			}
		} else if (type == Dimension::CONFIG) {
			protectedElems.push_back(SystemDatabase::NAME_CLIENT_CACHE_ELEMENT);
			protectedElems.push_back(SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT);
			protectedElems.push_back(SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT);
		} else if (type == Dimension::CELLPROPS) {
			protectedElems.push_back(SystemDatabase::NAME_RIGHTS_ELEMENT);
			protectedElems.push_back(SystemDatabase::NAME_FORMAT_STRING_ELEMENT);
			protectedElems.push_back(SystemDatabase::NAME_CELL_NOTE_ELEMENT);
		}
	}

	RightsDimension(const RightsDimension& dim) : SystemDimension(dim) {}

	//inherited from Dimension
	virtual void notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker);
	virtual void beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker);
	virtual void notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker);

	Element* addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal);

	void changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove);

	virtual PCommitable copy() const;

	virtual void checkElementAccessRight(const User *user, CPDatabase db, RightsType minimumRight) const;
	virtual RightsType getElementAccessRight(const User *user, CPDatabase db) const;
	virtual RightsType getDimensionDataRight(const User *user) const;
};

}

#endif
