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

#ifndef PARSER_FUNCTION_NODE_STR_H
#define PARSER_FUNCTION_NODE_STR_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node str
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeStr : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeStr(name, params);
	}

public:
	FunctionNodeStr() :
		FunctionNode() {
	}
	;
	FunctionNodeStr(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}
	;

public:

	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	Node * clone() {
		FunctionNodeStr * cloned = new FunctionNodeStr(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// add has two parameters
		if (!params || params->size() < 3) {
			error = "function '" + name + "' needs three parameters";
			return valid = false;
		}
		if (params->size() > 3) {
			error = "too many parameters for function '" + name + "'";
			return valid = false;
		}

		p1 = params->at(0);
		p2 = params->at(1);
		p3 = params->at(2);

		// validate parameters
		if (!p1->validate(server, database, cube, destination, error) || !p2->validate(server, database, cube, destination, error) || !p3->validate(server, database, cube, destination, error)) {
			return valid = false;
		}

		// check data type p1
		if (p1->getValueType() != Node::NODE_NUMERIC && p1->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "first parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		// check data type p2
		if (p2->getValueType() != Node::NODE_NUMERIC && p2->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "second parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		// check data type p3
		if (p3->getValueType() != Node::NODE_NUMERIC && p3->getValueType() != Node::NODE_UNKNOWN_VALUE) {
			error = "third parameter of function '" + name + "' has wrong data type";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {

		RuleValueType result;
		result.type = Node::NODE_STRING;

		if (valid) {
			RuleValueType v1 = p1->getValue(cellPath, isCachable, mem_context);
			RuleValueType v2 = p2->getValue(cellPath, isCachable, mem_context);
			RuleValueType v3 = p3->getValue(cellPath, isCachable, mem_context);

			if (v1.type == Node::NODE_NUMERIC && v2.type == Node::NODE_NUMERIC && v3.type == Node::NODE_NUMERIC) {
				double v = v1.doubleValue;
				int l = (int)v2.doubleValue;
				int d = (int)v3.doubleValue;
				result.stringValue = UTF8Comparer::doubleToString(v, l, d);
			}
		}

		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

protected:
	Node* p1;
	Node* p2;
	Node* p3;

};
}
#endif
