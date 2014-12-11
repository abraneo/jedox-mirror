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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/Cubes.h>
#include "ServerImpl.h"
#include <libpalo_ng/Palo/Cube.h>

namespace jedox {
namespace palo {

Cubes::Cubes(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_db(db)
{
}

Cube Cubes::operator[](const std::string& name)
{
	return Cube(m_ServerImpl, m_PaloClient, m_db, m_ServerImpl->getCube(m_PaloClient, m_db, name).cube);
}

Cube Cubes::operator[](unsigned int id)
{
	return Cube(m_ServerImpl, m_PaloClient, m_db, m_ServerImpl->getCube(m_PaloClient, m_db, id).cube);
}

bool Cubes::Exists(const std::string& name)
{
	return m_ServerImpl->existCube(m_PaloClient, m_db, name);
}

bool Cubes::Exists(unsigned int id)
{
	return m_ServerImpl->existCube(m_PaloClient, m_db, id);
}

std::unique_ptr<CubesCache::CacheIterator> Cubes::getIterator()
{
	return m_ServerImpl->getCubeIterator(m_PaloClient, m_db);
}

} /* palo */
} /* jedox */
