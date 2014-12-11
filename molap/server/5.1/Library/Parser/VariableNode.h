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

#ifndef PARSER_VARIABLE_NODE_H
#define PARSER_VARIABLE_NODE_H 1

#include "palo.h"

#include <string>
#include "Parser/ExprNode.h"
#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser variable node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS VariableNode : public ExprNode {

public:
	VariableNode(string* value);

	~VariableNode();

	Node * clone();

public:
	bool isDimensionTransformation(CPCube cube, IdentifierType* dimension) {
		if (valid) {
			*dimension = this->dimensionId;
			return true;
		}

		return false;
	}

	map<Element*, string> computeDimensionTransformations(CPDatabase db, CPCube cube) {
		map<Element*, string> elements;

		if (dimensionId != NO_IDENTIFIER) {
			PDimension dim = db->lookupDimension(dimensionId, false);
			ElementsType dimensionElements = dim->getElements(PUser(), false);

			for (ElementsType::iterator iter = dimensionElements.begin(); iter != dimensionElements.end(); iter++) {
				Element* element = *iter;

				elements[element] = element->getName(dim->getElemNamesVector());
			}
		}

		return elements;
	}

	NodeType getNodeType() const {
		return NODE_VARIABLE_NODE;
	}

	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	string getStringValue() {
		return *value;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context);

	bool validate(PServer, PDatabase, PCube, Node*, string&);

	void appendXmlRepresentation(StringBuffer*, int, bool);

	void appendRepresentation(StringBuffer*, CPDatabase db, CPCube) const;

	uint32_t guessType(uint32_t level);

	bool genCode(bytecode_generator& generator, uint8_t want) const;

	int getDimensionOrdinal() const {
		return num;
	};

	IdentifierType getDimensionId() const {
		return dimensionId;
	};

private:
	string* value;
	int num;
	IdentifierType dimensionId;
};
}
#endif
