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

#include "PaloDocumentation/FileDocumentation.h"

#include "Logger/Logger.h"

#include <iostream>
#include "InputOutput/FileUtils.h"

#include "Collections/StringUtils.h"
#include "Olap/Server.h"

namespace palo {
FileDocumentation::FileDocumentation(const string& filename) :
	lastParamId(-1), lastResultId(-1), lastExampleId(-1)
{
	string keyword;
	bool foundKeyword = false;
	string value = "";
	string line;
	int lineNumber = 0;

	FileUtils::paloifstream file(filename.c_str());

	if (!file) {
		Logger::error << "cannot open documentation file '" << filename << "'" << endl;
		return;
	}

	while (!file.eof()) {
		getline(file, line);

		if (line.empty()) {
			value.append("\n");
		} else {

			// check for keyword (starting with @)
			if (line[0] == '@') {
				if (foundKeyword) {
					processDocumentationEntry(keyword, value);
					value = "";
				}

				foundKeyword = true;

				// split line into keyword and value
				size_t end = line.find(" ");

				if (end == string::npos) {
					keyword = line;
				} else {
					keyword = line.substr(0, end);
					value = line.substr(end);
				}
			} else {
				value.append("\n");
				value.append(line);
			}
		}

		lineNumber++;
	}

	if (foundKeyword) {
		processDocumentationEntry(keyword, value);
	}
}

bool FileDocumentation::hasDocumentationEntry(const string& name)
{
	map<string, vector<string> >::const_iterator i = docEntries.find(name);

	if (i != docEntries.end()) {
		return true;
	} else {
		return false;
	}
}

const vector<string>& FileDocumentation::getDocumentationEntries(const string& name)
{
	static const vector<string> error;

	map<string, vector<string> >::const_iterator i = docEntries.find(name);

	if (i != docEntries.end()) {
		return i->second;
	} else {
		return error;
	}
}

const string& FileDocumentation::getDocumentationEntry(const string& name, size_t index)
{
	static const string error = "";
	const vector<string>& v = getDocumentationEntries(name);

	if (index < v.size()) {
		return v[index];
	} else {
		return error;
	}
}

void FileDocumentation::processDocumentationEntry(const string& name, const string& value)
{
	if (name == "@param") {
		lastParamId = addDocumentationEntry(name, value);

		// create empty values for some other parameter attributes
		addDocumentationEntry("@param_type", "");
		addDocumentationEntry("@param_description", "");
	} else if (name == "@result") {
		lastResultId = addDocumentationEntry(name, value);

		// create empty values for some other parameter attributes
		addDocumentationEntry("@result_type", "");
		addDocumentationEntry("@result_description", "");
	} else if (name == "@example") {
		lastExampleId = addDocumentationEntry(name, value);

		// create empty values for some other parameter attributes
		addDocumentationEntry("@example_description", "");
	} else if (name == "@server") {
		lastServerId = addDocumentationEntry(name, value);
		addDocumentationEntry("@server_description", "");
	} else if (name == "@database") {
		lastDatabaseId = addDocumentationEntry(name, value);
		addDocumentationEntry("@database_description", "");
	} else if (name == "@dimension") {
		lastDimensionId = addDocumentationEntry(name, value);
		addDocumentationEntry("@dimension_description", "");
	} else if (name == "@element") {
		lastElementId = addDocumentationEntry(name, value);
		addDocumentationEntry("@element_description", "");
	} else if (name == "@cube") {
		lastCubeId = addDocumentationEntry(name, value);
		addDocumentationEntry("@cube_description", "");
	} else if (name == "@cell") {
		lastCellId = addDocumentationEntry(name, value);
		addDocumentationEntry("@cell_description", "");
	} else if (name == "@param_type") {
		if (lastParamId != -1) {
			replaceDocumentationEntry(name, value, lastParamId);
		}
	} else if (name == "@param_description") {
		if (lastParamId != -1) {
			replaceDocumentationEntry(name, value, lastParamId);
		}
	} else if (name == "@result_type") {
		if (lastResultId != -1) {
			replaceDocumentationEntry(name, value, lastResultId);
		}
	} else if (name == "@result_description") {
		if (lastResultId != -1) {
			replaceDocumentationEntry(name, value, lastResultId);
		}
	} else if (name == "@example_description") {
		if (lastExampleId != -1) {
			replaceDocumentationEntry(name, value, lastExampleId);
		}
	} else if (name == "@server_description") {
		if (lastServerId != -1) {
			replaceDocumentationEntry(name, value, lastServerId);
		}
	} else if (name == "@database_description") {
		if (lastDatabaseId != -1) {
			replaceDocumentationEntry(name, value, lastDatabaseId);
		}
	} else if (name == "@dimension_description") {
		if (lastDimensionId != -1) {
			replaceDocumentationEntry(name, value, lastDimensionId);
		}
	} else if (name == "@element_description") {
		if (lastElementId != -1) {
			replaceDocumentationEntry(name, value, lastElementId);
		}
	} else if (name == "@cube_description") {
		if (lastCubeId != -1) {
			replaceDocumentationEntry(name, value, lastCubeId);
		}
	} else if (name == "@cell_description") {
		if (lastCellId != -1) {
			replaceDocumentationEntry(name, value, lastCellId);
		}
	} else if (name == "@version") {
		addDocumentationEntry("@version", Server::getVersionRevision());
	} else {
		addDocumentationEntry(name, value);
	}
}

ssize_t FileDocumentation::addDocumentationEntry(const string& name, const string& value)
{
	map<string, vector<string> >::iterator i = docEntries.find(name);

	if (i != docEntries.end()) {

		// append value
		i->second.push_back(StringUtils::trim(value));

		return (ssize_t)(i->second.size() - 1);
	} else {
		docEntries[name].push_back(StringUtils::trim(value));
		return 0;
	}
}

void FileDocumentation::replaceDocumentationEntry(const string& name, const string& value, size_t position)
{
	map<string, vector<string> >::iterator i = docEntries.find(name);

	if (i != docEntries.end()) {
		if (position < i->second.size()) {
			vector<string>::iterator vi = i->second.begin() + position;

			i->second.insert(vi, StringUtils::trim(value));
			vi = i->second.begin() + position + 1;
			i->second.erase(vi);
		}
	}
}
}
