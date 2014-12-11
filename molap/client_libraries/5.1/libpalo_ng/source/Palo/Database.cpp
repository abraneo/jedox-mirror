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

#include <string>
#include <sstream>
#include <vector>

#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Palo/Network/PaloClient.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "ServerImpl.h"

#include "Exception/TokenOutdatedException.h"
#include "Exception/CacheInvalidException.h"

#define CONFIGURATIONCUBE "#_CONFIGURATION"
#define CLIENTCACHE "ClientCache"

namespace jedox {
namespace palo {

/*!
 * \brief
 * little helper functions that return the id or ids of dimensions
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
struct DataBaseHelper {

	typedef std::vector<std::string> DIMENSION_NAME_LIST;

	static inline const long getDimID(boost::shared_ptr<const SIDimensions> cache, const std::string& dimension_name)
	{
		return (*cache)[dimension_name].dimension;
	}

	static inline DIMENSION_LIST getDimIDs(boost::shared_ptr<const SIDimensions> cache, const DIMENSION_NAME_LIST& dimensions)
	{
		DIMENSION_LIST return_vec;
		DIMENSION_NAME_LIST::size_type dim_ids_size = dimensions.size();
		return_vec.reserve(dim_ids_size);
		for (DIMENSION_NAME_LIST::size_type i = 0; i < dim_ids_size; ++i) {
			return_vec.push_back(getDimID(cache, dimensions[i]));
		}
		return return_vec;
	}
};

Database::Database(boost::shared_ptr<ServerImpl> serverImpl, boost::shared_ptr<PaloClient> paloClient, unsigned int db) :
	m_ServerImpl(serverImpl), m_PaloClient(paloClient), m_Database(db), m_Cache(serverImpl->getDatabases(m_PaloClient)), cube(m_ServerImpl, m_PaloClient, db), dimension(serverImpl, m_PaloClient, db)
{
}

void Database::createDimension(const std::string& Name, unsigned int type)
{
	std::stringstream query;

	unsigned int dummy, sequencenumber = 0;
	DIMENSION_TOKEN token(sequencenumber);

	query << "database=" << m_Database << "&new_name=";
	jedox::util::URLencoder(query, Name);
	query << "&type=" << type;
	try {
		m_PaloClient->request("/dimension/create", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

void Database::createCube(const std::string& Name, const std::vector<std::string> dimensions, unsigned int type)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	CUBE_TOKEN token(sequencenumber);

	query << "database=" << m_Database << "&new_name=";
	jedox::util::URLencoder(query, Name);
	query << "&dimensions=";
	jedox::util::TListe(query, DataBaseHelper::getDimIDs(m_ServerImpl->getDimensions(m_PaloClient, m_Database), dimensions), ',', false);
	query << "&type=" << type;
	try {
		m_PaloClient->request("/cube/create", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Database);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

DATABASE_INFO_PERMISSIONS Database::getInfo(bool showPermission)
{
	std::stringstream query;
	query << "database=" << m_Database;
	if (showPermission) {
		query << "&show_permission=1";
	} else {
		query << "&show_permission=0";
	}

	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	DATABASE_INFO_PERMISSIONS di;

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/info", query.str(), token, sequencenumber, dummy);
		(*stream) >> csv >> di;
		return di;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

void Database::rename(const std::string& newName)
{
	std::stringstream query;

	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	query << "database=" << m_Database << "&new_name=";
	jedox::util::URLencoder(query, newName);
	try {
		m_PaloClient->request("/database/rename", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDatabases(sequencenumber);
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

bool Database::destroy()
{
	char result;
	std::stringstream query;
	query << "database=" << m_Database;

	unsigned int dummy, sequencenumber = m_Cache->getSequenceNumber();
	SERVER_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/destroy", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDatabases(sequencenumber);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
	return result == '1';
}

bool Database::save(std::string path, bool complete) const
{
	char result;
	std::stringstream query;
	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	query << "database=" << m_Database
		<< "&complete=" << (complete ? "1" : "0");
	if (path != "") {
		query << "&external_identifier=" << path;
	}
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/save", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Database);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
	return result == '1';
}

bool Database::load() const
{
	char result;
	std::stringstream query;
	query << "database=" << m_Database;
	const DatabaseCacheB &db = (*m_Cache)[m_Database];

	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/load", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Database);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
	return result == '1';
}

bool Database::unload() const
{
	char result;
	std::stringstream query;
	query << "database=" << m_Database;
	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/unload", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateCubes(sequencenumber, m_Database);
		m_ServerImpl->invalidateDimensions(sequencenumber, m_Database);
		(*stream) >> result;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
	return result == '1';
}

const DIMENSION_INFO_PERMISSIONS_LIST Database::getDimensionList(bool showNormal, bool showSystem, bool showAttribute, bool showUserInfo, bool showPermissions) const
{
	std::stringstream query;
	query << "database=" << m_Database;
	query << "&show_normal=" << (showNormal ? "1" : "0")
			<< "&show_system=" << (showSystem ? "1" : "0")
			<< "&show_attribute=" << (showAttribute ? "1" : "0")
			<< "&show_info=" << (showUserInfo ? "1" : "0")
			<< "&show_permission=" << (showPermissions ? "1" : "0");

	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/dimensions", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDatabases(sequencenumber);
		DIMENSION_INFO_PERMISSIONS_LIST dimList;
		(*stream) >> csv >> dimList;
		return dimList;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

const CUBE_INFO_PERMISSIONS_LIST Database::getCubeList(jedox::palo::CUBE_INFO::TYPE type, bool showPermissions) const
{
	std::stringstream query;
	query << "database=" << m_Database;
	query << "&show_normal=" << (type == 0 ? "1" : "0")
			<< "&show_system=" << (type == 1 ? "1" : "0")
			<< "&show_attribute=" << (type == 2 ? "1" : "0")
			<< "&show_info=" << (type == 3 ? "1" : "0")
			<< "&show_gputype=" << (type == 4 ? "1" : "0")
			<< "&show_permission=" << (showPermissions ? "1" : "0");

	const DatabaseCacheB &db = (*m_Cache)[m_Database];
	unsigned int dummy, sequencenumber = db.getSequenceNumber();
	DATABASE_TOKEN token(sequencenumber);

	try {
		std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/database/cubes", query.str(), token, sequencenumber, dummy);
		m_ServerImpl->invalidateDatabases(sequencenumber);
		CUBE_INFO_PERMISSIONS_LIST cubeList;
		(*stream) >> csv >> cubeList;
		return cubeList;
	} catch (const TokenOutdatedException& e) {
		m_ServerImpl->invalidateDatabases(sequencenumber);
		throw e;
	}
}

const DATABASE_INFO& Database::getCacheData() const
{
	return (*m_Cache)[m_Database];
}

const DATABASE_INFO Database::getCacheDataCopy() const
{
	return getCacheData();
}

unsigned int Database::getSequenceNumberFromCache() const
{
	return (*m_Cache)[m_Database].getSequenceNumber();
}

std::string Database::getClientCacheMode()
{
	try {
		CELL_VALUE cv;
		cube[CONFIGURATIONCUBE].CellValue(cv, std::vector<std::string>(1, CLIENTCACHE), 0, 0, true);
		assert( cv.type == CELL_VALUE::STRING );
		return cv.val.s;
	} catch (const CubeNotFoundException&) {
		return "N";
	}

}

void Database::setClientCacheMode(const std::string& s)
{
	assert( s == "Y" || s == "N" || s == "E" );
	CELL_VALUE cv;
	cv.type = CELL_VALUE::STRING;
	cv.val.s = s;

	try {
		cube[CONFIGURATIONCUBE].CellReplace(std::vector<std::string>(1, "ClientCache"), cv, MODE_SPLASH_NONE);
	} catch (const CubeNotFoundException&) {
	}
}

} /* palo */
} /* jedox */
