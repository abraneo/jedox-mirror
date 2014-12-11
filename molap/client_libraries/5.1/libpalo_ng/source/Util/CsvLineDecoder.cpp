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

#include <cassert>
#include "CsvLineDecoder.h"

namespace jedox {
namespace util {

TOKEN_LIST CsvLineDecoder::decode(const std::string& line, const char seperator)
{
	static std::string brace = "\"";
	TOKEN_LIST tokens;
	std::string::size_type cursor = 0;
	std::string::size_type tmpcursor = 0;
	std::string::size_type end = line.length();
	while (cursor < end) {
		// Eat leading whitespace. strips blank and tab character
		while (cursor < end && (line[cursor] == ' ' || line[cursor] == '\t')) {
			++cursor;
		};
		// Test for the beginning of a string token
		// TODO Might be faster if we look for "" combinations explicitly
		// and use find() for lookup than iterating by character. For now neglect able.
		if (line[cursor] == '"') {
			std::string token;
			size_t pos = cursor;
			// If the loop ends because of this condition
			// we have got ourself an invalid CSV string
			// TODO Error checking
			while (++cursor < end) {
				// Check for a escaped quote character (double quote)
				if (line[cursor] == '"' && line[cursor + 1] == '"') {
					++cursor;
					token.append(brace);
					// Check for an single quote (termination sequence)
				} else if (line[cursor] == '"') {
					// Find the next separation character and set the
					// cursor to the beginning of the next field (if any)
					tmpcursor = line.find_first_of(seperator, cursor);
					// Test if this is the last field we're in
					if (tmpcursor == std::string::npos) {
						cursor = end;
						// Set offset to the beginning of the next field
					} else {
						cursor = tmpcursor + 1;
					}
					// Quit the loop
					break;
					// Append the current character
				} else {
					token.append(line.substr(cursor, 1));
				}
			}
			// Add the string token to the list of tokens
			tokens.push_back(TOKEN(token, false, pos));
			// Is Numeric
		} else {
			// Search for the next separation character
			tmpcursor = line.find_first_of(seperator, cursor);
			// If we have reached the end of the string this is the last
			// field. Push everything back and break the loop.
			if (tmpcursor == std::string::npos) {
				tokens.push_back(TOKEN(line.substr(cursor, end - cursor), true, cursor));
				break;
			}
			// Push back from the current cursor position till just before the separation character
			tokens.push_back(TOKEN(line.substr(cursor, tmpcursor - cursor), true, cursor));
			cursor = tmpcursor + 1;
		}
	}
	return tokens;
}

} /* namespace util */
} /* namespace  jedox */
