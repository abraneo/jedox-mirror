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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <iostream>
#include <time.h>

#include "PHPPaloLogger.h"

#define _INC_MATH
#include <zend.h>
#include "fix_zend.h"

#define MAX_TIME_SIZE 28

using namespace Palo::PHPPalo;
using namespace Palo::SpreadsheetFuncs;

bool PHPPaloLogger::dumb_idea = false;

bool PHPPaloLogger::log_to_stderr = false;

const std::string PHPPaloLogger::modul_notice = "[PHPPALO notice] ";
const std::string PHPPaloLogger::modul_warning = "[PHPPALO warning] ";
const std::string PHPPaloLogger::modul_error = "[PHPPALO error] ";

void PHPPaloLogger::log(Type t, std::string msg) throw ()
{
	std::string s;
	try {
		s = ctx.getConversions().to(msg);
	} catch (...) {
		s = "Error converting charset of error message!";
	}

	switch ((dumb_idea && (t == AbstractLogger::LOG_WARNING)) ? AbstractLogger::LOG_ERROR : t) {
	case AbstractLogger::LOG_ERROR:
		ProcessLog( E_USER_ERROR, "%s", s);
		break;
	case AbstractLogger::LOG_WARNING:
		ProcessLog( E_USER_WARNING, "%s", s);
		break;
	case AbstractLogger::LOG_INFO:
		ProcessLog( E_USER_NOTICE, "%s", s);
		break;
	default:
		ProcessLog( E_USER_ERROR, "(Unknown log message type requested): %s", msg);
	}
}

void PHPPaloLogger::ProcessLog(int type, const char *format, std::string& msg)
{
	zend_error(type, format, msg.c_str());

	if (log_to_stderr) {
		char buf[MAX_TIME_SIZE];
		time_t t = time(NULL);
		strftime(buf, MAX_TIME_SIZE, "[%a %b %d %H:%M:%S %Y] ", localtime(&t));

		std::cerr << buf;

		switch (type) {
		case E_USER_NOTICE:
			std::cerr << modul_notice;
			break;

		case E_USER_WARNING:
			std::cerr << modul_warning;
			break;

		case E_USER_ERROR:
		default:
			std::cerr << modul_error;
			break;
		}
		std::cerr << msg << std::endl;
	}
}

void PHPPaloLogger::InternalPaloWeb(long value)
{
	dumb_idea = (value > 0);
}

void PHPPaloLogger::ActivateLogStderr(long value)
{
	log_to_stderr = (value > 0);
}
