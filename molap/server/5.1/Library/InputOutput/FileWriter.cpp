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

#include "InputOutput/FileWriter.h"
#include "InputOutput/FileWriterTXT.h"
#include "InputOutput/FileWriterBF.h"
#include "Exceptions/FileOpenException.h"
#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"

#ifndef _MSC_VER
#include <sys/file.h>
#endif

namespace palo {

FileWriter::~FileWriter()
{
}

void FileWriter::deleteFile(const FileName& fileName)
{
	if (!FileUtils::remove(fileName)) {
		throw FileOpenException("could not delete file", fileName.fullPath());
	}
}

int32_t FileWriter::getFileSize(const FileName& fileName)
{
	struct stat fileStat;
	int result = stat(fileName.fullPath().c_str(), &fileStat);

	if (result < 0) {
		return 0;
	}

	return (int32_t)fileStat.st_size;
}

FileWriter *FileWriter::getFileWriter(const FileName& fileName)
{
	FileWriter *ret;
	if (bffilebuf::canCrypt()) {
		ret = new FileWriterBF(fileName);
	} else {
		ret = new FileWriterTXT(fileName);
	}
	return ret;
}

DirLock::DirLock(const string &dirName)
{
#if defined(_MSC_VER)
	fileName = dirName + "\\palo.lock";
	fd = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (fd == INVALID_HANDLE_VALUE) {
		Logger::error << "Directory is already used by another palo server process." << endl;
		throw FileOpenException("couldn't lock directory", dirName);
	}
#else
	fileName = dirName + "/palo.lock";
	fd = open(fileName.c_str(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) {
		Logger::error << "Directory is already used by another palo server process." << endl;
		throw FileOpenException("couldn't lock directory", dirName);
	}
	if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		Logger::error << "Directory is already used by another palo server process." << endl;
		throw FileOpenException("couldn't lock directory", dirName);
	}
#endif
}

DirLock::~DirLock()
{
#if defined(_MSC_VER)
	if (fd != INVALID_HANDLE_VALUE) {
		CloseHandle(fd);
	}
#else
	if (fd != -1) {
		close(fd);
	}
#endif
	remove(fileName.c_str());
}

}
