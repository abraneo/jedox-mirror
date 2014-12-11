////////////////////////////////////////////////////////////////////////////////
/// @brief collections of zip file functions
///
/// @file
///
/// Copyright (C) 2006-2013 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// You may obtain a copy of the License at
///
/// <a href="http://www.jedox.com/license_palo_suite.txt">
///   http://www.jedox.com/license_palo_suite.txt
/// </a>
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// Portions of the code developed by triagens GmbH, Koeln on behalf of Jedox
/// AG. Intellectual property rights for these portions has triagens GmbH,
/// Koeln, or othervise Jedox AG, Freiburg. Exclusive worldwide exploitation
/// right (commercial copyright) has Jedox AG, Freiburg.
///
/// @author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
////////////////////////////////////////////////////////////////////////////////

#include "palo.h"

#include "InputOutput/ZipUtils.h"
#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"
#include "Exceptions/ErrorException.h"
#include "Exceptions/ParameterException.h"
#include "Olap/Server.h"

#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#include "zip/iowin32.h"
#else
#include <unistd.h>
#include <utime.h>
#endif


namespace palo {

bool ZipUtils::zipDirectory(std::string fromDir, std::string toZipFile, bool append)
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

	if (!append && FileUtils::isRegularFile(toZipFile)) {
		throw ErrorException(ErrorException::ERROR_ZIP, "target file already exists");
	}

	zipFile z = zipOpen(toZipFile.c_str(), append ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);
	if (z) {
		vector<string> files = FileUtils::listFiles(fromDir);
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
	int ret = 0;
	struct stat s;
	struct tm *filedate;
	time_t tm_t = 0;

	if (strcmp(f, "-") != 0) {
		char name[MAXFILENAME + 1];
		int len = strlen(f);
		if (len > MAXFILENAME) {
			len = MAXFILENAME;
		}

		strncpy(name, f, MAXFILENAME - 1);
		/* strncpy doesnt append the trailing NULL, of the string is too long. */
		name[MAXFILENAME] = '\0';

		if (name[len - 1] == '/') {
			name[len - 1] = '\0';
		}
		/* not all systems allow stat'ing a file with / appended */
		if (stat(name, &s) == 0) {
			tm_t = s.st_mtime;
			ret = 1;
		}
	}
	filedate = localtime(&tm_t);

	tmzip->tm_sec = filedate->tm_sec;
	tmzip->tm_min = filedate->tm_min;
	tmzip->tm_hour = filedate->tm_hour;
	tmzip->tm_mday = filedate->tm_mday;
	tmzip->tm_mon = filedate->tm_mon;
	tmzip->tm_year = filedate->tm_year;

	return ret;
}
#endif

int isLargeFile(const char *filename)
{
	int largeFile = 0;
	ZPOS64_T pos = 0;
	FILE* pFile = fopen64(filename, "rb");

	if (pFile != NULL) {
		/*int n = */fseeko64(pFile, 0, SEEK_END);
		pos = ftello64(pFile);
		//printf("File : %s is %lld bytes\n", filename, pos);
		if (pos >= 0xffffffff) {
			largeFile = 1;
		}
		fclose(pFile);
	}

	return largeFile;
}

bool ZipUtils::addToZip(zipFile zf, string fileName)
{
	FILE * fin;
	int size_read;
	const char *filenameinzip = fileName.c_str();
	const char *savefilenameinzip;
	zip_fileinfo zi;
	unsigned long crcFile = 0;
	int zip64 = 0;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

	zip64 = isLargeFile(filenameinzip);

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

	int err = zipOpenNewFileInZip3_64(zf, savefilenameinzip, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, 0, crcFile, zip64);

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

void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _MSC_VER
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

	hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftm);
	SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
	CloseHandle(hFile);
#else
	struct utimbuf ut;
	struct tm newdate;
	newdate.tm_sec = tmu_date.tm_sec;
	newdate.tm_min = tmu_date.tm_min;
	newdate.tm_hour = tmu_date.tm_hour;
	newdate.tm_mday = tmu_date.tm_mday;
	newdate.tm_mon = tmu_date.tm_mon;
	if (tmu_date.tm_year > 1900) {
		newdate.tm_year = tmu_date.tm_year - 1900;
	} else {
		newdate.tm_year = tmu_date.tm_year;
	}
	newdate.tm_isdst = -1;

	ut.actime = ut.modtime = mktime(&newdate);
	utime(filename, &ut);
#endif
}

/*int mymkdir(char *dirname)
{
	int ret=0;
#ifdef _MSC_VER
	ret = _mkdir(dirname);
#else
	ret = mkdir(dirname,0775);
#endif
	return ret;
}

int makedir(char *newdir)
{
	char *buffer;
	char *p;
	int len = (int)strlen(newdir);

	if (len <= 0)
	return 0;

	buffer = (char*)malloc(len+1);
	if (buffer==NULL)
	{
		Logger::error << "Error allocating memory" << endl;
		return UNZ_INTERNALERROR;
	}
	strcpy(buffer,newdir);

	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (mymkdir(buffer) == 0) {
		free(buffer);
		return 1;
	}

	p = buffer+1;
	while (1) {
		char hold;

		while(*p && *p != '\\' && *p != '/') {
			p++;
		}
		hold = *p;
		*p = 0;
		if ((mymkdir(buffer) == -1) && (errno == ENOENT)) {
			printf("couldn't create directory %s\n",buffer);
			free(buffer);
			return 0;
		}
		if (hold == 0) {
			break;
		}
		*p++ = hold;
	}
	free(buffer);
	return 1;
}*/

int do_extract_currentfile(unzFile uf, bool popt_extract_without_path, bool popt_overwrite, const char *password, bool dbExtract, string newDbName, string *oldDbName)
{
	char filename_inzip[256];
	char *filename_withoutpath;

	unz_file_info64 file_info;
	//uLong ratio = 0;
	int err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err != UNZ_OK) {
		Logger::error << "error with zipfile in unzGetCurrentFileInfo " << err << endl;
		return err;
	}

	uInt size_buf = 8192;
	void *buf = (void *)malloc(size_buf);
	if (buf == NULL) {
		Logger::error << "Error allocating memory" << endl;
		return UNZ_INTERNALERROR;
	}

	char *p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0') {
		if (((*p) == '/') || ((*p) == '\\')) {
			filename_withoutpath = p+1;
		}
		p++;
	}

	string str_filename_inzip = filename_inzip;;
	string folderName = str_filename_inzip;

	while (folderName.length() > 0 && (folderName[0] == '.' || folderName[0] == '/' || folderName[0] == '\\')) {
		folderName = folderName.substr(1, string::npos);
	}

	size_t i = folderName.find_first_of("/\\", 0); // TODOMD for general extract maybe find_last_of
	if (i != string::npos) {
		folderName = folderName.substr(0, i);
	}

	if (dbExtract) {
		if (UTF8Comparer::compare(folderName.c_str(), Server::NAME_SYSTEM_DATABASE) == 0) {
			// System DB is not extracted
			return 0;
		}
	}

	if (newDbName != "") {
		size_t start_pos = str_filename_inzip.find(folderName);
		if (start_pos != std::string::npos) {
			str_filename_inzip.replace(start_pos, folderName.length(), newDbName);
		}
		folderName = newDbName;
	}

	if (oldDbName) {
		*oldDbName = folderName; // original DB name if kept or new db name as specified by request
	}

	if ((*filename_withoutpath) == '\0') {
		if (!popt_extract_without_path) {
			Logger::trace << "creating directory " << filename_inzip << endl;
			FileUtils::createDirectory(filename_inzip);
			if (!FileUtils::isRegularFile(filename_inzip, "_loading.lock")) {
				FileUtils::createFile(filename_inzip, "_loading.lock");
			}
		}
	} else {
		string write_filename;

		if (!popt_extract_without_path) {
			write_filename = str_filename_inzip;
		} else {
			write_filename = filename_withoutpath;
		}

		err = unzOpenCurrentFilePassword(uf, password);
		if (err != UNZ_OK) {
			Logger::error << "error with zipfile in unzOpenCurrentFilePassword" << err << endl;
		}

		if (!popt_overwrite && err == UNZ_OK) {
			//char rep = 0;
			FILE *ftestexist;
			ftestexist = fopen64(write_filename.c_str(), "rb");
			if (ftestexist != NULL) {
				fclose(ftestexist);
				Logger::error << "file already exists" << endl;
				err = UNZ_ALREADYEXISTS;
			}
		}

		FILE *fout = NULL;

		if (err == UNZ_OK) {
			fout = fopen64(write_filename.c_str(), "wb");

			/* some zipfile don't contain directory alone before file */
			if (fout == NULL && !popt_extract_without_path && (UTF8Comparer::compare(filename_withoutpath, str_filename_inzip.c_str()) != 0)) {
				Logger::trace << "creating directory " << folderName << endl;
				FileUtils::createDirectory(folderName);
				if (!FileUtils::isRegularFile(folderName, "_loading.lock")) {
					FileUtils::createFile(folderName, "_loading.lock");
				}
				fout = fopen64(write_filename.c_str(), "wb");
			}

			if (fout == NULL) {
				Logger::error << "error opening file " << write_filename << endl;
			}
		}

		if (fout != NULL) {
			Logger::trace << "extracting file " << write_filename << endl;

			do {
				err = unzReadCurrentFile(uf, buf, size_buf);
				if (err < 0) {
					Logger::error << "error with zipfile in unzReadCurrentFile " << err << endl;
					break;
				}
				if (err > 0) {
					if (fwrite(buf,err,1,fout) != 1) {
						Logger::error << "error in writing extracted file" << endl;
						err = UNZ_ERRNO;
						break;
					}
				}
			} while (err > 0);
			if (fout) {
				fclose(fout);
			}

			if (err == 0) {
				change_file_date(write_filename.c_str(), file_info.dosDate, file_info.tmu_date);
			}
		}

		if (err == UNZ_OK) {
			err = unzCloseCurrentFile(uf);
			if (err != UNZ_OK) {
				Logger::error << "error with zipfile in unzCloseCurrentFile" << err << endl;
			}
		} else {
			unzCloseCurrentFile(uf); /* don't lose the error */
		}
	}

	free(buf);
	return err;
}

int do_extract(unzFile uf, bool opt_extract_without_path, bool opt_overwrite, const char *password, bool dbExtract, string newDbName, string *oldDbName)
{
	uLong i;
	unz_global_info64 gi;
	//FILE *fout = NULL;

	int err = unzGetGlobalInfo64(uf, &gi);
	if (err != UNZ_OK) {
		Logger::error << "unzGetGlobalInfo error " << err << endl;
		return err;
	}

	for (i = 0; i < gi.number_entry; i++) {
		if ((err = do_extract_currentfile(uf, opt_extract_without_path, opt_overwrite, password, dbExtract, newDbName, oldDbName)) != UNZ_OK) {
			return err;
		}

		if ((i + 1) < gi.number_entry) {
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK) {
				Logger::error << "error with zipfile in unzGoToNextFile " << err << endl;
				return err;
			}
		}
	}

	return UNZ_OK;
}

void ZipUtils::checkZipDbValidity(unzFile uf)
{
	uLong i;
	unz_global_info64 gi;

	int err = unzGetGlobalInfo64(uf, &gi);
	if (err != UNZ_OK) {
		throw ErrorException(ErrorException::ERROR_ZIP, "cannot check validity of file (unzGetGlobalInfo)");
	}

	string dbName;
	bool databaseCsv = false;

	for (i = 0; i < gi.number_entry; i++) {
		checkBackupFileValidity(uf, dbName, databaseCsv);

		if ((i + 1) < gi.number_entry) {
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK) {
				throw ErrorException(ErrorException::ERROR_ZIP, "cannot check validity of file (unzGoToNextFile)");
			}
		}
	}

	if (!databaseCsv) {
		throw ErrorException(ErrorException::ERROR_ZIP, "not a valid backup file, database.csv file not found");
	}
}

void ZipUtils::checkBackupFileValidity(unzFile uf, string &dbName, bool &databaseCsv)
{
	char filename_inzip[256];
	char *filename_withoutpath;

	unz_file_info64 file_info;
	int err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err != UNZ_OK) {
		throw ErrorException(ErrorException::ERROR_ZIP, "cannot check validity of file (unzGetCurrentFileInfo64)");
	}

	char *p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0') {
		if (((*p) == '/') || ((*p) == '\\')) {
			filename_withoutpath = p+1;
		}
		p++;
	}

	string str_filename_inzip = filename_inzip;;
	string folderName = str_filename_inzip;

	while (folderName.length() > 0 && (folderName[0] == '.' || folderName[0] == '/' || folderName[0] == '\\')) {
		folderName = folderName.substr(1, string::npos);
	}

	size_t i = folderName.find_first_of("/\\", 0);
	if (i != string::npos) {
		string rest = folderName.substr(i + 1, folderName.length() - i);

		size_t j = rest.find_first_of("/\\", 0);
		if (j != string::npos) {
			throw ErrorException(ErrorException::ERROR_ZIP, "not a valid backup file, subfolders found");
		}

		folderName = folderName.substr(0, i);
	}

	if (UTF8Comparer::compare(folderName.c_str(), Server::NAME_SYSTEM_DATABASE) == 0) {
		// System DB can be ignored
		return;
	}

	if (dbName != "" && (UTF8Comparer::compare(dbName.c_str(), folderName.c_str()) != 0)) {
		throw ErrorException(ErrorException::ERROR_ZIP, "not a valid backup file, multiple folders found");
	}

	dbName = folderName;

	if ((*filename_withoutpath) == '\0') {
		Logger::trace << "found directory " << filename_inzip << endl;
		return;
	} else {
		string filename = filename_withoutpath;
		if (UTF8Comparer::compare(filename_withoutpath, "database.csv") == 0) {
			databaseCsv = true;
			return;
		}
	}
}

void ZipUtils::extractZip(string zipFileName, string targetFolder, bool dbExtract, string newDbName, string *oldDbName)
{
	const char *zipfilename = zipFileName.c_str();
    unzFile uf = NULL;

	if (zipfilename != NULL) {
#ifdef _MSC_VER
		zlib_filefunc64_def ffunc;
#endif

#ifdef _MSC_VER
		fill_win32_filefunc64A(&ffunc);
		uf = unzOpen2_64(zipfilename, &ffunc);
#else
		uf = unzOpen64(zipfilename);
#endif
	}

	if (uf == NULL) {
		throw ParameterException(ErrorException::ERROR_ZIP, "cannot open filename", "filename", zipFileName);
	}
	Logger::trace << zipFileName << " opened." << endl;


	if (targetFolder != "") {
		throw ErrorException(ErrorException::ERROR_ZIP, "extracting outside Data (current) folder not yet implemented");
#ifdef _MSC_VER
		if (_chdir(targetFolder.c_str())) {
#else
		if (chdir(targetFolder.c_str())) {
#endif
			throw ParameterException(ErrorException::ERROR_ZIP, "error changing into ", "dirname", targetFolder);
		}
	}

	int err = do_extract(uf, false, false, NULL, dbExtract, newDbName, oldDbName);

	unzClose (uf);

	if (err != UNZ_OK) {
		if (err == UNZ_ALREADYEXISTS) {
			throw ErrorException(ErrorException::ERROR_ZIP, "extract failed, target already exists");
		} else {
			throw ErrorException(ErrorException::ERROR_ZIP, "extract failed (error " + StringUtils::convertToString(err) + ")");
		}
	}
}

void ZipUtils::checkZipDbValidity(string zipFileName)
{
	const char *zipfilename = zipFileName.c_str();
	unzFile uf = NULL;

	if (zipfilename != NULL) {
#ifdef _MSC_VER
		zlib_filefunc64_def ffunc;
#endif

#ifdef _MSC_VER
		fill_win32_filefunc64A(&ffunc);
		uf = unzOpen2_64(zipfilename, &ffunc);
#else
		uf = unzOpen64(zipfilename);
#endif
	}

	if (uf == NULL) {
		throw ParameterException(ErrorException::ERROR_ZIP, "cannot open filename", "filename", zipFileName);
	}
	Logger::trace << zipFileName << " opened." << endl;

	try {
		checkZipDbValidity(uf);
	} catch (...) {
		unzClose (uf);
		throw;
	}

}

}
