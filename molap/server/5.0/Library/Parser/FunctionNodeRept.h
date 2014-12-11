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
 * 
 *
 */

#ifndef PARSER_FUNCTION_NODE_REPT_H
#define PARSER_FUNCTION_NODE_REPT_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node rept
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeRept : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeRept(name, params);
	}

public:
	FunctionNodeRept() :
		FunctionNode() {
	}

	FunctionNodeRept(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeRept * cloned = new FunctionNodeRept(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has two parameters
		if (!params || params->size() < 2) {
			error = "function '" + name + "' needs two parameters";
			return valid = false;
		}
		if (params->size() > 2) {
			error = "too many parameters for function '" + name + "'";
			return valid = false;
		}

		left = params->at(0);

		// validate parameters
		if (!left->validate(server, database, cube, destination, error)) {
			return valid = false;
		}

		// check data type left
		if (left->getValueType() != Node::NODE_STRING && left->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "first parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		right = params->at(1);

		if (!right->validate(server, database, cube, destination, error)) {
			return valid = false;
		}
		// check data type right
		if (right->getValueType() != Node::NODE_NUMERIC && right->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "second parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_STRING;
		result.stringValue = "";

		if (valid) {
			RuleValueType l = left->getValue(cellPath, isCachable, mem_context);
			RuleValueType r = right->getValue(cellPath, isCachable, mem_context);

			if (l.type == Node::NODE_STRING && l.stringValue.size() > 0 && r.type == Node::NODE_NUMERIC) {

				int max = (int)r.doubleValue;

				for (int i = 0; i < max; i++) {
					result.stringValue += l.stringValue;
				}
			}
		}

		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

protected:
	Node* left;
	Node* right;

};

}
#endif
