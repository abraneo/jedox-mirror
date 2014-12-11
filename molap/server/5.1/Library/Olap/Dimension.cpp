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

#include "Olap/Dimension.h"

#include "Exceptions/FileFormatException.h"

#include "InputOutput/FileReader.h"

#include "Olap/AliasDimension.h"
#include "Olap/AttributesDimension.h"
#include "Olap/Cube.h"
#include "Olap/CubeDimension.h"
#include "Olap/Database.h"
#include "Olap/DimensionDimension.h"
#include "Olap/NormalDimension.h"
#include "Olap/RightsDimension.h"
#include "Olap/VirtualDimension.h"
#include "Olap/Server.h"
#include "Olap/SubsetViewDimension.h"
#include "Olap/UserInfoDimension.h"
#include "Olap/PaloSession.h"
#include "InputOutput/JournalFileReader.h"

#include "Worker/DimensionWorker.h"


#define NEW_RIGHTS_CALC

namespace palo {

const size_t Dimension::MAX_ELEMS_IN_VECTOR = 100000;

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a dimension
////////////////////////////////////////////////////////////////////////////////

PDimension Dimension::loadDimensionFromType(PServer server, FileReader* file, const string& name, Dimension::SaveType type)
{
	int isDeletable = file->getDataInteger(3);
	int isRenamable = file->getDataInteger(4);
	bool isChangable_saved = file->getDataBool(5); // isChangable is redefined for some dimensions

	bool isChangable = true;
	PDimension dimension;

	switch (type) {
	case Dimension::ALIAS: {
		string nameAliasDatabase = file->getDataString(3);
		string nameAliasDimension = file->getDataString(4);
		PDatabase aliasDatabase = server->findDatabaseByName(nameAliasDatabase, PUser(), true, false);
		PDimension aliasDimension = aliasDatabase->findDimensionByName(nameAliasDimension, PUser(), false);
		isChangable = false;

		dimension = PDimension(new AliasDimension(name, aliasDimension->getId(), aliasDatabase->getId()));
		break;
	}

	case Dimension::NORMAL:
		dimension = PDimension(new NormalDimension(name));
		break;

	case Dimension::RIGHTS:
	case Dimension::CONFIG:
	case Dimension::CELLPROPS:
		if (name == SystemDatabase::NAME_RIGHT_OBJECT_DIMENSION || name == SystemDatabase::NAME_SESSION_PROPERTIES_DIMENSION ||
			name == SystemDatabase::NAME_JOB_PROPERTIES_DIMENSION || name == SystemDatabase::NAME_LICENSE_PROPERTIES_DIMENSION ||
			name == SystemDatabase::NAME_MESSAGE_DIMENSION)
		{
			isChangable = false;
		}
		dimension = PDimension(new RightsDimension(name, type));
		break;

	case Dimension::ATTRIBUTES:
		dimension = PDimension(new AttributesDimension(name));
		break;

	case Dimension::CUBE:
		isChangable = false;
		dimension = PDimension(new CubeDimension(name));
		break;

	case Dimension::DIMENSION:
		isChangable = false;
		dimension = PDimension(new DimensionDimension(name));
		break;

	case Dimension::SUBSETVIEW:
		isChangable = true;
		dimension = PDimension(new SubsetViewDimension(name));
		break;

	case Dimension::USERINFO:
		dimension = PDimension(new UserInfoDimension(name));
		break;

	case Dimension::VIRTUAL:
		isChangable = false;
		dimension = PDimension(new VirtualDimension(name));
		break;

	default:
		Logger::error << "found unknown dimension type '" << type << "' for dimension '" << name << "'" << endl;
		throw FileFormatException("unknown dimension type", file);
	}

	dimension->setDeletable(isDeletable != 0);
	dimension->setRenamable(isRenamable != 0);
	dimension->setChangable(isChangable);

	if (isChangable != isChangable_saved) {
		Logger::warning << "dimension '" << dimension->getName() << "' is modified to be unchangeable" << endl;
	}

	return dimension;
}

////////////////////////////////////////////////////////////////////////////////
// getter and setter
////////////////////////////////////////////////////////////////////////////////

void Dimension::setStatus(PDatabase database, DimensionStatus status)
{
	this->status = status;

	if (status == CHANGED) {
		database->setStatus(Database::CHANGED);
	}
}

uint32_t Dimension::getToken() const {
	return token;
}

vector<CPCube> Dimension::getCubes(PUser user, PDatabase db, const ItemType *filterType) const
{
	vector<CPCube> result;
	vector<CPCube> cubes = db->getCubes(user);

	for (vector<CPCube>::const_iterator c = cubes.begin(); c != cubes.end(); ++c) {
		const IdentifiersType* dimensions = (*c)->getDimensions();
		IdentifiersType::const_iterator d = find(dimensions->begin(), dimensions->end(), getId());

		if (d != dimensions->end()) {
			if (!filterType || *filterType == (*c)->getType()) {
				result.push_back(*c);
            }
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
// internal function
////////////////////////////////////////////////////////////////////////////////

bool Dimension::isLockedByCube(PDatabase db)
{
	vector<CPCube> cubes = db->getCubes(PUser());

	for (vector<CPCube>::const_iterator c = cubes.begin(); c != cubes.end(); ++c) {
		const IdentifiersType* dimensions = (*c)->getDimensions();
		IdentifiersType::const_iterator d = find(dimensions->begin(), dimensions->end(), getId());

		if (d != dimensions->end()) {
			if ((*c)->hasLockedArea()) {
				return true;
			}
		}
	}

	return false;
}

DimensionList::DimensionList(const DimensionList &l) :
	CommitableList(l)
{
}

PCommitableList DimensionList::createnew(const CommitableList& l) const
{
	return PCommitableList(new DimensionList(dynamic_cast<const DimensionList &>(l)));
}

////////////////////////////////////////////////////////////////////////////////
// functions to load and save a dimension
////////////////////////////////////////////////////////////////////////////////

uint32_t Dimension::loadDimensionOverview(FileReader* file)
{
	const string section = "DIMENSION " + StringUtils::convertToString(getId());
	uint32_t sizeElements = 0;

	if (file->isSectionLine() && file->getSection() == section) {
		file->nextLine();

		if (file->isDataLine()) {
			uint32_t level = file->getDataInteger(2);
			uint32_t indent = file->getDataInteger(3);
			uint32_t depth = file->getDataInteger(4);

			sizeElements = file->getDataInteger(5);
			elementList->setNewIDStart(sizeElements);

			maxLevel = level;
			maxIndent = indent;
			maxDepth = depth;

			file->nextLine();
		}
	} else {
		throw FileFormatException("Section '" + section + "' not found.", file);
	}

	return sizeElements;
}

void Dimension::loadDimensionElementParents(FileReader* file, Element &element, IdentifiersType* parents, uint32_t maxId, IdsToParentsMap &parentsMap)
{
	elementList->checkCheckedOut();

	IdsToParentsMap::const_iterator it = parentsMap.find(*parents);
	if (it != parentsMap.end()) {
		// this combination of parents already exists => share parent list
		element.setParents(it->second);
		return;
	}

	PParents parentIds = element.getParents(true);
	parentsMap[*parents] = parentIds; // save to reuse for other elements

	IdentifiersType::const_iterator parentsIter = parents->begin();

	for (; parentsIter != parents->end(); ++parentsIter) {
		IdentifierType id = *parentsIter;

		if (0 <= id && id < maxId) {
			parentIds->push_back(id);
		} else {
			Logger::error << "parent element identifier '" << getId() << "' of element '" << element.getName(getElemNamesVector()) << "' is greater or equal than maximum (" << maxId << ")" << endl;
			throw FileFormatException("illegal identifier in parents list", file);
		}
	}
}

void Dimension::loadDimensionElementChildren(FileReader* file, Element &element, IdentifiersType* children, vector<double>* weights, uint32_t sizeElements)
{
	IdentifiersWeightType *childrenWeight = element.getChildren(true);

	childrenWeight->resize(children->size());

	IdentifiersType::const_iterator childrenIter = children->begin();
	vector<double>::const_iterator weightsIter = weights->begin();

	IdentifiersWeightType::iterator j = childrenWeight->begin();

	for (; childrenIter != children->end(); ++childrenIter, ++weightsIter, ++j) {
		IdentifierType id = *childrenIter;

		if (0 <= id && id < sizeElements) {
			(*j).first = id;
			(*j).second = *weightsIter;
		} else {
			Logger::error << "child element identifier '" << getId() << "' of element '" << element.getName(getElemNamesVector()) << "' is greater or equal than maximum (" << sizeElements << ")" << endl;
			throw FileFormatException("illegal identifier in children list", file);
		}
	}
}

static IdentifierType remapIds(IdentifiersType &ids, ElementOld2NewMap &dimReMap)
{
	IdentifierType result = NO_IDENTIFIER;
	for (IdentifiersType::iterator id = ids.begin(); id != ids.end(); ++id) {
		IdentifierType newId = dimReMap.translate(*id);
		if (newId == NO_IDENTIFIER) {
			result = *id;
			break;
		}
		*id = newId;
	}
	return result;
}

void Dimension::loadDimensionElement(FileReader *file, uint32_t sizeElements, IdsToParentsMap &parentMap, ElementOld2NewMap *dimReMap)
{
	IdentifierType id = file->getDataInteger(0);
	string name = file->getDataString(1);
	if (dimReMap) {
		IdentifierType newId = dimReMap->translate(id);
		if (newId == NO_IDENTIFIER) {
			Logger::error << "no element translation for element identifier '" << id << "' name: '" << name << "'" << endl;
			throw FileFormatException("wrong element identifier found", file);
		}
		id = newId;
	}
	PositionType pos = file->getDataInteger(2);
	long i = file->getDataInteger(3);
	Element::Type type = Element::UNDEFINED;

	switch (i) {
	case 1:
		type = Element::NUMERIC;
		break;
	case 2:
		type = Element::STRING;
		break;
	case 4:
		type = Element::CONSOLIDATED;
		break;

	default:
		Logger::error << "element '" << name << "' has unknown type '" << i << "'" << endl;
		throw FileFormatException("element has wrong type", file);
	}

	string isStrCons = file->getDataString(4);
	long level = file->getDataInteger(5);
	long indent = file->getDataInteger(6);
	long depth = file->getDataInteger(7);
	IdentifiersType parents = file->getDataIdentifiers(8);
	IdentifiersType children = file->getDataIdentifiers(9);

	if (dimReMap) {
		IdentifierType errorId = remapIds(parents, *dimReMap);
		if (errorId != NO_IDENTIFIER) {
			Logger::error << "no element translation for element identifier '" << errorId << "' referenced by element '" << name << "'" << endl;
			throw FileFormatException("wrong element identifier found", file);
		}
		errorId = remapIds(children, *dimReMap);
		if (errorId != NO_IDENTIFIER) {
			Logger::error << "no element translation for element identifier '" << errorId << "' referenced by element '" << name << "'" << endl;
			throw FileFormatException("wrong element identifier found", file);
		}
	}

	vector<double> weights = file->getDataDoubles(10);

	if (id < 0 || id >= sizeElements) {
		Logger::error << "element identifier '" << getId() << "' of element '" << name << "' is greater or equal than maximum (" << sizeElements << ")" << endl;
		throw FileFormatException("wrong element identifier found", file);
	}

	Element element(id);
	maxId = max(maxId, id);
	minId = min(minId, id);
	StringVector::StringId nameId = namesMap->pushToVector(name);
	element.setName(nameId);
	element.setPosition(pos);
	element.setLevel(level);
	element.setIndent(indent);
	element.setDepth(depth);

	// children to parent
	if (!parents.empty()) {
		loadDimensionElementParents(file, element, &parents, sizeElements, parentMap);
	}

	// parent to children
	if (children.size() != weights.size()) {
		Logger::error << "size of children list and size of children weight list is not equal" << endl;
		throw FileFormatException("children size != children weight size", file);
	}

	if (!children.empty()) {
		loadDimensionElementChildren(file, element, &children, &weights, sizeElements);
	}

	// check element type for consolidated elements
	if (!children.empty() && type != Element::CONSOLIDATED) {
		type = Element::CONSOLIDATED;
	} else if (children.empty() && type == Element::CONSOLIDATED) {
		type = Element::NUMERIC;
	}

	element.setElementType(type);

	// string consolidations
	if (isStrCons == "1" && type == Element::CONSOLIDATED) {
		element.setStringConsolidation(true);
	}

	IdentifierType internalId = elementList->addElement(element, PDatabase(), 0);
	Element *pelement = &(*elementList)[internalId];

	// update mapping
	idsMap->insert(pelement->getIdentifier(), internalId, false); // will be sorted outside
	if (posIndex->size() <= pelement->getPosition()) {
		posIndex->resize(pelement->getPosition() + 128);
	}
	(*posIndex)[pelement->getPosition()] = internalId;
	namesMap->insert(pelement->getNameId(), internalId, false);
}


void Dimension::loadDimensionElements(FileReader* file, uint32_t sizeElements, ElementOld2NewMap *dimReMap)
{
	const string section = "ELEMENTS DIMENSION " + StringUtils::convertToString(getId());
	maxId = 0;
	minId = 0xFFFFFFFF;

	IdsToParentsMap parentsMap;

	// load elements
	if (file->isSectionLine() && file->getSection() == section) {
		file->nextLine();

		while (file->isDataLine()) {
			loadDimensionElement(file, sizeElements, parentsMap, dimReMap);

			file->nextLine();
		}
		IdIdSlimMap::iterator it = idsMap->begin();
		//Logger::trace << "idsMap sort..." << endl;
		idsMap->sort(true);
		//Logger::trace << "namesMap sort..." << endl;
		namesMap->sort(true);
	} else {
		throw FileFormatException("Section '" + section + "' not found.", file);
	}
}

void Dimension::loadDimension(PServer server, PDatabase db, FileReader* file)
{
	Logger::trace << "loading dimension '" << getName() << "'. " << endl;

	elementList = PElementList(new ElementList());
	idsMap = PIdIdSlimMap(new IdIdSlimMap(4096));
	namesMap = PNameIdSlimMap(new NameIdSlimMap(4096));
	posIndex = PIdVector(new IdVector());
	maxId = 0;
	minId = 0xFFFFFFFF;
	maxLevel = 0;
	maxIndent = 0;
	maxDepth = 0;

	ElementOld2NewMap *dimReMap = db->getDimensionMap(getId());

	// load dimension and elements from file
	uint32_t sizeElements = loadDimensionOverview(file);
	if (dimReMap) {
		sizeElements = (uint32_t)dimReMap->size()+1;
	}
	loadDimensionElements(file, sizeElements, dimReMap);

	// database is now loaded (memory and file image are in sync)
	setStatus(db, LOADED);

	changedElementsInfo = true;

	updateElementsInfo();
}

void Dimension::saveDimensionType(FileWriter* file)
{
	file->appendIdentifier(getId());
	file->appendEscapeString(getName());
	file->appendInteger(getDimensionType());
	if (getDimensionType() == Dimension::ALIAS) {
		CPAliasDimension a = CONST_COMMITABLE_CAST(AliasDimension, shared_from_this());
		file->appendEscapeString(Context::getContext()->getServer()->lookupDatabase(a->dbId, false)->getName());
		file->appendEscapeString(a->alias(false)->getName());
	}
	file->appendInteger(deletable ? 1 : 0);
	file->appendInteger(renamable ? 1 : 0);
	file->appendInteger(changable ? 1 : 0);
	file->nextLine();
}

void Dimension::saveDimensionOverview(FileWriter* file)
{
	const string section = "DIMENSION " + StringUtils::convertToString(getId());

	file->appendComment("Description of data: ");
	file->appendComment("ID;NAME,LEVEL,INDENT,DEPTH,SIZE_ELEMENTS; ");
	file->appendSection(section);
	file->appendIdentifier(getId());
	file->appendEscapeString(getName());
	file->appendInteger(getLevel());
	file->appendInteger(getIndent());
	file->appendInteger(getDepth());
	file->appendInteger((int32_t)elementList->getLastId());
}

void Dimension::saveDimensionElement(FileWriter* file, const Element* element)
{
	file->appendIdentifier(element->getIdentifier());
	file->appendEscapeString(element->getName(getElemNamesVector()));
	file->appendInteger(element->getPosition());

	switch (element->getElementType()) {
	case Element::NUMERIC:
		file->appendInteger(1);
		file->appendInteger(0);
		break;

	case Element::STRING:
		file->appendInteger(2);
		file->appendInteger(0);
		break;

	case Element::CONSOLIDATED:
		file->appendInteger(4);

		if (element->isStringConsolidation()) {
			file->appendInteger(1);
		} else {
			file->appendInteger(0);
		}

		break;

	default:
		Logger::info << "saveDimension: Element '" << element->getIdentifier() << "' has wrong type." << endl;
		file->appendInteger(0);
		file->appendInteger(0);
		break;
	}

	file->appendInteger(element->getLevel());
	file->appendInteger(element->getIndent());
	file->appendInteger(element->getDepth());

	// parents
	CPParents parentIds = element->getParents();
	file->appendIdentifiers(parentIds->begin(), parentIds->end());

	// children
	const IdentifiersWeightType *childrenWeights = element->getChildren();

	IdentifiersType identifiers(childrenWeights->size());
	vector<double> weights(childrenWeights->size());

	IdentifiersWeightType::const_iterator p = childrenWeights->begin();
	IdentifiersType::iterator q = identifiers.begin();
	vector<double>::iterator k = weights.begin();

	for (; p != childrenWeights->end(); ++p, ++q, ++k) {
		*q = (*p).first;
		*k = (*p).second;
	}

	file->appendIdentifiers(identifiers.begin(), identifiers.end());
	file->appendDoubles(&weights);
}

void Dimension::saveDimensionElements(FileWriter* file)
{
	const string section = "ELEMENTS DIMENSION " + StringUtils::convertToString(getId());

	// write data for dimension
	file->appendComment("Description of data: ");
	file->appendComment("ID;NAME,POSITION,TYPE,STRING_CONSOLIDATION,LEVEL,INDENT,DEPTH,PARENTS,CHILDREN,CHILDREN_WEIGHT; ");
	file->appendSection(section);

	for (ElementList::const_iterator i = elementList->begin(); i != elementList->end(); ++i) {
		const Element &element = *i;
		if (element.getElementType() == Element::UNDEFINED) {
			continue;
		}
		saveDimensionElement(file, &element);
		file->nextLine();
	}
}

void Dimension::saveDimension(PDatabase db, FileWriter* file)
{
	// save dimension and elements to file
	saveDimensionOverview(file);
	saveDimensionElements(file);

	// database is now saved (memory and file image are in sync)
	setStatus(db, LOADED);
}

////////////////////////////////////////////////////////////////////////////////
// getter and setter
////////////////////////////////////////////////////////////////////////////////

LevelType Dimension::getLevel() const
{
	return maxLevel;
}

IndentType Dimension::getIndent() const
{
	return maxIndent;
}

DepthType Dimension::getDepth() const
{
	return maxDepth;
}
;

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////

set<Element*> Dimension::ancestors(Element* element) const
{
	set<Element*> ac;

	// add myself as trivial ancestor
	ac.insert(element);

	// find direct parents
	CPParents parents = element->getParents();

	if (!parents) {
		return ac;
	}

	// add direct parents and their ancestors
	for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
		IdentifierType elementId = *iter;
		Element* parent = lookupElement(elementId, false);

		if (parent && ac.find(parent) == ac.end()) {
			set<Element*> aac = ancestors(parent);
			ac.insert(aac.begin(), aac.end());
		}
	}

	return ac;
}

////////////////////////////////////////////////////////////////////////////////
// element functions
////////////////////////////////////////////////////////////////////////////////

set<Element *> Dimension::descendants(Element *parent) const
{
	set<Element *> result;

	// add myself as trivial descendant
	result.insert(parent);

	// find direct parents
	const IdentifiersWeightType *children = parent->getChildren();

	if (!children) {
		return result;
	}

	// add direct parents and their ancestors
	for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
		Element *child = lookupElement(it->first, false);

		if (child && result.find(child) == result.end()) {
			set<Element *> desc = descendants(child);
			result.insert(desc.begin(), desc.end());
		}
	}

	return result;
}

void Dimension::descendants(const IdentifiersWeightType *elems, set<IdentifierType> &result) const
{
	if (!elems) {
		return;
	}

	for (IdentifiersWeightType::const_iterator it = elems->begin(); it != elems->end(); ++it) {

		if (result.find(it->first) != result.end()) {
			// element already inserted into set
			continue;
		}

		// add myself as trivial descendant
		result.insert(it->first);

		// find direct children
		Element *elem = lookupElement(it->first, false);
		if (!elem) {
			throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "child element not found in dimension", "id", it->first);
		}

		const IdentifiersWeightType *children = elem->getChildren();
		descendants(children, result);
	}
}

void Dimension::clearElements(PServer server, PDatabase db, PUser user, bool useDimWorker, bool useJournal)
{
	if (!protectedElems.empty()) {
		ElementsType elements = getElements(PUser(), false);
		for (ElementsType::iterator i = elements.begin(); i != elements.end(); ++i) {
			if (!isProtectedElement((*i)->getName(getElemNamesVector()))) {
				deleteElement(server, db, *i, user, true, NULL, useDimWorker);
			}
		}
	} else {
		checkCheckedOut();
		db->checkCheckedOut(); // cubes using this dimension will be erased

		checkElementAccessRight(user.get(), db, RIGHT_DELETE);

		if (user && !isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		// sync: make elementList, idsMap, namesMap, posIndex copy
		checkOut(true, true, true);

		vector<CPCube> cubes = getCubes(PUser(), db);
		PCubeList cubeList;
		Context *context = 0;

		for (vector<CPCube>::iterator c = cubes.begin(); c != cubes.end(); ++c) {
			if (cubeList == 0) {
				cubeList = db->getCubeList(true);
				db->setCubeList(cubeList);
				context = Context::getContext();
			}
			PCube cube = COMMITABLE_CAST(Cube, cubeList->get((*c)->getId(), true));
			context->saveParent(db, cube);
			cubeList->set(cube);
			cube->disableRules(server, db, CONST_COMMITABLE_CAST(Dimension, shared_from_this()));
			cube->clearCells(server, db, PUser(), false);
		}

		elementList->clear();
		elementList->idh = PIdHolder(new IdHolder());

		idsMap->clear();
		namesMap->clear();
		posIndex->clear();

		maxId = 0;
		minId = 0xFFFFFFFF;
		maxLevel = 0;
		maxIndent = 0;
		maxDepth = 0;

		changedElementsInfo = false;

		PJournalMem journal = db->getJournal();

		if (journal != 0 && useJournal) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_DIMENSION_CLEAR);
			journal->appendInteger(getId());
			journal->nextLine();
		}
	}
}

ElementsType Dimension::getElements(PUser user, bool onlyRoot, uint64_t *hiddenCount) const
{
	if (hiddenCount) {
		*hiddenCount = 0;
	}
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
	checkElementAccessRight(user.get(), db, RIGHT_READ);

	bool checkRights = User::checkUser(user) && hasRightsCube() && db->getHideElements();
	IdentifierType totalElements = (IdentifierType) sizeElements();

	ElementsType result;
	if (!onlyRoot && !checkRights) {
		result.reserve(totalElements);
	}

	User::ElemRightsMap *elemRights = 0;
	RightsType minRight = RIGHT_READ;
	const IdentifiersType *userGroups = 0;

	if (checkRights) {
		const User::DimRights *dimRights = user->getDimRights(db->getId(), getId());
		if (dimRights) {
			if (dimRights->maxRight < RIGHT_READ) {
				return result;
			}
			elemRights = dimRights->elemRights.get();
			minRight = dimRights->minRight;
			userGroups = &user->getUserGroups();
		}
	}

	for (IdentifierType position = 0; position < totalElements; position++) {
		const Element *element = lookupElementByPosition(position, false);

		if (element && element->getElementType() != Element::UNDEFINED) {
			bool addElem = false;
			if (checkRights) {
				if (minRight >= RIGHT_READ || user->checkElementRight(elemRights, userGroups, element->getIdentifier(), RIGHT_READ)) {
					addElem = true;
					if (onlyRoot && element->getParentsCount()) {
						CPParents parentIds = element->getParents();
						for (Parents::const_iterator parentit = parentIds->begin(); parentit != parentIds->end(); ++parentit) {
							// if the parent is visible - do not add child to result
							if (minRight >= RIGHT_READ || user->checkElementRight(elemRights, userGroups, *parentit, RIGHT_READ)) {
								if (hiddenCount) {
									++(*hiddenCount);
								}
								addElem = false;
								break;
							}
						}
					}
				} else {
					// no rights to see this element - it is hidden
					if (hiddenCount) {
						bool increaseCounter = false;
						if (element->getChildrenCount()) {
							const IdentifiersWeightType *childrenIds = element->getChildren();
							if (childrenIds && childrenIds->size()) {
								for (IdentifiersWeightType::const_iterator i = childrenIds->begin(); i != childrenIds->end(); ++i) {
									// if the child is visible - increase hidden counter
									if (minRight >= RIGHT_READ || user->checkElementRight(elemRights, userGroups, i->first, RIGHT_READ)) {
										increaseCounter = true;
										break;
									}
								}
							}
						}
						if (!increaseCounter && element->getParentsCount()) {
							CPParents parentIds = element->getParents();
							for (Parents::const_iterator parentit = parentIds->begin(); parentit != parentIds->end(); ++parentit) {
								// if the parent is visible - increase hidden counter
								if (minRight >= RIGHT_READ || user->checkElementRight(elemRights, userGroups, *parentit, RIGHT_READ)) {
									increaseCounter = true;
									break;
								}
							}
						}
						if (increaseCounter) {
							++(*hiddenCount);
						}
					}
				}
			} else {
				addElem = !onlyRoot || !element->getParentsCount();
			}
			if (addElem) {
				result.push_back(const_cast<Element *>(element));
			}
		}
	}
	return result;
}

void Dimension::moveElement(PServer server, PDatabase db, Element *element, PositionType newPosition, PUser user, bool useJournal)
{
	checkCheckedOut();

	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	if (0 <= newPosition && newPosition < sizeElements()) {
		PositionType oldPosition = element->getPosition();
		IdentifierType internalId = (*idsMap)[element->getIdentifier()];
		int dst = oldPosition;
		int step = 0;
		int count = 0;

		// find direction
		if (oldPosition < newPosition) {
			step = 1;
			count = newPosition - oldPosition;
		} else if (oldPosition > newPosition) {
			step = -1;
			count = oldPosition - newPosition;
		}

		if (count) {
			// sync: make copy of elementList and index of positions
			if (checkOut(false, false, true)) {
				// get element copy pointer
				element = lookupElement(element->getIdentifier(), true);
			}

			for (int src = dst + step; count; src += step, dst += step, count--) {
				IdentifierType currentInternalId = (*posIndex)[src];

				Element *current = lookupElementByInternal(currentInternalId, true);

				current->setPosition(dst);
				(*posIndex)[dst] = currentInternalId;
			}
			(*posIndex)[newPosition] = internalId;
			element->setPosition(newPosition);

			setStatus(db, CHANGED);
		}
	} else {
		throw ParameterException(ErrorException::ERROR_INVALID_POSITION, "the position of an element has to be less than the number of elements", "position", (int)newPosition);
	}

	PJournalMem journal = db->getJournal();

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_MOVE);
		journal->appendInteger(getId());
		journal->appendInteger(element->getIdentifier());
		journal->appendInteger(element->getPosition());
		journal->nextLine();
	}
}

void Dimension::moveElements(PServer server, PDatabase db, vector<pair<Element *, PositionType> > &elem_pos, PUser user, bool useJournal)
{
	checkCheckedOut();

	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	size_t elemCount = sizeElements();
	IdVector oldPos = *posIndex;
	for (size_t i = 0; i < elemCount; i++) {
		(*posIndex)[i] = NO_IDENTIFIER;
	}

	PositionType dst;
	for (size_t i = 0; i < elem_pos.size(); i++) {
		Element *elem = elem_pos[i].first;
		dst = elem_pos[i].second;

		IdentifierType internalId = oldPos[elem->getPosition()];
		(*posIndex)[dst] = internalId;
		elem->setPosition(dst);
	}

	// find the first empty position in posIndex
	for (dst = 0; dst < elemCount; dst++) {
		if ((*posIndex)[dst] == NO_IDENTIFIER) {
			break;
		}
	}
	if (dst < elemCount) {
		for (size_t j = 0; j < elemCount; j++) {
			IdentifierType internalId = oldPos[j];
			Element *elem = lookupElementByInternal(internalId, true);
			PositionType pos = elem->getPosition();

			if ((*posIndex)[pos] != NO_IDENTIFIER && (*posIndex)[pos] == internalId) {
				continue; // this element was already updated
			} else {
				// use the empty position in posIndex
				(*posIndex)[dst] = internalId;
				elem->setPosition(dst);
			}

			// find the next empty position in posIndex
			for (dst++; dst < elemCount; dst++) {
				if ((*posIndex)[dst] == NO_IDENTIFIER) {
					break;
				}
			}
			if (dst == elemCount) {
				break;
			}
		}
	}

	PJournalMem journal = db->getJournal();

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_MOVE_BULK);
		journal->appendInteger(getId());

		vector<uint32_t> v(elem_pos.size());
		for (size_t i = 0; i < elem_pos.size(); i++) {
			v[i] = elem_pos[i].first->getIdentifier();
		}
		journal->appendIdentifiers(v.begin(), v.end());

		for (size_t i = 0; i < elem_pos.size(); i++) {
			v[i] = elem_pos[i].second;
		}
		journal->appendIdentifiers(v.begin(), v.end());
		journal->nextLine();
	}
}

PSet Dimension::getElemIds(CubeArea::ExpandStarType type) const
{
	PSet s(new Set);
	if (getType() == SYSTEMTYPE && getDimensionType() == Dimension::ALIAS) {
		ElementsType elements = getElements(PUser(), false);
		for (ElementsType::iterator it = elements.begin(); it != elements.end(); ++it) {
			s->insert((*it)->getIdentifier());
		}
	} else {
		PElementList elems = getElementList();
		for (ElementList::const_iterator it = elems->begin(); it != elems->end(); ++it) {
			Element &elem = *it;
			bool insert = false;
			if (elem.getElementType() != Element::UNDEFINED) {
				switch (type) {
				case CubeArea::BASE_ELEMENTS:
					insert = elem.getChildrenCount() == 0;
					break;
				case CubeArea::TOP_ELEMENTS:
					insert = elem.getParentsCount() == 0;
					break;
				case CubeArea::ALL_ELEMENTS:
					insert = true;
					break;
				case CubeArea::NUMERIC_ELEMENTS:
					insert = (elem.getElementType() != Element::STRING && !elem.isStringConsolidation());
					break;
				}
			}
			if (insert) {
				s->insert(elem.getIdentifier());
			}
		}
	}
	return s;
}

bool Dimension::checkOut(bool ids, bool names, bool positions)
{
	checkCheckedOut();

	bool result = false;
	CPDimension oldDim = CONST_COMMITABLE_CAST(Dimension, old);

	if (!elementList->isCheckedOut()) {
		elementList = COMMITABLE_CAST(ElementList, elementList->copy());
		//cout << "EL copy dim: " << getId() << " version: " << elementList->getVersion() << " version: " << elementList->old->getVersion() << endl;
		result = true;
	}
	if (ids && oldDim && oldDim->idsMap == idsMap) {
		if (!idsMap->isCheckedOut()) {
			idsMap = boost::dynamic_pointer_cast<IdIdSlimMap, Commitable>(idsMap->copy());
		}
		//cout << "II copy dim: " << getId() << " " << idsIndex.get() << " from: " << ((Dimension *)old.get())->idsIndex.get() << endl;
	}
	if (names && oldDim && oldDim->namesMap == namesMap) {
		if (!namesMap->isCheckedOut()) {
			namesMap = boost::dynamic_pointer_cast<NameIdSlimMap, Commitable>(namesMap->copy());
		}
	}
	if (positions && oldDim && oldDim->posIndex == posIndex) {
		posIndex = PIdVector(new IdVector(*posIndex));
		//cout << "PI copy dim: " << getId() << " " << posIndex.get() << " from: " << ((Dimension *)old.get())->posIndex.get() << endl;
	}
	return result;
}

Element* Dimension::addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal)
{
	checkCheckedOut();

	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	if (user && !isChangable() && !m_bIsRightObject) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", name);
	}

	checkElementName(name);

	Element* eByName = lookupElementByName(name, false);

	if (eByName != 0) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NAME_IN_USE, "element name is already in use", "name", name);
	}

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	// create new element and add to the list of elements
	Element element;

	if (idElement != NO_IDENTIFIER) {
		element.setIdentifier(idElement);
	}

	// make copy of elementList and all indexes
	checkOut(true, true, true);

	StringVector::StringId nameId = namesMap->pushToVector(name);
	element.setName(nameId);

	if (elementType != Element::CONSOLIDATED) {
		element.setElementType(elementType);
	} else {
		// type CONSOLIDATED is set in addChildren
		element.setElementType(Element::NUMERIC);
	}

	element.setPosition((PositionType)sizeElements());
	element.setLevel(0);
	element.setIndent(1);
	element.setDepth(0);

	// do not change: maxLevel, maxDepth
	if (maxIndent == 0) {
		maxIndent = 1;
	}

	// dimension has been changed
	setStatus(db, CHANGED);

	IdentifierType internalId = elementList->addElement(element, db, this);
	Element &newElement = (*elementList)[internalId];

	// update mapping
	(*idsMap)[newElement.getIdentifier()] = internalId;
	if (posIndex->size() <= newElement.getPosition()) {
		posIndex->resize(newElement.getPosition() + 128);
	}
	(*posIndex)[newElement.getPosition()] = internalId;
	(*namesMap)[newElement.getNameId()] = internalId;

	maxId = max(maxId, newElement.getIdentifier());
	minId = min(minId, newElement.getIdentifier());

	PJournalMem journal = db->getJournal();

	if (useJournal && journal != 0) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_CREATE);
		journal->appendInteger(getId());
		journal->appendInteger(newElement.getIdentifier());
		journal->appendEscapeString(name);
		journal->appendInteger(elementType);
		journal->nextLine();
	}

	return &newElement;
}

void Dimension::addElementEvent(PServer server, PDatabase db, Element *element)
{
	addElementEvent(server, db, element->getIdentifier());
}

void Dimension::addElementEvent(PServer server, PDatabase db, IdentifierType elemId, string sessionId)
{
	PDimensionWorker worker = server->getDimensionWorker();
	if (worker) {
		bool ok = worker->start();
		if (!ok) {
			throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start dimension worker");
		}

		long function;
		if (worker->triggerCreateElement(db->getId(), getId(), function)) {
			ResultStatus status;
			if (sessionId != "") {
				status = worker->notifyElementCreated(db->getId(), getId(), elemId, function, sessionId);
			} else {
				boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
				status = worker->notifyElementCreated(db->getId(), getId(), elemId, function, session ? session->getSid() : "");
			}
			if (status != RESULT_OK) {
				throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot send element create notification to worker");
			}
		}
	}
}

void Dimension::changeElementName(PServer server, PDatabase db, Element *element, const string &name, PUser user, bool useJournal, bool useDimWorker)
{
	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	checkElementName(name);

	if (user && !isChangable() && !(m_bIsRightObject && element->getIdentifier() >= sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0]))) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", name);
	}

	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_NAME, "element name is empty", "name", name);
	}

	if (isProtectedElement(element->getName(getElemNamesVector()))) {
		stringstream str;
		str << "element '" << element->getName(getElemNamesVector()) << "' in dimension '" << getName() << "' is not renamable";
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_RENAMABLE, str.str(), "element", name);
	}

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	// check for double used name
	Element *eByName = lookupElementByName(name, false);
	string oldName = element->getName(getElemNamesVector());

	if (eByName != 0) {
		if (eByName->getIdentifier() != element->getIdentifier()) {
			throw ParameterException(ErrorException::ERROR_ELEMENT_NAME_IN_USE, "element name is already in use", "name", name);
		}

		if (oldName == name) {
			// new name = old name
			return;
		}
	}

	// sync: make copy of elementList and index of names
	if (checkOut(false, true, false)) {
		// get element copy pointer
		element = lookupElement(element->getIdentifier(), true);
	}

	PSystemDatabase sd = server->getSystemDatabase();
	if (sd && sd->getIdentifier() == db->getIdentifier()) {
		PDimension userDim = sd->getUserDimension();
		if (userDim && userDim->getId() == this->getId()) {
			Context::getContext()->setRefreshUsers();
			PUser user = sd->getUser(oldName, NULL);
			if (user) {
				Context::getContext()->addRenamedUser(user->getId(), name);
			}
		}
	}

	// delete old name in nameToElement
	namesMap->erase(element->getNameId());

	StringVector::StringId nameId = namesMap->pushToVector(name);
	// change name
	element->setName(nameId);

	// add new name to nameToElement
	(*namesMap)[nameId] = (*idsMap)[element->getIdentifier()];

	// dimension has been changed
	setStatus(db, CHANGED);

	if (useDimWorker) {
		PDimensionWorker worker = server->getDimensionWorker();
		if (worker) {
			bool ok = worker->start();
			if (!ok) {
				throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start dimension worker");
			}

			long function;
			if (worker->triggerRenameElement(db->getId(), getId(), function)) {
				boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
				ResultStatus status = worker->notifyElementRenamed(db->getId(), getId(), oldName, name, function, session ? session->getSid() : "");
				if (status != RESULT_OK) {
					throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot send element rename notification to worker");
				}
			}
		}
	}

	PJournalMem journal = db->getJournal();

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_RENAME);
		journal->appendInteger(getId());
		journal->appendInteger(element->getIdentifier());
		journal->appendEscapeString(name);
		journal->nextLine();
	}
}

bool Dimension::isCycle(CPParents checkParents, const set<IdentifierType> &descendants)
{
	if (!checkParents || !checkParents->size() || !descendants.size()) {
		return false;
	}

	for (Parents::const_iterator it = checkParents->begin(); it != checkParents->end(); ++it) {
		IdentifierType parentId = (*it);
		if (descendants.find(parentId) != descendants.end()) {
			// cycle found, parent in descendants
			return true;
		}
	}

	return false;
}

void Dimension::addBaseElements(Element *element, const WeightedSet *ws, double factor)
{
	const WeightedSet *oldSet = element->getBaseElements();
	WeightedSet *newSet;
	if (oldSet) {
		newSet = oldSet->add(*ws, factor);
	} else {
		newSet = new WeightedSet(*ws, factor);
	}
	element->setBaseElements(newSet);

	CPParents parents = element->getParents();
	if (parents) {
		for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
			Element *parent = lookupElement(*iter, true);
			const IdentifiersWeightType *ch = parent->getChildren();
			for (IdentifiersWeightType::const_iterator chit = ch->begin(); chit != ch->end(); ++chit) {
				if (chit->first == element->getIdentifier()) {
					addBaseElements(parent, ws, factor * chit->second);
					break;
				}
			}
		}
	}
}

void Dimension::removeBaseElements(Element *element, const WeightedSet *ws, double factor)
{
	const WeightedSet *oldSet = element->getBaseElements();
	if (oldSet) {
		WeightedSet *newSet = oldSet->subtract(*ws, factor);

		element->setBaseElements(newSet);

		CPParents parents = element->getParents();
		if (parents) {
			for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
				Element *parent = lookupElement(*iter, true);
				const IdentifiersWeightType *ch = parent->getChildren();
				for (IdentifiersWeightType::const_iterator chit = ch->begin(); chit != ch->end(); ++chit) {
					if (chit->first == element->getIdentifier()) {
						removeBaseElements(parent, ws, factor);
						break;
					}
				}
			}
		}
	}
}

void Dimension::removeBaseElement(Element *ancestor, Element *base, set<IdentifierType> &removed)
{
	const WeightedSet *oldSet = ancestor->getBaseElements();  //base has to be there
	if (oldSet) {
		WeightedSet baseItself;
		baseItself.pushSorted(base->getIdentifier(), oldSet->weight(base->getIdentifier()));

		WeightedSet *newSet = oldSet->subtract(baseItself, 1.0);
		ancestor->setBaseElements(newSet);

		CPParents parents = ancestor->getParents();
		if (parents) {
			for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
				if (removed.find(*iter) == removed.end()) { //not removed yet
					Element *p = lookupElement(*iter, true);
					removeBaseElement(p, base, removed);
					removed.insert(p->getIdentifier());
				}
			}
		}
	}
}

void Dimension::updateDepthAndIndent(Element *element, Element *newParent)
{
	DepthType depth = element->getDepth();
	IndentType indent = element->getIndent();
	CPParents parents = element->getParents();

//	>MDo< element/append optimization - newParent
//	if (parents) {
//		for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
//			Element *parent = lookupElement(*iter, false);
//			if (parent->getDepth() >= depth) {
//				depth = parent->getDepth() + 1;
//			}
//			if (iter == parents->begin()) {
//				indent = parent->getIndent() + 1;
//			}
//		}
//	}

	bool updated = false;

	if (newParent->getDepth() >= depth) {
		depth = newParent->getDepth() + 1;
		updated = true;
	}
	if (*parents->begin() == newParent->getIdentifier() && indent != newParent->getIndent() + 1) {
		indent = newParent->getIndent() + 1;
		updated = true;
	}

	if (updated) {
		element->setDepth(depth);
		element->setIndent(indent);

		if (depth > maxDepth) {
			maxDepth = depth;
		}
		if (indent > maxIndent) {
			maxIndent = indent;
		}

		const IdentifiersWeightType *children = element->getChildren();
		for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
			Element *child = lookupElement(it->first, true);
			updateDepthAndIndent(child, element);
		}
	}
}

void Dimension::updateLevel(Element *element, const vector<Element *> &newChildren, bool oldIsValid)
{
	LevelType level = (oldIsValid ? element->getLevel() : 0);

//	>MDo< element/append optimization - newChildren
//	const IdentifiersWeightType *children = element->getChildren();
//	for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
//		Element *child = lookupElement(it->first, false);
//		if (child->getLevel() >= level) {
//			level = child->getLevel() + 1;
//		}
//	}

	bool updated = false;

	for (vector<Element *>::const_iterator it = newChildren.begin(); it != newChildren.end(); ++it) {
		if ((*it)->getLevel() >= level) {
			level = (*it)->getLevel() + 1;
			updated = true;
		}
	}

	if (updated) {
		element->setLevel(level);

		if (level > maxLevel) {
			maxLevel = level;
		}

		CPParents parents = element->getParents();
		if (parents) {
			vector<Element *> one;
			one.push_back(element);
			for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
				Element *parent = lookupElement(*iter, true);
				updateLevel(parent, one, oldIsValid);
			}
		}
	}
}

void Dimension::addChildren(PServer server, PDatabase db, Element *parent, const IdentifiersWeightType *children, PUser user, CubeRulesArray* disabledRules, bool preserveOrder, bool updateElementInfo, bool useJournal, IdentifiersType *elemsToDeleteFromCubes)
{
	bool changedElementsInfoOld = changedElementsInfo;
	checkCheckedOut();
	elementList->checkCheckedOut();

	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	if (children->empty()) {
		return;
	}

	// stop right here if no change required
	if (preserveOrder) {
		bool bChange = false;
		const IdentifiersWeightType *oldChildren = parent->getChildren();
		for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end() && !bChange; ++it) {
			IdentifiersWeightType::const_iterator it2 = oldChildren->begin();
			while (it2 != oldChildren->end()) {
				if (it->first == it2->first) {
					if (it->second != it2->second) {
						bChange = true; // already there, but with different weight
					}
					break;
				}
				++it2;
			}
			if (it2 == oldChildren->end()) { // not there yet
				bChange = true;
			}
		}
		if (!bChange) {
			// nothing to change
			return;
		}
	}

	bool bRightObjectPossible = false;
	if (m_bIsRightObject) {
		if (parent->getIdentifier() >= sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0])) {
			bRightObjectPossible = true;
			for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
				if (it->first < sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0])) {
					bRightObjectPossible = false;
					break;
				}
			}
		}
	}

	if (user && !isChangable() && !bRightObjectPossible) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
	}

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	// check for cycle in graph
	CPParents parentIds = parent->getParents();
	set<IdentifierType> descendantsOfChildren;
	descendants(children, descendantsOfChildren);

	if (isCycle(parentIds, descendantsOfChildren)) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_CIRCULAR_REFERENCE, "cycle detected in element hierarchy detected", "parent", parent->getName(getElemNamesVector()));
	}

	// check for double elements
	set<Element*> eSet;

	for (IdentifiersWeightType::const_iterator i = children->begin(); i != children->end(); ++i) {
		Element *element = lookupElement(i->first, false);

		eSet.insert(element);

		if (element == parent) {
			throw ParameterException(ErrorException::ERROR_ELEMENT_CIRCULAR_REFERENCE, "found parent in list of children ", "parent", parent->getName(getElemNamesVector()));
		}
	}

	if (children->size() != eSet.size()) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_EXISTS, "element was found at least twice in list of children", "set of children", (int)eSet.size());
	}

	Element::Type oldType = parent->getElementType();
	bool oldISC = parent->isStringConsolidation();

	// set type of parent
	bool baseToCons = false;
	if (oldType != Element::CONSOLIDATED) {
		changeElementType(server, db, parent, Element::CONSOLIDATED, PUser(), true, disabledRules, elemsToDeleteFromCubes, false);
		baseToCons = true;
	}

	IdentifiersWeightMap oldWeights;
	// add children (no more checks)
	if (!children->empty()) {
		addChildrenNC(server, db, user, parent, children, disabledRules, preserveOrder, &oldWeights);
	}

	if (doRemoveElement(oldType, oldISC, parent->getElementType(), parent->isStringConsolidation())) {
		// delete cell path from cube
		DeleteCellType type = getDeleteCellType(oldType, oldISC);
		if (elemsToDeleteFromCubes && type == Dimension::DEL_NUM) {
			elemsToDeleteFromCubes->push_back(parent->getIdentifier());
		} else {
			removeElementFromCubes(server, db, user, parent->getIdentifier(), disabledRules, type, false);
		}
	}

	if (!updateElementInfo || changedElementsInfoOld) { // we dont want to update just one element || there is already more changes anyway
		// elements info will be updated in merge or before (in jobs)
		changedElementsInfo = true;
	} else {
		// update element info right here
		changedElementsInfo = false;

		vector<Element *> newChildren;

		for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
			// get each child added and add its baseElements to parent and his ancestors
			Element *child = lookupElement(it->first, false);
			newChildren.push_back(child);
			const WeightedSet *ws = child->getBaseElements();

			WeightedSet childItself;
			if (!ws) {
				ws = &childItself;
				childItself.pushSorted(it->first, 1);
			}

			IdentifiersWeightMap::iterator foundIt = oldWeights.find(it->first);
			if (foundIt != oldWeights.end()) {
				// was there already
				removeBaseElements(parent, ws, foundIt->second);
			}

			addBaseElements(parent, ws, it->second);

			addToMergeIndex(parent);
			updateDepthAndIndent(child, parent);
		}

		updateLevel(parent, newChildren, preserveOrder);

		if (baseToCons) {
			CPParents parents = parent->getParents();
			if (parents) {
				//remove parent from all ancestors
				set<IdentifierType> removed;
				for (Parents::const_iterator iter = parents->begin(); iter != parents->end(); ++iter) {
					if (removed.find(*iter) == removed.end()) { // not processed yet (by recursive multiple-parents removeBaseElement)
						Element *p = lookupElement(*iter, true);
						removeBaseElement(p, parent, removed);
						removed.insert(p->getIdentifier());
					}
				}
			}
		}
	}

	PJournalMem journal = db->getJournal();

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_APPEND);
		journal->appendInteger(getId());
		journal->appendInteger(parent->getIdentifier());

		IdentifiersType identifiers(children->size());
		vector<double> weights(children->size());

		IdentifiersWeightType::const_iterator p = children->begin();
		IdentifiersType::iterator q = identifiers.begin();
		vector<double>::iterator k = weights.begin();

		for (; p != children->end(); ++p, ++q, ++k) {
			*q = (*p).first;
			*k = (*p).second;
		}

		journal->appendIdentifiers(identifiers.begin(), identifiers.end());
		journal->appendDoubles(&weights);

		journal->nextLine();
	}
}

void Dimension::addChildrenNC(PServer server, PDatabase db, PUser user, Element *parent, const IdentifiersWeightType *children, CubeRulesArray* disabledRules, bool preserveOrder, IdentifiersWeightMap *oldWeights)
{
	// the level structure will change
	changedElementsInfo = true;

	// get our children
	IdentifiersWeightType *elementChildren = parent->getChildren(true);

	// get list of old children, new children will be added
	size_t oldSize = elementChildren->size();

	// check if we have a new string child
	bool hasStringChild = false;

	if (!preserveOrder) {
		elementChildren->clear();
	}
	for (IdentifiersWeightType::const_iterator childrenIter = children->begin(); childrenIter != children->end(); ++childrenIter) {
		const IdentifierWeightType &newChild = *childrenIter;

		if (preserveOrder) {
			// check if we already know this child
			IdentifiersWeightType::iterator f = elementChildren->begin();
			IdentifiersWeightType::iterator oewEnd = f + oldSize;

			for (; f != oewEnd && ((*f).first != newChild.first); ++f) {
			}

			// if we know this child, just change the weights
			if (f != oewEnd) {

				// already is there, save the old weight for updateElementInfo
				(*oldWeights)[f->first] = f->second;

				f->second = newChild.second;
				continue;
			}
		}

		Element *child = lookupElement(newChild.first, true);
		// get type for string consolidations
		Element::Type type = child->getElementType();

		hasStringChild = hasStringChild || (type == Element::STRING) || child->isStringConsolidation();

		// add child and weight to parent to children mapping
		elementChildren->push_back(*childrenIter);

		// add parent to children to parent mapping
		PParents parentIds = child->getParents(true);

		Parents::const_iterator found = find(parentIds->begin(), parentIds->end(), parent->getIdentifier());

		if (found == parentIds->end()) {
			parentIds->push_back(parent->getIdentifier());
		}
	}

	// check and update consolidation type
	bool isString = isStringConsolidation(parent);

	bool changedToString = !isString && hasStringChild;
	bool changedToNumeric = isString && !hasStringChild && !preserveOrder;
	if (changedToString || changedToNumeric) {

		// add parent to string consolidations
		parent->setStringConsolidation(changedToString);

		// check and update parents
		const ParentsType parents = getParents(parent);

		for (ParentsType::const_iterator i = parents.begin(); i != parents.end(); ++i) {
			checkUpdateConsolidationType(server, db, user, *i, disabledRules, NULL);
		}
	}

	// dimension has been changed
	setStatus(db, CHANGED);
}

void Dimension::removeParentInChildren(Element *parent, IdentifiersWeightType *ew, set<IdentifierType> *keep)
{
	if (keep) {
		set<IdentifierType> oldChildren;
		for (IdentifiersWeightType::iterator childIter = ew->begin(); childIter != ew->end(); ++childIter) {
			oldChildren.insert(childIter->first);
		}

		bool check = true;
		set<IdentifierType>::iterator kit = keep->begin();
		for (set<IdentifierType>::iterator it = oldChildren.begin(); it != oldChildren.end();) {
			if (check) {
				if (*it < *kit) {
					removeParentInChild(parent, *it);
					++it;
				} else if (*it == *kit) {
					++it;
					++kit;
				} else {
					++kit;
				}
				if (kit == keep->end()) {
					check = false;
				}
			} else {
				removeParentInChild(parent, *it);
				++it;
			}
		}
	} else {
		for (IdentifiersWeightType::iterator childIter = ew->begin(); childIter != ew->end(); ++childIter) {
			removeParentInChild(parent, childIter->first);
		}
	}
}

void Dimension::removeParentInChild(Element *parent, IdentifierType childId)
{
	Element *child = lookupElement(childId, true);
	// erase child to parent mapping
	PParents parentIds = child->getParents(true);

	if (parentIds) {
		// find parent in vector
		Parents::const_iterator pi = find(parentIds->begin(), parentIds->end(), parent->getIdentifier());

		if (pi != parentIds->end()) {
			parentIds->erase(pi);
		}
	}
}

void Dimension::removeChildInParents(PServer server, PDatabase db, PUser user, Element* element, bool isString, CubeRulesArray* disabledRules)
{
	checkCheckedOut();

	PParents parentIds = element->getParents(true);

	// no parents found, return
	if (!parentIds || parentIds->empty()) {
		return;
	}

	// loop over all parents
	for (Parents::const_iterator parentsIter = parentIds->begin(); parentsIter != parentIds->end(); ++parentsIter) {
		Element *parent = lookupElement(*parentsIter, true);

		// get parent to children map
		IdentifiersWeightType *children = parent->getChildren(true);

		if (children) {
			for (IdentifiersWeightType::iterator child = children->begin(); child != children->end(); ++child) {
				if ((*child).first == element->getIdentifier()) {
					children->erase(child);
					break;
				}
			}

			// last child of parent deleted
			if (children->empty()) {
				// change element type to numeric
				changeElementType(server, db, parent, Element::NUMERIC, PUser(), false, disabledRules, NULL, true);
			}
		}

		// check and update StringConsolidation
		if (isString) {
			checkUpdateConsolidationType(server, db, user, parent, disabledRules, NULL);
		}
	}

	// empty list of parents
	parentIds->clear();
}

void Dimension::removeChildren(PServer server, PDatabase db, PUser user, Element *element, CubeRulesArray* disabledRules, bool useJournal, bool changingToString)
{
	elementList->checkCheckedOut();

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	// check and update consolidation type
	bool oldISC = element->isStringConsolidation();

	// find children
	IdentifiersWeightType *children = element->getChildren(true);

	// if element has no children, return
	if (!children || !children->size()) {
		if (oldISC) {
			element->setStringConsolidation(false);
		}
		return;
	}

	// the level structure will change
	changedElementsInfo = true;

	// element found in mapping "parent to children", remove element in parents
	removeParentInChildren(element, children);

	// delete mapping
	children->clear();

	// element was a "string" consolidation and is a normal consolidation now
	if (oldISC) {

		// remove element from string consolidations
		element->setStringConsolidation(false);

		// remove element from cubes (string storage)
		if (!changingToString) {
			removeElementFromCubes(server, db, user, element->getIdentifier(), disabledRules, getDeleteCellType(Element::CONSOLIDATED, oldISC), false);
		}

		// check and update parents
		const ParentsType parents = getParents(element);

		for (ParentsType::const_iterator i = parents.begin(); i != parents.end(); ++i) {
			checkUpdateConsolidationType(server, db, user, *i, disabledRules, NULL);
		}
//	} else { -> element type is not changed, nothing has to be deleted
//		// remove element from cubes (cache)
//		removeElementFromCubes(server, db, user, element->getIdentifier(), true, disabledRules);
	}

	PJournalMem journal = db->getJournal();

	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN);
		journal->appendInteger(getId());
		journal->appendInteger(element->getIdentifier());
		journal->nextLine();
	}
}

template<class T> class notInSet : public unary_function<bool, T> {
public:
	notInSet(set<T> * keep) :
		keep(keep) {
	}

	bool operator()(pair<T, double> t) const {
		return keep->find(t.first) == keep->end();
	}

private:
	set<T> * keep;
};

void Dimension::removeChildrenNotIn(PServer server, PDatabase db, PUser user, Element *element, set<IdentifierType> *keep, CubeRulesArray* disabledRules, bool useJournal)
{
	checkCheckedOut();
	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	if (keep->empty()) {
		removeChildren(server, db, user, element, disabledRules, true, false);
		return;
	}

	// find children
	IdentifiersWeightType *children = element->getChildren(true);

	// if element has no children, return
	if (!children || !children->size()) {
		return;
	}

	// the level structure will change
	changedElementsInfo = true;

	// element found in mapping "parent to children", remove element in parents
	removeParentInChildren(element, children, keep);

	if (!useJournal) {
		// remove deleted children from pcp
		IdentifiersWeightType::iterator i = remove_if(children->begin(), children->end(), notInSet<IdentifierType> (keep));
		children->erase(i, children->end());

		// check and update consolidation type
		bool oldISC = isStringConsolidation(element);
		bool newISC = oldISC;
		if (newISC) {
			newISC = hasStringChild(element);
		}

		// element was a "string" consolidation and is a normal consolidation now
		if (oldISC && !newISC) {

			// remove element from string consolidations
			element->setStringConsolidation(false);

			// remove element from cubes (string storage)
			removeElementFromCubes(server, db, user, element->getIdentifier(), disabledRules, getDeleteCellType(Element::CONSOLIDATED, oldISC), false);

			// check and update parents
			CPParents p = element->getParents();

			if (p) {
				for (Parents::const_iterator i = p->begin(); i != p->end(); ++i) {
					checkUpdateConsolidationType(server, db, user, lookupElement(*i, false), disabledRules, NULL); // writable copy will be created later if the flag really changes
				}
			}
	//	} else { -> element type is not changed, nothing has to be deleted
	//		// remove element from cubes (cache)
	//		removeElementFromCubes(server, db, user, element->getIdentifier(), true, disabledRules);
		}
	} else {
		PJournalMem journal = db->getJournal();

		if (journal != 0) {
			journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN_NOT_IN);
			journal->appendInteger(getId());
			journal->appendInteger(element->getIdentifier());

			IdentifiersType identifiers(keep->size());

			set<IdentifierType>::const_iterator p = keep->begin();
			IdentifiersType::iterator q = identifiers.begin();

			for (; p != keep->end(); ++p, ++q) {
				*q = (*p);
			}

			journal->appendIdentifiers(identifiers.begin(), identifiers.end());

			journal->nextLine();
		}
	}
}

void Dimension::changeElementType(PServer server, PDatabase db, Element *element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove)
{
	checkCheckedOut();
	elementList->checkCheckedOut();

	checkElementAccessRight(user.get(), db, RIGHT_WRITE);

	if (user && !isChangable()) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
	}

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	if (isProtectedElement(element->getName(getElemNamesVector()))) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_CHANGABLE, "element is not changable", "user", user ? (int)user->getId() : 0);
	}

	// ignore change to consolidation
	if (!setConsolidated && elementType == Element::CONSOLIDATED) {
		return;
	}

	Element::Type oldType = element->getElementType();

	// same type, nothing to do
	if (oldType == elementType) {
		return;
	}

	// sync: make copy of elementList
	if (checkOut(false, false, false)) {
		// get element copy pointer
		element = lookupElement(element->getIdentifier(), true);
	}

	// remove children
	if (oldType == Element::CONSOLIDATED) {
		// delete cell path containing element (type: string consolidation) in removeChildren
		removeChildren(server, db, user, element, disabledRules, false, (elementType == Element::STRING));
	} else {
		//if STRING is changed to NON-STRING then string values with non-string elements in all other coordinates have to be deleted
		// delete cell path containing element
		bool newISC = false;
		if (elementType == Element::CONSOLIDATED) {
			newISC = hasStringChild(element);
		}
		if (doRemove && doRemoveElement(oldType, false, elementType, newISC)) {
			DeleteCellType type = getDeleteCellType(oldType, false);
			if (type == Dimension::DEL_NUM && elemsToDeleteFromCubes) {
				elemsToDeleteFromCubes->push_back(element->getIdentifier());
			} else {
				removeElementFromCubes(server, db, user, element->getIdentifier(), disabledRules, type, false);
			}
		}
	}

	// change element type
	element->setElementType(elementType);

	// check and update parents
	CPParents parents = element->getParents();

	for (Parents::const_iterator i = parents->begin(); i != parents->end(); ++i) {
		Element *parent = lookupElement(*i, false); // writable copy will be created later if the flag really changes
		checkUpdateConsolidationType(server, db, user, parent, disabledRules, element);
	}

	// database has been changed
	setStatus(db, CHANGED);

	// elements info will be updated in merge or before (in jobs)
	changedElementsInfo = true;

	PJournalMem journal = db->getJournal();

	if (journal != 0 && elementType != Element::CONSOLIDATED) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_REPLACE);
		journal->appendInteger(getId());
		journal->appendInteger(element->getIdentifier());
		journal->appendInteger(elementType);
		journal->nextLine();
	}
}

void Dimension::_destroyElementLeavePosition(PServer server, PDatabase db, PUser user, Element *element, CubeRulesArray* disabledRules)
{
	// we might have to update the string consolidation info
	bool isString = false;

	// delete all children
	if (element->getElementType() == Element::CONSOLIDATED) {
		IdentifiersWeightType *children = element->getChildren(true);

		if (children && children->size()) {
			removeParentInChildren(element, children);

			// delete mapping
			children->clear();
		}
	} else if (element->getElementType() == Element::STRING) {
		// simple string element
		isString = true;
	}

	// remove element in parents
	removeChildInParents(server, db, user, element, isString, disabledRules);

	// remove element from name index
	namesMap->erase(element->getNameId());
}

void Dimension::deleteElement(PServer server, PDatabase db, Element *element, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker)
{
	IdentifiersType elements;
	elements.push_back(element->getIdentifier());
	deleteElements(server, db, elements, user, useJournal, disabledRules, useDimWorker);
}

void Dimension::deleteElements(PServer server, PDatabase db, IdentifiersType elementsToDelete, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker)
{
	//verify();

	checkCheckedOut();
	checkOut(true, true, true);

	checkElementAccessRight(user.get(), db, RIGHT_DELETE);

	bool bRightObjectPossible = false;
	if (m_bIsRightObject) {
		bRightObjectPossible = true;
		for (IdentifiersType::iterator it = elementsToDelete.begin(); it != elementsToDelete.end(); ++it) {
			if (*it < sizeof(SystemDatabase::ROLE) / sizeof(SystemDatabase::ROLE[0])) {
				bRightObjectPossible = false;
				break;
			}
		}
	}

	if (user && !isChangable() && !bRightObjectPossible) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
	}

	if (isLockedByCube(db)) {
		throw ErrorException(ErrorException::ERROR_DIMENSION_LOCKED, "dimension is used in a locked cube");
	}

	// the level structure might change
	changedElementsInfo = true;

	std::vector<pair<PositionType, Element*> > deletedPositions;
	vector<string> deletedNames;
	size_t listSize = elementsToDelete.size();
	deletedPositions.reserve(listSize);
	IdentifiersType elementIdentifiers;
	elementIdentifiers.reserve(listSize);

	Logger::trace << "deleting " << listSize << " elements" << endl;
	// delete cell path containing element
	for (IdentifiersType::iterator it = elementsToDelete.begin(); it != elementsToDelete.end(); ++it) {
		Element *element = lookupElement(*it, true);
		if (!element) {
			continue; // invalid element
		}
		string name = element->getName(getElemNamesVector());
		deletedNames.push_back(name);
		if (isProtectedElement(name)) {
			stringstream str;
			str << "element '" << element->getName(getElemNamesVector()) << "' cannot be deleted from " << this->getName().c_str();
			throw ErrorException(ErrorException::ERROR_ELEMENT_NOT_DELETABLE, str.str());
		}
		PositionType positionDeletedElement = element->getPosition();
		deletedPositions.push_back(make_pair(positionDeletedElement, element));
		elementIdentifiers.push_back(element->getIdentifier());
	}
	if (!deletedPositions.empty()) {
		sort(deletedPositions.begin(), deletedPositions.end());
		Logger::trace << "element positions sorted" << endl;

		removeElementsFromCubes(server, db, user, elementsToDelete, disabledRules, DEL_ALL, true);

		Logger::trace << "elements deleted from cubes" << endl;

		int elementCount = (int)sizeElements();

//		if (checkOut(true, true, true)) {
		for (size_t i = 0; i < deletedPositions.size(); i++) {
			// get pointers to element copies
			deletedPositions[i].second = lookupElement(deletedPositions[i].second->getIdentifier(), true);
		}
//		}

		// update elements with positions > position of deleted element
		for (size_t i = 0; i < deletedPositions.size(); i++) {
			int pos = deletedPositions[i].first;
			int nextPos = i + 1 < deletedPositions.size() ? deletedPositions[i + 1].first : elementCount;
			Element *element = deletedPositions[i].second;

			//cout << "moving dim: " << getId() << " positions: " << pos+1 << " - " << nextPos << endl;
			//update positions
			for (int p = pos + 1; p < nextPos; p++) {
				Element *pEl = lookupElementByPosition(p, true);
				if (pEl) {
					pEl->setPosition((PositionType)(p - i - 1));
					(*posIndex)[p - i - 1] = (*idsMap)[pEl->getIdentifier()];
				}
			}
			//delete element
			_destroyElementLeavePosition(server, db, user, element, disabledRules);
		}

		Logger::trace << "elements deleted" << endl;

		for (size_t e = 0; e < elementIdentifiers.size(); e++) {
			elementList->deleteElement((*idsMap)[elementIdentifiers[e]]);
			// remove element from ids index
			idsMap->erase(elementIdentifiers[e]);
		}

		Logger::trace << "elements pointers updated" << endl;
	}
	Logger::trace << listSize << " elements deleted" << endl;

	// the dimension has been changed
	setStatus(db, CHANGED);

	// elements info will be updated in merge or before (in jobs)
	changedElementsInfo = true;

	if (useDimWorker) {
		updateElementsInfo();
		deleteElementsEvent(server, db, deletedNames);
	}

	PJournalMem journal = db->getJournal();
	if (journal != 0 && useJournal) {
		journal->appendCommand(server->getUsername(user), server->getEvent(), JournalFileReader::JOURNAL_ELEMENT_DESTROY);
		journal->appendInteger(getId());
		journal->appendIdentifiers(elementIdentifiers.begin(), elementIdentifiers.end());
		journal->nextLine();
	}

	//verify();
}

void Dimension::deleteElementsEvent(PServer server, PDatabase db, const vector<string> &deletedNames)
{
	PDimensionWorker worker = server->getDimensionWorker();
	if (worker) {
		bool ok = worker->start();
		if (!ok) {
			throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot start dimension worker");
		}

		long function;
		if (worker->triggerDestroyElement(db->getId(), getId(), function)) {
			boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
			for (vector<string>::const_iterator it = deletedNames.begin(); it != deletedNames.end(); ++it) {
				ResultStatus status = worker->notifyElementDestroyed(db->getId(), getId(), *it, function, session ? session->getSid() : "");
				if (status != RESULT_OK) {
					throw ErrorException(ErrorException::ERROR_WORKER_MESSAGE, "cannot send element destroy notification to worker");
				}
			}
		}
	}
}

set<Element*> Dimension::getBaseElements(Element *parent, bool *multiple) const
{
	set<Element*> baseElements;

	if (multiple != 0) {
		*multiple = false;
	}

	if (parent && parent->getElementType() == Element::CONSOLIDATED) {
		const IdentifiersWeightType *children = parent->getChildren();

		if (children) {
			for (IdentifiersWeightType::const_iterator iter = children->begin(); iter != children->end(); ++iter) {
				bool subMultiple;
				set<Element*> subBase = getBaseElements(lookupElement(iter->first, false), &subMultiple);

				if (multiple != 0) {
					*multiple |= subMultiple;
				}
				baseElements.insert(subBase.begin(), subBase.end());
			}
		}
	} else if (parent) {
		CPParents grandParents = parent->getParents();

		if (grandParents && grandParents->size() > 1 && multiple != 0) {
			*multiple = true;
		}
		baseElements.insert(parent);
	}

	return baseElements;
}

void Dimension::checkUpdateConsolidationType(PServer server, PDatabase db, PUser user, Element *element, CubeRulesArray* disabledRules, Element *oneNewChild)
{
	//locked by the caller
	bool oldISC = element->isStringConsolidation();
	bool newISC = false;

	if (oneNewChild) {
		// only one new child was added/changed
		if (oldISC) {
			// it was string-cons, maybe oneNewChild changed from string to numeric and it was the last string of all children
			if (oneNewChild->getElementType() == Element::STRING || (oneNewChild->getElementType() == Element::CONSOLIDATED && isStringConsolidation(oneNewChild))) {
				// no change to non-SC possible
				return;
			} else {
				// possible change to non-SC
				newISC = hasStringChild(element);
			}
		} else { // oldISC == false
			if (oneNewChild->getElementType() == Element::STRING || (oneNewChild->getElementType() == Element::CONSOLIDATED && isStringConsolidation(oneNewChild))) {
				newISC = true;
			} else {
				// no change to SC
				return;
			}
		}
	} else {
		// multiple changes, check all
		newISC = hasStringChild(element);
	}

	// type has changed!
	if (oldISC != newISC) {
		element = lookupElement(element->getIdentifier(), true);

		if (newISC) {
			element->setStringConsolidation(true);
		} else {
			element->setStringConsolidation(false);

			// delete cell path containing element in string storage
			removeElementFromCubes(server, db, user, element->getIdentifier(), disabledRules, getDeleteCellType(Element::CONSOLIDATED, oldISC), false);
		}

		// check and update parents
		CPParents pids = element->getParents();

		if (pids) {
			for (Parents::const_iterator i = pids->begin(); i != pids->end(); ++i) {
				Element *parent = lookupElement(*i, false); // writable copy will be created later if the flag really changes
				checkUpdateConsolidationType(server, db, user, parent, disabledRules, NULL);
			}
		}
	}
}

void Dimension::updateTopologicalSortedElements(IdentifiersType &sortedElements, BitVector &knownElements)
{
	// add parents first!
	size_t totalElements = elementList->size();

	for (IdentifierType internalId = 0; internalId < totalElements; internalId++) {
		Element &element = (*elementList)[internalId];

		if (element.getElementType() == Element::UNDEFINED) {
			continue;
		}
		if (!knownElements[internalId]) {
			// add parent
			addParrentsToSortedList(&element, sortedElements, knownElements);
			// add element
			sortedElements.push_back(internalId);
			knownElements[internalId] = true;
		}
	}
}

void Dimension::addParrentsToSortedList(Element *child, IdentifiersType &sortedElements, BitVector &knownElements)
{
	CPParents parents = child->getParents();

	if (!parents || parents->empty()) {
		return;
	}

	for (Parents::const_iterator pti = parents->begin(); pti != parents->end(); ++pti) {
		Element *parent = lookupElement(*pti, false);

		if (!parent) {
			throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with id '" + StringUtils::convertToString(*pti) + "' not found in dimension '" + getName() + "'", "id", *pti);
		}

		IdentifierType internalId = (*idsMap)[parent->getIdentifier()];

		if (!knownElements[internalId]) {
			// add parent
			addParrentsToSortedList(parent, sortedElements, knownElements);
			// add element
			sortedElements.push_back(internalId);
			knownElements[internalId] = true;
		}
	}
}

void Dimension::updateElementBaseElements(IdentifiersType &sortedElements)
{
	// as we are rebuilding merge info in the following loop let's clear the merge index first
	clearMergeIndex();

	for (IdentifiersType::reverse_iterator i = sortedElements.rbegin(); i != sortedElements.rend(); ++i) {
		Element *element = &(*elementList)[*i];

		if (element->getElementType() == Element::UNDEFINED || element->getElementType() == Element::STRING) {
			continue;
		}

		element = lookupElement(element->getIdentifier(), true);

		if (element->getBaseElements() && element->getBaseElements()->size() > 0) {
			element->baseElementsClear();
		}

		const IdentifiersWeightType *childrenIds = element->getChildren();
		if (childrenIds && childrenIds->size()) {

			WeightedSet *baseElements = element->getBaseElements(true);

			for (IdentifiersWeightType::const_iterator i = childrenIds->begin(); i != childrenIds->end(); ++i) {
				Element *child = lookupElement(i->first, false);
				const WeightedSet *basesOfChild = child->getBaseElements();
				double weight = i->second;

				if (!basesOfChild || basesOfChild->size() == 0) {
					baseElements->fastAdd(child->getIdentifier(), weight);
				} else {
					for (WeightedSet::const_iterator c = basesOfChild->begin(); c != basesOfChild->end(); ++c) {
						baseElements->fastAdd(c.first(), weight * c.second());
					}
				}
			}

			baseElements->consolidate();
		}

		addToMergeIndex(element);
	}
}

void Dimension::updateElementsInfo()
{
	if (!changedElementsInfo) {
		return;
	}

	checkCheckedOut();
	elementList->checkCheckedOut();

	// clear all info
	maxLevel = 0;
	maxIndent = 0;
	maxDepth = 0;

	changedElementsInfo = false;

	IdentifiersType sortedElements;
	BitVector knownElements;

	knownElements.resize(elementList->size());
	sortedElements.reserve(elementList->size());

	// update the topological sorted list of elements
	updateTopologicalSortedElements(sortedElements, knownElements);

	// Need to make sure base elements are correctly specified
	updateElementBaseElements(sortedElements);

	// update level
	updateLevel(sortedElements);

	// update depth and indent
	for (IdentifiersType::const_iterator i = sortedElements.begin(); i != sortedElements.end(); ++i) {
		Element *element = &(*elementList)[*i];

		DepthType depth = 0;
		IndentType indent = 1;

		CPParents parents = element->getParents();
		if (parents) {
			// depth
			for (Parents::const_iterator i = parents->begin(); i != parents->end(); ++i) {
				Element *parent = lookupElement(*i, false);

				if (indent == 1) {
					// first parent
					indent = parent->indent + 1;
				}
				DepthType d = parent->depth; // firstParent->getDepth(this);
				if (depth <= d) {
					depth = d + 1;
				}
			}
		}

		if (maxDepth < depth) {
			maxDepth = depth;
		}

		if (maxIndent < indent) {
			maxIndent = indent;
		}

		element->setDepth(depth);
		element->setIndent(indent);
	}
}

void Dimension::updateLevel(IdentifiersType &sortedElements)
{
	//called from Dimension.merge
	for (IdentifiersType::reverse_iterator i = sortedElements.rbegin(); i != sortedElements.rend(); ++i) {
		Element *element = &(*elementList)[*i];

		LevelType level = 0;

		const IdentifiersWeightType *children = element->getChildren();
		if (children) {
			for (IdentifiersWeightType::const_iterator childPair = children->begin(); childPair != children->end(); ++childPair) {
				Element *child = lookupElement(childPair->first, false);
				LevelType l = child->level; // child->getLevel(this);
				if (level <= l) {
					level = l + 1;
				}
			}
		}

		if (maxLevel < level) {
			maxLevel = level;
		}

		element->setLevel(level);
	}
}

void Dimension::removeElementFromCubes(PServer server, PDatabase db, PUser user, IdentifierType element, CubeRulesArray* disabledRules, DeleteCellType delType, bool completeRemove)
{
	db->checkCheckedOut(); // database can be modified

	PDimension thisDimension = COMMITABLE_CAST(Dimension, shared_from_this());
	vector<CPCube> cubes = getCubes(PUser(), db); // get cubes using this dimension
	PCubeList cubeList;
	Context* context = Context::getContext();

	for (vector<CPCube>::iterator c = cubes.begin(); c != cubes.end(); ++c) {
		if (cubeList == 0) {
			cubeList = db->getCubeList(true);
			db->setCubeList(cubeList);
		}

		PCube cube = COMMITABLE_CAST(Cube, cubeList->get((*c)->getId(), true));
		context->saveParent(db, cube);
		if (cube->deleteElement(server, db, user, server->getEvent(), thisDimension, element, disabledRules, delType, completeRemove)) {
			cubeList->set(cube);
		}
	}
}

void Dimension::removeElementsFromCubes(PServer server, PDatabase db, PUser user, IdentifiersType elementIds, CubeRulesArray* disabledRules, DeleteCellType type, bool completeRemove)
{
	PDimension thisDimension = COMMITABLE_CAST(Dimension, shared_from_this());
	PCubeList cl = db->getCubeList(true);
	db->setCubeList(cl);
	Context* context = Context::getContext();
	PSet fullSet(new Set(true));

	for (CubeList::Iterator c = cl->begin(); c != cl->end(); ++c) {
		PCube cube = COMMITABLE_CAST(Cube, cl->get((*c)->getId(), true));
		context->saveParent(db, cube);
		if (cube->deleteElements(server, db, user, server->getEvent(), thisDimension, elementIds, disabledRules, type, fullSet, completeRemove)) {
			cl->set(cube);
		}
	}
}

void Dimension::checkElementName(const string& name)
{
	if (name.empty()) {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_NAME, "element name is empty", "name", name);
	}

	if (name[0] == ' ' || name[name.length() - 1] == ' ') {
		throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_NAME, "element name begins or ends with a space character", "name", name);
	}

	for (size_t i = 0; i < name.length(); i++) {
		if (0 <= name[i] && name[i] < 32) {
			throw ParameterException(ErrorException::ERROR_INVALID_ELEMENT_NAME, "element name contains an illegal character", "name", name);
		}
	}
}


void Dimension::addToMergeIndex(Element *element)
{
	bool showDebug = false;

	if (showDebug) {
		Logger::debug << "Adding Element " << element->getIdentifier() << " to merge index..." << endl;
	}

	// now merge base ranges if possible
	WeightedSet *ranges = element->getBaseElements(false);

	// only continue if there is more than one range
	if (ranges && ranges->rangesCount() > 1 && element->getElementType() == Element::CONSOLIDATED) {
		ranges = element->getBaseElements(true); // checkOut (create new Relations only here if really needed)
		WeightedSet::range_iterator last = ranges->rangeBegin();
		WeightedSet::range_iterator j = ranges->rangeBegin();
		++j;
		for (; j != ranges->rangeEnd();  ++j, ++last) {
			bool can_merge = true;

			for (IdentifierType e = last.high() + 1; e < j.low(); e++) {
				const Element *tstElem = lookupElement(e, false);

				if (tstElem && tstElem->getElementType() == Element::NUMERIC) {
					can_merge = false;
					break;
				}
			}

			if (can_merge) {
				// store all ("mergeable") holes that don't contain base elements
					if (showDebug) {
						Logger::debug << "Can merge ranges." << endl;
					}

					mergeNext[last.high()] = j.low();
			} else {
				if (showDebug) {
					Logger::debug << "Cannot merge ranges (base values exist in gaps)." << endl;
				}
			}
		}
	}

	if (showDebug) {
		Logger::debug << "Update done." << endl;
		}
	}


const ParentsType Dimension::getParents(Element * child) const
{
	CPParents parentIds = child->getParents();
	ParentsType parents;
	parents.reserve(parentIds->size());
	for (Parents::const_iterator it = parentIds->begin(); it != parentIds->end(); ++it) {
		parents.push_back(lookupElement(*it, false));
	}
	return parents;
}

ParentsType Dimension::getParents(PUser user, Element *child) const
{
	ElementsType parents;
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
	if (hasRightsCube() && User::checkUser(user) && db->getHideElements()) {
		CPDimension dim = CONST_COMMITABLE_CAST(Dimension, shared_from_this());
		CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
		ElementsType allParents = Dimension::getParents(PUser(), child);
		for (ElementsType::const_iterator it = allParents.begin(); it != allParents.end(); ++it) {
			if (user->checkElementRight(db->getId(), getId(), (*it)->getIdentifier(), RIGHT_READ)) {
				parents.push_back(*it);
			}
		}
	} else {
		parents = getParents(child);
	}
	return parents;
}

// return children of parent or root elements if parent == NULL
const ElementsWeightType Dimension::getChildren(PUser user, Element *parent) const
{
	ElementsWeightType children;
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
	if (hasRightsCube() && User::checkUser(user) && db->getHideElements()) {
		CPDimension dim = CONST_COMMITABLE_CAST(Dimension, shared_from_this());
		CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
		if (parent) {
			ElementsWeightType allChildren = Dimension::getChildren(PUser(), parent);
			// TODO filter hidden children

			for (ElementsWeightType::const_iterator it = allChildren.begin(); it != allChildren.end(); ++it) {
				if (user->checkElementRight(db->getId(), getId(), it->first->getIdentifier(), RIGHT_READ)) {
					children.push_back(*it);
				}
			}
		} else {
			// only roots or visible children of invisible elements
			ElementsType allVisible = getElements(user, true);

			// iterate over visible elements and return only those visible
			for (ElementsType::const_iterator it = allVisible.begin(); it != allVisible.end(); ++it) {
				bool addElem = false;
				if (!(*it)->getParentsCount()) {
					addElem = true;
				} else {
					CPParents parentIds = (*it)->getParents();
					addElem = true;

#ifdef NEW_RIGHTS_CALC
					for (ElementsType::const_iterator testParent = allVisible.begin(); testParent != allVisible.end(); ++testParent) {
						IdentifierType otherId = (*testParent)->getIdentifier();
						if (find(parentIds->begin(), parentIds->end(), otherId) != parentIds->end()) {
							// found
							addElem = false;
							break;
						}
					}
#else
					for (Parents::const_iterator parentit = parentIds->begin(); parentit != parentIds->end(); ++parentit) {
						// if the parent is visible - do not add child to result
						if (user->checkGroupDimensionDataRight(db, groupDimensionDataCube, dim, *parentit, RIGHT_READ)) {
							addElem = false;
							break;
						}
					}
#endif
				}
				if (addElem) {
					children.push_back(ElementWeightType(*it, 1));
				}
			}
		}
	} else {
		// no hiding
		if (parent) {
			const IdentifiersWeightType *childrenIds = parent->getChildren();
			children.reserve(childrenIds->size());
			for (IdentifiersWeightType::const_iterator it = childrenIds->begin(); it != childrenIds->end(); ++it) {
				children.push_back(ElementWeightType(lookupElement(it->first, false), it->second));
			}
		} else {
			ElementsType elems = getElements(PUser(), true);
			for (ElementsType::const_iterator it = elems.begin(); it != elems.end(); ++it) {
				children.push_back(ElementWeightType(*it, 1)); // weight not needed for roots
			}
		}
	}
	return children;
}

Element* Dimension::lookupElement(IdentifierType elementIdentifier, bool write) const
{
	if (write || elementVector.empty() || elementIdentifier < minId || elementIdentifier > maxId) {
		IdIdSlimMap::const_iterator it = idsMap->find(elementIdentifier);
		if (it != idsMap->end()) {
			if (write) {
				const_cast<Dimension *>(this)->checkOut(true, false, false);
				// check the page is writable
				elementList->checkOut(it.second());
			}
			return &(*elementList)[it.second()];
		} else {
			return 0;
		}
	} else {
		return elementVector[elementIdentifier-minId];
	}
}

Element* Dimension::lookupElementByName(const string& name, bool write) const
{
	NameIdSlimMap::const_iterator it = namesMap->find(name);
	if (it != namesMap->end()) {
		if (write) {
			const_cast<Dimension *>(this)->checkOut(true, false, false);
			// check the page is writable
			elementList->checkOut(it.second());
		}
		return &(*elementList)[it.second()];
	} else {
		return 0;
	}
}

Element* Dimension::lookupElementByInternal(IdentifierType elementInternalId, bool write) const
{
	if (write) {
		const_cast<Dimension *>(this)->checkOut(true, false, false);
		// check the page is writable
		elementList->checkOut(elementInternalId);
	}
	return &(*elementList)[elementInternalId];
}

Element* Dimension::lookupElementByPosition(PositionType position, bool write) const
{
	Element *element = 0;
	if (position < posIndex->size()) {
		IdentifierType internalId = (*posIndex)[position];
		if (internalId != NO_IDENTIFIER) {
			if (write) {
				const_cast<Dimension *>(this)->checkOut(true, false, false);
				// check the page is writable
				elementList->checkOut(internalId);
			}
			return &(*elementList)[internalId];
		}
	}
	return element;
}

Element* Dimension::findElement(IdentifierType elementIdentifier, User *user, bool write) const
{
	if (User::checkUser(user)) {
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		checkElementAccessRight(user, db, RIGHT_READ);
	}

	Element *element = lookupElement(elementIdentifier, write);
	if (element == 0) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with identifier " + StringUtils::convertToString((uint32_t)elementIdentifier) + " not found in dimension '" + this->getName() + "'", "elementIdentifier", (int)elementIdentifier);
	}
	checkHideElement(element, user);
	return element;
}

Element* Dimension::findElementByName(const string &name, User *user, bool write) const
{
	if (User::checkUser(user)) {
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(shared_from_this()));
		checkElementAccessRight(user, db, RIGHT_READ);
	}

	Element *element = lookupElementByName(name, write);
	if (element == 0) {
		throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element with name '" + name + "' not found in dimension '" + this->getName() + "'", "name", name);
	}
	checkHideElement(element, user);
	return element;
}

Element* Dimension::findElementByPosition(PositionType position, User *user, CPDatabase db, bool write) const
{
	checkElementAccessRight(user, db, RIGHT_READ);

	Element *element = lookupElementByPosition(position, write);
	if (element == 0) {
		throw ParameterException(ErrorException::ERROR_INVALID_POSITION, "element with position " + StringUtils::convertToString((uint32_t)position) + " not found", "position", (int)position);
	}
	checkHideElement(element, user);
	return element;
}

void Dimension::checkHideElement(Element *element, User *user) const
{
	if (hasRightsCube() && User::checkUser(user)) {
		CPDimension dim = CONST_COMMITABLE_CAST(Dimension, shared_from_this());
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(dim));
		if (db->getHideElements()) {
			CPCube groupDimensionDataCube = db->findCubeByName(SystemCube::PREFIX_GROUP_DIMENSION_DATA + dim->getName(), PUser(), true, false);
			if (!(user->checkElementRight(db->getId(), getId(), element->getIdentifier(), RIGHT_READ))) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights for element", "user", (int)user->getId());
			}
		}
	}
}

bool Dimension::verify()
{
	bool ret = true;

	if (idsMap->size() != namesMap->size()) {
		cout << "idsMap->size() != namesMap->size() dimension: " << getId() << endl;
		ret = false;
	}
	for (IdentifierType pos = 0; pos < sizeElements(); pos++) {
		Element &element = (*elementList)[(*posIndex)[pos]];
		if (element.getElementType() == Element::UNDEFINED) {
			cout << "invalid elem with position dim: " << getId() << " int id: " << (*posIndex)[pos] << " pos: " << element.getPosition() << endl;
			ret = false;
			break;
		}
	}

	IdentifierType internalId = 0;
	size_t tstcount = 0;
	for (ElementList::const_iterator i = elementList->begin(); i != elementList->end(); ++i, internalId++) {
		const Element &element = *i;
		if (element.getElementType() == Element::UNDEFINED) {
			continue;
		}
		tstcount++;
		// check if position is ok
		if ((*posIndex)[element.getPosition()] != internalId) {
			// wrong position;
			cout << "invalid position dim: " << getId() << " elem: '" << element.getName(getElemNamesVector()) << "' id: " << element.getIdentifier() << "/" << internalId  << " pos: " << element.getPosition() << endl;
			ret = false;
			break;
		}
		// check if name is ok
		if (namesMap->find(element.getName(getElemNamesVector())) == namesMap->end() || namesMap->find(element.getName(getElemNamesVector())).second() != internalId) {
			// wrong name;
			cout << "invalid name dim: " << getId() << " elem: '" << element.getName(getElemNamesVector()) << "' id: " << element.getIdentifier() << "/" << internalId  << " pos: " << element.getPosition() << endl;
			ret = false;
			break;
		}
		// check if id is ok
		if ((*idsMap)[element.getIdentifier()] != internalId) {
			// wrong name;
			cout << "invalid id dim: " << getId() << " elem: '" << element.getName(getElemNamesVector()) << "' id: " << element.getIdentifier() << "/" << internalId  << " pos: " << element.getPosition() << endl;
			ret = false;
			break;
		}
	}
	if (idsMap->size() != tstcount) {
		cout << "idsMap->size() != tstcount: " << getId() << " - " << idsMap->size() << " != " << tstcount << endl;
		ret = false;
	}
	return ret;
}

bool Dimension::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);
	CPDimension dim = CONST_COMMITABLE_CAST(Dimension, o);
	CPDimension olddim = CONST_COMMITABLE_CAST(Dimension, old);
	if (o != 0) {
		token = dim->token + 1;
		if (!token) {
			++token;
		}
	}
	if (old != 0) {
		if (dim != 0) {
			if (status == olddim->status) {
				status = dim->status;
			}
			if (deletable == olddim->deletable) {
				deletable = dim->deletable;
			}
			if (renamable == olddim->renamable) {
				renamable = dim->renamable;
			}
			if (changable == olddim->changable) {
				changable = dim->changable;
			}
		}
	}

	if (ret) {
		// return false if any conflict in:
		//   elementList - relations, element types, names
		//   idsSlimMap - new,delete element
		//   nameIndex - rename
		//   posIndex - element move
		if (dim && olddim) {
			//cout << "EL merge compare. dim: " << getId() << " ver: " << elementList->getVersion() << " from: " << elementList->old->getVersion() << endl;
			if (elementList != olddim->elementList && dim->elementList != olddim->elementList) {
				ret = false;
			} else if (idsMap != olddim->idsMap && dim->idsMap != olddim->idsMap) {
				ret = false;
			} else if (namesMap != olddim->namesMap && dim->namesMap != olddim->namesMap) {
				ret = false;
			} else if (posIndex != olddim->posIndex && dim->posIndex != olddim->posIndex) {
				ret = false;
			}
		}
	}
	if (ret) {
		mergeUpdateVector();
//		if (elementList->old) {
//			cout << "EL merge dim: " << getId() << " ptr: " << elementList->getVersion() << " from: " << elementList->old->getVersion() << endl;
//		}

		if (elementList->isCheckedOut()) {
			ret = elementList->merge(dim ? dim->elementList : PElementList(), shared_from_this());
		} else  if (dim) {
			elementList = dim->elementList;
		}
	}

	if (ret && idsMap->isCheckedOut()) {
		ret = idsMap->merge(dim ? dim->idsMap : PIdIdSlimMap(), shared_from_this());
	}

	if (ret && namesMap->isCheckedOut()) {
		ret = namesMap->merge(dim ? dim->namesMap : PNameIdSlimMap(), shared_from_this());
	}

	if (ret && dim && olddim) {
		// get changes made by others
		if (maxId == olddim->maxId) {
			maxId = dim->maxId;
		}
		if (minId == olddim->minId) {
			minId = dim->minId;
		}
		if (maxLevel == olddim->maxLevel) {
			maxLevel = dim->maxLevel;
		}
		if (maxIndent == olddim->maxIndent) {
			maxIndent = dim->maxIndent;
		}
		if (maxDepth == olddim->maxDepth) {
			maxDepth = dim->maxDepth;
		}
		if (posIndex == olddim->posIndex) {
			//cout << "PI others changes accepted" << endl;
			posIndex = dim->posIndex;
		}
	}
	if (ret) {
		// verify the dimension integrity
		// TODO -jj- can be removed after stabilization or reduced to _DEBUG only
		//ret = verify();
		if (ret) {
			commitintern();
			//cout << "dim merge ok dim: " << getId() << endl;
		}
	}
	if (!ret) {
		//cout << "dim merge failed dim: " << getId() << endl;
	}
	return ret;
}

void Dimension::mergeUpdateVector()
{
	if (elementList->isCheckedOut()) {
		// update elementVector for small dimensions
		size_t totalElements = elementList->size();
		bool buildElemVector = false;
		elementVector.clear();
		if (maxId - minId < MAX_ELEMS_IN_VECTOR) {
			elementVector.resize(maxId - minId + 1);
			buildElemVector = true;
		}

		for (IdentifierType internalId = 0; buildElemVector && internalId < totalElements; internalId++) {
			Element &element = (*elementList)[internalId];

			if (element.getElementType() == Element::UNDEFINED) {
				continue;
			}
			if (element.getIdentifier() >= minId && element.getIdentifier() <= maxId) {
				elementVector[element.getIdentifier()-minId] = &element;
			} else {
				Logger::error << "Element Id out of limits. Dimension: '"<< getName() << "' Element: '" << element.getName(getElemNamesVector()) << "' Id: " << element.getIdentifier() << " MaxId: " << maxId << " MinId: " << minId << endl;
				buildElemVector = false;
				elementVector.clear();
				break;
			}
		}
	}

	updateElementsInfo();
}

//Dimension::DeleteCellType Dimension::getDeleteCellType(Element::Type oldType, bool oldIsStringCons, Element::Type newType, bool newIsStringCons)
//{
//	DeleteCellType result = DEL_NONE;
//
//	if (oldType == Element::NUMERIC) {
//		if (newType != Element::NUMERIC) {
//			result = DEL_OLD_NUM;
//		}
//	} else {
//		bool oldStr = oldType == Element::STRING || (oldType == Element::CONSOLIDATED && oldIsStringCons);
//		bool newNum = newType == Element::NUMERIC || (newType == Element::CONSOLIDATED && !newIsStringCons);
//		if (oldStr && newNum) {
//			result = DEL_OLD_STR_NEW_NUM;
//		}
//	}
//	return result;
//}

bool Dimension::hasStringChild(Element* element) const
{
	bool result = false;
	const IdentifiersWeightType *children = element->getChildren();
	if (children && children->size()) {
		for (IdentifiersWeightType::const_iterator it = children->begin(); it != children->end(); ++it) {
			Element* child = lookupElement(it->first, false);

			switch (child->getElementType()) {
			case Element::STRING:
				result = true;
				break;

			case Element::CONSOLIDATED:
				if (isStringConsolidation(child)) {
					result = true;
				}
				break;

			default:
				// ignore type
				break;
			}
		}
	}
	return result;
}

bool Dimension::doRemoveElement(Element::Type oldType, bool oldIsStringCons, Element::Type newType, bool newIsStringCons)
{
	bool result = true;

	if (oldType == Element::NUMERIC) {
		if (newType == Element::NUMERIC) {
			result = false; // both are NUMERIC
		}
	} else if (oldType == Element::CONSOLIDATED && !oldIsStringCons) {
		result = false; // old is a normal consolidation
	} else if (newType == Element::STRING ||(newType == Element::CONSOLIDATED && newIsStringCons)) {
		result = false; // both are string or "string" consolidation
	}
	return result;
}

bool Dimension::isStringElement(Element::Type type, bool isStringCons)
{
	return type == Element::STRING || (type == Element::CONSOLIDATED && isStringCons);
}

Dimension::DeleteCellType Dimension::getDeleteCellType(Element::Type type, bool isStringCons)
{
	return isStringElement(type, isStringCons) ? DEL_STR : DEL_NUM;
}

}
