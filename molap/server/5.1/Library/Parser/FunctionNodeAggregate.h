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

#ifndef PARSER_FUNCTION_NODE_AGGREGATE_H
#define PARSER_FUNCTION_NODE_AGGREGATE_H 1

#include "palo.h"

#include "Parser/FunctionNode.h"
#include "Parser/SourceNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node str
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeAggregate : public FunctionNode {
	enum AggregateFunction {
		Sum, Product, Min, Max, Count, First, Last, Average, And, Or, Unknown
	} aggregateFunction;

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeAggregate(name, params);
	}

public:
	FunctionNodeAggregate() :
		FunctionNode() {
	}
	;
	FunctionNodeAggregate(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
		if (name == "sum")
			aggregateFunction = Sum;
		else if (name == "product")
			aggregateFunction = Product;
		else if (name == "min")
			aggregateFunction = Min;
		else if (name == "max")
			aggregateFunction = Max;
		else if (name == "count")
			aggregateFunction = Count;
		else if (name == "first")
			aggregateFunction = First;
		else if (name == "last")
			aggregateFunction = Last;
		else if (name == "average")
			aggregateFunction = Average;
		else if (name == "and")
			aggregateFunction = And;
		else if (name == "or")
			aggregateFunction = Or;
		else
			aggregateFunction = Unknown;
	}
	;

public:

	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	Node * clone() {
		FunctionNodeAggregate * cloned = new FunctionNodeAggregate(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// add has two parameters
		if (!params || params->size() == 0) {
			error = "function '" + name + "' needs at least one parameter";
			return valid = false;
		}

		p.resize(params->size());
		for (size_t i = 0; i < params->size(); i++) {
			Node* p1 = params->at(i);

			// validate parameters
			if (!p1->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			if (p1->getValueType() != Node::NODE_NUMERIC && p1->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "first parameter of function '" + name + "' has wrong data type";
				return valid = false;
			}

			p[i] = p1;
		}
		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {

		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		double acc = 0;
		double count = 0;

		if (valid) {
			for (size_t pi = 0; pi < p.size(); pi++) {
				Node* p1 = p[pi];
				RuleValueType v1 = p1->getValue(cellPath, isCachable, mem_context);

				if (v1.type == Node::NODE_NUMERIC) {
					aggregateValue(v1.doubleValue, acc, count);
				}
			}
		}

		result.doubleValue = getResult(acc, count);
		return result;
	}
protected:
	std::vector<Node*> p;

	void aggregateValue(const double& value, double& acc, double& count) {
		count += 1;
		switch (aggregateFunction) {
		case Sum:
		case Average:
			acc += value;
			break;
		case Product:
			acc *= value;
			break;
		case Min:
			if (count == 1 || value < acc)
				acc = value;
			break;
		case Max:
			if (count == 1 || value > acc)
				acc = value;
			break;
		case First:
			if (count == 1)
				acc = value;
			break;
		case Last:
			acc = value;
			break;
		case Count:
			break;
		case And: //inverse logic
			if (value == 0)
				acc = 1.0;
			break;
		case Or:
			if (value != 0)
				acc = 1.0;
			break;
		case Unknown:
			acc = 0;
			break;
		}
	}

	double getResult(double& acc, double& count) {
		switch (aggregateFunction) {
		case Average:
			return count > 0 ? acc / count : 0;
		case Count:
			return count;
		case And: //inverse logic
			return 1.0 - acc;
		default:
			;
		}
		return acc;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		int32_t i;
		for (i = 0; i < (int32_t)params->size(); i++) {
			if (!params->at(i)->genCode(generator, Node::NODE_NUMERIC))
				return false;
		}
		if (!generator.EmitAggrCode(name, (uint8_t)params->size()))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

};

}
#endif
