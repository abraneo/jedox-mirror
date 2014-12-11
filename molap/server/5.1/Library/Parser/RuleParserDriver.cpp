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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Parser/RuleParserDriver.h"
#include "Parser/RuleParser.hpp"
#include "Parser/FunctionNodeFactory.h"
#include "Parser/PaloFunctionNodeFactory.h"
#include "Parser/ParseFunctionNodeFactory.h"
#include "Parser/FunctionNodeError.h"

#include <sstream>

const string RuleParserDriver::SIMPLE_RULE = "simple";
const string RuleParserDriver::POSITION_RULE = "position";
const string RuleParserDriver::PALO_FUNC_PREFIX = "palo.";

RuleParserDriver::RuleParserDriver() :
	trace_scanning(false), trace_parsing(false), simpleRulesOnly(false), position(0)
{
	result = 0;
	factory = new PaloFunctionNodeFactory();
}

RuleParserDriver::RuleParserDriver(set<string>* functionList) :
	trace_scanning(false), trace_parsing(false), simpleRulesOnly(false), position(0)
{
	result = 0;
	if (functionList && functionList->size() > 0) {
		factory = new ParseFunctionNodeFactory(functionList);
	} else {
		factory = new PaloFunctionNodeFactory();
	}
}

RuleParserDriver::~RuleParserDriver()
{
	if (factory) {
		delete factory;
	}
}

void RuleParserDriver::parse(const string &rule)
{
	size_t targetPos = rule.find_first_of("[{");
	if (targetPos && targetPos != string::npos) {
		// extract commands and rule
		string ruleCommandsRaw = rule.substr(0, targetPos);
		ruleString = rule.substr(targetPos);

		// parse commands
		vector<string> ruleCommands;
		StringUtils::splitString(ruleCommandsRaw, &ruleCommands, ';');

		for (vector<string>::const_iterator cmd = ruleCommands.begin(); cmd != ruleCommands.end(); ++cmd) {
			if (!cmd->compare(0,SIMPLE_RULE.length(), SIMPLE_RULE)) {
				simpleRulesOnly = true;
			} else if (!cmd->compare(0,POSITION_RULE.length(), POSITION_RULE)) {
				size_t equalPos = cmd->find('=');
				if (equalPos != string::npos) {
					const char* str = &(*cmd)[equalPos+1];
					setPosition(strtod(str, 0));
				}
			} else {
				Logger::warning << "Unknown rule option: " << *cmd << endl;
			}
		}
	} else {
		ruleString = rule;
	}
	scan_begin();
	yy::RuleParser parser(*this);
	parser.set_debug_level(trace_parsing);
	parser.parse();
	scan_end();
}

void RuleParserDriver::error(const yy::location& loc, const string& m)
{
	stringstream s;

	yy::position last = loc.end - 1;

	s << m << " at position " << last.column << " of line " << last.line;

	errorMessage = s.str();
}

void RuleParserDriver::error(const string& m)
{
	errorMessage = m;
}

FunctionNode* RuleParserDriver::createFunction(const string& name, vector<Node*> *params, string *error)
{
	if (simpleMode() && !name.compare(0,PALO_FUNC_PREFIX.length(), PALO_FUNC_PREFIX) && name.compare("palo.data")) {
		if (error) {
			*error = "function is not supported for simple rules";
			return 0;
		} else {
			return FunctionNodeError::createNode(name, params);
		}
	}
	return factory->createFunction(name, params);
}
SourceNode* RuleParserDriver::createMarker(RPVpsval *elements, RPVpival *elementsIds, string *error)
{
	if (simpleMode()) {
		*error = "markers are not supported in simple rules";
		return 0;

	}
	return new SourceNode(elements, elementsIds, true);
}
