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

#include <math.h>
#include <limits>

#if defined(_MSC_VER)
#include <float.h>
#include <limits>
#endif

#include "Olap/RollbackStorage.h"
#include "Olap/Rule.h"
#include "Olap/Cube.h"
#include "Engine/EngineBase.h"

#include "Logger/Logger.h"
#include "InputOutput/Statistics.h"
#include "InputOutput/FileReader.h"
#include "InputOutput/FileWriter.h"

#include "Thread/WriteLocker.h"

#include <iostream>

namespace palo {

size_t RollbackStorage::maximumFileRollbackSize = 100 * 1024 * 1024;
size_t RollbackStorage::maximumMemoryRollbackSize = 10 * 1024 * 1024;

size_t RollbackStorage::getSize(const IdentifiersType &path, const CellValue &val)
{
	return (path.size() * sizeof(IdentifierType)) + sizeof(CellValue) + (val.isString() ? val.length() : 0);
}

RollbackStorage::RollbackStorage(size_t dimCount, PFileName cubeFileName, IdentifierType id) :
	numPages(1), sizeSavedPages(0), currentPageSize(0)
{
	fileName.reset(new FileName(cubeFileName->path, cubeFileName->name + "_lock_" + StringUtils::convertToString(id), cubeFileName->extension));
}

RollbackStorage::~RollbackStorage()
{
	for (size_t num = 0; num < numPages; num++) {
		FileName pageFileName = computePageFileName(num);
		FileUtils::remove(pageFileName);
	}
}

void RollbackStorage::checkCurrentPage()
{
	if (currentPageSize >= maximumMemoryRollbackSize) {
		// save curentPage to disk and delete it from memory
		savePageToFile();
		currentPage.clear();
		currentPageSize = 0;
		numPages++;
	}
}

void RollbackStorage::addCellValue(const vector<pair<IdentifiersType, CellValue> > &cells)
{
	WriteLocker locker(&lock);
	if (!hasCapacity((double)cells.size())) {
		throw ErrorException(ErrorException::ERROR_CUBE_LOCK_NO_CAPACITY, "rollback size is exceeded");
	}
	for (vector<pair<IdentifiersType, CellValue> >::const_iterator it = cells.begin(); it != cells.end(); ++it) {
		checkCurrentPage();
		currentPage.push_back(make_pair(it->first, it->second));
		currentPageSize += getSize(it->first, it->second);
	}
	steps.push(cells.size());
}

void RollbackStorage::rollback(PServer server, PDatabase db, PCube cube, size_t numSteps, PUser user)
{
	WriteLocker locker(&lock);
	if (steps.size() < numSteps) {
		numSteps = steps.size();
	}

	vector<pair<IdentifiersType, CellValue> >::reverse_iterator rit = currentPage.rbegin();
	set<PCube> changedCubes;
	for (size_t i = 0; i < numSteps; ++i) {
		size_t subSteps = steps.top();
		steps.pop();
		for (size_t j = 0; j < subSteps; ++j) {
			if (rit == currentPage.rend()) {
				numPages--;
				loadPageFromFile();
				rit = currentPage.rbegin();
			}
			cube->setCellValue(server, db, PCubeArea(new CubeArea(db, cube, rit->first)), rit->second, PLockedCells(), PUser(), boost::shared_ptr<PaloSession>(), false, false, DEFAULT, true, 0, changedCubes, true, CubeArea::NONE);
			currentPageSize -= getSize(rit->first, rit->second);
			currentPage.pop_back();
			rit = currentPage.rbegin();
		}
	}
	cube->commitChanges(false, user, changedCubes, false);
}

FileName RollbackStorage::computePageFileName(size_t identifier)
{
	return FileName(fileName->path, fileName->name + "_" + StringUtils::convertToString((uint32_t)identifier), fileName->extension);
}

void RollbackStorage::savePageToFile()
{
	FileName pageFileName = computePageFileName(numPages - 1);

	// open a new temp-cube file
	boost::shared_ptr<FileWriter> fw(FileWriter::getFileWriter(FileName(pageFileName, "csv")));

	fw->openFile();

	fw->appendComment("PALO ROLLBACK PAGE DATA");
	fw->appendComment("");

	fw->appendComment("Description of data: ");
	fw->appendComment("PATH;TYPE;VALUE ");
	fw->appendSection("VALUES");

	for (vector<pair<IdentifiersType, CellValue> >::iterator it = currentPage.begin(); it != currentPage.end(); ++it) {
		fw->appendIdentifiers(it->first.begin(), it->first.end());
		fw->appendInteger(it->second.isString() ? Element::STRING : Element::NUMERIC);
		if (it->second.isString()) {
			fw->appendEscapeString(it->second);
		} else {
			if (it->second.isEmpty()) {
				fw->appendString("");
			} else {
				fw->appendDouble(it->second.getNumeric());
			}
		}
		fw->nextLine();
	}

	// that's it
	fw->appendComment("");
	fw->appendComment("PALO CUBE DATA END");

	fw->closeFile();

	size_t pageSize = FileWriter::getFileSize(pageFileName);
	sizeSavedPages += pageSize;
	pageSizes.push_back(pageSize);

	Logger::debug << "rollback page '" << numPages - 1 << "' saved (sizeSavedPages = " << sizeSavedPages << ")" << endl;

}

void RollbackStorage::loadPageFromFile()
{
	currentPage.clear();
	currentPageSize = 0;

	FileName pageFileName = computePageFileName(numPages - 1);

	sizeSavedPages -= FileWriter::getFileSize(pageFileName);

	{
		boost::shared_ptr<FileReader> fr(FileReader::getFileReader(pageFileName));
		fr->openFile(true, false);

		if (fr->isSectionLine() && fr->getSection() == "VALUES") {
			fr->nextLine();

			while (fr->isDataLine()) {
				IdentifiersType path = fr->getDataIdentifiers(0);
				int type = fr->getDataInteger(1);
				string value = fr->getDataString(2);

				CellValue val;
				if (type == 1) {
					// integer
					char *p;
					double d = strtod(value.c_str(), &p);

					if (*p == '\0') {
						val = d;
					}
				} else {
					// string
					val = value;
				}

				currentPage.push_back(make_pair(path, val));
				currentPageSize += getSize(path, val);
				// get next line of saved data
				fr->nextLine();
			}
		}
	}

	FileUtils::remove(pageFileName);
	pageSizes.pop_back();

	Logger::debug << "rollback page '" << numPages - 1 << "' loaded (sizeSavedPages = " << sizeSavedPages << ")" << endl;
}

bool RollbackStorage::hasCapacity(double num)
{
	if (currentPage.empty()) {
		return true;
	} else {
		size_t rowSize = currentPage[0].first.size() * sizeof(IdentifierType) + sizeof(CellValue);
		double max = (maximumFileRollbackSize + maximumMemoryRollbackSize) * 1.0;
		double used = (sizeSavedPages + currentPageSize) * 1.0;
		double needed = num * rowSize;
		return max > used + needed;
	}
}

}
