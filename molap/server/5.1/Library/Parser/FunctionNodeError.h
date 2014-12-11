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

#ifndef PARSER_FUNCTION_NODE_ERROR_H
#define PARSER_FUNCTION_NODE_ERROR_H 1

#include "palo.h"

#include "Parser/FunctionNode.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node error
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeError : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeError(name, params);
	}

public:
	FunctionNodeError() :
		FunctionNode() {
	}

	FunctionNodeError(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeError * cloned = new FunctionNodeError(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

	bool validate(PServer, PDatabase, PCube, Node*, string& message) {
		message = "unknown function '" + name + "'";
		return valid = false;
	}
	;

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_UNKNOWN_VALUE;
		return result;
	}

	virtual bool genCode(bytecode_generator& generator, uint8_t want) const {
		return generator.EmitNopCode();
	}

};

}
#endif
