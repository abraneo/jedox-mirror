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
 * 
 *
 */

#ifndef PARSER_FUNCTION_NODE_CONTINUE_H
#define PARSER_FUNCTION_NODE_CONTINUE_H 1

#include "palo.h"

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node simple continue
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeContinue : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeContinue(name, params);
	}

public:
	FunctionNodeContinue() :
		FunctionNode() {
	}

	FunctionNodeContinue(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeContinue * cloned = new FunctionNodeContinue(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:

	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

	bool validate(PServer, PDatabase, PCube, Node*, string& error) {
		// add has two parameters
		if (params && params->size() > 0) {
			error = "function '" + name + "' has no parameter";
			return valid = false;
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_UNKNOWN_VALUE;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		return generator.EmitContinue();
	}

	virtual bool isPlanCompatible(CPCubeArea area, Variability &varDimensions) const {
		varDimensions.fill();
		return  true;
	}

	virtual PPlanNode createPlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, Planner *parentPlanner) const
	{
		PCubeArea areaCopy(new CubeArea(*area));
		Planner planner(area->getCube(), areaCopy, parentPlanner);
		planner.setCurrentRule(rule);
		planner.setContinueRule(rule);
		PPlanNode source = planner.createPlan(CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
		return source;
	}
};

}
#endif
