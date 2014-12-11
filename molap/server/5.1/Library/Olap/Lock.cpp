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

#include "Olap/Lock.h"
#include "Olap/Cube.h"
#include "Olap/Dimension.h"
#include "Olap/Element.h"
#include "Olap/Database.h"
#include "Engine/EngineBase.h"

namespace palo {
PLock Lock::checkLock = PLock(new Lock());

Lock::Lock(CPCube cube, PArea areaVector, const string& areaString, bool whole, IdentifierType userId) :
	Commitable(""), areaString(areaString), userId(userId), whole(whole)
{
	if (!whole) {
		computeContains(cube, areaVector);
	}
}

Lock::~Lock()
{
}

////////////////////////////////////////////////////////////////////////////////
// destination related methods
////////////////////////////////////////////////////////////////////////////////
void Lock::createStorage(CPCube cube, PFileName cubeFileName)
{
	storage.reset(new RollbackStorage(cube->getDimensions()->size(), cubeFileName, getId()));
}


bool Lock::contains(const IdentifiersType &key) const
{
	if (whole) {
		return true;
	}
	return Cube::isInArea(&key[0], containsArea.get());
}

bool Lock::blocks(const IdentifiersType &key) const
{
	if (whole) {
		return true;
	}
	return Cube::isInArea(&key[0], overlapArea.get());
}

void Lock::computeContains(CPCube cube, PArea area)
{
	size_t dimCount = area->dimCount();
	containsArea.reset(new Area(dimCount));
	ancestorsIdentifiers.reset(new Area(dimCount));
	overlapArea.reset(new Area(dimCount));

	const IdentifiersType *dims = cube->getDimensions();

	for (size_t i = 0; i < dimCount; i++) {
		if (area->elemCount(i)) {
			CPDimension d = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube))->lookupDimension((*dims)[i], false);

			PSet containsSubset(new Set);
			PSet overlapSubset(new Set);
			containsArea->insert((IdentifierType)i, containsSubset);
			overlapArea->insert((IdentifierType)i, overlapSubset);
			// compute children
			for (Area::ConstElemIter iter = area->elemBegin(i); iter != area->elemEnd(i); ++iter) {
				IdentifierType id = *iter;
				Element * e = d->findElement(id, 0, false);

				// simple children
				containsSubset->insert(e->getIdentifier());
				overlapSubset->insert(e->getIdentifier());

				if (e->getElementType() == Element::CONSOLIDATED && !d->isStringConsolidation(e)) {
					computeChildren(i, d, e, containsSubset, overlapSubset);
				}
			}

			// compute (not string) ancestors
			for (Area::ConstElemIter iter = area->elemBegin(i); iter != area->elemEnd(i); ++iter) {
				IdentifierType id = *iter;
				Element * e = d->findElement(id, 0, false);

				// do not add parents of string nodes
				if (e->getElementType() == Element::NUMERIC || (e->getElementType() == Element::CONSOLIDATED && !d->isStringConsolidation(e))) {
					computeAncestors(i, d, e, containsSubset, overlapSubset);
				}
			}
		}
	}
}

void Lock::computeChildren(size_t dimId, CPDimension dimension, Element *parent, PSet childrencont, PSet childrenover)
{

	const IdentifiersWeightType *childrenIds = parent->getChildren();

	CPSet containsSubset = containsArea->getDim(dimId);
	CPSet overlapSubset = overlapArea->getDim(dimId);
	for (IdentifiersWeightType::const_iterator i = childrenIds->begin(); i != childrenIds->end(); ++i) {
		Element *child = dimension->lookupElement(i->first, false);
		childrencont->insert(i->first);
		childrenover->insert(i->first);

		if (child->getElementType() == Element::CONSOLIDATED) {
			computeChildren(dimId, dimension, child, childrencont, childrenover);
		}
	}
}

void Lock::computeAncestors(size_t dimId, CPDimension dimension, Element *child, PSet ancestorscont, PSet ancestorsover)
{

	CPParents parents = child->getParents();

	CPSet containsSubset = containsArea->getDim(dimId);
	CPSet overlapSubset = overlapArea->getDim(dimId);
	if (parents) {
		for (Parents::const_iterator i = parents->begin(); i != parents->end(); ++i) {
			Element *parent = dimension->lookupElement(*i, false);

			if (!parent->isStringConsolidation()) {
				ancestorscont->insert(parent->getIdentifier());
				ancestorsover->insert(parent->getIdentifier());
				computeAncestors(dimId, dimension, parent, ancestorscont, ancestorsover);
			}
		}
	}
}

bool Lock::overlaps(CPArea area) const
{
	if (whole) {
		return true;
	}
	size_t dimA = 0;
	size_t dimB = 0;

	for (; dimA != area->dimCount(); dimA++, dimB++) {
		CPSet identifiersA = area->getDim(dimA);
		CPSet identifiersB = area->getDim(dimB);

		bool foundIdentifier = false;

		for (Set::Iterator idIterA = identifiersA->begin(); idIterA != identifiersA->end(); ++idIterA) {
			Set::Iterator found = identifiersB->find(*idIterA);
			if (found != identifiersB->end()) {
				foundIdentifier = true;
				break;
			}
		}

		if (!foundIdentifier) {
			// no dimension element of area found in containsArea
			return false;
		}
	}

	return true;
}

PCommitableList LockList::createnew(const CommitableList& l) const
{
	return PCommitableList(new LockList(dynamic_cast<const LockList &>(l)));
}

bool Lock::merge(const CPCommitable &o, const PCommitable &p)
{
	if (o && old) {
		throw ErrorException(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED, "Locks don't merge.");
	}
	commitintern();
	return true;
}

PCommitable Lock::copy() const
{
	throw ErrorException(ErrorException::ERROR_API_CALL_NOT_IMPLEMENTED, "Locks don't copy.");
}

}
