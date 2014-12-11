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

#ifndef GENERIC_ARGUMENT_ARRAY_H
#define GENERIC_ARGUMENT_ARRAY_H

#include <memory>
#include <vector>

#include <PaloSpreadsheetFuncs/GenericCell.h>

namespace Palo {
namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief A list of GenericCell pointers with some special features.
 */
class GenericArgumentArray {
public:
	GenericArgumentArray(std::vector<GenericCell*>& args);
	~GenericArgumentArray();

	/*! \brief Retrieve an argument.
	 *
	 *  Returns a reference to the GenericCell referenced by the GenericCell pointer
	 *  stored at the requested index.
	 */
	GenericCell& operator [](size_t idx);

	/*! \brief Length of argument list.
	 */
	size_t length() const;

	/*! \brief Collapse the entire list into one argument.
	 *
	 *  Converts the entire list into an SimpleCell::ArrayCell containing
	 *  all the former arguments.
	 */
	void collapseToArray(unsigned int from_idx);

	/*! \brief Rectify passed connection and/or database.
	 *
	 *  Some Palo API's (e.g. PHPPalo) support passing the connection and database as
	 *  one argument or separetely from eachother. This function assures that the second
	 *  will be the case by splitting the argument at idx into two argument where necessary.
	 */
	void fixConnection(unsigned int);

	/*! \brief Assert argument count.
	 *
	 *  This function asserts that the argument list consists of exactly num_args. If not
	 *  it will throw an WrongParameterCountException.
	 */
	void checkArgCount(unsigned int num_args);

	/*! \brief Shift the argument list.
	 *
	 *  Shift the argument list left by offset entries. Entries "shifted out" to the left will
	 *  be appended at the end of the list.
	 */
	void lshift(unsigned int offset);

	/*! \brief Shift the argument list.
	 *
	 *  Shift the argument list right by offset entries. Entries "shifted out" to the right will
	 *  be appended at the front of the list.
	 */
	void rshift(unsigned int offset);
	void insert(unsigned int idx, std::string val);

private:
	// non-copyable
	GenericArgumentArray(const GenericArgumentArray&);

	std::vector<GenericCell*>& args;
	bool connection_fixed;
};
}
}
#endif
