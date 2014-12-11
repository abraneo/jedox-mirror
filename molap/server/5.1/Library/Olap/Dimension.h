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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_DIMENSION_H
#define OLAP_DIMENSION_H 1

#include "palo.h"

#include <set>
#include <map>

#include "Exceptions/ParameterException.h"

#include "Olap/Element.h"
#include "Olap/User.h"
#include "Olap/CommitableList.h"

#include "Collections/StringVector.h"
#include "Collections/StringUtils.h"
#include "Collections/SlimMap.h"
#include "Collections/NameIdSlimMap.h"

namespace palo {
class FileReader;
class FileWriter;
class Cube;
class PaloSession;
class ElementOld2NewMap;

class SERVER_CLASS DimensionList : public CommitableList {
public:
	DimensionList(const PIdHolder &newidh) : CommitableList(newidh) {}
	DimensionList() {}
	DimensionList(const DimensionList &l);
	virtual PCommitableList createnew(const CommitableList& l) const;
};

typedef vector<pair<PCube, vector<PRule> > > CubeRulesArray;
typedef map<IdentifiersType, PParents> IdsToParentsMap;

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract super class for OLAP dimension
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Dimension : public Commitable {
private:
	typedef SlimMap<IdentifierType, IdentifierType> IdIdSlimMap;
	typedef boost::shared_ptr<SlimMap<IdentifierType, IdentifierType> > PIdIdSlimMap;

public:
	enum SaveType {
		NORMAL = 1, RIGHTS = 2, ALIAS = 3, ATTRIBUTES = 4, CUBE = 5, CONFIG = 6, DIMENSION = 7, SUBSETVIEW = 8, USERINFO = 9, CELLPROPS = 10, VIRTUAL = 11
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief status of the dimension
	///
	/// LOADED:   the dimension is loaded and not changed<br>
	/// CHANGED:  the dimension is new or changed
	////////////////////////////////////////////////////////////////////////////////

	enum DimensionStatus {
		LOADED, CHANGED
	};

	enum DeleteCellType {
		DEL_NUM = 1, DEL_STR = 2, DEL_ALL = 3
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief types
	////////////////////////////////////////////////////////////////////////////////

	typedef IdentifiersType IdVector;
	typedef boost::shared_ptr<IdVector> PIdVector;
	typedef boost::shared_ptr<const IdVector> CPIdVector;

	static PDimension loadDimensionFromType(PServer server, FileReader*, const string& name, Dimension::SaveType type);
	static bool isStringElement(Element::Type type, bool isStringCons);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new dimension with given identifier
	////////////////////////////////////////////////////////////////////////////////

	Dimension(const string& name, SaveType saveType) :
		Commitable(name), token(rand()), status(CHANGED), deletable(true), renamable(true), changable(true), saveType(saveType),
		elementList(PElementList(new ElementList())), idsMap(PIdIdSlimMap(new IdIdSlimMap(4096))), namesMap(PNameIdSlimMap(new NameIdSlimMap(4096))),
		posIndex(PIdVector(new IdVector())), maxId(0), minId(0), maxLevel(0), maxIndent(0), maxDepth(0), changedElementsInfo(false), m_bIsRightObject(false),
		attribCube(false), rightCube(false)
	{
		if (!token) {
			++token;
		}
		switch (saveType) {
			case NORMAL:
			case CUBE:
			case DIMENSION:
			case SUBSETVIEW:
			case USERINFO:
				attribCube = true;
			default:;
		}
		switch (saveType) {
			case NORMAL:
			case ATTRIBUTES:
			case CELLPROPS:
				rightCube = true;
			default:;
		}
	}

	Dimension(const Dimension &dim) :
		Commitable(dim), token(dim.token), status(dim.status), deletable(dim.deletable), renamable(dim.renamable), changable(dim.changable), protectedElems(dim.protectedElems), saveType(dim.saveType),
		elementList(dim.elementList), idsMap(dim.idsMap), namesMap(dim.namesMap), posIndex(dim.posIndex), maxId(dim.maxId), minId(dim.minId), maxLevel(dim.maxLevel),
		maxIndent(dim.maxIndent), maxDepth(dim.maxDepth), changedElementsInfo(dim.changedElementsInfo), m_bIsRightObject(dim.m_bIsRightObject),
		attribCube(dim.attribCube), rightCube(dim.rightCube)
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes dimension
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Dimension() {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name notification callbacks
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a dimension has been added to a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called before a dimension has been removed from a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a dimension has been removed from a database
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRemoveDimension(PDatabase database) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief called after a dimension has been renamed
	////////////////////////////////////////////////////////////////////////////////

	virtual void notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker) {
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions to save and load the dimension
	////////////////////////////////////////////////////////////////////////////////

	SaveType getDimensionType() const {return saveType;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief saves dimension name and type to file
	////////////////////////////////////////////////////////////////////////////////

	void saveDimensionType(FileWriter* file);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name getter and setter
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets the status of the dimension
	////////////////////////////////////////////////////////////////////////////////

	void setStatus(PDatabase database, DimensionStatus status);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the status of the dimension
	////////////////////////////////////////////////////////////////////////////////

	DimensionStatus getStatus() {
		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets deletable attribute
	////////////////////////////////////////////////////////////////////////////////

	void setDeletable(bool deletable) {
		this->deletable = deletable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets deletable attribute
	////////////////////////////////////////////////////////////////////////////////

	bool isDeletable() const {
		return deletable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets renamable attribute
	////////////////////////////////////////////////////////////////////////////////

	void setRenamable(bool renamable) {
		this->renamable = renamable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets renamable attribute
	////////////////////////////////////////////////////////////////////////////////

	bool isRenamable() const {
		return renamable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets changable attribute
	////////////////////////////////////////////////////////////////////////////////

	void setChangable(bool changable) {
		this->changable = changable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets changable attribute
	////////////////////////////////////////////////////////////////////////////////

	bool isChangable() const {
		return changable;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets the token
	////////////////////////////////////////////////////////////////////////////////

	virtual uint32_t getToken() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns dimension type
	////////////////////////////////////////////////////////////////////////////////

	virtual ItemType getType() const = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the list of cubes using the dimension (type filter optional)
	////////////////////////////////////////////////////////////////////////////////

	vector<CPCube> getCubes(PUser user, PDatabase db, const ItemType *filterType = 0) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name versioning related methods
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates writable copy for next version
	////////////////////////////////////////////////////////////////////////////////

	virtual PCommitable copy() const = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	virtual void loadDimension(PServer server, PDatabase db, FileReader* file);

	virtual void saveDimension(PDatabase db, FileWriter* file);

	virtual LevelType getLevel() const;

	virtual IndentType getIndent() const;

	virtual DepthType getDepth() const;

	virtual ElementsType getElements(PUser user, bool onlyRoot, uint64_t *hiddenCount=0) const;

	PSet getElemIds(CubeArea::ExpandStarType type) const;

	virtual IdentifierType getMaximalIdentifier() const {
		return maxId;
	};

	virtual PElementList getElementList() const {
		return elementList;
	}

	void _destroyElementLeavePosition(PServer server, PDatabase db, PUser user, Element* element, CubeRulesArray* disabledRules);

	virtual void clearElements(PServer server, PDatabase db, PUser user, bool useDimWorker, bool useJournal);

	virtual Element* addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal);

	void addElementEvent(PServer server, PDatabase db, Element *element);
	void addElementEvent(PServer server, PDatabase db, IdentifierType elemId, string sessionId = "");

	virtual void deleteElement(PServer server, PDatabase db, Element *element, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker);

	void deleteElements(PServer server, PDatabase db, IdentifiersType elements, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker);

	void deleteElementsEvent(PServer server, PDatabase db, const vector<string> &deletedNames);

	virtual void changeElementName(PServer server, PDatabase db, Element * element, const string& name, PUser user, bool useJournal, bool useDimWorker);

	virtual void changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove);

	virtual void moveElement(PServer server, PDatabase db, Element * element, PositionType newPosition, PUser user, bool useJournal);

	virtual void moveElements(PServer server, PDatabase db, vector<pair<Element *, PositionType> > &elem_pos, PUser user, bool useJournal);

	virtual void addChildren(PServer server, PDatabase db, Element *parent, const IdentifiersWeightType *children, PUser user, CubeRulesArray* disabledRules, bool preserveOrder, bool updateElementInfo, bool useJournal, IdentifiersType *elemsToDeleteFromCubes);

	void removeChildrenNotIn(PServer server, PDatabase db, PUser user, Element *parent, set<IdentifierType> *keep, CubeRulesArray* disabledRules, bool useJournal);

	virtual void removeChildren(PServer server, PDatabase db, PUser user, Element *parent, CubeRulesArray* disabledRules, bool useJournal, bool changingToString);


	virtual set<Element*> getBaseElements(Element* parent, bool* multiple) const;
	virtual size_t sizeElements() const {
		return (size_t)idsMap->size();
	}

    virtual size_t sizeBaseElements() const {
        size_t numBaseElements = 0;
        for(ElementList::const_iterator el = elementList->begin(); el != elementList->end(); ++el){
            if(getChildrenCount(&*el) == 0)
                ++numBaseElements;
        }
        return numBaseElements;
    }

	set<Element*> ancestors(Element * child) const;
	set<Element *> descendants(Element *parent) const;
	void descendants(const IdentifiersWeightType *elems, set<IdentifierType> &result) const; // for multiple elems

	virtual const ParentsType getParents(Element * child) const;
	virtual ParentsType getParents(PUser user, Element *child) const;

	virtual const ElementsWeightType getChildren(PUser user, Element* parent) const;

	virtual size_t getChildrenCount(Element *parent) const {
		const IdentifiersWeightType *children = parent->getChildren();
		if (children) {
			return children->size();
		} else {
			return 0;
		}
	}

	virtual bool isStringConsolidation(Element * element) const {
		return element->isStringConsolidation();
	}

	void setRightObject(bool isRightObject) {
		this->m_bIsRightObject = isRightObject;
	}

	bool isRightObject() const {
		return m_bIsRightObject;
	}

	bool isAttributed() const {
		return attribCube;
	}

	bool hasRightsCube() const {
		return rightCube;
	}

	virtual Element *lookupElement(IdentifierType elementIdentifier, bool write) const;
	virtual Element *lookupElementByName(const string &name, bool write) const;
	virtual Element *lookupElementByInternal(IdentifierType elementInternalId, bool write) const;
	virtual Element *lookupElementByPosition(PositionType position, bool write) const;
	virtual Element *findElement(IdentifierType elementIdentifier, User *user, bool write) const;
	virtual Element *findElementByName(const string &name, User *user, bool write) const;
	virtual Element *findElementByPosition(PositionType position, User *user, CPDatabase db, bool write) const;

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	void mergeUpdateVector();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates level, indent, and depth information
	////////////////////////////////////////////////////////////////////////////////

	void updateElementsInfo();

	virtual const StringVector & getElemNamesVector() const {
		return namesMap->getStringVector();
	}

	void removeElementsFromCubes(PServer server, PDatabase db, PUser user, IdentifiersType elementIds, CubeRulesArray* disabledRules, DeleteCellType delType, bool completeRemove);

	map<IdentifierType, IdentifierType> getMergeMap() const {
		return mergeNext;
	}

	void addProtectedElement(string name) {
		protectedElems.push_back(name);
	}

	virtual void checkElementAccessRight(const User *user, CPDatabase db, RightsType minimumRight) const {
		if (User::checkUser(user)) {
			RightsType rt = user->getRoleDbRight(User::elementRight, db);
			if (rt < minimumRight) {
				throw ParameterException(ErrorException::ERROR_NOT_AUTHORIZED, "insufficient access rights", "user", (int)user->getId());
			}
		}
	}

	virtual RightsType getElementAccessRight(const User *user, CPDatabase db) const {
		return User::checkUser(user) ? user->getRoleDbRight(User::elementRight, db) : RIGHT_DELETE;
	}

	virtual RightsType getDimensionDataRight(const User *user) const {
		if (hasRightsCube()) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid call of Dimension::getDimensionDataRight() method");
		} else {
			return RIGHT_DELETE;
		}
	}

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if the dimension is used in a locked cube
	////////////////////////////////////////////////////////////////////////////////

	bool isLockedByCube(PDatabase db);

	void addBaseElements(Element *element, const WeightedSet *ws, double factor);
	void removeBaseElements(Element *element, const WeightedSet *ws, double factor);
	void removeBaseElement(Element *ancestor, Element *base, set<IdentifierType> &removed);
	void updateDepthAndIndent(Element *element, Element *newParent);
	void updateLevel(Element *element, const vector<Element *> &newChildren, bool oldIsValid);

private:
	static ElementsWeightType emptyChildren;
	static ParentsType emptyParents;

	uint32_t loadDimensionOverview(FileReader* file);

	void loadDimensionElementParents(FileReader* file, Element &element, IdentifiersType* parents, uint32_t maxId, IdsToParentsMap &parentsMap);

	void loadDimensionElementChildren(FileReader* file, Element &element, IdentifiersType* children, vector<double>* weights, uint32_t numElements);

	void loadDimensionElement(FileReader* file, uint32_t numElements, IdsToParentsMap &parentsMap, ElementOld2NewMap *dimReMap);

	void loadDimensionElements(FileReader* file, uint32_t numElements, ElementOld2NewMap *dimReMap);

	void saveDimensionOverview(FileWriter* file);

	void saveDimensionElement(FileWriter* file, const Element* element);

	void saveDimensionElements(FileWriter* file);

	void checkUpdateConsolidationType(PServer server, PDatabase db, PUser user, Element* element, CubeRulesArray* disabledRules, Element *oneNewChild);

	void removeParentInChildren(Element *parent, IdentifiersWeightType *iw, set<IdentifierType> *keep = 0);

	void removeParentInChild(Element *parent, IdentifierType childId);

	void removeChildInParents(PServer server, PDatabase db, PUser user, Element *element, bool isString, CubeRulesArray* disabledRules);

	void addChildrenNC(PServer server, PDatabase db, PUser, Element *parent, const IdentifiersWeightType *children, CubeRulesArray* disabledRules, bool preserveOrder, IdentifiersWeightMap *oldWeights);

	bool isCycle(CPParents parents, const set<IdentifierType> &descendants);

	void removeElementFromCubes(PServer server, PDatabase db, PUser user, IdentifierType elementId, CubeRulesArray* disabledRules, DeleteCellType delType, bool completeRemove);

	void updateLevel(IdentifiersType &sortedElements);

	void checkElementName(const string& name);

	void addParrentsToSortedList(Element *child, IdentifiersType &sortedElements, BitVector &knownElements);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates base element list of each element and rebuilds merge index
	////////////////////////////////////////////////////////////////////////////////

	void updateElementBaseElements(IdentifiersType &sortedElements);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates the list of topological sorted elements
	////////////////////////////////////////////////////////////////////////////////

	void updateTopologicalSortedElements(IdentifiersType &sortedElements, BitVector &knownElements);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief add info about base ranges of given element to merge index
	///
	/// Adds mergeability information for the base ranges of given (consolidated)
	/// element to the merge index.
	/// Each consolidated element consists of a set of base elements (and their weights).
	/// A shorter description of these base elements is by ranges of elements, where each
	/// range consists of elements with contiguous indexes that have the same weight.
	///
	/// @author Tobias Lauer
	/// @author Christoffer Anselm
	/// @author Martin Dolezal
	////////////////////////////////////////////////////////////////////////////////

	void addToMergeIndex(Element *element);

	void clearMergeIndex() {
		mergeNext.clear();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a writable copy of elementList and indexes
	////////////////////////////////////////////////////////////////////////////////

	bool checkOut(bool ids, bool names, bool positions);
	bool verify();
public:
	bool isProtectedElement(const string &name) const {
		for (vector<string>::const_iterator it = protectedElems.begin(); it != protectedElems.end(); ++it) {
			if (*it == name) {
				return true;
			}
		}
		return false;
	}
private:

	bool hasStringChild(Element* element) const;
	static bool doRemoveElement(Element::Type oldType, bool oldIsStringCons, Element::Type newType, bool newIsStringCons);
	static DeleteCellType getDeleteCellType(Element::Type type, bool isStringCons);

	void checkHideElement(Element *element, User *user) const;

protected:
	uint32_t token; // token for changes
	DimensionStatus status; // status of the dimension
	bool deletable;
	bool renamable;
	bool changable;
	vector<string> protectedElems;

	SaveType saveType;

private:
	PElementList elementList;	// list of elements - positions = internal ids

	PIdIdSlimMap idsMap; 		// map from external id to internal id
	PNameIdSlimMap namesMap; 	// map from name to internal id
	PIdVector posIndex;			// map from position(index) to internal id

	IdentifierType maxId;		// highest element identifier
	IdentifierType minId;		// smallest element identifier

	LevelType maxLevel;			// max level of elements, 0 = base element, levelParent = max(levelChild) + 1
	IndentType maxIndent;		// max indents of elements, 1 = element has no parent, indentChild = indent of first father + 1
	DepthType maxDepth;			// max depth of elements, 0 = element has no parent, depthChild = max(depthFather) + 1

	bool changedElementsInfo;	// true if the list of elements has to be updated, base elements, Levels, Indent, Depth, maxLevel -
	// can be set only in new version copy and commit must update element and reset this flag

	bool m_bIsRightObject; // this is #_RIGHT_OBJECT_ dimension

	static const size_t MAX_ELEMS_IN_VECTOR;
	ElementsType elementVector;	// vector of pointers to element for fast lookup - limited by

	map<IdentifierType, IdentifierType> mergeNext;       // merges with next range if ignoring weight (found == true)

	bool attribCube;			// true if the dimension has attribute cube
	bool rightCube;				// true if the dimension has dimension data rights cube
};

}

#endif
