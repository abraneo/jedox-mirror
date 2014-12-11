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

#ifndef PARSER_FUNCTION_NODE_UPPER_H
#define PARSER_FUNCTION_NODE_UPPER_H 1

#include "palo.h"

#include "Collections/StringUtils.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node upper
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeUpper : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeUpper(name, params);
	}

public:
	FunctionNodeUpper() :
		FunctionNode() {
	}

	FunctionNodeUpper(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeUpper * cloned = new FunctionNodeUpper(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	bool isDimensionTransformation(CPCube cube, IdentifierType* dimension) {
		if (valid) {
			return params->at(0)->isDimensionTransformation(cube, dimension);
		}

		return false;
	}

	map<Element*, string> computeDimensionTransformations(CPDatabase db, CPCube cube) {
		map<Element*, string> elements = params->at(0)->computeDimensionTransformations(db, cube);

		for (map<Element*, string>::iterator i = elements.begin(); i != elements.end(); i++) {
			i->second = StringUtils::toupper(i->second);
		}

		return elements;
	}

	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// upper has one params
		if (!params || params->size() != 1) {
			error = "function '" + name + "' needs one parameter";
			return valid = false;
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
		result.type = Node::NODE_STRING;

		if (valid) {
			RuleValueType value = params->at(0)->getValue(cellPath, isCachable, mem_context);

			if (value.type == Node::NODE_STRING) {
				result.stringValue = StringUtils::toupper(value.stringValue);
				return result;
			}
		}

		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

};
}
#endif
