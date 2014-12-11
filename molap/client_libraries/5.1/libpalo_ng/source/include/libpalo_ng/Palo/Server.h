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

#ifndef LIBPALO_NG_SERVER_H
#define LIBPALO_NG_SERVER_H

#include <string>

#include <boost/shared_ptr.hpp>

#include "Cache/ServerCache.h"
#include <libpalo_ng/Palo/Network/PaloClient.h>


namespace jedox {
namespace palo {

class ServerImpl;
class Database;
class View;

/** @brief
 *  Class for performing Server operations.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT Server {
	friend class ServerPool;
	friend class View;
public:
	explicit Server(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient);
	~Server();
	Server(const Server& other);

	Server& operator=(const Server& other);

	/** @brief
	 *  Get a Database Object by its name.
	 *
	 *  @param name : name of the database
	 *
	 *  @return database object by its name
	 */
	Database operator[](const std::string& name);

	/** @brief
	 *  Get a Database Object by its id.
	 *
	 *  @param id : id of the database
	 *
	 *  @return database object by its id
	 */
	Database operator[](IdentifierType id);

	/** @brief
	 *  Check the existence of the database with name 'name'
	 */
	bool Exists(const std::string& name);

	/** @brief
	 *  Check the existence of the database with id 'id'
	 */
	bool Exists(IdentifierType id);

	/** @brief
	 *  Get all information about this server in a structure of type SERVER_INFO
	 *
	 *  @return SERVER_INFO for this server.
	 */
	const SERVER_INFO& getCacheData() const;

	/** @brief
	 *  Get all information about this server in a structure of type SERVER_INFO
	 *
	 *  @return SERVER_INFO for this server.
	 */
	const SERVER_INFO getCacheDataCopy() const;

	/** @brief
	 *  Create a new database on this server
	 *
	 *  @param Name : Name of the new DB.
	 *  @param type : typ of DB
	 */
	void createDatabase(const std::string& Name, const DATABASE_INFO::TYPE type = DATABASE_INFO::NORMAL, std::string path = "");

	/** @brief
	 *  reload this server from disk.
	 *
	 *  @return : true if the operation succeeded, false otherwise
	 */
	bool load();

	/** @brief
	 *  save this server to disk.
	 *
	 *  @return : true if the operation succeeded, false otherwise
	 */
	bool save();

	/** @brief
	 *  shut this server down
	 *
	 *  @return: true if operation succeeded, false otherwise
	 */
	bool shutdown();

	/** @brief
	 *  Checks if the server is reachable
	 */
	bool ping();

	/** @brief
	 *  Returns server license info.
	 *
	 *  @param hostname Host name
	 *  @param port Port
	 *  @param protocol Http or Https
	 *
	 *  @return : License info.
	 */
	static LICENSE_LIST getLicenseInfo(const std::string& hostname, unsigned int port, ServerProtocol protocol = Http);

	/** @brief
	 *  Returns server license info.
	 *
	 *  @return : License info.
	 */
	LICENSE_LIST getLicenseInfo();

	/** @brief
	 *  Change user's password.
	 */
	void changePassword(const std::string& oldPassword, const std::string& newPassword);

	/** @brief
	 *  Change password of specified User.
	 */
	void changeUserPassword(const std::string& userName, const std::string& newPassword);

	/** @brief
	 *  Check if password is valid.
	 */
	void checkPassword(const std::string& password) const;

	/** @brief
	 *  Locks the server from all operations except those coming from the worker that initiated the lock.
	 *  DO NOT use this explicitly except you know precisely what you are doing.
	 *
	 *  @return : True if the lock succeeded.
	 */
	bool EventLockBegin(std::string source, std::string AreaID);

	/** @brief
	 *  Ends the lock initiated by the worker.
	 *  DO NOT use this explicitly except you know precisely what you are doing.
	 */
	bool EventLockEnd();

	/** @brief
	 *  get the sid used to connect to Server
	 *
	 *  @return used SID
	 */
	const std::string getSID() const;
	unsigned int getTTL() const;

	/** @brief
	 *  get info about the Supervision Server
	 *
	 *  @return SVS Info
	 */
	const SUPERVISION_SERVER_INFO getSVSInfo() const;

	/** @brief
	 *  restart Supervision Server
	 */
	void restartSVS(int mode = 0) const;

	const DATABASE_INFO_PERMISSIONS_LIST getDatabaseList(bool system, bool userInfo, bool permissions) const;

	/** @brief
	 *  Get an iterator for the databases.
	 *  Template avoids inclusion of Cache header, which is vital for serialization
	 */
	std::unique_ptr<ServerCache::CacheIterator> getIterator();

	/** @brief
	 *  get a list of all rule functions.
	 *
	 *  @return : XML representation of the list.
	 */
	std::string RuleFunctions();

	/** @brief
	 *  get info about the user
	 *
	 *  @return User Info
	 */
	USER_INFO getUserInfo(const std::string& sid) const;
	USER_INFO getUserInfo() const;

	/** @brief
	 *  activate license
	 */
	void activateLicense(const std::string &licenseKey, const std::string &activationCode);

	void ActivateHttps(bool activate);

	unsigned int getSequenceNumberFromCache() const;

	void setSequenceNumberToCache(unsigned int SequenceNumber);

	void InvalidateCache();

	unsigned int getServerToken();
	unsigned int getDataToken();

	void setSvsMode();

	bool willExpire(unsigned int days);

	void forceNextCacheUpdate();

	std::vector<DATABASE_INFO> getAdvanced();

	std::string defineViewSubset(const std::string &database, const std::string &dimension, int indent, const std::vector<BasicFilterSettings> &basic, const TextFilterSettings &text, const SortingFilterSettings &sorting, const AliasFilterSettings &alias, const FieldFilterSettings &field, const std::vector<StructuralFilterSettings> &structural, const std::vector<DataFilterSettings> &data);
	std::string defineViewAxis(const std::string &database, int axisId, const AxisSubsets &as);
	std::string defineViewArea(const std::string &database, const std::string &cube, const std::vector<std::string> &axes, const std::vector<std::string> &properties);
	size_t getViewAxisSize(const std::string &database, const std::string &viewHandle, int axisId);
	ELEMENT_INFO_EXT getViewSubset(const std::string &database, const std::string &viewHandle, int axisId, size_t subsetPos, size_t elemPos, UINT &indent);
	AxisElement getViewSubset(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos);
	CELL_VALUE_PROPS getViewArea(const std::string &database, const std::string &viewHandle, const std::vector<std::string> &coord, bool cubeException, std::vector<std::string> *&properties);
	std::vector<AxisElement> getViewSubsetTop(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, size_t start, size_t limit);
	std::vector<AxisElement> getViewSubsetChildren(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, const std::string &element);

private:
	void logout(bool force);

	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	boost::shared_ptr<const ServerCacheB> m_Server;
	boost::shared_ptr<View> m_View;
};

} /* palo */
} /* jedox */

#endif							 // LIBPALO_NG_SERVER_H
