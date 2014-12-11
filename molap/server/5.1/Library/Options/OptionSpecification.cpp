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

#include "Options/OptionSpecification.h"

#include <iostream>

namespace palo {
bool OptionSpecification::hasSyntaxError(const string& programName) const
{
	if (specification.empty()) {
		cerr << programName << ": empty option specifier, " << "must be at least 1 character long." << endl;

		return true;
	} else if (1 < specification.length() && specification.find_first_of("|?:*+") == string::npos) {
		cerr << programName << ": bad option specifier \"" << specification << ", " << "second character must be in the set \"|?:*+\"." << endl;

		return true;
	}

	return false;
}

string OptionSpecification::format(uint32_t optctrls) const
{
	string result = "";

	if (isHiddenOption()) {
		return result;
	}

	char optchar = getShortOption();
	string longopt = getLongOption();
	string value = getValue();

	if ((optctrls & SHORT_ONLY) && ((!isNullOption()) || (optctrls & NOGUESSING))) {
		longopt = "";
	}

	if ((optctrls & LONG_ONLY) && (longopt != "" || (optctrls & NOGUESSING))) {
		optchar = 0;
	}

	if (isNullOption() && (longopt.empty())) {
		return result;
	}

	result.append("[");

	// print the single character option
	if (optchar != 0 && !isNullOption()) {
		result.append("-");
		result.append(1, optchar);

		if (longopt != "") {
			result.append("|");
		}
	}

	// print the long option
	if (longopt != "") {
		result.append("-");

		if (!(optctrls & (LONG_ONLY | SHORT_ONLY))) {
			result.append("-");
		}

		result.append(longopt);
	}

	// print any argument the option takes
	if (isValueTaken()) {
		result.append(" ");

		if (isValueOptional())
			result.append("[");

		result.append(value);

		if (isList()) {
			result.append(" ...");
		}

		if (isValueOptional())
			result.append("]");
	}

	result.append("]");

	return result;
}

void OptionSpecification::checkSpecification()
{
	hidden = false;
	optionChar = 0;
	longOption = "";
	valueSpecification = '|';
	valueDescription = "<value>";

	if (specification.empty()) {
		return;
	}

	if (specification[0] == '-') {
		hidden = true;
		specification = specification.substr(1);
	}

	if (specification.empty()) {
		return;
	}

	optionChar = specification[0];

	if (1 < specification.length()) {
		valueSpecification = specification[1];
	}

	if (2 < specification.length()) {
		if (specification[2] != ' ') {
			string rem = specification.substr(2);
			size_t pos = rem.find_first_of(' ');

			if (pos != string::npos) {
				longOption = rem.substr(0, pos);

				pos = rem.find_first_not_of(' ', pos);

				if (pos != string::npos) {
					valueDescription = rem.substr(pos);
				}
			} else {
				longOption = rem;
			}
		} else {
			size_t pos = specification.find_first_not_of(' ', 2);

			if (pos != string::npos) {
				valueDescription = specification.substr(pos);
			}
		}
	}
}
}
