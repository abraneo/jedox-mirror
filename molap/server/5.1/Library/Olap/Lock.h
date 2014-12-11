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

#ifndef OLAP_LOCK_H
#define OLAP_LOCK_H 1

#include "palo.h"

#include "Olap/RollbackStorage.h"

namespace palo {
class Cube;
class Dimension;

class SERVER_CLASS LockList : public CommitableList {
public:
	LockList(const PIdHolder &newidh) : CommitableList(newidh) {}
	LockList() {}
	LockList(const CubeList &l);
	virtual PCommitableList createnew(const CommitableList& l) const;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP cube area lock
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Lock : public Commitable {

public:
	static PLock checkLock;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new locked area
	////////////////////////////////////////////////////////////////////////////////

	Lock(CPCube cube, PArea areaVector, const string& areaString, bool whole, IdentifierType userId);

	Lock() :
		Commitable("") {
	}

	void createStorage(CPCube cube, PFileName cubeFileName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Lock();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get lock area as String
	////////////////////////////////////////////////////////////////////////////////

	const string& getAreaString() const {
		return areaString;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get the identifier of the user
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getUserIdentifier() const {
		return userId;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get contains area
	////////////////////////////////////////////////////////////////////////////////

	CPArea getContainsArea() const {
		return containsArea;
	}

	bool isWhole() const {
		return whole;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if a locked area is touched by a cell path
	///
	/// Returns true if the cell path changes values in the locked area.
	////////////////////////////////////////////////////////////////////////////////

	bool contains(const IdentifiersType &key) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if a locked area is blocking a cell path
	///
	/// Returns true if the cell path splashes or set values in the locked area.
	////////////////////////////////////////////////////////////////////////////////

	bool blocks(const IdentifiersType &key) const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if another locked area overlaps to the locked area
	////////////////////////////////////////////////////////////////////////////////

	bool overlaps(CPArea area) const;

	PRollbackStorage getStorage() const {
		return storage;
	}

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

private:

	void computeContains(CPCube cube, PArea area);

	void computeChildren(size_t dimId, CPDimension, Element*, PSet childrencont, PSet childrenover);

	void computeAncestors(size_t dimId, CPDimension, Element*, PSet ancestorscont, PSet ancestorsover);

private:
	string areaString;
	IdentifierType userId;

	// area elements and all children
	PArea containsArea;

	// ancestor elements of the locked area
	PArea ancestorsIdentifiers;

	// area elements, all children and ancestor elements
	PArea overlapArea;

	PRollbackStorage storage;

	bool whole;
};

}

#endif
