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

#include "PaloDocumentation/StatisticsDocumentation.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "Collections/StringUtils.h"

namespace palo {
StatisticsDocumentation::StatisticsDocumentation(const Statistics& statistics)
{
	fullTimings = statistics.getFullTimings();
	combinedTimings = statistics.getCombinedTimings();

	generateTimings(fullPath, fullTimings, "full_path");
	generateTimings(combinedPath, combinedTimings, "combined");
}

void StatisticsDocumentation::generateTimings(vector<string>& path, map<string, Statistics::Timing> timings, const string& postfix)
{
	for (map<string, Statistics::Timing>::const_iterator i = timings.begin(); i != timings.end(); ++i) {
		path.push_back(i->first);
	}

	sort(path.begin(), path.end());

	for (vector<string>::const_iterator i = path.begin(); i != path.end(); ++i) {
		Statistics::Timing timing = timings[*i];

		values["@calls_" + postfix].push_back(StringUtils::convertToString(timing.calls));

		values["@total_wall_time_" + postfix].push_back(StringUtils::convertToString(timing.wallTime / 1000));
		values["@total_user_time_" + postfix].push_back(StringUtils::convertToString(timing.userTime / 1000));
		values["@total_sys_time_" + postfix].push_back(StringUtils::convertToString(timing.sysTime / 1000));

		if (timing.calls != 0) {
			values["@average_wall_time_" + postfix].push_back(StringUtils::convertToString(timing.wallTime / 1000 / timing.calls));
			values["@average_user_time_" + postfix].push_back(StringUtils::convertToString(timing.userTime / 1000 / timing.calls));
			values["@average_sys_time_" + postfix].push_back(StringUtils::convertToString(timing.sysTime / 1000 / timing.calls));
		} else {
			values["@average_wall_time_" + postfix].push_back("-");
			values["@average_user_time_" + postfix].push_back("-");
			values["@average_sys_time_" + postfix].push_back("-");
		}
	}
}

bool StatisticsDocumentation::hasDocumentationEntry(const string& name)
{
	if (name == "@full_path") {
		return true;
	} else if (name == "@combined") {
		return true;
	} else {
		map<string, vector<string> >::const_iterator i = values.find(name);

		return (i == values.end()) ? false : true;
	}

	return false;
}

const vector<string>& StatisticsDocumentation::getDocumentationEntries(const string& name)
{
	static vector<string> error;

	if (name == "@full_path") {
		return fullPath;
	} else if (name == "@combined") {
		return combinedPath;
	} else {
		map<string, vector<string> >::const_iterator i = values.find(name);

		return (i == values.end()) ? error : i->second;
	}
}

const string& StatisticsDocumentation::getDocumentationEntry(const string& name, size_t index)
{
	static string error = "";
	const vector<string> * entries;

	if (name == "@full_path") {
		entries = &fullPath;
	} else if (name == "@combined") {
		entries = &combinedPath;
	} else {
		map<string, vector<string> >::const_iterator i = values.find(name);

		if (i == values.end()) {
			return error;
		}

		entries = &(i->second);
	}

	if (index < 0 || entries->size() <= index) {
		return error;
	}

	return (*entries)[index];
}
}
