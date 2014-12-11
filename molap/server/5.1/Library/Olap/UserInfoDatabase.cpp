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

#include "Exceptions/FileFormatException.h"
#include "Olap/Server.h"
#include "Olap/NormalDimension.h"
#include "Olap/UserInfoDatabase.h"

namespace palo {

const string ConfigDatabase::NAME_TASKS_CUBE = "tasks";
const string ConfigDatabase::NAME_TASKS_NOTIF_CUBE = "tasks_notif";
const string ConfigDatabase::NAME_TASKS_PRIVATE_CUBE = "tasks_private";
const string ConfigDatabase::NAME_ETLS_CUBE = "etls";

const string ConfigDatabase::NAME_CONNECTIONS_DIMENSION = "connections";
const string ConfigDatabase::NAME_CONFIG_DIMENSION = "config";
const string ConfigDatabase::NAME_TASKPROPS_DIMENSION = "taskprops";
const string ConfigDatabase::NAME_NOTIFTYPES_DIMENSION = "notiftypes";
const string ConfigDatabase::NAME_TASKS_DIMENSION = "tasks";
const string ConfigDatabase::NAME_VARSETS_DIMENSION = "varsets";
const string ConfigDatabase::NAME_ETLS_DIMENSION = "etls";
const string ConfigDatabase::NAME_ETLPROPS_DIMENSION = "etlprops";

const ElementItem ConfigDatabase::CONNECTIONS_ATTR_ITEMS[] = {
	{"type", Element::STRING},
	{"description", Element::STRING},
	{"host", Element::STRING},
	{"port",Element::NUMERIC},
	{"username", Element::STRING},
	{"password", Element::STRING},
	{"active",Element::NUMERIC},
	{"useLoginCred",Element::NUMERIC},
	{"dsn", Element::STRING}
};

const ElementItem ConfigDatabase::CONFIG_ITEMS[] = {
	{"def_storage_path", Element::STRING},
	{"tasks_seq", Element::STRING},
	{"tasks_inactive", Element::STRING},
	{"scheduler_smtp_host", Element::STRING},
	{"scheduler_smtp_port", Element::STRING},
	{"scheduler_smtp_user", Element::STRING},
	{"scheduler_smtp_password", Element::STRING},
	{"scheduler_smtp_auth", Element::STRING},
	{"scheduler_smtp_starttls_enable", Element::STRING},
	{"etls_seq", Element::STRING}
};

const ElementItem ConfigDatabase::CONFIG_ATTR_ITEMS[] = {
	{"value", Element::STRING},
	{"type", Element::STRING},
	{"desc", Element::STRING},
	{"categ", Element::STRING},
	{"show", Element::NUMERIC}
};

const ElementItem ConfigDatabase::TASKPROPS_ITEMS[] = {
	{"name", Element::STRING},
	{"type", Element::STRING},
	{"desc", Element::STRING},
	{"trig_type", Element::STRING},
	{"trig", Element::STRING},
	{"def", Element::STRING},
	{"inactive", Element::STRING}
};

const ElementItem ConfigDatabase::NOTIFTYPES_ITEMS[] = {
	{"default", Element::STRING}
};

const ElementItem ConfigDatabase::VARSETS_ATTR_ITEMS[] = {
	{"data", Element::STRING}
};

const ElementItem ConfigDatabase::ETLPROPS_ITEMS[] = {
	{"name", Element::STRING},
	{"definition", Element::STRING},
	{"chagedby", Element::STRING},
	{"changeddata", Element::STRING}
};

PCommitable UserInfoDatabase::copy() const
{
	checkNotCheckedOut();
	PUserInfoDatabase newd(new UserInfoDatabase(*this));
	return newd;
}

ConfigDatabase::ConfigDatabase(const string &name) : UserInfoDatabase(name), protect(false)
{
}

ConfigDatabase::ConfigDatabase(const ConfigDatabase &d) : UserInfoDatabase(d),
	protect(d.protect), protArea(d.protArea), protValues(d.protValues)
{
}

void ConfigDatabase::notifyAddDatabase(PServer server, PUser user, bool useDimWorker) {
	if (status != UNLOADED) {
		createSystemItems(server, false);
	}
}

PCommitable ConfigDatabase::copy() const
{
	checkNotCheckedOut();
	PConfigDatabase newd(new ConfigDatabase(*this));
	return newd;
}

bool ConfigDatabase::createSystemItems(PServer server, bool forceCreate)
{
	checkCheckedOut();
	bool dbChanged = NormalDatabase::createSystemItems(server, forceCreate);

	if (getName() == Server::NAME_CONFIG_DATABASE) {
		PDatabase db = COMMITABLE_CAST(Database, shared_from_this());

		checkAndCreateDimension(server, db, NAME_CONNECTIONS_DIMENSION, 0, 0, CONNECTIONS_ATTR_ITEMS, CONNECTIONS_ATTR_ITEMS + array_size(CONNECTIONS_ATTR_ITEMS), dbChanged);
		checkAndCreateDimension(server, db, NAME_CONFIG_DIMENSION, CONFIG_ITEMS, CONFIG_ITEMS + array_size(CONFIG_ITEMS), CONFIG_ATTR_ITEMS, CONFIG_ATTR_ITEMS + array_size(CONFIG_ATTR_ITEMS), dbChanged);
		PDimension taskPropsDim = checkAndCreateDimension(server, db, NAME_TASKPROPS_DIMENSION, TASKPROPS_ITEMS, TASKPROPS_ITEMS + array_size(TASKPROPS_ITEMS), dbChanged);
		PDimension notifTypesDim = checkAndCreateDimension(server, db, NAME_NOTIFTYPES_DIMENSION, NOTIFTYPES_ITEMS, NOTIFTYPES_ITEMS + array_size(NOTIFTYPES_ITEMS), dbChanged);
		PDimension tasksDim = checkAndCreateDimension(server, db, NAME_TASKS_DIMENSION, 0, 0, dbChanged);
		checkAndCreateDimension(server, db, NAME_VARSETS_DIMENSION, 0, 0, VARSETS_ATTR_ITEMS, VARSETS_ATTR_ITEMS + array_size(VARSETS_ATTR_ITEMS), dbChanged);
		PDimension etls = checkAndCreateDimension(server, db, NAME_ETLS_DIMENSION, 0, 0, dbChanged);
		PDimension etlprops = checkAndCreateDimension(server, db, NAME_ETLPROPS_DIMENSION, ETLPROPS_ITEMS, ETLPROPS_ITEMS + array_size(ETLPROPS_ITEMS), dbChanged);

		IdentifiersType dims;
		dims.push_back(etls->getId());
		dims.push_back(etlprops->getId());
		checkAndCreateCube(server, db, NAME_ETLS_CUBE, dims, dbChanged);

		dims[0] = tasksDim->getId();
		dims[1] = taskPropsDim->getId();
		checkAndCreateCube(server, db, NAME_TASKS_CUBE, dims, dbChanged);

		PDimension userDim = lookupDimensionByName(SystemDatabase::NAME_USER_DIMENSION, false);
		dims[1] = notifTypesDim->getId();
		dims.push_back(userDim->getId());
		checkAndCreateCube(server, db, NAME_TASKS_NOTIF_CUBE, dims, dbChanged);

		dims[1] = taskPropsDim->getId();
		checkAndCreateCube(server, db, NAME_TASKS_PRIVATE_CUBE, dims, dbChanged);

		checkConfigCube(server, db, dbChanged);

		if (isDeletable()) {
			setDeletable(false);
			dbChanged = true;
		}
		if (isRenamable()) {
			setRenamable(false);
			dbChanged = true;
		}
		protect = true;
	}
	return dbChanged;
}

bool ConfigDatabase::isProtected(const IdentifiersType &key) const
{
	return protect && protValues.find(make_pair(key[0], key[1])) != protValues.end();
}

bool ConfigDatabase::isConfigCube(const string &dbName, const string &cubeName)
{
	return DB_TYPE == 3 && dbName == Server::NAME_CONFIG_DATABASE && cubeName == Cube::PREFIX_ATTRIBUTE_CUBE + NAME_CONFIG_DIMENSION;
}

PDimension ConfigDatabase::checkAndCreateElement(PServer server, PDatabase db, PDimension dim, const ElementItem *ei, bool &dbChanged)
{
	bool add = false;
	bool protect = false;
	Element *elem = dim->lookupElementByName(ei->name, false);
	if (!elem) {
		add = true;
		protect = true;
	} else {
		if (elem->getElementType() != ei->type) {
			elem->setElementType(ei->type);
			Logger::warning << "type of element '" << ei->name << "' in dimension '" << dim->getName() << "' changed" << endl;
		}
		if (!dim->isProtectedElement(ei->name)) {
			protect = true;
		}
	}
	if (add || protect) {
		if (!dim->isCheckedOut()) {
			PDimensionList dbs = db->getDimensionList(true);
			db->setDimensionList(dbs);
			dim = db->lookupDimension(dim->getId(), true);
			dbs->set(dim);
		}
		if (add) {
			dim->addElement(server, db, NO_IDENTIFIER, ei->name, ei->type, PUser(), false);
		}
		if (protect) {
			dim->addProtectedElement(ei->name);
		}
		dbChanged = true;
	}
	return dim;
}

PDimension ConfigDatabase::checkAndCreateDimension(PServer server, PDatabase db, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, bool &dbChanged)
{
	return checkAndCreateDimension(server, db, name, elemBegin, elemEnd, 0, 0, dbChanged);
}

PDimension ConfigDatabase::checkAndCreateDimension(PServer server, PDatabase db, const string &name, const ElementItem *elemBegin, const ElementItem *elemEnd, const ElementItem *attrBegin, const ElementItem *attrEnd, bool &dbChanged)
{
	string attrName;
	if (attrBegin) {
		attrName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + name + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;
	}

	PDimension dim = lookupDimensionByName(name, false);
	if (!dim) {
		dim = PDimension(new NormalDimension(name));
		dim->setDeletable(false);
		dim->setRenamable(false);
		dim->setChangable(true);
		for (const ElementItem *elem = elemBegin; elem != elemEnd; elem++) {
			checkAndCreateElement(server, db, dim, elem, dbChanged);
		}
		db->addDimension(server, dim, true, true, NULL, NULL, NULL, NULL, false);

		if (attrBegin) {
			PDimension attrDim = findDimensionByName(attrName, PUser(), true);
			for (const ElementItem *elem = attrBegin; elem != attrEnd; elem++) {
				checkAndCreateElement(server, db, attrDim, elem, dbChanged);
			}
		}
		dbChanged = true;
	} else {
		if (dim->getType() != NORMALTYPE) {
			throw FileFormatException("dimension '" + name + "' corrupted", 0);
		}
		for (const ElementItem *elem = elemBegin; elem != elemEnd; elem++) {
			dim = checkAndCreateElement(server, db, dim, elem, dbChanged);
		}
		if (attrBegin) {
			PDimension attrDim = findDimensionByName(attrName, PUser(), false);
			for (const ElementItem *elem = attrBegin; elem != attrEnd; elem++) {
				attrDim = checkAndCreateElement(server, db, attrDim, elem, dbChanged);
			}
		}
		if (dim->isDeletable() || dim->isRenamable()) {
			if (!dim->isCheckedOut()) {
				PDimensionList dbs = db->getDimensionList(true);
				db->setDimensionList(dbs);
				dim = db->lookupDimension(dim->getId(), true);
				dbs->set(dim);
			}
			dim->setDeletable(false);
			dim->setRenamable(false);
			dbChanged = true;
		}
	}
	return dim;
}

void ConfigDatabase::checkAndCreateCube(PServer server, PDatabase db, const string &name, IdentifiersType &dims, bool &dbChanged)
{
	PCube cube = lookupCubeByName(name, false);
	if (!cube) {
		cube = PCube(new Cube(db, name, &dims, Cube::NORMAL));
		cube->setDeletable(false);
		cube->setRenamable(false);
		db->addCube(server, cube, false, true, NULL, NULL, NULL, false);
		dbChanged = true;
	} else {
		bool corrupted = cube->getType() != NORMALTYPE;
		if (!corrupted) {
			const IdentifiersType *cubeDims = cube->getDimensions();
			corrupted = dims != *cubeDims;
		}
		if (corrupted) {
			throw FileFormatException("cube '" + name + "' corrupted", 0);
		}
		if (cube->isDeletable() || cube->isRenamable()) {
			if (!cube->isCheckedOut()) {
				PCubeList dbs = db->getCubeList(true);
				db->setCubeList(dbs);
				cube = db->lookupCube(cube->getId(), true);
				dbs->set(cube);
			}
			cube->setDeletable(false);
			cube->setRenamable(false);
			dbChanged = true;
		}
	}
}

void ConfigDatabase::checkConfigCube(PServer server, PDatabase db, bool &dbChanged)
{
	PCube configCube = lookupCubeByName(Cube::PREFIX_ATTRIBUTE_CUBE + NAME_CONFIG_DIMENSION, true);
	string attrName = AttributesDimension::PREFIX_ATTRIBUTE_DIMENSION + NAME_CONFIG_DIMENSION + AttributesDimension::SUFFIX_ATTRIBUTE_DIMENSION;
	PDimension attrDim = lookupDimensionByName(attrName, false);
	PDimension configDim = lookupDimensionByName(NAME_CONFIG_DIMENSION, false);

	IdentifierType typeId = attrDim->lookupElementByName(CONFIG_ATTR_ITEMS[1].name, false)->getIdentifier();
	IdentifierType categId = attrDim->lookupElementByName(CONFIG_ATTR_ITEMS[3].name, false)->getIdentifier();
	IdentifierType showId = attrDim->lookupElementByName(CONFIG_ATTR_ITEMS[4].name, false)->getIdentifier();

	IdentifierType pathId = configDim->lookupElementByName(CONFIG_ITEMS[0].name, false)->getIdentifier();
	IdentifierType hostId = configDim->lookupElementByName(CONFIG_ITEMS[3].name, false)->getIdentifier();
	IdentifierType portId = configDim->lookupElementByName(CONFIG_ITEMS[4].name, false)->getIdentifier();
	IdentifierType userId = configDim->lookupElementByName(CONFIG_ITEMS[5].name, false)->getIdentifier();
	IdentifierType passwordId = configDim->lookupElementByName(CONFIG_ITEMS[6].name, false)->getIdentifier();
	IdentifierType authId = configDim->lookupElementByName(CONFIG_ITEMS[7].name, false)->getIdentifier();
	IdentifierType enableId = configDim->lookupElementByName(CONFIG_ITEMS[8].name, false)->getIdentifier();
	IdentifierType etls_seqId = configDim->lookupElementByName(CONFIG_ITEMS[9].name, false)->getIdentifier();

	string s_string = "string";
	string s_int = "int";
	string s_bool = "bool";
	protValues[make_pair(typeId, pathId)] = s_string;
	protValues[make_pair(typeId, hostId)] = s_string;
	protValues[make_pair(typeId, portId)] = s_int;
	protValues[make_pair(typeId, userId)] = s_string;
	protValues[make_pair(typeId, passwordId)] = "password";
	protValues[make_pair(typeId, authId)] = s_bool;
	protValues[make_pair(typeId, enableId)] = s_bool;
	protValues[make_pair(typeId, etls_seqId)] = s_int;

	string s_scheduler = "scheduler";
	protValues[make_pair(categId, pathId)] = "files";
	protValues[make_pair(categId, hostId)] = s_scheduler;
	protValues[make_pair(categId, portId)] = s_scheduler;
	protValues[make_pair(categId, userId)] = s_scheduler;
	protValues[make_pair(categId, passwordId)] = s_scheduler;
	protValues[make_pair(categId, authId)] = s_scheduler;
	protValues[make_pair(categId, enableId)] = s_scheduler;
	protValues[make_pair(categId, etls_seqId)] = "etl";

	string s_1 = "1";
	protValues[make_pair(showId, pathId)] = s_1;
	protValues[make_pair(showId, hostId)] = s_1;
	protValues[make_pair(showId, portId)] = s_1;
	protValues[make_pair(showId, userId)] = s_1;
	protValues[make_pair(showId, passwordId)] = s_1;
	protValues[make_pair(showId, authId)] = s_1;
	protValues[make_pair(showId, enableId)] = s_1;
	protValues[make_pair(showId, etls_seqId)] = "";

	protArea.reset(new CubeArea(db, configCube, 2));
	PSet s(new Set);
	s->insert(typeId);
	s->insert(categId);
	s->insert(showId);
	protArea->insert(0, s);
	s.reset(new Set);
	s->insert(pathId);
	s->insert(hostId);
	s->insert(portId);
	s->insert(userId);
	s->insert(passwordId);
	s->insert(authId);
	s->insert(enableId);
	s->insert(etls_seqId);
	protArea->insert(1, s);

	bool changed = false;
	PCellStream cs = configCube->calculateArea(protArea, CubeArea::ALL, NO_RULES, false, UNLIMITED_UNSORTED_PLAN);
	while (cs->next()) {
		const IdentifiersType &key = cs->getKey();
		CellValue value = cs->getValue();
		if (value.isEmpty()) {
			set<PCube> changedCubes;
			map<pair<IdentifierType, IdentifierType>, string>::iterator it = protValues.find(make_pair(key[0], key[1]));
			if (it != protValues.end()) {
				PCubeArea path(new CubeArea(db, configCube, key));
				configCube->setCellValue(server, db, path, it->second, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, false, 0, changedCubes, false, CubeArea::NONE);
				changed = true;
			}
		}
	}
	if (changed) {
		set<PCube> changedCubes;
		configCube->commitChanges(false, PUser(), changedCubes, false);
		dbChanged = true;
	}
}

}
