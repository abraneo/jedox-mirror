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

#include "HttpServer/HttpServer.h"

#include <iostream>

#include "Collections/StringBuffer.h"

#include "Collections/StringUtils.h"
#include "HttpServer/DirectHttpResponse.h"
#include "HttpServer/HttpJobRequest.h"
#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpRequestHandler.h"
#include "HttpServer/HttpResponse.h"
#include "HttpServer/HttpServerTask.h"
#include "Logger/Logger.h"
#include "Dispatcher/JobAnalyser.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer() :
	IoTask(INVALID_SOCKET, INVALID_SOCKET), trace(0)
{
	notFoundHandler = 0;
}

HttpServer::~HttpServer()
{
	for (map<string, HttpRequestHandler*>::iterator i = handlers.begin(); i != handlers.end(); ++i) {
		HttpRequestHandler* requestHandler = i->second;
		delete requestHandler;
	}

	if (trace != 0) {
		trace->close();
		delete trace;
	}

	if (notFoundHandler) {
		delete notFoundHandler;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// trace
// /////////////////////////////////////////////////////////////////////////////

void HttpServer::setTraceFile(const string& name)
{
	if (trace != 0) {
		trace->close();
		delete trace;
		trace = 0;
	}

	trace = new FileUtils::paloofstream(name.c_str());

	if (!*trace) {
		Logger::error << "cannot open trace file '" << name << "'" << endl;
		delete trace;
		trace = 0;
	} else {
		Logger::info << "using trace file '" << name << "'" << endl;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// handlers
// /////////////////////////////////////////////////////////////////////////////

void HttpServer::addHandler(const string& path, HttpRequestHandler* handler)
{
	map<string, HttpRequestHandler*>::iterator i = handlers.find(path);

	if (i != handlers.end()) {
		delete i->second;
	}

	handlers[path] = handler;
}

void HttpServer::addNotFoundHandler(HttpRequestHandler* handler)
{
	if (notFoundHandler) {
		delete notFoundHandler;
	}

	notFoundHandler = handler;
}

// //////////////////
///////////////////////////////////////////////////////////
// task methods
// /////////////////////////////////////////////////////////////////////////////

Task* HttpServer::handleConnected(socket_t fd)
{
	return new HttpServerTask(fd, this, analyser);
}

void HttpServer::handleShutdown()
{
	Logger::info << "beginning shutdown of listener on " << readSocket << endl;
#if defined(_MSC_VER)
	shutdown(readSocket, SD_BOTH);
#else
	shutdown(readSocket, SHUT_RDWR);
#endif
	closesocket(readSocket);
	readSocket = INVALID_SOCKET;
}

// /////////////////////////////////////////////////////////////////////////////
// request handling
// /////////////////////////////////////////////////////////////////////////////

HttpRequest* HttpServer::createHttpRequest(const string& path)
{
	map<string, HttpRequestHandler*>::iterator iter;
	string tmpPath(path);

	//AbsoluteURI => RelativeURI
	if (!tmpPath.find("http")) {
		size_t pos = tmpPath.find("://");
		if (pos != string::npos) {
			pos += 3;
			pos = tmpPath.find("/", pos);
			if (pos != string::npos) {
				tmpPath = tmpPath.substr(pos);
			} else {
				tmpPath = "/";
			}
		}
	}
	// delete last '/' from request
	if (!tmpPath.empty() && *(tmpPath.rbegin()) == '/') {
		tmpPath = tmpPath.substr(0, tmpPath.find_last_not_of("/") + 1);

		if (tmpPath.empty()) {
			tmpPath = "/";
		}

	}
	iter = handlers.find(tmpPath);

	if (iter != handlers.end()) {
		HttpRequestHandler* handler = iter->second;

		return handler->createHttpRequest(tmpPath);
	} else {
		Logger::warning << "no http request handler for '" << path << "' found" << endl;

		return new HttpRequest(path, 0);
	}
}

void HttpServer::handleRequest(HttpServerTask* task, HttpRequest* request)
{
	HttpJobRequest * jobRequest = 0;

	try {
		HttpRequestHandler* handler = request->getHttpRequestHandler();

		if (handler != 0) {
			jobRequest = handler->handleHttpRequest(request, task);
		} else {
			if (notFoundHandler != 0) {
				jobRequest = notFoundHandler->handleHttpRequest(request, task);
			} else {
				jobRequest = new DirectHttpResponse(request->getRequestPath(), new HttpResponse(HttpResponse::NOT_IMPLEMENTED));
			}
		}
	} catch (const ErrorException& e) {
		jobRequest = handleException(e, request);
	}

	task->handleJobRequest(jobRequest);
}

HttpJobRequest *HttpServer::handleException(const ErrorException& e, HttpRequest* request)
{
	HttpResponse* response = new HttpResponse(HttpResponse::BAD);

	StringBuffer& body = response->getBody();

	body.appendCsvInteger((int32_t)e.getErrorType());
	body.appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(e.getErrorType())));
	body.appendCsvString(StringUtils::escapeString(e.getMessage()));
	body.appendEol();

	Logger::warning << "error code: " << (int32_t)e.getErrorType() << " description: " << ErrorException::getDescriptionErrorType(e.getErrorType()) << " message: " << e.getMessage() << endl;

	return new DirectHttpResponse(request->getRequestPath(), response);
}

HttpServerTask* HttpServer::CreateConnectionTask()
{
	socket_t socket = waitConnection();
	return dynamic_cast<HttpServerTask*>(handleConnected(socket));
}
}
