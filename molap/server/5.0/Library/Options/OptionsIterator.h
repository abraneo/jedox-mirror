/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#ifndef OPTIONS_OPTIONS_ITERATOR_H
#define OPTIONS_OPTIONS_ITERATOR_H 1

#include "palo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract class to iterate through options and arguments
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS OptionsIterator {
public:
	OptionsIterator() {
	}

	virtual ~OptionsIterator() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the current item
	///
	/// Returns the current item in the iterator without advancing on to the next
	/// item. If we are at the end of items then 0 is returned.
	////////////////////////////////////////////////////////////////////////////////

	virtual const string * getCurrent() const = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief advances to the next item.
	////////////////////////////////////////////////////////////////////////////////

	virtual void advance() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the current item
	///
	/// Returns the current item in the iterator and then advances on to the next
	/// item. If we are at the end of items then 0 is returned.
	////////////////////////////////////////////////////////////////////////////////

	virtual const string * operator()() {
		const string* element = getCurrent();
		advance();
		return element;
	}

private:
	OptionsIterator(const OptionsIterator&);
	OptionsIterator& operator=(const OptionsIterator&);
};

}

#endif
