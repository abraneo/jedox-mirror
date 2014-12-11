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
 *
 */

#include <PaloSpreadsheetFuncs/SpreadsheetFuncsBase.h>
#include "PHPPaloLogger.h"

#include "c2c_plus_plus.h"

extern "C" void ChangeToNewStartIndex(void) {
	Palo::SpreadsheetFuncs::SpreadsheetFuncsBase::ChangeToNewStartIndex();
};

extern "C" void PHPInitSSL(const char * trustfile) {
	std::string t;
	if (trustfile) {
		t = trustfile;
	}
	Palo::SpreadsheetFuncs::SpreadsheetFuncsBase::InitSSL(t);
};

extern "C" void InternalPaloWeb(long value) {
	Palo::PHPPalo::PHPPaloLogger::InternalPaloWeb(value);
};

extern "C" void ActivateLogStderr(long value) {
	Palo::PHPPalo::PHPPaloLogger::ActivateLogStderr(value);
};

extern "C" long GetGoalSeekType(PHPPaloGoalSeekType type) {
	switch (type) {
		case PHPPalo_GOALSEEK_COMPLETE:
			return Palo::Types::GoalSeekComplete;

		case PHPPalo_GOALSEEK_EQUAL:
			return Palo::Types::GoalSeekEqual;

		case PHPPalo_GOALSEEK_RELATIVE:
			return Palo::Types::GoalSeekRelative;

		default:
			return -1;
	}
};

