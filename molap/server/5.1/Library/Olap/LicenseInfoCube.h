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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_LICENSE_INFO_CUBE_H
#define OLAP_LICENSE_INFO_CUBE_H 1

#include "palo.h"

#include "Olap/SystemCube.h"
#include "Olap/Server.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief server log OLAP cube
///
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS LicenseInfoProcessor : public ProcessorBase {
public:
	LicenseInfoProcessor(CPArea filter, PLicIter licIter) : ProcessorBase(true, PEngineBase()), filter(filter), i(0), licIter(licIter), licCont(false) {}
	virtual ~LicenseInfoProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const {return key;}
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset() {}
private:
	bool nextLicense();
	CPArea filter;
	CellValue value;
	IdentifiersType key;
	Set::Iterator propertyIt;
	size_t i;
	PLicIter licIter;
	bool licCont;
};

class SERVER_CLASS LicenseInfoCube : public SystemCube {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates empty cube
	///
	/// @throws ParameterException on double or missing dimensions
	////////////////////////////////////////////////////////////////////////////////

	LicenseInfoCube(PDatabase db, const string& name, const IdentifiersType* dimensions);

	LicenseInfoCube(const LicenseInfoCube &c) : SystemCube(c) {}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if cube can have worker
	////////////////////////////////////////////////////////////////////////////////

	virtual bool supportsWorker() const {return false;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief changes cube path
	////////////////////////////////////////////////////////////////////////////////

	virtual void setCubeFileAndLoad(PServer server, PDatabase db, const FileName& fileName, bool newid) {}

	////////////////////////////////////////////////////////////////////////////////
	/// @{
	/// @name functions dealing with cells and cell values
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a value to a cell
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus setCellValue(PServer server, PDatabase db, PCubeArea cellPath, CellValue value, PLockedCells lockedCells, PUser user, boost::shared_ptr<PaloSession> session, bool checkArea, bool addValue, SplashMode splashMode, bool bWriteToJournal, User::RightSetting* checkRights, set<PCube> &changedCubes, bool possibleCommit, CubeArea::CellType ct);

	////////////////////////////////////////////////////////////////////////////////
	/// @}
	////////////////////////////////////////////////////////////////////////////////

	virtual PCommitable copy() const;
	virtual void saveCube(PServer server, PDatabase db) {}
	virtual PProcessorBase evaluatePlan(PPlanNode plan, EngineBase::Type engineType, bool sortedOutput) const;
private:
};

}

#endif // OLAP_LICENSE_INFO_CUBE_H
