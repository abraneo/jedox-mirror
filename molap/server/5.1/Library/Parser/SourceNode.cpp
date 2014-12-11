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

#include "Parser/SourceNode.h"
#include "Engine/LegacySource.h"

namespace palo {
SourceNode::SourceNode(RPVpsval *elements, RPVpival *elementsIds) :
	AreaNode(elements, elementsIds)
{
	marker = false;
}

SourceNode::SourceNode(RPVpsval *elements, RPVpival *elementsIds, bool marker) :
	AreaNode(elements, elementsIds), marker(marker)
{
}

SourceNode::~SourceNode()
{
}

Node *SourceNode::clone()
{
	RPVpsval *clonedElements = 0;
	RPVpival *clonedElementsIds = 0;

	// create a copy of the elements and elements identifiers
	if (elements != 0) {
		clonedElements = new RPVpsval;

		for (RPVpsval::iterator i = elements->begin(); i != elements->end(); ++i) {
			if (*i != 0) {
				string *newDim = (*i)->first == 0 ? 0 : new string(*((*i)->first));
				string *newElem = (*i)->second.first == 0 ? 0 : new string(*((*i)->second.first));
				RPPsval *p = new RPPsval(newDim, make_pair(newElem, (*i)->second.second));

				clonedElements->push_back(p);
			} else {
				clonedElements->push_back((RPPsval*)0);
			}
		}
	}

	if (elementsIds != 0) {
		clonedElementsIds = new RPVpival;

		for (RPVpival::iterator i = elementsIds->begin(); i != elementsIds->end(); ++i) {
			if (*i != 0) {
				RPPival *p = new RPPival((*i)->first, (*i)->second);

				clonedElementsIds->push_back(p);
			} else {
				clonedElementsIds->push_back((RPPival*)0);
			}
		}
	}

	SourceNode * cloned = new SourceNode(clonedElements, clonedElementsIds);

	cloned->dimensionIDs = this->dimensionIDs;
	cloned->elementIDs = this->elementIDs;
	cloned->restriction = this->restriction;
	cloned->isQualified = this->isQualified;
	cloned->elementSequence = this->elementSequence;
	cloned->unrestrictedDimensions = this->unrestrictedDimensions;

	cloned->nodeArea.reset(new Area(*nodeArea));

	cloned->databaseid = this->databaseid;
	cloned->cubeid = this->cubeid;

	if (this->fixedCellPath) {
		cloned->fixedCellPath = PCubeArea(new CubeArea(*this->fixedCellPath));
	}

	cloned->marker = this->marker;

	return cloned;
}

SourceNode::ValueType SourceNode::getValueType()
{
	return Node::NODE_UNKNOWN_VALUE;
}

bool SourceNode::validateNames(PServer server, PDatabase database, PCube cube, Node* source, string& error)
{
	this->databaseid = database->getId();
	this->cubeid = cube->getId();

	bool result = validateNamesArea(server, database, cube, source, error);

	if (!result) {
		return false;
	}

	if (!unrestrictedDimensions) {
		// all dimensions are restricted, so we can build the CellPath here
		fixedCellPath.reset(new CubeArea(database, cube, elementIDs));
	}

	return true;
}

bool SourceNode::validateIds(PServer server, PDatabase database, PCube cube, Node* source, string& error)
{
	this->databaseid = database->getId();
	this->cubeid = cube->getId();

	bool result = validateIdsArea(server, database, cube, source, error);

	if (!result) {
		return false;
	}

	if (!unrestrictedDimensions) {
		// all dimensions are restricted, so we can build the CellPath here
		fixedCellPath.reset(new CubeArea(database, cube, elementIDs));
	}

	return true;
}

Node::RuleValueType SourceNode::getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context)
{
	RuleValueType result;
	PServer server = Context::getContext()->getServer();
	CPDatabase database = server->lookupDatabase(databaseid, false);
	PCube cube = database->lookupCube(cubeid, false);
	PCubeArea cp;

	if (unrestrictedDimensions) {
		// compute source CellPath
		IdentifiersType elements(restriction.size());

		Area::PathIterator requestElements = cellPath->pathBegin();
		IdentifiersType::const_iterator source = elementIDs.begin();
		IdentifiersType::const_iterator request = (*requestElements).begin();
		vector<uint8_t>::const_iterator i = restriction.begin();
		IdentifiersType::iterator eIter = elements.begin();

		for (; i != restriction.end(); ++i, ++source, ++request, ++eIter) {
			if (*i == ABS_RESTRICTION) {
				*eIter = *source;
			} else if (*i == OFFSET_RESTRICTION) {
				// Todo: -jj- no name specified - use relative offset
				throw ErrorException(ErrorException::ERROR_INTERNAL, "source with offset not yet implemented!");;
			} else {
				*eIter = *request;
			}
		}
		cp.reset(new CubeArea(database, cube, elements));
	} else {
		cp = fixedCellPath;
	}

	CellValue value;
	PCellStream cs = cube->calculateArea(cp, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
	if (cs->next()) {
		value = cs->getValue();
	}
	if (!value.isEmpty()) {
		if (value.isString()) {
			result.type = Node::NODE_STRING;
			result.stringValue = value;
		} else {
			result.type = Node::NODE_NUMERIC;
			result.doubleValue = value.getNumeric();
		}
	} else {
		if (value.isString()) {
			result.type = Node::NODE_STRING;
			result.stringValue = "";
		} else {
			result.type = Node::NODE_NUMERIC;
			result.doubleValue = 0.0;
		}
	}

	return result;
}

bool SourceNode::hasElement(CPDimension dimension, IdentifierType element) const
{
	IdentifiersType::const_iterator dIter = dimensionIDs.begin();
	IdentifiersType::const_iterator eIter = elementIDs.begin();
	vector<uint8_t>::const_iterator rIter = restriction.begin();

	for (; dIter != dimensionIDs.end() && eIter != elementIDs.end() && rIter != restriction.end(); ++dIter, ++eIter, ++rIter) {
		if (dimension->getId() == *dIter && (element == ALL_IDENTIFIERS || element == *eIter) && *rIter == ABS_RESTRICTION) {
			return true;
		}
	}

	return false;
}

void SourceNode::appendXmlRepresentation(StringBuffer* sb, int indent, bool names)
{
	if (marker) {
		appendXmlRepresentationType(sb, indent, names, "marker");
	} else {
		appendXmlRepresentationType(sb, indent, names, "source");
	}
}

uint32_t SourceNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "SourceNode " << " type " << "none" << endl;
	return Node::NODE_UNKNOWN_VALUE;
}

bool SourceNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	return generator.EmitSourceCode(elementIDs, restriction, marker, want);
}

}
