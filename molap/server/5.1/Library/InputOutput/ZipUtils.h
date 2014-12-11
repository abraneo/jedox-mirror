////////////////////////////////////////////////////////////////////////////////
/// @brief collections of zip file functions
///
/// @file
///
/// Copyright (C) 2013-2013 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// You may obtain a copy of the License at
///
/// <a href="http://www.jedox.com/license_palo_suite.txt">
///   http://www.jedox.com/license_palo_suite.txt
/// </a>
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// Portions of the code developed by triagens GmbH, Koeln on behalf of Jedox
/// AG. Intellectual property rights for these portions has triagens GmbH,
/// Koeln, or othervise Jedox AG, Freiburg. Exclusive worldwide exploitation
/// right (commercial copyright) has Jedox AG, Freiburg.
///
/// @author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
////////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_OUTPUT_ZIP_UTILS_H
#define INPUT_OUTPUT_ZIP_UTILS_H 1

#include "palo.h"
#include <fstream>

#include "zip/zip.h"
#include "zip/unzip.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief collections of file functions
////////////////////////////////////////////////////////////////////////////////

class ZipUtils {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief compresses a directory to zip
	////////////////////////////////////////////////////////////////////////////////
	static bool zipDirectory(std::string fromDir, std::string toZipFile, bool append);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief add file to zip
	////////////////////////////////////////////////////////////////////////////////
	static bool addToZip(zipFile zf, string fileName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extract zip to folder
	////////////////////////////////////////////////////////////////////////////////
	static void extractZip(string zipFileName, string targetFolder, bool dbExtract, string newDbName, string *oldDbName);

	static void checkZipDbValidity(string zipFileName);
	static void checkZipDbValidity(unzFile uf);
	static void checkBackupFileValidity(unzFile uf, string &dbName, bool &databaseCsv);
};

}
#endif
