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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_USER_INFO_DATABASE_H
#define OLAP_USER_INFO_DATABASE_H 1

#include "palo.h"

#include "Olap/NormalDatabase.h"

namespace palo {
struct ElementItem;

class SERVER_CLASS UserInfoDatabase : public NormalDatabase {
public:
	static const uint32_t DB_TYPE = 3;
	static const string NAME_CONFIG_DIMENSION;

	UserInfoDatabase(const string &name) : NormalDatabase(name) {}
	UserInfoDatabase(const UserInfoDatabase &d) : NormalDatabase(d) {}

	virtual uint32_t getDatabaseType() {
		return DB_TYPE;
	}
	virtual ItemType getType() const {
		return USER_INFOTYPE;
	}
	virtual PCommitable copy() const;
};

class SERVER_CLASS ConfigDatabase : public UserInfoDatabase {
public:
	static const string NAME_CONFIG_DIMENSION;

	ConfigDatabase(const string &name);
	ConfigDatabase(const ConfigDatabase &d);

	virtual void notifyAddDatabase(PServer server, PUser user, bool useDimWorker);
	virtual void notifyRemoveDatabase(PServer server, bool useDimWorker) {}
	virtual void notifyRenameDatabase(PServer server, const string &oldName, bool useDimWorker) {}

	virtual PCommitable copy() const;
	virtual bool createSystemItems(PServer server, bool forceCreate);

	bool isProtected(const IdentifiersType &key) const;
	const Area &getProtectedArea() const {return *protArea.get();}

	static bool isConfigCube(const string &dbName, const string &cubeName);

private:
	static const string NAME_TASKS_CUBE;
	static const string NAME_TASKS_NOTIF_CUBE;
	static const string NAME_TASKS_PRIVATE_CUBE;
	static const string NAME_ETLS_CUBE;

	static const string NAME_CONNECTIONS_DIMENSION;
//	static const string NAME_CONFIG_DIMENSION;
	static const string NAME_TASKPROPS_DIMENSION;
	static const string NAME_NOTIFTYPES_DIMENSION;
	static const string NAME_TASKS_DIMENSION;
	static const string NAME_VARSETS_DIMENSION;
	static const string NAME_ETLS_DIMENSION;
	static const string NAME_ETLPROPS_DIMENSION;

	static const ElementItem CONNECTIONS_ATTR_ITEMS[];
	static const ElementItem CONFIG_ITEMS[];
	static const ElementItem CONFIG_ATTR_ITEMS[];
	static const ElementItem TASKPROPS_ITEMS[];
	static const ElementItem NOTIFTYPES_ITEMS[];
	static const ElementItem VARSETS_ATTR_ITEMS[];
	static const ElementItem ETLPROPS_ITEMS[];

	PDimension checkAndCreateElement(PServer server, PDatabase db, PDimension dim, const ElementItem *elem, bool &dbChanged);
	PDimension checkAndCreateDimension(PServer server, PDatabase db, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, bool &dbChanged);
	PDimension checkAndCreateDimension(PServer server, PDatabase db, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, const ElementItem *attrBegin, const ElementItem *attrEnd, bool &dbChanged);
	void checkAndCreateCube(PServer server, PDatabase db, const string &name, IdentifiersType &dims, bool &dbChanged);
	void checkConfigCube(PServer server, PDatabase db, bool &dbChanged);

	bool protect;
	PCubeArea protArea;
	map<pair<IdentifierType, IdentifierType>, string> protValues;
};

}

#endif
