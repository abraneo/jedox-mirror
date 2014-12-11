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

#include "Olap/MarkerStorage.h"
#include "Olap/Context.h"
#include "Collections/CellMap.h"

namespace palo {

MarkerStorage::MarkerStorage(PRuleMarker marker, size_t targetDimCount, vector<Dimension*>* dimensions) :
	numberDimensions(targetDimCount), dimensions(dimensions)
{
	permutations = marker->getPermutations();
	maps = marker->getMapping();

	tmpKeyBuffer.resize(numberDimensions);

	const uint32_t* qtr = marker->getFixed();
	const int16_t* perm = permutations;
	for (size_t i = 0; i < numberDimensions; i++, perm++, qtr++) {
		if (*perm == FIXED_ELEMENT) {
			tmpKeyBuffer[i] = *qtr;
		}
	}

	markerSet = CreateCellSet(targetDimCount);
}

MarkerStorage::~MarkerStorage()
{
}

void MarkerStorage::generateMarkers(PCube fromCube, const Area* fromArea)
{
	Context* context = Context::getContext();
	CPDatabase db = CONST_COMMITABLE_CAST(Database, context->getParent(fromCube));
	PEngineBase engine = context->getServer()->getEngine(EngineBase::CPU, false);

	PCubeArea ca(new CubeArea(db, fromCube, *fromArea));
	PArea area = ca->expandStar(CubeArea::BASE_ELEMENTS);

	//go through string storage
	PSourcePlanNode sn(new SourcePlanNode(fromCube->getStringStorageId(), area, fromCube->getObjectRevision()));
	PProcessorBase cs = engine->createProcessor(sn, true);
	while (cs->next()) {
		addMarker(cs->getKey());
	}
	//go through numeric storage
	sn.reset(new SourcePlanNode(fromCube->getNumericStorageId(), area, fromCube->getObjectRevision()));
	cs = engine->createProcessor(sn, true);
	while (cs->next()) {
		addMarker(cs->getKey());
	}
	//go through marker storage
	sn.reset(new SourcePlanNode(fromCube->getMarkerStorageId(), area, fromCube->getObjectRevision())); // TODO: -jj- right version object?
	cs = engine->createProcessor(sn, true);
	while (cs->next()) {
		addMarker(cs->getKey());
	}

	//go through changedCells
	PCellMapPlanNode cmpn(new CellMapPlanNode(fromCube->getMarkerStorageId(), area));
	cs = engine->createProcessor(cmpn, true);
	while (cs->next()) {
		addMarker(cs->getKey());
	}
}

const MarkerStorage::PMarkerSet MarkerStorage::getMarkers() const
{
	return markerSet;
}

void MarkerStorage::addMarker(const IdentifiersType& key)
{
	const uint32_t* path = &key[0];
	const int16_t* perm = permutations;

	MarkerMappingIterator mmi(numberDimensions, permutations, maps, path, dimensions);

	ICellSet *pMarkerSet = markerSet.get();
	do {
		if (!maps && !dimensions) {
			for (size_t i = 0; i < numberDimensions; i++, perm++) {
				if (*perm != FIXED_ELEMENT) {
					tmpKeyBuffer[i] = path[*perm];
				}
			}
		} else {
			if (!mmi.init()) {
				// no combination -> return
				return;
			}
			// generate all combinations
			for (size_t targetDim = 0; targetDim < numberDimensions; targetDim++) {
				int16_t sourceDim = permutations[targetDim];
				if (sourceDim != FIXED_ELEMENT) {
					tmpKeyBuffer[targetDim] = mmi[targetDim];
				}
			}

			++mmi; // next combination
		}

		pMarkerSet->set(tmpKeyBuffer);
	} while (!mmi.isEndOfCombinations());
}

MarkerMappingIterator::MarkerMappingIterator(size_t numberTargetDimensions, const int16_t* permutations, const RuleMarker::PMappingType* maps, const uint32_t* path, const vector<Dimension*>* dimensions) :
	permutations(permutations), numberTargetDimensions(numberTargetDimensions), maps(maps), path(path), endOfCombinations(!maps && !dimensions), dimensions(dimensions)
{
}

bool MarkerMappingIterator::isEndOfCombinations() const {
	return endOfCombinations;
}

const uint32_t MarkerMappingIterator::operator[](size_t targetDim) const {
	if (permutations[targetDim] == MarkerStorage::ALL_ELEMENTS) {
		return (*currentBaseElemIterator[targetDim]).getIdentifier();
	} else if (maps && maps[targetDim]) {
		return currentMapIterators[targetDim]->second;
	} else {
		return path[permutations[targetDim]];
	}
}

MarkerMappingIterator& MarkerMappingIterator::operator++() { //++o
	// iterate to next combination
	for (size_t targetDim = 0; targetDim < numberTargetDimensions; targetDim++) {
		int16_t sourceDim = permutations[targetDim];
		if (sourceDim == MarkerStorage::ALL_ELEMENTS && dimensions && (*dimensions)[targetDim]) {
			while (++currentBaseElemIterator[targetDim] != endBaseElemIterator[targetDim]) {
				Element::Type elemType = (*currentBaseElemIterator[targetDim]).getElementType();
				if (elemType == Element::NUMERIC || elemType == Element::STRING) {
					break;
				}
			}
			if (currentBaseElemIterator[targetDim] == endBaseElemIterator[targetDim]) {
				currentBaseElemIterator[targetDim] = initialBaseElemIterator[targetDim];
			} else {
				return *this;
			}
		} else if (sourceDim != MarkerStorage::FIXED_ELEMENT && maps && maps[targetDim]) {
			if (++currentMapIterators[targetDim] == maps[targetDim]->end() || currentMapIterators[targetDim]->first != path[sourceDim]) {
				// no more combinations in this dimension
				currentMapIterators[targetDim] = initialMapIterators[targetDim];
				// continue for next dimensions;
			} else {
				return *this;
			}
		}
	}
	endOfCombinations = true;
	return *this;
}

bool MarkerMappingIterator::init() {
	if (!currentMapIterators.size()) {
		// initialize first combination from map
		currentMapIterators.resize(numberTargetDimensions);
		initialBaseElemIterator.resize(numberTargetDimensions);
		currentBaseElemIterator.resize(numberTargetDimensions);
		endBaseElemIterator.resize(numberTargetDimensions);
		for (size_t targetDim = 0; targetDim < numberTargetDimensions; targetDim++) {
			int16_t sourceDim = permutations[targetDim];
			if (sourceDim == MarkerStorage::ALL_ELEMENTS && dimensions) {
				const PElementList elementList = (*dimensions)[targetDim]->getElementList();
				currentBaseElemIterator[targetDim] = elementList->begin();
				endBaseElemIterator[targetDim] = elementList->end();
				while (currentBaseElemIterator[targetDim] != endBaseElemIterator[targetDim]) {
					Element::Type elemType = (*currentBaseElemIterator[targetDim]).getElementType();
					if (elemType == Element::NUMERIC || elemType == Element::STRING) {
						initialBaseElemIterator[targetDim] = currentBaseElemIterator[targetDim];
						break;
					}
					++currentBaseElemIterator[targetDim];
				}
				if (currentBaseElemIterator[targetDim] == endBaseElemIterator[targetDim]) {
					// no combination -> return
					endOfCombinations = true;
					return false;
				}
			} else if (sourceDim != MarkerStorage::FIXED_ELEMENT) {
				if (maps && maps[targetDim]) {
					currentMapIterators[targetDim] = maps[targetDim]->find(path[sourceDim]);
					if (currentMapIterators[targetDim] == maps[targetDim]->end()) {
						// no combination -> return
						endOfCombinations = true;
						return false;
					}
				} else {
					// 1:1 mapping - use path[sourceDim]
				}
			}
		}
		// remember search result for next combinations
		initialMapIterators = currentMapIterators;
	}
	return true;
}

}
