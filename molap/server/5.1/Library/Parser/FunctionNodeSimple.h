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

#ifndef PARSER_FUNCTION_NODE_SIMPLE_H
#define PARSER_FUNCTION_NODE_SIMPLE_H 1

#include "palo.h"

#include <string>
#include "Olap/Rule.h"
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node simple
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeSimple : public FunctionNode {
public:
	FunctionNodeSimple() :
		FunctionNode() {
	}

	FunctionNodeSimple(const string& name, vector<Node*> *params, bool infix = false) :
		FunctionNode(name, params), infix(infix) {
	}

public:
	virtual PlanNodeType getPlanType(bool &valid) const
	{
		valid = false;
		return SOURCE;
	}

	virtual PPlanNode createPlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, Planner *parentPlanner) const
	{
		PPlanNode result;
		valid = true;
		constResult = 0;

		// recursive call for function operand nodes
		vector<PPlanNode> sourceNodes(2);
		double leftConst = 0;
		double rightConst = 0;
		sourceNodes[0] = getLeftNode()->createPlan(area, rule, leftConst, valid, parentPlanner);
		if (sourceNodes[0]) {
			sourceNodes[1] = getRightNode()->createPlan(area, rule, rightConst, valid, parentPlanner);
		}
		if (!sourceNodes[1]) {
			return result;
		}
		const ConstantPlanNode *left = sourceNodes[0]->getType() == CONSTANT ? dynamic_cast<const ConstantPlanNode *>(sourceNodes[0].get()) : 0;
		const ConstantPlanNode *right = sourceNodes[1]->getType() == CONSTANT ? dynamic_cast<const ConstantPlanNode *>(sourceNodes[1].get()) : 0;

		if (left && right && left->getDefaultValue() && right->getDefaultValue()) {
			valid = true;
			// both sources are constants
			switch (getNodeType()) {
				case Node::NODE_FUNCTION_MULT:
					constResult = left->getDefaultValue()->getNumeric() * right->getDefaultValue()->getNumeric();
					break;
				case Node::NODE_FUNCTION_DIV:
					constResult = rightConst == 0 ? 0 : left->getDefaultValue()->getNumeric() / right->getDefaultValue()->getNumeric();
					break;
				case Node::NODE_FUNCTION_ADD:
					constResult = left->getDefaultValue()->getNumeric() + right->getDefaultValue()->getNumeric();
					break;
				case Node::NODE_FUNCTION_SUB:
					constResult = left->getDefaultValue()->getNumeric() - right->getDefaultValue()->getNumeric();
					break;
				default:
					constResult = 0;
					valid = false;
					CPCube cube = area->getCube();
					Logger::debug << "unexpected rule node type in Planner::createRulePlan! Cube: \"" << cube->getName() << "\" Rule: " << rule->getId() << endl;
					break;
			}
			if (valid) {
				result = PPlanNode(new ConstantPlanNode(area, CellValue(constResult)));
			}
		} else {
			PlanNodeType planType = getPlanType(valid); // just to identify supported operation type
			CPCube cube = area->getCube();
			double otherConst = 0;
			if (!valid) {
				Logger::debug << "unexpected rule node type in Planner::createRulePlan! Cube: \"" << cube->getName() << "\" Rule: " << rule->getId() << endl;
				return result;
			}

			// only multiplication and division is supported
			if (planType == MULTIPLICATION || planType == DIVISION) {
				// one of the sources is constant
				if (left && left->getDefaultValue()) {
					otherConst = left->getDefaultValue()->getNumeric();
					sourceNodes[0].reset();
				} else if (right && right->getDefaultValue()) {
					otherConst = right->getDefaultValue()->getNumeric();
					sourceNodes[1].reset();
				}
			}
			result = PPlanNode(new PlanNode(planType, area, sourceNodes, cube->getCache(), cube, otherConst));
		}
		return result;
	}

	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	bool isSimple() const {
		return !infix;
	}

	bool validate(PServer, PDatabase, PCube, Node*, string&);

	void appendRepresentation(StringBuffer*, CPDatabase db, CPCube cube) const;

	uint32_t guessType(uint32_t level);

	bool genCode(bytecode_generator& generator, uint8_t want) const;

	Node* getLeftNode() const {
		return left;
	}

	Node* getRightNode() const {
		return right;
	}

protected:
	bool isPlanCompatible(CPCubeArea area, Variability &varDimensions, bool enableConstants) const {
		// recursive call for function operand nodes
		bool res1, res2, result = false;
		Variability leftVarDimensions;
		Variability rightVarDimensions;
		res1 = getLeftNode()->isPlanCompatible(area, leftVarDimensions);
		res2 = getRightNode()->isPlanCompatible(area, rightVarDimensions);
		if (res1 && res2) {
			result = true;
		} else if (!res1 && !res2) {
			result = false;
		} else if (res1 && enableConstants) {
			// res2 == false:  [] * const
			result = rightVarDimensions.empty();
		} else if (enableConstants) {
			// res1 == false: : const * []
			result = leftVarDimensions.empty();
		}
		varDimensions |= leftVarDimensions;
		varDimensions |= rightVarDimensions;
		return  result;
	}

	Node* left;
	Node* right;
	bool infix;
};
}
#endif
