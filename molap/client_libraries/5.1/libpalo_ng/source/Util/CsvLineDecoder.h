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

#pragma once

// To be replaced with http://www.boost.org/libs/tokenizer/escaped_list_separator.htm

#include <string>
#include <vector>

namespace jedox {
namespace util {

/*!
 * \brief
 * Represents an single field of an CVS.
 *
 * Represents an single field in an CSV.
 * - token represents the parsed value
 * - IsNumeric indicates if it's of numeric type or an string
 *
 * \see
 * Separate items with the '|' character.
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
typedef struct tCSV_TOKEN {
	tCSV_TOKEN(const std::string& token, bool isNumeric, size_t startPos)
	{
		this->token = token;
		this->IsNumeric = isNumeric;
		this->startPos = startPos;
	}
	std::string token;
	bool IsNumeric;
	size_t startPos;
} TOKEN;

/*!
 * \brief
 * Represents a list of tokens representing fields in their
 * order of appearance in the CSV.
 *
 * \see
 * TOKEN | CsvLineDecoder
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
typedef std::vector<TOKEN> TOKEN_LIST;

/*!
 * \brief
 * Splits a line of CSV Data into fields represented by a TOKEN_LIST
 *
 * \see
 * TOKEN_LIST | TOKEN
 * \author Florian Schaper <florian.schaper@jedox.com>
 */
class CsvLineDecoder {
public:
	/*!
	 * \brief
	 * Splits a line of CSV data into their individual field-tokens and returns
	 * a list of those tokens back to the caller.
	 *
	 * \param line
	 * std::string with a line of CSV data separated by the seperator character
	 *
	 * \param seperator
	 * character by which the individual fields are separated
	 *
	 * \returns
	 * TOKEN_LIST Representation of fields in the CSV line. The first field in the CSV is
	 * the first element in the TOKEN_LIST.
	 *
	 * \see
	 * TOKEN_LIST | TOKEN
	 * \author Florian Schaper <florian.schaper@jedox.com>
	 */
	static TOKEN_LIST decode(const std::string& line, const char seperator = ';');
};

} /* namespace util */
} /* namespace  jedox */
