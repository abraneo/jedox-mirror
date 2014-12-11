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

#ifndef PARSER_FUNCTION_NODE_PALO_EPREV_H
#define PARSER_FUNCTION_NODE_PALO_EPREV_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNodePalo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo eprev
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePaloEprev : public FunctionNodePalo {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodePaloEprev(name, params);
	}

public:
	FunctionNodePaloEprev() :
		FunctionNodePalo() {
	}

	FunctionNodePaloEprev(const string& name, vector<Node*> *params) :
		FunctionNodePalo(name, params) {
	}

	Node * clone() {
		FunctionNodePaloEprev * cloned = new FunctionNodePaloEprev(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->databaseid = this->databaseid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {
		return validateParameter(server, database, cube, destination, error, 3, 0);
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_STRING;

		if (valid) {
			PDatabase database = getDatabase(Context::getContext()->getServer(), params->at(0), cellPath, isCachable, mem_context);
			PDimension dimension = getDimension(database, params->at(1), cellPath, isCachable, mem_context);
			Element* element = getElement(dimension, params->at(2), cellPath, isCachable, mem_context);

			if (element) {
				ElementsType elements = dimension->getElements(PUser(), false);
				ElementsType::iterator iter = find(elements.begin(), elements.end(), element);

				if (iter != elements.begin() && iter != elements.end()) {
					iter--;
					result.stringValue = (*iter)->getName(dimension->getElemNamesVector());
					return result;
				}
			}
		}

		result.stringValue = "";

		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(2)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode("palo.eprev"))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

};

}
#endif
