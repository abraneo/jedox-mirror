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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef INPUT_OUTPUT_FILE_UTILS_H
#define INPUT_OUTPUT_FILE_UTILS_H 1

#include "palo.h"
#include <fstream>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "zip/zip.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief collections of file functions
////////////////////////////////////////////////////////////////////////////////

class FileUtils {
public:
#if defined(_MSC_VER)
	class wfilebuf : public std::streambuf {
		static const size_t BUF_SIZE = 4 * 1024;
	public:
		wfilebuf(std::string filename, std::ios::openmode mode);
		virtual ~wfilebuf();
		void close();
		bool is_open();
		virtual int sync();
		virtual int overflow(int c);
		virtual int underflow();
		virtual std::streamsize showmanyc();
	private:
		int __writetofile(size_t len);
		HANDLE hFile;
		unsigned char *readbuf;
		unsigned char *writebuf;
		__int64 remaining;
	};

	class paloofstream : public std::ostream {
	public:
		paloofstream(std::string filename, std::ios::openmode mode = std::ios::out);
		void close();
		bool is_open();
	private:
		wfilebuf _buf;
	};

	class paloifstream : public std::istream {
	public:
		paloifstream(std::string filename);
		void close();
		bool is_open();
	private:
		wfilebuf _buf;
	};
#else
	typedef std::ifstream paloifstream;
	typedef std::ofstream paloofstream;
#endif

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a new ifstream or 0
	////////////////////////////////////////////////////////////////////////////////
	static paloifstream *newIfstream(const std::string& filename);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a new ofstream or 0
	////////////////////////////////////////////////////////////////////////////////
	static paloofstream *newOfstream(const std::string& filename);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a new ofstream or 0
	////////////////////////////////////////////////////////////////////////////////
	static paloofstream *newOfstreamAppend(const std::string& filename);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a file is readable
	////////////////////////////////////////////////////////////////////////////////
	static bool isReadable(const FileName& fileName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a file could be removed
	////////////////////////////////////////////////////////////////////////////////
	static bool remove(const FileName& fileName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a file could be renamed
	////////////////////////////////////////////////////////////////////////////////
	static bool rename(const FileName& oldName, const FileName& newName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a file directory be renamed
	////////////////////////////////////////////////////////////////////////////////
	static bool renameDirectory(const FileName& oldName, const FileName& newName);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a directory could be removed
	////////////////////////////////////////////////////////////////////////////////
	static bool removeDirectory(const FileName& name);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a new directory
	////////////////////////////////////////////////////////////////////////////////
	static bool createDirectory(const std::string& name);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies a directory
	////////////////////////////////////////////////////////////////////////////////
	static bool copyDirectory(std::string fromDir, std::string toDir);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies a file
	////////////////////////////////////////////////////////////////////////////////
	static bool copyFile(const std::string& fromFile, const std::string &toFile);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns list of files
	////////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
	static std::vector<std::string> _stdcall listFiles(const std::string& directory);
#else
	static std::vector<std::string> listFiles(const std::string& directory);
#endif

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if path is a directory
	////////////////////////////////////////////////////////////////////////////////
	static bool isDirectory(const std::string& path);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if path is a regular file
	////////////////////////////////////////////////////////////////////////////////
	static bool isRegularFile(const std::string& path);
	static bool isRegularFile(const std::string& path, const std::string& name);

	static bool createFile(const std::string &dirName, const std::string &name);
};

}
#endif
