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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_SYSTEM_DATABASE_H
#define OLAP_SYSTEM_DATABASE_H 1

#include "palo.h"

#include "Olap/Database.h"

namespace palo {

struct ElementItem {
	string name;
	Element::Type type;
};

template <typename T, size_t N>
 inline
 size_t array_size(const T (&lhs)[N])
 {
   return N;
 }

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP system database
///
/// An OLAP database consists of dimensions and cubes
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SystemDatabase : public Database {
public:
	static const uint32_t DB_TYPE = 2;

	static const string NAME_USER_DIMENSION;
	static const string NAME_USER_PROPERTIES_DIMENSION;
	static const string NAME_GROUP_DIMENSION;
	static const string NAME_GROUP_PROPERTIES_DIMENSION;
	static const string NAME_ROLE_DIMENSION;
	static const string NAME_ROLE_PROPERTIES_DIMENSION;
	static const string NAME_RIGHT_OBJECT_DIMENSION;
	static const string NAME_DATABASE_DIMENSION;
	static const string NAME_CUBE_DIMENSION;
	static const string NAME_LINE_DIMENSION;
	static const string NAME_LICENSE_DIMENSION;
	static const string NAME_SESSION_PROPERTIES_DIMENSION;
	static const string NAME_JOB_PROPERTIES_DIMENSION;
	static const string NAME_LICENSE_PROPERTIES_DIMENSION;
	static const string NAME_CUBE_PROPERTIES_DIMENSION;
	static const string NAME_SERVER_CONFIGURATION_DIMENSION;
	static const string NAME_MESSAGE_DIMENSION;
	static const string NAME_SESSION_DIMENSION;
	static const string NAME_JOB_DIMENSION;

	static const string NAME_USER_USER_PROPERTIERS_CUBE;
	static const string NAME_GROUP_GROUP_PROPERTIES_CUBE;
	static const string NAME_ROLE_ROLE_PROPERTIES_CUBE;
	static const string NAME_USER_GROUP_CUBE;
	static const string NAME_ROLE_RIGHT_OBJECT_CUBE;
	static const string NAME_GROUP_ROLE;
	static const string NAME_GROUP_DATABASE_CUBE;
	static const string NAME_SERVER_LOG_CUBE;
	static const string NAME_SESSIONS_CUBE;
	static const string NAME_JOBS_CUBE;
	static const string NAME_LICENSES_CUBE;

	static const string NAME_CLIENT_CACHE_ELEMENT;
	static const string NAME_HIDE_ELEMENTS_ELEMENT;
	static const string NAME_DEFAULT_RIGHT_ELEMENT;
	static const string NAME_CONFIGURATION_DIMENSION;

	static const string NAME_DIMENSION_DIMENSION;
	static const string NAME_SUBSET_DIMENSION;
	static const string NAME_VIEW_DIMENSION;

	static const string NAME_CELL_PROPERTIES_DIMENSION;
	static const string NAME_RIGHTS_ELEMENT;
	static const string NAME_FORMAT_STRING_ELEMENT;
	static const string NAME_CELL_NOTE_ELEMENT;

	static const string NAME_ADMIN;
	static const string PASSWORD_ADMIN;
	static const string NAME_IPS;
	static const string PASSWORD_IPS;

	static const string PASSWORD;
	static const string EXPIRED;
	static const string MUST_CHANGE;
	static const string EDITOR;
	static const string VIEWER;
	static const string NOTHING;
	static const string POWER_USER;
	static const string INACTIVE;
	static const string EMAIL;
	static const string LICENSES;
	static const string ROLE[28];
	static const ElementItem MESSAGE_ITEMS[4];
	static const ElementItem SESSION_PROPERTIES_ITEMS[];
	static const ElementItem JOB_PROPERTIES_ITEMS[];
	static const ElementItem LICENSE_PROPERTIES_ITEMS[];

	enum UpdateType {
		ADD, REMOVE, RENAME
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new database with given identifier
	////////////////////////////////////////////////////////////////////////////////

	SystemDatabase(const string& name) :
		Database(name),
		userDimensionId(NO_IDENTIFIER), groupDimensionId(NO_IDENTIFIER), userPropertiesDimensionId(NO_IDENTIFIER), rolePropertiesDimensionId(NO_IDENTIFIER),
		groupPropertiesDimensionId(NO_IDENTIFIER), roleDimensionId(NO_IDENTIFIER), rightObjectDimensionId(NO_IDENTIFIER), databaseDimensionId(NO_IDENTIFIER),
		userUserPropertiesCubeId(NO_IDENTIFIER), userGroupCubeId(NO_IDENTIFIER), roleRightObjectCubeId(NO_IDENTIFIER), groupRoleCubeId(NO_IDENTIFIER),
		roleRolePropertiesCubeId(NO_IDENTIFIER), groupGroupPropertiesCubeId(NO_IDENTIFIER), groupDatabaseCubeId(NO_IDENTIFIER),
		passwordElement(NO_IDENTIFIER), users(new UserList()), useExternalUser(false) {
		deletable = false;
		renamable = false;
		extensible = false;
	}

	SystemDatabase(const SystemDatabase& d);

	~SystemDatabase();

	virtual uint32_t getDatabaseType() {
		return DB_TYPE;
	}

	void saveDatabaseType(FileWriter*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns type of database
	////////////////////////////////////////////////////////////////////////////////

	virtual ItemType getType() const {
		return SYSTEMTYPE;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks and creates system dimensions and cubes
	////////////////////////////////////////////////////////////////////////////////

	virtual bool createSystemItems(PServer server, bool forceCreate);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets user by name and password
	////////////////////////////////////////////////////////////////////////////////

	PUser getUser(const string& name, const string& password, bool useMD5, bool *inactive);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets user by name
	////////////////////////////////////////////////////////////////////////////////

	PUser getUser(const string& name, bool *inactive);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets user by identifier
	////////////////////////////////////////////////////////////////////////////////

	PUser getUser(IdentifierType);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get external users by name (and groups)
	////////////////////////////////////////////////////////////////////////////////

	PUser getExternalUser(const string& name);

	bool getUseExternalUser() const { return useExternalUser; }

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the system group dimension
	////////////////////////////////////////////////////////////////////////////////

	PDimension getGroupDimension() const {
		Context::getContext()->saveParent(shared_from_this(), groupDimension);
		return groupDimension;
	}

	PDimension getGroupPropertiesDimension() const {
		Context::getContext()->saveParent(shared_from_this(), groupPropertiesDimension);
		return groupPropertiesDimension; ////////////////////////////////////////////////////////////////////////////////
	}
	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the system user dimension
	////////////////////////////////////////////////////////////////////////////////

	PDimension getUserDimension() const {
		Context::getContext()->saveParent(shared_from_this(), userDimension);
		return userDimension;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the system user properties dimension
	////////////////////////////////////////////////////////////////////////////////

	PDimension getUserPropertiesDimension() const {
		Context::getContext()->saveParent(shared_from_this(), userPropertiesDimension);
		return userPropertiesDimension;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the system role dimension
	////////////////////////////////////////////////////////////////////////////////

	PDimension getRoleDimension() const {
		Context::getContext()->saveParent(shared_from_this(), roleDimension);
		return roleDimension;
	}

	PDimension getRolePropertiesDimension() const {
		Context::getContext()->saveParent(shared_from_this(), rolePropertiesDimension);
		return rolePropertiesDimension;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the system rights object dimension
	////////////////////////////////////////////////////////////////////////////////

	PDimension getRightsObjectDimension() const {
		Context::getContext()->saveParent(shared_from_this(), rightObjectDimension);
		return rightObjectDimension;
	}

	PDimension getDatabaseDimension() const {
		Context::getContext()->saveParent(shared_from_this(), databaseDimension);
		return databaseDimension;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the user user-properties cube
	////////////////////////////////////////////////////////////////////////////////

	PCube getUserUserPropertiesCube() const {
		Context::getContext()->saveParent(shared_from_this(), userUserPropertiesCube);
		return userUserPropertiesCube;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the user to group cube
	////////////////////////////////////////////////////////////////////////////////

	PCube getUserGroupCube() const {
		Context::getContext()->saveParent(shared_from_this(), userGroupCube);
		return userGroupCube;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the group to role cube
	////////////////////////////////////////////////////////////////////////////////

	PCube getGroupRoleCube() const {
		Context::getContext()->saveParent(shared_from_this(), groupRoleCube);
		return groupRoleCube;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the role to right object cube
	////////////////////////////////////////////////////////////////////////////////

	PCube getRoleRightObjectCube() const {
		Context::getContext()->saveParent(shared_from_this(), roleRightObjectCube);
		return roleRightObjectCube;
	}

	PCube getRoleRolePropertiesCube() const {
		Context::getContext()->saveParent(shared_from_this(), roleRolePropertiesCube);
		return roleRolePropertiesCube;
	}

	PCube getGroupGroupPropertiesCube() const {
		Context::getContext()->saveParent(shared_from_this(), groupGroupPropertiesCube);
		return groupGroupPropertiesCube;
	}

	PCube getGroupDatabaseCube() const {
		Context::getContext()->saveParent(shared_from_this(), groupDatabaseCube);
		return groupDatabaseCube;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief refreshes all user objects
	////////////////////////////////////////////////////////////////////////////////

	void refreshUsers();

	PUserList getUsers(bool write) {
		return write && !users->isCheckedOut() ? COMMITABLE_CAST(UserList, users->copy()) : users;
	}
	void setUsers(PUserList l) {
		users = l;
	}
	PUser createExternalUser(PServer server, const string&, vector<string>*, IdentifierType *createdElement);
	PUser createUser(const string&);
	bool userExist(string name, bool external);
	int existingGroupsCount(vector<string> &groups);
	bool groupsUpdateRequired(PServer server, const string &username, vector<string> *groups);

	virtual PCommitable copy() const;
	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	bool mergespecial(const CPCommitable &o, const PCommitable &p, bool final);

	Element* getUserElement(const string &name, bool *inactive);
	Element* getGroupElement(const string &name);
	ElementsType getGroupElements();
	ElementsType getRoleElements() const;

	void changePassword(PServer server, PUser userChanging, IdentifierType userId, const string& new_password);
	void updateDatabaseDim(PServer server, bool useDimWorker);
	void updateDatabaseDim(PServer server, UpdateType type, const string &dbName, const string &dbOldName, bool useDimWorker);
	void setDbRight(PServer server, PUser user, const string &dbName);

private:
	PDimension checkAndCreateDimension(PServer server, const string &name, bool changeable);
	PDimension checkAndCreateDimension(PServer server, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, bool rebuild, bool changeable);
	PDimension checkAndCreateVirtualDimension(PServer server, const string& name);

	PCube checkAndCreateCube(PServer server, const string&, PDimension, PDimension);

	Element* checkAndReturnElement(PServer server, PDimension, const string&, Element::Type, bool forceCreate, bool *wasCreated = 0);
	IdentifierType checkAndCreateElement(PServer server, PDimension, const string&, Element::Type, bool forceCreate, bool *wasCreated = 0);
	void checkAndDeleteElement(PServer server, PDimension, const string&);

	void setCell(PServer server, PCube, IdentifierType, IdentifierType, const string&, bool overwrite, set<PCube> &changedCubes);

	PUser createUser(Element* userElement);

	Element* getElementIntern(const string &name, PDimension dim, PDimension propDim, PCube propCube, bool *inactive);
	ElementsType getElementsIntern(PDimension dim, PDimension propDim, PCube propCube) const;

private:
	PDimension userDimension;
	PDimension groupDimension;
	PDimension userPropertiesDimension;
	PDimension rolePropertiesDimension;
	PDimension groupPropertiesDimension;
	PDimension roleDimension;
	PDimension rightObjectDimension;
	PDimension databaseDimension;

	PCube userUserPropertiesCube;
	PCube userGroupCube;
	PCube roleRightObjectCube;
	PCube groupRoleCube;
	PCube roleRolePropertiesCube;
	PCube groupGroupPropertiesCube;
	PCube groupDatabaseCube;

	IdentifierType userDimensionId;
	IdentifierType groupDimensionId;
	IdentifierType userPropertiesDimensionId;
	IdentifierType rolePropertiesDimensionId;
	IdentifierType groupPropertiesDimensionId;
	IdentifierType roleDimensionId;
	IdentifierType rightObjectDimensionId;
	IdentifierType databaseDimensionId;

	IdentifierType userUserPropertiesCubeId;
	IdentifierType userGroupCubeId;
	IdentifierType roleRightObjectCubeId;
	IdentifierType groupRoleCubeId;
	IdentifierType roleRolePropertiesCubeId;
	IdentifierType groupGroupPropertiesCubeId;
	IdentifierType groupDatabaseCubeId;

	IdentifierType passwordElement;
	PUserList users;
	bool useExternalUser;
};

}

#endif
