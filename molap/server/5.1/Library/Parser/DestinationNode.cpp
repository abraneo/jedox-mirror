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

#include "Parser/DestinationNode.h"
#include "Engine/EngineBase.h"

namespace palo {
DestinationNode::DestinationNode(RPVpsval *elements, RPVpival *elementsIds) :
	AreaNode(elements, elementsIds)
{
}

DestinationNode::~DestinationNode()
{
}

Node * DestinationNode::clone()
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

	DestinationNode * cloned = new DestinationNode(clonedElements, clonedElementsIds);

	cloned->dimensionIDs = this->dimensionIDs;
	cloned->elementIDs = this->elementIDs;
	cloned->restriction = this->restriction;
	cloned->isQualified = this->isQualified;
	cloned->elementSequence = this->elementSequence;
	cloned->unrestrictedDimensions = this->unrestrictedDimensions;

	cloned->nodeArea.reset(new Area(*nodeArea));
	return cloned;
}

DestinationNode::ValueType DestinationNode::getValueType()
{
	return Node::NODE_UNKNOWN_VALUE;
}

bool DestinationNode::validateNames(PServer server, PDatabase database, PCube cube, Node* destination, string& error)
{
	return validateNamesArea(server, database, cube, destination, error);
}

bool DestinationNode::validateIds(PServer server, PDatabase database, PCube cube, Node* destination, string& error)
{
	return validateIdsArea(server, database, cube, destination, error);
}

bool DestinationNode::hasElement(CPDimension dimension, IdentifierType element) const
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

void DestinationNode::appendXmlRepresentation(StringBuffer* sb, int indent, bool names)
{
	appendXmlRepresentationType(sb, indent, names, "destination");
}

uint32_t DestinationNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "DestinationNode " << " type " << "none" << endl;
	return Node::NODE_UNKNOWN_VALUE;
}

bool DestinationNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	return generator.EmitNopCode();
}

}
