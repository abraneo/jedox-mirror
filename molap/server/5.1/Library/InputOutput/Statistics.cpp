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

#include "InputOutput/Statistics.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <iostream>

#include "Logger/Logger.h"

namespace palo {
Statistics Statistics::statistics;

#ifdef ENABLE_TIME_PROFILER

Statistics::Timer::Timer(const string& path)
	: name(path)
{
	gettimeofday(&start, 0);

#ifdef HAVE_GETRUSAGE
	getrusage(RUSAGE_SELF, &usage);
#endif

	statistics.begin(statistics.timings, name);

	statistics.fullTimings[statistics.timings.activePath].calls++;
	statistics.combinedTimings[statistics.timings.activeName].calls++;
}

Statistics::Timer::Timer(const string& path, const string& suffix)
	: name(path + "[" + suffix + "]")
{
	gettimeofday(&start, 0);

#ifdef HAVE_GETRUSAGE
	getrusage(RUSAGE_SELF, &usage);
#endif

	statistics.begin(statistics.timings, name);

	statistics.fullTimings[statistics.timings.activePath].calls++;
	statistics.combinedTimings[statistics.timings.activeName].calls++;
}

Statistics::Timer::~Timer()
{
	timeval stop;

	gettimeofday(&stop, 0);

	statistics.setWallTime(subtract(stop, start));

#ifdef HAVE_GETRUSAGE
	rusage used;

	getrusage(RUSAGE_SELF, &used);
	statistics.setUserTime(subtract(used.ru_utime, usage.ru_utime));
	statistics.setSysTime(subtract(used.ru_stime, usage.ru_stime));
#endif

	statistics.end(statistics.timings, name);
}

int64_t Statistics::Timer::subtract(const timeval& l, const timeval& r)
{
	time_t sec = l.tv_sec - r.tv_sec;
	suseconds_t msc = l.tv_usec - r.tv_usec;

	while (msc < 0) {
		msc += 1000000;
		sec -= 1;
	}

	return (sec * 1000000LL) + msc;
}

#endif

Statistics::Statistics()
{
	timings.isActive = false;
	usage.isActive = false;
}

Statistics::~Statistics()
{
}

void Statistics::computeActivePath(Path& path)
{
	path.activePath = "";

	for (vector<string>::const_iterator i = path.path.begin(); i != path.path.end(); ++i) {
		path.activePath += *i + ".";
	}

	path.activePath += path.activeName;
}

void Statistics::begin(Path& path, const string& name)
{
	if (path.isActive) {
		path.path.push_back(path.activeName);
	} else {
		path.isActive = true;

		path.toplevel.insert(name);
	}

	path.activeName = name;

	computeActivePath(path);
}

void Statistics::end(Path& path, const string& name)
{
	if (!path.isActive) {
		Logger::error << "missing begin timer call for '" << name << "'" << endl;
	} else if (path.activeName != name) {
		Logger::error << "mismatch in timer calls, expecting '" << path.activeName << "', got '" << name << "'" << endl;
	} else {
		if (path.path.empty()) {
			path.isActive = false;
			path.activePath = "";
		} else {
			path.activeName = path.path.back();
			path.path.pop_back();
			computeActivePath(path);
		}
	}
}

void Statistics::setUsage(int64_t bytes)
{
	fullUsage[usage.activePath] += bytes;
	combinedUsage[usage.activeName] += bytes;
}

void Statistics::setWallTime(uint64_t time)
{
	fullTimings[timings.activePath].wallTime += time;
	combinedTimings[timings.activeName].wallTime += time;
}

void Statistics::setUserTime(uint64_t time)
{
	fullTimings[timings.activePath].userTime += time;
	combinedTimings[timings.activeName].userTime += time;
}

void Statistics::setSysTime(uint64_t time)
{
	fullTimings[timings.activePath].sysTime += time;
	combinedTimings[timings.activeName].sysTime += time;
}

void Statistics::clearTimings()
{
	fullTimings.clear();
	combinedTimings.clear();
}
}
