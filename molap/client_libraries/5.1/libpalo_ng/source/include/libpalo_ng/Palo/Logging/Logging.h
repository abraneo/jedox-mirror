/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 *
 */

#ifndef PALO_LOGGING_H
#define PALO_LOGGING_H

#include <sstream>
#include <string>
#include <iosfwd>

#include <boost/scoped_ptr.hpp>

#define TO_STRING( text ) #text
#define LOG_LINE( logText ) COMMIT_TO_LOG(logText, __FILE__, __LINE__)
#define COMMIT_TO_LOG( logText, sourceFile, sourceLine ) { std::stringstream logStream; logStream << logText << " (File: " << sourceFile "/Line: " << TO_STRING( sourceLine ) << ")"; Logging& logger = Logging::getInstance(); logger.log( logStream.str() ); }

class Logging {
public:
	static Logging& getInstance()
	{
		static Logging instance;
		return instance;
	}

	void log(const std::string& log);

private:
	Logging();
	boost::scoped_ptr<std::ostream> m_Stream;
};
#endif							 // PALO_LOGGING_H
