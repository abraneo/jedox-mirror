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

#include "InputOutput/FileWriterBF.h"
#include "Logger/Logger.h"
#include "Exceptions/FileOpenException.h"

namespace palo {

const char bffilebuf::BF_FILE_IDENTIFICATOR[] = "PALO_ENCRYPTED_FILE_1_0";
string bffilebuf::passphrase;
int64_t bffilebuf::initivec = 0;
bool bffilebuf::usecrypt = false;

void bffilebuf::setOptions(const string &str, int64_t ivec, bool crypt)
{
	passphrase = str;
	initivec = ivec;
	usecrypt = crypt;
}

bool bffilebuf::canCrypt()
{
	return usecrypt && !passphrase.empty();
}

bffilebuf::bffilebuf(const FileName &filename, ios::openmode mode) :
	in(0), out(0), readbuf(0), writebuf(0), remaining(0), num(0)
{
	if (mode & ios::in) {
		remaining = FileWriter::getFileSize(filename) - strlen(BF_FILE_IDENTIFICATOR) - 1;
		in = new FileUtils::paloifstream(filename.fullPath().c_str());
		string ident;
		getline(*in, ident);
		if (ident == BF_FILE_IDENTIFICATOR) {
			readbuf = new unsigned char[BUF_SIZE];
			setg((char *)readbuf, (char *)readbuf, (char *)readbuf);
		} else {
			delete in;
			in = 0;
		}
	}
	if (mode & ios::out) {
		size_t s = FileWriter::getFileSize(filename);
		out = new FileUtils::paloofstream(filename.fullPath().c_str(), mode);
		if (!(s && (mode & ios::app))) {
			*out << BF_FILE_IDENTIFICATOR << endl;
		}
		writebuf = new unsigned char[BUF_SIZE];
		setp((char *)writebuf, (char *)(writebuf + BUF_SIZE - 1));
	}
	memcpy(ivec, &initivec, 8);
	BF_set_key(&key, (int)passphrase.size(), (const unsigned char *)passphrase.c_str());
}

bffilebuf::~bffilebuf()
{
	close();
	if (readbuf) {
		delete [] readbuf;
	}
	if (writebuf) {
		delete [] writebuf;
	}
	if (in) {
		delete in;
	}
	if (out) {
		delete out;
	}
}

void bffilebuf::close()
{
	if (in) {
		in->close();
	}
	if (out) {
		out->close();
	}
}

bool bffilebuf::is_open()
{
	if (in) {
		return in->is_open();
	}
	if (out) {
		return out->is_open();
	}
	return false;
}

int bffilebuf::__writetofile(size_t len)
{
	if (writebuf && out) {
		unsigned char crypt[BUF_SIZE];
		BF_cfb64_encrypt(writebuf, crypt, (long)len, &key, ivec, &num, BF_ENCRYPT);
		out->write((const char *)crypt, len);
		if (!out->bad() && !out->fail()) {
			setp((char *)writebuf, (char *)(writebuf + BUF_SIZE - 1));
			return 0;
		}
	}
	return EOF;
}

int bffilebuf::sync()
{
	int ret = __writetofile(pptr() - pbase());
	out->flush();
	return ret;
}

int bffilebuf::overflow(int c)
{
	if (writebuf) {
		writebuf[BUF_SIZE - 1] = (char)c;
		return __writetofile(BUF_SIZE);
	}
	return EOF;
}

int bffilebuf::underflow()
{
	if (readbuf && in) {
		unsigned char crypt[BUF_SIZE - 1];
		in->read((char *)crypt, BUF_SIZE - 1);
		streamsize read = in->gcount();
		if (read) {
			BF_cfb64_encrypt(crypt, readbuf, (long)read, &key, ivec, &num, BF_DECRYPT);
			remaining -= read;
			setg((char *)readbuf, (char *)readbuf, (char *)(readbuf + read));
			return readbuf[0];
		}
	}
	return EOF;
}

streamsize bffilebuf::showmanyc()
{
	return remaining;
}

FileWriterBF::FileWriterBF(const FileName& fileName)
	: FileWriterTXT(fileName), bf(0)
{
}

FileWriterBF::~FileWriterBF()
{
	closeFile();
}

void FileWriterBF::openFile(bool append)
{
	bf = new bffilebuf(fileName, append ? ios::out | ios::app : ios::out);
	if (!bf || !bf->is_open()) {
		Logger::warning << "could not write to file '" << fileName.fullPath() << "'" << endl;
		throw FileOpenException("Could not open file for writing.", fileName.fullPath());
	}
	outputFile = new ostream(bf);
}

void FileWriterBF::closeFile()
{
	if (outputFile != 0) {
		nextLine();
		writeBuffer();
		delete outputFile;
		outputFile = 0;
	}
	if (bf) {
		delete bf;
		bf = 0;
	}
}

}
