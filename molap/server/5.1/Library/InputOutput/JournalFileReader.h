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
 * 
 *
 */

#ifndef INPUT_OUTPUT_JOURNAL_FILE_READER_H
#define INPUT_OUTPUT_JOURNAL_FILE_READER_H 1

#include "palo.h"

#include "InputOutput/FileReader.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief journal file reader
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS JournalFileReader : public FileReader {

public:
	// cube log
	static const string JOURNAL_CELL_CLEAR;
	static const string JOURNAL_CELL_COPY;
	static const string JOURNAL_CELL_GOALSEEK;
	static const string JOURNAL_CELL_REPLACE_DOUBLE;
	static const string JOURNAL_CELL_REPLACE_STRING;
	static const string JOURNAL_CUBE_AREA_CLEAR;
	static const string JOURNAL_CUBE_CLEAR;
	static const string JOURNAL_RULE_CREATE;
	static const string JOURNAL_RULE_DESTROY;
	static const string JOURNAL_RULE_MODIFY;
	static const string JOURNAL_RULE_ACTIVATE;
	static const string JOURNAL_RULE_MOVE;
	static const string JOURNAL_CELL_REPLACE_BULK_START;
	static const string JOURNAL_CELL_REPLACE_BULK_STOP;

	// database log
	static const string JOURNAL_CUBE_CONVERT;
	static const string JOURNAL_CUBE_CREATE;
	static const string JOURNAL_CUBE_DESTROY;
	static const string JOURNAL_CUBE_RENAME;
	static const string JOURNAL_DIMENSION_CLEAR;
	static const string JOURNAL_DIMENSION_CREATE;
	static const string JOURNAL_DIMENSION_DESTROY;
	static const string JOURNAL_DIMENSION_RENAME;
	static const string JOURNAL_ELEMENT_APPEND;
	static const string JOURNAL_ELEMENT_CREATE;
	static const string JOURNAL_ELEMENT_DESTROY;
	static const string JOURNAL_ELEMENT_MOVE;
	static const string JOURNAL_ELEMENT_MOVE_BULK;
	static const string JOURNAL_ELEMENT_REMOVE_CHILDREN_NOT_IN;
	static const string JOURNAL_ELEMENT_REMOVE_CHILDREN;
	static const string JOURNAL_ELEMENT_RENAME;
	static const string JOURNAL_ELEMENT_REPLACE;
	static const string JOURNAL_ELEMENTS_BULK_START;
	static const string JOURNAL_ELEMENTS_BULK_STOP;

	// common
	static const string JOURNAL_VERSION;

	struct Version {
		int release;
		int sr;
		int build;

		Version() : release(0), sr(0), build(0) {}
		Version(int release, int sr, int build) : release(release), sr(sr), build(build) {}

		bool isUnknown() const {
			return release == 0;
		}

		bool isOld() const {
			return (*this) < minVersion;
		}

		bool operator< (const Version &v) const;
	};

	static const Version minVersion;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new journal file reader
	////////////////////////////////////////////////////////////////////////////////

	JournalFileReader(const FileName& fileName);
	JournalFileReader(boost::shared_ptr<FileReader> stringVectorReader, const FileName &fileName) : FileReader(fileName) {this->reader = stringVectorReader;}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief opens file
	////////////////////////////////////////////////////////////////////////////////

	virtual bool openFile(bool throwError, bool skipMessage);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief reads next line
	////////////////////////////////////////////////////////////////////////////////

	virtual void nextLine(bool strip = false);

	virtual bool isDataLine() const {return reader->isDataLine();}
	virtual bool isSectionLine() const {return reader->isSectionLine();}
	virtual bool isEndOfFile() const {return reader->isEndOfFile();}
	virtual const string& getSection() const {return reader->getSection();}
	virtual size_t getDataSize() const {return reader->getDataSize();}
	virtual const string& getDataString(int num) const {return reader->getDataString(num);}
	virtual long getDataInteger(int num, int defaultValue = 0) const {return reader->getDataInteger(num, defaultValue);}
	virtual double getDataDouble(int num) const {return reader->getDataDouble(num);}
	virtual bool getDataBool(int num, bool defaultValue = false) const {return reader->getDataBool(num, defaultValue);}
	virtual const vector<string> getDataStrings(int num, char separator) {return reader->getDataStrings(num, separator);}
	virtual const vector<int> getDataIntegers(int num) {return reader->getDataIntegers(num);}
	virtual const IdentifiersType getDataIdentifiers(int num) {return reader->getDataIdentifiers(num);}
	virtual PArea getDataArea(int num, size_t dimCount) {return reader->getDataArea(num, dimCount);}
	virtual PPaths getDataPaths(int num) {return reader->getDataPaths(num);}
	virtual const vector<double> getDataDoubles(int num) {return reader->getDataDoubles(num);}
	virtual void getTimeStamp(long *seconds, long *useconds, int num) {return reader->getTimeStamp(seconds, useconds, num);}
	virtual int32_t getLineNumber() {return reader->getLineNumber();}
	virtual void getRaw(char *p, streamsize size);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gotos time stamp
	////////////////////////////////////////////////////////////////////////////////

	void gotoTimeStamp(long int seconds, long int useconds);

	const Version& getVersion() const {
		return version;
	}

	void setVersion(int release, int sr, int build) {
		version.release = release;
		version.sr = sr;
		version.build = build;
	}

private:
	int lastFileId;
	Version version;
	boost::shared_ptr<FileReader> reader;
};

}

#endif
