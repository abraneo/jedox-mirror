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

#ifndef ELEMENT_LIST_H
#define ELEMENT_LIST_H

#include "StringArray.h"
#include "InvalidRequestException.h"

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Dimension element list.
 *
 *  This class represents a list of dimension elements. You can use it also to specify
 *  all elements of a dimension.
 */
class ElementList {
public:
	ElementList() :
			_All(false)
	{
	}

	/*! \brief Sets the list of dimension elements.
	 */
	void set(StringArray elements)
	{
		_All = false;
		Elements = elements;
	}

	/*! \brief Sets the list to all elements of a dimension.
	 */
	void setAll()
	{
		_All = true;
		Elements.empty();
	}

	/*! \brief Returns the list of dimension elements.
	 *  \warning Make sure that all() is false, otherwise the call will result in an exception.
	 */
	const StringArray &getArray() const
	{
		if (_All) {
			throw InvalidRequestException(CurrentSourceLocation);
		}
		return Elements;
	}

	/*! \brief Checks if the element list is set to all elements of a dimension (e.g. "*" passed by the user)
	 */
	bool all() const
	{
		return _All;
	}

private:
	bool _All;
	StringArray Elements;
};
}
}
#endif
