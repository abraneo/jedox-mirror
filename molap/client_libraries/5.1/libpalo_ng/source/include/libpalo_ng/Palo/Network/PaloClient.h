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
#pragma warning(disable : 4099)
#endif

#ifndef PALOCLIENT_H
#define PALOCLIENT_H

#include <iosfwd>
#include <memory>
#include <map>
#include <string>

#include <boost/scoped_ptr.hpp>

#include <libpalo_ng/Palo/types.h>

#include "Tokens.h"

namespace jedox {
namespace palo {

enum ServerProtocol {
	Http = 0, Https = 1
};
class PaloClientImpl;

class LIBPALO_NG_CLASS_EXPORT PaloClient {
public:

	PaloClient(const std::string& username, const std::string& password, const std::string& hostname, unsigned int port, ServerProtocol protocol, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures);
	PaloClient(const std::string& hostname, unsigned int port, ServerProtocol protocol, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId);
	PaloClient(const std::string& hostname, unsigned int port, const std::string& sessionid, ServerProtocol protocol = Http);

	void Init(const std::string& username, const std::string& password, const std::string& hostname, unsigned int port, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures, bool winSSO, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId);

	PaloClient(const PaloClient& other);
	~PaloClient();

	std::unique_ptr<std::istringstream> request(const std::string& command, const std::string& query, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, bool headRequest = false) const;
	std::unique_ptr<std::istringstream> request(const std::string& command, TOKEN& token, unsigned int& ssequencenumber, unsigned int& vsequencenumber, bool headRequest = false) const;
	std::unique_ptr<std::istringstream> request(const std::string& command, TOKEN& token, unsigned int& ssequencenumber, unsigned int& vsequencenumber, const std::string &sid) const;

	PaloClient& operator=(const PaloClient& rhs);

	std::string getSID() const;
	unsigned int getTTL() const;
	void connect(bool checkOnly);
	void ActivateHttps(bool activate);
	void SetHttpsParams(unsigned int encrypt, unsigned int port);
	void Check4Proxy();

	std::string getUsername();
	std::string getHost();
	unsigned int getPort();
	bool getAutomatic() const;
	bool getFinished() const;
	std::string getWWWAuthenticateMsg() const;

	void setUsernameTTL(const std::string &name, unsigned int ttl);
	void changePassword(const std::string& password);
	bool isAdopted() const;
	void setDescription(const std::string &description);
	void setAdopted();
	std::string getOptionalFeatures();

	static std::string getMD5(const std::string &s);

	USER_INFO getUserInfo(const std::string& sid) const;

private:
	boost::scoped_ptr<PaloClientImpl> m_PaloClientImpl;
};

} /* palo */
} /* jedox */

#endif							 // PALOCLIENT_H
