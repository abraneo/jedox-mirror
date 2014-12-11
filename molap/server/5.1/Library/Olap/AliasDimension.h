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
 * 
 *
 */

#ifndef OLAP_ALIAS_DIMENSION_H
#define OLAP_ALIAS_DIMENSION_H 1

#include "palo.h"

#include "Olap/SystemDimension.h"
#include "Olap/Server.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief alias OLAP dimension
///
/// An OLAP dimension is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS AliasDimension : public SystemDimension {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new dimension with given identifier
	////////////////////////////////////////////////////////////////////////////////

	AliasDimension(const string& name, IdentifierType aliasId, IdentifierType dbId) :
		SystemDimension(name, Dimension::ALIAS), aliasId(aliasId), dbId(dbId)
	{
		status = LOADED;
	}

	virtual uint32_t getToken() const;

	void loadDimension(PServer server, PDatabase db, FileReader* file) {
		return;
	}

	void saveDimension(PDatabase db, FileWriter* file) {
		return;
	}

	LevelType getLevel() const {
		return alias(false)->getLevel();
	}

	IndentType getIndent() const {
		return alias(false)->getIndent();
	}

	DepthType getDepth() const {
		return alias(false)->getDepth();
	}

	ElementsType getElements(PUser user, bool onlyRoot) const {
		return alias(false)->getElements(user, onlyRoot);
	}

	IdentifierType getMaximalIdentifier() const {
		return alias(false)->getMaximalIdentifier();
	}

	PElementList getElementList() const {
		return alias(false)->getElementList();
	}

	void clearElements(PServer server, PDatabase db, PUser user, bool useDimWorker, bool useJournal) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->clearElements(server, database, user, useDimWorker, useJournal);
	}

	Element* addElement(PServer server, PDatabase db, IdentifierType idElement, const string& name, Element::Type elementType, PUser user, bool useJournal) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		return alias(true)->addElement(server, database, idElement, name, elementType, user, useJournal);
	}

	void deleteElement(PServer server, PDatabase db, Element *element, PUser user, bool useJournal, CubeRulesArray* disabledRules, bool useDimWorker) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->deleteElement(server, database, element, user, useJournal, disabledRules, useDimWorker);
	}

	void changeElementName(PServer server, PDatabase db, Element * element, const string& name, PUser user, bool useJournal, bool useDimWorker) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->changeElementName(server, database, element, name, user, useJournal, useDimWorker);
	}

	void changeElementType(PServer server, PDatabase db, Element * element, Element::Type elementType, PUser user, bool setConsolidated, CubeRulesArray* disabledRules, IdentifiersType *elemsToDeleteFromCubes, bool doRemove) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->changeElementType(server, database, element, elementType, user, setConsolidated, disabledRules, elemsToDeleteFromCubes, doRemove);
	}

	void moveElement(PServer server, PDatabase db, Element * element, PositionType newPosition, PUser user, bool useJournal) {
		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->moveElement(server, database, element, newPosition, user, useJournal);
	}

	void moveElements(PServer server, PDatabase db, vector<pair<Element *, PositionType> > &elem_pos, PUser user, bool useJournal) {
		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->moveElements(server, database, elem_pos, user, useJournal);
	}

	void addChildren(PServer server, PDatabase db, Element *parent, const IdentifiersWeightType *children, PUser user, CubeRulesArray* disabledRules, bool preserveOrder, bool updateElementInfo, bool useJournal, IdentifiersType *elemsToDeleteFromCubes) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->addChildren(server, database, parent, children, user, disabledRules, preserveOrder, updateElementInfo, useJournal, elemsToDeleteFromCubes);
	}

	void removeChildren(PServer server, PDatabase db, PUser user, Element * parent, CubeRulesArray* disabledRules, bool useJournal, bool changingToString) {
		if (!isChangable()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "dimension cannot be changed", "dimension", getName());
		}

		PDatabase database = server->findDatabase(dbId, user, true, true);
		alias(true)->removeChildren(server, database, user, parent, disabledRules, useJournal, changingToString);
	}

	size_t sizeElements() const {
		return alias(false)->sizeElements();
	}

	const ParentsType getParents(Element * child) const {
		return alias(false)->getParents(child);
	}
	virtual ParentsType getParents(PUser user, Element *child) const
	{
		return alias(false)->getParents(user, child);
	}

	const ElementsWeightType getChildren(PUser user, Element* parent) const {
		return alias(false)->getChildren(user, parent);
	}

	size_t getChildrenCount(Element *parent) const {
		return alias(false)->getChildrenCount(parent);
	}

	set<Element*> getBaseElements(Element* parent, bool* multiple) const {
		return alias(false)->getBaseElements(parent, multiple);
	}

	bool isStringConsolidation(Element * element) const {
		return alias(false)->isStringConsolidation(element);
	}

	Element * lookupElement(IdentifierType elementIdentifier, bool write) const {
		return alias(write)->lookupElement(elementIdentifier, write);
	}

	Element * lookupElementByName(const string& name, bool write) const {
		return alias(write)->lookupElementByName(name, write);
	}

	Element * lookupElementByPosition(PositionType position, bool write) const {
		return alias(false)->lookupElementByPosition(position, write);
	}

	Element* findElement(IdentifierType elementIdentifier, User *user, bool write) const {
		return alias(write)->findElement(elementIdentifier, user, write);
	}

	Element * findElementByName(const string& name, User *user, bool write) const {
		return alias(write)->findElementByName(name, user, write);
	}

	Element * findElementByPosition(PositionType position, User *user, CPDatabase db, bool write) const {
		return alias(false)->findElementByPosition(position, user, db, write);
	}

	virtual const StringVector & getElemNamesVector() const {
		return alias(false)->getElemNamesVector();
	}

	virtual bool merge(const CPCommitable &o, const PCommitable &p);

	virtual PCommitable copy() const;

	virtual void checkElementAccessRight(const User *user, CPDatabase db, RightsType minimumRight) const {
		alias(false)->checkElementAccessRight(user, db, minimumRight);
	}

	virtual RightsType getElementAccessRight(const User *user, CPDatabase db) const {
		return alias(false)->getElementAccessRight(user, db);
	}

private:
	PDimension alias(bool write) const;

	IdentifierType aliasId;
	IdentifierType dbId;

	friend class Dimension;
};

}

#endif
