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
 * \author Tobias Lauer, Jedox AG, Freiburg, Germany
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Olap/Element.h"

#include "Olap/Database.h"
#include "Olap/Dimension.h"
#include "Olap/Context.h"
#include "Olap/Cube.h"


namespace palo {
IdentifiersWeightType Relations::emptyChildren;
PParents Relations::emptyParents(new Parents());

BaseRangesWeightType Relations::emptyRanges;
PWeightedSet Relations::emptyBaseElements(new WeightedSet());


IdentifierType Parents::at(uint32_t index) const
{
	if (parentCount == 1) {
		return singleParent;
	} else {
		return (*parents)[index];
	}
}


void Parents::erase(const_iterator it)
{
	if (parentCount == 1) {
		parentCount = 0;
	} else if (parentCount == 2) {
		parentCount = 1;
		singleParent = (*parents)[1 - it.index];
		delete parents;
		parents = NULL;
	} else {
		parentCount--;
		parents->erase(parents->begin() + it.index);
	}
}


void Parents::push_back(IdentifierType id)
{
	if (parentCount == 0) {
		parentCount = 1;
		singleParent = id;
	} else if (parentCount == 1) {
		parentCount = 2;
		parents = new IdentifiersType();
		parents->push_back(singleParent);
		parents->push_back(id);
	} else {
		parents->push_back(id);
		parentCount++;
	}
}


PRelations Relations::checkOut(PRelations current)
{
	if (current && current->isCheckedOut()) {
		return current;
	}
	PRelations newRel(current ? new Relations(*current.get()) : new Relations());
	return newRel;
}


Relations::Relations(const Relations &r) :
		isStringConsolidation(r.isStringConsolidation), checkedOut(true)
{
	if (r.baseElements) {
		baseElements = new WeightedSet(*r.baseElements);
	} else {
		baseElements = NULL;
	}

	if (r.children) {
		children = new IdentifiersWeightType(*r.children);
	} else {
		children = NULL;
	}

	if (r.parents) {
		parents = PParents(new Parents(*r.parents));
	} else {
		parents = PParents();
	}
}


CPParents Element::getParents() const
{
	if (relations && relations->parents) {
		return relations->parents;
	}
	return Relations::emptyParents;
}

// check in caller if element's page is checkedOut
PParents Element::getParents(bool writable)
{
	if (writable) {
		relations = Relations::checkOut(relations);
		if (!relations->parents) {
			relations->parents = PParents(new Parents());
		} else {
			relations->parents = PParents(new Parents(*relations->parents));
		}
	}
	return (relations && relations->parents) ? relations->parents : Relations::emptyParents;
}

// check in caller if element's page is checkedOut
void Element::setParents(PParents parents)
{
	relations = Relations::checkOut(relations);
	relations->parents = parents;
}

size_t Element::getParentsCount() const
{
	if (relations && relations->parents) {
		return relations->parents->size();
	} else {
		return 0;
	}
}

const IdentifiersWeightType * Element::getChildren() const
{
	if (relations != 0 && relations->children) {
		return relations->children;
	} else {
		return &Relations::emptyChildren;
	}
}

// check in caller if element's page is checkedOut
IdentifiersWeightType * Element::getChildren(bool writable)
{
	if (writable) {
		relations = Relations::checkOut(relations);
		if (!relations->children) {
			relations->children = new IdentifiersWeightType();
		}
	}
	return (relations && relations->children) ? relations->children : &Relations::emptyChildren;
}

size_t Element::getChildrenCount() const
{
	if (relations && relations->children) {
		return relations->children->size();
	} else {
		return 0;
	}
}

const WeightedSet * Element::getBaseElements() const
{
	if (relations != 0) {
		return relations->baseElements;
	} else {
		return NULL;
	}
}


// check in caller if element's page is checkedOut
WeightedSet * Element::getBaseElements(bool writable)
{
	if (writable) {
		relations = Relations::checkOut(relations);
		if (!relations->baseElements) {
			relations->baseElements = new WeightedSet();
		}
	}
	return relations ? relations->baseElements : NULL;
}


WeightedSet::const_iterator Element::baseElementsBegin() const
{
	if (relations && relations->baseElements) {
		return relations->baseElements->begin();
	} else {
		return relations->emptyBaseElements->begin();
	}
}


WeightedSet::const_iterator Element::baseElementsEnd() const
{
	if (relations && relations->baseElements) {
		return relations->baseElements->end();
	} else {
		return relations->emptyBaseElements->end();
	}
}


size_t Element::getBaseElementsCount() const
{
	if (relations && relations->baseElements) {
		return relations->baseElements->size();
	} else {
		return 0;
	}
}


// check in caller if element's page is checkedOut
void Element::baseElementsClear()
{
	relations = Relations::checkOut(relations);
	if (relations->baseElements) {
		delete relations->baseElements;
	}
	relations->baseElements = NULL;
}


// check in caller if element's page is checkedOut
void Element::setBaseElements(WeightedSet *ws)
{
	relations = Relations::checkOut(relations);
	delete relations->baseElements;
	relations->baseElements = ws;
}


bool Element::isStringConsolidation() const
{
	bool isStringCons = false;
	if (relations != 0) {
		isStringCons = (relations->isStringConsolidation == 1);
	}
	return isStringCons;
}

// check in caller if element's page is checkedOut
void Element::setStringConsolidation(bool isString)
{
	if (isString) {
		relations = Relations::checkOut(relations);
	}
	if (relations) {
		relations->isStringConsolidation = (isString ? 1 : 0);
	}
}

void
Element::release()
{
	setName(StringVector::StringId());
	setElementType(UNDEFINED);
	setIdentifier(NO_IDENTIFIER);
	relations.reset();
}

bool
ElementList::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);
	CPElementList elemList = CONST_COMMITABLE_CAST(ElementList, o);
	CPElementList oldElemList = CONST_COMMITABLE_CAST(ElementList, old);

	if (ret) {
		// return false if any conflict in pages
		if (elemList && oldElemList) {
			if (pages != elemList->pages && elemList->pages != oldElemList->pages) {
				ret = false;
			}
		}
	}
	if (ret) {
		if (elemList && oldElemList) {
			// get changes made by others
			if (pages == oldElemList->pages) {
				pages = elemList->pages;
				freeInternals = elemList->freeInternals;
			}
		}
		for (ElementList::const_iterator it = this->begin(); it != this->end(); ++it) {
			Element &element = *it;

			if (element.getElementType() == Element::UNDEFINED) {
				continue;
			}
			if (element.relations) {
				element.relations->commit();
			}
		}
	}

	if (ret) {
		commitintern();
	}
	return ret;
}

PCommitable ElementList::copy() const
{
	checkNotCheckedOut();
	PElementList newEL(new ElementList(*this));
	return newEL;
}

void ElementList::checkOut(IdentifierType id)
{
	checkCheckedOut();

	size_t page = id / ELEMENTS_PER_PAGE;
	if (old) {
		CPElementList oldList = CONST_COMMITABLE_CAST(ElementList, old);

		if (oldList->pages.size() > page && pages[page] == oldList->pages[page]) {
			pages[page] = oldList->pages[page]->copy();
		}
	}
}

IdentifierType ElementList::addElement(const Element &elem, PDatabase db, const Dimension *dim)
{
	checkCheckedOut();

	Element *pelement;
	IdentifierType internalId;
	if (!freeInternals.empty()) {
		// reuse free slot
		internalId = freeInternals.back();
		freeInternals.pop_back();
	} else {
		// if last page is full
		if (pages.empty() || pages.back()->size == ELEMENTS_PER_PAGE) {
			// allocate new page
			// add the page to the list
			pages.push_back(PElementPage(new ElementPage()));
		}
		internalId = (IdentifierType)((pages.size() - 1) * ELEMENTS_PER_PAGE + pages.back()->size);
		pages.back()->size++;
	}
	checkOut(internalId);
	(*this)[internalId] = elem;

	// position to pointer
	pelement = &(*this)[internalId];

	// assign Id
	if (pelement->getIdentifier() == NO_IDENTIFIER) {
		pelement->setIdentifier(idh->getNewId());
	} else {
		IdentifierType last = idh->getLastId();
		IdentifierType curr = pelement->getIdentifier();
		if (last <= curr) {
			idh->setStart(curr + 1);
		}
	}

/*	if (db && dim && pelement->getIdentifier() > dim->getMaximalIdentifier()) {
		// here go through all cubes and check if their PathTranslator has to change
        ItemType gpuFilter = GPUTYPE;
		vector<CPCube> cubes = dim->getCubes(PUser(), db, &gpuFilter);
		for (vector<CPCube>::iterator cubeIt = cubes.begin(); cubeIt != cubes.end(); ++cubeIt) {
			const IdentifiersType *cubeDimIds =  (*cubeIt)->getDimensions();
			if (cubeDimIds) {
				uint32_t dimIdx = 0;
                for(IdentifiersType::const_iterator cIt = cubeDimIds->begin(); cIt != cubeDimIds->end(); cIt++, dimIdx++){
                    if(*cIt == dim->getId()) {
                        break;
                    }
                }
                //get number of bits used to represent this dimension
                const uint32_t numBits = (*cubeIt)->getPathTranslator()->getDimensionBitSize(dimIdx);
                //number of base-elements in this dimension
                if(pelement->getIdentifier() >= (size_t)(1 << numBits)) { //if there is sufficient space for a new element in this dimension
                	// destroy the gpuStorage
                	db->deleteGpuStorage(Context::getContext()->getServerCopy(), *cubeIt);
                }
			}
		}
	}*/

	return internalId;
}

void ElementList::deleteElement(IdentifierType internalId)
{
	if (internalId != NO_IDENTIFIER) {
		// position to pointer
		Element *element = &(*this)[internalId];
		// recycle internal id
		freeInternals.push_back(internalId);
		element->release();
	}
}

const Element &
ElementList::operator[](const size_t& pos) const
{
	size_t page = pos / ELEMENTS_PER_PAGE;
	if (page < pages.size()) {
		return pages[page]->elements[pos % ELEMENTS_PER_PAGE];
	}
	throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid internal element id");
}

Element &
ElementList::operator[](const size_t& pos)
{
	size_t page = pos / ELEMENTS_PER_PAGE;
	if (page < pages.size()) {
		return pages[page]->elements[pos % ELEMENTS_PER_PAGE];
	}
	throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid internal element id");
}

ElementList::const_iterator
ElementList::const_iterator::operator=(const const_iterator &iter)
{
	it = iter.it;
	pages = iter.pages;
	pagePos = iter.pagePos;
	return *this;
}

bool
ElementList::const_iterator::operator==(const const_iterator &iter) const
{
	return this->it == iter.it && this->pagePos == iter.pagePos;
}

bool
ElementList::const_iterator::operator!=(const const_iterator &iter) const
{
	return !operator==(iter);
}

ElementList::const_iterator
ElementList::const_iterator::operator++(int i) //o++
{
	ElementList::const_iterator it(*this);
	operator++();
	return it;
}

ElementList::const_iterator &
ElementList::const_iterator::operator++() //++o
{
	while (it != pages->end()) {
		if (++pagePos < ELEMENTS_PER_PAGE) {
			// TODO -jj- test validity of element
			break;
		} else {
			// end of the page reached
			pagePos = 0;
			++it;
			if (it == pages->end()) {
				break;
			}
			// TODO -jj- test validity of current element
			break;
		}
	}
	return *this;
}

Element&
ElementList::const_iterator::operator*() const
{
	return (*it)->elements[pagePos];
}

ElementList::const_iterator::const_iterator() : pages(0), it(), pagePos(0)
{
}

ElementList::const_iterator::const_iterator(const vector<PElementPage> &pgs, vector<PElementPage>::const_iterator &i) : pages(&pgs), it(i), pagePos(0)
{
}


ElementList::const_iterator ElementList::begin() const
{
	vector<PElementPage>::const_iterator it1(pages.begin());
	ElementList::const_iterator it(pages, it1);
	return it;
}

ElementList::const_iterator ElementList::end() const
{
	vector<PElementPage>::const_iterator it1(pages.end());
	ElementList::const_iterator it(pages, it1);
	return it;
}

}
