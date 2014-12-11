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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PARSER_FUNCTION_NODE_SIMPLE_H
#define PARSER_FUNCTION_NODE_SIMPLE_H 1

#include "palo.h"

#include <string>
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
	Node* left;
	Node* right;
	bool infix;
};
}
#endif
