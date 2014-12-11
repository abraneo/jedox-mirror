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

#ifndef PALO_CONFIG_H
#define PALO_CONFIG_H

#include <map>
#include <string>

#include <libpalo_ng/Util/StringUtils.h>

namespace jedox {

class CaseInsensitiv;

namespace palo {

class Config {
public:
	static Config& getInstance()
	{
		static Config instance;
		return instance;
	}

	const std::string& getOption(const std::string& key);

private:
	Config();
	Config(const Config&);
	Config& operator=(const Config&);

	std::map<std::string, std::string, util::UTF8Comparer> m_ConfigMap;

};

} /* namespace palo */
} /* namespace jedox */
#endif							 // PALO_CONFIG_H
