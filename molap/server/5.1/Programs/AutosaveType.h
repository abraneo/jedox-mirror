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
 * \author Radu Ialovoi, Yalos Solutions, Bucharest, Romania
 * 
 *
 */

#ifndef PROGRAMS_AUTOSAVE_TYPE_H
#define PROGRAMS_AUTOSAVE_TYPE_H 1

#include <vector>

#include "Collections/StringUtils.h"

namespace palo {
class autosave_type {
	friend class AutosaveTimer;
protected:
	enum mode_type {
		AS_DISABLE, AS_LOOP, AS_EXACT_TIME
	} mode;

	char hour;
	char minute;

	enum state_type {
		ST_MODE, ST_TIME, ST_VALID, ST_ERROR
	} state;

	void set_mode(char arg) {
		switch (arg) {
		case 'l':
		case 'L':
			mode = AS_LOOP;
			break;
		case 't':
		case 'T':
			mode = AS_EXACT_TIME;
			break;
		default:
			throw ErrorException(ErrorException::ERROR_INVALID_STRING, (string)"invalid autosave parameter '" + arg + "'");
//			mode = AS_DISABLE;
//
//			break;
		}
	}

public:
	autosave_type() :
		mode(AS_DISABLE), hour(0), minute(0), state(ST_MODE) {
	}
	autosave_type(const autosave_type& src) :
		mode(src.mode), hour(src.hour), minute(src.minute), state(src.state) {
	}

	void push(const std::string& arg) {
		switch (state) {
		case ST_MODE:
			set_mode(arg[0]);
			(mode == AS_DISABLE) ? (state = ST_ERROR) : (state = ST_TIME);
			break;
		case ST_TIME: {
			std::vector<std::string> tokens;
			StringUtils::splitString(arg, &tokens, ':');
			if (2 != tokens.size()) {
				state = ST_ERROR;
				mode = AS_DISABLE;
			} else {
				hour = (char)StringUtils::stringToInteger(tokens[0]);
				minute = (char)StringUtils::stringToInteger(tokens[1]);
				state = ST_VALID;
			}
		}
		break;
		default:
			//do nothing. error
			break;
		}
	}

};
}
;

#endif
