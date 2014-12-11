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

#include "Parser/VariableNode.h"

#include "Collections/StringUtils.h"

namespace palo {
VariableNode::VariableNode(string *value) :
	value(value), dimensionId(NO_IDENTIFIER)
{
	valid = false;
	num = -1;
}

VariableNode::~VariableNode()
{
	delete value;
}

Node * VariableNode::clone()
{
	VariableNode * cloned = new VariableNode(new string(*value));
	cloned->valid = this->valid;
	cloned->num = this->num;
	cloned->dimensionId = this->dimensionId;
	return cloned;
}

bool VariableNode::validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error)
{
	if (database == 0 || cube == 0) {
		valid = true;
		return valid;
	}

	CPDimension dimension = database->lookupDimensionByName(*value, false);

	if (!dimension) {
		error = "dimension '" + *value + "' not found";
		valid = false;
		return valid;
	}
	dimensionId = dimension->getId();

	const IdentifiersType *dimensions = cube->getDimensions();
	int pos = 0;

	for (IdentifiersType::const_iterator i = dimensions->begin(); i != dimensions->end(); ++i, ++pos) {
		if (*i == dimensionId) {
			num = pos;
			valid = true;
			return valid;
		}
	}

	error = "dimension '" + *value + "' is not member of cube '" + cube->getName() + "'";
	valid = false;
	return valid;
}

Node::RuleValueType VariableNode::getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context)
{
	RuleValueType result;
	result.type = Node::NODE_STRING;

	if (num >= 0) {
		CPCube cube = cellPath->getCube();
		CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));
		const IdentifiersType* dims = cube->getDimensions();
		CPDimension dim = db->lookupDimension(dims->at(num), false);
		Element* element = dim->lookupElement((*cellPath->pathBegin())[num], false);
		result.stringValue = element->getName(dim->getElemNamesVector());
	} else {
		result.stringValue = "";
	}

	return result;
}

void VariableNode::appendXmlRepresentation(StringBuffer* sb, int ident, bool)
{
	identXML(sb, ident);
	sb->appendText("<variable>");
	sb->appendText(StringUtils::escapeXml(*value));
	sb->appendText("</variable>");
	sb->appendEol();
}

void VariableNode::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const
{
	sb->appendChar('!');
	sb->appendText(StringUtils::escapeStringSingle(*value));
}

uint32_t VariableNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "VariableNode " << " type " << "none" << endl;
	return Node::NODE_UNKNOWN_VALUE;
}

bool VariableNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	if (!generator.EmitVariableStrCode(num))
		return false;
	return generator.EmitForceTypeCode(Node::NODE_STRING, want);
}

}
