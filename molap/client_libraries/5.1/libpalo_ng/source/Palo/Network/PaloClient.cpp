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
 * \author Martin Dolezal <martin.dolezal@qbicon.cz>
 * 
 *
 */

#define _SCL_SECURE_NO_WARNINGS

#include <cassert>
#include <ostream>
#include <string>
#include <sstream>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Network/PaloClient.h>
#include <libpalo_ng/Palo/Exception/PaloExceptionFactory.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "../Exception/MaximumServerRetrysReachedException.h"
#include "../../Network/HttpClient/Http.h"
#include "../../Network/HttpClient/HttpClientException.h"
#include "../../Network/HttpClient/Http.h"
#include "../../Util/CsvLineDecoder.h"

#include <openssl/md5.h>
#include <iomanip>

#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>

#if defined (WIN32) || defined (WIN64)
#include <Windows.h>
#include <Sspi.h>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif

namespace jedox {
namespace palo {

enum Encryption {
	HttpsNone = 0, HttpsOptional = 1, HttpsRequired = 2
};

class PaloClientImpl {
public:

	PaloClientImpl(ServerProtocol protocol = Http, unsigned int port = 0) :
		m_Port(port), m_Adopted(false), m_Protocol(protocol), m_Encrypt(0), m_HttpPort(port), m_HttpsPort(0), m_Target(""), m_TargetPort(port), m_Sid(""), m_TTL(0)
	{
	}

	PaloClientImpl(const PaloClientImpl& other) :
		m_Hostname(other.m_Hostname), m_Port(other.m_Port), m_Username(other.m_Username), m_Password(other.m_Password), m_PlainPassword(other.m_PlainPassword),
		m_WinSSO(other.m_WinSSO), m_automatic(other.m_automatic), m_finished(other.m_finished), m_negoString(other.m_negoString), m_negotiationId(other.m_negotiationId),
		m_ServerSequenceNumber(other.m_ServerSequenceNumber), m_Adopted(other.m_Adopted), machineString(other.machineString), requiredFeatures(other.requiredFeatures),
		optionalFeatures(other.optionalFeatures), optionalRetFeatures(other.optionalRetFeatures), description(other.description), m_Client(other.m_Client), m_Protocol(other.m_Protocol),
		m_Encrypt(other.m_Encrypt), m_HttpPort(other.m_HttpPort), m_HttpsPort(other.m_HttpsPort), m_Target(other.m_Target), m_TargetPort(other.m_TargetPort), m_Sid(other.m_Sid), m_TTL(other.m_TTL)
	{
	}

	std::unique_ptr<std::istringstream> request(const std::string& command, const std::string& query, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, PaloClient* client, bool headRequest, const std::string *sid, int trials = 4, HEADER_LIST *argHeaders = NULL, unsigned int *responseCode = NULL, bool ignore401 = false)
	{
		std::stringstream querysid;
		unsigned int errorcode;
		int max_trials = trials;
		size_t bodysize = 0;

		CLIENT_RESPONSE_APTR result;
		std::unique_ptr<std::istringstream> stream(new std::istringstream);

		bool requestError = false;
		do {
			querysid << query;

			if (!query.empty() && (!m_Sid.empty() || sid)) {
				querysid << "&";
			}

			std::string curSID;
			if (sid) {
				curSID = *sid;
			} else {
				boost::unique_lock<boost::mutex> wl(sidLock);
				curSID = m_Sid;
			}

			if (!curSID.empty()) {
				querysid << "sid=" << curSID;
			}

			HEADER_LIST headers;
			headers[token.getTokenName()] = util::lexicalConversion(std::string, unsigned int, token.getSequenceNumber());
			if (argHeaders) {
				for (HEADER_LIST::const_iterator it = argHeaders->begin(); it != argHeaders->end(); ++it) {
					headers[it->first] = it->second;
				}
			}

			try {
				result = m_Client.sendRequest(m_Protocol == Https, HttpClientRequest(Url(m_Hostname, m_Port, command, querysid.str()), headers, headRequest ? HttpClientRequest::HEAD : HttpClientRequest::GET), m_Target, m_TargetPort);
				if (result->getResponseCode() != 200) {
					if (ignore401 && result->getResponseCode() == 401) {
						requestError = false;
					} else {
						bodysize = result->getBody().size();
						if (bodysize > 0) {
							util::TOKEN_LIST tokens = util::CsvLineDecoder::decode(std::string(&result->getBody().front(), bodysize - 1));
							errorcode = util::lexicalConversion(unsigned int, std::string, tokens[0].token);

							if (!sid && (errorcode == PaloExceptionFactory::ERROR_INVALID_SESSION) && !m_Username.empty() && (command.compare("/server/login") != 0) && (command.compare("/server/logout") != 0)) {
								if (this->setSID(client, false, curSID)) {
									requestError = true;
								} else {
									PaloExceptionFactory::raise(errorcode, tokens[1].token, tokens.size() >= 3 ? tokens[2].token : tokens[1].token);
								}
							} else {
								PaloExceptionFactory::raise(errorcode, tokens[1].token, tokens.size() >= 3 ? tokens[2].token : tokens[1].token);
							}
						} else {
							PaloExceptionFactory::raise(PaloExceptionFactory::ERROR_INTERNAL, "internal error", "server return empty body");
						}
					}
				} else {
					requestError = false;
				}
			} catch (const HttpClientException& e) {
				requestError = true;
				if (--max_trials == 0) {
					// throw something
					throw MaximumServerRetrysReachedException(e.longDescription());
				}
			}
			querysid.str("");
		} while (requestError);

		if (argHeaders) {
			*argHeaders = result->getHeaders();
		}

		if (responseCode) {
			*responseCode = result->getResponseCode();
		}

		std::string tokenname = token.getTokenName();

		if (result->getHeaders().find(tokenname) != result->getHeaders().end()) {
			sSequenceNumber = util::lexicalConversion(unsigned int, std::string, result->getHeader(tokenname));
			token.setSequenceNumber(sSequenceNumber);
		}

		tokenname = CUBEDATATOKENNAME;
		if (result->getHeaders().find(tokenname) != result->getHeaders().end()) {
			vSequenceNumber = util::lexicalConversion(unsigned int, std::string, result->getHeader(tokenname));
		}

		if (result->getBody().size() > 0) {
			(*stream).str(std::string(&result->getBody().front(), result->getBody().size() - 1));
		} else {
			(*stream).setstate(std::ios::eofbit);
		}
		return stream;
	}

	std::unique_ptr<std::istringstream> negotiate(PaloClientImpl& https_conn, std::stringstream& query, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, PaloClient* client)
	{
		std::unique_ptr<std::istringstream> stream;
#if defined (WIN32) || defined (WIN64)
		std::string authorizationMsg;

		TimeStamp ts;
		SecPkgInfoA *secPackInfo;
		QuerySecurityPackageInfoA((SEC_CHAR *)"Negotiate", &secPackInfo);

		CredHandle credentialsCli;
		char principal[256];
		strncpy(principal, std::string("HTTP/").append(https_conn.m_Hostname).c_str(), 255);
		SECURITY_STATUS retCli = AcquireCredentialsHandleA(principal, (SEC_CHAR *)"Negotiate", SECPKG_CRED_OUTBOUND, 0, 0, 0, 0, &credentialsCli, &ts);

		bool firstCli = true;
		CtxtHandle cliContext;
		ULONG attr;
		SecBufferDesc outputCli;
		SecBuffer secBuffCli;
		SecBufferDesc outputSrv;
		SecBuffer secBuffSrv;
		ULONG contextReq = ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT | ISC_REQ_CONFIDENTIALITY;
		std::string negId = "init";

		boost::shared_array<char> cliBuff(new char[secPackInfo->cbMaxToken]);
		boost::shared_array<char> srvBuff(new char[secPackInfo->cbMaxToken]);

		while (true) {
			secBuffCli.BufferType = SECBUFFER_TOKEN;
			secBuffCli.cbBuffer = secPackInfo->cbMaxToken;
			secBuffCli.pvBuffer = cliBuff.get();

			outputCli.cBuffers = 1;
			outputCli.pBuffers = &secBuffCli;
			outputCli.ulVersion = SECBUFFER_VERSION;

			retCli = InitializeSecurityContextA(&credentialsCli, firstCli ? NULL : &cliContext, (SEC_CHAR *)principal, contextReq, 0, SECURITY_NATIVE_DREP, firstCli ? NULL : &outputSrv, 0, &cliContext, &outputCli, &attr, &ts);

			if (retCli == SEC_I_COMPLETE_NEEDED || retCli == SEC_I_COMPLETE_AND_CONTINUE) {
				CompleteAuthToken(&cliContext, &outputCli);
			} else if (retCli != SEC_I_CONTINUE_NEEDED && retCli != SEC_E_OK) {
				std::string descr = std::string("Not possible to initialize security context (error ") + boost::lexical_cast<std::string>(retCli) + ").";
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
			}

			if (retCli == SEC_E_OK && outputCli.pBuffers[0].cbBuffer == 0) {
				std::string descr = "Output token of zero length, cannot continue negotiation.";
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
			}

			std::string encodedMsg;
			{
				BIO *bmem, *b64;
				BUF_MEM *bptr;
				b64 = BIO_new(BIO_f_base64());
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
				bmem = BIO_new(BIO_s_mem());
				b64 = BIO_push(b64, bmem);
				BIO_write(b64, outputCli.pBuffers[0].pvBuffer, outputCli.pBuffers[0].cbBuffer);
				(void)BIO_flush(b64);
				BIO_get_mem_ptr(b64, &bptr);
				encodedMsg = std::string(bptr->data, bptr->length);
				BIO_free_all(b64);
			}

			HEADER_LIST headers;
			headers["Authorization"] = "Negotiate " + encodedMsg;
			headers["negotiation-id"] = negId;
			unsigned int responseCode;
			stream = https_conn.request("/server/login", query.str(), token, sSequenceNumber, vSequenceNumber, client, false, 0, 2, &headers, &responseCode, true);

			std::string serverAnswer;
			if (responseCode == 200) {
				break;
			} else if (responseCode == 401) {
				HEADER_LIST::const_iterator it = headers.find("www-authenticate");
				if (it != headers.end()) {
					serverAnswer = it->second;
				} else {
					std::string descr = "Invalid response from server (www-authenticate not received).";
					LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
				}
			} else {
				std::string descr = std::string("Invalid response from server (code ") + boost::lexical_cast<std::string>(responseCode) + ").";
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
			}

			negId = headers.find("negotiation-id")->second;

			if (serverAnswer.substr(0, 10) == "Negotiate ") {
				serverAnswer = serverAnswer.substr(10, std::string::npos);
			} else {
				std::string descr("Invalid response from server (Negotiate substring not found).");
				LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
			}

			int size = 0;
			{
				BIO *bmem, *b64;
				b64 = BIO_new(BIO_f_base64());
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
				bmem = BIO_new_mem_buf((void *)serverAnswer.c_str(), (int)serverAnswer.size());
				b64 = BIO_push(b64, bmem);
				size = BIO_read(b64, srvBuff.get(), secPackInfo->cbMaxToken);
				BIO_free_all(b64);
			}
	
			secBuffSrv.BufferType = SECBUFFER_TOKEN;
			secBuffSrv.cbBuffer = (unsigned long)size;
			secBuffSrv.pvBuffer = srvBuff.get();
	
			outputSrv.cBuffers = 1;
			outputSrv.pBuffers = &secBuffSrv;
			outputSrv.ulVersion = SECBUFFER_VERSION;

			firstCli = false;
		}

		FreeContextBuffer(secPackInfo);
#else
		std::string descr = "SSO negotiation possible only on Windows platform.";
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_NEGOTIATION_FAILED, &descr);
#endif
		return stream;
	}

	bool setSID(PaloClient* client, bool checkOnly, std::string oldSID)
	{
		boost::unique_lock<boost::mutex> wl(sidLock);
		bool ret = true;
		if (!m_Adopted && !(!oldSID.empty() && m_WinSSO && !m_automatic)) {
			if (oldSID.empty() || oldSID == m_Sid) {
				LOGIN_DATA result;

				if (!checkOnly) {
					m_Sid.clear();
				}

				std::stringstream query;

				if (!m_WinSSO) {
					query << "user=";
					jedox::util::URLencoder(query, m_Username);
					query << "&password=" << m_Password;
				}
				query << "&machine=" << machineString;
				query << "&required=" << requiredFeatures;
				query << "&optional=" << optionalFeatures;
				if (!description.empty()) {
					query << "&new_name=";
					jedox::util::URLencoder(query, description);
				}

				if (checkOnly) {
					query << "&type=1";
				}

				SERVER_TOKEN servertoken(m_ServerSequenceNumber);
				std::unique_ptr<std::istringstream> stream;
				unsigned int dummy;

				PaloClientImpl https_conn(*this);
				if (m_Encrypt != HttpsNone) {
					https_conn.m_Port = m_HttpsPort;
					https_conn.m_Protocol = Https;
					query << "&extern_password=";
					jedox::util::URLencoder(query, m_PlainPassword);
				}
				query << "&external_identifier=" << setlocale(LC_COLLATE, 0);
				try {
					if (!m_WinSSO) {
						stream = https_conn.request("/server/login", query.str(), servertoken, m_ServerSequenceNumber, dummy, client, false, 0, 2);
					} else {
						if (m_automatic) {
							stream = negotiate(https_conn, query, servertoken, m_ServerSequenceNumber, dummy, client);
						} else {
							if (!m_negoString || m_negoString->empty()) {
								std::string descr = "Negotiation string required for WinSSO login.";
								LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
							}
							if (!m_negotiationId) {
								std::string descr = "Negotiation id required for WinSSO login.";
								LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
							}
							if (!m_finished) {
								std::string descr = "Boolean parameter for \"finished\" state required.";
								LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
							}
							HEADER_LIST headers;
							headers["Authorization"] = *m_negoString;
							headers["negotiation-id"] = *m_negotiationId;
							unsigned int responseCode;
							stream = https_conn.request("/server/login", query.str(), servertoken, m_ServerSequenceNumber, dummy, client, false, 0, 2, &headers, &responseCode, true);
							if (responseCode == 200) {
								*this->m_finished = true;
							} else {
								HEADER_LIST::const_iterator it = headers.find("www-authenticate");
								if (it != headers.end()) {
									*this->m_finished = false;
									this->m_wwwAuthenticate = it->second;
									*this->m_negotiationId = headers.find("negotiation-id")->second;
									return true;
								} else {
									std::string descr = "HTTP header www-authenticate not found.";
									LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_WRONG_PARAMETER, &descr);
								}
							}
						}
					}
				} catch (PaloServerException e) {
					if (e.code() == PaloExceptionFactory::ERROR_WORKER_AUTHORIZATION_FAILED) {
						// authentication is done externally, so try it again with plain password.
						query.str("");
						query << "user=";
						jedox::util::URLencoder(query, m_Username);
						query << "&password=";
						jedox::util::URLencoder(query, m_PlainPassword);
						query << "&machine=" << machineString;
						query << "&required=" << requiredFeatures;
						query << "&optional=" << optionalFeatures;
						if (!description.empty()) {
							query << "&new_name=";
							jedox::util::URLencoder(query, description);
						}
						if (checkOnly) {
							query << "&type=1";
						}
						stream = https_conn.request("/server/login", query.str(), servertoken, m_ServerSequenceNumber, dummy, client, false, 0, 2);
					} else {
						throw e;
					}
				}

				(*stream) >> csv >> result;

				if (!oldSID.empty()) {
					if (optionalRetFeatures != result.optFeatures) {
						ret = false;
					} else {
						m_Sid = result.sid;
					}
				} else {
					if (!checkOnly) {
						m_Sid = result.sid;
						m_TTL = result.ttl;
						optionalRetFeatures = result.optFeatures;
					}
				}
			}
		} else {
			ret = false;
		}
		return ret;
	}

	void setSID(const std::string& sid)
	{
		boost::unique_lock<boost::mutex> wl(sidLock);
		m_Adopted = true;
		m_Sid = sid;
	}

	void ActivateHttps(bool activate)
	{
		if (activate && (m_Encrypt != HttpsNone)) {
			m_Protocol = Https;
			m_Port = m_HttpsPort;
		} else if (!activate && (m_Encrypt != HttpsRequired)) {
			m_Protocol = Http;
			m_Port = m_HttpPort;
		}
	}

	void SetHttpsParams(unsigned int encrypt, unsigned int port)
	{
		m_HttpsPort = port;

		if (m_HttpsPort == 0) {
			m_Encrypt = HttpsNone;
		} else {
			m_Encrypt = encrypt;
		}

		switch (m_Encrypt) {

		case HttpsRequired:
			ActivateHttps(true);
			break;

		case HttpsNone:
			ActivateHttps(false);
			break;
		}
	}

	const std::string& getSID() const
	{
		boost::unique_lock<boost::mutex> wl(const_cast<PaloClientImpl *>(this)->sidLock);
		return m_Sid;
	}

	unsigned int getTTL() const
	{
		return m_TTL;
	}

	void setTTL(unsigned int ttl)
	{
		m_TTL = ttl;
	}

	void Check4Proxy()
	{
		std::string::size_type pos1 = m_Hostname.find("@");
		if (pos1 != std::string::npos) {
			std::string::size_type pos2 = m_Hostname.find(":", pos1);
			if (pos2 != std::string::npos) {
				m_TargetPort = m_Port;
				m_Target = m_Hostname.substr(0, pos1);
				m_Port = atoi(m_Hostname.substr(pos2 + 1).c_str());
				m_HttpPort = m_Port;
				m_Hostname = m_Hostname.substr(pos1 + 1, pos2 - pos1 - 1);
			}
		}
	}

	USER_INFO getUserInfo(PaloClient* client, const std::string& sid)
	{
		unsigned int dummy, sequencenumber = 0;
		SERVER_TOKEN servertoken(sequencenumber);

		std::unique_ptr<std::istringstream> stream = request("/server/user_info", "show_permission=1&show_info=1", servertoken, sequencenumber, dummy, client, false, &sid);
		USER_INFO ui;
		(*stream) >> csv >> ui;
		return ui;
	}

	void setAdopted(bool a)
	{
		boost::unique_lock<boost::mutex> wl(const_cast<PaloClientImpl *>(this)->sidLock);
		m_Adopted = a;
	}

	std::string m_Hostname;
	unsigned int m_Port;
	std::string m_Username;
	std::string m_Password;
	std::string m_PlainPassword;
	bool m_WinSSO;
	bool m_automatic;
	bool *m_finished;
	const std::string *m_negoString;
	std::string *m_negotiationId;
	std::string m_wwwAuthenticate;
	unsigned int m_ServerSequenceNumber;
	bool m_Adopted;
	std::string machineString;
	std::string requiredFeatures;
	std::string optionalFeatures;
	std::string optionalRetFeatures;
	std::string description;
	std::string m_OrigHost;
	unsigned int m_OrigPort;

private:
	HttpClient m_Client;
	ServerProtocol m_Protocol;
	unsigned short m_Encrypt;
	unsigned int m_HttpPort;
	unsigned int m_HttpsPort;
	std::string m_Target;
	unsigned int m_TargetPort;
	std::string m_Sid;
	unsigned int m_TTL;
	boost::mutex sidLock;
};

PaloClient::~PaloClient()
{
	// we cannot use sockets during excel tear-down
}

PaloClient::PaloClient(const std::string& username, const std::string& password, const std::string& hostname, unsigned int port, ServerProtocol protocol, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures) :
	m_PaloClientImpl(new PaloClientImpl(protocol, port))
{
	Init(username, password, hostname, port, machineString, requiredFeatures, optionalFeatures, false, false, NULL, NULL, NULL);
}

PaloClient::PaloClient(const std::string& hostname, unsigned int port, ServerProtocol protocol, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId) :
	m_PaloClientImpl(new PaloClientImpl(protocol, port))
{
	Init("", "", hostname, port,machineString, requiredFeatures, optionalFeatures, true, automatic, finished, negoString, negotiationId);
}

void PaloClient::Init(const std::string& username, const std::string& password, const std::string& hostname, unsigned int port, const std::string& machineString, const std::string& requiredFeatures, const std::string& optionalFeatures, bool winSSO, bool automatic, bool *finished, const std::string *negoString, std::string *negotiationId)
{
	m_PaloClientImpl->m_Username = username;
	m_PaloClientImpl->m_PlainPassword = password;
	m_PaloClientImpl->m_Hostname = hostname;
	m_PaloClientImpl->m_OrigHost = hostname;
	m_PaloClientImpl->m_Port = port;
	m_PaloClientImpl->m_OrigPort = port;
	m_PaloClientImpl->m_ServerSequenceNumber = 0;
	m_PaloClientImpl->m_WinSSO = winSSO;
	m_PaloClientImpl->m_automatic = automatic;
	m_PaloClientImpl->m_finished = finished;
	m_PaloClientImpl->m_negoString = negoString;

	if (negotiationId && negotiationId->empty()) {
		*negotiationId = "init";
	}
	m_PaloClientImpl->m_negotiationId = negotiationId;

	m_PaloClientImpl->m_Password = getMD5(password);
	//hashed password not accepted anymore: issue 15480
	//if (password.substr(0,3) == "\t1\t") {
	//	m_PaloClientImpl->m_Password = password.substr(3);
	//} else {
	//	m_PaloClientImpl->m_Password = getMD5(password);
	//}
	m_PaloClientImpl->machineString = machineString;
	m_PaloClientImpl->requiredFeatures = requiredFeatures;
	m_PaloClientImpl->optionalFeatures = optionalFeatures;
}

PaloClient::PaloClient(const std::string& hostname, unsigned int port, const std::string& sessionid, ServerProtocol protocol) :
	m_PaloClientImpl(new PaloClientImpl(protocol, port))
{
	m_PaloClientImpl->m_Hostname = hostname;
	m_PaloClientImpl->m_Port = port;
	m_PaloClientImpl->m_Username.clear();
	m_PaloClientImpl->m_Password.clear();
	m_PaloClientImpl->m_PlainPassword.clear();
	m_PaloClientImpl->setSID(sessionid);
}

void PaloClient::connect(bool checkOnly)
{
	m_PaloClientImpl->setSID(const_cast<PaloClient*> (this), checkOnly, "");
}

void PaloClient::ActivateHttps(bool activate)
{
	m_PaloClientImpl->ActivateHttps(activate);
}

void PaloClient::SetHttpsParams(unsigned int encrypt, unsigned int port)
{
	m_PaloClientImpl->SetHttpsParams(encrypt, port);
}

void PaloClient::Check4Proxy()
{
	m_PaloClientImpl->Check4Proxy();
}

PaloClient::PaloClient(const PaloClient& other) : m_PaloClientImpl(new PaloClientImpl(*other.m_PaloClientImpl))
{
}

std::unique_ptr<std::istringstream> PaloClient::request(const std::string& command, const std::string& query, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, const bool headRequest) const
{
	return m_PaloClientImpl->request(command, query, token, sSequenceNumber, vSequenceNumber, const_cast<PaloClient*> (this), headRequest, 0);
}

std::unique_ptr<std::istringstream> PaloClient::request(const std::string& command, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, const bool headRequest) const
{
	std::string par1("");
	return m_PaloClientImpl->request(command, par1, token, sSequenceNumber, vSequenceNumber, const_cast<PaloClient*> (this), headRequest, 0);
}

std::unique_ptr<std::istringstream> PaloClient::request(const std::string& command, TOKEN& token, unsigned int& sSequenceNumber, unsigned int& vSequenceNumber, const std::string &sid) const
{
	std::string par1("");
	return m_PaloClientImpl->request(command, par1, token, sSequenceNumber, vSequenceNumber, const_cast<PaloClient*> (this), false, &sid);
}

PaloClient& PaloClient::operator=(const PaloClient& rhs)
{
	m_PaloClientImpl.reset(new PaloClientImpl(*rhs.m_PaloClientImpl));
	return *this;
}

std::string PaloClient::getSID() const
{
	return m_PaloClientImpl->getSID();
}

unsigned int PaloClient::getTTL() const
{
	return m_PaloClientImpl->getTTL();
}

void PaloClient::changePassword(const std::string& password)
{
	m_PaloClientImpl->m_Password = getMD5(password);
	m_PaloClientImpl->m_PlainPassword = password;
	m_PaloClientImpl->setAdopted(false);
}

std::string PaloClient::getUsername()
{
	return m_PaloClientImpl->m_Username;
}

std::string PaloClient::getHost()
{
	return m_PaloClientImpl->m_OrigHost;
}

unsigned int PaloClient::getPort()
{
	return m_PaloClientImpl->m_OrigPort;
}

bool PaloClient::getAutomatic() const
{
	return m_PaloClientImpl->m_automatic;
}

bool PaloClient::getFinished() const
{
	if (m_PaloClientImpl->m_finished) {
		return *m_PaloClientImpl->m_finished;
	} else {
		return true;
	}
}

std::string PaloClient::getWWWAuthenticateMsg() const
{
	return m_PaloClientImpl->m_wwwAuthenticate;
}

void PaloClient::setUsernameTTL(const std::string &name, unsigned int ttl)
{
	m_PaloClientImpl->m_Username = name;
	m_PaloClientImpl->setTTL(ttl);
}

void PaloClient::setDescription(const std::string &description)
{
	m_PaloClientImpl->description = description;
}

bool PaloClient::isAdopted() const
{
	return m_PaloClientImpl->m_Adopted;
}

void PaloClient::setAdopted()
{
	m_PaloClientImpl->setAdopted(true);
}

std::string PaloClient::getOptionalFeatures()
{
	return m_PaloClientImpl->optionalRetFeatures;
}

std::string PaloClient::getMD5(const std::string &s)
{
	unsigned char md[16];
	MD5((const unsigned char *)s.c_str(), s.size(), md);
	std::stringstream str;
	str << std::hex << std::setfill('0');
	for (int i = 0; i < 16; ++i) str << std::setw(2) << (int)md[i];
	return str.str();
}

USER_INFO PaloClient::getUserInfo(const std::string& sid) const
{
	return m_PaloClientImpl->getUserInfo(const_cast<PaloClient*> (this), sid);
}

} /* palo */
} /* jedox */
