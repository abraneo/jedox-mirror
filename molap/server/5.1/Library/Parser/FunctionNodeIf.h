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

#ifndef PARSER_FUNCTION_NODE_IF_H
#define PARSER_FUNCTION_NODE_IF_H 1

#include "palo.h"

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node if
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeIf : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeIf(name, params);
	}

public:
	FunctionNodeIf() :
		FunctionNode() {
	}

	FunctionNodeIf(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeIf * cloned = new FunctionNodeIf(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	NodeType getNodeType() const {
		return NODE_FUNCTION_IF;
	}

	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has three parameters
		if (!params || params->size() != 3) {
			error = "function '" + name + "' needs three parameters";
			return valid = false;
		}

		for (int i = 0; i < 3; i++) {
			Node* param = params->at(i);
			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {

		if (valid) {
			RuleValueType test = params->at(0)->getValue(cellPath, isCachable, mem_context);
			bool first = true;

			if (test.type == Node::NODE_NUMERIC) {
				if (test.doubleValue == 0.0) {
					first = false;
				}
			} else if (test.type == Node::NODE_STRING) {
				if (test.stringValue.length() == 0) {
					first = false;
				}
			}

			if (first) {
				return params->at(1)->getValue(cellPath, isCachable, mem_context);
			} else {
				return params->at(2)->getValue(cellPath, isCachable, mem_context);
			}
		}

		RuleValueType result;
		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		uint32_t end_of_cond;
		uint32_t end_of_if;
		if (!params->at(0)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		generator.increaseIfLevel();
		if (!generator.EmitIfCondCode(&end_of_cond, &end_of_if))
			return false;
		if (!params->at(1)->genCode(generator, want))
			return false;
		if (!generator.EmitIfIfCode(&end_of_cond, &end_of_if))
			return false;
		if (!params->at(2)->genCode(generator, want))
			return false;
		generator.decreaseIfLevel();
		return generator.EmitIfElseCode(&end_of_cond, &end_of_if);
	}

};

}
#endif
