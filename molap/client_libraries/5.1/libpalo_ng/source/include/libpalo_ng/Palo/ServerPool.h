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

#ifndef LIBPALO_NG_SERVERPOOL_H
#define LIBPALO_NG_SERVERPOOL_H

#include <boost/smart_ptr/shared_ptr.hpp>

#include "Server.h"

namespace jedox {
namespace palo {

typedef boost::shared_ptr<Server> ServerSPtr;

/** @brief
 *  Class that creates server objects. Use this class to create an initial server.
 *  All Strings are UTF8.
 */
class LIBPALO_NG_CLASS_EXPORT ServerPool {
public:

	//serialization is not supported anymore!
	enum SERIALIZATION_MODE {
		SERIALIZE_TEXT = 0, SERIALIZE_BINARY, SERIALIZE_XML
	};

	~ServerPool();

	/** @brief
	 *  get a reference to a server
	 *
	 *  @param host IP address or DNS name of host
	 *  @param port port the server listens to
	 *  @param user name of the connecting user
	 *  @param password password of connecting user
	 *  @param protocol whished protocol to use (Http or Https) (default to Http))
	 *
	 *  @return  key for calling getServer
	 */
	std::string connectServer(const std::string& host, const unsigned int port, const std::string& user, const std::string& password, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures, ServerProtocol protocol = Http);
	std::string connectServerWinSSO(const std::string& host, const unsigned int port, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures, ServerProtocol protocol = Http);
	std::string adoptServer(const std::string& host, const unsigned int port, const std::string& sid, ServerProtocol protocol = Http);
	void defineAlias(const std::string &key, const std::string &alias);

	/** @brief
	 *  get a reference to a server
	 *
	 *	@param key
	 *  @return pointer to Object of class palo::jedox::server
	 */
	ServerSPtr getServer(const std::string& key);

	/** @brief
	 *  disconnects server
	 *
	 *	@param key
	 */
	void disconnectServer(const std::string& key, bool force = false);

	/** @brief
	 *  get an instance of server pool
	 *
	 *  @return Instance of Server Pool.
	 */
	static ServerPool& getInstance();

	/** @brief
	 *  sets client application description.
	 *
	 */
	void setClientDescription(const std::string clientDescription) {this->clientDescription = clientDescription;}

	static std::string getRandId();

private:
	ServerPool();
	ServerPool(const ServerPool &);
	std::string connectIntern(boost::shared_ptr<PaloClient> pc, bool connect, ServerProtocol protocol);

	struct ServerPoolImpl;
	boost::scoped_ptr<ServerPoolImpl> m_ServerPoolImpl;

	std::string clientDescription;
};

} /* palo */
} /* jedox */


#endif							 // LIBPALO_NG_SERVERPOOL_H
