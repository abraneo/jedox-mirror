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

#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include <string>

#include "Dimension.h"
#include "Cache/DimensionsCache.h"

namespace jedox {
namespace palo {

class ServerImpl;

/** @brief
 *  Helper class that gives access to the dimensions of a database
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Dimensions {
public:

	/** @brief
	 *  Do NOT use explicitly.
	 */
	explicit Dimensions(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, IdentifierType database);

	/** @brief
	 *  get a Dimension Object by its name.
	 *
	 *  @param name : name of the dimension
	 *
	 *  @return Dimension Object for the dimension with the name "name"
	 */
	Dimension operator[](const std::string& name);

	/** @brief
	 *  get a Dimension Object by its id.
	 *
	 *  @param id : id of the dimension
	 *
	 *  @return Dimension Object for the dimension with the id "id"
	 */
	Dimension operator[](IdentifierType id);

	/** @brief
	 *  Check the existence of the dimension with name 'name'
	 */
	bool Exists(const std::string& name);

	/** @brief
	 *  Check the existence of the dimension with id 'id'
	 */
	bool Exists(IdentifierType id);

	/** @brief
	 *  Get an iterator for the dimensions.
	 */
	std::unique_ptr<DimensionsCache::CacheIterator> getIterator();

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	IdentifierType m_DatabaseCache;
};

} /* palo */
} /* jedox */

#endif							 // DIMENSIONS_H
