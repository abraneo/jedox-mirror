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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 *
 *
 */

#ifndef LOCK_INFO_H
#define LOCK_INFO_H

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Palo/Cube.h>
#include <libpalo_ng/Palo/Dimension.h>

namespace Palo {
namespace Types {

struct LockInfo {
	LockInfo()
	{
	}

	LockInfo(boost::shared_ptr<jedox::palo::Server> s, const std::string &database, const std::string &cube, const jedox::palo::LOCK_INFO &li) :
		lockid(li.lockid), user(li.user), steps(li.steps)
	{
		area.resize(li.area.size());
		jedox::palo::Database db = (*s)[database];
		jedox::palo::Cube cb = db.cube[cube];
		const jedox::palo::DIMENSION_LIST &dims = cb.getCacheData().dimensions;
		for (size_t i = 0; i < li.area.size(); ++i) {
			jedox::palo::Dimension dim = db.dimension[dims[i]];
			area[i].reserve(li.area[i].size());
			for (std::vector<long>::const_iterator it = li.area[i].begin(); it != li.area[i].end(); ++it) {
				area[i].push_back(dim[*it].getCacheData().nelement);
			}
		}
	}

	UINT lockid;
	std::vector<std::vector<std::string> > area;
	std::string user;
	ULONG steps;
};

}
}
#endif
