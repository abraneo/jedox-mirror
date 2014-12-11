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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_RULE_MARKER_H
#define OLAP_RULE_MARKER_H 1

#include "palo.h"

#include <iostream>

#include "Parser/AreaNode.h"
#include "Olap/Cube.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief palo rule marker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS RuleMarker {
	friend ostream& operator <<(ostream&, const RuleMarker&);

public:
	class MappingType : public multimap<uint32_t, uint32_t>
	{
		public:
			map<uint32_t, uint32_t>	&getReverseMap() {return reverseMap;}
		private:
			map<uint32_t, uint32_t> reverseMap;
	};

	typedef MappingType *PMappingType;

	RuleMarker(dbID_cubeID allDbCube, const Area* fromArea, const Area* toArea, IdentifierType ruleId);

	RuleMarker(dbID_cubeID fromDbCube, dbID_cubeID toDbCube, const vector<Node *>& path, const Area* toArea, IdentifierType ruleId);

	~RuleMarker();

	dbID_cubeID getFromDbCube() const;

	dbID_cubeID getToDbCube() const;

	CPArea getFromBase() const;

	CPArea getSourceArea() const {return sourceArea;}

	const int16_t *getPermutations() const;

	const uint32_t *getFixed() const;

	const PMappingType *getMapping() const;

	const bool isSourceCons() const;

	const bool isMultiplicating() const;

	const IdentifierType getId() const;

private:
	static IdHolder idHolder;

	size_t numberTargetDimensions;
	dbID_cubeID fromDbCube; //dbID, cubeID
	dbID_cubeID toDbCube;

	PArea fromBase;
	PArea sourceArea;

	int16_t *permutations;
	uint32_t *fixed;

	PMappingType *mapping;
	bool isCons;
	bool isMulti;

	IdentifierType id;
	IdentifierType ruleId;
};

ostream& operator <<(ostream& out, const RuleMarker& marker);

}

#endif
