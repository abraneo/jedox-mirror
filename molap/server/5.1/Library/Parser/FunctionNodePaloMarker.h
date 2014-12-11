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

#ifndef PARSER_FUNCTION_NODE_PALO_MARKER_H
#define PARSER_FUNCTION_NODE_PALO_MARKER_H 1

#include "palo.h"

#include <string>
#include <boost/scoped_array.hpp>

#include "Parser/FunctionNodePalo.h"
#include "Parser/StringNode.h"
#include "Parser/VariableNode.h"
#include "Engine/LegacySource.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo marker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePaloMarker : public FunctionNodePalo {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodePaloMarker(name, params);
	}

public:
	FunctionNodePaloMarker() :
		FunctionNodePalo() {
	}

	FunctionNodePaloMarker(const string& name, vector<Node*> *params) :
		FunctionNodePalo(name, params) {
	}

	Node * clone() {
		FunctionNodePaloMarker * cloned = new FunctionNodePaloMarker(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->databaseid = this->databaseid;
		return cloned;
	}

public:
	NodeType getNodeType() const {
		return NODE_FUNCTION_PALO_MARKER;
	}

	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

	const string& getDatabaseName() const {
		return databaseName;
	}

	const string& getCubeName() const {
		return cubeName;
	}

	const vector<Node*>& getPath() const {
		return path;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has three parameters
		if (!params || params->size() < 3) {
			error = "function '" + name + "' needs more than two parameters";
			return valid = false;
		}

		for (size_t i = 0; i < params->size(); i++) {
			Node* param = params->at(i);

			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			// check data type left
			if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "parameter of function '" + name + "' has wrong data type";
				return valid = false;
			}

			// check paramater type, should be constant string or variable
			if (i < 2) {
				if (param->getNodeType() != Node::NODE_STRING_NODE) {
					error = "database or cube paramater of function '" + name + "' must be a constant";
					return valid = false;
				}

				StringNode * s = dynamic_cast<StringNode*>(param);

				if (i == 0) {
					databaseName = s->getStringValue();
				} else if (i == 1) {
					cubeName = s->getStringValue();
				}
			} else {
				if (param->getNodeType() != Node::NODE_STRING_NODE && param->getNodeType() != Node::NODE_VARIABLE_NODE) {
					error = "parameter of function '" + name + "' must be a constant or a variable";
					return valid = false;
				}

				path.push_back(param);
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea *cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;

		if (valid) {
			PServer server = Context::getContext()->getServer();
			PDatabase database = getDatabase(server, params->at(0), cellPath, isCachable, mem_context);

			if (database == 0) {
				throw ParameterException(ErrorException::ERROR_DATABASE_NOT_FOUND, "database unknown", "database", params->at(0)->getValue(cellPath, isCachable, mem_context).stringValue);
			}

			PCube cube = getCube(database, params->at(1), cellPath, isCachable, mem_context);

			if (cube == 0) {
				throw ParameterException(ErrorException::ERROR_CUBE_NOT_FOUND, "cube unknown", "database", params->at(1)->getValue(cellPath, isCachable, mem_context).stringValue);
			}

			const IdentifiersType* dimensions = cube->getDimensions();

			if (dimensions->size() != params->size() - 2) {
				throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", "number of coordinates", (int)(params->size() - 2));
			}

			PArea path(new Area(dimensions->size()));

			vector<Node*>::iterator node = params->begin() + 2;
			for (size_t dim = 0; dim < dimensions->size(); dim++, node++) {
				Element* element = getElement(database->lookupDimension(dimensions->at(dim), false), *node, cellPath, isCachable, mem_context);

				if (!element) {
					RuleValueType value = (*node)->getValue(cellPath, isCachable, mem_context);
					string name = "decimal value";

					if (value.type == Node::NODE_STRING) {
						name = value.stringValue;
					}

					throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element of dimension not found", "element", name);
				}

				PSet s(new Set);
				s->insert(element->getIdentifier());
				path->insert((IdentifierType)dim, s);
			}

			CellValue value;
			PCubeArea calcArea(new CubeArea(database, CONST_COMMITABLE_CAST(Cube, cube), *path));
			PCellStream cs = cube->calculateArea(calcArea, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), false, UNLIMITED_UNSORTED_PLAN);
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

		result.type = Node::NODE_NUMERIC;
		result.doubleValue = 0.0;

		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		int32_t i;
		int16_t contextCubeIndex = -1;
		PCube acube;
		boost::scoped_array<int16_t> spDimensionOrder;
		int16_t *dimensionOrder = 0;
		boost::scoped_array<uint32_t> spDimensionElements;
		uint32_t *dimensionElements = 0;
		size_t dimensions = params->size() - 2;
		CPDatabase adb;

		if (params->size() > 2 && params->at(0)->isConstant() && params->at(1)->isConstant()) {
			RuleValueType dbName = params->at(0)->getValue(0, 0, 0);
			RuleValueType cubeName = params->at(1)->getValue(0, 0, 0);
			if (dbName.type == NODE_STRING && cubeName.type == NODE_STRING) {
				Context *context = Context::getContext();
				// lookup the cube and store it in context
				CPServer asrv = context->getServer();
				if (dbName.stringValue.empty()) {
					adb = CONST_COMMITABLE_CAST(Database, generator.m_rule.cube->database->shared_from_this());
				} else {
					adb = asrv->lookupDatabaseByName(dbName.stringValue, false);
				}
				if (adb && adb->getIdentifier() == generator.get_rule()->cube->database->getIdentifier()) {
					if (cubeName.stringValue.empty()) {
						cubeName.stringValue = generator.m_rule.cube->acube->getName();
					}
					acube = adb->lookupCubeByName(cubeName.stringValue, false);
					if (acube) {
						contextCubeIndex = context->savePaloDataCube(acube, adb);
						dimensions  = acube->getDimensions()->size();
						spDimensionOrder.reset(new int16_t[dimensions]);
						dimensionOrder = spDimensionOrder.get();
						for (size_t dim = 0; dim < dimensions; dim++) {
							dimensionOrder[dim] = (int16_t) - 1;
						}
						spDimensionElements.reset(new uint32_t[dimensions]);
						dimensionElements = spDimensionElements.get();
						for (size_t dim = 0; dim < dimensions; dim++) {
							dimensionElements[dim] = (uint32_t) - 1;
						}
					}
				} else {
					// different database
				}
			}
		}
		if (acube && acube->getDimensions()->size() != params->size() - 2) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", "number of coordinates", (int)(params->size() - 2));
		}
		uint8_t parametersStored = 0;
		for (i = (int32_t)params->size() - 1; 0 <= i; i--) {
			if (acube) {
				if (i > 1) { // cell coordinates
					Node *param = params->at(i);
					if (param->getNodeType() == NODE_VARIABLE_NODE) {
						VariableNode *varNode = dynamic_cast<VariableNode*>(param);
						int dimensionOrdinal = varNode->getDimensionOrdinal();
						if (dimensionOrdinal >= 0 && (*acube->getDimensions())[i-2] == varNode->getDimensionId()) {
							// dimension element - do not emit parameter code, store just the dimension ordinal
							dimensionOrder[i-2] = (int16_t)dimensionOrdinal;
							continue;
						}
					} else if (param->getNodeType() == NODE_STRING_NODE) {
						CPDimension adim = adb->lookupDimension(acube->getDimensions()->at(i-2), false);
						RuleValueType elemName = param->getValue(0, 0, 0);
						Element *aelm = adim->lookupElementByName(elemName.stringValue, false);
						if (aelm) {
							dimensionElements[i-2] = (uint32_t)aelm->getIdentifier();
							continue;
						}
					}
				} else {
					// cube and database name - do not save for known cube
					continue;
				}
			}
			if (!params->at(i)->genCode(generator, Node::NODE_STRING)) {
				return false;
			}
			parametersStored++;
		}
		if (want == Node::NODE_NUMERIC) {
			if (!generator.EmitCallDataDblCode(parametersStored, (uint8_t)dimensions, contextCubeIndex, dimensionOrder, dimensionElements)) {
				return false;
			}
		} else {
			if (!generator.EmitCallDataStrCode(parametersStored, (uint8_t)dimensions, contextCubeIndex, dimensionOrder, dimensionElements)) {
				return false;
			}
		}

		return true;
	}

	void collectMarkers(vector<Node*>& markers) {
		markers.push_back(this);
	}

private:
	string databaseName;
	string cubeName;
	vector<Node*> path;
};

}
#endif
