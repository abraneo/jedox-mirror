/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "palo.h"

#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"
#include "Exceptions/ErrorException.h"
#include "Exceptions/ParameterException.h"

namespace palo {

#if defined(_MSC_VER)
FileUtils::wfilebuf::wfilebuf(string filename, ios::openmode mode) :
	hFile(INVALID_HANDLE_VALUE), readbuf(0), writebuf(0), remaining(0)
{
	DWORD gen = 0;
	DWORD flags = 0;
	if (mode & ios::in) {
		gen |= GENERIC_READ;
		flags = OPEN_EXISTING;
		readbuf = new unsigned char[BUF_SIZE];
		setg((char *)readbuf, (char *)readbuf, (char *)readbuf);
	}
	if (mode & ios::out) {
		gen |= GENERIC_WRITE;
		if (mode & ios::app) {
			flags = OPEN_ALWAYS;
		} else {
			flags = CREATE_ALWAYS;
		}
		writebuf = new unsigned char[BUF_SIZE];
		setp((char *)writebuf, (char *)writebuf, (char *)(writebuf + BUF_SIZE - 1));
	}
	hFile = CreateFile(filename.c_str(), gen, FILE_SHARE_READ, 0, flags, FILE_ATTRIBUTE_NORMAL, 0);
	if (mode & ios::app) {
		SetFilePointer(hFile, 0, 0, FILE_END);
	}
	LARGE_INTEGER tmp;
	if (GetFileSizeEx(hFile, &tmp)) {
		remaining = tmp.QuadPart;
	}
}

FileUtils::wfilebuf::~wfilebuf()
{
	close();
	if (readbuf) {
		delete [] readbuf;
	}
	if (writebuf) {
		delete [] writebuf;
	}
}

void FileUtils::wfilebuf::close()
{
	if (hFile != INVALID_HANDLE_VALUE) {
		sync();
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

bool FileUtils::wfilebuf::is_open()
{
	return hFile != INVALID_HANDLE_VALUE;
}

int FileUtils::wfilebuf::__writetofile(size_t len)
{
	if (writebuf) {
		DWORD written = 0;
		if (!len) {
			return 0;
		}
		if (WriteFile(hFile, writebuf, (DWORD)len, &written, 0) && len == written) {
			setp((char *)writebuf, (char *)writebuf, (char *)(writebuf + BUF_SIZE - 1));
			return 0;
		}
	}
	return EOF;
}

int FileUtils::wfilebuf::sync()
{
	return __writetofile(pptr() - pbase());
}

int FileUtils::wfilebuf::overflow(int c)
{
	if (writebuf) {
		writebuf[BUF_SIZE - 1] = (char)c;
		return __writetofile(BUF_SIZE);
	}
	return EOF;
}

int FileUtils::wfilebuf::underflow()
{
	if (readbuf) {
		DWORD read = 0;
		if (ReadFile(hFile, readbuf, BUF_SIZE - 1, &read, 0) && read) {
			remaining -= read;
			setg((char *)readbuf, (char *)readbuf, (char *)(readbuf + read));
			return readbuf[0];
		}
	}
	return EOF;
}

streamsize FileUtils::wfilebuf::showmanyc()
{
	return remaining;
}

FileUtils::paloofstream::paloofstream(string filename, ios::openmode mode) :
	ostream(&_buf), _buf(filename, ios::out | mode)
{
	if (!_buf.is_open()) {
		setstate(failbit | badbit);
	}
}

void FileUtils::paloofstream::close()
{
	_buf.close();
	setstate(failbit | badbit);
}

bool FileUtils::paloofstream::is_open()
{
	return _buf.is_open();
}

FileUtils::paloifstream::paloifstream(string filename) :
	istream(&_buf), _buf(filename, ios::in)
{
	if (!_buf.is_open()) {
		setstate(failbit | badbit);
	}
}

void FileUtils::paloifstream::close()
{
	_buf.close();
	setstate(failbit | badbit);
}

bool FileUtils::paloifstream::is_open()
{
	return _buf.is_open();
}
#endif

FileUtils::paloifstream *FileUtils::newIfstream(const string& filename)
{
	paloifstream *s = new paloifstream(filename.c_str());
	if (s && !*s) {
		delete s;
		return 0;
	}
	return s;
}

FileUtils::paloofstream *FileUtils::newOfstream(const string& filename)
{
	paloofstream *s = new paloofstream(filename.c_str());
	if (s && !*s) {
		delete s;
		return 0;
	}
	return s;
}

FileUtils::paloofstream *FileUtils::newOfstreamAppend(const string& filename)
{
	paloofstream *s = new paloofstream(filename.c_str(), ios::app);
	if (s && !*s) {
		delete s;
		return 0;
	}
	return s;
}

bool FileUtils::isReadable(const FileName& fileName)
{
	FILE* file = fopen(fileName.fullPath().c_str(), "r");

	if (file == 0) {
		return false;
	} else {
		fclose(file);
#if defined(_MSC_VER)
		usleep(1000);

		file = fopen(fileName.fullPath().c_str(), "r");

		if (file == 0) {
			Logger::warning << "encountered MS file-system change-log bug during isReadable" << endl;
			return false;
		} else {
			fclose(file);
		}
#endif
		return true;
	}
}

bool FileUtils::remove(const FileName& fileName)
{
	int result = std::remove(fileName.fullPath().c_str());

#if defined(_MSC_VER)
	if (result != 0 && errno == EACCES) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during remove, sleeping to recover" << endl;
			usleep(1000);

			result = std::remove(fileName.fullPath().c_str());

			if (result == 0 || errno != EACCES) {
				break;
			}
		}
	}
#endif

	return (result != 0) ? false : true;
}

bool FileUtils::rename(const FileName& oldName, const FileName& newName)
{
	int result = std::rename(oldName.fullPath().c_str(), newName.fullPath().c_str());

#if defined(_MSC_VER)
	if (result != 0 && errno == EACCES) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during rename, sleeping to recover" << endl;
			usleep(1000);

			result = std::rename(oldName.fullPath().c_str(), newName.fullPath().c_str());

			if (result == 0 || errno != EACCES) {
				break;
			}
		}
	}
#endif

	return (result != 0) ? false : true;
}

bool FileUtils::renameDirectory(const FileName& oldName, const FileName& newName)
{
	int result = std::rename(oldName.path.c_str(), newName.path.c_str());

#if defined(_MSC_VER)
	if (result != 0 && errno == EACCES) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during rename directory, sleeping to recover" << endl;
			usleep(1000);

			result = std::rename(oldName.path.c_str(), newName.path.c_str());

			if (result == 0 || errno != EACCES) {
				break;
			}
		}
	}
#endif

	return (result != 0) ? false : true;
}

bool FileUtils::removeDirectory(const FileName& name)
{
	int result = rmdir(name.path.c_str());

#if defined(_MSC_VER)
	if (result != 0 && (errno == EACCES || errno == ENOTEMPTY)) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during rmdir, sleeping to recover" << endl;
			usleep(1000);

			result = rmdir(name.path.c_str());

			if (result == 0 || (errno != EACCES && errno != ENOTEMPTY)) {
				break;
			}
		}
	}
#endif

	return (result != 0) ? false : true;
}

bool FileUtils::createDirectory(const string& name)
{
#if defined(_MSC_VER)
	int result = mkdir(name.c_str());

	if (result != 0 && errno == EACCES) {
		for (int ms = 0; ms < 5; ms++) {
			Logger::warning << "encountered MS file-system change-log bug during mkdir, sleeping to recover" << endl;
			usleep(1000);

			result = mkdir(name.c_str());

			if (result == 0 || errno != EACCES) {
				break;
			}
		}
	}
#else
	int result = mkdir(name.c_str(), 0777);
#endif

	if (result != 0 && errno == EEXIST && isDirectory(name)) {
		result = 0;
	}

	return (result != 0) ? false : true;
}

bool FileUtils::copyFile(const std::string& fromFile, const std::string &toFile)
{
	throw ErrorException(ErrorException::ERROR_COPY_FAILED, "file copy not implemented");
	//return true;
}

bool FileUtils::copyDirectory(std::string fromDir, std::string toDir)
{
	if (fromDir.empty()) {
		throw ErrorException(ErrorException::ERROR_COPY_FAILED, "source folder not specified");
	}
	if (toDir.empty()) {
		throw ErrorException(ErrorException::ERROR_COPY_FAILED, "target folder not specified");
	}
#if defined(_MSC_VER)
	if (fromDir.at(fromDir.size() - 1) != '\\') {
		fromDir += "\\";
	}
	if (toDir.at(toDir.size() - 1) != '\\') {
		toDir += "\\";
	}
#else
	if (fromDir.at(fromDir.size() - 1) != '/') {
		fromDir += "/";
	}
	if (toDir.at(toDir.size() - 1) != '/') {
		toDir += "/";
	}
#endif
	if (isDirectory(toDir)) {
		throw ErrorException(ErrorException::ERROR_COPY_FAILED, "target folder already exists");
	}
	if (!createDirectory(toDir)) {
		throw ErrorException(ErrorException::ERROR_COPY_FAILED, "target folder can't be created");
	}
	vector<string> files = listFiles(fromDir);
	if (files.empty()) {
		throw ErrorException(ErrorException::ERROR_COPY_FAILED, "can't read file list to backup");
	}
	for (vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
		if (!copyFile(fromDir + *it, toDir + *it)) {
			throw ParameterException(ErrorException::ERROR_COPY_FAILED, "database folder can't be copied", "filename", *it);
		}
	}

	return true;
}

bool FileUtils::zipDirectory(std::string fromDir, std::string toZipFile)
{
	if (fromDir.empty()) {
		throw ErrorException(ErrorException::ERROR_ZIP, "source folder not specified");
	}
	if (toZipFile.empty()) {
		throw ErrorException(ErrorException::ERROR_ZIP, "target filename not specified");
	}
#if defined(_MSC_VER)
	if (fromDir.at(fromDir.size() - 1) != '\\') {
		fromDir += "\\";
	}
#else
	if (fromDir.at(fromDir.size() - 1) != '/') {
		fromDir += "/";
	}
#endif
	if (toZipFile.size() < 4 || toZipFile.substr(toZipFile.size() - 4) != ".zip") {
		throw ErrorException(ErrorException::ERROR_ZIP, "target filename without .zip extension");
	}

	if (isRegularFile(toZipFile)) {
		throw ErrorException(ErrorException::ERROR_ZIP, "target file already exists");
	}

	zipFile z = zipOpen(toZipFile.c_str(), APPEND_STATUS_CREATE);
	if (z) {
		vector<string> files = listFiles(fromDir);
		if (files.empty()) {
			throw ErrorException(ErrorException::ERROR_ZIP, "can't read file list to backup");
		}
		for (vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
			if (!addToZip(z, fromDir + *it)) {
				throw ParameterException(ErrorException::ERROR_ZIP, "database folder can't be copied", "filename", *it);
			}
		}
		int errclose = zipClose(z, NULL);
		if (errclose != ZIP_OK) {
			throw ErrorException(ErrorException::ERROR_ZIP, "can't close ZIP file.");
		}
	} else {
		throw ErrorException(ErrorException::ERROR_ZIP, "cannot create zip file");
	}

	return true;
}

#if defined(_MSC_VER)
vector<string> _stdcall FileUtils::listFiles(const string& directory)
{
	vector<string> result;

	struct _finddata_t fd;
	intptr_t handle;

	string filter = directory + "\\*";
	handle = _findfirst(filter.c_str(), &fd);

	if (handle == -1) {
		return result;
	}

	do {
		if (strcmp(fd.name, ".") != 0 && strcmp(fd.name, "..") != 0) {
			result.push_back(fd.name);
		}
	} while (_findnext(handle, &fd) != -1);

	_findclose(handle);

	return result;
}
#else
vector<string> FileUtils::listFiles(const string& directory)
{
	vector<string> result;

	DIR * d = opendir(directory.c_str());

	if (d == 0) {
		return result;
	}

	struct dirent * de = readdir(d);

	while (de != 0) {
		if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			result.push_back(de->d_name);
		}

		de = readdir(d);
	}

	closedir(d);

	return result;
}
#endif

bool FileUtils::isDirectory(const string& path)
{
	struct stat stbuf;
	stat(path.c_str(), &stbuf);

	return (stbuf.st_mode & S_IFMT) == S_IFDIR;
}

bool FileUtils::isRegularFile(const string& path)
{
	struct stat stbuf;
	stat(path.c_str(), &stbuf);

	return (stbuf.st_mode & S_IFMT) == S_IFREG;
}

#if defined(_MSC_VER)
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
#define MAXFILENAME (256)
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#endif

bool FileUtils::addToZip(zipFile zf, string fileName) {
	FILE * fin;
	int size_read;
	const char *filenameinzip = fileName.c_str();
	const char *savefilenameinzip;
	zip_fileinfo zi;
	unsigned long crcFile = 0;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

	// the path name saved, should not include a leading slash
	// if it did, windows/xp and dynazip couldn't read the zip file
	savefilenameinzip = filenameinzip;
	while (savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/') {
		savefilenameinzip++;
	}
	if (savefilenameinzip[0] == '.' && (savefilenameinzip[1] == '\\' || savefilenameinzip[1] == '/')) {
		savefilenameinzip += 2;
	}
	// TODOMD add current dir parameter to use only part of filepath in the zip

	/*should the zip file contain any path at all?*/
	/*if (opt_exclude_path) {
		const char *tmpptr;
		const char *lastslash = 0;
		for (tmpptr = savefilenameinzip; *tmpptr; tmpptr++) {
			if( *tmpptr == '\\' || *tmpptr == '/') {
				lastslash = tmpptr;
			}
		}
		if (lastslash != NULL) {
			savefilenameinzip = lastslash+1; // base filename follows last slash.
		}
	}*/

	int err = zipOpenNewFileInZip3(zf, savefilenameinzip, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, 0, crcFile);

	if (err != ZIP_OK) {
		Logger::error << "error in opening " << filenameinzip << " in zipfile" << endl;
	} else {
		fin = fopen(filenameinzip, "rb");
		if (fin == NULL) {
			err = ZIP_ERRNO;
			Logger::error << "error in opening " << filenameinzip << " for reading" << endl;
		}
	}

	const int size_buf = 16384;
	void *buf = (void*)malloc(size_buf);
	if (!buf) {
		Logger::error << "error in allocating buffer for zip writing" << endl;
		return false;
	}

	if (err == ZIP_OK) {
		do {
			err = ZIP_OK;
			size_read = (int)fread(buf, 1, size_buf, fin);
			if (size_read < size_buf)
				if (feof(fin) == 0) {
					Logger::error << "error in reading " << filenameinzip << endl;
					err = ZIP_ERRNO;
				}

				if (size_read > 0) {
					err = zipWriteInFileInZip(zf,buf,size_read);
					if (err < 0) {
						Logger::error << "error in writing in zipfile " << filenameinzip << endl;
					}
				}
		} while ((err == ZIP_OK) && (size_read > 0));

		if (fin) {
			fclose(fin);
		}

		if (err < 0) {
			err = ZIP_ERRNO;
		} else {
			err = zipCloseFileInZip(zf);
			if (err != ZIP_OK)
				Logger::error << "error in closing zipfile " << filenameinzip << endl;
		}
	}

	free(buf);

	if (err == ZIP_OK) {
		return true;
	} else {
		return false;
	}
}

}
