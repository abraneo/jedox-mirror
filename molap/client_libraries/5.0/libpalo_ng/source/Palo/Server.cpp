/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
#pragma warning(disable : 4099)
#endif

#define _SCL_SECURE_NO_WARNINGS

#include <fstream>
#include <vector>

#include <boost/foreach.hpp>

#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "ServerImpl.h"
#include "libpalo_ng/Palo/Network/PaloClient.h"
#include "../Palo/Exception/PaloNGGeneralException.h"
#include "../Util/CsvTokenFromStream.h"

namespace jedox {
namespace palo {

using namespace std;

Server::Server(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient) : m_ServerImpl(s), m_PaloClient(paloClient), m_Server(s->getServerCache())
{
}

Server::~Server()
{
}

Database Server::operator[](const std::string& name)
{
	return Database(m_ServerImpl, m_PaloClient, m_ServerImpl->getDatabase(m_PaloClient, name).database);
}

Database Server::operator[](unsigned int id)
{
	return Database(m_ServerImpl, m_PaloClient, m_ServerImpl->getDatabase(m_PaloClient, id).database);
}

bool Server::Exists(const std::string& name)
{
	return m_ServerImpl->existDatabase(m_PaloClient, name);
}

bool Server::Exists(unsigned int id)
{
	return m_ServerImpl->existDatabase(m_PaloClient, id);
}

void Server::createDatabase(const std::string& Name, const DATABASE_INFO::TYPE type)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	DATABASE_TOKEN token(sequencenumber);
	query << "new_name=";
	jedox::util::URLencoder(query, Name);
	query << "&type=" << type;

	m_PaloClient->request("/database/create", query.str(), token, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);
}

bool Server::load()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);
	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/load", servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);

	char result;
	(*stream) >> result;
	return result == '1';
}

bool Server::save()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);
	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/save", servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);

	char result;
	(*stream) >> result;
	return result == '1';
}

bool Server::shutdown()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);
	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/shutdown", servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);

	char result;
	(*stream) >> result;
	return result == '1';
}

bool Server::EventLockBegin(std::string source, std::string AreaID)
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	query << "source=" << source << "&event=" << AreaID;

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/event/begin", query.str(), servertoken, sequencenumber, dummy);

	char result;
	(*stream) >> result;
	return result == '1';
}

bool Server::EventLockEnd()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/event/end", servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);

	char result;
	(*stream) >> result;
	return result == '1';
}

bool Server::ping()
{
	m_ServerImpl->updateServerCache(m_PaloClient);
	m_Server = m_ServerImpl->getServerCache();
	return true;
}

unsigned int Server::getServerToken()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/info", servertoken, sequencenumber, dummy);
	
	return servertoken.getSequenceNumber();
}

void Server::setSvsMode()
{
	m_ServerImpl->setSvsMode(true);
}

void Server::ActivateHttps(bool activate)
{
	m_PaloClient->ActivateHttps(activate);
}

void Server::logout(bool force)
{
	try {
		if (!m_PaloClient->isAdopted() || force) {
			unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
			SERVER_TOKEN servertoken(sequencenumber);

			m_PaloClient->request("/server/logout", servertoken, sequencenumber, dummy);
		}
	} catch (std::exception&) {
#ifndef _DEBUG
	} catch (...) {
#endif
	}
}

const std::string Server::getSID() const
{
	return m_PaloClient->getSID();
}

unsigned int Server::getTTL() const
{
	return m_PaloClient->getTTL();
}

const SUPERVISION_SERVER_INFO Server::getSVSInfo() const
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/svs/info", servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);

	SUPERVISION_SERVER_INFO svsinfo;
	(*stream) >> csv >> svsinfo;

	return svsinfo;
}

void Server::restartSVS(int mode) const
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::stringstream query;
	query << "mode=" << mode;
	m_PaloClient->request("/svs/restart", query.str(), servertoken, sequencenumber, dummy);
	m_ServerImpl->invalidateDatabases(sequencenumber);
}

const SERVER_INFO& Server::getCacheData() const
{
	return *m_Server;
}

const SERVER_INFO Server::getCacheDataCopy() const
{
	return getCacheData();
}

LICENSE_LIST Server::getLicenseInfo(const std::string& hostname, unsigned int port, ServerProtocol protocol)
{
	std::string optf;
	PaloClient cl("", "", hostname, port, protocol, "", "", optf);
	unsigned int d1, d2;
	TOKEN t(SERVERTOKENNAME, 0, Token::SERVER);

	std::unique_ptr<std::istringstream> stream = cl.request("/server/licenses", t, d1, d2);
	LICENSE_LIST lic;
	(*stream) >> csv >> lic;
	return lic;
}

LICENSE_LIST Server::getLicenseInfo()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/licenses", servertoken, sequencenumber, dummy);

	LICENSE_LIST lic;
	(*stream) >> csv >> lic;
	return lic;
}

void Server::changePassword(const std::string& oldPassword, const std::string& newPassword)
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	checkPassword(oldPassword);

	std::stringstream query;
	query << "password=";
	jedox::util::URLencoder(query, newPassword);
	m_PaloClient->request("/server/change_password", query.str(), servertoken, sequencenumber, dummy);
	m_PaloClient->changePassword(newPassword);
}

void Server::changeUserPassword(const std::string& userName, const std::string& newPassword)
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::stringstream query;
	query << "password=";
	jedox::util::URLencoder(query, newPassword);
	query << "&user=";
	jedox::util::URLencoder(query, userName);
	m_PaloClient->request("/server/change_password", query.str(), servertoken, sequencenumber, dummy);
	if (!util::UTF8Comparer::compare(userName, m_PaloClient->getUsername())) {
		m_PaloClient->changePassword(newPassword);
	}
}

void Server::checkPassword(const std::string& password) const
{
	PaloClient p(*m_PaloClient);
	p.changePassword(password);
	p.connect(true);
}

USER_INFO Server::getUserInfo(const string& sid) const
{
	return m_PaloClient->getUserInfo(sid);
}

void Server::activateLicense(const std::string &licenseKey, const std::string &activationCode)
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::stringstream query;
	query << "lickey=";
	jedox::util::URLencoder(query, licenseKey);
	query << "&actcode=";
	jedox::util::URLencoder(query, activationCode);
	m_PaloClient->request("/server/activate_license", query.str(), servertoken, sequencenumber, dummy);
}

bool Server::willExpire(unsigned int days)
{
	time_t t;
	time(&t);
	LICENSE_LIST lic = getLicenseInfo();
	bool ret = false;
	for (vector<LICENSE_INFO>::const_iterator it = lic.licenses.begin(); it != lic.licenses.end(); ++it) {
		if (t >= (time_t)it->expiration || ((time_t)it->expiration - t)/(3600 * 24) <= days) {
			ret = true;
			break;
		}
	}
	return ret;
}

void Server::forceNextCacheUpdate()
{
	m_ServerImpl->forceNextUpdate();
}

std::vector<DATABASE_INFO> Server::getAdvanced()
{
	std::stringstream query;
	unsigned int dummy, sequencenumber = 0;
	SERVER_TOKEN sequenceToken(sequencenumber);

	query << "show_normal=1&show_system=1&show_user_info=1&mode=1";

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/server/databases", query.str(), sequenceToken, sequencenumber, dummy);
	jedox::util::CsvTokenFromStream tfs(*stream);
	std::vector<DATABASE_INFO> ret;;
	while ((*stream).eof() == false) {
		DATABASE_INFO db;
		(*stream) >> csv >> db;
		unsigned int seq_nr;
		tfs.get(seq_nr, 0);
		ret.push_back(db);
		tfs.clear_done();
	}
	return ret;
}

std::string Server::RuleFunctions()
{
	unsigned int dummy, sequencenumber = m_Server->getSequenceNumber();
	SERVER_TOKEN servertoken(sequencenumber);

	std::unique_ptr<std::istringstream> stream = m_PaloClient->request("/rule/functions", servertoken, sequencenumber, dummy);

	return (*stream).str();
}

unsigned int Server::getSequenceNumberFromCache() const
{
	return m_ServerImpl->getDatabasesSN(m_PaloClient);
}

void Server::InvalidateCache()
{
	m_ServerImpl->invalidateDatabases();
}

std::unique_ptr<ServerCache::CacheIterator> Server::getIterator()
{
	return m_ServerImpl->getDatabaseIterator(m_PaloClient);
}

} /* palo */
} /* jedox */
