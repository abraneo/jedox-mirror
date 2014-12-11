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
 * \author Martin Dolezal <martin.dolezal@qbicon.cz>
 * 
 *
 */

#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include <libpalo_ng/Palo/types.h>

namespace Palo {
namespace Types {
/*! \author Martin Dolezal <martin.dolezal@qbicon.cz>
 *  \brief Stores information about a database.
 */
struct DatabaseInfo {
public:

	DatabaseInfo()
	{
	}

	DatabaseInfo(jedox::palo::DATABASE_INFO di)
	: name(di.ndatabase), type(di.type), status(di.status), dimensionCount(di.number_dimensions), cubeCount(di.number_cubes)
	{
	}

	std::string name;
	jedox::palo::DATABASE_INFO::TYPE type;
	jedox::palo::DATABASE_INFO::STATUS status;
	unsigned int dimensionCount;
	unsigned int cubeCount;
	std::string permission;
};

typedef std::vector<DatabaseInfo> DatabaseInfoArray;

}
}
#endif
