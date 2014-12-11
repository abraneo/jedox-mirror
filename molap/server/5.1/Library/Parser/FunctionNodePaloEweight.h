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

#ifndef PARSER_FUNCTION_NODE_PALO_EWEIGHT_H
#define PARSER_FUNCTION_NODE_PALO_EWEIGHT_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNodePalo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo eweight
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePaloEweight : public FunctionNodePalo {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodePaloEweight(name, params);
	}

public:
	FunctionNodePaloEweight() :
		FunctionNodePalo() {
	}

	FunctionNodePaloEweight(const string& name, vector<Node*> *params) :
		FunctionNodePalo(name, params) {
	}

	Node * clone() {
		FunctionNodePaloEweight * cloned = new FunctionNodePaloEweight(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->databaseid = this->databaseid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {
		return validateParameter(server, database, cube, destination, error, 4, 0);
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			PDatabase database = getDatabase(Context::getContext()->getServer(), params->at(0), cellPath, isCachable, mem_context);
			PDimension dimension = getDimension(database, params->at(1), cellPath, isCachable, mem_context);
			Element* parent = getElement(dimension, params->at(2), cellPath, isCachable, mem_context);
			Element* child = getElement(dimension, params->at(3), cellPath, isCachable, mem_context);

			if (parent && child) {
				const IdentifiersWeightType *children = parent->getChildren();

				for (IdentifiersWeightType::const_iterator i = children->begin(); i != children->end(); i++) {
					Element *c = dimension->lookupElement(i->first, false);
					if (c == child) {
						result.doubleValue = i->second;
						return result;
					}
				}

			}
		}

		result.doubleValue = 0.0;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(3)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode("palo.eweight"))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

};

}
#endif
