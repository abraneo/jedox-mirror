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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * 
 *
 */

#ifndef GPU_IDLE_WORKER
#define GPU_IDLE_WORKER 1

#include <boost/thread.hpp>

#include "Logger/Logger.h"
#include "Programs/PaloHttpInterface.h"

namespace palo {

//////////////////////////////////////////////////////////////////////////
// @brief class Gpu Idle Worker to start optimization of gpu storages
//////////////////////////////////////////////////////////////////////////

class SERVER_CLASS GpuIdleWorker {
private:
	class OptimizeWorker {
	public:
		OptimizeWorker(PaloHttpInterface* iface, bool* isActive) :
				isActive(isActive), iface(iface)
		{
		}

		void operator()()
		{
			while (*isActive) {
				// optimize gpu storages
				boost::this_thread::sleep(boost::posix_time::seconds(5));
				if (needGpuOptimization()) {
					if (doOptimization()){
						Logger::trace << "Optimize done" << endl;
						boost::this_thread::sleep(boost::posix_time::seconds(10));
					}
				} else {
					boost::this_thread::sleep(boost::posix_time::seconds(10));
				}

			}
		}
	private:

		bool doOptimization()
		{
			return iface->optimizeGpuEngine();
		}

		bool needGpuOptimization()
		{
			return iface->needGpuOptimization();
		}

	private:
		bool* isActive;
		PaloHttpInterface* iface;
	};

public:

	GpuIdleWorker(PaloHttpInterface* iface) :
			isActive(true)
	{
		optimizeWorker = new boost::thread(OptimizeWorker(iface, &isActive));
#ifdef ENABLE_GOOGLE_CPU_PROFILER
            ProfilerRegisterThread();
#endif
		Logger::debug << "GpuIdleWorker started with thread id: " << getThreadId() << endl;
	}

	virtual ~GpuIdleWorker()
	{
		stopWorker();
	}

	boost::thread::id getThreadId()
	{
		return optimizeWorker->get_id();
	}

	void stopWorker()
	{
		if (optimizeWorker != NULL) {
			isActive = false;
			optimizeWorker->join();
			Logger::debug << "GpuIdleWorker stopped" << endl;
			optimizeWorker = NULL;
		}
	}
private:

	bool isActive;
	boost::thread* optimizeWorker;
};

}

#endif // GPU_IDLE_WORKER
