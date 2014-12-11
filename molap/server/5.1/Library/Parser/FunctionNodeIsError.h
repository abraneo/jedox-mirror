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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PARSER_FUNCTION_NODE_ISERROR_H
#define PARSER_FUNCTION_NODE_ISERROR_H 1

#include "palo.h"

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node str
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeIsError : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeIsError(name, params);
	}

public:
	FunctionNodeIsError() :
		FunctionNode() {
	}
	;
	FunctionNodeIsError(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}
	;

public:

	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	Node * clone() {
		FunctionNodeIsError * cloned = new FunctionNodeIsError(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// add has two parameters
		if (!params || params->size() != 1) {
			error = "function '" + name + "' needs exactly one parameter";
			return valid = false;
		}

		p1 = params->at(0);

		// validate parameters
		if (!p1->validate(server, database, cube, destination, error)) {
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {

		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			try {
				RuleValueType v1 = p1->getValue(cellPath, isCachable, mem_context);
				if (v1.type == Node::NODE_STRING || v1.type == Node::NODE_NUMERIC) {
					result.doubleValue = 0;
					return result;
				}
			} catch (...) {

			}
		}

		result.doubleValue = 1;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (p1->getValueType() == Node::NODE_NUMERIC || p1->getValueType() == Node::NODE_UNKNOWN_VALUE) {
			if (!p1->genCode(generator, Node::NODE_NUMERIC))
				return false;
			if (!generator.EmitIsError())
				return false;
		} else {
			if (!p1->genCode(generator, Node::NODE_STRING))
				return false;
			if (!generator.EmitIsError())
				return false;
		}
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

protected:
	Node* p1;
};

}
#endif
