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

#ifndef PARSER_FUNCTION_NODE_DIV_H
#define PARSER_FUNCTION_NODE_DIV_H 1

#include "palo.h"

#include "Parser/FunctionNodeSimple.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node div
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeDiv : public FunctionNodeSimple {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeDiv(name, params, false);
	}

	static FunctionNode* createNodeInfix(const string& name, vector<Node*> *params) {
		return new FunctionNodeDiv(name, params, true);
	}

public:
	FunctionNodeDiv() :
		FunctionNodeSimple() {
	}

	FunctionNodeDiv(const string& name, vector<Node*> *params, bool infix) :
		FunctionNodeSimple(name, params, infix) {
	}

	Node * clone() {
		FunctionNodeDiv * cloned = new FunctionNodeDiv(name, cloneParameters(), this->infix);
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
		return DIVISION;
	}

	NodeType getNodeType() const {
		return NODE_FUNCTION_DIV;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			RuleValueType l = left->getValue(cellPath, isCachable, mem_context);
			RuleValueType r = right->getValue(cellPath, isCachable, mem_context);
			if (l.type == Node::NODE_NUMERIC && r.type == Node::NODE_NUMERIC && r.doubleValue != 0.0) {
				result.doubleValue = l.doubleValue / r.doubleValue;
				return result;
			}
		}

		result.doubleValue = 0.0;
		return result;
	}

};

}
#endif
