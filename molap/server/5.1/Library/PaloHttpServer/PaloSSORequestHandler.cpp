/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "PaloHttpServer/PaloSSORequestHandler.h"
#include "PaloDispatcher/PaloJobRequest.h"

#include <iostream>
#include "InputOutput/FileUtils.h"

#include "HttpServer/DirectHttpResponse.h"
#include "PaloJobs/ServerLoginJob.h"
#include "Logger/Logger.h"

#if defined(WIN32)
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif

#include <boost/shared_array.hpp>

namespace palo {

#if defined(WIN32)
AuthMap PaloSSORequestHandler::auths;
Mutex PaloSSORequestHandler::ssoLock;

#define MAX_NAME 256;

AuthenticationContext::AuthenticationContext(string host) : firstSrv(true), isDone(false)
{
	SecInvalidateHandle(&srvContext);

	SECURITY_STATUS retSrv = QuerySecurityPackageInfoA((SEC_CHAR *)"Negotiate", &secPackInfo);
	if (retSrv != SEC_E_OK) {
		Logger::debug << "QuerySecurityPackage error: " << GetLastError() << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "security package info query failed");
	}

	TimeStamp ts;

	std::wstring wHost(host.size(), L' ');
    std::copy(host.begin(), host.end(), wHost.begin());
    WCHAR principal[256];
    wcsncpy(principal, std::wstring(L"HTTP/").append(wHost).c_str(), 255);
	Logger::trace << "Principal name: " << "HTTP/" << host << endl;

	retSrv = AcquireCredentialsHandleW(principal, L"Negotiate", SECPKG_CRED_INBOUND, 0, 0, 0, 0, &credentialsSrv, &ts);
	if (retSrv != SEC_E_OK) {
		Logger::debug << "AcquireCredentialsHandle error: " << GetLastError() << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "can't acquire credentials handle");
	}

	lastOutputMessage = "Negotiate";

	refreshTimer();
}

AuthenticationContext::~AuthenticationContext()
{
	Logger::trace << "Deleting AuthenticationContext (" << this << ")" << endl;

	FreeContextBuffer(secPackInfo);
	FreeCredentialHandle(&credentialsSrv);
	DeleteSecurityContext(&srvContext);
}

void AuthenticationContext::refreshTimer()
{
	lastAction = boost::posix_time::second_clock::local_time();
}

bool AuthenticationContext::isOld() const
{
	return (lastAction + boost::posix_time::hours(0) + boost::posix_time::minutes(5) < boost::posix_time::second_clock::local_time());
}

string AuthenticationContext::accept(string &inputMessage)
{
	BIO *bmem, *b64;
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf((void *)inputMessage.c_str(), (int)inputMessage.size());
	b64 = BIO_push(b64, bmem);
	
	char bytes[256];
	vector<char> buffer;
	int size;
	while ((size = BIO_read(b64, bytes, sizeof(bytes))) > 0) {
		buffer.insert(buffer.end(), bytes, bytes + size);
	}
	BIO_free_all(b64);

	secBuffCli.BufferType = SECBUFFER_TOKEN;
	secBuffCli.cbBuffer = (unsigned long)buffer.size();
	secBuffCli.pvBuffer = (void *)&buffer[0];
	
	outputCli.cBuffers = 1;
	outputCli.pBuffers = &secBuffCli;
	outputCli.ulVersion = SECBUFFER_VERSION;

	boost::shared_array<char> outputBytes(new char[secPackInfo->cbMaxToken]);
	secBuffSrv.BufferType = SECBUFFER_TOKEN;
	secBuffSrv.cbBuffer = secPackInfo->cbMaxToken;
	secBuffSrv.pvBuffer = outputBytes.get();

	outputSrv.cBuffers = 1;
	outputSrv.pBuffers = &secBuffSrv;
	outputSrv.ulVersion = SECBUFFER_VERSION;

	TimeStamp ts;
	ULONG attrSrv;
	SECURITY_STATUS retSrv = AcceptSecurityContext(&credentialsSrv, firstSrv ? NULL : &srvContext, &outputCli, 0, SECURITY_NATIVE_DREP, &srvContext, &outputSrv, &attrSrv, &ts);

	switch (retSrv) {
	case SEC_E_INVALID_TOKEN:
		Logger::debug << "AcceptSecurityContext error: " << SEC_E_INVALID_TOKEN << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "Invalid token..");
		break;
	case SEC_E_NO_AUTHENTICATING_AUTHORITY:
		Logger::debug << "AcceptSecurityContext error: " << SEC_E_NO_AUTHENTICATING_AUTHORITY << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "No authenticating authority.");
		break;
	case SEC_E_LOGON_DENIED:
		Logger::debug << "AcceptSecurityContext error: " << SEC_E_LOGON_DENIED << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "Logon denied.");
		break;
	case SEC_E_OK:
		Logger::trace << "AcceptSecurityContext SEC_E_OK" << endl;
		break;
	case SEC_I_COMPLETE_AND_CONTINUE:
		Logger::trace << "AcceptSecurityContext SEC_I_COMPLETE_AND_CONTINUE" << endl;
		break;
	case SEC_I_COMPLETE_NEEDED:
		Logger::trace << "AcceptSecurityContext SEC_I_COMPLETE_NEEDED" << endl;
		break;
	case SEC_I_CONTINUE_NEEDED:
		Logger::trace << "AcceptSecurityContext SEC_I_CONTINUE_NEEDED" << endl;
		break;
	default:
		Logger::debug << "AcceptSecurityContext error: " << retSrv << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "AcceptSecurityContext failed.");
		break;
	}

	if (retSrv == SEC_I_COMPLETE_NEEDED || retSrv == SEC_I_COMPLETE_AND_CONTINUE ) {
		CompleteAuthToken(&srvContext, &outputSrv);
	}

	firstSrv = false;

	if (retSrv == SEC_E_OK) {
		Success();
		isDone = true;
		refreshTimer();
		return lastOutputMessage;
	} else {
		BIO *bmem, *b64;
		BUF_MEM *bptr;
		b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		bmem = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, bmem);
		BIO_write(b64, outputBytes.get(), secBuffSrv.cbBuffer);
		(void)BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bptr);
		std::string encodedMsg(bptr->data, bptr->length);
		BIO_free_all(b64);

		lastOutputMessage = "Negotiate " + encodedMsg;
		refreshTimer();
		return lastOutputMessage;
	}
}

string wchar2utf8string(WCHAR *wStr)
{
	char str[4096];
	str[0] = '\0';
	WideCharToMultiByte(CP_UTF8, 0, wStr, -1, str, sizeof(str),  NULL, NULL);
	return string(str);
}

void AuthenticationContext::Success()
{
	lastOutputMessage = "Done";

	HANDLE hToken;
	SECURITY_STATUS retSrv = QuerySecurityContextToken(&srvContext, &hToken);
	if (retSrv != SEC_E_OK) {
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "unable obtain security token");
	}

	BYTE buf[1024];
	PTOKEN_USER ptu;
	DWORD len;
	if (GetTokenInformation(hToken, TokenUser, (LPVOID)buf, 1024, &len)) {
		ptu = (PTOKEN_USER)buf;
		WCHAR wName[1024];
		WCHAR wDomain[1024];
		DWORD NameSize, DomainSize;
		NameSize = DomainSize = 1024;
		SID_NAME_USE snu;
		if (LookupAccountSidW(0, ptu->User.Sid, wName, &NameSize, wDomain, &DomainSize, &snu)) {	
			setUsername(wchar2utf8string(wName));
			setDomain(wchar2utf8string(wDomain));
		} else {
			Logger::debug << "LookupAccountSid error: " << GetLastError() << endl;
			throw ErrorException(ErrorException::ERROR_SSO_FAILED, "can't lookup account sid");
		}
	} else {
		Logger::debug << "GetTokenInformation error: " << GetLastError() << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "can't read user information from token");
	}

	DWORD dwSize = 0, dwResult = 0;
	if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))  {
		//dwResult = GetLastError();
	}

	PTOKEN_GROUPS pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

	if(!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize)) {
		Logger::debug << "GetTokenInformation error: " << GetLastError() << endl;
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "can't read group information from token");
	}

	WCHAR wName[256];
	WCHAR wDomain[256];
	SID_NAME_USE SidType;

	for(DWORD i=0; i<pGroupInfo->GroupCount; i++) {
		dwSize = MAX_NAME;
		if(!LookupAccountSidW(NULL, pGroupInfo->Groups[i].Sid, wName, &dwSize, wDomain, &dwSize, &SidType)) {
			dwResult = GetLastError();
			if (dwResult == ERROR_NONE_MAPPED) {
				wcscpy_s(wName, dwSize, L"NONE_MAPPED");
			} else {
				Logger::debug << "LookupAccountSid error: " << GetLastError() << endl;
				throw ErrorException(ErrorException::ERROR_SSO_FAILED, "can't lookup account sid for groups");
			}
		}

		addGroup(wchar2utf8string(wDomain) + "\\" + wchar2utf8string(wName));

		/*// Find out whether the SID is enabled in the token.
		if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED) {
			printf("The group SID is enabled.\n");
		} else if (pGroupInfo->Groups[i].Attributes & SE_GROUP_USE_FOR_DENY_ONLY) {
			printf("The group SID is a deny-only SID.\n");
		} else {
			printf("The group SID is not enabled.\n");
		}*/
	}

	if (pGroupInfo) {
		GlobalFree(pGroupInfo);
	}

	CloseHandle(hToken);
}
#endif

PaloSSORequestHandler::PaloSSORequestHandler(bool enabled) :
	PaloRequestHandler(enabled)
{
}

bool PaloSSORequestHandler::getAuthenticationInfo(string negId, string &winUsername, string &winDomain, vector<string> &winGroups)
{
#if defined(WIN32)
	WriteLocker locker(&ssoLock);

	AuthMap::iterator it = auths.find(negId);
	if (it != auths.end()) {
		AuthenticationContext *auth = it->second;
		winUsername = auth->getUsername();
		winDomain = auth->getDomain();
		winGroups = auth->getGroups();

		Logger::debug << "SSO successful: " << winUsername << " " << winDomain << " (" << winGroups.size() << " win groups)" << endl;

		removeAuthenticationInfo(negId);

		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

void PaloSSORequestHandler::removeAuthenticationInfo(string negId)
{
#if defined(WIN32)
	//WriteLocker locker(&ssoLock); has to be under lock already

	Logger::trace << "Number of AuthenticationContext objects: " << auths.size() << endl;

	AuthMap::iterator it = auths.find(negId);
	if (it != auths.end()) {
		Logger::trace << "Deleting old AuthenticationContext for " << negId << "." << endl;
		delete it->second;
		auths.erase(it);
	}
#endif
}

void PaloSSORequestHandler::clearOldContexts()
{
#if defined(WIN32)
	//WriteLocker locker(&ssoLock); has to be under lock already

	for (AuthMap::iterator it = auths.begin(); it != auths.end();) {
		if (it->second->isOld()) {
			Logger::trace << "Deleting old AuthenticationContext for " << it->first << "." << endl;
			delete it->second;
			auths.erase(it++);
		} else {
			++it;
		}
	}
#endif
}

HttpJobRequest * PaloSSORequestHandler::handleHttpRequest(HttpRequest * request, const HttpServerTask *task)
{
	Context *context = Context::getContext();
	PServer server = context->getServer();
	PaloHttpRequest *phr = dynamic_cast<PaloHttpRequest *>(request);
	PaloJobRequest *pjr = NULL;
	if (phr) {
		pjr = phr->getPaloJobRequest();
	}
	if (!pjr) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "PaloJobRequest can't be initialized.");
	}
	if (!server->winAuthEnabled() || pjr->user) {
		return PaloRequestHandler::handleHttpRequest(request, task); // simple old /server/login
	}

#if defined(WIN32)
	WriteLocker locker(&ssoLock);

	clearOldContexts();

	string negId = request->getHeader("negotiation-id");
	AuthMap::iterator it = auths.find(negId);

	socket_t socket = task->getReadSocket();
	if (negId == "") { 
		// simple browser request, assign socket identifier
		negId = "socket-" + boost::lexical_cast<string>(socket);
		it = auths.find(negId); // search once more
	} else if (negId == "init") { 
		// initial sso login request from libpalo_ng
		int i = rand() % 10000;
		do {
			i++;
			negId = "lpng-" + boost::lexical_cast<string>(i);
		} while (auths.find(negId) != auths.end());
	} else if (it == auths.end()) { // non-empty but non-initialized identifier
		throw ErrorException(ErrorException::ERROR_SSO_FAILED, "Unknown negotiation identifier used.");
	} else {
		// following request, properly found
	}

	AuthenticationContext *auth;
	if (it == auths.end()) {
		Logger::trace << "Creating new AuthenticationContext for " << negId << " (" << socket << ")." << endl;
		auth = new AuthenticationContext(request->getHeader("Host"));
		auths[negId] = auth;
	} else {
		auth = it->second;
	}
		
	string message = request->getHeader("Authorization");
	if (!message.empty()) { // could be initial login request from browser without Authorization header, just Negotiate will be return
		Logger::trace << "Received authorization message (" << negId << "): " << message << endl;
		if (message.substr(0, 10) == "Negotiate ") {
			message = message.substr(10, string::npos);
			try {
				auth->accept(message);
			} catch (...) {
				removeAuthenticationInfo(negId);
				throw;
			}
		} else {
			removeAuthenticationInfo(negId);
			throw ErrorException(ErrorException::ERROR_SSO_FAILED, "Wrong format of Authorization message.");
		}
	}

	if (auth->done()) {
		pjr->negotiationId = negId;
		return PaloRequestHandler::handleHttpRequest(request, task);
	} else {
		HttpResponse *response = new HttpResponse(HttpResponse::UNAUTHORIZED);
		response->setHeaderField(pair<string, string>("WWW-Authenticate", auth->lastMessage()));
		response->setHeaderField(pair<string, string>("negotiation-id", negId));
		Logger::trace << "Sending www-authenticate (" << negId << "): " << auth->lastMessage() << endl;
		response->getBody().appendText("SSO Authentication in progress.");
		return new DirectHttpResponse(request->getRequestPath(), response);
	}
#else
	throw ErrorException(ErrorException::ERROR_SSO_FAILED, "SSO authentication only possible with Windows server.");
#endif
}
}
