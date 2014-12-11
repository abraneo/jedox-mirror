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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef INPUT_OUTPUT_FILE_WRITER_H
#define INPUT_OUTPUT_FILE_WRITER_H 1

#include "palo.h"

namespace palo {

class SERVER_CLASS FileWriter {
public:
	FileWriter(const FileName& fileName) : fileName(fileName) {}
	virtual ~FileWriter();
	virtual void openFile(bool append = false) = 0;
	virtual void closeFile() = 0;
	virtual void appendComment(const string& value) = 0;
	virtual void appendSection(const string& value) = 0;
	virtual void appendString(const string& value, char terminator = ';') = 0;
	virtual void appendEscapeString(const string& value, char terminator = ';') = 0;
	virtual void appendEscapeStrings(const vector<string>* value) = 0;
	virtual void appendInteger(const int32_t value) = 0;
	virtual void appendIntegers(const vector<int32_t>* value) = 0;
	virtual void appendIdentifier(const IdentifierType value, char terminator = ';') = 0;

	virtual void appendSet(const Set &value) = 0;
	virtual void appendAreaCompact(CPArea value, CPArea parent) = 0;
	virtual void appendPlan(CPPlanNode plan, CPArea parentArea=CPArea()) = 0;
	virtual void appendAggregationMaps(CPAggregationMaps aggrMaps) = 0;

	template<typename Iter> void appendIdentifiers(Iter begin, Iter end, char separator = ',', char terminator = ';') {
		bool first = true;
		for (; begin != end; ++begin) {
			appendArray(first, (IdentifierType)*begin, separator);
			first = false;
		}
		appendArrayFinish(terminator);
	}

	virtual void appendArea(CPArea value) = 0;
	virtual void appendArea(vector<IdentifiersType> &value, bool emptyAll) = 0;
	virtual void appendPaths(CPPaths value) = 0;
	virtual void appendDouble(const double value, char terminator = ';') = 0;
	virtual void appendDoubles(const vector<double>* value) = 0;
	virtual void appendBool(const bool value) = 0;
	virtual void appendTimeStamp() = 0;
	virtual void appendTimeStamp(timeval &tv) = 0;
	virtual void nextLine() = 0;
	virtual void appendRaw(const string& value) = 0;
	virtual void appendRaw(const char *p, streamsize size) = 0;
	static void deleteFile(const FileName& fileName);
	static int32_t getFileSize(const FileName& fileName);
	static FileWriter *getFileWriter(const FileName& fileName);

protected:
	virtual void appendArray(bool bFirst, IdentifierType value, char separator) = 0;
	virtual void appendArrayFinish(char terminator) = 0;

	FileName fileName;
};

class SERVER_CLASS DirLock {
public:
	DirLock(const string &dirName);
	~DirLock();
private:
#if defined(_MSC_VER)
	HANDLE fd;
#else
	int fd;
#endif
	string fileName;
};

}

#endif
