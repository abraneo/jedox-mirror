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

#ifndef PARSER_DESTINATION_NODE_H
#define PARSER_DESTINATION_NODE_H 1

#include "palo.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

#include "Parser/AreaNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser destination node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DestinationNode : public AreaNode {

public:
	DestinationNode(RPVpsval *elements, RPVpival *elementsIds);

	virtual ~DestinationNode();

	Node * clone();

public:
	Node::ValueType getValueType();

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;
		return result;
	}

	bool hasElement(CPDimension, IdentifierType) const;

	bool isSimple() const {
		return true;
	}

	void appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const {
		AreaNode::appendRepresentation(sb, db, cube, false);
	}

	uint32_t guessType(uint32_t level);

	bool genCode(bytecode_generator& generator, uint8_t want) const;

	void appendXmlRepresentation(StringBuffer* sb, int ident, bool names);

	Area *getDestinationArea() {
		return nodeArea.get();
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* node, string& err) {
		return AreaNode::validate(server, database, cube, node, err);
	}

protected:
	bool validateNames(PServer, PDatabase, PCube, Node*, string&);

	bool validateIds(PServer, PDatabase, PCube, Node*, string&);
};
}
#endif
