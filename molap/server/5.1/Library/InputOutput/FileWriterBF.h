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

#ifndef INPUT_OUTPUT_FILE_WRITER_BF_H
#define INPUT_OUTPUT_FILE_WRITER_BF_H 1

#include "InputOutput/FileWriterTXT.h"
#include "InputOutput/FileUtils.h"

#include <openssl/blowfish.h>

namespace palo {

class SERVER_CLASS bffilebuf : public std::streambuf {
	static const size_t BUF_SIZE = 4 * 1024;
public:
	static const char BF_FILE_IDENTIFICATOR[];
	bffilebuf(const FileName &filename, std::ios::openmode mode);
	virtual ~bffilebuf();
	void close();
	bool is_open();
	virtual int sync();
	virtual int overflow(int c);
	virtual int underflow();
	virtual std::streamsize showmanyc();
	static void setOptions(const string &str, int64_t ivec, bool crypt);
	static bool canCrypt();
private:
	static string passphrase;
	static int64_t initivec;
	static bool usecrypt;
	int __writetofile(size_t len);
	FileUtils::paloifstream *in;
	FileUtils::paloofstream *out;
	unsigned char *readbuf;
	unsigned char *writebuf;
	uint64_t remaining;
	BF_KEY key;
	unsigned char ivec[8];
	int num;
};

class SERVER_CLASS FileWriterBF : public FileWriterTXT {
public:
	FileWriterBF(const FileName& fileName);
	virtual ~FileWriterBF();
	virtual void openFile(bool append = false);
	virtual void closeFile();
private:
	bffilebuf *bf;
};

}

#endif
