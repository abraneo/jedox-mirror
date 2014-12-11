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
 * \author Radu Ialovoi
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#if defined(_MSC_VER)
	#define FD_SETSIZE 1024
#endif

#include "MTPaloInterface.h"

#include "HttpServer/HttpServer.h"
#include "HttpServer/HttpServerTask.h"
#include "Logger/Logger.h"
#include "HttpServer/HttpRequestHandler.h"
#include "PaloHttpServer/PaloHttpServer.h"
#include "Exceptions/LicenseException.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace palo {

void MTPaloHttpInterface::ConnectionHandler::operator()()
{
	boost::posix_time::time_duration t;
	boost::posix_time::time_duration timeoutHigh(boost::posix_time::milliseconds(500));
	boost::posix_time::time_duration timeoutLow(boost::posix_time::milliseconds(5000));

	fd_set sockets;
	int ret = 0;
	do {
		boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
		task->Tranzact();
		t += boost::posix_time::microsec_clock::universal_time() - start;

		socket_t crt = task->getReadSocket();
		if (crt == INVALID_SOCKET) {
			break;
		}

		bool highUsage = tp->getUsage() > 50;

		if (t > (highUsage ? timeoutHigh : timeoutLow)) {
			break;
		}

		FD_ZERO(&sockets);
		FD_SET(crt, &sockets);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = highUsage ? 5000 : 500000;
		ret = select((int)crt + 1, &sockets, NULL, NULL, &tv);
	} while (ret > 0);
	WriteLocker connection_locker(&intf->connections_lock);
	task->notWorking();
}

void MTPaloHttpInterface::handleShutdown()
{
	bool wasNew;
	PServer server = Context::getContext(&wasNew)->getServer();
	server->ShutdownLoginWorker();
	server->ShutdownDimensionWorker();
	Context::reset(wasNew);
	PaloHttpInterface::handleShutdown();
}

void MTPaloHttpInterface::run()
{
	PThreadPool tp = Context::getContext()->getServer()->getThreadPool();
	ThreadPool::ThreadGroup tg = Context::getContext()->getServer()->getThreadPool()->createThreadGroup(true);
	Context::reset();
	fd_set sockets;
	runnable = true;
	struct timeval tv;

	for (;;) {
		socket_t max = 0;
		FD_ZERO(&sockets);
		for (vector<HttpServer*>::iterator i = servers.begin(); i != servers.end(); ++i) {
			socket_t crt = (*i)->getReadSocket();
			if (crt != INVALID_SOCKET) {
				FD_SET(crt, &sockets);
				if (max < crt) {
					max = crt;
				}
			}
		}
		{
			WriteLocker lock(&connections_lock);
			for (std::vector<PHttpServerTask>::iterator i = active_connections.begin(); i != active_connections.end();) {
				socket_t crt = (*i)->getReadSocket();
				if (crt == INVALID_SOCKET) {
					i = active_connections.erase(i);
				} else {
					FD_SET(crt, &sockets);
					if (max < crt) {
						max = crt;
					}
					++i;
				}
			}
		}
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		int ret = select((int)max + 1, &sockets, NULL, NULL, &tv);
		if (!runnable) {
			break;
		}
		if (0 < ret) {
			for (server_list_type::iterator pServer = servers.begin(); pServer != servers.end(); ++pServer) {
				socket_t crt = (*pServer)->getReadSocket();
				if (crt != INVALID_SOCKET) {
					if (FD_ISSET(crt, &sockets)) {
						PHttpServerTask crt((*pServer)->CreateConnectionTask());
						{
							WriteLocker connection_locker(&connections_lock);
							active_connections.push_back(crt);
						}
					}
				}
			}
			{
				WriteLocker lock(&connections_lock);
				for (std::vector<PHttpServerTask>::iterator i = active_connections.begin(); i != active_connections.end(); ++i) {
					socket_t crt = (*i)->getReadSocket();
					if (crt != INVALID_SOCKET) {
						if (FD_ISSET(crt, &sockets) && !(*i)->isWorking()) {
							(*i)->working();
							tp->addJob(PThreadPoolJob(new ConnectionHandler(*i, this, tg, tp)));
						}
					}
				}
			}
		}
	}

	PaloSession::printActiveJobs("found running \"", "\" job");
	{
		WriteLocker lock(&connections_lock);
		for (std::vector<PHttpServerTask>::iterator i = active_connections.begin(); i != active_connections.end(); ++i) {
			(*i)->handleShutdown();
		}
	}
	tp->join(tg);
	tp->destroy();
	Logger::info << "all jobs finished, proceeding with shutdown" << endl;
}

void MTPaloHttpInterface::EnablePaloInterface(PaloHttpServer *paloHttpServer)
{
	PaloHttpInterface::EnablePaloInterface(paloHttpServer);
}

}
