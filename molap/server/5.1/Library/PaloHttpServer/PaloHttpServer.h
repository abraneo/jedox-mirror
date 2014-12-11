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

#ifndef PALO_HTTP_SERVER_PALO_HTTP_SERVER_H
#define PALO_HTTP_SERVER_PALO_HTTP_SERVER_H 1

#include "palo.h"

#include "HttpServer/HttpServer.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief http server
///
/// The http server is implemented as a listener task. It listens on a port
/// for a client connection. As soon as a client requests a connection the
/// function handleConnected is called. It will then create a new read/write
/// task used to communicate with the client.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloHttpServer : public virtual HttpServer {

public:

	class HandleCreator {
	public:
		virtual ~HandleCreator();
		virtual HttpRequestHandler* create(bool enabled, bool writeRequest) const;
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new http server
	///
	/// Constructs a new http server. The port to which the http server listens
	/// must be specified when adding the task to the scheduler.
	////////////////////////////////////////////////////////////////////////////////

	PaloHttpServer(const string& templateDirectory);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enables API browser
	////////////////////////////////////////////////////////////////////////////////

	virtual void enableBroswer();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enables server/info commands
	////////////////////////////////////////////////////////////////////////////////

	virtual void enableServerInfo(int httpsPort);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief enables PALO commands
	////////////////////////////////////////////////////////////////////////////////

	virtual void enablePalo();
	virtual void enablePalo(const HandleCreator &handleCreator);
	virtual void enableLogin();

protected:
	virtual void addCommandHandlers(bool enabled, const HandleCreator &handleCreator);

	virtual void addStaticHandlers();

	virtual void addDocumentationHandlers(const string& tmpl);

	virtual void addBrowserHandlers();

protected:
	const string templateDirectory;
};

}

#endif
