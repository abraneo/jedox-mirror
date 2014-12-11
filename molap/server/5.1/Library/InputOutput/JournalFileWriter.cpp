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
 * 
 *
 */

#include <boost/scoped_array.hpp>

#include "InputOutput/JournalFileWriter.h"

#include "Collections/StringUtils.h"

#include "Exceptions/FileOpenException.h"

#include "InputOutput/JournalFileReader.h"
#include "InputOutput/FileReader.h"
#include "InputOutput/FileUtils.h"
#include "InputOutput/FileWriterBF.h"
#include "Logger/Logger.h"
#include "Olap/Context.h"
#include "Thread/WriteLocker.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

#ifdef _MSC_VER
#pragma warning(disable : 4355)
#endif

extern "C" {
#include <time.h>
}

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <iostream>

namespace palo {

ContextBuffer::int_type ContextBuffer::overflow(ContextBuffer::int_type c)
{
	buffer[sizeof(buffer) - 1] = (char)c;
	Context::getContext()->getJournalStream(journal->shared_from_this()).write((char *)buffer, sizeof(buffer));
	setp((char *)buffer, (char *)(buffer + sizeof(buffer) - 1));
	return 0;
}

ContextBuffer::int_type ContextBuffer::sync()
{
	Context::getContext()->getJournalStream(journal->shared_from_this()).write((char *)buffer, pptr() - pbase());
	setp((char *)buffer, (char *)(buffer + sizeof(buffer) - 1));
	return 0;
}

JournalMem::JournalMem(JournalFile *file) :
	FileWriterTXT(FileName()), buf(file)
{
	outputFile = new ostream(&buf);
}

JournalMem::~JournalMem()
{
	delete outputFile;
	outputFile = 0;
}

JournalFile::JournalFile(const FileName& fileName, PSharedMutex filelock, IdentifierType db, IdentifierType cube) :
	fileName(fileName), db(db), cube(cube), firstLine(true), filelock(filelock), fileSize(0)
{
	openFile();
}

JournalFile::~JournalFile()
{
	closeFile();
}

void JournalFile::openFile()
{

	// find last journal file
	bool lastJournalFound = false;
	int last = 0;
	int next = 0;

	while (!lastJournalFound) {
		stringstream se;
		se << fileName.name << "_" << next;

		FileName jf(fileName.path, se.str(), fileName.extension);

		if (!FileUtils::isReadable(jf)) {
			lastJournalFound = true;
		} else {
			last = next;
			next++;
		}
	}

	lastFileId = last;

	stringstream se;
	se << fileName.name << "_" << lastFileId;

	lastFileName = FileName(fileName.path, se.str(), fileName.extension);

	// open last file
	fileSize = FileWriter::getFileSize(lastFileName);
	writer.reset(FileWriter::getFileWriter(lastFileName));
	writer->openFile(true);

	// check size of found journal file
	if (last != next) {
		checkFileSize();
	}
}

void JournalFile::closeFile()
{
	if (writer) {
		writer->closeFile();
		writer.reset();
	}
}

void JournalMem::appendCommand(const string& user, const string& event, const string& command)
{
	if (!isFirstValue()) {
		nextLine();
	}

	appendTimeStamp();

	*outputFile << StringUtils::escapeString(user) << ";" << StringUtils::escapeString(event) << ";" << command << ";";
}

bool JournalFile::checkFileSize()
{
	if (fileSize > 100000000) {
		closeFile();

		lastFileId++;

		stringstream se;
		se << fileName.name << "_" << lastFileId;

		lastFileName = FileName(fileName.path, se.str(), fileName.extension);
		fileSize = 0;
		writer.reset(FileWriter::getFileWriter(lastFileName));
		writer->openFile(true);
	}

	return !(lastFileId == 0 && fileSize == 0); // return true if journal is non-empty
}

void JournalFile::deleteJournalFiles(const FileName& fileName, bool deleteArchiv)
{
	// find last journal file
	bool lastJournalFound = false;
	int num = 0;

	while (!lastJournalFound) {
		stringstream se;
		se << fileName.name << "_" << num;

		FileName jf(fileName.path, se.str(), fileName.extension);

		if (!FileUtils::isReadable(jf)) {
			lastJournalFound = true;
		} else {
			FileWriter::deleteFile(jf);
			num++;
		}
	}

	if (deleteArchiv) {
		try {
			FileWriter::deleteFile(FileName(fileName.path, fileName.name, "archived"));
		} catch (const FileOpenException& e) {
			Logger::info << "ignoring " << e.getMessage() << "/" << e.getDetails() << endl;
		}
	}
}

void JournalFile::archiveJournalFiles(const FileName& fileName)
{
	// find last journal file
	bool lastJournalFound = false;
	int num = 0;

	FileName af(fileName.path, fileName.name, "archived");
	FileUtils::paloofstream outputFile(af.fullPath().c_str(), ios::app);

	while (!lastJournalFound) {
		stringstream se;
		se << fileName.name << "_" << num;

		FileName jf(fileName.path, se.str(), fileName.extension);

		if (!FileUtils::isReadable(jf)) {
			lastJournalFound = true;
		} else {
			bool isbf = false;
			{
				bffilebuf bf(jf, ios::in);
				isbf = bf.is_open();
			}
			if (!isbf) {
				FileUtils::paloifstream inputFile(jf.fullPath().c_str());
				static const size_t BUF_SIZE = 1024*1024;
				boost::scoped_array<char> spBuf(new char[BUF_SIZE]);
				char *buf = spBuf.get();

				while (inputFile) {
					inputFile.read(buf, BUF_SIZE);
					outputFile.write(buf, inputFile.gcount());
				}
			}

			FileWriter::deleteFile(jf);
			num++;
		}
	}
}

void JournalFile::flush(ContextStream &str)
{
	bool cont = true;
	if (db != (IdentifierType) - 1) {
		PDatabase d = Context::getContext()->getServer()->lookupDatabase(db, false);
		if (!d) {
			cont = false;
		} else {
			if (cube != (IdentifierType) - 1) {
				PCube c = d->lookupCube(cube, false);
				if (!c) {
					cont = false;
				}
			}
		}
	}
	if (cont) {
		WriteLocker wl(filelock->getLock());
		checkFileSize();
		if (!Context::getContext()->getJournalIsFirst(shared_from_this())) {
			str << endl;
		}
		if (!writer) {
			openFile();
		}

		if (firstLine) {
			firstLine = false;
			stringstream ss;

			timeval tv;
			gettimeofday(&tv, 0);
			ss << tv.tv_sec << "." << tv.tv_usec << ";";

			ss << StringUtils::escapeString("") << ";" << StringUtils::escapeString("") << ";" << JournalFileReader::JOURNAL_VERSION << ";";
			ss << Server::getVersion() << ";" << Server::getRevision() << ";" << endl;

			fileSize += ss.str().length();
			writer->appendRaw(ss.str());
		}

		fileSize += str.str().length();
		writer->appendRaw(str.str());
	}
}

void JournalFile::clear()
{
	Context::getContext()->deleteJournalStream(shared_from_this());
	fileSize = 0;
	firstLine = true;
}

bool JournalFile::isFirstValue()
{
	return Context::getContext()->getJournalIsFirst(shared_from_this());
}

void JournalFile::setFirstValue(bool v)
{
	Context::getContext()->setJournalIsFirst(shared_from_this(), v);
}

}
