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
 * \author Florian Schaper <florian.schaper@jedox.com>
 * 
 *
 */

#include <istream>
#include "CsvTokenFromStream.h"

namespace jedox {
namespace util {

CsvTokenFromStream::CsvTokenFromStream(std::istream& stream, char seperator) :
	m_Stream(stream), m_Seperator(seperator), m_Done(false)
{
}

std::ostream& operator<<(std::ostream& out, CsvTokenFromStream& tokenStream)
{
	std::string token;
	tokenStream.get(token);
	out << token;
	return out;
}

} /* util */
} /* jedox */
