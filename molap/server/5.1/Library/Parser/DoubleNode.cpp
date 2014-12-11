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

#include "Parser/DoubleNode.h"

#include <sstream>

namespace palo {
DoubleNode::DoubleNode(double value) :
	value(value)
{
	valid = true;
}

DoubleNode::~DoubleNode()
{
}

Node * DoubleNode::clone()
{
	DoubleNode * cloned = new DoubleNode(value);
	cloned->valid = this->valid;
	return cloned;
}

void DoubleNode::appendXmlRepresentation(StringBuffer* sb, int ident, bool)
{
	identXML(sb, ident);
	sb->appendText("<double>");
	sb->appendDecimal(value);
	sb->appendText("</double>");
	sb->appendEol();
}

void DoubleNode::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const
{
	sb->appendDecimal(value);
}

uint32_t DoubleNode::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "DoubleNode " << " type " << "none" << endl;
	return Node::NODE_UNKNOWN_VALUE;
}

bool DoubleNode::genCode(bytecode_generator& generator, uint8_t want) const
{
	if (!generator.EmitLdConstDblCode(value))
		return false;
	return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
}

}
