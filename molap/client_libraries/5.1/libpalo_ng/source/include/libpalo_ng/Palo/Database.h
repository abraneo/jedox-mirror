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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4251)
#endif

#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "Dimensions.h"
#include "Cubes.h"

namespace jedox {
namespace palo {

class ServerImpl;
class SIDatabases;

/** @brief
 *  Class for performing database operations.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Database {
public:

	/** @brief
	 *  Do NOT use explicitly.
	 *  Use Server::CreateDatabase.
	 */
	explicit Database(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, IdentifierType db);

	/** @brief
	 *  Get information about this database in a structure of type DATABASE_INFO.
	 *
	 *  @return DATABASE_INFO for this database.
	 */

	const DATABASE_INFO& getCacheData() const;

	/** @brief
	 *  Get information about this database in a structure of type DATABASE_INFO.
	 *
	 *  @return DATABASE_INFO for this database.
	 */

	const DATABASE_INFO getCacheDataCopy() const;

	/** @brief
	 *  Add a dimension to this DB.
	 *
	 *  @param Name : Name of new dimension
	 *  @param type : type of the dimension
	 */
	void createDimension(const std::string& Name, unsigned int type = DIMENSION_INFO::NORMAL);

	/** @brief
	 *  Add a cube to this DB.
	 *
	 *  @param Name : Name of new cube
	 *  @param dimensions : list of the dimensions of the cube
	 *  @param type : type of the cube
	 */
	void createCube(const std::string& Name, const std::vector<std::string> dimensions, unsigned int type = CUBE_INFO::NORMAL);

	/** @brief
	 *  Give this DB a new name
	 *
	 *  @param newName : Name that replaces the old one.
	 */
	void rename(const std::string& newName);

	/** @brief
	 *  completely remove this database
	 *
	 *  @return : True if success, false otherwise
	 */
	bool destroy();

	/** @brief
	 *  save this database to disk
	 *
	 *  @return : True if success, false otherwise
	 */
	bool save(std::string path = "", bool complete = false) const;

	/** @brief
	 *  reload this database from disk
	 *
	 *  @return : True if success, false otherwise
	 */
	bool load() const;

	/** @brief
	 *  completely unload this database
	 *
	 *  @return : True if success, false otherwise
	 */
	bool unload() const;

	DATABASE_INFO_PERMISSIONS getInfo(bool showPermission);

	unsigned int getSequenceNumberFromCache() const;

	std::string getClientCacheMode();

	void setClientCacheMode(const std::string&);

	const DIMENSION_INFO_PERMISSIONS_LIST getDimensionList(bool showNormal, bool showSystem, bool showAttribute, bool showUserInfo, bool showPermissions) const;
	const CUBE_INFO_PERMISSIONS_LIST getCubeList(jedox::palo::CUBE_INFO::TYPE type, bool showPermissions) const;

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	IdentifierType m_Database;
	boost::shared_ptr<const SIDatabases> m_Cache;

public:
	/** @brief
	 *  The cube object allows to access the Cubes of this DB using the [] operator
	 */
	Cubes cube;

	/** @brief
	 *  The dimension object allows to access the Dimensions this DB using the [] operator
	 */
	Dimensions dimension;

};

} /* palo */
} /* jedox */

#endif							 // DATABASE_H
