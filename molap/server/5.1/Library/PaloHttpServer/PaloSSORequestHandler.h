////////////////////////////////////////////////////////////////////////////////
/// @brief palo http SSO request handler
///
/// @file
///
/// Copyright (C) 2006-2013 Jedox AG
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
/// You may obtain a copy of the License at
///
/// <a href="http://www.jedox.com/license_palo_suite.txt">
///   http://www.jedox.com/license_palo_suite.txt
/// </a>
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// Portions of the code developed by triagens GmbH, Koeln on behalf of Jedox
/// AG. Intellectual property rights for these portions has triagens GmbH,
/// Koeln, or othervise Jedox AG, Freiburg. Exclusive worldwide exploitation
/// right (commercial copyright) has Jedox AG, Freiburg.
///
/// @author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
////////////////////////////////////////////////////////////////////////////////

#ifndef PALO_SSO_REQUEST_HANDLER_H
#define PALO_SSO_REQUEST_HANDLER_H 1

#include "palo.h"

#if defined(WIN32)
#include <Sspi.h>
#endif

#include <map>
#include <boost/thread/thread_time.hpp>

#include "HttpServer/HttpJobRequest.h"
#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpRequestHandler.h"
#include "PaloHttpServer/PaloRequestHandler.h"

namespace palo {

#if defined(WIN32)
class PaloSSORequestHandler;

class AuthenticationContext {
public:
	AuthenticationContext(string host);
	~AuthenticationContext();

	string accept(string &message);
	string lastMessage() const {return lastOutputMessage;}
	bool done() const {return isDone;}

	string getUsername() const {
		return winUsername;
	}

	string getDomain() const {
		return winDomain;
	}

	vector<string> getGroups() const {
		return winGroups;
	}

	bool isOld() const;

private:
	void setUsername(const string username) {
		winUsername = username;
	}

	void setDomain(const string domain) {
		winDomain = domain;
	}

	void addGroup(const string group) {
		winGroups.push_back(group);
	}

	void Success();

	void refreshTimer();

private:
	boost::posix_time::ptime lastAction;

	SecPkgInfoA *secPackInfo;
	bool firstSrv;

	SecBuffer secBuffSrv;
	SecBufferDesc outputSrv;
	CtxtHandle srvContext;
	CredHandle credentialsSrv;
	
	SecBuffer secBuffCli;
	SecBufferDesc outputCli;

	string lastOutputMessage;
	bool isDone;

	string winUsername;
	string winDomain;
	vector<string> winGroups;
};

typedef map<string, AuthenticationContext *> AuthMap;

#endif

class SERVER_CLASS PaloSSORequestHandler : public PaloRequestHandler {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new handler
	///
	/// Constructs a new file request handler given a filename and a content type.
	////////////////////////////////////////////////////////////////////////////////

	PaloSSORequestHandler(bool enabled);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~PaloSSORequestHandler() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	virtual HttpJobRequest * handleHttpRequest(HttpRequest*, const HttpServerTask*);

	static bool getAuthenticationInfo(string negId, string &winUsername, string &winDomain, vector<string> &winGroups);
	static void removeAuthenticationInfo(string negId);
	void clearOldContexts();

private:

#if defined(WIN32)
	static AuthMap auths;
	static Mutex ssoLock;
#endif
};

}

#endif
