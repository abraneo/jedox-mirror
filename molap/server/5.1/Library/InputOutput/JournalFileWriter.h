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

#ifndef INPUT_OUTPUT_JOURNAL_FILE_WRITER_H
#define INPUT_OUTPUT_JOURNAL_FILE_WRITER_H 1

#include "palo.h"

#include "InputOutput/FileUtils.h"
#include "InputOutput/FileWriterTXT.h"
#include "boost/enable_shared_from_this.hpp"

namespace palo {

class ContextStream;

class ContextBuffer : public std::streambuf {
public:
	ContextBuffer(JournalFile* journal) : journal(journal) {setp((char *)buffer, (char *)(buffer + sizeof(buffer) - 1));}
private:
	virtual int_type overflow(int_type c);
	virtual int_type sync();

	JournalFile* journal;
	unsigned char buffer[8192];
};

class SERVER_CLASS JournalMem : public FileWriterTXT, public boost::enable_shared_from_this<JournalMem> {
public:
	JournalMem(JournalFile *file);
	virtual ~JournalMem();
	void appendCommand(const string& user, const string& event, const string& command);
	void flush() {outputFile->flush();}

private:
	ContextBuffer buf;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief journal file writer
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS JournalFile : public boost::enable_shared_from_this<JournalFile> {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes journal files
	////////////////////////////////////////////////////////////////////////////////

	static void deleteJournalFiles(const FileName& fileName, bool deleteArchiv = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief archives journal files
	////////////////////////////////////////////////////////////////////////////////

	static void archiveJournalFiles(const FileName& fileName);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new journal file writer
	////////////////////////////////////////////////////////////////////////////////

	JournalFile(const FileName& fileName, PSharedMutex filelock, IdentifierType db, IdentifierType cube);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes journal file write
	////////////////////////////////////////////////////////////////////////////////

	virtual ~JournalFile();

	void flush(ContextStream &str);
	void clear();
	virtual void closeFile();

private:
	bool checkFileSize();
	void openFile();
	virtual bool isFirstValue();
	virtual void setFirstValue(bool);

private:
	int lastFileId;
	FileName lastFileName;
	FileName fileName;
	IdentifierType db, cube;
	bool firstLine;

	boost::shared_ptr<FileWriter> writer;
	PSharedMutex filelock;
	size_t fileSize;
};

}

#endif
