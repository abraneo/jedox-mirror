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
 * \author Radu Ialovoi
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H 1

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <queue>

namespace palo {

class Semaphore {
public:
	Semaphore() : c(0) {}
	void wait(size_t count = 1) {
		boost::unique_lock<boost::mutex> lock(m);
		while (c < count) {
			v.wait(lock);
		}
		c -= count;
	}

	void release(size_t count = 1) {
		boost::unique_lock<boost::mutex> lock(m);
		c += count;
		v.notify_all();
	}

	void set(size_t count) {
		boost::unique_lock<boost::mutex> lock(m);
		c = count;
	}
private:
	boost::condition_variable v;
	boost::mutex m;
	size_t c;
};

class SemaphoreReleaser {
public:
	SemaphoreReleaser(Semaphore &s) :s(s) {}
	~SemaphoreReleaser() {s.release();}
private:
	Semaphore &s;
};

class ThreadPoolJob;
typedef boost::shared_ptr<ThreadPoolJob> PThreadPoolJob;

class ThreadPool {
	friend class ThreadPoolJob;
	friend class ThreadStarter;
public:
	typedef std::list<size_t>::iterator ThreadGroup;

	ThreadPool();
	~ThreadPool() {}
	void addJob(PThreadPoolJob job, unsigned int priority = 0);
	void join(ThreadGroup &tg);
	ThreadGroup createThreadGroup();
	void destroyThreadGroup(ThreadGroup &tg);
	bool hasFreeCore(bool countThis) {return freeThreads - (countThis ? 1 : 0) >= threads - processorCount;}
	size_t getCoreCount() {return processorCount;}
	void destroy();
	int getUsage() {return (int)((threads - freeThreads) * 100 / threads);}

private:
	void operator()(bool hpOnly);
	ThreadPool(const ThreadPool &);
	std::queue<PThreadPoolJob> tasks;
	std::deque<PThreadPoolJob> hptasks;
	boost::mutex m;
	size_t processorCount;
	size_t initSize;
	Semaphore stopped;
	Semaphore wakeup;
	Semaphore finished;
	size_t freeThreads;
	size_t threads;
	size_t hpFreeThreads;
	size_t hpThreads;
	Semaphore hpWakeup;
	bool stop;
	std::list<size_t> threadGroups;
};
typedef boost::shared_ptr<ThreadPool> PThreadPool;

class ThreadPoolJob {
	friend class ThreadPool;
public:
	ThreadPoolJob(ThreadPool::ThreadGroup &tg) : tg(tg) {}
	virtual ~ThreadPoolJob();
	ThreadPool::ThreadGroup &getThreadGroup() {return tg;}
private:
	virtual void operator()() = 0;
	ThreadPool::ThreadGroup &tg;
};

class ThreadStarter
{
public:
	ThreadStarter(ThreadPool &tp, bool hpOnly) : tp(tp), hpOnly(hpOnly) {}
	void operator()() {tp(hpOnly);}
private:
	ThreadPool &tp;
	bool hpOnly;
};

}

#endif
