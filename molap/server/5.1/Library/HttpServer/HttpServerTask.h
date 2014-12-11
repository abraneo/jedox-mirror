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

#ifndef HTTP_SERVER_HTTP_SERVER_TASK_H
#define HTTP_SERVER_HTTP_SERVER_TASK_H 1

#include "palo.h"

#include <deque>

#include "Scheduler/ReadWriteTask.h"
#include "Scheduler/JobTask.h"
#include "HttpServer/HttpJobRequest.h"
#include "Dispatcher/JobAnalyser.h"

#if defined (HAVE_SIGNAL_H) || defined (_MSC_VER)
#include <signal.h>
#endif

namespace palo {
class HttpRequest;
class HttpResponse;
class HttpServer;

////////////////////////////////////////////////////////////////////////////////
/// @brief read/write task of a http server
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HttpServerTask : public ReadWriteTask, public JobTask {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new read/write task of a http server
	///
	/// The http server constructs a HttpServerTask for each client
	/// connection. This task is responsible for dealing for the input/output
	/// to/from the client.
	////////////////////////////////////////////////////////////////////////////////

	HttpServerTask(socket_t fd, HttpServer*, JobAnalyser* analyzer);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes a read/write task of a http server
	////////////////////////////////////////////////////////////////////////////////

	~HttpServerTask();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds a job request
	////////////////////////////////////////////////////////////////////////////////

	void handleJobRequest(HttpJobRequest*);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool canHandleRead();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool handleRead();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void handleHangup();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void handleShutdown();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool isJobDone();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	JobRequest* getJobRequest() {
		return httpJobRequest;
	}

	const HttpRequest * getHttpRequest() {
		return httpRequest;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////
	void handleDone();

	bool Tranzact() {
		if (!handleRead()) {
			closesocket(readSocket);
			invalidateReadSocket();
			return false;
		}
		if (!handleWrite()) {
			closesocket(readSocket);
			invalidateReadSocket();
			return false;
		}

		if (bShutdown) {
#if defined(_MSC_VER)
		raise(SIGTERM);
#else
		kill(0, SIGTERM);
#endif
		}

		return true;
	}

	void working() {
		work = true;
	}

	void notWorking() {
		work = false;
	}

	bool isWorking() {
		return work;
	}

private:
	void invalidateReadSocket();

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void completedWriteBuffer();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool processRead();

private:
	void fillWriteBuffer();
	void addResponse(HttpResponse*);
	string extractRequestPath();

private:
	JobAnalyser * analyzer;
	HttpServer * server;
	HttpJobRequest * httpJobRequest;
	deque<StringBuffer*> writeBuffers;
	size_t readPosition;
	size_t bodyPosition;

	bool httpRequestPending;
	HttpRequest * httpRequest;
	bool readRequestBody;
	size_t bodyLength;
	bool work;
	bool bShutdown;
};

}

#endif
