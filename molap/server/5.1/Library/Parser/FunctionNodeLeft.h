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

#ifndef PARSER_FUNCTION_NODE_LEFT_H
#define PARSER_FUNCTION_NODE_LEFT_H 1

#include "palo.h"

#include "Parser/DoubleNode.h"
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node left
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeLeft : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeLeft(name, params);
	}

public:
	FunctionNodeLeft() :
		FunctionNode() {
	}

	FunctionNodeLeft(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeLeft * cloned = new FunctionNodeLeft(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	bool isDimensionTransformation(CPCube cube, IdentifierType* dimension) {
		if (valid) {
			return left->isDimensionTransformation(cube, dimension) && (right == 0 || right->isConstant());
		}

		return false;
	}

	map<Element*, string> computeDimensionTransformations(CPDatabase db, CPCube cube) {
		map<Element*, string> elements = left->computeDimensionTransformations(db, cube);
		size_t pos = 1;

		if (right) {
			if (right->getNodeType() != Node::NODE_DOUBLE_NODE) {
				for (map<Element*, string>::iterator i = elements.begin(); i != elements.end(); i++) {
					i->second = "";
				}

				return elements;
			}

			pos = (unsigned int)dynamic_cast<DoubleNode*>(right)->getDoubleValue();
		}

		for (map<Element*, string>::iterator i = elements.begin(); i != elements.end(); i++) {

			i->second = i->second.substr(0, pos);
		}

		return elements;
	}

	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has one or two parameters
		if (!params || params->size() < 1) {
			error = "function '" + name + "' needs one or two parameters";
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

		if (params->size() == 1) {
			right = 0;
		} else {
			right = params->at(1);

			if (!right->validate(server, database, cube, destination, error)) {
				return valid = false;
			}
			// check data type right
			if (right->getValueType() != Node::NODE_NUMERIC && right->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "second parameter of function '" + name + "' has wrong data type";
				return valid = false;
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		if (valid) {
			RuleValueType l = left->getValue(cellPath, isCachable, mem_context);

			if (l.type == Node::NODE_STRING && l.stringValue.size() > 0) {

				if (right) {
					RuleValueType r = right->getValue(cellPath, isCachable, mem_context);
					if (r.type == Node::NODE_NUMERIC) {
						int32_t len = (int32_t)r.doubleValue;
						RuleValueType result;
						result.type = Node::NODE_STRING;
						result.stringValue = UTF8Comparer::left(l.stringValue, len);
						return result;
					}
				} else {
					// default = 1
					RuleValueType result;
					result.type = Node::NODE_STRING;
					result.stringValue = UTF8Comparer::left(l.stringValue, 1);
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
		if (1 < params->size()) {
			if (!params->at(1)->genCode(generator, Node::NODE_NUMERIC))
				return false;
		} else {
			if (!generator.EmitLdConstDblCode(1.0))
				return false;
		}
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
