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
 * \author Tobias Lauer, Jedox AG, Freiburg, Germany
 * 
 *
 */

#ifndef PALO_JOBS_AREA_JOB_H
#define PALO_JOBS_AREA_JOB_H 1

#include "palo.h"

#include "Exceptions/ErrorException.h"
#include "Olap/Cube.h"
#include "PaloDispatcher/DirectPaloJob.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief area
////////////////////////////////////////////////////////////////////////////////


class SERVER_CLASS AreaJob : public DirectPaloJob {
public:
	static size_t s_max_cell_count;
	static const uint64_t UNLIMITED_COUNT = (uint64_t)-1;

	static void setMaxCellCount(size_t size) {
		s_max_cell_count = size;
	}

public:

	static const int MAX_AREA_SIZE = 350000;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	AreaJob(PaloJobRequest* jobRequest) :
		DirectPaloJob(jobRequest), max_cell_count(s_max_cell_count), isNoPermission(false) {
	}

	PCellStream getCellPropsStream(CPDatabase db, CPCube cube, CPCubeArea area, const IdentifiersType &properties);
	void fillProps(vector<CellValue> &result, const IdentifiersType &key, PCellStream &propStream, const IdentifiersType &properties, RightsType right);
	static double fillEmptyDim(vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, vector<IdentifiersType> &area, PCube &cube, PDatabase &database, PUser &user);
	static PCubeArea checkRights(vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, CPArea area, bool* hasStringElem, PCube &cube, PDatabase &database, PUser &user, bool reduceCalcArea, PArea &noPermission, bool &isNoPermission, vector<CPDimension> &dims);

protected:
	size_t max_cell_count;
	PArea noPermission;
	bool isNoPermission;
	vector<CPDimension> dims;

	virtual void appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals);

	bool loop(CPArea area, PCubeArea calcArea, PCellStream cs, uint64_t *maxCount, PCellStream props, vector<User::RoleDbCubeRight> &vRights, IdentifiersType *lastKey, bool emptyValues);
	virtual bool checkCondition(const CellValue &value) const {return true;}
	bool isReadable(PCubeArea area, User::RightSetting& rs, bool *defaultUsed) const;

private:
	static bool checkElement(CPDimension &dim, Element *e, vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, PDatabase &database, PUser &user);
	void generateMissingValues(Area::PathIterator &curr, const Area::PathIterator &end, const IdentifiersType *newKey, PCellStream props, bool emptyValues, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount);
	void generateMissingValues_intern(Area::PathIterator &curr, PCellStream props, bool emptyValues, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount);
	void generateValue(const IdentifiersType &key, const CellValue value, PCellStream props, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount);
	bool generateError(Area::PathIterator &curr, uint64_t &freeCount, PArea &restriction, ErrorException::ErrorType error);
	void insertProperties(vector<CellValue> &result, PCellStream &propStream, const IdentifiersType &key);
	bool keyCompareProp(const IdentifiersType &key, const IdentifiersType &keyProp);

	map<IdentifierType, vector<size_t> > propPositions;
	bool firstProp;
	bool propsFinish;
};

}
#endif
