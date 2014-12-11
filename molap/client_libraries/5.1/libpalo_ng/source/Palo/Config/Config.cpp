/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include "Config.h"

namespace jedox {
namespace palo {

Config::Config()
{
	std::fstream stream("config_ng.ini", std::ios::in);
	if (stream.fail() == false) {
		std::string line;
		while (stream.eof() == false) {
			stream >> line;
			std::vector<std::string> value;
			boost::algorithm::split(value, line, boost::algorithm::is_any_of("="));
			if (value.size() == 2) {
				boost::algorithm::trim(value[0]);
				boost::algorithm::trim(value[1]);
				m_ConfigMap[value[0]] = value[1];
			}
		}
		stream.close();
	}
}

const std::string& Config::getOption(const std::string& key)
{
	static std::string empty;
	std::map<std::string, std::string, util::UTF8Comparer>::const_iterator it(m_ConfigMap.find(key));
	if (it != m_ConfigMap.end()) {
		return it->second;
	}
	return empty;
}

} /* namespace palo */
} /* namespace jedox */
