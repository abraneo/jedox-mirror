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

#include <libpalo_ng/Palo/Dimensions.h>
#include "ServerImpl.h"
#include <libpalo_ng/Palo/Network/PaloClient.h>

#include "Exception/CacheInvalidException.h"

namespace jedox {
namespace palo {

Dimensions::Dimensions(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int database) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_DatabaseCache(database)
{
}

Dimension Dimensions::operator[](const std::string& name)
{
	return Dimension(m_ServerImpl, m_PaloClient, m_DatabaseCache, m_ServerImpl->getDim(m_PaloClient, m_DatabaseCache, name).dimension);
}

Dimension Dimensions::operator[](unsigned int id)
{
	return Dimension(m_ServerImpl, m_PaloClient, m_DatabaseCache, m_ServerImpl->getDim(m_PaloClient, m_DatabaseCache, id).dimension);
}

bool Dimensions::Exists(const std::string& name)
{
	return m_ServerImpl->existDim(m_PaloClient, m_DatabaseCache, name);
}

bool Dimensions::Exists(unsigned int id)
{
	return m_ServerImpl->existDim(m_PaloClient, m_DatabaseCache, id);
}

std::unique_ptr<DimensionsCache::CacheIterator> Dimensions::getIterator()
{
	return m_ServerImpl->getDimIterator(m_PaloClient, m_DatabaseCache);
}

} /* palo */
} /* jedox */
