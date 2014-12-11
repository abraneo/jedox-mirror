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

#ifndef PARSER_FUNCTION_NODE_DATEVALUE_H
#define PARSER_FUNCTION_NODE_DATEVALUE_H 1

#include "palo.h"

#include <math.h>

#include "Collections/StringUtils.h"

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node datevalue
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeDatevalue : public FunctionNode {
public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeDatevalue(name, params);
	}

public:
	FunctionNodeDatevalue() :
		FunctionNode() {
	}

	FunctionNodeDatevalue(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeDatevalue * cloned = new FunctionNodeDatevalue(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {
		if (!params || params->size() != 1) {
			error = "function '" + name + "' needs one parameter";
			valid = false;
			return valid;
		}

		Node* param = params->at(0);

		// validate parameter
		if (!param->validate(server, database, cube, destination, error)) {
			return valid = false;
		}

		// check data type left
		if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			RuleValueType date = params->at(0)->getValue(cellPath, isCachable, mem_context);

			if (date.type == Node::NODE_STRING && date.stringValue.length() == 8) {
				try {
					int m = StringUtils::stringToInteger(date.stringValue.substr(0, 2));
					int d = StringUtils::stringToInteger(date.stringValue.substr(3, 2));
					int y = StringUtils::stringToInteger(date.stringValue.substr(6, 4));
					bool validParams = true;

					struct tm t;
					t.tm_sec = 0;
					t.tm_min = 0;
					t.tm_hour = 12;
					t.tm_wday = 0;
					t.tm_yday = 0;
					t.tm_isdst = -1;
					t.tm_year = y;
					if (t.tm_year >= 1970 && t.tm_year < 2038) {
						t.tm_year -= 1900;
					} else if (t.tm_year >= 0 && t.tm_year < 100) {
						t.tm_year += 100;
					} else {
						result.doubleValue = 0;
						validParams = false;
					}
					if (validParams) {
						t.tm_mon = m - 1;
						t.tm_mday = d;

						time_t dtm = mktime(&t);
						result.doubleValue = (double)(dtm / 86400 * 86400);
					}
				} catch (ParameterException e) {
					result.doubleValue = 0.0;
				}

				return result;
			}
		}

		result.doubleValue = 0.0;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

};

}
#endif
