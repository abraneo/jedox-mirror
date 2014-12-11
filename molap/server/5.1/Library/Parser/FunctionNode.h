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

#ifndef PARSER_FUNCTION_NODE_H
#define PARSER_FUNCTION_NODE_H 1

#include "palo.h"

#include "Parser/ExprNode.h"
#include "Parser/Node.h"

#include "Olap/Server.h"
#include "Olap/Database.h"
#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNode : public ExprNode {
public:
	typedef FunctionNode *(*CreateFunc_ptr)(const string& name, vector<Node*> *params);

public:
	FunctionNode();

	FunctionNode(const string& name, vector<Node*> *params);

	virtual ~FunctionNode();

public:
	ValueType getValueType();

	bool validate(PServer, PDatabase, PCube, Node*, string&) = 0;

	bool hasElement(CPDimension, IdentifierType) const;

	bool isSimple() const {
		return true;
	}

	void appendXmlRepresentation(StringBuffer*, int, bool);

	void appendRepresentation(StringBuffer*, CPDatabase db, CPCube) const;

	uint32_t guessType(uint32_t level);

	vector<Node*> * getParameters() {
		return params;
	}

	void collectMarkers(vector<Node*>& markers) {
		for (vector<Node*>::iterator i = params->begin(); i != params->end(); i++) {
			(*i)->collectMarkers(markers);
		}
	}

protected:
	vector<Node*> * cloneParameters();

protected:
	string name;
	vector<Node*> * params;
};

}
#endif
