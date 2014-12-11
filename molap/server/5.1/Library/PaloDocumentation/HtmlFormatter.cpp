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

#include "PaloDocumentation/HtmlFormatter.h"

#include "Logger/Logger.h"

#include <iostream>
#include "InputOutput/FileUtils.h"

#include "Collections/StringUtils.h"

namespace palo {
HtmlFormatter::HtmlFormatter(const string& templateName)
{
	FileUtils::paloifstream file(templateName.c_str());

	if (!file) {
		Logger::error << "cannot open documentation file '" << templateName << "'" << endl;
		return;
	}

	while (!file.eof()) {
		string line;

		getline(file, line);
		templateBuffer.push_back(line);
	}
}

const StringBuffer& HtmlFormatter::getDocumentation(Documentation * documentation)
{
	generateDocumentation(documentation);
	return htmlDocumentation;
}
;

void HtmlFormatter::generateDocumentation(Documentation * documentation)
{
	for (vector<string>::const_iterator iter = templateBuffer.begin(); iter != templateBuffer.end(); ++iter) {
		const string& line = *iter;

		// check for "LOOP"
		size_t s = line.find("LOOP");

		if (s == 0) {
			string loopString = "";
			string loopVariable = "";
			bool foundEndLoop = false;

			// get loop variable
			size_t end = line.find(" ");

			if (end != string::npos) {
				loopVariable = line.substr(end);
				loopVariable = StringUtils::trim(loopVariable);
			}

			while (iter != templateBuffer.end() && !foundEndLoop) {
				const string& loopLine = *++iter;

				s = loopLine.find("END_LOOP");

				if (s == 0) {
					foundEndLoop = true;
				} else {
					loopString.append(loopLine);
					loopString.append("\n");
				}
			}

			// process loop
			if (documentation->hasDocumentationEntry(loopVariable)) {

				// get size of
				size_t maxLoop = documentation->getDocumentationEntries(loopVariable).size();

				if (maxLoop > 0) {
					processTemplateLine(documentation, loopString, maxLoop);
				}
			}
		} else {
			processTemplateLine(documentation, line + "\n");
		}
	}
}

void HtmlFormatter::processTemplateLine(Documentation * documentation, const string& line)
{
	processTemplateLine(documentation, line, false, 1);
}

void HtmlFormatter::processTemplateLine(Documentation * documentation, const string& line, size_t maxLoops)
{
	processTemplateLine(documentation, line, true, maxLoops);
}

void HtmlFormatter::processTemplateLine(Documentation * documentation, const string& line, bool withLoop, size_t maxLoops)
{
	// find keywords in the documentation template
	// example "{@keyword}" or "{@keyword[]}" (for loops)

	// process line for every loop
	// (this not fast but very easy)
	for (uint32_t i = 0; i < maxLoops; i++) {
		size_t begin = 0;

		// find first keyword
		size_t end = line.find("{@");

		while (end != string::npos) {
			htmlDocumentation.appendText(line.substr(begin, end - begin));

			size_t last = line.find("}", end);

			if (last != string::npos) {
				string keyword = line.substr(end + 1, last - end - 1);
				long int index = 0;

				if (withLoop) {
					size_t e = keyword.find("[]");

					if (e != string::npos) {
						keyword = keyword.substr(0, e);
						index = i;
					}
				}

				// special template keywords
				if (keyword == "@loop_number") {
					htmlDocumentation.appendInteger(i);
				} else if (documentation->hasDocumentationEntry(keyword)) {
					htmlDocumentation.appendText(documentation->getDocumentationEntry(keyword, index));
				}

				begin = last + 1;
			} else {
				begin = end;
			}

			// find next keyword
			end = line.find("{@", begin);
		}

		htmlDocumentation.appendText(line.substr(begin, end - begin));
	}
}
}
