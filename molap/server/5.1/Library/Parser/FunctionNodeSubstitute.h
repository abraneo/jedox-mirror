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

#ifndef PARSER_FUNCTION_NODE_SUBSTITUTE_H
#define PARSER_FUNCTION_NODE_SUBSTITUTE_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node substitute
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeSubstitute : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeSubstitute(name, params);
	}

public:
	FunctionNodeSubstitute() :
		FunctionNode() {
	}

	FunctionNodeSubstitute(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeSubstitute * cloned = new FunctionNodeSubstitute(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		if (!params || params->size() < 3) {
			error = "function '" + name + "' needs three parameters";
			return valid = false;
		}
		if (params->size() > 3) {
			error = "too many parameters for function '" + name + "'";
			return valid = false;
		}

		for (int i = 0; i < 3; i++) {
			Node* param = params->at(i);
			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			// validate data type
			if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "a parameter of function '" + name + "' has a wrong data type";
				return valid = false;
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_STRING;

		if (valid) {
			RuleValueType text = params->at(0)->getValue(cellPath, isCachable, mem_context);
			RuleValueType oldt = params->at(1)->getValue(cellPath, isCachable, mem_context);
			RuleValueType newt = params->at(2)->getValue(cellPath, isCachable, mem_context);

			if (text.type == Node::NODE_STRING && oldt.type == Node::NODE_STRING && newt.type == Node::NODE_STRING) {
				string r = text.stringValue;

				if (!oldt.stringValue.empty()) {
					size_t pos = 0;

					while ((pos = r.find(oldt.stringValue, pos)) != string::npos) {
						r = r.substr(0, pos) + newt.stringValue + r.substr(pos + oldt.stringValue.size());
						pos += newt.stringValue.size();
					}
				}

				result.stringValue = r;

				return result;
			}
		}

		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

};
}
#endif
