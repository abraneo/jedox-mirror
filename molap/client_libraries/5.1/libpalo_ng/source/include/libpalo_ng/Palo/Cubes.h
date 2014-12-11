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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4251)
#endif

#ifndef CUBES_H
#define CUBES_H

#include <memory>
#include <string>

#include "Cube.h"
#include "Cache/CubesCache.h"

namespace jedox {
namespace palo {

class ServerImpl;

/** @brief
 *  Helper class that gives access to the cubes of a database.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Cubes {
public:
	explicit Cubes(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db);

	/** @brief
	 *  get a Cube Object by its name
	 *
	 *  @return Cube Object with name "name"
	 */
	Cube operator[](const std::string& name);

	/** @brief
	 *  get a Cube Object by its id
	 *
	 *  @return Cube Object with id "id"
	 */
	Cube operator[](IdentifierType id);

	/** @brief
	 *  Check the existence of the cube with name 'name'
	 */
	bool Exists(const std::string& name);

	/** @brief
	 *  Check the existence of the cube with id 'id'
	 */
	bool Exists(IdentifierType id);

	/** @brief
	 *  Get an iterator for the cubes.
	 */
	std::unique_ptr<CubesCache::CacheIterator> getIterator();

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	IdentifierType m_db;
};

} /* palo */
} /* jedox */

#endif							 // CUBES_H
