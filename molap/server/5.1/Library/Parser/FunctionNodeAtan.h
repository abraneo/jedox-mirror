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

#ifndef PARSER_FUNCTION_NODE_ATAN_H
#define PARSER_FUNCTION_NODE_ATAN_H 1

#include "palo.h"

#include "math.h"

#include "Parser/FunctionNodeSimpleSingle.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node atan
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodeAtan : public FunctionNodeSimpleSingle {
public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodeAtan(name, params);
	}

public:
	FunctionNodeAtan() :
		FunctionNodeSimpleSingle() {
	}

	FunctionNodeAtan(const string& name, vector<Node*> *params) :
		FunctionNodeSimpleSingle(name, params) {
	}

	Node * clone() {
		FunctionNodeAtan * cloned = new FunctionNodeAtan(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->node = this->valid ? (*(cloned->params))[0] : 0;
		return cloned;
	}

public:
	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;

		if (valid) {
			result = node->getValue(cellPath, isCachable, mem_context);

			if (result.type == Node::NODE_NUMERIC) {
				result.doubleValue = atan(result.doubleValue);

				return result;
			}
		}

		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;

		return result;
	}

};

}

#endif
