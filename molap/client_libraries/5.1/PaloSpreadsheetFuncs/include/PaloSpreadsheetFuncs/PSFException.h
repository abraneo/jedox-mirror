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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4251)
#endif

#include <exception>
#include <string>
#include <sstream>

#undef GetMessage

namespace Palo {
namespace Types {

class SourceLocation {
private:
	std::string file;
	unsigned int line;

public:
	SourceLocation(const std::string& file, unsigned int line) :
			file(file), line(line)
	{
	}

	std::string GetLocation() const
	{
		std::ostringstream ss;
		ss << file << ':' << line;
		return ss.str();
	}
};

/*! \brief The current source location.
 */
#define CurrentSourceLocation SourceLocation(__FILE__, __LINE__)

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Exception class
 *
 *  This class is used to describe exceptions. All other exceptions should be
 *  derived from this class and override the GetType() and GetMessage() members.
 *  Usually you will want to pass CurrentSourceLocation to the constructor of this class.
 */
class PSFException : public std::exception {
private:
	std::string msg;
	SourceLocation loc;

protected:
	PSFException(const SourceLocation& location, const std::string& message) :
			msg(message), loc(location)
	{
	}

	PSFException(const SourceLocation& location) :
			loc(location)
	{
	}

public:
	/*! \brief Returns a type string like "InvalidArgumentException"
	 */
	virtual const std::string& GetType() const
	{
		static const std::string type = "Exception";
		return type;
	}

	virtual const std::string& GetMessage() const
	{
		return msg;
	}

	virtual std::string GetLocation() const
	{
		return loc.GetLocation();
	}

	virtual const char * what() const throw ()
	{
		return GetMessage().c_str();
	}

	~PSFException() throw ()
	{
	}
};
}
}
#endif
