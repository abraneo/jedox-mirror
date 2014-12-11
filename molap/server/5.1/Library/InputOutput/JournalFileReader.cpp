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

#include "InputOutput/JournalFileReader.h"

#include "Exceptions/FileOpenException.h"

#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"

#include <iostream>

namespace palo {

// cube log
const string JournalFileReader::JOURNAL_CELL_CLEAR = "CLEAR_CELL"; // obsolete, use JOURNAL_CELL_REPLACE
const string JournalFileReader::JOURNAL_CELL_COPY = "COPY_VALUES";
const string JournalFileReader::JOURNAL_CELL_GOALSEEK = "CELL_GOALSEEK";
const string JournalFileReader::JOURNAL_CELL_REPLACE_DOUBLE = "SET_DOUBLE";
const string JournalFileReader::JOURNAL_CELL_REPLACE_STRING = "SET_STRING";
const string JournalFileReader::JOURNAL_CUBE_AREA_CLEAR = "CLEAR_AREA";
const string JournalFileReader::JOURNAL_CUBE_CLEAR = "CLEAR_CELLS";
const string JournalFileReader::JOURNAL_RULE_CREATE = "CREATE_RULE";
const string JournalFileReader::JOURNAL_RULE_DESTROY = "DESTROY_RULE";
const string JournalFileReader::JOURNAL_RULE_MODIFY = "MODIFY_RULE";
const string JournalFileReader::JOURNAL_RULE_ACTIVATE = "ACTIVATE_RULE";
const string JournalFileReader::JOURNAL_RULE_MOVE = "MOVE_RULE";
const string JournalFileReader::JOURNAL_CELL_REPLACE_BULK_START = "BULK_START";
const string JournalFileReader::JOURNAL_CELL_REPLACE_BULK_STOP = "BULK_STOP";

// database log
const string JournalFileReader::JOURNAL_CUBE_CONVERT = "CONVERT_CUBE";
const string JournalFileReader::JOURNAL_CUBE_CREATE = "ADD_CUBE";
const string JournalFileReader::JOURNAL_CUBE_DESTROY = "DELETE_CUBE";
const string JournalFileReader::JOURNAL_CUBE_RENAME = "RENAME_CUBE";
const string JournalFileReader::JOURNAL_DIMENSION_CLEAR = "CLEAR_ELEMENTS";
const string JournalFileReader::JOURNAL_DIMENSION_CREATE = "ADD_DIMENSION";
const string JournalFileReader::JOURNAL_DIMENSION_DESTROY = "DELETE_DIMENSION";
const string JournalFileReader::JOURNAL_DIMENSION_RENAME = "RENAME_DIMENSION";
const string JournalFileReader::JOURNAL_ELEMENT_APPEND = "ADD_CHILDREN";
const string JournalFileReader::JOURNAL_ELEMENT_CREATE = "ADD_ELEMENT";
const string JournalFileReader::JOURNAL_ELEMENT_DESTROY = "DELETE_ELEMENTS";
const string JournalFileReader::JOURNAL_ELEMENT_MOVE = "MOVE_ELEMENT";
const string JournalFileReader::JOURNAL_ELEMENT_MOVE_BULK = "MOVE_ELEMENT_BULK";
const string JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN_NOT_IN = "REMOVE_CHILDREN_NOT_INT";
const string JournalFileReader::JOURNAL_ELEMENT_REMOVE_CHILDREN = "REMOVE_CHILDREN";
const string JournalFileReader::JOURNAL_ELEMENT_RENAME = "RENAME_ELEMENT";
const string JournalFileReader::JOURNAL_ELEMENT_REPLACE = "CHANGE_ELEMENT";
const string JournalFileReader::JOURNAL_ELEMENTS_BULK_START = "ELEMENTS_BULK_START";
const string JournalFileReader::JOURNAL_ELEMENTS_BULK_STOP = "ELEMENTS_BULK_STOP";

// common
const string JournalFileReader::JOURNAL_VERSION = "VERSION";

// minimal version required
const JournalFileReader::Version JournalFileReader::minVersion = JournalFileReader::Version(5, 1, 5594);


JournalFileReader::JournalFileReader(const FileName& fileName) :
	FileReader(fileName)
{
	lastFileId = 0;
}

bool JournalFileReader::openFile(bool throwError, bool skipMessage)
{
	// find first journal file
	lastFileId = 0;

	stringstream se;
	se << fileName.name << "_" << lastFileId;

	reader.reset(FileReader::getFileReader(FileName(fileName.path, se.str(), fileName.extension)));

	return reader->openFile(throwError, skipMessage);
}

void JournalFileReader::nextLine(bool strip)
{
	if (!reader->isEndOfFile()) {
		reader->nextLine(strip);

		if (reader->isEndOfFile()) {

			// check next journal file
			stringstream filename;
			filename << fileName.name << "_" << (lastFileId + 1);

			FileName jf(fileName.path, filename.str(), fileName.extension);

			if (FileUtils::isReadable(jf)) {
				string fn = jf.fullPath();

				// next journal file found
				lastFileId++;
				reader.reset(FileReader::getFileReader(jf));
				reader->openFile(true, false);
			}
		}
	}
}

void JournalFileReader::gotoTimeStamp(long int seconds, long int useconds)
{
	while (isDataLine()) {
		long int s, u;
		getTimeStamp(&s, &u, 0);

		if (s > seconds) {
			return;
		} else if (s == seconds && u >= useconds) {
			return;
		}

		nextLine();
	}
}

void JournalFileReader::getRaw(char *p, streamsize size)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "JournalFileReader::getRaw not implemented");
}

bool JournalFileReader::Version::operator< (const Version &v) const
{
	if (release < v.release) {
		return true;
	} else if (release > v.release) {
		return false;
	}

	if (sr < v.sr) {
		return true;
	} else if (sr > v.sr) {
		return false;
	}

	if (build < v.build) {
		return true;
	} else {
		return false;
	}
}


}
