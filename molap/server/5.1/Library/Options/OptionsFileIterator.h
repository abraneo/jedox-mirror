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

#ifndef OPTIONS_OPTIONS_FILE_ITERATOR_H
#define OPTIONS_OPTIONS_FILE_ITERATOR_H 1

#include "palo.h"

#include "Options/OptionsIterator.h"
#include "InputOutput/FileUtils.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief iterate through an init
///
/// Class to iterate through an init file. The file is parsed and the tokens
/// are generated. Lines beginning with "#" are treated as comments.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS OptionsFileIterator : public OptionsIterator {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs iterator from null-terminated argument list
	////////////////////////////////////////////////////////////////////////////////

	OptionsFileIterator(const string& filename);

	virtual ~OptionsFileIterator() {
	}

public:
	const string * getCurrent() const {
		return (index == arguments.size()) ? 0 : &arguments[index];
	}

	void advance() {
		if (index != arguments.size()) {
			index++;
		}
	}

	const string * operator()() {
		return (index == arguments.size()) ? 0 : &arguments[index++];
	}

private:
	void parseFile(FileUtils::paloifstream&);
	void parseLine(const string& line);
	string nextToken(const string& line, size_t& pos);

private:
	size_t index;
	vector<string> arguments;
};

}

#endif
