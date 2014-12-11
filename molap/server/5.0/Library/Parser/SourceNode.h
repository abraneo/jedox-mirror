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

#ifndef PARSER_SOURCE_NODE_H
#define PARSER_SOURCE_NODE_H

#include "palo.h"

#include <string>
#include <vector>
#include <iostream>

#include "Parser/AreaNode.h"
#include "Parser/ExprNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser source node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SourceNode : public AreaNode, public ExprNode {
public:
	SourceNode(RPVpsval *elements, RPVpival *elementsIds);

	SourceNode(RPVpsval *elements, RPVpival *elementsIds, bool marker);

	virtual ~SourceNode();

	Node * clone();

public:
	NodeType getNodeType() const {
		return NODE_SOURCE_NODE;
	}

	Node::ValueType getValueType();

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context);

	bool hasElement(CPDimension, IdentifierType) const;

	bool isSimple() const {
		return true;
	}

	bool isMarker() const {
		return marker;
	}

	void appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const {
		AreaNode::appendRepresentation(sb, db, cube, marker);
	}

	uint32_t guessType(uint32_t level);

	bool genCode(bytecode_generator& generator, uint8_t want) const;

	void appendXmlRepresentation(StringBuffer*, int, bool);

	Area* getSourceArea() {
		return nodeArea.get();
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* node, string& err) {
		return AreaNode::validate(server, database, cube, node, err);
	}

	void collectMarkers(vector<Node*>& markers) {
		if (marker) {
			markers.push_back(this);
		}
	}

protected:
	bool validateNames(PServer, PDatabase, PCube, Node*, string&);

	bool validateIds(PServer, PDatabase, PCube, Node*, string&);

private:
	IdentifierType databaseid;
	IdentifierType cubeid;
	PCubeArea fixedCellPath;
	bool marker;
};
}
#endif

