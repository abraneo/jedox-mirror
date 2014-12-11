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

#ifndef PARSER_FUNCTION_NODE_PALO_DATA_H
#define PARSER_FUNCTION_NODE_PALO_DATA_H 1

#include "palo.h"

#include <iostream>
#include <boost/scoped_array.hpp>

#include "Parser/FunctionNodePalo.h"
#include "Parser/VariableNode.h"
#include "Engine/LegacySource.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo data
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePaloData : public FunctionNodePalo {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		// printf("FunctionNodePaloData created\n");
		return new FunctionNodePaloData(name, params);
	}

public:
	FunctionNodePaloData() :
		FunctionNodePalo() {
	}

	FunctionNodePaloData(const string& name, vector<Node*> *params) :
		FunctionNodePalo(name, params) {
	}

	Node * clone() {
		FunctionNodePaloData * cloned = new FunctionNodePaloData(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->databaseid = this->databaseid;
		return cloned;
	}

public:
	NodeType getNodeType() const {
		return NODE_FUNCTION_PALO_DATA;
	}

	ValueType getValueType() {
		return Node::NODE_UNKNOWN_VALUE;
	}

#ifdef ENABLE_PLAN_FOR_PALO_DATA
	virtual bool isPlanCompatible(CPCubeArea area, Variability &varDimensions) const
	{
		bool result = false;
		if (params->size() > 2 && params->at(0)->isConstant() && params->at(1)->isConstant()) {
			RuleValueType dbName = params->at(0)->getValue(0, 0, 0);
			RuleValueType cubeName = params->at(1)->getValue(0, 0, 0);
			if (dbName.type == NODE_STRING && cubeName.type == NODE_STRING) {
				Context *context = Context::getContext();
				// lookup the cube and store it in context
				CPServer asrv = context->getServer();
				CPDatabase adb = area->getDatabase();
				if (!dbName.stringValue.empty()) {
					adb = asrv->lookupDatabaseByName(dbName.stringValue, false);
				}
				if (adb && adb->getIdentifier() == area->getDatabase()->getIdentifier()) {
					CPCube acube = area->getCube();
					if (!cubeName.stringValue.empty()) {
						acube = adb->lookupCubeByName(cubeName.stringValue, false);
					}
					if (acube && acube->getDimensions()->size() == params->size() - 2) {
						vector<uint32_t> dimensionMap;
//						bool sameCube = adb->getIdentifier() == area->getDatabase()->getIdentifier() && acube->getId() == area->getCube()->getId();
						int32_t i;
						for (i = (int32_t)params->size() - 1; i > 1; i--) {
							Node *param = params->at(i);
							if (param->getNodeType() == NODE_VARIABLE_NODE) {
								VariableNode *varNode = dynamic_cast<VariableNode*>(param);
								int dimensionOrdinal = varNode->getDimensionOrdinal();
								if (dimensionOrdinal >= 0 && acube->getDimensions()->at(i-2) == varNode->getDimensionId()) {
									// the same dimension
								} else {
									// different dimension
									// TODO: -jj- element mapping by names
									break;
								}
							} else if (param->getNodeType() == NODE_STRING_NODE) {
								CPDimension adim = adb->lookupDimension(acube->getDimensions()->at(i-2), false);
								RuleValueType elemName = param->getValue(0, 0, 0);
								Element *aelm = adim->lookupElementByName(elemName.stringValue, false);
								if (aelm) {
									// element found
								} else {
									// element not found
									break;
								}
							} else {
								// unsupported parameter type
								break;
							}
						}
						if (i == 1) {
//						if (i == 1 && sameCube) { // TODO: -jj- restricted to sameCube because of CPU engine
							// all parameters OK
							result = true;
						}
					}
				}
			}
		}
		if (!result) {
			varDimensions.fill();
		}
		return result;
	}

	virtual PPlanNode createPlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, class Planner *parentPlanner) const
	{
		PPlanNode result;
		constResult = 0;
		valid = false;

		if (params->size() > 2 && params->at(0)->isConstant() && params->at(1)->isConstant()) {
			RuleValueType dbName = params->at(0)->getValue(0, 0, 0);
			RuleValueType cubeName = params->at(1)->getValue(0, 0, 0);
			if (dbName.type == NODE_STRING && cubeName.type == NODE_STRING) {
				Context *context = Context::getContext();
				// lookup the cube and store it in context
				CPServer asrv = context->getServer();
				CPDatabase adb = area->getDatabase();
				if (!dbName.stringValue.empty()) {
					adb = asrv->lookupDatabaseByName(dbName.stringValue, false);
				}
				if (adb && adb->getIdentifier() == area->getDatabase()->getIdentifier()) {
					CPCube acube = area->getCube();
					if (!cubeName.stringValue.empty()) {
						acube = adb->lookupCubeByName(cubeName.stringValue, false);
					}
					if (acube && acube->getDimensions()->size() == params->size() - 2) {
						vector<uint32_t> dimensionMap;
						bool sameCube = adb->getIdentifier() == area->getDatabase()->getIdentifier() && acube->getId() == area->getCube()->getId();
						PCubeArea srcCubeArea(new CubeArea(adb, acube, acube->getDimensions()->size()));
						int32_t i;
						for (i = (int32_t)params->size() - 1; i > 1; i--) {
							Node *param = params->at(i);
							if (param->getNodeType() == NODE_VARIABLE_NODE) {
								VariableNode *varNode = dynamic_cast<VariableNode*>(param);
								int dimensionOrdinal = varNode->getDimensionOrdinal();
								if (dimensionOrdinal >= 0 && acube->getDimensions()->at(i-2) == varNode->getDimensionId()) {
									// the same dimension
									srcCubeArea->insert(size_t(i-2), area->getDim(dimensionOrdinal));
									if (!sameCube) {
										dimensionMap.push_back(uint32_t(i-2)); // source dimension ordinal
										dimensionMap.push_back(uint32_t(dimensionOrdinal)); // target dimension ordinal
									}
								} else {
									// different dimension
									// TODO: -jj- element mapping by names
									break;
								}
							} else if (param->getNodeType() == NODE_STRING_NODE) {
								CPDimension adim = adb->lookupDimension(acube->getDimensions()->at(i-2), false);
								RuleValueType elemName = param->getValue(0, 0, 0);
								Element *aelm = adim->lookupElementByName(elemName.stringValue, false);
								if (aelm) {
									PSet s(new Set());
									s->insert(aelm->getIdentifier());
									srcCubeArea->insert(size_t(i-2), s);
								} else {
									// element not found
									break;
								}
							} else {
								// unsupported parameter type
								break;
							}
						}
						if (i == 1) {
							// all parameters OK, Subcube ready
							if (rule->isCustom()) {
								CPCube cube = srcCubeArea->getCube();
								boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
								PUser user = session ? session->getUser() : PUser();
								CPDatabase database = srcCubeArea->getDatabase();
								try {
									if (User::checkUser(user)) {
										User::RightSetting rs(User::checkCellDataRightCube(database, cube));
										cube->checkAreaAccessRight(database, user, srcCubeArea, rs, false, RIGHT_READ, 0);
									}
								} catch (ErrorException &e) {
									CellValue c(e.getErrorType());
									return PPlanNode(new ConstantPlanNode(area, c));
								}
							} // area and srcCubeArea are from the same database therefore User::checkRuleDatabaseRight doesn't have to be called
							Planner planner(acube, srcCubeArea, parentPlanner);
							planner.setCurrentRule(rule);
							PPlanNode source = planner.createPlan(CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
							if (source) {
								TransformationPlanNode *tpn = new TransformationPlanNode(area, source, SetMultimaps(), 1.0, dimensionMap);
								result = PPlanNode(tpn);
								if (result) {
									tpn->setSourceCubeId(dbID_cubeID(adb->getId(), acube->getId()));
									valid = true;
								}
							}
						}
					}
				}
			}
		}

//		int32_t i;
//		int16_t contextCubeIndex = -1;
//		PCube acube;
//		boost::scoped_array<int16_t> spDimensionOrder;
//		int16_t *dimensionOrder = 0;
//		boost::scoped_array<uint32_t> spDimensionElements;
//		uint32_t *dimensionElements = 0;
//		size_t dimensions = params->size() - 2;
//		PDatabase adb;
//
//		if (params->size() > 2 && params->at(0)->isConstant() && params->at(1)->isConstant()) {
//			RuleValueType dbName = params->at(0)->getValue(0, 0, 0);
//			RuleValueType cubeName = params->at(1)->getValue(0, 0, 0);
//			if (dbName.type == NODE_STRING && cubeName.type == NODE_STRING) {
//
//				Context *context = Context::getContext();
//				// lookup the cube and store it in context
//				CPServer asrv = context->getServer();
//				adb = asrv->lookupDatabaseByName(dbName.stringValue, false);
//				if (adb && adb->getIdentifier() == generator.get_rule()->cube->database->getIdentifier()) {
//					acube = adb->lookupCubeByName(cubeName.stringValue, false);
//					if (acube) {
//						contextCubeIndex = context->savePaloDataCube(acube, adb);
//						dimensions  = acube->getDimensions()->size();
//						spDimensionOrder.reset(new int16_t[dimensions]);
//						dimensionOrder = spDimensionOrder.get();
//						for (size_t dim = 0; dim < dimensions; dim++) {
//							dimensionOrder[dim] = (int16_t) - 1;
//						}
//						spDimensionElements.reset(new uint32_t[dimensions]);
//						dimensionElements = spDimensionElements.get();
//						for (size_t dim = 0; dim < dimensions; dim++) {
//							dimensionElements[dim] = (uint32_t) - 1;
//						}
//					}
//				} else {
//					// different database
//				}
//			}
//		}
//		if (acube && acube->getDimensions()->size() != params->size() - 2) {
//			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "wrong number of path elements", "number of coordinates", (int)(params->size() - 2));
//		}
//		uint8_t parametersStored = 0;
//		for (i = (int32_t)params->size() - 1; 0 <= i; i--) {
//			if (acube) {
//				if (i > 1) { // cell coordinates
//					Node *param = params->at(i);
//					if (param->getNodeType() == NODE_VARIABLE_NODE) {
//						VariableNode *varNode = dynamic_cast<VariableNode*>(param);
//						int dimensionOrdinal = varNode->getDimensionOrdinal();
//						if (dimensionOrdinal >= 0 && (*acube->getDimensions())[i-2] == varNode->getDimensionId()) {
//							// dimension element - do not emit parameter code, store just the dimension ordinal
//							dimensionOrder[i-2] = (int16_t)dimensionOrdinal;
//							continue;
//						}
//					} else if (param->getNodeType() == NODE_STRING_NODE) {
//						CPDimension adim = adb->lookupDimension(acube->getDimensions()->at(i-2), false);
//						RuleValueType elemName = param->getValue(0, 0, 0);
//						Element *aelm = adim->lookupElementByName(elemName.stringValue, false);
//						if (aelm) {
//							dimensionElements[i-2] = (uint32_t)aelm->getIdentifier();
//							continue;
//						}
//					}
//				} else {
//					// cube and database name - do not save for known cube
//					continue;
//				}
//			}
//			if (!params->at(i)->genCode(generator, Node::NODE_STRING)) {
//				return false;
//			}
//			parametersStored++;
//		}
//		if (want == Node::NODE_NUMERIC) {
//			if (!generator.EmitCallDataDblCode(parametersStored, (uint8_t)dimensions, contextCubeIndex, dimensionOrder, dimensionElements)) {
//				return false;
//			}
//		} else {
//			if (!generator.EmitCallDataStrCode(parametersStored, (uint8_t)dimensions, contextCubeIndex, dimensionOrder, dimensionElements)) {
//				return false;
//			}
//		}
		return result;
	}
#endif

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {

		// has three parameters
		if (!params || params->size() < 3) {
			error = "function '" + name + "' needs more than two parameters";
			return valid = false;
		}

		for (size_t i = 0; i < params->size(); i++) {
			Node* param = params->at(i);
			/*
			 // if we have a string cube param diff. from current
			 // then swap it
			 if ( 1 == i && param->getNodeType() == Node::NODE_STRING_NODE)
			 {
			 std::string value = dynamic_cast<StringNode*>(param)->getStringValue();

			 if ( value != cube->getName() )
			 {
			 palo::PCube c = database->lookupCubeByName(value);
			 if ( c )
			 {
			 cube = c;
			 }
			 }
			 }
			 */
			// validate parameter
			if (!param->validate(server, database, cube, destination, error)) {
				return valid = false;
			}

			// check data type left
			if (param->getValueType() != Node::NODE_STRING && param->getValueType() != Node::NODE_UNKNOWN_VALUE) {
				error = "parameter of function '" + name + "' has wrong data type";
				return valid = false;
			}
		}

		return valid = true;
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
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

			IdentifiersType::const_iterator dim = dimensions->begin();
			vector<Node*>::iterator node = params->begin() + 2;
			size_t pathPos = 0;

			for (; dim != dimensions->end(); ++dim, ++node, ++pathPos) {
				Element* element = getElement(database->lookupDimension(*dim, false), *node, cellPath, isCachable, mem_context);

				if (!element) {
					RuleValueType value = (*node)->getValue(cellPath, isCachable, mem_context);

					if (value.type == Node::NODE_STRING) {
						throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "element of dimension not found", "element", value.stringValue);
					} else {
						throw ParameterException(ErrorException::ERROR_ELEMENT_NOT_FOUND, "decimal value as element name is not allowed", "element", "decimal value");
					}

				}
				PSet s(new Set);
				s->insert(element->getIdentifier());
				path->insert(pathPos, s);
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
		CPCube acube;
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

};

}
#endif
