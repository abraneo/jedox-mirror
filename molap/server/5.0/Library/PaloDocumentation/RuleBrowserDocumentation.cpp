/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloDocumentation/RuleBrowserDocumentation.h"

#include <iostream>
#include <sstream>

#include "Olap/Cube.h"
#include "Olap/Rule.h"

namespace palo {
RuleBrowserDocumentation::RuleBrowserDocumentation(PDatabase database, PCube cube, const string& ruleString, const string& message) :
	BrowserDocumentation(message)
{
	vector<string> ids(1, StringUtils::convertToString(database->getId()));
	values["@database_identifier"] = ids;

	vector<string> ids2(1, StringUtils::convertToString(cube->getId()));
	values["@cube_identifier"] = ids2;

	vector<string> newRule(1, ruleString);
	values["@rule_new_rule"] = newRule;

	vector<string> identifiers;
	vector<string> text;
	vector<string> external;
	vector<string> comment;
	vector<string> timestamp;
	vector<string> active;
	vector<string> evalCnt;
	vector<string> nullCnt;

	vector<PRule> rules = cube->getRules(PUser());

	if (!rules.empty()) {
		for (vector<PRule>::iterator i = rules.begin(); i != rules.end(); ++i) {
			uint64_t evalCounter = 0;
			uint64_t evalCounterNull = 0;
			identifiers.push_back(StringUtils::convertToString((*i)->getId()));

			StringBuffer sb;
			(*i)->appendRepresentation(&sb, database, cube, true);
			text.push_back(StringUtils::escapeHtml(sb.c_str()));

			external.push_back(StringUtils::escapeHtml((*i)->getExternal()));
			comment.push_back(StringUtils::escapeHtml((*i)->getComment()));
			timestamp.push_back(StringUtils::convertToString((uint32_t)((*i)->getTimeStamp())));

			if ((*i)->isActive()) {
				active.push_back("yes");
			} else {
				active.push_back("no");
			}
			(*i)->getEvalCounters(&evalCounter, &evalCounterNull);
			evalCnt.push_back( StringUtils::convertToString( (uint32_t)evalCounter ) );
			nullCnt.push_back( StringUtils::convertToString( (uint32_t)evalCounterNull ) );
		}

		values["@rule_identifier"] = identifiers;
		values["@rule_text"] = text;
		values["@rule_external"] = external;
		values["@rule_comment"] = comment;
		values["@rule_timestamp"] = timestamp;
		values["@rule_active"] = active;
		values["@eval_counter"] = evalCnt;
		values["@null_counter"] = nullCnt;
	}
}
}
