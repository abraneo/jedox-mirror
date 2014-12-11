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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_ROLLBACK_STORAGE_H
#define OLAP_ROLLBACK_STORAGE_H 1

#include "palo.h"

#include <stack>

#include "Exceptions/ErrorException.h"

#include "Logger/Logger.h"

#include "Olap/Cube.h"
#include "Olap/Dimension.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief palo rollback storage
///
/// A rollback storage uses a rollback page
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RollbackStorage {

public:

	static void setMaximumMemoryRollbackSize(size_t maximum) {
		maximumMemoryRollbackSize = maximum;
	}

	static void setMaximumFileRollbackSize(size_t maximum) {
		maximumFileRollbackSize = maximum;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Creates a filled cache storage
	////////////////////////////////////////////////////////////////////////////////

	RollbackStorage(size_t dimCount, PFileName cubeFileName, IdentifierType id);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief Remove a cache storage
	////////////////////////////////////////////////////////////////////////////////

	~RollbackStorage();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get number of rollback steps
	////////////////////////////////////////////////////////////////////////////////

	size_t getNumberSteps() {
		WriteLocker locker(&lock);
		return steps.size();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief add a rollback step
	////////////////////////////////////////////////////////////////////////////////
	//void addRollbackStep();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief rollback steps
	/// this removes the related values from the storage
	////////////////////////////////////////////////////////////////////////////////
	void rollback(PServer server, PDatabase db, PCube cube, size_t numSteps, PUser user);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief add a value to the rollback storage
	////////////////////////////////////////////////////////////////////////////////
	void addCellValue(const vector<pair<IdentifiersType, CellValue> > &cells);

private:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if the rollback storage can hold num more values
	////////////////////////////////////////////////////////////////////////////////

	bool hasCapacity(double num);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief check if the current page can hold one more element
	/// this method saves full pages to file and creates new pages
	////////////////////////////////////////////////////////////////////////////////

	void checkCurrentPage();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a file name for a rollback page
	////////////////////////////////////////////////////////////////////////////////

	FileName computePageFileName(size_t identifier);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief save a rollback page to file
	////////////////////////////////////////////////////////////////////////////////

	void savePageToFile();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief create a new rollback page and fill the page from file
	////////////////////////////////////////////////////////////////////////////////

	void loadPageFromFile();

	static size_t getSize(const IdentifiersType &path, const CellValue &val);

private:
	size_t numPages;
	PFileName fileName;
	size_t sizeSavedPages;
	vector<size_t> pageSizes;
	size_t currentPageSize;
	stack<size_t> steps;
	vector<pair<IdentifiersType, CellValue> > currentPage;
	Mutex lock;

private:
	static size_t maximumFileRollbackSize;
	static size_t maximumMemoryRollbackSize;
};

}

#endif
