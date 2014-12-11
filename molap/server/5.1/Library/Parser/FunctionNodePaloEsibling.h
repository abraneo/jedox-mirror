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

#ifndef PARSER_FUNCTION_NODE_PALO_ESIBLING_H
#define PARSER_FUNCTION_NODE_PALO_ESIBLING_H 1

#include "palo.h"

#include <string>
#include "Parser/FunctionNodePalo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser function node palo esibling
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FunctionNodePaloEsibling : public FunctionNodePalo {

public:
	static FunctionNode* createNode(const string& name, vector<Node*> *params) {
		return new FunctionNodePaloEsibling(name, params);
	}

public:
	FunctionNodePaloEsibling() :
		FunctionNodePalo() {
	}

	FunctionNodePaloEsibling(const string& name, vector<Node*> *params) :
		FunctionNodePalo(name, params) {
	}

	Node * clone() {
		FunctionNodePaloEsibling * cloned = new FunctionNodePaloEsibling(name, cloneParameters());
		cloned->valid = this->valid;
		cloned->databaseid = this->databaseid;
		return cloned;
	}

public:
	ValueType getValueType() {
		return Node::NODE_STRING;
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* destination, string& error) {
		return validateParameter(server, database, cube, destination, error, 3, 1);
	}

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context) {
		RuleValueType result;
		result.type = Node::NODE_STRING;

		if (valid) {
			PDatabase database = getDatabase(Context::getContext()->getServer(), params->at(0), cellPath, isCachable, mem_context);
			PDimension dimension = getDimension(database, params->at(1), cellPath, isCachable, mem_context);
			Element* element = getElement(dimension, params->at(2), cellPath, isCachable, mem_context);
			RuleValueType num = params->at(3)->getValue(cellPath, isCachable, mem_context);

			if (element && num.type == Node::NODE_NUMERIC) {
				int32_t offset = (int32_t)num.doubleValue;

				// return element name for num == 0
				if (offset == 0) {
					result.stringValue = element->getName(dimension->getElemNamesVector());
					return result;
				}

				bool bRoot = true;
				IdentifiersWeightType rootElems;
				const IdentifiersWeightType *childrenList = &rootElems;

				// get parents
				CPParents parentsList =  element->getParents();
				if (parentsList->size() > 0) {
					// get first parent
					Element* parent = dimension->lookupElement(parentsList->at(0), false);
					childrenList = parent->getChildren();
					bRoot = false;
				} else {
					PSet tops = dimension->getElemIds(CubeArea::TOP_ELEMENTS);
					rootElems.reserve(tops->size());
					for (Set::Iterator it = tops->begin(); it != tops->end(); ++it) {
						rootElems.push_back(make_pair(*it, 1.0));
					}
				}

				ssize_t childPos = 0;

				for (IdentifiersWeightType::const_iterator iter = childrenList->begin(); iter != childrenList->end(); childPos++, iter++) {
					if ((*iter).first == element->getIdentifier()) {
						break;
					}
				}

				if (childPos >= (ssize_t)childrenList->size()) {
					result.stringValue = "";
					return result;
				}

				// in case of negative offset, try to find element left of the given element
				if (offset < 0) {
					if (childPos + offset < 0) {
						result.stringValue = "";
						return result;
					} else {
						Element *resultElement = dimension->lookupElement((*childrenList)[childPos + offset].first, false);
						result.stringValue = resultElement->getName(dimension->getElemNamesVector());
						return result;
					}
				}

				// in case of positive offset, try to find element right of the given element
				if (childPos + offset < (ssize_t)childrenList->size()) {
					Element *resultElement = dimension->lookupElement((*childrenList)[childPos + offset].first, false);
					result.stringValue = resultElement->getName(dimension->getElemNamesVector());
					return result;
				}

				offset -= (ssize_t)(childrenList->size() - childPos);

				if (!bRoot) {
					// try other parents
					for (Parents::const_iterator iter = parentsList->begin() + 1; iter != parentsList->end(); ++iter) {
						childrenList = dimension->lookupElement(*iter, false)->getChildren();

						for (IdentifiersWeightType::const_iterator c = childrenList->begin(); c != childrenList->end(); c++) {
							if ((*c).first != element->getIdentifier()) {
								if (offset == 0) {
									result.stringValue = dimension->lookupElement(c->first, false)->getName(dimension->getElemNamesVector());
									return result;
								}
								offset--;
							}
						}
					}
				}
			}
		}

		result.stringValue = "";
		return result;
	}

	bool genCode(bytecode_generator& generator, uint8_t want) const {
		if (!params->at(3)->genCode(generator, Node::NODE_NUMERIC))
			return false;
		if (!params->at(2)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(1)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!params->at(0)->genCode(generator, Node::NODE_STRING))
			return false;
		if (!generator.EmitFuncCode("palo.esibling"))
			return false;
		return generator.EmitForceTypeCode(Node::NODE_STRING, want);
	}

};

}
#endif
