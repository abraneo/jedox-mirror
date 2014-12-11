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

#include "Olap/SystemDatabase.h"

#include "Exceptions/FileFormatException.h"

#include "Olap/RightsCube.h"
#include "Olap/ServerLogCube.h"
#include "Olap/SessionInfoCube.h"
#include "Olap/JobInfoCube.h"
#include "Olap/LicenseInfoCube.h"
#include "Olap/RightsDimension.h"
#include "Olap/VirtualDimension.h"
#include "Engine/EngineBase.h"
#include "Olap/Password.h"
#include "Olap/Server.h"

namespace palo {
const string SystemDatabase::NAME_USER_DIMENSION = "#_USER_";
const string SystemDatabase::NAME_USER_PROPERTIES_DIMENSION = "#_USER_PROPERTIES_";
const string SystemDatabase::NAME_GROUP_DIMENSION = "#_GROUP_";
const string SystemDatabase::NAME_GROUP_PROPERTIES_DIMENSION = "#_GROUP_PROPERTIES_";
const string SystemDatabase::NAME_ROLE_DIMENSION = "#_ROLE_";
const string SystemDatabase::NAME_ROLE_PROPERTIES_DIMENSION = "#_ROLE_PROPERTIES_";
const string SystemDatabase::NAME_RIGHT_OBJECT_DIMENSION = "#_RIGHT_OBJECT_";
const string SystemDatabase::NAME_DATABASE_DIMENSION = "#_DATABASE_";
const string SystemDatabase::NAME_CUBE_DIMENSION = "#_CUBE_";
const string SystemDatabase::NAME_LINE_DIMENSION = "#_LINE_";
const string SystemDatabase::NAME_LICENSE_DIMENSION = "#_LICENSE_";
const string SystemDatabase::NAME_SESSION_PROPERTIES_DIMENSION = "#_SESSION_PROPERTIES_";
const string SystemDatabase::NAME_JOB_PROPERTIES_DIMENSION = "#_JOB_PROPERTIES_";
const string SystemDatabase::NAME_LICENSE_PROPERTIES_DIMENSION = "#_LICENSE_PROPERTIES_";
const string SystemDatabase::NAME_CUBE_PROPERTIES_DIMENSION = "#_CUBE_PROPERTIES_";
const string SystemDatabase::NAME_MESSAGE_DIMENSION = "#_MESSAGE_";
const string SystemDatabase::NAME_SESSION_DIMENSION = "#_SESSION_";
const string SystemDatabase::NAME_JOB_DIMENSION = "#_JOB_";

const string SystemDatabase::NAME_USER_USER_PROPERTIERS_CUBE = "#_USER_USER_PROPERTIES";
const string SystemDatabase::NAME_GROUP_GROUP_PROPERTIES_CUBE = "#_GROUP_GROUP_PROPERTIES";
const string SystemDatabase::NAME_ROLE_ROLE_PROPERTIES_CUBE = "#_ROLE_ROLE_PROPERTIES";
const string SystemDatabase::NAME_USER_GROUP_CUBE = "#_USER_GROUP";
const string SystemDatabase::NAME_ROLE_RIGHT_OBJECT_CUBE = "#_ROLE_RIGHT_OBJECT";
const string SystemDatabase::NAME_GROUP_ROLE = "#_GROUP_ROLE";
const string SystemDatabase::NAME_GROUP_DATABASE_CUBE = "#_GROUP_DATABASE_DATA";
const string SystemDatabase::NAME_SERVER_LOG_CUBE = "#_SERVER_LOG_";
const string SystemDatabase::NAME_SESSIONS_CUBE = "#_SESSIONS_";
const string SystemDatabase::NAME_JOBS_CUBE = "#_JOBS_";
const string SystemDatabase::NAME_LICENSES_CUBE = "#_LICENSES_";

const string SystemDatabase::NAME_CLIENT_CACHE_ELEMENT = "ClientCache";
const string SystemDatabase::NAME_HIDE_ELEMENTS_ELEMENT = "HideElements";
const string SystemDatabase::NAME_DEFAULT_RIGHT_ELEMENT = "DefaultRight";
const string SystemDatabase::NAME_CONFIGURATION_DIMENSION = "#_CONFIGURATION_";

const string SystemDatabase::NAME_DIMENSION_DIMENSION = "#_DIMENSION_";
const string SystemDatabase::NAME_SUBSET_DIMENSION = "#_SUBSET_";
const string SystemDatabase::NAME_VIEW_DIMENSION = "#_VIEW_";

const string SystemDatabase::NAME_CELL_PROPERTIES_DIMENSION = "#_CELL_PROPERTIES_";
const string SystemDatabase::NAME_RIGHTS_ELEMENT = "#_Rights";
const string SystemDatabase::NAME_FORMAT_STRING_ELEMENT = "FormatString";
const string SystemDatabase::NAME_CELL_NOTE_ELEMENT = "CellNote";

const string SystemDatabase::NAME_ADMIN = "admin";
const string SystemDatabase::PASSWORD_ADMIN = "admin";
const string SystemDatabase::NAME_IPS = "_internal_suite";
const string SystemDatabase::PASSWORD_IPS = "_internal_suite";

const string SystemDatabase::PASSWORD = "password";
const string SystemDatabase::EXPIRED = "expired";
const string SystemDatabase::MUST_CHANGE = "must change";
const string SystemDatabase::EDITOR = "editor";
const string SystemDatabase::VIEWER = "viewer";
const string SystemDatabase::POWER_USER = "poweruser";

const string SystemDatabase::INACTIVE = "inactive";
const string SystemDatabase::EMAIL = "email";
const string SystemDatabase::LICENSES = "licenses";

const string SystemDatabase::ROLE[] = {
// never change the order of the elements!
	"user", "password", "group", "database", "cube", "dimension", "dimension element", "cell data", "rights", "system operations", "event processor", "sub-set view", "user info", "rule", "ste_reports", "ste_files", "ste_palo", "ste_users", "ste_etl", "ste_conns", "drillthrough", "ste_scheduler", "ste_logs", "ste_licenses", "ste_mobile", "ste_analyzer", "ste_sessions", "ste_settings"
};

const ElementItem SystemDatabase::MESSAGE_ITEMS[] = {
		{"Date",	Element::STRING},
		{"Time",	Element::STRING},
		{"Level",	Element::STRING},
		{"Message", Element::STRING}
};

const ElementItem SystemDatabase::SESSION_PROPERTIES_ITEMS[] = {
	// never change the order of the elements!
	{"User",	Element::STRING},
	{"Jobs",	Element::NUMERIC},
	{"Login Time",	Element::STRING},
	{"Expiration Time", Element::STRING},
	{"Time",	Element::NUMERIC},
	{"Active Jobs", Element::STRING},
	{"License",	Element::STRING},
	{"Address",	Element::STRING},
	{"Command",	Element::STRING},
	{"Description",	Element::STRING},
	{"MachineId", Element::STRING},
	{"CurrentSession", Element::NUMERIC}
};

const ElementItem SystemDatabase::JOB_PROPERTIES_ITEMS[] = {
	// never change the order of the elements!
	{"Name",       Element::STRING},
	{"Duration",   Element::NUMERIC},
	{"Parameters", Element::STRING},
	{"Progress", Element::NUMERIC},
	{"Command",	Element::STRING}
};

const ElementItem SystemDatabase::LICENSE_PROPERTIES_ITEMS[] = {
	// never change the order of the elements!
	{"Code",    		Element::STRING},		// license key
	{"Customer",   		Element::STRING},		// customer name
	{"Features",    	Element::STRING},		// hex string representing the bit-field of supported features
	{"Concurrent Seats",Element::NUMERIC},		// total number of concurrent seats licensed
	{"Named Seats",		Element::NUMERIC},
	{"Sessions",		Element::STRING},	// comma delimited list of sessions holding concurrent License slot
	{"GPU Cards",		Element::NUMERIC},
	{"Free Concurrent",	Element::NUMERIC},
	{"Free Named",		Element::NUMERIC},
	{"Activation Time", Element::STRING},
	{"Expiration Time", Element::STRING}
};

////////////////////////////////////////////////////////////////////////////////
// constructors and destructors
////////////////////////////////////////////////////////////////////////////////

SystemDatabase::SystemDatabase(const SystemDatabase& d) :
	Database(d),
	userDimension(d.userDimension), groupDimension(d.groupDimension), userPropertiesDimension(d.userPropertiesDimension), rolePropertiesDimension(d.rolePropertiesDimension),
	groupPropertiesDimension(d.groupPropertiesDimension), roleDimension(d.roleDimension), rightObjectDimension(d.rightObjectDimension), databaseDimension(d.databaseDimension),
	userUserPropertiesCube(d.userUserPropertiesCube), userGroupCube(d.userGroupCube), roleRightObjectCube(d.roleRightObjectCube), groupRoleCube(d.groupRoleCube),
	roleRolePropertiesCube(d.roleRolePropertiesCube), groupGroupPropertiesCube(d.groupGroupPropertiesCube), groupDatabaseCube(d.groupDatabaseCube),
	userDimensionId(d.userDimensionId), groupDimensionId(d.groupDimensionId), userPropertiesDimensionId(d.userPropertiesDimensionId), rolePropertiesDimensionId(d.rolePropertiesDimensionId),
	groupPropertiesDimensionId(d.groupPropertiesDimensionId), roleDimensionId(d.roleDimensionId), rightObjectDimensionId(d.rightObjectDimensionId), databaseDimensionId(d.databaseDimensionId),
	userUserPropertiesCubeId(d.userUserPropertiesCubeId), userGroupCubeId(d.userGroupCubeId), roleRightObjectCubeId(d.roleRightObjectCubeId), groupRoleCubeId(d.groupRoleCubeId),
	roleRolePropertiesCubeId(d.roleRolePropertiesCubeId), groupGroupPropertiesCubeId(d.groupGroupPropertiesCubeId), groupDatabaseCubeId(d.groupDatabaseCubeId),
	passwordElement(d.passwordElement), users(d.users), useExternalUser(d.useExternalUser)
{
}

SystemDatabase::~SystemDatabase()
{
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a dimension
////////////////////////////////////////////////////////////////////////////////

void SystemDatabase::saveDatabaseType(FileWriter* file)
{
	file->appendInteger(getId());
	file->appendEscapeString(getName());
	file->appendInteger(getDatabaseType());
	file->appendInteger(deletable ? 1 : 0);
	file->appendInteger(renamable ? 1 : 0);
	file->appendInteger(extensible ? 1 : 0);
	file->nextLine();
}

////////////////////////////////////////////////////////////////////////////////
// other stuff
////////////////////////////////////////////////////////////////////////////////

bool SystemDatabase::createSystemItems(PServer server, bool forceCreate)
{
	checkCheckedOut();
	bool changed = true; //SystemDatabase is always changed, e.g. sessionProperties, jobProperties and licenseProperties dimensions are recreated
	extensible = true;

	// user dimension
	userDimension = checkAndCreateDimension(server, NAME_USER_DIMENSION, true);

	// user elements
	IdentifierType adminUserElement = checkAndCreateElement(server, userDimension, NAME_ADMIN, Element::STRING, true);
	bool ipsUserWasCreated = false;
	IdentifierType ipsUserElement = checkAndCreateElement(server, userDimension, NAME_IPS, Element::STRING, true, &ipsUserWasCreated);
	IdentifierType powerUser = checkAndCreateElement(server, userDimension, POWER_USER, Element::STRING, forceCreate);
	IdentifierType editorUser = checkAndCreateElement(server, userDimension, EDITOR, Element::STRING, forceCreate);
	IdentifierType viewerUser = checkAndCreateElement(server, userDimension, VIEWER, Element::STRING, forceCreate);

	if (forceCreate && (adminUserElement == NO_IDENTIFIER || powerUser == NO_IDENTIFIER || editorUser == NO_IDENTIFIER || viewerUser == NO_IDENTIFIER)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "system database corrupted");
	}

	// user properties dimension
	userPropertiesDimension = checkAndCreateDimension(server, NAME_USER_PROPERTIES_DIMENSION, true);

	// user properties elements
	passwordElement = checkAndCreateElement(server, userPropertiesDimension, PASSWORD, Element::STRING, true);
	checkAndDeleteElement(server, userPropertiesDimension, EXPIRED);
	checkAndDeleteElement(server, userPropertiesDimension, MUST_CHANGE);
	IdentifierType inactiveElement = checkAndCreateElement(server, userPropertiesDimension, INACTIVE, Element::STRING, true);
	checkAndCreateElement(server, userPropertiesDimension, EMAIL, Element::STRING, true);
	checkAndCreateElement(server, userPropertiesDimension, LICENSES, Element::STRING, true);

	// group dimension
	groupDimension = checkAndCreateDimension(server, NAME_GROUP_DIMENSION, true);

	// group elements
	IdentifierType adminGroupElement = checkAndCreateElement(server, groupDimension, NAME_ADMIN, Element::STRING, true);
	IdentifierType poweruserGroup = checkAndCreateElement(server, groupDimension, POWER_USER, Element::STRING, forceCreate);
	IdentifierType editorGroup = checkAndCreateElement(server, groupDimension, EDITOR, Element::STRING, forceCreate);
	IdentifierType viewerGroup = checkAndCreateElement(server, groupDimension, VIEWER, Element::STRING, forceCreate);

	if (forceCreate && (adminGroupElement == NO_IDENTIFIER || poweruserGroup == NO_IDENTIFIER || editorGroup == NO_IDENTIFIER || viewerGroup == NO_IDENTIFIER)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "system database corrupted");
	}

	// role dimension
	roleDimension = checkAndCreateDimension(server, NAME_ROLE_DIMENSION, true);

	// role elements
	IdentifierType adminRoleElement = checkAndCreateElement(server, roleDimension, NAME_ADMIN, Element::STRING, true);
	IdentifierType poweruserRole = checkAndCreateElement(server, roleDimension, POWER_USER, Element::STRING, forceCreate);
	IdentifierType editorRole = checkAndCreateElement(server, roleDimension, EDITOR, Element::STRING, forceCreate);
	IdentifierType viewerRole = checkAndCreateElement(server, roleDimension, VIEWER, Element::STRING, forceCreate);

	if (forceCreate && (adminRoleElement == NO_IDENTIFIER || poweruserRole == NO_IDENTIFIER || editorRole == NO_IDENTIFIER || viewerRole == NO_IDENTIFIER)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "system database corrupted");
	}

	// right objects dimension
	rightObjectDimension = checkAndCreateDimension(server, NAME_RIGHT_OBJECT_DIMENSION, false);

	PRightsDimension rd = COMMITABLE_CAST(RightsDimension, rightObjectDimension);
	if (rd) {
		rd->setRightObject(true);
	}

	// right objects elements
	IdentifierType rightObjects[sizeof(ROLE) / sizeof(ROLE[0])];

	for (size_t i = 0; i < sizeof(ROLE) / sizeof(ROLE[0]); i++) {
		rightObjects[i] = checkAndCreateElement(server, rightObjectDimension, ROLE[i], Element::STRING, true);
	}

	databaseDimension = checkAndCreateDimension(server, NAME_DATABASE_DIMENSION , true);

	PDimension lineDimension = checkAndCreateVirtualDimension(server, NAME_LINE_DIMENSION);
	PDimension licenseDimension = checkAndCreateVirtualDimension(server, NAME_LICENSE_DIMENSION);
	PDimension sessionDimension = checkAndCreateVirtualDimension(server, NAME_SESSION_DIMENSION);
	PDimension jobDimension = checkAndCreateVirtualDimension(server, NAME_JOB_DIMENSION);

	PDimension sessionPropertiesDimension = checkAndCreateDimension(server, NAME_SESSION_PROPERTIES_DIMENSION, SESSION_PROPERTIES_ITEMS, SESSION_PROPERTIES_ITEMS+array_size(SESSION_PROPERTIES_ITEMS), true, false);
	PDimension jobPropertiesDimension = checkAndCreateDimension(server, NAME_JOB_PROPERTIES_DIMENSION, JOB_PROPERTIES_ITEMS, JOB_PROPERTIES_ITEMS+array_size(JOB_PROPERTIES_ITEMS), true, false);
	PDimension licensePropertiesDimension = checkAndCreateDimension(server, NAME_LICENSE_PROPERTIES_DIMENSION, LICENSE_PROPERTIES_ITEMS, LICENSE_PROPERTIES_ITEMS+array_size(LICENSE_PROPERTIES_ITEMS), true, false);

	/*PDimension cubePropertiesDimension = */checkAndCreateDimension(server, NAME_CUBE_PROPERTIES_DIMENSION, true);
	PDimension messageDimension = checkAndCreateDimension(server, NAME_MESSAGE_DIMENSION, MESSAGE_ITEMS, MESSAGE_ITEMS+array_size(MESSAGE_ITEMS), false, false);

	set<PCube> changedCubes;

	// setting up some cubes
	userUserPropertiesCube = checkAndCreateCube(server, NAME_USER_USER_PROPERTIERS_CUBE, userDimension, userPropertiesDimension);

	setCell(server, userUserPropertiesCube, adminUserElement, passwordElement, Password(PASSWORD_ADMIN).increaseLevel(Password::SAVED_PASSWORD_LEVEL).getHashed(), false, changedCubes);
	setCell(server, userUserPropertiesCube, ipsUserElement, passwordElement, Password(PASSWORD_IPS).increaseLevel(Password::SAVED_PASSWORD_LEVEL).getHashed(), false, changedCubes);
	if (ipsUserWasCreated) {
		setCell(server, userUserPropertiesCube, ipsUserElement, inactiveElement, "1", false, changedCubes);
	}
	if (powerUser != NO_IDENTIFIER) {
		setCell(server, userUserPropertiesCube, powerUser, passwordElement, Password(POWER_USER).increaseLevel(Password::SAVED_PASSWORD_LEVEL).getHashed(), false, changedCubes);
	}
	if (editorUser != NO_IDENTIFIER) {
		setCell(server, userUserPropertiesCube, editorUser, passwordElement, Password(EDITOR).increaseLevel(Password::SAVED_PASSWORD_LEVEL).getHashed(), false, changedCubes);
	}
	if (viewerUser != NO_IDENTIFIER) {
		setCell(server, userUserPropertiesCube, viewerUser, passwordElement, Password(VIEWER).increaseLevel(Password::SAVED_PASSWORD_LEVEL).getHashed(), false, changedCubes);
	}
	userUserPropertiesCube->commitChanges(false, PUser(), changedCubes, false);

	userGroupCube = checkAndCreateCube(server, NAME_USER_GROUP_CUBE, userDimension, groupDimension);

	setCell(server, userGroupCube, adminUserElement, adminGroupElement, "1", true, changedCubes);
	setCell(server, userGroupCube, ipsUserElement, adminGroupElement, "1", true, changedCubes);
	if (powerUser != NO_IDENTIFIER && poweruserGroup != NO_IDENTIFIER) {
		setCell(server, userGroupCube, powerUser, poweruserGroup, "1", false, changedCubes);
	}
	if (editorUser != NO_IDENTIFIER && editorGroup != NO_IDENTIFIER) {
		setCell(server, userGroupCube, editorUser, editorGroup, "1", false, changedCubes);
	}
	if (viewerUser != NO_IDENTIFIER && viewerGroup != NO_IDENTIFIER) {
		setCell(server, userGroupCube, viewerUser, viewerGroup, "1", false, changedCubes);
	}
	userGroupCube->commitChanges(false, PUser(), changedCubes, false);

	roleRightObjectCube = checkAndCreateCube(server, NAME_ROLE_RIGHT_OBJECT_CUBE, roleDimension, rightObjectDimension);

	string adminDefault[sizeof(ROLE) / sizeof(ROLE[0])] = {"D", "D", "D", "D", "D", "D", "D", "S", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D", "D"};
	string poweruserDefault[sizeof(ROLE) / sizeof(ROLE[0])] = {"R", "N", "R", "R", "D", "D", "D", "S", "R", "W", "N", "D", "D", "D", "D", "D", "D", "N", "D", "N", "D", "N", "N", "N", "N", "N", "N", "N"};
	string editorDefault[sizeof(ROLE) / sizeof(ROLE[0])] = {"N", "N", "N", "R", "R", "R", "R", "W", "N", "N", "N", "W", "W", "W", "R", "W", "D", "N", "N", "N", "N", "N", "N", "N", "N", "N", "N", "N"};
	string viewerDefault[sizeof(ROLE) / sizeof(ROLE[0])] = {"N", "N", "N", "R", "R", "R", "R", "R", "N", "N", "N", "R", "R", "R", "R", "R", "N", "N", "N", "N", "D", "N", "N", "N", "N", "N", "N", "N"};

	for (size_t i = 0; i < sizeof(ROLE) / sizeof(ROLE[0]); i++) {
		setCell(server, roleRightObjectCube, adminRoleElement, rightObjects[i], adminDefault[i], true, changedCubes);
		if (poweruserRole != NO_IDENTIFIER) {
			setCell(server, roleRightObjectCube, poweruserRole, rightObjects[i], poweruserDefault[i], false, changedCubes);
		}
		if (editorRole != NO_IDENTIFIER) {
			setCell(server, roleRightObjectCube, editorRole, rightObjects[i], editorDefault[i], false, changedCubes);
		}
		if (viewerRole != NO_IDENTIFIER) {
			setCell(server, roleRightObjectCube, viewerRole, rightObjects[i], viewerDefault[i], false, changedCubes);
		}
	}
	roleRightObjectCube->commitChanges(false, PUser(), changedCubes, false);

	groupRoleCube = checkAndCreateCube(server, NAME_GROUP_ROLE, groupDimension, roleDimension);

	setCell(server, groupRoleCube, adminGroupElement, adminRoleElement, "1", true, changedCubes);
	if (poweruserGroup != NO_IDENTIFIER && poweruserRole != NO_IDENTIFIER) {
		setCell(server, groupRoleCube, poweruserGroup, poweruserRole, "1", false, changedCubes);
	}
	if (editorGroup != NO_IDENTIFIER && editorRole != NO_IDENTIFIER) {
		setCell(server, groupRoleCube, editorGroup, editorRole, "1", false, changedCubes);
	}
	if (viewerGroup != NO_IDENTIFIER && viewerRole != NO_IDENTIFIER) {
		setCell(server, groupRoleCube, viewerGroup, viewerRole, "1", false, changedCubes);
	}
	groupRoleCube->commitChanges(false, PUser(), changedCubes, false);

	//more dimensions
	groupPropertiesDimension = checkAndCreateDimension(server, NAME_GROUP_PROPERTIES_DIMENSION, true);
	checkAndCreateElement(server, groupPropertiesDimension, INACTIVE, Element::STRING, true);
	rolePropertiesDimension = checkAndCreateDimension(server, NAME_ROLE_PROPERTIES_DIMENSION, true);
	checkAndCreateElement(server, rolePropertiesDimension, INACTIVE, Element::STRING, true);

	//more cubes
	roleRolePropertiesCube = checkAndCreateCube(server, NAME_ROLE_ROLE_PROPERTIES_CUBE, roleDimension, rolePropertiesDimension);
	groupGroupPropertiesCube = checkAndCreateCube(server, NAME_GROUP_GROUP_PROPERTIES_CUBE, groupDimension, groupPropertiesDimension);
	groupDatabaseCube = checkAndCreateCube(server, NAME_GROUP_DATABASE_CUBE, groupDimension, databaseDimension);

	// server log cube #_SERVER_LOG_
	PCube serverLogCube = lookupCubeByName(SystemDatabase::NAME_SERVER_LOG_CUBE, false);

	if (serverLogCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(lineDimension->getId());
		dimensions.push_back(messageDimension->getId());

		serverLogCube = PCube(new ServerLogCube(COMMITABLE_CAST(Database, shared_from_this()), SystemDatabase::NAME_SERVER_LOG_CUBE, &dimensions));
		addCube(server, serverLogCube, true, true, NULL, NULL, NULL, false);
	} else if (serverLogCube->getCubeType() != Cube::LOG) {
		throw FileFormatException("server log cube '" + SystemDatabase::NAME_SERVER_LOG_CUBE + "' corrupted", 0);
	}

	// server log cube #_SESSIONS_
	PCube sessionsCube = lookupCubeByName(SystemDatabase::NAME_SESSIONS_CUBE, false);

	if (sessionsCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(sessionDimension->getId());
		dimensions.push_back(sessionPropertiesDimension->getId());

		sessionsCube = PCube(new SessionInfoCube(COMMITABLE_CAST(Database, shared_from_this()), SystemDatabase::NAME_SESSIONS_CUBE, &dimensions));
		addCube(server, sessionsCube, true, true, NULL, NULL, NULL, false);
	} else if (sessionsCube->getCubeType() != Cube::SESSIONS) {
		throw FileFormatException("sessions cube '" + SystemDatabase::NAME_SESSIONS_CUBE + "' corrupted", 0);
	}
	sessionsCube->setDeletable(false);
	sessionsCube->setRenamable(false);

	// server jobs cube #_JOBS_
	PCube jobsCube = lookupCubeByName(SystemDatabase::NAME_JOBS_CUBE, false);

	if (jobsCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(sessionDimension->getId());
		dimensions.push_back(jobDimension->getId());
		dimensions.push_back(jobPropertiesDimension->getId());

		jobsCube = PCube(new JobInfoCube(COMMITABLE_CAST(Database, shared_from_this()), SystemDatabase::NAME_JOBS_CUBE, &dimensions));
		addCube(server, jobsCube, true, true, NULL, NULL, NULL, false);
	} else if (jobsCube->getCubeType() != Cube::JOBS) {
		throw FileFormatException("jobs cube '" + SystemDatabase::NAME_JOBS_CUBE + "' corrupted", 0);
	}
	jobsCube->setDeletable(false);
	jobsCube->setRenamable(false);

	// server licenses cube #_LICENSES_
	PCube licensesCube = lookupCubeByName(SystemDatabase::NAME_LICENSES_CUBE, false);

	if (licensesCube == 0) {
		IdentifiersType dimensions;
		dimensions.push_back(licenseDimension->getId());
		dimensions.push_back(licensePropertiesDimension->getId());

		licensesCube = PCube(new LicenseInfoCube(COMMITABLE_CAST(Database, shared_from_this()), SystemDatabase::NAME_LICENSES_CUBE, &dimensions));
		addCube(server, licensesCube, true, true, NULL, NULL, NULL, false);
	} else if (licensesCube->getCubeType() != Cube::LICENSES) {
		throw FileFormatException("licenses cube '" + SystemDatabase::NAME_LICENSES_CUBE + "' corrupted", 0);
	}
	licensesCube->setDeletable(false);
	licensesCube->setRenamable(false);

	extensible = false;

	server->updateLicenses(COMMITABLE_CAST(SystemDatabase, shared_from_this()));
	saveDatabase(server);

	return changed;
}

PDimension SystemDatabase::checkAndCreateDimension(PServer server, const string &name, bool changeable)
{
	return checkAndCreateDimension(server, name, 0, 0, false, changeable);
}

PDimension SystemDatabase::checkAndCreateDimension(PServer server, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, bool rebuild, bool changeable)
{
	PDimension dimension = lookupDimensionByName(name, false);

	if (dimension == 0) {
		dimension = PDimension(new RightsDimension(name, Dimension::RIGHTS));
		dimension->setDeletable(false);
		dimension->setRenamable(false);
		addDimension(server, dimension, true, true, NULL, NULL, NULL, NULL, false);
	}
	dimension->setChangable(changeable); // correction of 'changable' member
	if (rebuild) {
		dimension->clearElements(server, COMMITABLE_CAST(Database, shared_from_this()), PUser(), false, false);
	}
	for (const ElementItem* elem = elemBegin; elemBegin && elem != elemEnd; elem++) {
		checkAndCreateElement(server, dimension, elem->name, elem->type, true);
	}

	return dimension;
}

PDimension SystemDatabase::checkAndCreateVirtualDimension(PServer server, const string& name)
{
	PDimension dimension = lookupDimensionByName(name, false);

	if (dimension && dimension->getDimensionType() != Dimension::VIRTUAL) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "system database corrupted. "+name+" found on disk!");
	}
	if (!dimension) {
		dimension = PDimension(new VirtualDimension(name));
		addDimension(server, dimension, true, true, NULL, NULL, NULL, NULL, false);
	}
	return dimension;
}

PCube SystemDatabase::checkAndCreateCube(PServer server, const string& name, PDimension d1, PDimension d2)
{
	PCube cube = lookupCubeByName(name, false);
	PDatabase thisDatabase = COMMITABLE_CAST(Database, shared_from_this());

	if (cube) {
		if (cube->getStatus() == Cube::UNLOADED) {
			cube->loadCube(server, thisDatabase, false);
		}
		return cube;
	}

	IdentifiersType dims;
	dims.push_back(d1->getId());
	dims.push_back(d2->getId());

	// create new cube and add cube to cube vector
	cube = PCube(new RightsCube(thisDatabase, name, &dims));
	cube->setDeletable(false);
	cube->setRenamable(false);

	// and add cube to structure
	addCube(server, cube, true, true, NULL, NULL, NULL, false);

	// return new rights cube
	return cube;
}

Element* SystemDatabase::checkAndReturnElement(PServer server, PDimension dimension, const string& name, Element::Type type, bool forceCreate, bool *wasCreated)
{
	Element *element = dimension->lookupElementByName(name, false);

	if (element == 0 && forceCreate) {
		element = dimension->addElement(server, COMMITABLE_CAST(Database, shared_from_this()), NO_IDENTIFIER, name, type, PUser(), false);
		if (wasCreated) {
			*wasCreated = true;
		}
	} else if (element) {
		if (element->getElementType() != type) {
			element = dimension->lookupElementByName(name, true);
			dimension->changeElementType(server, COMMITABLE_CAST(Database, shared_from_this()), element, type, PUser(), true, NULL, NULL, true);
		}
	}
	return element;
}

IdentifierType SystemDatabase::checkAndCreateElement(PServer server, PDimension dimension, const string& name, Element::Type type, bool forceCreate, bool *wasCreated)
{
	Element *elem = checkAndReturnElement(server, dimension, name, type, forceCreate, wasCreated);
	if (elem) {
		return elem->getIdentifier();
	} else {
		return NO_IDENTIFIER;
	}
}

void SystemDatabase::checkAndDeleteElement(PServer server, PDimension dimension, const string& name)
{
	Element* element = dimension->lookupElementByName(name, true);

	if (element != 0) {
		dimension->deleteElement(server, COMMITABLE_CAST(Database, shared_from_this()), element, PUser(), false, NULL, false);
	}
}

void SystemDatabase::setCell(PServer server, PCube cube, IdentifierType e1, IdentifierType e2, const string& value, bool overwrite, set<PCube> &changedCubes)
{
	PDatabase db = COMMITABLE_CAST(Database, shared_from_this());
	PCubeArea path(new CubeArea(db, cube, 2));
	PSet s(new Set);
	s->insert(e1);
	path->insert(0, s);
	s.reset(new Set);
	s->insert(e2);
	path->insert(1, s);

	CellValue oldValue = cube->getCellValue(CPCubeArea(path));

	if (!oldValue.isEmpty()) {
		if ((string &)oldValue != value && overwrite) {
			cube->setCellValue(server, db, path, value, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, true, CubeArea::BASE_STRING);
		}
	} else {
		cube->setCellValue(server, db, path, value, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, true, CubeArea::BASE_STRING);
	}
}

PUser SystemDatabase::getUser(const string& name, const string& password, bool useMD5, bool *inactive)
{
	if (useMD5 && password.length() != 32) {
		return PUser();
	}

	Element* user = getUserElement(name, inactive);

	if (user) {
		PCubeArea path(new CubeArea(COMMITABLE_CAST(Database, shared_from_this()), getUserUserPropertiesCube(), 2));
		PSet s(new Set);
		s->insert(user->getIdentifier());
		path->insert(0, s);
		s.reset(new Set);
		s->insert(passwordElement);
		path->insert(1, s);

		CellValue value = getUserUserPropertiesCube()->getCellValue(CPCubeArea(path));

		if (!value.isEmpty() && value.isString()) {
			Password savedPassword(value);
			// found password, use MD5 coding
			Password receivedPassword(useMD5 ? "\t1\t"+password : password);

			if (savedPassword == receivedPassword) {
				return COMMITABLE_CAST(User, users->get(user->getIdentifier(), false));
			}
		} else {
			Logger::info << "no password for user '" << name << "' found." << endl;
		}
	}

	return PUser();
}

PUser SystemDatabase::getUser(const string& name, bool *inactive)
{
	Element* user = getUserElement(name, inactive);

	if (user) {
		return COMMITABLE_CAST(User, users->get(user->getIdentifier(), false));
	}

	return PUser();
}

PUser SystemDatabase::getExternalUser(const string& name)
{
	return COMMITABLE_CAST(User, users->get(name, false));
}

PUser SystemDatabase::getUser(IdentifierType identifier)
{
	if (useExternalUser) {
		return COMMITABLE_CAST(User, users->get(identifier, false));
	}

	Element* user = userDimension->lookupElement(identifier, false);

	if (user) {
		return COMMITABLE_CAST(User, users->get(user->getIdentifier(), false));
	}

	return PUser();
}

PUser SystemDatabase::createUser(const string& name)
{
	checkCheckedOut();
	Element* user = getUserElement(name, NULL);

	if (user) {
		return createUser(user);
	}
	return PUser();
}

PUser SystemDatabase::createUser(Element* userElement)
{
	PUser user(new User(userElement->getName(userDimension->getElemNamesVector()), 0, false));
	user->setID(userElement->getIdentifier());
	user->refreshRights();
	if (!users->isCheckedOut()) {
		users = COMMITABLE_CAST(UserList, users->copy());
	}
	users->add(user, false);
	return user;
}

PUser SystemDatabase::createExternalUser(PServer server, const string& name, vector<string>* groups, IdentifierType *createdElement)
{
	checkCheckedOut();
	useExternalUser = true;

	PDimensionList dimensions = getDimensionList(true);
	setDimensionList(dimensions);
	userDimension = lookupDimension(userDimension->getId(), true);
	dimensions->set(userDimension);
	bool wasCreated = false;
	Element *userElement = checkAndReturnElement(server, userDimension, name, Element::STRING, true, &wasCreated);
	PUser user(new User(userElement->getName(userDimension->getElemNamesVector()), groups, true)); // to have original case of the name
	user->setID(userElement->getIdentifier());

	if (createdElement) { // info required by caller
		boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
		if (wasCreated && (!session || !session->isWorker())) {
			*createdElement = userElement->getIdentifier();
		} else {
			*createdElement = NO_IDENTIFIER;
		}
	}

	Context::getContext()->saveParent(shared_from_this(), userGroupCube);
	PCubeList cs = getCubeList(true);
	setCubeList(cs);
	userGroupCube = lookupCube(userGroupCube->getId(), true);
	cs->set(userGroupCube);
	ElementsType groupelems = groupDimension->getElements(PUser(), false);

	set<PCube> changedCubes;
	for (ElementsType::iterator it = groupelems.begin(); it != groupelems.end(); ++it) {
		PCubeArea path(new CubeArea(COMMITABLE_CAST(Database, shared_from_this()), userGroupCube, 2));
		PSet s(new Set);
		s->insert(user->getId());
		path->insert(0, s);
		s.reset(new Set);
		s->insert((*it)->getIdentifier());
		path->insert(1, s);
		string value;

		for (vector<string>::iterator itg = groups->begin(); itg != groups->end(); ++itg) {
			if (!UTF8Comparer::compare((*it)->getName(groupDimension->getElemNamesVector()).c_str(), itg->c_str())) {
				value = "1";
				break;
			}
		}

		userGroupCube->setCellValue(server, COMMITABLE_CAST(Database, shared_from_this()), path, value, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, true, CubeArea::BASE_STRING);
	}
	userGroupCube->commitChanges(false, PUser(), changedCubes, false);

	user->refreshRights();
	if (!users->isCheckedOut()) {
		users = COMMITABLE_CAST(UserList, users->copy());
	}
	users->add(user, false);
	return user;
}

bool SystemDatabase::groupsUpdateRequired(PServer server, const string& name, vector<string>* groups)
{
	if (!groups) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "groups vector not received");
	}

	PUser user = getExternalUser(name);
	PCubeArea area(new CubeArea(COMMITABLE_CAST(Database, shared_from_this()), userGroupCube, 2));
	PSet s(new Set);
	s->insert(user->getId());
	area->insert(0, s);
	
	ElementsType groupElems = groupDimension->getElements(PUser(), false);
	s.reset(new Set);
	for (ElementsType::iterator it = groupElems.begin(); it != groupElems.end(); ++it) {
		s->insert((*it)->getIdentifier());
	}
	area->insert(1, s);

	PCellStream cs = userGroupCube->calculateArea(area, CubeArea::ALL, NO_RULES, false, UNLIMITED_UNSORTED_PLAN);

	while (cs->next()) {
		CellValue cv = cs->getValue();
		IdentifiersType key = cs->getKey();
		Element *elem = groupDimension->lookupElement(key[1], false);

		bool found = false;
		for (vector<string>::iterator it = groups->begin(); it != groups->end(); ++it) {
			if (!UTF8Comparer::compare(elem->getName(groupDimension->getElemNamesVector()).c_str(), it->c_str())) {
				found = true;
				break;
			}
		}

		if (found && (cv.isEmpty() || cv.compare("1") != 0)) {
			return true; // "1" has to be written
		} else if (!found && cv.compare("1") == 0) {
			return true; // value has to be deleted
		}
	}
		
	return false;
}

int SystemDatabase::existingGroupsCount(vector<string> &groups)
{
	int count = 0;

	ElementsType groupelems = groupDimension->getElements(PUser(), false);
	for (ElementsType::iterator it = groupelems.begin(); it != groupelems.end(); ++it) {
		for (vector<string>::iterator itg = groups.begin(); itg != groups.end(); ++itg) {
			if (!UTF8Comparer::compare((*it)->getName(groupDimension->getElemNamesVector()).c_str(), itg->c_str())) {
				count++;
				break;
			}
		}
	}

	if (count == 0) {
		for (vector<string>::iterator it = groups.begin(); it != groups.end(); ++it) {
			Logger::trace << "group '" << it->c_str() << "' not found in group dimension" << endl;
		}
	}

	return count;
}

bool SystemDatabase::userExist(string name, bool external)
{
	PCommitable user = users->get(name, false);

	if (!external) {
		return user != NULL;
	} else {
		return (user != NULL && userDimension->lookupElementByName(name, false) != NULL);
	}
}

void SystemDatabase::refreshUsers()
{
	if (!users->isCheckedOut()) {
		users = COMMITABLE_CAST(UserList, users->copy());
	}

	map<IdentifierType, string> renamedUsers = Context::getContext()->getRenamedUsers();
	for (map<IdentifierType, string>::const_iterator it = renamedUsers.begin(); it != renamedUsers.end(); ++it) {
		PUser user = COMMITABLE_CAST(User, users->get(it->first, true));
		if (user) {
			user->setName(it->second);
			users->set(user);
		}
	}

	for (UserList::Iterator it = users->begin(); it != users->end();) {
		PUser user = (*it)->isCheckedOut() ? COMMITABLE_CAST(User, (*it)) : COMMITABLE_CAST(User, (*it)->copy());
		if (user->refreshAll()) {
			users->set(user);
			++it;
		} else {
			// already deleted user, remove from list
			users->erase(it++);
		}
	}
}

PCommitable SystemDatabase::copy() const
{
	checkNotCheckedOut();
	PSystemDatabase newd(new SystemDatabase(*this));
	return newd;
}

bool SystemDatabase::merge(const CPCommitable &o, const PCommitable &p)
{
	return true;
}

bool SystemDatabase::mergespecial(const CPCommitable &o, const PCommitable &p, bool final)
{
	checkCheckedOut();
	bool ret = true;
	CPSystemDatabase db = CONST_COMMITABLE_CAST(SystemDatabase, o);
	CPSystemDatabase olddb = CONST_COMMITABLE_CAST(SystemDatabase, old);

	if (old != 0 && o != 0 && useExternalUser == olddb->useExternalUser) {
		useExternalUser = db->useExternalUser;
	}
	if (users->isCheckedOut()) {
		ret = users->merge(o != 0 && !final ? db->users : PUserList(), shared_from_this());
	} else if (o != 0) {
		users = db->users;
	}

	if (final) {
		commitintern();
	} else {
		if (ret) {
			ret = Database::merge(o, p);
		}

		if (ret) {
			if (userDimensionId == NO_IDENTIFIER) {
				userDimension = lookupDimensionByName(NAME_USER_DIMENSION, false);
				userDimensionId = userDimension->getId();
				groupDimension = lookupDimensionByName(NAME_GROUP_DIMENSION, false);
				groupDimensionId = groupDimension->getId();
				userPropertiesDimension = lookupDimensionByName(NAME_USER_PROPERTIES_DIMENSION, false);
				userPropertiesDimensionId = userPropertiesDimension->getId();
				rolePropertiesDimension = lookupDimensionByName(NAME_ROLE_PROPERTIES_DIMENSION, false);
				rolePropertiesDimensionId = rolePropertiesDimension->getId();
				groupPropertiesDimension = lookupDimensionByName(NAME_GROUP_PROPERTIES_DIMENSION, false);
				groupPropertiesDimensionId = groupPropertiesDimension->getId();
				roleDimension = lookupDimensionByName(NAME_ROLE_DIMENSION, false);
				roleDimensionId = roleDimension->getId();
				rightObjectDimension = lookupDimensionByName(NAME_RIGHT_OBJECT_DIMENSION, false);
				rightObjectDimensionId = rightObjectDimension->getId();
				databaseDimension = lookupDimensionByName(NAME_DATABASE_DIMENSION, false);
				databaseDimensionId = databaseDimension->getId();
				userUserPropertiesCube = lookupCubeByName(NAME_USER_USER_PROPERTIERS_CUBE, false);
				userUserPropertiesCubeId = userUserPropertiesCube->getId();
				userGroupCube = lookupCubeByName(NAME_USER_GROUP_CUBE, false);
				userGroupCubeId = userGroupCube->getId();
				roleRightObjectCube = lookupCubeByName(NAME_ROLE_RIGHT_OBJECT_CUBE, false);
				roleRightObjectCubeId = roleRightObjectCube->getId();
				groupRoleCube = lookupCubeByName(NAME_GROUP_ROLE, false);
				groupRoleCubeId = groupRoleCube->getId();
				roleRolePropertiesCube = lookupCubeByName(NAME_ROLE_ROLE_PROPERTIES_CUBE, false);
				roleRolePropertiesCubeId = roleRolePropertiesCube->getId();
				groupGroupPropertiesCube = lookupCubeByName(NAME_GROUP_GROUP_PROPERTIES_CUBE, false);
				groupGroupPropertiesCubeId = groupGroupPropertiesCube->getId();
				groupDatabaseCube = lookupCubeByName(NAME_GROUP_DATABASE_CUBE, false);
				groupDatabaseCubeId = groupDatabaseCube->getId();
			} else {
				userDimension = lookupDimension(userDimensionId, false);
				groupDimension = lookupDimension(groupDimensionId, false);
				userPropertiesDimension = lookupDimension(userPropertiesDimensionId, false);
				rolePropertiesDimension = lookupDimension(rolePropertiesDimensionId, false);
				groupPropertiesDimension = lookupDimension(groupPropertiesDimensionId, false);
				roleDimension = lookupDimension(roleDimensionId, false);
				rightObjectDimension = lookupDimension(rightObjectDimensionId, false);
				databaseDimension = lookupDimension(databaseDimensionId, false);
				userUserPropertiesCube = lookupCube(userUserPropertiesCubeId, false);
				userGroupCube = lookupCube(userGroupCubeId, false);
				roleRightObjectCube = lookupCube(roleRightObjectCubeId, false);
				groupRoleCube = lookupCube(groupRoleCubeId, false);
				roleRolePropertiesCube = lookupCube(roleRolePropertiesCubeId, false);
				groupGroupPropertiesCube = lookupCube(groupGroupPropertiesCubeId, false);
				groupDatabaseCube = lookupCube(groupDatabaseCubeId, false);
			}
			if (userPropertiesDimension) {
				passwordElement = userPropertiesDimension->lookupElementByName(PASSWORD, false)->getIdentifier();
			}
			checkOut();
		}
		if (ret && Context::getContext()->doTokenUpdate()) {
			User::updateGlobalDatabaseToken(COMMITABLE_CAST(Server, p), COMMITABLE_CAST(Database, shared_from_this()));
		}
	}

	return ret;
}

Element* SystemDatabase::getElementIntern(const string &name, PDimension dim, PDimension propDim, PCube propCube, bool *isInactive)
{
	Context::getContext()->saveParent(shared_from_this(), propCube);
	Element* elem = dim->lookupElementByName(name, false);
	Element* inactive = propDim->lookupElementByName(INACTIVE, false);
	if (isInactive) {
		*isInactive = false;
	}

	if (elem && inactive) {
		PCubeArea path(new CubeArea(COMMITABLE_CAST(Database, shared_from_this()), propCube, 2));
		PSet s(new Set);
		s->insert(elem->getIdentifier());
		path->insert(0, s);
		s.reset(new Set);
		s->insert(inactive->getIdentifier());
		path->insert(1, s);

		CellValue oldValue = propCube->getCellValue(CPCubeArea(path));

		if (!oldValue.isEmpty() && (string &)oldValue == "1") {
			elem = 0;
			if (isInactive) {
				*isInactive = true;
			}
		}
	}
	return elem;
}

ElementsType SystemDatabase::getElementsIntern(PDimension dim, PDimension propDim, PCube propCube) const
{
	Context::getContext()->saveParent(shared_from_this(), propCube);
	ElementsType elems = dim->getElements(PUser(), false);
	Element* inactive = propDim->lookupElementByName(INACTIVE, false);
	if (inactive) {
		for (ElementsType::iterator it = elems.begin(); it != elems.end();) {
			PCubeArea path(new CubeArea(CONST_COMMITABLE_CAST(Database, shared_from_this()), propCube, 2));
			PSet s(new Set);
			s->insert((*it)->getIdentifier());
			path->insert(0, s);
			s.reset(new Set);
			s->insert(inactive->getIdentifier());
			path->insert(1, s);

			CellValue oldValue = propCube->getCellValue(CPCubeArea(path));

			if (!oldValue.isEmpty() && (string &)oldValue == "1") {
				it = elems.erase(it);
			} else {
				++it;
			}
		}
	}
	return elems;
}

Element* SystemDatabase::getUserElement(const string &name, bool *inactive)
{
	return getElementIntern(name, userDimension, userPropertiesDimension, userUserPropertiesCube, inactive);
}

Element* SystemDatabase::getGroupElement(const string &name)
{
	return getElementIntern(name, groupDimension, groupPropertiesDimension, groupGroupPropertiesCube, NULL);
}

ElementsType SystemDatabase::getGroupElements()
{
	return getElementsIntern(groupDimension, groupPropertiesDimension, groupGroupPropertiesCube);
}

ElementsType SystemDatabase::getRoleElements() const
{
	return getElementsIntern(roleDimension, rolePropertiesDimension, roleRolePropertiesCube);
}

void SystemDatabase::changePassword(PServer server, PUser userChanging, IdentifierType userId, const string& new_password)
{
	Context::getContext()->saveParent(shared_from_this(), userUserPropertiesCube);
	userUserPropertiesCube = lookupCube(userUserPropertiesCube->getId(), true);
	PCubeArea path(new CubeArea(COMMITABLE_CAST(Database, shared_from_this()), userUserPropertiesCube, 2));
	PSet s(new Set);
	s->insert(userId);
	path->insert(0, s);
	s.reset(new Set);
	s->insert(passwordElement);
	path->insert(1, s);
	PCubeList cs = getCubeList(true);
	setCubeList(cs);
	cs->set(userUserPropertiesCube);
	set<PCube> changedCubes;
	Password password(new_password);
	password.increaseLevel(Password::SAVED_PASSWORD_LEVEL);
	userUserPropertiesCube->setCellValue(server, COMMITABLE_CAST(Database, shared_from_this()), path, password.getHashed(), PLockedCells(), userChanging, boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, false, CubeArea::BASE_STRING);
	userUserPropertiesCube->commitChanges(false, PUser(), changedCubes, false);
}

void SystemDatabase::updateDatabaseDim(PServer server, bool useDimWorker)
{
	checkCheckedOut();
	PDimensionList dimensions = getDimensionList(true);
	setDimensionList(dimensions);
	databaseDimension = lookupDimension(databaseDimension->getId(), true);
	dimensions->set(databaseDimension);

	PDatabaseList dbs = server->getDatabaseList(false);
	for (DatabaseList::Iterator dbit = dbs->begin(); dbit != dbs->end(); ++dbit) {
		PDatabase db = COMMITABLE_CAST(Database, *dbit);
		if (db && (db->getType() == NORMALTYPE || (db->getType() == USER_INFOTYPE && db->getName() != Server::NAME_CONFIG_DATABASE))) {
			checkAndReturnElement(server, databaseDimension, db->getName(), Element::STRING, true);
		}
	}
	if (dbs->size() != databaseDimension->sizeElements()) {
		ElementsType elements = databaseDimension->getElements(PUser(), false);
		for (ElementsType::const_iterator it = elements.begin(); it != elements.end(); ++it) {
			Element *elem = *it;
			if (!dbs->get(elem->getName(databaseDimension->getElemNamesVector()), false)) {
				databaseDimension->deleteElement(server, COMMITABLE_CAST(Database, shared_from_this()), elem, PUser(), false, NULL, useDimWorker);
			}
		}
	}
}

void SystemDatabase::updateDatabaseDim(PServer server, UpdateType type, const string &dbName, const string &dbOldName, bool useDimWorker)
{
	checkCheckedOut();
	PDimensionList dimensions = getDimensionList(true);
	setDimensionList(dimensions);
	databaseDimension = lookupDimension(databaseDimension->getId(), true);
	dimensions->set(databaseDimension);

	PDatabase sysDb = COMMITABLE_CAST(Database, shared_from_this());
	Element *elem = 0;
	switch (type) {
	case ADD: {
		elem = checkAndReturnElement(server, databaseDimension, dbName, Element::STRING, true, NULL);
		boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
		if (useDimWorker && (!session || !session->isWorker())) {
			databaseDimension->addElementEvent(server, sysDb, elem);
		}
		break;
	}
	case REMOVE:
		elem = databaseDimension->lookupElementByName(dbName, false);
		if (elem) {
			databaseDimension->deleteElement(server, sysDb, elem, PUser(), false, NULL, useDimWorker);
		}
		break;
	case RENAME:
		elem = databaseDimension->lookupElementByName(dbOldName, false);
		if (elem) {
			databaseDimension->changeElementName(server, sysDb, elem, dbName, PUser(), false, useDimWorker);
		}
		break;
	}
}

void SystemDatabase::setDbRight(PServer server, PUser user, const string &dbName)
{
	checkCheckedOut();
	PCubeList cubes = getCubeList(true);
	setCubeList(cubes);
	groupDatabaseCube = lookupCube(groupDatabaseCube->getId(), true);
	cubes->set(groupDatabaseCube);

	PDatabase sysDb = COMMITABLE_CAST(Database, shared_from_this());
	PCubeArea cellPath(new CubeArea(sysDb, groupDatabaseCube, 2));

	Element *elem = databaseDimension->lookupElementByName(dbName, false);
	PSet s(new Set);
	s->insert(elem->getIdentifier());
	cellPath->insert(1, s);

	ResultStatus status = RESULT_OK;
	set<PCube> changedCubes;
	string value("D");
	const IdentifiersType &groups = user->getUserGroups();
	for (IdentifiersType::const_iterator it = groups.begin(); it != groups.end(); ++it) {
		PSet s(new Set);
		s->insert(*it);
		cellPath->insert(0, s);

		status = groupDatabaseCube->setCellValue(server, sysDb, cellPath, value, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, false, CubeArea::BASE_STRING);
		if (status != RESULT_OK) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "SystemDatabase::setDbRight failed");
		}
	}
	if (status == RESULT_OK) {
		groupDatabaseCube->commitChanges(true, PUser(), changedCubes, false);
	}
}

}
