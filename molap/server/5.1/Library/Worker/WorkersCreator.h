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
 * \author Vali Nitu, Yalos Solutions, Bucharest, Romania
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PROGRAMS_WORKER_CREATOR_H
#define PROGRAMS_WORKER_CREATOR_H 1

#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>

#include "Olap/Server.h"
#include "Olap/PaloSession.h"

typedef void(*worker_callback)();

namespace palo {
class WorkersCreator {
private:
	worker_callback sink;
public:
	WorkersCreator(worker_callback sink) :
		sink(sink) {
		boost::thread(*this);
#ifdef ENABLE_GOOGLE_CPU_PROFILER
            ProfilerRegisterThread();
#endif
	}

	void operator()();

	static void startAll(bool startup);
	static void quitAll(PServer server, vector<PCube> &vCubes, bool restart);
	static void prepareNewStart(PServer server);
};

}
;

#endif
