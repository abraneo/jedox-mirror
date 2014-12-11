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

#ifndef PARSER_FUNCTION_NODE_MUL_H
#define PARSER_FUNCTION_NODE_MUL_H 1

#include "palo.h"

#include "Parser/FunctionNodeSimple.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node mul
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeMul : public FunctionNodeSimple {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeMul(name, params, false);
	}

	static FunctionNode* createNodeInfix(const string& name, vector<Node*> *params) {
		return new FunctionNodeMul(name, params, true);
	}

public:
	FunctionNodeMul() :
		FunctionNodeSimple() {
	}

	FunctionNodeMul(const string& name, vector<Node*> *params, bool infix) :
		FunctionNodeSimple(name, params, infix) {
	}

	Node * clone() {
		FunctionNodeMul * cloned = new FunctionNodeMul(name, cloneParameters(), this->infix);
		cloned->valid = this->valid;
		cloned->left = this->valid ? (*(cloned->params))[0] : 0;
		cloned->right = this->valid ? (*(cloned->params))[1] : 0;
		return cloned;
	}

public:
	virtual bool isPlanCompatible(CPCubeArea area, Variability &varDimensions) const {
		return FunctionNodeSimple::isPlanCompatible(area, varDimensions, true);
	}

	virtual PlanNodeType getPlanType(bool &valid) const {
		valid = true;
		return MULTIPLICATION;
	}

	NodeType getNodeType() const {
		return NODE_FUNCTION_MULT;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;

		if (!valid)
			return result;

		RuleValueType l = left->getValue(cellPath, isCachable, mem_context);
		if (Node::NODE_NUMERIC != l.type || 0.0 == l.doubleValue)
			return result;

		RuleValueType r = right->getValue(cellPath, isCachable, mem_context);

		if (Node::NODE_NUMERIC != r.type || 0.0 == r.doubleValue) {
			palo::Node* temp = left;
			left = right;
			right = temp;

			return result;

		}

		result.doubleValue = l.doubleValue * r.doubleValue;

		return result;
	}

};

}
#endif
