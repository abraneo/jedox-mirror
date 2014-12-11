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

#ifndef HTTPS_SERVER_PALO_HTTPS_SERVER_H
#define HTTPS_SERVER_PALO_HTTPS_SERVER_H 1

#include "palo.h"

#include "HttpsServer/HttpsServer.h"
#include "PaloHttpServer/PaloHttpServer.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////
/// @brief http server
///
/// The http server is implemented as a listener task. It listens on a port
/// for a client connection. As soon as a client requests a connection the
/// function handleConnected is called. It will then create a new read/write
/// task used to communicate with the client.
////////////////////////////////////////////////////////////////////////////////

class HTTPS_CLASS PaloHttpsServer : public HttpsServer, public PaloHttpServer {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloHttpsServer(const string& rootfile, const string& keyfile, const string& password, const string& dhfile, const string& templateDirectory) :
		IoTask(INVALID_SOCKET, INVALID_SOCKET), HttpServer(), HttpsServer(rootfile, keyfile, password, dhfile), PaloHttpServer(templateDirectory) {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	Task* handleConnected(socket_t fd) {
		return HttpsServer::handleConnected(fd);
	}
};

}

#endif
