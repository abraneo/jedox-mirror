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
 * \author Martin Dolezal, qBicon, Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef INPUT_OUTPUT_STRING_VECTOR_READER_H
#define INPUT_OUTPUT_STRING_VECTOR_READER_H 1

#include "palo.h"

#include "InputOutput/FileReaderTXT.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief journal file reader
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS StringVectorReader : public FileReaderTXT {

public:
	StringVectorReader(vector<vector<string>> &commands, const FileName &fn) : FileReaderTXT(fn) {this->commands = commands; pos = 0;}

	virtual bool openFile(bool throwError, bool skipMessage) {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}
	virtual bool isDataLine() const {return !isEndOfFile();}
	virtual bool isSectionLine() const {return false;}
	virtual bool isEndOfFile() const {return pos >= commands.size();}
	virtual const string& getSection() const {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}
	virtual size_t getDataSize() const {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}

	virtual const string& getDataString(int num) const {
		if ((size_t)num >= commands[pos].size()) {
			Logger::error << "invalid field " << num << " asked for command " << commands[pos][0] << endl;
			static const string empty;
			return empty;
		} else {
			return commands[pos][num];
		}
	}

	virtual void nextLine(bool strip = false) {pos++;}
	virtual int32_t getLineNumber() {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}
	virtual void getRaw(char *p, streamsize size) {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}

	FileName getFileName() {throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid method called for StringVectorReader");}

protected:
	vector<vector<string> > commands;
	size_t pos;
};

}

#endif
