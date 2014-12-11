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

#include "Parser/StringNode.h"

#include <sstream>

namespace palo {
StringNode::StringNode(string *value) :
	value(value)
{
	valid = true;
}

StringNode::~StringNode()
{
	delete value;
}

Node * StringNode::clone()
{
	StringNode * cloned = new StringNode(new string(*value));
	cloned->valid = this->valid;
	return cloned;
}

void StringNode::appendXmlRepresentation(StringBuffer* sb, int ident, bool)
{
	identXML(sb, ident);
	sb->appendText("<string>");
	sb->appendText(StringUtils::escapeXml(*value));
	sb->appendText("</string>");
	sb->appendEol();
}

void StringNode::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const
{
	sb->appendText(StringUtils::escapeString(*value));
}

uint32_t StringNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "StringNode " << " type " << "none" << endl;
	return Node::NODE_STRING;
}

bool StringNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	if (!generator.EmitLdConstStrCode(*value))
		return false;
	return generator.EmitForceTypeCode(Node::NODE_STRING, want);
}

}
