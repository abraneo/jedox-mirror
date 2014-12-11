////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////
/*!
* \brief
* TODO
*
* \author Florian Schaper <florian.schaper@jedox.com>
* \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
* \author Frieder Hofmann <frieder.hofmann@jedox.com>
*/

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#if defined(WIN32) || defined(WIN64)
#	include <WinSock2.h>
#	include <WS2tcpip.h>
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#endif

#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>

#include <libpalo_ng/libpalo_ng_version.h>
#include <libpalo_ng/Palo/Database.h>

#include "JedoxXLHelper.h"

namespace jedox {
	namespace palo {

		typedef std::map<std::string, std::string> t_LookupMap;
		typedef std::map<std::string, std::vector<std::string> > t_LookupMap2;

		inline bool CheckForSSO(const std::string& user);
		inline std::string CalculateMachinid();
		inline std::string CreateKey(const std::string& host, const unsigned int port, const std::string& user);
		std::string getNewServerKey(const std::string& client_description, const std::string& host, const unsigned int port, const std::string& user, const std::string& password, const std::string& useFeature, std::string& optionalFeature, jedox::palo::ServerProtocol protocol, bool isSSO, ServerSPtr& server);
		
		struct JedoxXLHelper::JedoxXLHelperImpl : boost::noncopyable {
			boost::mutex m_Lock;
			t_LookupMap m_LookupMap;
			t_LookupMap m_ReverseLookupMap;
			std::map<std::string, ServerSPtr> m_Servers;
			t_LookupMap2 m_Servers_Passwords;
			std::string desc;
		};

		class MachineID {
		public:
			static std::string& getMachineID() {
				if (machineid.empty()) {
					machineid = CalculateMachinid();
				}

				return machineid;
			}

		private:
			static std::string machineid;
		};
		
		static const std::string atsign = "@";

		std::string MachineID::machineid = "";

		ServerPool& sp = ServerPool::getInstance();

		class TestConnectionHelper {
		public:
			TestConnectionHelper(JedoxXLHelper* helper, const std::string& host, const unsigned int port, const std::string& user, const std::string& password);
			~TestConnectionHelper();

		private:
			JedoxXLHelper* m_helper;
			std::string m_key;
			std::string m_empty;
			ServerSPtr m_server;
		};

		JedoxXLHelper::JedoxXLHelper() : m_JedoxXLHelperImpl(new JedoxXLHelperImpl) {
		}

		JedoxXLHelper::~JedoxXLHelper() {
		}

		ServerSPtr JedoxXLHelper::getServer(const std::string& host, const unsigned int port, const std::string& user, const std::string& password, std::string& key, jedox::palo::ServerProtocol protocol, bool diretclyfromxll) {
			bool isSSO = CheckForSSO(user);
			std::string tmpkey = CreateKey(host, port, user);
			key.clear();

			ServerSPtr server;
			{
				boost::unique_lock<boost::mutex> guard(m_JedoxXLHelperImpl->m_Lock);
				t_LookupMap::iterator it(m_JedoxXLHelperImpl->m_LookupMap.find(tmpkey));
				if (it == m_JedoxXLHelperImpl->m_LookupMap.end()) {
					std::string empty;
					std::string newkey = getNewServerKey(getVersionInfo(), host, port, user, password, empty, empty, protocol, isSSO, server);

					m_JedoxXLHelperImpl->m_Servers[newkey] = server;
					m_JedoxXLHelperImpl->m_Servers_Passwords[newkey] = std::vector<std::string>(1, password);

					m_JedoxXLHelperImpl->m_LookupMap[tmpkey] = newkey;
					m_JedoxXLHelperImpl->m_ReverseLookupMap[newkey] = tmpkey;
					key = newkey;
				} else {
					key = it->second;
					server = sp.getServer(key);
					if (!isSSO) {
						t_LookupMap2::iterator it2(m_JedoxXLHelperImpl->m_Servers_Passwords.find(key));
						if (std::find(it2->second.begin(), it2->second.end(), password) == it2->second.end()) {
							server->checkPassword(password);
							it2->second.push_back(password);
						}
					}
				}
			}

			server->ActivateHttps(protocol == jedox::palo::Https);
			return server;
		}

		void JedoxXLHelper::TestConnection(const std::string& host, const unsigned int port, const std::string& user, const std::string& password) {
			bool isSSO = CheckForSSO(user);
			std::string key = CreateKey(host, port, user);
			{
				boost::unique_lock<boost::mutex> guard(m_JedoxXLHelperImpl->m_Lock);
				t_LookupMap::iterator it(m_JedoxXLHelperImpl->m_LookupMap.find(key));
				if (it == m_JedoxXLHelperImpl->m_LookupMap.end()) {
					TestConnectionHelper Testconnectionhelper(this, host, port, user, password);
				} else {
					if (!isSSO) {
						m_JedoxXLHelperImpl->m_Servers.at(it->second)->checkPassword(password);
					}
				}
			}
		}

		void JedoxXLHelper::removeServer(const std::string& key, bool disconnect) {
			boost::unique_lock<boost::mutex> guard(m_JedoxXLHelperImpl->m_Lock);

			t_LookupMap::iterator rit(m_JedoxXLHelperImpl->m_ReverseLookupMap.find(key));
			if (rit != m_JedoxXLHelperImpl->m_ReverseLookupMap.end()) {
				if (disconnect) {
					sp.disconnectServer(key, true);
					m_JedoxXLHelperImpl->m_Servers.erase(key);
					m_JedoxXLHelperImpl->m_Servers_Passwords.erase(key);
				}
				m_JedoxXLHelperImpl->m_LookupMap.erase(rit->second);
				m_JedoxXLHelperImpl->m_ReverseLookupMap.erase(rit);
			}
		}

		JedoxXLHelper& JedoxXLHelper::getInstance() {
			static JedoxXLHelper instance;
			return instance;
		}

		void JedoxXLHelper::Cleanup() {
			t_LookupMap::iterator it, begin = m_JedoxXLHelperImpl->m_LookupMap.begin();
			t_LookupMap::iterator end = m_JedoxXLHelperImpl->m_LookupMap.end();
			for (it = begin; it != end; it++) {
				try {
					sp.disconnectServer(it->second, true);
				} catch (...) {
				}
			}
			m_JedoxXLHelperImpl->m_LookupMap.clear();
			m_JedoxXLHelperImpl->m_ReverseLookupMap.clear();
			m_JedoxXLHelperImpl->m_Servers.clear();
			m_JedoxXLHelperImpl->m_Servers_Passwords.clear();
		}


		void JedoxXLHelper::SetVersionInfo(const std::string& version) {
			m_JedoxXLHelperImpl->desc = "{\"client\":\"Open Source client\",\"client_ver\":\"";
			m_JedoxXLHelperImpl->desc.append(version);
			m_JedoxXLHelperImpl->desc.append("\",\"lib\":\"Open Source libpalo_ng\",\"lib_ver\":\"");
			m_JedoxXLHelperImpl->desc.append("OS 5.1");
			m_JedoxXLHelperImpl->desc.append("\",\"desc\":\"user login\"}");
		}

		std::string JedoxXLHelper::getVersionInfo() const {
			return m_JedoxXLHelperImpl->desc;
		}

		TestConnectionHelper::TestConnectionHelper(JedoxXLHelper* helper, const std::string& host, const unsigned int port, const std::string& user, const std::string& password) : m_helper(helper), m_key(""), m_empty("") {
			m_key = getNewServerKey(m_helper->getVersionInfo(), host, port, user, password, m_empty, m_empty, jedox::palo::Http, CheckForSSO(user), m_server);
		}

		TestConnectionHelper::~TestConnectionHelper() {
			if (!m_key.empty()) {
				try {
					sp.disconnectServer(m_key, true);
				} catch (...) {
				}
			}
		}

		std::string CalculateMachinid() {
			struct addrinfo *addr = NULL;
			struct addrinfo hints;
			char* LocalIP = (char *)"0.0.0.0"; // Dummy

#			if defined(WIN32) || defined(WIN64)
				WSADATA wsaData;
				int wsaret = WSAStartup( 0x101, &wsaData );
#			endif

			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			DWORD dwRetval = getaddrinfo("", NULL, &hints, &addr);
			if (dwRetval == 0) {
				LocalIP = inet_ntoa(((struct sockaddr_in *)addr->ai_addr)->sin_addr);
				freeaddrinfo(addr);
			}

			return LocalIP;
		}

		std::string CreateKey(const std::string& host, const unsigned int port, const std::string& user) {
			std::stringstream helperstr;
			helperstr << "U:" << host << ":" << port << ":" << user;
			return helperstr.str().c_str();
		}

		bool CheckForSSO(const std::string& user) {
			return (user.find(atsign) != std::string::npos);
		}

		std::string getNewServerKey(const std::string& client_description, const std::string& host, const unsigned int port, const std::string& user, const std::string& password, const std::string& useFeature, std::string& optionalFeature, jedox::palo::ServerProtocol protocol, bool isSSO, ServerSPtr& server) {
			std::string key;
			sp.setClientDescription(client_description);
			std::string& machineid = MachineID::getMachineID();

			if (!isSSO) {
				key = sp.connectServer(host, port, user, password, machineid, useFeature, optionalFeature, protocol);
			} else {
				key = sp.connectServerWinSSO(host, port, true, NULL, NULL, NULL, machineid, useFeature, optionalFeature, protocol);
			}
			server = sp.getServer(key);

			return key;
		}


	} /* Palo */
} /* Jedox */
