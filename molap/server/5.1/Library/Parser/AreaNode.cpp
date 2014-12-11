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

#include "Parser/AreaNode.h"
#include "Parser/SourceNode.h"
#include "Engine/EngineBase.h"

namespace palo {

RPPival *RPPivalFromParam(int dimensionId, const string *functionName, int functionParam, string &errorMessage)
{
	RPPival *result = 0;
	if (!functionName->compare("offset")) {
		result = new RPPival;
		result->first = dimensionId;
		result->second.first = -1;
		result->second.second = functionParam;
	} else {
		errorMessage = "unknown function: " + *functionName;
	}
	return result;
}

RPPsval *RPPsvalFromParam(string *dimensionName, const string *functionName, int functionParam, string &errorMessage)
{
	RPPsval *result = 0;
	if (!functionName->compare("offset")) {
		result = new RPPsval;
		result->first = dimensionName;
		result->second.first = 0;
		result->second.second = functionParam;
	} else {
		errorMessage = "unknown function: " + *functionName;
	}
	return result;
}

AreaNode::AreaNode(RPVpsval *elements, RPVpival *elementsIds) :
	elements(elements), elementsIds(elementsIds), unrestrictedDimensions(false), nodeArea()
{
}

AreaNode::~AreaNode()
{
	if (elements) {
		for (RPVpsval::iterator i = elements->begin(); i != elements->end(); ++i) {
			if (*i != 0) {
				if ((*i)->first) {
					delete(*i)->first;
				}
				if ((*i)->second.first) {
					delete(*i)->second.first;
				}

				delete *i;
			}
		}

		delete elements;
	}

	if (elementsIds) {
		for (RPVpival::iterator i = elementsIds->begin(); i != elementsIds->end(); ++i) {
			if (*i != 0) {
				delete *i;
			}
		}

		delete elements;
	}
}

bool AreaNode::canBeConsolidation(CPDatabase db) const
{
//	for IdentifiersType elementIDs;
	for (size_t pos = 0; pos < restriction.size(); pos++) {
		IdentifierType elemId = NO_IDENTIFIER;
		if (restriction[pos] != SourceNode::NO_RESTRICTION) {
			IdentifierType d = dimensionIDs[pos];
			if (restriction[pos] == SourceNode::ABS_RESTRICTION) {
				elemId = elementIDs[pos];
			} else {
				// Todo: -jj- simplification: relative source addressing can always be consolidation - detail implementation if needed
				return true;
			}
			PDimension dim = db->findDimension(d, PUser(), false);
			Element *elm = dim->findElement(elemId, 0, false);
			if (elm && elm->getElementType() == Element::CONSOLIDATED && !elm->isStringConsolidation()) {
				return true;
			}
		}
	}
	return false;
}

bool AreaNode::validate(PServer server, PDatabase database, PCube cube, Node* area, string& error)
{
	if (database == 0 || cube == 0) {
		valid = true;
	} else if (elements) {
		valid = validateNames(server, database, cube, area, error);
	} else if (elementsIds) {
		valid = validateIds(server, database, cube, area, error);
	} else {
		valid = false;
	}

	return valid;
}

bool AreaNode::validateNamesArea(PServer server, PDatabase database, PCube cube, Node* area, string& error)
{
	const IdentifiersType* dimensions = cube->getDimensions();

	// list of dimension identifiers
	dimensionIDs.resize(dimensions->size(), 0);

	// list of element identifiers
	elementIDs.resize(dimensions->size(), 0);

	// is restricted, i. e. DIMENSION:ELEMENT or ELEMENT
	restriction.resize(dimensions->size(), false);

	// is qualified, i. e. DIMENSION:ELEMENT
	isQualified.resize(dimensions->size(), false);

	// sequence given by user
	elementSequence.resize(dimensions->size(), -1);

	// set up dimension map and keep track of the remaining dimensions
	map<CPDimension, int> dim2pos;
	set<CPDimension> remainingDimensions;

	int pos = 0;

	for (IdentifiersType::const_iterator i = dimensions->begin(); i != dimensions->end(); ++i) {
		CPDimension dim = database->lookupDimension(*i, false);
		dim2pos[dim] = pos++;
		remainingDimensions.insert(dim);
	}

	// first: process elements with a dimension name
	pos = 0;

	for (RPVpsval::iterator i = elements->begin(); i != elements->end(); ++i, pos++) {
		if (*i != 0) {
			string* dimName = (*i)->first;
			string* elmName = (*i)->second.first;
			int elmOffset = (*i)->second.second;

			if (dimName) { // only if dimension specified
				PDimension dim = database->lookupDimensionByName(*dimName, false);

				// error: dimension not found
				if (!dim) {
					error = "dimension '" + *dimName + "' not found";
					return false;
				}

				if (dim2pos.find(dim) == dim2pos.end()) {
					error = "dimension '" + *dimName + "' is not a cube dimension";
					return false;
				}

				int dimp = dim2pos[dim];

				isQualified[dimp] = true;
				elementSequence[pos] = dimp;
				dimensionIDs[dimp] = dim->getId();
				remainingDimensions.erase(dim);

				if (elmName) { // element specified
					Element* elm = dim->lookupElementByName(*elmName, false);

					// error: element not found
					if (!elm) {
						error = "element '" + *elmName + "' not found";
						return false;
					}

					elementIDs[dimp] = elm->getIdentifier();
					restriction[dimp] = ABS_RESTRICTION;
				} else {
					elementIDs[dimp] = elmOffset;
					restriction[dimp] = OFFSET_RESTRICTION;
				}
			}
		}
	}

	// process elements without dimension name
	pos = 0;

	for (RPVpsval::iterator i = elements->begin(); i != elements->end(); ++i, pos++) {
		if (*i != 0) {
			string* dimName = (*i)->first;
			string* elmName = (*i)->second.first;

			// element has no name
			if (!dimName && elmName) {
				Element* elm = 0;

				// 1. lookup element in dimension number "pos"
				if (pos >= (int)dimensions->size()) {
					error = "too many dimensions";
					return false;
				}

				CPDimension dim = database->lookupDimension(dimensions->at(pos), false);
				set<CPDimension>::iterator d = remainingDimensions.find(dim);

				if (d != remainingDimensions.end()) {
					elm = dim->lookupElementByName(*elmName, false); // if element is not found, fall through next if statement
				}

				// 2. lookup element in remaining dimensions
				if (!elm) {
					for (int offset = 1; offset < (int)dimensions->size(); offset++) {
						int dimp = (pos + offset) % (int)dimensions->size();

						if (dimp >= (int)dimensions->size()) {
							error = "too many dimensions";
							return false;
						}

						dim = database->lookupDimension(dimensions->at(dimp), false);

						if (remainingDimensions.find(dim) != remainingDimensions.end()) {
							elm = dim->lookupElementByName(*elmName, false);

							// element found
							if (elm) {

								break;
							}
						}
					}

					// error: element not found
					if (!elm) {
						error = "element '" + *elmName + "' not found";
						return false;
					}
				}

				int dimp = dim2pos[dim];

				remainingDimensions.erase(dim);
				dimensionIDs[dimp] = dim->getId();
				elementIDs[dimp] = elm->getIdentifier();
				restriction[dimp] = ABS_RESTRICTION;
				isQualified[dimp] = false;
				elementSequence[pos] = dimp;
			} else if (!dimName) {
				// Todo: -jj- no name specified - use relative offset
				throw ErrorException(ErrorException::ERROR_INTERNAL, "source with offset not yet implemented!");;
			}
		}
	}

	for (set<CPDimension>::iterator j = remainingDimensions.begin(); j != remainingDimensions.end(); ++j) {
		CPDimension dim = *j;
		dimensionIDs[dim2pos[dim]] = dim->getId();
		restriction[dim2pos[dim]] = NO_RESTRICTION;
		unrestrictedDimensions = true;
	}

	// construct area
	nodeArea.reset(new Area(dimensions->size()));

	IdentifiersType::iterator e = elementIDs.begin();
	vector<uint8_t>::iterator r = restriction.begin();

	for (size_t i = 0; r != restriction.end(); ++e, ++r, i++) {
		if (*r == ABS_RESTRICTION) {
			PSet s(new Set);
			s->insert(*e);
			nodeArea->insert((IdentifierType)i, s);
		}
	}

	return true;
}

bool AreaNode::validateIdsArea(PServer server, PDatabase database, PCube cube, Node* destination, string& error)
{
	const IdentifiersType *dimensions = cube->getDimensions();

	// list of dimension identifiers
	dimensionIDs.resize(dimensions->size(), 0);

	// list of element identifiers
	elementIDs.resize(dimensions->size(), 0);

	// is restricted, i. e. DIMENSION:ELEMENT or ELEMENT
	restriction.resize(dimensions->size(), false);

	// is qualified, i. e. DIMENSION:ELEMENT
	isQualified.resize(dimensions->size(), false);

	// sequence given by user
	elementSequence.resize(dimensions->size(), -1);

	// set up dimension map and keep track of the remaining dimensions
	map<CPDimension, int> dim2pos;
	set<CPDimension> remainingDimensions;

	vector<ElementOld2NewMap *> dimReMap;
	for (IdentifiersType::const_iterator dimId = dimensions->begin(); dimId != dimensions->end(); ++dimId) {
		ElementOld2NewMap *dimMap = database->getDimensionMap(*dimId);
		dimReMap.push_back(dimMap);
	}

	int pos = 0;

	for (IdentifiersType::const_iterator i = dimensions->begin(); i != dimensions->end(); ++i) {
		CPDimension dim = database->lookupDimension(*i, false);
		dim2pos[dim] = pos++;
		remainingDimensions.insert(dim);
	}

	// process element identifiers of the area
	pos = 0;

	for (RPVpival::iterator i = elementsIds->begin(); i != elementsIds->end(); ++i, pos++) {
		RPPival *ei = *i;

		if (ei != 0) {
			int dimId = (*i)->first;
			int elmId = (*i)->second.first;
			bool qualified = false;

			if (dimId < 0) {
				dimId = -(dimId + 1);
			} else {
				qualified = true;
			}

			PDimension dim = database->lookupDimension(dimId, false);

			if (dim == 0) {
				error = "dimension not found";
				return false;
			}

			remainingDimensions.erase(dim);

			int dimp = dim2pos[dim];

			if (elmId < 0) {
				int offset = (*i)->second.second;
				unrestrictedDimensions = true;
				if (offset != 0) {
					elementIDs[dimp] = (IdentifierType)offset;
					restriction[dimp] = OFFSET_RESTRICTION;
					isQualified[dimp] = qualified;
					elementSequence[pos] = dimp;
				} else {
					restriction[dimp] = NO_RESTRICTION;
				}
			} else {
				if (dimReMap[dimp]) {
					IdentifierType newId = dimReMap[dimp]->translate(elmId);
					if (newId != NO_IDENTIFIER) {
						elmId = newId;
					} else {
						error = "element not found";
						return false;

					}
				}
				Element * elm = dim->lookupElement(elmId, false);

				if (elm == 0) {
					error = "element not found";
					return false;
				}

				elementIDs[dimp] = elmId;
				restriction[dimp] = ABS_RESTRICTION;
				isQualified[dimp] = qualified;
				elementSequence[pos] = dimp;
			}

			dimensionIDs[dimp] = dimId;
		}
	}

	for (set<CPDimension>::iterator j = remainingDimensions.begin(); j != remainingDimensions.end(); ++j) {
		CPDimension dim = *j;
		dimensionIDs[dim2pos[dim]] = dim->getId();
		restriction[dim2pos[dim]] = NO_RESTRICTION;
		unrestrictedDimensions = true;
	}

	nodeArea.reset(new Area(dimensions->size()));

	IdentifiersType::iterator e = elementIDs.begin();
	vector<uint8_t>::iterator r = restriction.begin();

	for (size_t i = 0; r != restriction.end(); ++e, ++r, i++) {
		if (*r == ABS_RESTRICTION) {
			PSet s(new Set);
			s->insert(*e);
			nodeArea->insert((IdentifierType)i, s);
		}
	}

	return true;
}

void AreaNode::appendXmlRepresentationType(StringBuffer* sb, int indent, bool names, const string& type)
{
	identXML(sb, indent);

	sb->appendText("<" + type + ">");
	sb->appendEol();

	if (names) {
		for (RPVpsval::iterator i = elements->begin(); i != elements->end(); ++i) {
			identXML(sb, indent + 1);

			sb->appendText("<dimension");

			if (*i != 0) {
				if ((*i)->first) {
					sb->appendText(" id=\"");
					sb->appendText(StringUtils::escapeXml(*(*i)->first));
					sb->appendText("\"");
				}

				if ((*i)->second.first) {
					sb->appendText(" restriction=\"");
					sb->appendText(StringUtils::escapeXml(*(*i)->second.first));
					sb->appendText("\"");
				}
			}

			sb->appendText("/>");
			sb->appendEol();
		}
	} else {
		IdentifiersType::iterator d = dimensionIDs.begin();
		IdentifiersType::iterator e = elementIDs.begin();
		vector<uint8_t>::iterator r = restriction.begin();

		for (; d != dimensionIDs.end(); ++d, ++e, ++r) {
			identXML(sb, indent + 1);

			sb->appendText("<dimension id=\"");
			sb->appendInteger(*d);

			if (*r == SourceNode::ABS_RESTRICTION) {
				sb->appendText("\" restriction=\"");
				sb->appendInteger(*e);
				sb->appendText("\"/>");
			} else if (*r == SourceNode::OFFSET_RESTRICTION) {
				sb->appendText("\" offset=\"");
				sb->appendInteger((int64_t)(int32_t)*e);
				sb->appendText("\"/>");
			} else {
				sb->appendText("\"/>");
			}

			sb->appendEol();
		}
	}

	identXML(sb, indent);
	sb->appendText("</" + type + ">");
	sb->appendEol();
}

void AreaNode::appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube, bool isMarker) const
{
	if (cube) {
		sb->appendText(isMarker ? "[[" : "[");

		if (!elementSequence.empty()) {
			vector<int>::const_iterator esb = elementSequence.begin();
			vector<int>::const_iterator ese = elementSequence.end() - 1;

			// find first entry from the end which is not equal -1
			bool found = false;

			// MSVC++ does not allow ese to be incremented below esb
			for (; esb <= ese; --ese) {
				if (*ese != -1) {
					found = true;
					break;
				}

				if (esb == ese) {
					break;
				}
			}

			// produce restrictions within this range
			if (found) {
				for (vector<int>::const_iterator iter = esb; iter <= ese; ++iter) {
					if (iter != esb) {
						sb->appendChar(',');
					}

					int pos = *iter;

					if (pos != -1 && restriction[pos] != SourceNode::NO_RESTRICTION) {
						bool qualified = isQualified[pos];

						IdentifierType d = dimensionIDs[pos];
						IdentifierType e = elementIDs[pos];
						PDimension dim = db->findDimension(d, PUser(), false);
						Element *elm = restriction[pos] == SourceNode::ABS_RESTRICTION ? dim->findElement(e, 0, false) : 0;

						if (qualified) {
							sb->appendText(StringUtils::escapeStringSingle(dim->getName()));
							if (restriction[pos] == SourceNode::ABS_RESTRICTION) {
								sb->appendChar(':');
								sb->appendText(StringUtils::escapeStringSingle(elm->getName(dim->getElemNamesVector())));
							} else if (restriction[pos] == SourceNode::OFFSET_RESTRICTION) {
								sb->appendText(":offset(");
								int32_t elmOffset = (int32_t)e;
								sb->appendInteger(elmOffset);
								sb->appendChar(')');
							}
						} else {
							sb->appendText(StringUtils::escapeStringSingle(elm->getName(dim->getElemNamesVector())));
						}
					}
				}
			}
		}

		sb->appendText(isMarker ? "]]" : "]");
	} else {
		sb->appendText(isMarker ? "{{" : "{");

		if (!elementSequence.empty()) {
			vector<int>::const_iterator esb = elementSequence.begin();
			vector<int>::const_iterator ese = elementSequence.end() - 1;

			// find first entry from the end which is not equal -1
			bool found = false;

			// MSVC++ does not allow ese to be incremented below esb
			for (; esb <= ese; --ese) {
				if (*ese != -1) {
					found = true;
					break;
				}

				if (esb == ese) {
					break;
				}
			}

			// produce restrictions within this range
			if (found) {
				for (vector<int>::const_iterator iter = esb; iter <= ese; ++iter) {
					if (iter != esb) {
						sb->appendChar(',');
					}

					int pos = *iter;

					if (pos != -1 && restriction[pos] != NO_RESTRICTION) {
						bool qualified = isQualified[pos];

						IdentifierType d = dimensionIDs[pos];
						IdentifierType e = elementIDs[pos];

						if (qualified) {
							sb->appendInteger(d);
							if (restriction[pos] == SourceNode::ABS_RESTRICTION) {
								sb->appendChar(':');
								sb->appendInteger(e);
							} else if (restriction[pos] == SourceNode::OFFSET_RESTRICTION) {
								sb->appendText(":offset(");
								int32_t elmOffset = (int32_t)e;
								sb->appendInteger(elmOffset);
								sb->appendChar(')');
							}
						} else {
							sb->appendInteger(d);
							sb->appendChar('@');
							sb->appendInteger(e);
						}
					}
				}
			}
		}

		sb->appendText(isMarker ? "}}" : "}");
	}
}
}
