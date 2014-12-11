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

#ifndef INPUT_OUTPUT_STATISTICS_H
#define INPUT_OUTPUT_STATISTICS_H 1

#include "palo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief stores profiling information
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Statistics {
public:
	struct Timing {
		int64_t wallTime;
		int64_t userTime;
		int64_t sysTime;

		uint32_t calls;
	};

	struct Path {
		bool isActive;
		string activeName;
		string activePath;

		set<string> toplevel;
		vector<string> path;
	};

#ifdef ENABLE_TIME_PROFILER
	class Timer {
	public:
		Timer(const string&);
		Timer(const string&, const string&);
		~Timer();

	private:
		Timer(const Timer&);
		Timer& operator= (const Timer&);

	private:
		int64_t subtract(const timeval&, const timeval&);

	private:
		const string name;

#ifdef HAVE_GETRUSAGE
		rusage usage;
#endif

		timeval start;
	};
#else
	class Timer {
	public:
		Timer(const string&) {
		}

		Timer(const string&, const string&) {
		}
	};
#endif

public:
	static Statistics statistics;

public:
	Statistics();
	~Statistics();

public:
	void begin(Path&, const string& name);
	void end(Path&, const string& name);

	void setUsage(int64_t);
	void setWallTime(uint64_t);
	void setUserTime(uint64_t);
	void setSysTime(uint64_t);

	void clearTimings();

public:
	const map<string, Timing>& getFullTimings() const {
		return fullTimings;
	}

	const map<string, Timing>& getCombinedTimings() const {
		return combinedTimings;
	}

	const map<string, int64_t>& getFullUsage() const {
		return fullUsage;
	}

	const map<string, int64_t>& getCombinedUsage() const {
		return combinedUsage;
	}

private:
	void computeActivePath(Path&);

private:
	Path timings;
	map<string, Timing> fullTimings;
	map<string, Timing> combinedTimings;

	Path usage;
	map<string, int64_t> fullUsage;
	map<string, int64_t> combinedUsage;
};

}

#endif
