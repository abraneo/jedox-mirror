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
 * 
 *
 */

#ifndef PROGRAMS_MT_PALO_HTTP_INTERFACE_H
#define PROGRAMS_MT_PALO_HTTP_INTERFACE_H 1

#include "palo.h"

#include "PaloHttpInterface.h"
#include "PaloOptions.h"
#include "Thread/ThreadPool.h"
#include "Thread/Mutex.h"
#include "Olap/Server.h"

namespace palo {

class SERVER_CLASS MTPaloHttpInterface : public PaloHttpInterface {
private:
	typedef boost::shared_ptr<HttpServerTask> PHttpServerTask;
	class ConnectionHandler : public ThreadPoolJob {
	private:
		PHttpServerTask task;
		MTPaloHttpInterface *intf;
		PThreadPool tp;
	public:
		ConnectionHandler(PHttpServerTask task, MTPaloHttpInterface *intf, ThreadPool::ThreadGroup &tg, PThreadPool tp) : ThreadPoolJob(tg), task(task), intf(intf), tp(tp) {}
	private:
		virtual void operator()();
	};

	std::vector<PHttpServerTask> active_connections;
	Mutex connections_lock;
public:
	MTPaloHttpInterface(PaloOptions* options, JobAnalyser* analyser) : PaloHttpInterface(options, analyser) {}
	virtual ~MTPaloHttpInterface() {}

	void handleShutdown();
	void run();
private:
	virtual void EnablePaloInterface(PaloHttpServer *paloHttpServer);
};

}

#endif
