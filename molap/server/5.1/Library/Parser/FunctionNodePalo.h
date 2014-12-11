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

#ifndef PARSER_FUNCTION_NODE_PALO_H
#define PARSER_FUNCTION_NODE_PALO_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNode.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePalo : public FunctionNode {

public:
	FunctionNodePalo() :
		FunctionNode() {
		databaseid = -1;
	}

	FunctionNodePalo(const string& name, vector<Node*> *params) :
		FunctionNode(name, params) {
		databaseid = -1;
	}

public:
	bool validateParameter(PServer server, PDatabase database, PCube cube, Node* destination, string& error, int numStrings, int numNumeric) {
		if (!params || params->size() < (size_t)(numStrings + numNumeric)) {
			error = "function '" + name + "' needs more parameters";
			return valid = false;
		} else if (params->size() > (size_t)(numStrings + numNumeric)) {
			error = "too many parameters in function '" + name + "'";
			return valid = false;
		}

		for (size_t i = 0; i < params->size(); i++) {
			Node* param = params->at(i);

			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			// check data type
			if (i >= (size_t)numStrings) {
				if (param->getValueType() != Node::NODE_NUMERIC && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
					error = "parameter of function '" + name + "' has wrong data type";
					return valid = false;
				}
			} else {
				if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
					error = "parameter of function '" + name + "' has wrong data type";
					return valid = false;
				}
			}
		}

		databaseid = database->getId();

		return valid = true;
	}

	PDatabase getDatabase(PServer server, Node* stringNode, const CubeArea *cellPath, bool* isCachable, RulesContext* mem_context) {
		if (server) {
			// we assume that the string node is valid
			RuleValueType value = stringNode->getValue(cellPath, isCachable, mem_context);
			if (value.type == Node::NODE_STRING) {
				PDatabase d = server->lookupDatabaseByName(value.stringValue, false);

				// check for other database
				if (d && databaseid != d->getId()) {
					*isCachable = false;
				}

				return d;
			}
		}
		return PDatabase();
	}

	PCube getCube(PDatabase database, Node* stringNode, const CubeArea *cellPath, bool* isCachable, RulesContext* mem_context) {
		if (database) {
			// we assume that the string node is valid
			RuleValueType value = stringNode->getValue(cellPath, isCachable, mem_context);
			if (value.type == Node::NODE_STRING) {
				return database->findCubeByName(value.stringValue, PUser(), true, false);
			}
		}
		return PCube();
	}

	PDimension getDimension(PDatabase database, Node* stringNode, const CubeArea *cellPath, bool* isCachable, RulesContext* mem_context) {
		if (database) {
			// we assume that the string node is valid
			RuleValueType value = stringNode->getValue(cellPath, isCachable, mem_context);
			if (value.type == Node::NODE_STRING) {
				return database->findDimensionByName(value.stringValue, PUser(), false);
			}
		}
		return PDimension();
	}

	Element* getElement(CPDimension dimension, Node* stringNode, const CubeArea *cellPath, bool* isCachable, RulesContext* mem_context) {
		if (dimension) {
			// we assume that the string node is valid
			RuleValueType value = stringNode->getValue(cellPath, isCachable, mem_context);
			if (value.type == Node::NODE_STRING) {
				return dimension->findElementByName(value.stringValue, 0, false);
			}
		}
		return 0;
	}

protected:
	IdentifierType databaseid;

};

}
#endif
