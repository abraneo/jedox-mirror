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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "palo.h"
#include "Olap/SubCubeList.h"
#include "Engine/EngineBase.h"

namespace palo {

SubCubeList::SubCubeList(PCubeArea &area)
{
	push_back(make_pair(getRank(*area.get()), area));
}

SubCubeList::SubCubeList(const SubCubeList &l)
{
	for (const_iterator it = l.begin(); it != l.end(); ++it) {
		push_back(*it);
	}
}

void SubCubeList::insertAndMerge(CPCubeArea area)
{
	insertAndMerge(make_pair(getRank(*area.get()), area));
}

void SubCubeList::insertAndMerge(pair<uint8_t, CPCubeArea> rankedArea)
{
	push_front(rankedArea);

	if (size() > 1) {
		iterator bit = begin();
		iterator it = bit;
		for (++it; it != end(); ++it) {
			if (abs(it->first - bit->first) <= 1 && joinSubCubes(*(it->second), *(bit->second))) {
				erase(bit);
				erase(it);
				if (size() == 1) {
					break;
				} else {
					bit = begin();
					it = bit;
				}
			}
		}
	}
}

bool SubCubeList::joinSubCubes(const CubeArea &area1, const CubeArea &area2)
{
	size_t diffCount = 0;
	size_t diffDim = -1;
	size_t dimCount = area1.dimCount();
	for (size_t i = 0; i < dimCount; i++) {
//		CPSet set1 = area1.getDim(i);
//		CPSet set2 = area2.getDim(i);
		if (*area1.getDim(i) != *area2.getDim(i)) {
			diffCount++;
			diffDim = i;
		}/* else if (set1 != set2) {
			// TODO -jj- identical content different instances - do it also for other dimensions
			area1->insert(i,set2);
		}*/
		if (diffCount > 1) {
			return false; // there are at least two differences
		}
	}

	PCubeArea newCube(new CubeArea(area1.getDatabase(), area1.getCube(), dimCount));
	uint8_t rank = 0;
	for (size_t i = 0; i < dimCount; i++) {
		if (i == diffDim) {
			//insert IDs from the shorter set to the copy of the longer set
			bool smaller = area1.elemCount(i) < area2.elemCount(i);
			const Set *shortSet = smaller ? area1.getDim(i).get() : area2.getDim(i).get();
			const Set *longSet =  smaller ? area2.getDim(i).get() : area1.getDim(i).get();
			PSet unionSet(new Set(*longSet));
			unionSet->insert(shortSet->begin(), shortSet->end());
			newCube->insert(i, unionSet);
		} else {
			newCube->insert(i, area1.getDim(i));
		}
		if (newCube->elemCount(i) > 1) {
			rank++;
		}
	}
	push_front(make_pair(rank, newCube));
	return true;
}

void SubCubeList::push_back(CPCubeArea area)
{
	SubCubeListBase::push_back(make_pair(getRank(*area.get()), area));
}

void SubCubeList::push_back(pair<uint8_t, CPCubeArea> rankedArea)
{
	SubCubeListBase::push_back(rankedArea);
}

double SubCubeList::cellCount()
{
	double cells = 0;
	for (iterator lit = begin(); lit != end(); ++lit) {
		cells += lit->second->getSize();
	}
	return cells;
}

uint8_t SubCubeList::getRank(const CubeArea &area)
{
	uint8_t rank = 0;
	for (size_t i = 0; i < area.dimCount(); i++) {
		if (area.elemCount(i) > 1) {
			rank++;
		}
	}
	return rank;
}

ostream& operator<<(ostream& ostr, const SubCubeList& cubeArea)
{
	for (SubCubeList::const_iterator lit = cubeArea.begin(); lit != cubeArea.end(); ++lit) {
		ostr << *(lit->second);
		ostr << endl;
	}
	return ostr;
}

}
