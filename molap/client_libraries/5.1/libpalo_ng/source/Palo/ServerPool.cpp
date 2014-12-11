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

#include <iomanip>
#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Palo/ServerPool.h>
#include "ServerImpl.h"
#include <openssl/sha.h>

#include "Exception/PaloNGGeneralException.h"

namespace jedox {
namespace palo {

struct ServerPool::ServerPoolImpl {
	ServerPoolImpl() {}

	boost::mutex m_Lock;
	std::map<std::string, std::pair<boost::shared_ptr<ServerImpl>, int>, jedox::util::UTF8Comparer> m_cache;
	std::map<std::string, ServerSPtr> m_server;
	std::map<std::string, std::string, jedox::util::UTF8Comparer> m_alias2id;
	std::multimap<std::string, std::string> m_id2alias;
	std::map<std::string, std::string, jedox::util::UTF8Comparer> m_sid2id;
};

ServerPool::~ServerPool()
{
}

ServerPool::ServerPool() :
	m_ServerPoolImpl(new ServerPoolImpl)
{
}

std::string ServerPool::connectServer(const std::string& host, const unsigned int port, const std::string& user, const std::string& password, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures, ServerProtocol protocol)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	boost::shared_ptr<PaloClient> pc(new PaloClient(user, password, host, port, Http, machineString, requiredFeatures, optionalFeatures));
	if (!clientDescription.empty()) {
		pc->setDescription(clientDescription);
	}
	std::string ret = connectIntern(pc, true, protocol);
	optionalFeatures = pc->getOptionalFeatures();
	return ret;
}

std::string ServerPool::connectServerWinSSO(const std::string& host, const unsigned int port, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId, const std::string& machineString, const std::string& requiredFeatures, std::string& optionalFeatures, ServerProtocol protocol)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	boost::shared_ptr<PaloClient> pc(new PaloClient(host, port, Http, machineString, requiredFeatures, optionalFeatures, automatic, finished, negoString, negotiationId));
	if (!clientDescription.empty()) {
		pc->setDescription(clientDescription);
	}
	std::string ret = connectIntern(pc, true, protocol);
	optionalFeatures = pc->getOptionalFeatures();
	return ret;
}

std::string ServerPool::adoptServer(const std::string& host, const unsigned int port, const std::string& sid, ServerProtocol protocol)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	std::stringstream sidKey;
	sidKey << host << port << sid;
	std::map<std::string, std::string, jedox::util::UTF8Comparer>::iterator it = m_ServerPoolImpl->m_sid2id.find(sidKey.str());
	if (it != m_ServerPoolImpl->m_sid2id.end()) {
		m_ServerPoolImpl->m_server.find(it->second)->second->m_PaloClient->setAdopted();
		return it->second;
	} else {
		boost::shared_ptr<PaloClient> pc(new PaloClient(host, port, sid, Http));
		return connectIntern(pc, false, protocol);
	}
}

void ServerPool::defineAlias(const std::string &key, const std::string &alias)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	if (m_ServerPoolImpl->m_server.find(key) == m_ServerPoolImpl->m_server.end()) {
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_SERVER_NOT_FOUND);
	}
	if (m_ServerPoolImpl->m_alias2id.find(alias) != m_ServerPoolImpl->m_alias2id.end()) {
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_SERVER_ALIAS_EXISTS);
	}
	m_ServerPoolImpl->m_alias2id.insert(std::make_pair(alias, key));
	m_ServerPoolImpl->m_id2alias.insert(std::make_pair(key, alias));
}

ServerSPtr ServerPool::getServer(const std::string& key)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	std::map<std::string, ServerSPtr>::iterator its = m_ServerPoolImpl->m_server.find(key);
	if (its == m_ServerPoolImpl->m_server.end()) {
		std::map<std::string, std::string, jedox::util::UTF8Comparer>::iterator ita = m_ServerPoolImpl->m_alias2id.find(key);
		if (ita == m_ServerPoolImpl->m_alias2id.end()) {
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_SERVER_NOT_FOUND);
		}
		its = m_ServerPoolImpl->m_server.find(ita->second);
		if (its == m_ServerPoolImpl->m_server.end()) {
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_SERVER_NOT_FOUND);
		}
	}
	return its->second;
}

void ServerPool::disconnectServer(const std::string& key, bool force)
{
	boost::unique_lock<boost::mutex> guard(m_ServerPoolImpl->m_Lock);
	std::map<std::string, ServerSPtr>::iterator its = m_ServerPoolImpl->m_server.find(key);
	if (its == m_ServerPoolImpl->m_server.end()) {
		std::map<std::string, std::string, jedox::util::UTF8Comparer>::iterator ita = m_ServerPoolImpl->m_alias2id.find(key);
		if (ita != m_ServerPoolImpl->m_alias2id.end()) {
			its = m_ServerPoolImpl->m_server.find(ita->second);
			std::stringstream sidKey;
			sidKey << its->second->m_PaloClient->getHost() << its->second->m_PaloClient->getPort() << its->second->m_PaloClient->getSID();
			m_ServerPoolImpl->m_sid2id.erase(sidKey.str());
			its->second->logout(force);
			std::stringstream userKey;
			userKey << its->second->m_PaloClient->getHost() << its->second->m_PaloClient->getPort() << its->second->m_PaloClient->getUsername();
			std::map<std::string, std::pair<boost::shared_ptr<ServerImpl>, int>, jedox::util::UTF8Comparer>::iterator itc = m_ServerPoolImpl->m_cache.find(userKey.str());
			if (!--itc->second.second) {
				m_ServerPoolImpl->m_cache.erase(itc);
			}
			for (std::multimap<std::string, std::string>::iterator it = m_ServerPoolImpl->m_id2alias.find(its->first); it != m_ServerPoolImpl->m_id2alias.end() && it->first == its->first;) {
				m_ServerPoolImpl->m_alias2id.erase(it->second);
				m_ServerPoolImpl->m_id2alias.erase(it++);
			}
			m_ServerPoolImpl->m_server.erase(its);
		}
	} else {
		std::stringstream sidKey;
		sidKey << its->second->m_PaloClient->getHost() << its->second->m_PaloClient->getPort() << its->second->m_PaloClient->getSID();
		m_ServerPoolImpl->m_sid2id.erase(sidKey.str());
		its->second->logout(force);
		std::stringstream userKey;
		userKey << its->second->m_PaloClient->getHost() << its->second->m_PaloClient->getPort() << its->second->m_PaloClient->getUsername();
		std::map<std::string, std::pair<boost::shared_ptr<ServerImpl>, int>, jedox::util::UTF8Comparer>::iterator itc = m_ServerPoolImpl->m_cache.find(userKey.str());
		if (!--itc->second.second) {
			m_ServerPoolImpl->m_cache.erase(itc);
		}
		for (std::multimap<std::string, std::string>::iterator it = m_ServerPoolImpl->m_id2alias.find(key); it != m_ServerPoolImpl->m_id2alias.end() && it->first == key;) {
			m_ServerPoolImpl->m_alias2id.erase(it->second);
			m_ServerPoolImpl->m_id2alias.erase(it++);
		}
		m_ServerPoolImpl->m_server.erase(its);
	}
}

ServerPool& ServerPool::getInstance()
{
	static ServerPool instance;
	return instance;
}

std::string ServerPool::connectIntern(boost::shared_ptr<PaloClient> pc, bool connect, ServerProtocol protocol)
{
	boost::shared_ptr<ServerImpl> si(new ServerImpl);

	pc->Check4Proxy();
	si->updateServerCache(pc);
	pc->ActivateHttps(protocol == Https);
	if (connect) {
		pc->connect(false);
		if (!pc->getAutomatic() && !pc->getFinished()) {
			return pc->getWWWAuthenticateMsg();
		}
	}
	if (pc->getUsername().empty()) {
		USER_INFO u = pc->getUserInfo(pc->getSID());
		pc->setUsernameTTL(u.nuser, u.ttl);
	}

	std::stringstream userKey;
	userKey << pc->getHost() << pc->getPort() << pc->getUsername();
	std::map<std::string, std::pair<boost::shared_ptr<ServerImpl>, int>, jedox::util::UTF8Comparer>::iterator it = m_ServerPoolImpl->m_cache.find(userKey.str());
	if (it != m_ServerPoolImpl->m_cache.end()) {
		si = it->second.first;
		++it->second.second;
	} else {
		m_ServerPoolImpl->m_cache.insert(std::make_pair(userKey.str(), std::make_pair(si, 1)));
	}

	ServerSPtr s(new Server(si, pc));
	std::string server_id;
	do {
		server_id = getRandId();
	} while (m_ServerPoolImpl->m_server.find(server_id) != m_ServerPoolImpl->m_server.end());
	m_ServerPoolImpl->m_server.insert(std::make_pair(server_id, s));
	std::stringstream sidKey;
	sidKey << s->m_PaloClient->getHost() << s->m_PaloClient->getPort() << s->m_PaloClient->getSID();
	m_ServerPoolImpl->m_sid2id[sidKey.str()] = server_id;
	return server_id;
}

std::string ServerPool::getRandId()
{
	unsigned char sha[20];
	std::string server_id;
	std::stringstream str;
	size_t id = rand();
	SHA1((const unsigned char *)&id, sizeof(id), sha);
	str << std::hex << std::setfill('0');
	for (size_t i = 0; i < sizeof(sha); ++i) {
		str << std::setw(2) << (int)sha[i];
	}
	return str.str();
}

} /* palo */
} /* jedox */
