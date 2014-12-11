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
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Thread/ThreadPool.h"

#if defined(WIN32)
#include "DumpHandler/DumpHandler.h"
#include <Windows.h>
#endif

namespace palo {

ThreadPoolJob::~ThreadPoolJob()
{
}

ThreadPool::ThreadPool() : processorCount(boost::thread::hardware_concurrency()), initSize(processorCount * 2 < 16 ? 16 : processorCount * 2), freeThreads(0), threads(0), hpFreeThreads(0), hpThreads(0), stop(false)
{
	for (size_t i = 0; i < initSize; ++i) {
		++threads;
		boost::thread th(ThreadStarter(*this, false));
	}
}

void ThreadPool::destroy()
{
	{
		boost::unique_lock<boost::mutex> lock(m);
		stop = true;
	}
	hpWakeup.release(hpThreads);
	wakeup.release(threads);
	stopped.wait(threads + hpThreads);
}

void ThreadPool::addJob(PThreadPoolJob job, unsigned int priority)
{
	bool useHP = false;
	{
		boost::unique_lock<boost::mutex> lock(m);
		switch (priority) {
		case 2:
			hptasks.push_front(job);
			break;
		case 1:
			hptasks.push_back(job);
			break;
		default:
			tasks.push(job);
		}
		if (priority && !freeThreads) {
			useHP = true;
			if (!hpFreeThreads) {
				++hpThreads;
				boost::thread th(ThreadStarter(*this, true));
			}
		}
		++(*job->tg);
	}
	if (useHP) {
		hpWakeup.release();
	} else {
		wakeup.release();
	}
	boost::thread::yield();
}

void ThreadPool::join(ThreadGroup &tg)
{
	finished.set(0);
	for (;;) {
		{
			boost::unique_lock<boost::mutex> lock(m);
			if (!*tg) {
				break;
			}
		}
		finished.wait();
	}
}

ThreadPool::ThreadGroup ThreadPool::createThreadGroup()
{
	boost::unique_lock<boost::mutex> lock(m);
	threadGroups.push_back(0);
	return --threadGroups.end();
}

void ThreadPool::destroyThreadGroup(ThreadGroup &tg)
{
	boost::unique_lock<boost::mutex> lock(m);
	threadGroups.erase(tg);
}

void ThreadPool::operator()(bool hpOnly)
{
#if defined(WIN32)
	DumpHandler::register_handler("");
	srand((int)time(NULL) + (int)GetCurrentThreadId());
#else
	srand((int)time(NULL) + (int)pthread_self());
#endif

	SemaphoreReleaser st(stopped);
	PThreadPoolJob job;
	{
		boost::unique_lock<boost::mutex> lock(m);
		if (hpOnly) {
			++hpFreeThreads;
		} else {
			++freeThreads;
		}
	}
	for (;;) {
		SemaphoreReleaser fin(finished);
		if (hpOnly) {
			hpWakeup.wait();
		} else {
			wakeup.wait();
		}
		{
			boost::unique_lock<boost::mutex> lock(m);
			if (stop) {
				break;
			}
			if (hpOnly) {
				--hpFreeThreads;
			} else {
				--freeThreads;
			}
			if (hptasks.empty()) {
				job = tasks.front();
				tasks.pop();
			} else {
				job = hptasks.front();
				hptasks.pop_front();
			}
		}
		(*job)();
		{
			boost::unique_lock<boost::mutex> lock(m);
			if (hpOnly) {
				++hpFreeThreads;
			} else {
				++freeThreads;
			}
			--(*job->tg);
		}
		job.reset();
	}
}

}
