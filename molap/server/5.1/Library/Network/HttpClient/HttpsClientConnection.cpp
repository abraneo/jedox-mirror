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
 * \author Marko Stijak <mstijak@gmail.com>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#   pragma warning( push )
#   pragma warning( disable : 4996 )
#endif

#include <openssl/err.h>

#include <boost/iostreams/stream.hpp>

#if defined(WIN32) || defined(WIN64)
#   pragma warning( pop )
#endif

#include "../NetInitialisation.h"

#include "HttpsClientConnection.h"
#include "../SocketAddress.h"
#include "../ClientSocket.h"
#include "../TcpDevice/TcpDevice.h"
#include "Url.h"

#include "../../Collections/StringUtils.h"

namespace palo {

class SslDevice : public device<bidirectional> {
	BIO* bio;
public:
	SslDevice(BIO* bio) :
		bio(bio)
	{
	}
	;

	std::streamsize read(char *s, std::streamsize n)
	{
		return BIO_read(bio, s, (UINT)n);
	}
	;

	std::streamsize write(const char *s, std::streamsize n)
	{
		return BIO_write(bio, s, (UINT)n);
	}

	void close(std::ios_base::openmode mode)
	{

	}

	bool is_open() const
	{
		return true;
	}
	;
};

HttpsClientConnection::HttpsClientConnection(const std::string& hostname, unsigned int port) :
	m_Hostname(hostname), m_Port(port), m_hasValidCertificate(false)
{
	/* TODOMD SSL
	m_Bio = BIO_new_ssl_connect((SSL_CTX *)NetInitialisation::instance().getSslContext());
	BIO_get_ssl(m_Bio, &m_Ssl);

	SSL_set_mode(m_Ssl, SSL_MODE_AUTO_RETRY);

	BIO_set_conn_hostname(m_Bio, (hostname + ":" + lexicalConversion(std::string, unsigned int, port)).c_str());

	if (BIO_do_connect(m_Bio) <= 0) {
		BIO_free_all(m_Bio);
		throw SslException(std::string("SSL conection failed! (").append(ERR_error_string(ERR_get_error(), NULL)).append(")"));
	}

	m_hasValidCertificate = SSL_get_verify_result(m_Ssl) == X509_V_OK;
	if (m_hasValidCertificate) {
		X509 *cert = SSL_get_peer_certificate(m_Ssl);
		if (cert) {
			std::string certname = cert->name;
			size_t pos = certname.find("CN=");
			size_t posend = certname.find("/", pos);
			if (pos != std::string::npos) {
				if (UTF8Comparer::compare(hostname.c_str(), certname.substr(pos + 3, posend != std::string::npos ? posend - pos - 3 : std::string::npos).c_str())) {
					m_hasValidCertificate = false;
				}
			} else {
				m_hasValidCertificate = false;
			}
			X509_free(cert);
		} else {
			m_hasValidCertificate = false;
		}
	}
	*/
}

HttpsClientConnection::~HttpsClientConnection()
{
	BIO_free_all(m_Bio);
}

void HttpsClientConnection::reset()
{
	// Close the connection if still open
	// TODOMD SSL
	//SSL_clear(m_Ssl);
}

const std::string& HttpsClientConnection::getHostname() const
{
	return m_Hostname;
}

bool HttpsClientConnection::hasValidCertificate() const
{
	return m_hasValidCertificate;
}

unsigned int HttpsClientConnection::getPort() const
{
	return m_Port;
}

void HttpsClientConnection::incrementRequestCount()
{
	m_RequestCount++;
}

boost::shared_ptr<std::iostream> HttpsClientConnection::getStream()
{
	return boost::shared_ptr<std::iostream>(new boost::iostreams::stream<SslDevice>(SslDevice(m_Bio)));
}
} /* palo */
