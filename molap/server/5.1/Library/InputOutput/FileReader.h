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

#ifndef INPUT_OUTPUT_FILE_READER_H
#define INPUT_OUTPUT_FILE_READER_H 1

#include "palo.h"

#include "InputOutput/FileUtils.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief read a data file
///
/// The file reader reads data from a CSV file.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FileReader {
public:
	FileReader(const FileName& fileName) : fileName(fileName) {}
	virtual ~FileReader();
	virtual bool openFile(bool throwError, bool skipMessage) = 0;
	virtual bool isDataLine() const = 0;
	virtual bool isSectionLine() const = 0;
	virtual bool isEndOfFile() const = 0;
	virtual const string& getSection() const = 0;
	virtual size_t getDataSize() const = 0;
	virtual const string& getDataString(int num) const = 0;
	virtual long getDataInteger(int num, int defaultValue = 0) const = 0;
	virtual double getDataDouble(int num) const = 0;
	virtual bool getDataBool(int num, bool defaultValue = false) const = 0;
	virtual const vector<string> getDataStrings(int num, char separator) = 0;
	virtual const vector<int> getDataIntegers(int num) = 0;
	virtual const IdentifiersType getDataIdentifiers(int num) = 0;
	virtual PArea getDataArea(int num, size_t dimCount) = 0;
	virtual PPaths getDataPaths(int num) = 0;
	virtual const vector<double> getDataDoubles(int num) = 0;
	virtual void getTimeStamp(long *seconds, long *useconds, int num) = 0;
	virtual void nextLine(bool strip = false) = 0;
	virtual int32_t getLineNumber() = 0;
	virtual void getRaw(char *p, streamsize size) = 0;

	FileName getFileName() {
		return fileName;
	}

	static FileReader *getFileReader(const FileName& fileName);

protected:
	FileName fileName;
};

}

#endif
