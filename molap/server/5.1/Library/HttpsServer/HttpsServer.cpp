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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * 
 *
 */

#include "HttpsServer/HttpsServer.h"

#include <openssl/err.h>

#include "Exceptions/CommunicationException.h"
#include "HttpsServer/HttpsServerTask.h"
#include "Logger/Logger.h"
#include "Olap/Server.h"

#if defined(ENABLE_HTTPS_MODULE)
#include "Programs/extension.h"
#endif

namespace palo {

/////////////////////
// SSL help functions
/////////////////////

static string lastSSLError()
{
	char buf[121];

	unsigned long err = ERR_get_error();
	string errstr = ERR_error_string(err, buf);

	return errstr;
}

static string DefaultPassword;

// this is not thread safe
static int passwordCallback(char * buffer, int num, int rwflag, void* userdata)
{
	if (num < (int)DefaultPassword.size() + 1) {
		return 0;
	}

	strcpy(buffer, DefaultPassword.c_str());

	return (int)strlen(buffer);
}

static SSL_CTX * initializeCTX(const string& rootfile, const string& keyfile, const string& password)
{
	static bool initialized = false;

	// global system initialization
	if (!initialized) {
		SSL_library_init();
		SSL_load_error_strings();

		initialized = true;
	}

	// Create our context
	SSL_CTX* sslctx = SSL_CTX_new(TLSv1_2_method());

	// Load our keys and certificates
	if (!SSL_CTX_use_certificate_chain_file(sslctx, keyfile.c_str())) {
		Logger::error << "cannot read certificate from '" << keyfile << "'" << endl;
		Logger::error << lastSSLError() << endl;
		return 0;
	}

	DefaultPassword = password;
	SSL_CTX_set_default_passwd_cb(sslctx, passwordCallback);

	if (!SSL_CTX_use_PrivateKey_file(sslctx, keyfile.c_str(), SSL_FILETYPE_PEM)) {
		Logger::error << "cannot read key from '" << keyfile << "'" << endl;
		Logger::error << lastSSLError() << endl;
		return 0;
	}

	// load the CAs we trust
	if (!SSL_CTX_load_verify_locations(sslctx, rootfile.c_str(), 0)) {
		Logger::error << "cannot read CA list from '" << rootfile << "'" << endl;
		Logger::error << lastSSLError() << endl;
		return 0;
	}

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
	SSL_CTX_set_verify_depth(sslctx, 1);
#endif

	return sslctx;
}

static void destroyCTX(SSL_CTX* sslctx)
{
	SSL_CTX_free(sslctx);
}

static bool loadDHParameters(SSL_CTX* sslctx, const string& file)
{
	BIO* bio = BIO_new_file(file.c_str(), "r");

	if (bio == NULL) {
		Logger::error << "cannot open diffie-hellman file '" << file << "'" << endl;
		Logger::error << lastSSLError() << endl;
		return false;
	}

	DH * ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
	BIO_free(bio);

	if (SSL_CTX_set_tmp_dh(sslctx, ret) < 0) {
		Logger::error << "cannot set diffie-hellman parameters" << endl;
		Logger::error << lastSSLError() << endl;
		return false;
	}

	return true;
}

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

HttpsServer::HttpsServer(SSL_CTX* ctxParam) :
	IoTask(INVALID_SOCKET, INVALID_SOCKET), HttpServer(), ctx(ctxParam)
{
}

HttpsServer::HttpsServer(const string& rootfile, const string& keyfile, const string& password, const string& dhfile) :
	IoTask(INVALID_SOCKET, INVALID_SOCKET), HttpServer()
{

	// initialize the session context
	ctx = initializeCTX(rootfile, keyfile, password);

	if (ctx == 0) {
		throw new CommunicationException(ErrorException::ERROR_SSL_FAILED, lastSSLError());
	}

	// load the diffie-hellman paramaters
	bool ok = loadDHParameters(ctx, dhfile);

	if (!ok) {
		throw new CommunicationException(ErrorException::ERROR_SSL_FAILED, lastSSLError());
	}
}

HttpsServer::~HttpsServer()
{
	destroyCTX(ctx);
}

// /////////////////////////////////////////////////////////////////////////////
// ListenerTask methods
// /////////////////////////////////////////////////////////////////////////////

Task* HttpsServer::handleConnected(socket_t fd)
{
	Logger::info << "trying to establish secure connection" << endl;

	// convert in a SSL BIO structure
	BIO * sbio = BIO_new_socket((int)fd, BIO_NOCLOSE);

	if (sbio == 0) {
		Logger::warning << "cannot build new SSL BIO" << endl;
		Logger::warning << lastSSLError() << endl;
		closesocket(fd);
		fd = INVALID_SOCKET;
		return 0;
	}

	// build a new connection
	SSL * ssl = SSL_new(ctx);

	if (ssl == 0) {
		BIO_free_all(sbio);
		Logger::warning << "cannot build new SSL connection" << endl;
		Logger::warning << lastSSLError() << endl;
		closesocket(fd);
		fd = INVALID_SOCKET;
		return 0;
	}

	// with the above bio
	SSL_set_bio(ssl, sbio, sbio);

	return new HttpsServerTask(fd, this, ssl, sbio, analyser);
}
}
