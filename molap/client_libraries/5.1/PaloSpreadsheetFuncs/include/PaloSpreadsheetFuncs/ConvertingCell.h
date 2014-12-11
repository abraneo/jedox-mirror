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

#ifndef CONVERTING_CELL_H
#define CONVERTING_CELL_H

#include <PaloSpreadsheetFuncs/GenericCell.h>
#include <PaloSpreadsheetFuncs/GenericContext.h>

namespace Palo {
namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Converting cell.
 *
 *  This class extends GenericCell with the ability to convert between UTF-8 (internal)
 *  and some other (external) character encoding.
 */
class ConvertingCell : public GenericCell {
public:
	virtual ~ConvertingCell();

	GenericCell& set(const std::string& s);
	std::string getString();

protected:
	ConvertingCell(GenericContext& s);

	virtual GenericCell& setImpl(const std::string& s) = 0;
	virtual std::string getStringImpl() = 0;

	GenericContext& settings;
};
}
}
#endif
