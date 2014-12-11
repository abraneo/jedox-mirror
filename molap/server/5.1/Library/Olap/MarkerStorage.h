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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_MARKER_STORAGE_H
#define OLAP_MARKER_STORAGE_H 1

#include "palo.h"

#include "Olap/RuleMarker.h"
#include "Olap/Element.h"
#include "Collections/CellSet.h"


namespace palo {

class SERVER_CLASS MarkerStorage {
public:
	static const int16_t FIXED_ELEMENT = -1;
	static const int16_t ALL_ELEMENTS = -2;

	typedef boost::shared_ptr<ICellSet> PMarkerSet;

	MarkerStorage(PRuleMarker marker, size_t targetDimCount, vector<Dimension*>* dimensions);
	~MarkerStorage();

	void generateMarkers(PCube fromCube, const Area* fromArea);
	const PMarkerSet getMarkers() const;

private:
	void addMarker(const IdentifiersType& key);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief total number of dimensions in destination cube
	////////////////////////////////////////////////////////////////////////////////

	size_t numberDimensions;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief temporary buffer
	////////////////////////////////////////////////////////////////////////////////

	IdentifiersType tmpKeyBuffer;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief permutation of keys
	////////////////////////////////////////////////////////////////////////////////

	const int16_t* permutations;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief mapping of identifiers between cubes
	///
	/// The mapping is not owned by MarkerStorage. It must not be deleted.
	////////////////////////////////////////////////////////////////////////////////

	const RuleMarker::PMappingType* maps;

	const vector<Dimension*> *dimensions;
	PMarkerSet markerSet;
};

class SERVER_CLASS MarkerMappingIterator {
public:
	MarkerMappingIterator(size_t numberTargetDimensions, const int16_t* permutations, const RuleMarker::PMappingType* maps, const uint32_t* path, const vector<Dimension*>* dimensions);
	bool isEndOfCombinations() const;
	const uint32_t operator[](size_t targetDim) const;
	MarkerMappingIterator& operator++(); //++o
	bool init();
private:
	const int16_t* permutations;
	size_t numberTargetDimensions;
	const RuleMarker::PMappingType* maps;
	const uint32_t* path;
	bool endOfCombinations;
	vector<ElementList::const_iterator> initialBaseElemIterator;
	vector<ElementList::const_iterator> currentBaseElemIterator;
	vector<ElementList::const_iterator> endBaseElemIterator;
	vector<RuleMarker::MappingType::const_iterator> currentMapIterators;
	vector<RuleMarker::MappingType::const_iterator> initialMapIterators;
	const vector<Dimension*>* dimensions;
};

}

#endif
