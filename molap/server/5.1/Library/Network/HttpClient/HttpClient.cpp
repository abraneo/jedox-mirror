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
 * \author Marko Stijak <mstijak@gmail.com>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#   pragma warning( push )
#   pragma warning( disable : 4996 )
#endif

#include <iostream>
#include <boost/iostreams/filtering_stream.hpp>

#if defined(WIN32) || defined(WIN64)
#   pragma warning( pop )
#endif

#include <boost/range/iterator_range.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "Url.h"
#include "HttpClient.h"

#include "HttpClientConnection.h"

#include "HttpClientResponse.h"
#include "HttpClientRequest.h"

#include "HttpClientConnectionPool.h"

#include "HttpsClientConnection.h"
#include "HttpsClientConnectionPool.h"

#include "BasicAuthentication.h"

#include "../../Exceptions/ErrorException.h"

/**
 *	This is a first draft version of the HttpClient. I'm currently getting deeper into the HTTP protocol and as I
 *	learn about it, the implementation _will_ change to meet the requirements (of good OOP and the RFC2616).
 *	I have, however, not the time by now to get into this alto deep, so some things in the current implementation
 *	might be right-out wrong or only half-heartedly addressed. I will get to that later but meanwhile any comments
 *	& fixes are welcome.
 *
 */

/*  TODO
 *
 *  - Respond to HTTP/1.0 401 Unauthorized, WWW-Authenticate: Basic realm="SokEvo"
 *    Authentication-Realm
 *  - Add option that defines if an HEAD request should be issued before the real request
 *  - Fully implement redirections
 *  - Handle disconnected HttpClientConnections (maybe transparently implemented in the HttpClientConnection)
 *  - Header fields can extend over multiple lines
 *  - Header fields may not be unique!?
 *  - Proper exception handling
 */

/*	Major parts of the documentation for this source-file shamelessly taken from the RFC 2616 for which the following
 copyright informations apply:

 Copyright (C) The Internet Society (1999). All Rights Reserved.
 This document and translations of it may be copied and furnished to others, and derivative works that comment on or
 otherwise explain it or assist in its implementation may be prepared, copied, published and distributed, in whole or
 in part, without restriction of any kind, provided that the above copyright notice and this paragraph are included
 on all such copies and derivative works.
 However, this document itself may not be modified in any way, such as by removing the copyright notice or references
 to the Internet Society or other Internet organizations, except as needed for the purpose of developing Internet
 standards in which case the procedures for copyrights defined in the Internet Standards process must be followed,
 or as required to translate it into languages other than English. The limited permissions granted above are perpetual
 and will not be revoked by the Internet Society or its successors or assigns.

 This document and the information contained herein is provided on an
 "AS IS" basis and THE INTERNET SOCIETY AND THE INTERNET ENGINEERING TASK FORCE DISCLAIMS ALL WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE INFORMATION HEREIN WILL NOT INFRINGE ANY
 RIGHTS OR ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. */

static inline void getHeaders(palo::HEADER_LIST& headerList, std::iostream& stream)
{
	/*	Each header field consists of a name followed by a colon (":") and the field value. Field names are case-insensitive.
	 The field value MAY be preceded by any amount of LWS, though a single SP is preferred.
	 Header fields can be extended over multiple lines by preceding each extra line with at least one SP or HT.
	 Applications ought to follow "common form", where one is known or indicated, when generating HTTP constructs,
	 since there might exist some implementations that fail to accept anything beyond the common forms. */

	/*	There are a few header fields which have general applicability for both request and response messages, but which do
	 not apply to the entity being transferred. These header fields apply only to the message being transmitted. */

	/*	The response-header fields allow the server to pass additional information about the response which cannot be placed
	 in the Status- Line. These header fields give information about the server and about further access to the resource
	 identified by the Request-URI. */

	/*	Entity-header fields define meta information about the entity-body or, if no body is present, about the resource
	 identified by the request. Some of this meta information is OPTIONAL; some might be REQUIRED by portions of this
	 specification. */
	std::string line;
	do {
		// TODO Header fields can extend over multiple lines
		// TODO Header fields may not be unique!?
		std::getline(stream, line, '\n');
		std::string::size_type seperatorPos = line.find_first_of(":");
		if (seperatorPos != std::string::npos) {
			// Convert all field-names to lower-case since they are case-insensitive by definition
			headerList[boost::algorithm::to_lower_copy(line.substr(0, seperatorPos))] = boost::algorithm::trim_copy(line.substr(seperatorPos + 1));
		}
	} while (line != "\r");
}

static inline palo::CLIENT_RESPONSE_APTR sendRequest(boost::shared_ptr<std::iostream> stream, const palo::HttpClientRequest& clientRequest, palo::HttpClient::ClientMode mode, bool &outClosesConnection, const std::string &Target, unsigned int TargetPort)
{
	static const char CRLF[] = "\r\n";
	static const char END_OF_HEADER[] = "\r\n";

	const palo::Url& url = clientRequest.getUrl();

	bool useProxy = !Target.empty();

	/*	The Request-Line begins with a method token, followed by the Request-URI and the protocol version, and ending with
	 CRLF. The elements are separated by SP characters. No CR or LF is allowed except in the final CRLF sequence.*/

	const palo::HttpClientRequest::Method method = clientRequest.getMethod();
	switch (method) {
	case palo::HttpClientRequest::POST:
		/*	The POST method is used to request that the origin server accept the entity enclosed in the request as a new
		 subordinate of the resource identified by the Request-URI in the Request-Line. */
		break;

	case palo::HttpClientRequest::GET:
	case palo::HttpClientRequest::HEAD:
		if (method == palo::HttpClientRequest::GET) {
			/*	The GET method means retrieve whatever information (in the form of an entity) is identified by the Request-URI.
			 If the Request-URI refers to a data-producing process, it is the produced data which shall be returned as the
			 entity in the response and not the source text of the process, unless that text happens to be the output of the
			 process. */
			(*stream) << "GET ";
		} else {
			/*	The HEAD method means retrieve only the header information which is identified by the Request-URI. */
			(*stream) << "HEAD ";
		}

		if (useProxy) {
			(*stream) << "http" << ((url.getScheme() == 1) ? "s" : "") << "://" << Target << ":" << TargetPort;
		}
		(*stream) << url.getPath();
		if (url.isQuerySet()) {
			(*stream) << "?" << url.getQuery();
		} else {
			(*stream) << "/";
		}

		break;

	default:
		throw palo::ErrorException(palo::ErrorException::ERROR_INTERNAL, "undefined request");
	}
	(*stream) << " HTTP/1.1" << CRLF;

	/*	The Content-Length entity-header field indicates the size of the entity-body, in decimal number of OCTETs,
	 sent to the recipient or, in the case of the HEAD method, the size of the entity-body that would have been sent
	 had the request been a GET. */
	(*stream) << "Content-Length: " << (clientRequest.getMethod() == palo::HttpClientRequest::POST ? url.getQuerySize() : 0) << CRLF;

	/*	The Host request-header field specifies the Internet host and port number of the resource being requested,
	 as obtained from the original URI given by the user or referring resource (generally an HTTP URL, as described
	 in section 3.2.2). The Host field value MUST represent the naming authority of the origin server or gateway
	 given by the original URL. This allows the origin server or gateway to differentiate between internally-ambiguous
	 URLs, such as the root "/" URL of a server for multiple host names on a single IP address.*/
	(*stream) << "Host: ";
	if (useProxy) {
		(*stream) << Target << ":" << TargetPort;
	} else {
		(*stream) << url.getHostname() << ":" << url.getPort();
	}
	(*stream) << CRLF;

	///*	A user agent that wishes to authenticate itself with a server-- usually, but not necessarily, after receiving
	// a 401 response--does so by including an Authorization request-header field with the request. The Authorization
	// field value consists of credentials containing the authentication information of the user agent for the realm
	// of the resource being requested.*/

	//if (mode & palo::HttpClient::PERFORM_AUTHENTICATION && url.isAuthenticatable()) {
	//	(*stream) << palo::BasicAuthentication(url.getUsername(), url.getPassword()).getHeader();
	//}

	/*	HTTP/1.1 defines the "close" connection option for the sender to signal that the connection will be closed
	 after completion of the response. For example, Connection: close in either the request or the response header
	 fields indicates that the connection SHOULD NOT be considered `persistent' (section 8.1) after the current
	 request/response is complete. */
	if (mode & palo::HttpClient::PERFORM_KEEP_ALIVE) {
		(*stream) << "Connection: Keep-Alive" << CRLF;
	} else {
		(*stream) << "Connection: close" << CRLF;
	}

	(*stream) << "Accept-Encoding: identity, *;q=0" << CRLF;

	for (palo::HEADER_LIST::const_iterator it = clientRequest.getHeaderList().begin(); it != clientRequest.getHeaderList().end(); ++it) {
		(*stream) << it->first << ": " << it->second << CRLF;
	}

	// End of header
	(*stream) << END_OF_HEADER;

	// If this is an POST request we still have to send our query in the body
	if (clientRequest.getMethod() == palo::HttpClientRequest::POST) {
		(*stream) << url.getQuery();
	}

	// Send the request
	(*stream).flush();

	std::string protocolVersionText;
	unsigned int statusCode = 0;
	std::string statusText;

	/*	The first line of a Response message is the Status-Line, consisting of the protocol version followed by a numeric status
	 code and its associated textual phrase, with each element separated by SP characters. No CR or LF is allowed except in
	 the final CRLF sequence. */
	std::string line;
	std::getline((*stream), line, '\r');
	std::vector<std::string> header;
	boost::algorithm::split(header, line, boost::algorithm::is_space());
	if (header.size() > 2) {
		protocolVersionText = header[0];
		statusCode = lexicalConversion(unsigned int, std::string, header[1]);
		for (std::vector<std::string>::iterator it = header.begin() + 2; it < header.end(); ++it) {
			statusText.append(*it);
		}
	} else {
		throw palo::ErrorException(palo::ErrorException::ERROR_INTERNAL, "invalid header");
	}

	// Get response headers. See the documentation in the getHeaders function
	palo::HEADER_LIST headerList;
	getHeaders(headerList, *stream);

	/*	RFC 2616 - 4.4 Message Length extract
	 Any response message which "MUST NOT" include a message-body (such as the 1xx, 204, and 304 responses and any response
	 to a HEAD request) is always terminated by the first empty line after the header fields, regardless of the entity-header
	 fields present in the message. */
	std::vector<char> body;
	bool responseHasBody = (statusCode < 100 || statusCode > 199) && statusCode != 204 && statusCode != 304;
	if (responseHasBody) {
		// Receive the body
		enum {
			BufferSize = 4096
		};
		size_t contentLength = BufferSize;
		bool usesChunkedTransfer = false;
		bool closesConnection = false;
		bool usesContentLegth = false;

		/*	RFC 2616 - 4.4 Message Length extract
		 2.If a Transfer-Encoding header field (section 14.41) is present and has any value other than "identity", then the
		 transfer-length is defined by use of the "chunked" transfer-coding (section 3.6), unless the message is terminated
		 by closing the connection.

		 3.If a Content-Length header field (section 14.13) is present, its decimal value in OCTETs represents both the
		 entity-length and the transfer-length. The Content-Length header field MUST NOT be sent if these two lengths are
		 different (i.e., if a Transfer-Encoding header field is present). If a message is received with both a Transfer-
		 Encoding header field and a Content-Length header field, the latter MUST be ignored. */
		if (headerList.find("transfer-encoding") != headerList.end()) {
			if (headerList["transfer-encoding"].compare("identity") == 0) {
				usesChunkedTransfer = false;
			} else if (headerList["transfer-encoding"].compare("chunked") == 0) {
				usesChunkedTransfer = true;
			}
		} else if (headerList.find("content-length") != headerList.end()) {
			contentLength = lexicalConversion(size_t, std::string, headerList["content-length"]);
			usesContentLegth = true;
		} else {
			// TODO Support multi part/byte ranges
		}

		if (headerList.find("connection") != headerList.end() && headerList["connection"].compare("Keep-Alive") == 0) {
			closesConnection = false;
		} else {
			closesConnection = true;
		}

		if (usesChunkedTransfer == true) {
			/*	The chunk-size field is a string of hex digits indicating the size of the chunk. The chunked encoding is ended by
			 any chunk whose size is zero, followed by the trailer, which is terminated by an empty line. */
			size_t bufferSize = contentLength;
			boost::scoped_array<char> buffer(new char[bufferSize]);
			do {
				// Get the length of the content (provided as an hex string)
				(*stream) >> std::hex >> contentLength;
				// Only resize the buffer if we have to
				if (contentLength > bufferSize) {
					bufferSize = contentLength;
					buffer.reset(new char[bufferSize]);
				}
				// Ignore the CRLF following the chunk length
				(*stream).ignore(2);
				(*stream).read(buffer.get(), contentLength);
				body.insert(body.end(), buffer.get(), buffer.get() + (*stream).gcount());
				getHeaders(headerList, *stream);
			} while (contentLength > 0);
		} else if (usesContentLegth == true) {
			/*	The Content-Length entity-header field indicates the size of the entity-body, in decimal number of OCTETs, sent to
			 the recipient*/
			boost::scoped_array<char> buffer(new char[contentLength]);
			body.reserve(contentLength);
			(*stream).read(buffer.get(), contentLength);
			body.insert(body.end(), buffer.get(), buffer.get() + (*stream).gcount());
		} else {
			// Reads until the connection closes or an timeout occurs
			boost::scoped_array<char> buffer(new char[contentLength]);
			body.reserve(contentLength);
			while ((*stream).eof() == false) {
				(*stream).read(buffer.get(), contentLength);
				body.insert(body.end(), buffer.get(), buffer.get() + (*stream).gcount());
			}
		}

		// If the connection supports keep-alive, return it to the ConnectionPool
		outClosesConnection = closesConnection;

	}

	// Return the request result
	return palo::CLIENT_RESPONSE_APTR(new palo::HttpClientResponse(clientRequest, protocolVersionText, statusCode, headerList, body));;
}

static inline palo::CLIENT_RESPONSE_APTR sendHttpsRequest(const palo::HttpClientRequest& clientRequest, palo::HttpClient::ClientMode mode, const std::string &Target, unsigned int TargetPort)
{

	static palo::HttpsClientConnectionPool &connectionPool = palo::HttpsClientConnectionPool::instance();
	std::unique_ptr<palo::HttpsClientConnection> connection = connectionPool.adoptClientConnection(clientRequest.getUrl());

	if (!connection->hasValidCertificate()) {
		throw palo::ErrorException(palo::ErrorException::ERROR_INTERNAL, "invalid certificate");
	}

	bool closesConnection;
	palo::CLIENT_RESPONSE_APTR res(sendRequest(connection->getStream(), clientRequest, mode, closesConnection, Target, TargetPort));

	// If the connection supports keep-alive, return it to the ConnectionPool
	if (closesConnection == false) {
		connectionPool.returnClientConnection(connection);
	}

	return res;
}

static inline palo::CLIENT_RESPONSE_APTR sendHttpRequest(const palo::HttpClientRequest& clientRequest, palo::HttpClient::ClientMode mode, const std::string &Target, unsigned int TargetPort)
{

	static palo::HttpClientConnectionPool &connectionPool = palo::HttpClientConnectionPool::instance();
	std::unique_ptr<palo::HttpClientConnection> connection = connectionPool.adoptClientConnection(clientRequest.getUrl());

	bool closesConnection;
	palo::CLIENT_RESPONSE_APTR res(sendRequest(connection->getStream(), clientRequest, mode, closesConnection, Target, TargetPort));

	// If the connection supports keep-alive, return it to the ConnectionPool
	if (closesConnection == false) {
		connectionPool.returnClientConnection(connection);
	}
	return res;
}

namespace palo {

HttpClient::HttpClient(ClientMode mode) :
	m_ClientMode(mode)
{
}

CLIENT_RESPONSE_APTR HttpClient::sendRequest(bool UseHttps, const HttpClientRequest& clientRequest, const std::string &Target, unsigned int TargetPort) const
{
	CLIENT_RESPONSE_APTR response;
	bool performRedirect = false;
	unsigned int redirectGuard = 0;
	do {
		if (UseHttps) {
			response = ::sendHttpsRequest(clientRequest, m_ClientMode, Target, TargetPort);
		} else {
			response = ::sendHttpRequest(clientRequest, m_ClientMode, Target, TargetPort);
		}

		/*	301 Moved Permanently
		 The requested resource has been assigned a new permanent URI and any future references to this resource SHOULD
		 use one of the returned URIs. */

		/*	302 Found
		 The requested resource resides temporarily under a different URI. Since the redirection might be altered on occasion,
		 the client SHOULD continue to use the Request-URI for future requests. This response is only cacheable if indicated
		 by a Cache-Control or Expires header field. */

		/*	303 See Other
		 The response to the request can be found under a different URI and SHOULD be retrieved using a GET method on that
		 resource. This method exists primarily to allow the output of a POST-activated script to redirect the user agent
		 to a selected resource. The new URI is not a substitute reference for the originally requested resource.
		 The 303 response MUST NOT be cached, but the response to the second (redirected) request might be cacheable.*/

		/*	If the 301 (or 302) status code is received in response to a request other than GET or HEAD, the user agent MUST NOT
		 automatically redirect the request unless it can be confirmed by the user, since this might change the conditions
		 under which the request was issued. */
		/*
		 performRedirect = m_ClientMode & PERFORM_REDIRECTION
		 && ( ( clientRequest.getMethod() != HttpClientRequest::POST && (response->getResponseCode() == 301 || response->getResponseCode() == 302) ) || (response->getResponseCode() == 303));
		 */
		if (performRedirect) {
			redirectGuard++;
			if (redirectGuard > REDIRECTION_LIMIT) {
				// TODO Throw exception. To many levels of indirection
			}
			// TODO Redirect
		}
	} while (performRedirect);
	return response;
}

const unsigned int HttpClient::REDIRECTION_LIMIT = 20;

} /* palo */
