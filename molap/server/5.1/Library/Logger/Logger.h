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

#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H 1

#include "palo.h"

#include <iostream>
#include <sstream>

extern "C" {
#include <time.h>
#ifdef PALO_HMMM_FINER_LOGGING
#include <sys/time.h>
#endif
}

#include "Thread/WriteLocker.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief logger
///
/// This class provides various static members which can be used as logging
/// streams. Output to the logging stream is appended by using the operator <<,
/// as soon as a line is completed endl should be used to flush the stream.
/// Each line of output is prefixed by some informational data.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Logger {
public:
	class LoggerStream {
	public:
		LoggerStream(ostream& output) :
			on(true), output(&output), duplicateToStdout(false) {
		}

	public:
		template<typename T>
		LoggerStream& operator<<(const T& value) {
			WriteLocker w(&m);
			if (on) {
				(*output) << value;
				if (duplicateToStdout) {
					cout << value;
				}
				output->flush();
			}
			return *this;
		}

#if defined(_MSC_VER)
		template<>
		LoggerStream& operator<< (const socket_t& value) {
			WriteLocker w(&m);
			if (on) {
				(*output) << (unsigned int) value;
				if (duplicateToStdout) {
					cout << (unsigned int) value;
				}
			}
			return *this;
		}

/*
		LoggerStream& operator<< (const size_t& value) {
			WriteLocker w(&m);
			if (on) {
				(*output) << (unsigned int) value;
			}
			return *this;
		}
*/
#endif

		LoggerStream& operator<<(ostream& (*fptr)(ostream&)) {
			WriteLocker w(&m);
			if (on) {
				fptr((*output));
				if (duplicateToStdout) {
					fptr(cout);
				}
				output->flush();
			}
			return *this;
		}

		void activate() {
			WriteLocker w(&m);
			on = true;
		}

		void deactivate() {
			WriteLocker w(&m);
			on = false;
		}

		void setStream(ostream& os, bool duplicateToStdout) {
			WriteLocker w(&m);
			output = &os;
			this->duplicateToStdout = duplicateToStdout;
		}

	private:
		Mutex m;
		bool on;
		ostream* output;
		bool duplicateToStdout;
	};

private:
	class LoggerStreamHeader {
	public:
		LoggerStreamHeader(const string& prefix, ostream&);

	public:
		template<typename T>
		LoggerStream& operator<<(const T& value) {

			if (on) {
#ifndef PALO_HMMM_FINER_LOGGING
				time_t tt = time(0);
				struct tm* t = localtime(&tt);
				int max = 32;
				char* s = new char[max];
				strftime(s, max, "%Y-%m-%d %H:%M:%S ", t);
				stream << s;
				/*if (duplicateToStdout) {
					cout << s;
				}*/
				delete[] s;
#endif
#ifdef PALO_HMMM_FINER_LOGGING
				struct timeval tv;
				gettimeofday(&tv, 0);
				stream << tv.tv_sec << "." << tv.tv_usec << " " << clock() << " ";
#endif

				stream << prefix << ": ";
				stream << value;
				/*if (duplicateToStdout) {
					cout << prefix << ": " << value;
				}*/
				logger.linesCount++;
			}

			return stream;
		}

		void activate() {
			on = true;
			stream.activate();
		}

		void deactivate() {
			on = false;
			stream.deactivate();
		}

		bool isActive() {
			return on;
		}

		void setStream(ostream& os, bool duplicateToStdout) {
			this->duplicateToStdout = duplicateToStdout;
			stream.setStream(os, duplicateToStdout);
		}

	private:
		bool on;
		const string prefix;
		LoggerStream stream;
		bool duplicateToStdout;
	};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief changes log level
	////////////////////////////////////////////////////////////////////////////////

	static void setLogLevel(const string& level);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief changes the log file
	///
	/// "-" means stdout, "+" means stderr
	////////////////////////////////////////////////////////////////////////////////

	static void setLogFile(const string& file, bool duplicateToStdout);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief retrieves the log file
	///
	/// stdout or stderr returns empty string
	////////////////////////////////////////////////////////////////////////////////

	static string getLogFile() {return logger.fileName;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief retrieves the size of the file when opened for append
	////////////////////////////////////////////////////////////////////////////////
	static streampos getZeroPos() {return logger.zeroPosition;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief retrieves the count of lines added to log in current session
	////////////////////////////////////////////////////////////////////////////////
	static uint64_t getLinesCount() {return logger.linesCount;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logger for error messages
	////////////////////////////////////////////////////////////////////////////////

	static LoggerStreamHeader error;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logger for warning messages
	////////////////////////////////////////////////////////////////////////////////

	static LoggerStreamHeader warning;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logger for info messages
	////////////////////////////////////////////////////////////////////////////////

	static LoggerStreamHeader info;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logger for debug messages
	////////////////////////////////////////////////////////////////////////////////

	static LoggerStreamHeader debug;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief logger for trace messages
	////////////////////////////////////////////////////////////////////////////////

	static LoggerStreamHeader trace;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief is enabled for debug messages
	////////////////////////////////////////////////////////////////////////////////

	static bool isDebug() {
		return debug.isActive();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief is enabled for trace messages
	////////////////////////////////////////////////////////////////////////////////

	static bool isTrace() {
		return trace.isActive();
	}

private:
	Logger();

private:
	static Logger logger;
	string fileName;
	streampos zeroPosition;
	uint64_t linesCount;
};

}

#endif
