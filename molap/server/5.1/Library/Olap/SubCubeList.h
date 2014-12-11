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

#ifndef PALO_SUBCUBE_LIST_H
#define PALO_SUBCUBE_LIST_H 1

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief SubCubeList
///
/// List of subcubes
////////////////////////////////////////////////////////////////////////////////

typedef list<pair<uint8_t, CPCubeArea> > SubCubeListBase;

class SubCubeList : public SubCubeListBase {
public:
	typedef SubCubeListBase::iterator iterator;
	typedef SubCubeListBase::const_iterator const_iterator;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	SubCubeList() : SubCubeListBase() {}
	SubCubeList(PCubeArea &area);
	SubCubeList(const SubCubeList &l);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds a subcube containing one cell to the list
	////////////////////////////////////////////////////////////////////////////////

	void insertAndMerge(CPCubeArea area);
	void insertAndMerge(pair<uint8_t, CPCubeArea> rankedArea);
	void push_back(CPCubeArea area);
	void push_back(pair<uint8_t, CPCubeArea> rankedArea);
	double cellCount();

private:
	bool joinSubCubes(const CubeArea &area1, const CubeArea &area2);
	uint8_t getRank(const CubeArea &area);
};

ostream& operator<<(ostream& ostr, const SubCubeList& cubeArea);

}

#endif
