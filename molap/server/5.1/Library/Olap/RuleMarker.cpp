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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Olap/RuleMarker.h"
#include "Olap/MarkerStorage.h"
#include "Engine/EngineBase.h"
#include "Parser/StringNode.h"
#include "Parser/VariableNode.h"

namespace palo {

IdHolder RuleMarker::idHolder;

RuleMarker::RuleMarker(dbID_cubeID allDbCube, const Area* fromArea, const Area* toArea, IdentifierType ruleId) :
	numberTargetDimensions(0), fromDbCube(allDbCube), toDbCube(allDbCube), fromBase(new Area(fromArea->dimCount())), sourceArea(new Area(fromArea->dimCount())), mapping(0), isCons(false), isMulti(false), id(idHolder.getNewId()), ruleId(ruleId)
{
	PDatabase db = Context::getContext()->getServer()->lookupDatabase(toDbCube.first, false);
	PCube toCube = db->lookupCube(toDbCube.second, false);

	numberTargetDimensions = toCube->getDimensions()->size();

	// [[ ]] does not allow for permutations
	permutations = new int16_t[numberTargetDimensions];
	fixed = new uint32_t[numberTargetDimensions];

	for (size_t i = 0; i < numberTargetDimensions; i++) {
		permutations[i] = (uint32_t)i;
	}

	// convert FROM AREA description into a list of sets of base elements
	PCube fromCube = db->lookupCube(fromDbCube.second, false);
	IdentifiersType::const_iterator d = fromCube->getDimensions()->begin();

	for (size_t i = 0; i < fromArea->dimCount(); i++, ++d) {
		if (fromArea->elemCount(i)) {
			CPDimension dimension = db->lookupDimension(*d, false);

			PSet source;
			Element *elem = dimension->lookupElement(*fromArea->elemBegin(i), false);
			if (elem->getElementType() == Element::CONSOLIDATED) {
				isCons = true;
				source.reset(new Set);
				source->insert(elem->getIdentifier());
			}
			set<Element *> e = dimension->getBaseElements(elem, 0);
			PSet base(new Set);

			for (set<Element *>::iterator j = e.begin(); j != e.end(); ++j) {
				Element *element = *j;

				base->insert(element->getIdentifier());
			}
			fromBase->insert(i, base);
			if (!source) {
				source = base;
			}
			sourceArea->insert(i, source);
		}

	}

	// convert TO AREA description into a list of identifiers
	size_t f = 0;

	for (size_t i = 0; i != toArea->dimCount(); i++, f++) {
		if (toArea->elemCount(i)) {
			fixed[f] = *toArea->elemBegin(i);
			if (!fromBase->elemCount(f)) {
				PSet s(new Set);
				s->insert(fixed[f]);
				fromBase->insert(f, s); // restrict from area if not restricted yet
			}
			permutations[f] = MarkerStorage::FIXED_ELEMENT;
		} else {
			fixed[f] = NO_IDENTIFIER;
			if (fromBase->elemCount(f)) {
				permutations[f] = MarkerStorage::ALL_ELEMENTS;
				isMulti = true;
			}
		}
	}
}

// path contains either constants or variables. A constant is a
// string denoting a dimension element in the fromCube. A variable
// is a dimension shared by both the fromCube and the toCube or at
// least two dimensions with the same elements.

RuleMarker::RuleMarker(dbID_cubeID fromDbCube, dbID_cubeID toDbCube, const vector<Node *>& path, const Area* toArea, IdentifierType ruleId) :
	numberTargetDimensions(0), fromDbCube(fromDbCube), toDbCube(toDbCube), fromBase(), sourceArea(), mapping(0), isCons(false), isMulti(false), id(idHolder.getNewId()), ruleId(ruleId)
{
	Context *context = Context::getContext();

	// split into constants and variables
	vector<string> constants;
	vector<string> variables;

	for (vector<Node*>::const_iterator i = path.begin(); i != path.end(); ++i) {
		Node* node = *i;

		if (node->getNodeType() == Node::NODE_STRING_NODE) {
			constants.push_back(dynamic_cast<StringNode*> (node)->getStringValue());
			variables.push_back("");
		} else if (node->getNodeType() == Node::NODE_VARIABLE_NODE) {
			constants.push_back("");
			variables.push_back(dynamic_cast<VariableNode*> (node)->getStringValue());
		} else {
			constants.push_back("");
			variables.push_back("");
		}
	}

	// it is possible that we have a variable which is constant
	// because of a restriction in the toArea
	PServer server = context->getServer();
	PCube toCube = server->lookupDatabase(toDbCube.first, false)->lookupCube(toDbCube.second, false);
	const IdentifiersType * toDimensions = toCube->getDimensions();
	vector<string>::iterator j = constants.begin();

	for (vector<string>::iterator i = variables.begin(); i != variables.end(); ++i, ++j) {
		const string& name = *i;

		if (name.empty()) {
			continue;
		}

		PDimension variableDimension = CONST_COMMITABLE_CAST(Database, context->getParent(toCube))->findDimensionByName(name, PUser(), false);

		IdentifiersType::const_iterator d = toDimensions->begin();
		size_t dim = 0;

		for (; d != toDimensions->end(); ++d, dim++) {
			if (*d == variableDimension->getId()) {
				break;
			}
		}

		if (d == toDimensions->end()) {
			throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "cannot find variable dimension", "dimension", name);
		}

		const CPSet s = toArea->getDim(dim);

		if (s && s->size()) {
			Element *element = variableDimension->findElement(*s->begin(), 0, false);
			Logger::trace << "variable '" << name << "' in rule is constant because of destination '" << element->getName(variableDimension->getElemNamesVector()) << "'" << endl;

			*i = "";
			*j = element->getName(variableDimension->getElemNamesVector());
		}
	}

	// convert FROM AREA description into a list of sets of base elements
	PCube fromCube = server->lookupDatabase(fromDbCube.first, false)->lookupCube(fromDbCube.second, false);
	IdentifiersType::const_iterator d = fromCube->getDimensions()->begin();
	fromBase.reset(new Area(fromCube->getDimensions()->size()));
	sourceArea.reset(new Area(fromCube->getDimensions()->size()));

	for (size_t i = 0; i < constants.size(); i++, ++d) {
		const string& name = constants[i];
		CPDimension dimension = CONST_COMMITABLE_CAST(Database, context->getParent(fromCube))->lookupDimension(*d, false);

		// s is either empty or contains one element, unfold to base elements
		if (!name.empty()) {
			Element *elem = dimension->findElementByName(name, 0, false);
			PSet source;
			if (elem->getElementType() == Element::CONSOLIDATED) {
				isCons = true;
				source.reset(new Set);
				source->insert(elem->getIdentifier());
			}
			set<Element*> e = dimension->getBaseElements(elem, 0);
			PSet base(new Set);

			for (set<Element*>::iterator j = e.begin(); j != e.end(); ++j) {
				Element *element = *j;

				base->insert(element->getIdentifier());
			}
			fromBase->insert(i, base);
			if (!source) {
				source = base;
			}
			sourceArea->insert(i, source);
		}
	}

	// convert variable names into dimensions
	vector<CPDimension> varDimensions;

	for (vector<string>::iterator i = variables.begin(); i != variables.end(); ++i) {
		const string& name = *i;

		if (name.empty()) {
			varDimensions.push_back(PDimension());
		} else {
			PDimension vd = CONST_COMMITABLE_CAST(Database, context->getParent(toCube))->findDimensionByName(name, PUser(), false);
			varDimensions.push_back(vd);
		}
	}

	// convert TO AREA description into a list of identifiers
	numberTargetDimensions = toDimensions->size();

	permutations = new int16_t[numberTargetDimensions];
	fixed = new uint32_t[numberTargetDimensions];

	size_t f = 0;

	IdentifiersType::const_iterator dd = toDimensions->begin();
	vector<pair<CPDimension, CPDimension> > dimPairs;
	const IdentifiersType * fromDimensions = fromCube->getDimensions();

	for (size_t i = 0; i < toArea->dimCount(); i++, f++, ++dd) {
		const CPSet s = toArea->getDim(i);

		if (s && s->size()) {
			fixed[f] = *s->begin();
			permutations[f] = MarkerStorage::FIXED_ELEMENT;
			dimPairs.push_back(pair<CPDimension, CPDimension>(CPDimension(), CPDimension()));
		} else {
			// find matching variable dimension
			fixed[f] = NO_IDENTIFIER;
			CPDimension td = CONST_COMMITABLE_CAST(Database, context->getParent(toCube))->lookupDimension(*dd, false);
			vector<CPDimension>::iterator vIter = varDimensions.begin();
			uint32_t pos = 0;

			for (; vIter != varDimensions.end(); ++vIter, ++pos) {
				if (*vIter == td) {
					break;
				}
			}

			if (vIter == varDimensions.end()) {
				permutations[f] = MarkerStorage::ALL_ELEMENTS;
				isMulti = true;
//				throw ParameterException(ErrorException::ERROR_DIMENSION_NOT_FOUND, "cannot find variable dimension", "dimension", td->getName());
				//TODO
				//map here the entire target dimension to have similar results to
				//PALO.DATA.
				//possible solution:
				//1. write here a magic signalling ALL elements
				//2. in general, when enumerating cube pages, test this magic as a valid
				//   entry and substitute with the enumerated id from path.
				//RIA thinks enumerating here the entire dimension will lead to unnecessary
				//data explosion
			} else {
				permutations[f] = pos;

				dimPairs.push_back(pair<CPDimension, CPDimension> (CONST_COMMITABLE_CAST(Database, context->getParent(fromCube))->lookupDimension(fromDimensions->at(pos), false), td));
			}
		}
	}

	// mapping needed
	mapping = new PMappingType[numberTargetDimensions];
	memset(mapping, 0, sizeof(PMappingType)*numberTargetDimensions);

	PMappingType *m = mapping;

	for (vector<pair<CPDimension, CPDimension> >::iterator i = dimPairs.begin(); i != dimPairs.end(); ++i, ++m) {
		*m = 0;

		pair<CPDimension, CPDimension>& p = *i;

		if (!p.first) {
			continue;
		}

		CPDimension fromDimension = p.first;
		CPDimension toDimension = p.second;

		if (fromDimension == toDimension) {
			// identical dimensions - do not create 1:1 mapping and continue
			continue;
		}
		*m = new MappingType();

		ElementsType elements = fromDimension->getElements(PUser(), false);

		for (ElementsType::iterator e = elements.begin(); e != elements.end(); ++e) {
			Element * fe = *e;

			if (!fe) {
				continue;
			}

			Element* te = toDimension->lookupElementByName(fe->getName(fromDimension->getElemNamesVector()), false);

			if (!te || te->getElementType() == Element::CONSOLIDATED) {
				continue;
			}

			Logger::trace << "using mapping " << fe->getIdentifier() << " -> " << te->getIdentifier() << " for name '" << fe->getName(fromDimension->getElemNamesVector()) << "'" << endl;

			(*m)->getReverseMap()[te->getIdentifier()] = fe->getIdentifier();

			if (fe->getElementType() == Element::CONSOLIDATED) {
				for (WeightedSet::const_iterator febi = fe->baseElementsBegin(); febi != fe->baseElementsEnd(); ++febi) {
					(*m)->insert(pair<uint32_t, uint32_t>(febi.first(), te->getIdentifier()));
				}
			} else {
				(*m)->insert(pair<uint32_t, uint32_t>(fe->getIdentifier(), te->getIdentifier()));
			}
		}
		if (!(*m)->size()) {
			// TODO -jj- :no elements mapped -> marker will not work - dimension has to change later and marker mapping has to be updated
		}
	}
}

RuleMarker::~RuleMarker()
{
	delete[] permutations;
	delete[] fixed;
	if (mapping) {
		for (size_t dim = 0; dim < numberTargetDimensions; dim++) {
			delete mapping[dim];
		}
		delete[] mapping;
	}
}

dbID_cubeID RuleMarker::getFromDbCube() const
{
	return fromDbCube;
}

dbID_cubeID RuleMarker::getToDbCube() const
{
	return toDbCube;
}

CPArea RuleMarker::getFromBase() const
{
	return fromBase;
}

const int16_t *RuleMarker::getPermutations() const
{
	return permutations;
}

const uint32_t *RuleMarker::getFixed() const
{
	return fixed;
}

const RuleMarker::PMappingType *RuleMarker::getMapping() const
{
	return mapping;
}

const bool RuleMarker::isSourceCons() const
{
	return isCons;
}

const bool RuleMarker::isMultiplicating() const
{
	return isMulti;
}

const IdentifierType RuleMarker::getId() const
{
	return id;
}

ostream& operator <<(ostream& out, const RuleMarker& marker)
{
	PServer server = Context::getContext()->getServer();

	dbID_cubeID fromDbCube = marker.getFromDbCube();
	PCube fromCube;
	PDatabase fromDB = server->lookupDatabase(fromDbCube.first, false);
	if (fromDB) {
		fromCube = fromDB->lookupCube(fromDbCube.second, false);
	}

	dbID_cubeID toDbCube = marker.getToDbCube();
	PCube toCube = server->lookupDatabase(toDbCube.first, false)->lookupCube(toDbCube.second, false);

	out << "MARKER from '" << (fromCube ? fromCube->getName() : "DELETED") << "' to '" << toCube->getName() << "': ";

	for (size_t i = 0; i < marker.fromBase->dimCount(); i++) {
		const CPSet s = marker.fromBase->getDim(i);

		out << "(";

		if (s) {
			for (Set::Iterator j = s->begin(); j != s->end(); ++j) {
				if (j != s->begin()) {
					out << ' ';
				}
				out << *j;
			}
		}

		out << ") ";
	}

	out << "=> (";

	string sep = "";

	size_t nd = toCube->getDimensions()->size();
	uint32_t* q = marker.fixed;
	for (int16_t* p = marker.permutations; 0 < nd; nd--, p++, q++) {
		out << sep;

		int16_t id = *p;

		if (id == MarkerStorage::FIXED_ELEMENT) {
			out << "[" << *q << "]";
		} else {
			out << id;
		}

		sep = " ";
	}

	out << ")";

	return out;
}

}
