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
 * \author Marko Stijak, Banja Luka, Bosnia and Herzegovina
 * 
 *
 */

#ifndef PARSER_FUNCTION_NODE_DATEFORMAT_H
#define PARSER_FUNCTION_NODE_DATEFORMAT_H 1

#include "palo.h"

#include <math.h>

#include <sstream>

#include "Collections/StringUtils.h"

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node datevalue
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeDateformat : public FunctionNode {
public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeDateformat(name, params);
	}

public:
	FunctionNodeDateformat() :
		FunctionNode() {
	}

	FunctionNodeDateformat(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeDateformat * cloned = new FunctionNodeDateformat(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {
		if (!params || params->size() != 2) {
			error = "function '" + name + "' needs two parameter";
			valid = false;
			return valid;
		}

		Node* param1 = params->at(0);
		Node* param2 = params->at(1);

		// validate parameter
		if (!param1->validate(server, database, cube, destination, error) || !param2->validate(server, database, cube, destination, error)) {
			return valid = false;
		}

		// check data type param1
		if (param1->getValueType() != Node::NODE_NUMERIC && param1->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "parameter1 of function '" + name + "' has wrong data type";
			return valid = false;
		}

		// check data type param2
		if (param2->getValueType() != Node::NODE_STRING && param2->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "parameter2 of function '" + name + "' has wrong data type";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_STRING;

		if (valid) {
			RuleValueType date = params->at(0)->getValue(cellPath, isCachable, mem_context);
			RuleValueType fmt = params->at(1)->getValue(cellPath, isCachable, mem_context);

			if (date.type == Node::NODE_NUMERIC && fmt.type == Node::NODE_STRING) {
				result.stringValue = Format(date.doubleValue, fmt.stringValue);
				return result;
			}
		}

		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

	static string Format(double time, string format) {
		string result;

		try {
			time_t tt = (time_t)time;

			/*
			 \y the last two digits of the year (97, 98, etc.)
			 \Y the four digits of the year (1997, 1998, etc.)
			 \m the two digits of the month (01 through 12)
			 \M the abbreviation of the month (JAN, FEB, etc.)
			 \d the two digits of the day (01 through 31)
			 \D the digit of the day (1 through 31)
			 \h the hour in military time (00 through 23)
			 \H the standard hour (1 through 12)
			 \i the minute (00 through 59)
			 \s the second (00 through 59)
			 \p a.m. or p.m. */

			stringstream ss;

			if (format.empty()) {
				ss << "%m-%d-%y";
			} else {
				size_t i;
				for (i = 0; i + 1 < format.length(); i++) {
					bool es = false;
					if (format[i] == '\\')
						switch (format[i + 1]) {
						case '\\':
							ss << "\\";
							es = true;
							break;
						case 'Y':
							ss << "%Y";
							es = true;
							break;
						case 'y':
							ss << "%y";
							es = true;
							break;
						case 'm':
							ss << "%m";
							es = true;
							break;
						case 'M':
							ss << "%b";
							es = true;
							break;
						case 'd':
							ss << "%d";
							es = true;
							break;
//						case 'D': // \D cannot be handled by strftime
//							ss << "%#d";
//							es = true;
//							break;
						case 'h':
							ss << "%H";
							es = true;
							break;
						case 'H':
							ss << "%I";
							es = true;
							break;
						case 'i':
							ss << "%M";
							es = true;
							break;
						case 's':
							ss << "%S";
							es = true;
							break;
						case 'p':
							ss << "%p";
							es = true;
							break;
						}
					if (es) {
						i++;
					} else {
						ss << format[i];
					}
				}
				if (i < format.length()) {
					ss << format[i];
				}
			}
			struct tm* t = localtime(&tt);
			if (t) {
				int max = 256;
				char *s = new char[max];
				strftime(s, max, ss.str().c_str(), t);
				result = s;

				//extra handler for \D
				strftime(s, max, "%d", t);
				char *p = s[0] == '0' ? s + 1 : s;
				for (;;) {
					size_t pos = result.find("\\D");
					if (pos == string::npos) {
						break;
					}
					result.replace(pos, 2, p);
				}

				delete[] s;
			} else {
				result = "";
			}
		} catch (ParameterException e) {
			result = "";
		}

		return result;
	}
};

}
#endif
