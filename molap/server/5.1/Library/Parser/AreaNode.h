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

#ifndef PARSER_AREA_NODE_H
#define PARSER_AREA_NODE_H

#include "palo.h"

#include <iostream>

#include "Parser/Node.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser area node types
////////////////////////////////////////////////////////////////////////////////
typedef pair<int, pair<int, int> >			RPPival;
typedef vector< RPPival *>					RPVpival;
typedef pair<string*, pair<string*, int> >	RPPsval;
typedef vector< RPPsval *>					RPVpsval;

RPPival *RPPivalFromParam(int dimensionId, const string *functionName, int functionParam, string &errorMessage);
RPPsval *RPPsvalFromParam(string *dimensionName, const string *functionName, int functionParam, string &errorMessage);

////////////////////////////////////////////////////////////////////////////////
/// @brief base class for parser source or destination node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS AreaNode : virtual public Node {
public:
	AreaNode(RPVpsval *elements, RPVpival *elementsIds);

	virtual ~AreaNode();

public:
	bool canBeConsolidation(CPDatabase db) const;

	bool validate(PServer, PDatabase, PCube, Node*, string&);

	Area *getNodeArea() {
		return nodeArea.get();
	}

	const IdentifiersType* getElementIDs() const {
		return &elementIDs;
	}

	IdentifiersType* getElementIDs() {
		return &elementIDs;
	}

	const vector<uint8_t> *getRestriction() const {
		return &restriction;
	}

	vector<uint8_t> *getRestriction() {
		return &restriction;
	}

	static const uint8_t NO_RESTRICTION = 0;
	static const uint8_t ABS_RESTRICTION = 1;
	static const uint8_t OFFSET_RESTRICTION = 2;
protected:
	virtual bool validateNames(PServer, PDatabase, PCube, Node*, string&) = 0;

	virtual bool validateIds(PServer, PDatabase, PCube, Node*, string&) = 0;

protected:
	void appendRepresentation(StringBuffer*, CPDatabase db, CPCube, bool isMarker) const;

	bool validateNamesArea(PServer, PDatabase, PCube, Node*, string&);

	bool validateIdsArea(PServer, PDatabase, PCube, Node*, string&);

	bool isSimple() const {
		return true;
	}

	void appendXmlRepresentationType(StringBuffer*, int indent, bool names, const string& type);

protected:
	// first = name of dimension (can be empty)
	// second = name of dimension element (can be empty)
	// or:
	// first = id of dimension
	// second = id of dimension element (-1 for empty)

	RPVpsval *elements;
	RPVpival *elementsIds;

	IdentifiersType dimensionIDs;
	IdentifiersType elementIDs;
	vector<uint8_t> restriction;
	vector<bool> isQualified;
	vector<int> elementSequence;

	// true if the source node has an unrestricted dimension
	bool unrestrictedDimensions;

	PArea nodeArea;
};
}
#endif
