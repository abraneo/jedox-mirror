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

#include "InputOutput/FileReaderBF.h"
#include "Logger/Logger.h"
#include "Exceptions/FileOpenException.h"

namespace palo {

FileReaderBF::FileReaderBF(const FileName& fileName)
	: FileReaderTXT(fileName), bf(0)
{
}

FileReaderBF::~FileReaderBF()
{
	if (inputFile != 0) {
		delete inputFile;
		inputFile = 0;
	}
	if (bf) {
		delete bf;
	}
}

bool FileReaderBF::openFile(bool throwError, bool skipMessage)
{
	bf = new bffilebuf(fileName, ios::in);
	if (!bf || !bf->is_open()) {
		if (throwError) {
			Logger::warning << "could not read from file '" << fileName.fullPath() << "'" << " (" << strerror(errno) << ")" << endl;
			throw FileOpenException("could not open file for reading", fileName.fullPath());
		} else {
			if (!skipMessage) {
				Logger::debug << "file not found: '" << fileName.fullPath() << "'" << " (" << strerror(errno) << ")" << endl;
			}
			return false;
		}
	}
	inputFile = new istream(bf);
	lineNumber = 0;
	nextLine();

	return true;
}

}
