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

#ifndef PARSER_FUNCTION_NODE_LT_H
#define PARSER_FUNCTION_NODE_LT_H 1

#include "palo.h"

#include "Parser/FunctionNodeComparison.h"
#include "Parser/StringNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node lt (<)
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeLt : public FunctionNodeComparison {
public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeLt(name, params, false);
	}

	static FunctionNode* createNodeInfix(const string& name, vector<Node*> *params) {
		return new FunctionNodeLt(name, params, true);
	}

public:
	FunctionNodeLt() :
		FunctionNodeComparison() {
	}

	FunctionNodeLt(const string& name, vector<Node*> *params, bool infix) :
		FunctionNodeComparison(name, params, infix) {
	}

	Node * clone() {
		FunctionNodeLt * cloned = new FunctionNodeLt(name, cloneParameters(), this->infix);
		cloned->valid = this->valid;
		cloned->left = this->valid ? (*(cloned->params))[0] : 0;
		cloned->right = this->valid ? (*(cloned->params))[1] : 0;
		return cloned;
	}

public:
	bool isDimensionRestriction(CPCube cube, IdentifierType *dimension) {
		if (valid) {
			return (left->isDimensionTransformation(cube, dimension) && right->isConstant()) || (left->isConstant() && right->isDimensionTransformation(cube, dimension));
		}

		return false;
	}

	set<Element*> computeDimensionRestriction(CPDatabase db, CPCube cube) {
		IdentifierType dimension;
		map<Element*, string> elements;
		set<Element*> result;
		string constant;
		bool isLeft;

		if (left->isDimensionTransformation(cube, &dimension) && right->isConstant()) {
			Logger::debug << "computing restrictions on dimension " << dimension << endl;

			elements = left->computeDimensionTransformations(db, cube);

			if (right->getNodeType() == Node::NODE_STRING_NODE) {
				constant = dynamic_cast<StringNode*>(right)->getStringValue();
				isLeft = false;
			} else {
				return result;
			}
		} else if (right->isDimensionTransformation(cube, &dimension) && left->isConstant()) {
			Logger::debug << "computing restrictions on dimension " << dimension << endl;

			elements = right->computeDimensionTransformations(db, cube);

			if (left->getNodeType() == Node::NODE_STRING_NODE) {
				constant = dynamic_cast<StringNode*>(left)->getStringValue();
				isLeft = true;
			} else {
				return result;
			}
		} else {
			return result;
		}

		Logger::debug << "restriction '<' for constant " << constant << endl;

		for (map<Element*, string>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
			Element* element = iter->first;
			const string& value = iter->second;

			if ((isLeft && constant < value) || (!isLeft && value < constant)) {
				Logger::debug << "element " << element->getName(db->lookupDimension(dimension, false)->getElemNamesVector()) << " maps to " << value << endl;
				result.insert(element);
			}
		}

		return result;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			RuleValueType l = left->getValue(cellPath, isCachable, mem_context);
			RuleValueType r = right->getValue(cellPath, isCachable, mem_context);
			if (l.type == Node::NODE_NUMERIC) {
				if (r.type == Node::NODE_NUMERIC) {
					result.doubleValue = (l.doubleValue < r.doubleValue) ? 1.0 : 0.0;
				}
				// 10 < "egal"       => true
				else {
					result.doubleValue = 1.0;
				}
			} else if (l.type == Node::NODE_STRING) {
				if (r.type == Node::NODE_STRING) {
					result.doubleValue = (l.stringValue < r.stringValue) ? 1.0 : 0.0;
				}
				// "egal"  <=100          => false
				else {
					result.doubleValue = 0.0;
				}
			}
			return result;
		}

		// not valid or unknown type
		result.doubleValue = 0.0;
		return result;
	}
};

}
#endif
