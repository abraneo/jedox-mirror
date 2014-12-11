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

#ifndef HTTP_SERVER_HTTP_SERVER_H
#define HTTP_SERVER_HTTP_SERVER_H 1

#include "palo.h"

#include "InputOutput/FileUtils.h"
#include "Scheduler/ListenTask.h"

namespace palo {
class HttpRequest;
class HttpRequestHandler;
class HttpResponse;
class HttpServerTask;
class JobAnalyser;
class ErrorException;
class HttpJobRequest;

////////////////////////////////////////////////////////////////////////////////
/// @brief http server
///
/// The http server is implemented as a listener task. It listens on a port
/// for a client connection. As soon as a client requests a connection the
/// function handleConnected is called. It will then create a new read/write
/// task used to communicate with the client.
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpServer : public ListenTask {
	friend class MTPaloHttpInterface;
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new http server
	///
	/// Constructs a new http server. The port to which the http server listens
	/// must be specified when adding the task to the scheduler.
	////////////////////////////////////////////////////////////////////////////////

	HttpServer();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes a http server
	////////////////////////////////////////////////////////////////////////////////

	virtual ~HttpServer();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets a new trace file
	////////////////////////////////////////////////////////////////////////////////

	void setTraceFile(const string& name);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds a new request handler
	///
	/// The functions add a new request handler for the path given. If the http
	/// server receives an request with this path it calls the
	/// HttpRequestHandler::handleHttpRequest method of the handler. This method
	/// must a return a suitable HttpResponse object.
	////////////////////////////////////////////////////////////////////////////////

	void addHandler(const string& path, HttpRequestHandler*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds a default request handler
	///
	/// The functions add a new request handler which is called in cases where
	/// no request handler has been defined for the given path. The http
	/// server calls the HttpRequestHandler::handleHttpRequest method of the
	/// handler. This method must a return a suitable HttpResponse object.
	////////////////////////////////////////////////////////////////////////////////

	void addNotFoundHandler(HttpRequestHandler*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief handles client request
	///
	/// The callback functions is called by the HttpServerTask if the client has
	/// received a new, syntactically valid request.
	////////////////////////////////////////////////////////////////////////////////

	void handleRequest(HttpServerTask*, HttpRequest*);
	HttpJobRequest *handleException(const ErrorException& e, HttpRequest* request);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief create a http request
	////////////////////////////////////////////////////////////////////////////////

	virtual HttpRequest * createHttpRequest(const string& url);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////
	Task* handleConnected(socket_t);

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////
	void handleShutdown();

#ifdef ENABLE_TRACE_OPTION
	void traceRequest(const string& text) {
		if (trace) {
			*trace << text << endl;
		}
	}
#endif

	void setParams(const std::string& address, int port) {
		this->address = address;
		this->port = port;
	}

	bool start(JobAnalyser* analyser) {
		this->analyser = analyser;
		return Listen(address, port);
	}

	void SetAnalyser(JobAnalyser* analyser) {
		this->analyser = analyser;
	}

	HttpServerTask* CreateConnectionTask();
private:
	FileUtils::paloofstream *trace;

	map<string, HttpRequestHandler*> handlers;
	HttpRequestHandler* notFoundHandler;
	std::string address;
	int port;
protected:
	JobAnalyser* analyser;
};

}

#endif
