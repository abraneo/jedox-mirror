/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#include "HttpServer/HttpServerTask.h"

#include <iostream>

#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpResponse.h"
#include "HttpServer/HttpServer.h"
#include "HttpServer/HttpJobRequest.h"
#include "PaloHttpServer/PaloSSORequestHandler.h"
#include "Logger/Logger.h"
#include "Dispatcher/Job.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

HttpServerTask::HttpServerTask(socket_t fd, HttpServer* server, JobAnalyser* analyzer) :
	IoTask(fd, fd), ReadWriteTask(fd, fd), analyzer(analyzer), server(server), readPosition(0), bodyPosition(0), bodyLength(0), work(false), bShutdown(false)
{
	httpRequestPending = false;
	httpRequest = 0;
	readRequestBody = false;
	httpJobRequest = 0;
}

HttpServerTask::~HttpServerTask()
{
	for (deque<StringBuffer*>::iterator i = writeBuffers.begin(); i != writeBuffers.end(); ++i) {
		StringBuffer * buffer = *i;
		delete buffer;
	}

	if (httpRequest != 0) {
		delete httpRequest;
	}

	writeBuffers.clear();
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

void HttpServerTask::handleJobRequest(HttpJobRequest* jobRequest)
{
	httpJobRequest = jobRequest;

	if (!jobRequest->isDone()) {
		// convert request into job
		Job* job = analyzer->analyse(jobRequest);

		job->setTask(this);
		// the algorithm behind analyzer->analyze ensures that job != 0;
		if (job->initialize()) {
			job->work();
		}
		jobRequest->handleDone(job);
		bShutdown = job->getShutdown();

		job->cleanup();
	}
	handleDone();
}

// /////////////////////////////////////////////////////////////////////////////
// IoTask
// /////////////////////////////////////////////////////////////////////////////

bool HttpServerTask::canHandleRead()
{
	return !httpRequest || readRequestBody;
}

bool HttpServerTask::handleRead()
{
	bool res = fillReadBuffer();

	if (!res) {
		return false;
	}

	return processRead();
}

void HttpServerTask::invalidateReadSocket()
{
	//PaloSSORequestHandler::removeAuthenticationInfo(readSocket); replaced by timeout
	readSocket = INVALID_SOCKET;
}

void HttpServerTask::handleShutdown()
{
#if defined(_MSC_VER)
	shutdown(readSocket, SD_BOTH);
#else
	shutdown(readSocket, SHUT_RDWR);
#endif
	closesocket(readSocket);
	invalidateReadSocket();
}

void HttpServerTask::handleHangup()
{
	Logger::trace << "got hangup on socket " << readSocket << endl;
#if defined(_MSC_VER)
	shutdown(readSocket, SD_BOTH);
#else
	shutdown(readSocket, SHUT_RDWR);
#endif
	closesocket(readSocket);
	invalidateReadSocket();
}

// /////////////////////////////////////////////////////////////////////////////
// JobTask
// /////////////////////////////////////////////////////////////////////////////

bool HttpServerTask::isJobDone()
{
	if (httpJobRequest) {
		return httpJobRequest->isDone();
	}

	return false;
}

void HttpServerTask::handleDone()
{

	// get response from job request
	HttpResponse* response = httpJobRequest->getResponse();

	if (response == 0) {
		response = new HttpResponse(HttpResponse::NO_RESPONSE);
	}

	// trace
#ifdef ENABLE_TRACE_OPTION
	{
		string requestString = "";

		if (httpRequest != 0) {
			requestString = httpRequest->getHeaderString();

			const string bs = httpRequest->getBodyString();

			if (!bs.empty()) {
				if (requestString.find("?") == string::npos) {
					requestString += "?";
				}

				requestString += bs;
			}
		}

		server->traceRequest(requestString + response->getBody().c_str());
	}
#endif

	// add response to output buffer
	addResponse(response);

	// we can now free the response
	delete response;

	// and the jobe request
	delete httpJobRequest;
	httpJobRequest = 0;

	// and the http request
	if (httpRequest != 0) {
		httpRequest->releaseBuffers();
		delete httpRequest;
		httpRequest = 0;
	}

	httpRequestPending = false;

	// remove body from read buffer and reset read position
	readBuffer.erase_front(bodyPosition + bodyLength);
	readPosition = 0;
	bodyPosition = 0;
	bodyLength = 0;
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

string HttpServerTask::extractRequestPath()
{
	const char* begin = readBuffer.c_str();
	const char* end = begin + readPosition;

	for (; begin < end && *begin != ' '; ++begin) {
	}

	if (begin == end) {
		return "";
	}

	++begin;

	const char * reqe = begin;

	for (; reqe < end && *reqe != ' ' && *reqe != '?' && *reqe != '\n' && *reqe != '\r'; ++reqe) {
	}

	return string(begin, reqe);
}

bool HttpServerTask::processRead()
{
	if (httpRequestPending) {
		return true;
	}

	bool handleRequest = false;

	if (!readRequestBody) {
		const char * ptr = readBuffer.c_str() + readPosition;
		const char * end = readBuffer.end() - 3;

		for (; ptr < end; ptr++) {
			if (ptr[0] == '\r' && ptr[1] == '\n' && ptr[2] == '\r' && ptr[3] == '\n') {
				break;
			}
		}

		if (ptr < end) {
			readPosition = ptr - readBuffer.c_str() + 4;
			string url = extractRequestPath();
			httpRequest = server->createHttpRequest(url);
			httpRequest->extractHeader(readBuffer.str(), readBuffer.str() + readPosition);
			bodyPosition = readPosition;

			switch (httpRequest->getRequestType()) {
			case HttpRequest::HTTP_REQUEST_GET:
				handleRequest = true;
				break;

			case HttpRequest::HTTP_REQUEST_POST:
				bodyLength = httpRequest->getContentLength();

				if (bodyLength > 0) {
					readRequestBody = true;
				} else {
					handleRequest = true;
				}
				break;

			default:
				Logger::warning << "got corrupted HTTP request" << endl;
				//handleHangup();
				//handleRequest = true;
				return false;
			}

		} else {
			if (readBuffer.c_str() < end) {
				readPosition = end - readBuffer.c_str();
			}
		}
	}

	// readRequestBody might have changed, so cannot use else
	if (readRequestBody) {
		if (readBuffer.length() - bodyPosition < bodyLength) {
			return true;
		}

		// read "bodyLength" from read buffer and add this body to "httpRequest"
		httpRequest->extractBody(readBuffer.str() + bodyPosition, readBuffer.str() + bodyPosition + bodyLength);

		// handle request
		readRequestBody = false;
		handleRequest = true;
	}

	if (handleRequest) {
		httpRequestPending = true;
		server->handleRequest(this, httpRequest);
	}

	return true;
}

void HttpServerTask::addResponse(HttpResponse* response)
{
	StringBuffer * buffer;

	// save header
	buffer = new StringBuffer();
	buffer->replaceText(response->getHeader());
	buffer->appendText(response->getBody());

	writeBuffers.push_back(buffer);

	// clear body
	response->getBody().clear();

	// start output
	fillWriteBuffer();
}

void HttpServerTask::completedWriteBuffer()
{
	fillWriteBuffer();
}

void HttpServerTask::fillWriteBuffer()
{
	if (!hasWriteBuffer() && !writeBuffers.empty()) {
		StringBuffer * buffer = writeBuffers.front();
		writeBuffers.pop_front();

		setWriteBuffer(buffer);
	}
}
}
