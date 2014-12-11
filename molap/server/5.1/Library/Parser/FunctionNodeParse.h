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

#ifndef PARSER_FUNCTION_NODE_PARSE_H
#define PARSER_FUNCTION_NODE_PARSE_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node parse
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeParse : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeParse(name, params);
	}

public:
	FunctionNodeParse() :
		FunctionNode() {
	}

	FunctionNodeParse(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeParse * cloned = new FunctionNodeParse(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

	bool validate(PServer, PDatabase, PCube, Node*, string& message) {
		return valid = true;
	}
	;

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;
		return result;
	}

	virtual bool genCode(bytecode_generator& generator, uint8_t want) const {
		return generator.EmitNopCode();
	}

};

}
#endif
