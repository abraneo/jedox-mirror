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

#include "Parser/FunctionNodeComparison.h"

namespace palo {
bool FunctionNodeComparison::validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error)
{

	// add has two parameters
	if (!params || params->size() < 2) {
		error = "function '" + name + "' needs two parameters";
		return valid = false;
	}
	if (params->size() > 2) {
		error = "too many parameters for function '" + name + "'";
		return valid = false;
	}

	left = params->at(0);
	right = params->at(1);

	// validate parameters
	if (!left->validate(server, database, cube, destination, error) || !right->validate(server, database, cube, destination, error)) {
		return valid = false;
	}

	// check data type left
	if (left->getValueType() != Node::NODE_NUMERIC && left->getValueType() != Node::NODE_UNKNOWN_VALUE && left->getValueType() != Node::NODE_STRING) {
		error = "first parameter of function '" + name + "' has wrong data type";
		return valid = false;
	}

	// check data type right
	if (right->getValueType() != Node::NODE_NUMERIC && right->getValueType() != Node::NODE_UNKNOWN_VALUE && right->getValueType() != Node::NODE_STRING) {
		error = "second parameter of function '" + name + "' has wrong data type";
		return valid = false;
	}

	return valid = true;
}

void FunctionNodeComparison::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const
{
	if (infix) {
		if (!left->isSimple()) {
			sb->appendChar('(');
		}

		left->appendRepresentation(sb, db, cube);

		if (!left->isSimple()) {
			sb->appendChar(')');
		}

		sb->appendChar(' ');
		sb->appendText(name);
		sb->appendChar(' ');

		if (!right->isSimple()) {
			sb->appendChar('(');
		}

		right->appendRepresentation(sb, db, cube);

		if (!right->isSimple()) {
			sb->appendChar(')');
		}
	} else {
		sb->appendText(name);
		sb->appendChar('(');
		left->appendRepresentation(sb, db, cube);
		sb->appendChar(',');
		right->appendRepresentation(sb, db, cube);
		sb->appendChar(')');
	}
}

uint32_t FunctionNodeComparison::guessType(uint32_t level)
{
	Logger::trace << "guessType " << "level " << level << "node " << "FunctionNodeComparison " << name << " type " << "none" << endl;
	left->guessType(level + 1);
	right->guessType(level + 1);
	return Node::NODE_NUMERIC;
}

bool FunctionNodeComparison::genCode(bytecode_generator& generator, uint8_t want) const
{
	if (left->getValueType() == Node::NODE_NUMERIC && right->getValueType() == Node::NODE_NUMERIC) {
		if (!left->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!right->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!generator.EmitOp2ComparisonDblCode(name))
			return false;
	} else {
		if (!left->genCode(generator, Node::NODE_STRING))
			return false;
		if (!right->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitOp2ComparisonStrCode(name))
			return false;
	}

	return generator.EmitForceTypeCode(Node::NODE_NUMERIC, want);
}

}
