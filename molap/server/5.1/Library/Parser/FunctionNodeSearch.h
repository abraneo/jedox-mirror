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

#ifndef PARSER_FUNCTION_NODE_SEARCH_H
#define PARSER_FUNCTION_NODE_SEARCH_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node search
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeSearch : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeSearch(name, params);
	}

public:
	FunctionNodeSearch() :
		FunctionNode() {
	}

	FunctionNodeSearch(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeSearch * cloned = new FunctionNodeSearch(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_NUMERIC;
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
		if (right->getValueType() != Node::NODE_STRING && right->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "second parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;

		if (valid) {
			RuleValueType l = left->getValue(cellPath, isCachable, mem_context);
			RuleValueType r = right->getValue(cellPath, isCachable, mem_context);

			if (l.type == Node::NODE_STRING && r.type == Node::NODE_STRING) {
				result.doubleValue = (double)StringUtils::simpleSearch(r.stringValue, l.stringValue);
			}
		}

		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

protected:
	Node* left;
	Node* right;

};
}
#endif
