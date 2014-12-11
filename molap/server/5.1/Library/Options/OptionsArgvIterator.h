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

#ifndef OPTIONS_OPTIONS_ARGV_ITERATOR_H
#define OPTIONS_OPTIONS_ARGV_ITERATOR_H 1

#include "palo.h"

#include "Options/OptionsIterator.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief iterate through an array of tokens
///
/// Class to iterate through an array of tokens. The array may be terminated
/// by null or a count containing the number of tokens may be given.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS OptionsArgvIterator : public OptionsIterator {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs iterator from null-terminated argument list
	////////////////////////////////////////////////////////////////////////////////

	OptionsArgvIterator(const char * const argv[]) :
		index(0) {
		for (int i = 0; argv[i] != 0; i++) {
			arguments.push_back(argv[i]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs iterator from argument list
	////////////////////////////////////////////////////////////////////////////////

	OptionsArgvIterator(int argc, const char * const argv[]) :
		index(0) {
		for (int i = 0; i < argc; i++) {
			if (argv[i] != 0) {
				arguments.push_back(argv[i]);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~OptionsArgvIterator() {
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

	size_t size() const {
		return arguments.size() - index;
	}

	const string * operator()() {
		return (index == arguments.size()) ? 0 : &arguments[index++];
	}

private:
	size_t index;
	vector<string> arguments;
};

}

#endif
