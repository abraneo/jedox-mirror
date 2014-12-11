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

#include <ctime>
#include <fstream>

#include <libpalo_ng/Palo/Logging/Logging.h>

#include "../Config/Config.h"

Logging::Logging()
{
	jedox::palo::Config& config = jedox::palo::Config::getInstance();
	const std::string& logfile = config.getOption("logfile");
	if (logfile.length() > 0) {
		boost::scoped_ptr<std::ostream> myptr(new std::ofstream(logfile.c_str()));
		m_Stream.swap(myptr);
	}
}

void Logging::log(const std::string& log)
{
	if (m_Stream.get() == NULL) {
		return;
	}
	time_t myTime;
	struct tm * conv;
	time(&myTime);
	conv = localtime(&myTime);
	(*m_Stream.get()) << "[" << conv->tm_hour << ":" << conv->tm_min << ":" << (conv->tm_sec) << "] " << log << std::endl;
}
