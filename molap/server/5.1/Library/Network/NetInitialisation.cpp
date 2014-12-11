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
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#include <WinSock2.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "NetInitialisation.h"
#include "../Exceptions/ErrorException.h"

namespace palo {

NetInitialisation& NetInitialisation::instance()
{
	static NetInitialisation obj;
	return obj;
}

NetInitialisation::NetInitialisation()
{
#if defined(WIN32) || defined(WIN64)
	WSADATA wsaData;
	int wsaret = WSAStartup( 0x101, &wsaData );
#endif
	ctx = NULL;
}

void NetInitialisation::initSSL(std::string trustFile)
{
// TODOMD SSL
/*	if (ctx == NULL) {
		SSL_library_init();
		SSL_load_error_strings();

		ctx = SSL_CTX_new(SSLv23_client_method());
	}

	if ((ctx != NULL) && !trustFile.empty() && !SSL_CTX_load_verify_locations((SSL_CTX *)ctx, trustFile.c_str(), NULL)) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "bad certificate file");
	}*/
}

void * NetInitialisation::getSslContext()
{
	//check if openSSL is initialised
	if (ctx == NULL) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "openssl not initialized");
	}
	return ctx;
}
;

NetInitialisation::~NetInitialisation()
{
	//TODOMD SSL
	/*
	if (ctx != NULL) {
		SSL_CTX_free((SSL_CTX *)ctx);
		ERR_free_strings();
	}*/

#if defined(WIN32) || defined(WIN64)
	WSACleanup();
#endif
}

} /* palo */
