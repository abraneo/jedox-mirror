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

#include "Options/OptionsFileIterator.h"
#include "Logger/Logger.h"
#include "Collections/StringUtils.h"

namespace palo {
OptionsFileIterator::OptionsFileIterator(const string& filename) :
	index(0)
{
	FileUtils::paloifstream is(filename.c_str());

	if (!is) {
		Logger::warning << "cannot open options file '" << filename << "'" << endl;
		return;
	}

	parseFile(is);
}

void OptionsFileIterator::parseFile(FileUtils::paloifstream& is)
{
	while (!is.eof()) {
		string line;

		getline(is, line);

		if (StringUtils::trim(line).empty()) {
			continue;
		}

		if (line[0] == '#') {
			continue;
		}

		parseLine(line);
	}
}

void OptionsFileIterator::parseLine(const string& line)
{
	bool startOption = true;
	size_t pos = 0;

	// if a line starts with a white space, it is a continued from the previous line
	if (line[0] == ' ' || line[0] == '\t') {
		startOption = false;
		pos = line.find_first_not_of(" \t", pos);

		if (pos == string::npos) {
			return;
		}

		startOption = false;
	}

	while (pos != string::npos) {
		string arg = nextToken(line, pos);

		if (startOption) {
			arg = "--" + arg;
			startOption = false;
		}

		arguments.push_back(arg);
	}
}

string OptionsFileIterator::nextToken(const string& line, size_t& pos)
{
	string arg;

	if (line[pos] == '"') {
		size_t e = line.find_first_of("\"", pos + 1);

		if (e == string::npos) {
			arg = line.substr(pos);
			pos = e;
			return arg;
		}

		arg = line.substr(pos + 1, e - pos - 1);
		pos = e + 1;
	} else {
		size_t e = line.find_first_of(" \t\r\n", pos + 1);

		if (e == string::npos) {
			arg = line.substr(pos);
			pos = e;
			return arg;
		}

		arg = line.substr(pos, e - pos);
		pos = e + 1;
	}

	pos = line.find_first_not_of(" \t\r\n", pos);

	return arg;
}
}
