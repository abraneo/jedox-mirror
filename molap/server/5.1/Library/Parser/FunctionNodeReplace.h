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

#ifndef PARSER_FUNCTION_NODE_REPLACE_H
#define PARSER_FUNCTION_NODE_REPLACE_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node replace
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeReplace : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeReplace(name, params);
	}

public:
	FunctionNodeReplace() :
		FunctionNode() {
	}

	FunctionNodeReplace(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeReplace * cloned = new FunctionNodeReplace(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		if (!params || params->size() < 4) {
			error = "function '" + name + "' needs four parameters";
			return valid = false;
		}
		if (params->size() > 4) {
			error = "too many parameters for function '" + name + "'";
			return valid = false;
		}

		for (int i = 0; i < 4; i++) {
			Node* param = params->at(i);
			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			// validate data type
			if (i == 0 || i == 3) {
				if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
					error = "a parameter of function '" + name + "' has a wrong data type";
					return valid = false;
				}
			} else {
				if (param->getValueType() != Node::NODE_NUMERIC && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
					error = "a parameter of function '" + name + "' has a wrong data type";
					return valid = false;
				}
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		if (valid) {
			RuleValueType text = params->at(0)->getValue(cellPath, isCachable, mem_context);
			RuleValueType start = params->at(1)->getValue(cellPath, isCachable, mem_context);
			RuleValueType num = params->at(2)->getValue(cellPath, isCachable, mem_context);
			RuleValueType newt = params->at(3)->getValue(cellPath, isCachable, mem_context);

			if (text.type == Node::NODE_STRING && newt.type == Node::NODE_STRING && start.type == Node::NODE_NUMERIC && num.type == Node::NODE_NUMERIC) {

				int len = (int)text.stringValue.length();
				int s = (int)start.doubleValue;
				int n = (int)num.doubleValue;

				if (0 <= s && s < len && 0.0 <= n) {
					RuleValueType result;
					result.type = Node::NODE_STRING;

					if (s + n > len) {
						result.stringValue = text.stringValue.substr(0, s) + newt.stringValue;
					} else {
						result.stringValue = text.stringValue.substr(0, s) + newt.stringValue + text.stringValue.substr(s + n);
					}

					return result;
				}
			}
		}

		RuleValueType result;
		result.type = Node::NODE_STRING;
		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(3)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

};

}
#endif
