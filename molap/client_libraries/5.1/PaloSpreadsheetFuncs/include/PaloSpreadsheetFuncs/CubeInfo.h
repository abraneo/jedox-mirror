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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#ifndef CUBE_INFO_H
#define CUBE_INFO_H

#include <libpalo_ng/Palo/types.h>

namespace Palo {
namespace Types {
/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief Stores information about a cube.
 */
struct CubeInfo {
public:
	enum CubeType {
		NORMAL = jedox::palo::CUBE_INFO::NORMAL, SYSTEM = jedox::palo::CUBE_INFO::SYSTEM, ATTRIBUTE = jedox::palo::CUBE_INFO::ATTRIBUTE, USERINFO = jedox::palo::CUBE_INFO::USERINFO, GPU = jedox::palo::CUBE_INFO::GPU
	};

	enum CubeStatus {
		UNLOADED = jedox::palo::CUBE_INFO::UNLOADED, LOADED = jedox::palo::CUBE_INFO::LOADED, CHANGED = jedox::palo::CUBE_INFO::CHANGED
	};

	CubeInfo()
	{
	}

	long int identifier;
	std::string name;
	unsigned int number_dimensions;
	StringArray dimensions;
	long double number_cells;
	long double number_filled_cells;
	CubeStatus status;
	CubeType type;
	std::string permission;
};

typedef std::vector<CubeInfo> CubeInfoArray;

}
}
#endif
