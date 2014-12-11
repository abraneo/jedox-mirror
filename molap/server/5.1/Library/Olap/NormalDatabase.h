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

#ifndef OLAP_NORMAL_DATABASE_H
#define OLAP_NORMAL_DATABASE_H 1

#include "palo.h"

#include "Olap/Database.h"

namespace palo {
class User;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP normal database
///
/// An OLAP database consists of dimensions and cubes
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS NormalDatabase : public Database {
public:
	static const uint32_t DB_TYPE = 1;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new database with given identifier
	////////////////////////////////////////////////////////////////////////////////

	NormalDatabase(const string& name);
	NormalDatabase(const NormalDatabase& d);

	virtual uint32_t getDatabaseType() {
		return DB_TYPE;
	}

	virtual void notifyAddDatabase(PServer server, PUser user, bool useDimWorker);
	virtual void notifyRemoveDatabase(PServer server, bool useDimWorker);
	virtual void notifyRenameDatabase(PServer server, const string &oldName, bool useDimWorker);

	void saveDatabaseType(FileWriter*);
	virtual void loadDatabase(PServer server, bool addSystemDimension);

	PDimension getCubeDimension();
	PDimension getDimensionDimension();
	CPDimension getCellPropertiesDimension() const {
		return cellPropsDimension;
	}

	virtual ItemType getType() const {
		return NORMALTYPE;
	}

	virtual bool createSystemItems(PServer server, bool forceCreate);

	virtual PCommitable copy() const;
	virtual bool merge(const CPCommitable &o, const PCommitable &p);

protected:
	void fillRightProperty(PServer server);

private:
	PDimension cubeDimension; // alias dimension for #_CUBE_
	PDimension dimensionDimension; // dimension dimension for cube #_SUBSET_* and #_VIEW_*
	PDimension cellPropsDimension;

	IdentifierType cubeDimensionId;
	IdentifierType dimensionDimensionId;
	IdentifierType cellPropsDimensionId;
};

}

#endif
