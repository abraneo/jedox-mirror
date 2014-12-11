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

#ifndef PARSER_FUNCTION_NODE_DATE_H
#define PARSER_FUNCTION_NODE_DATE_H 1

#include "palo.h"

#include <math.h>

#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node date
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeDate : public FunctionNode {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeDate(name, params);
	}

public:
	FunctionNodeDate() :
		FunctionNode() {
	}

	FunctionNodeDate(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
	}

	Node * clone() {
		FunctionNodeDate * cloned = new FunctionNodeDate(name, cloneParameters());
		cloned->valid = this->valid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_NUMERIC;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has three parameters
		if (!params || params->size() < 3) {
			error = "function '" + name + "' needs three parameters";
			return valid = false;
		}
		if (params->size() > 3) {
			error = "too many parameters for function '" + name + "'";
			return valid = false;
		}

		for (int i = 0; i < 3; i++) {
			Node* param = params->at(i);
			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}
			// check data type
			if (param->getValueType() != Node::NODE_NUMERIC && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "a parameter of function '" + name + "' has a wrong data type";
				return valid = false;
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;

		if (valid) {
			RuleValueType year = params->at(0)->getValue(cellPath, isCachable, mem_context);
			RuleValueType month = params->at(1)->getValue(cellPath, isCachable, mem_context);
			RuleValueType day = params->at(2)->getValue(cellPath, isCachable, mem_context);

			if (year.type == Node::NODE_NUMERIC && month.type == Node::NODE_NUMERIC && day.type == Node::NODE_NUMERIC) {

				struct tm t;
				t.tm_sec = 0;
				t.tm_min = 0;
				t.tm_hour = 12;
				t.tm_wday = 0;
				t.tm_yday = 0;
				t.tm_isdst = -1;
				t.tm_year = (int)year.doubleValue - 1900;
				t.tm_mon = (int)month.doubleValue - 1;
				t.tm_mday = (int)day.doubleValue;

				result.doubleValue = (double)(mktime(&t) / 86400 * 86400);
				return result;
			}
		}

		result.doubleValue = 0.0;
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(0)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!generator.EmitFuncCode(name))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
	}

};

}
#endif
