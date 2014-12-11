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

#include "Parser/RuleOptimizer.h"
#include "Parser/DoubleNode.h"
#include "Parser/ExprNode.h"
#include "Parser/FunctionNodeSimple.h"
#include "Parser/RuleNode.h"
#include "Parser/SourceNode.h"
#include "Engine/EngineBase.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
// auxillary functions
////////////////////////////////////////////////////////////////////////////////

static string rule2string(CPDatabase db, CPCube cube, Node* node)
{
	StringBuffer sb;
	node->appendRepresentation(&sb, db, cube);

	return sb.c_str();
}

////////////////////////////////////////////////////////////////////////////////
// constructors and destructors
////////////////////////////////////////////////////////////////////////////////
RuleOptimizer::~RuleOptimizer()
{
	if (restrictedRule != 0) {
		delete restrictedRule;
		restrictedRule = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
// optimizer
////////////////////////////////////////////////////////////////////////////////

bool RuleOptimizer::optimize(RuleNode* node)
{
	Logger::trace << "starting to optimize " << rule2string(db, cube, node) << endl;

	if (restrictedRule != 0) {
		delete restrictedRule;
		restrictedRule = NULL;
	}

	// STET rule check (sets restrictedRule and other variables)
	checkStetRule(node->getExprNode(), node->getDestinationArea());

	// linear rule check
	linearRule = false;

	if (node->getRuleOption() == RuleNode::BASE) {
		if (restrictedRule != 0) {
			checkLinearRule(restrictedRule, node->getDestinationArea());
		} else {
			checkLinearRule(node->getExprNode(), node->getDestinationArea());
		}
	}

	// if STET test was successful
	return restrictedRule != 0;
}

ExprNode * RuleOptimizer::getRestrictedRule() const
{
	if (restrictedRule == 0) {
		return 0;
	}

	return dynamic_cast<ExprNode*>(restrictedRule->clone());
}

const set<IdentifierType>& RuleOptimizer::getRestrictedIdentifiers() const
{
	return restrictedIdentifiers;
}

IdentifierType RuleOptimizer::getRestrictedDimension() const
{
	return restrictedDimension;
}

bool RuleOptimizer::isLinearRule()
{
	return linearRule;
}

////////////////////////////////////////////////////////////////////////////////
// auxillary methods
////////////////////////////////////////////////////////////////////////////////

void RuleOptimizer::checkLinearRule(ExprNode *node, const Area *area)
{
	Logger::trace << "checking for linearity " << rule2string(db, cube, node) << endl;

	if (!node->isValid()) {
		Logger::trace << "rule is invalid, stopping linearity check" << endl;
		return;
	}

	// the rule should be "[] = [] * CONSTANT" or "[] = CONSTANT * []"
	Node::NodeType t = node->getNodeType();

	if (t != Node::NODE_FUNCTION_MULT && t != Node::NODE_FUNCTION_DIV) {
		Logger::trace << "linearity works only for '*' or '/', stopping linearity check" << endl;
		return;
	}

	FunctionNodeSimple * op = dynamic_cast<FunctionNodeSimple*>(node);
	Node* left = op->getLeftNode();
	Node* right = op->getRightNode();
//	DoubleNode* doubleNode = 0;
	SourceNode* sourceNode = 0;

	// case "[] = CONSTANT * []"
	if (t == Node::NODE_FUNCTION_MULT && left->getNodeType() == Node::NODE_DOUBLE_NODE && right->getNodeType() == Node::NODE_SOURCE_NODE) {
		Logger::trace << "found DOUBLE_NODE * SOURCE_NODE" << endl;

//		doubleNode = dynamic_cast<DoubleNode*>(left);
		sourceNode = dynamic_cast<SourceNode*>(right);
	} else if (left->getNodeType() == Node::NODE_SOURCE_NODE && right->getNodeType() == Node::NODE_DOUBLE_NODE) {
		// case "[] = [] * CONSTANT" or "[] = CONSTANT * []"
		Logger::trace << "found SOURCE_NODE op DOUBLE_NODE" << endl;

//		doubleNode = dynamic_cast<DoubleNode*>(right);
		sourceNode = dynamic_cast<SourceNode*>(left);
	} else {
		// something else
		Logger::trace << "neither SOURCE_NODE op DOUBLE_NODE nor DOUBLE_NODE * SOURCE_NODE, stopping linearity check" << endl;
		return;
	}

	// check that the destination area contains only base elements
	const IdentifiersType *dimensionIds = cube->getDimensions();
	IdentifiersType::const_iterator i = dimensionIds->begin();
	size_t j = 0;

	for (; i != dimensionIds->end(); ++i, j++) {
		CPDimension dimension = db->lookupDimension(*i, false);

		CPSet s = area->getDim(j);
		if (s) {
			for (Area::ConstElemIter k = area->elemBegin(j); k != area->elemEnd(j); ++k) {
				Element *element = dimension->lookupElement(*k, false);

				if (element == 0) {
					Logger::warning << "cannot find dimension element with identifier " << *k << " in dimension " << dimension->getName() << endl;
				}

				if (element->getElementType() != Element::NUMERIC) {
					Logger::trace << "found non-numeric element " << element->getName(dimension->getElemNamesVector()) << " in dimension " << dimension->getName() << ", stopping linearity check" << endl;
					return;
				}
			}
		}
	}

	// check that the destination and source area are of the same shape
	Area *sourceArea = sourceNode->getSourceArea();
	j = 0;

	for (size_t k = 0; j != area->dimCount() && k != sourceArea->dimCount(); j++, k++) {
		CPSet sj = area->getDim(j);
		CPSet sk = sourceArea->getDim(k);
		if (sj && sk) {
			if (area->elemCount(j) != sourceArea->elemCount(k)) {
				Logger::trace << "source and destination area have different shape, stopping linearity check" << endl;
				return;
			}
		}
	}

	Logger::trace << "rule is linear" << endl;
	linearRule = true;
}

void RuleOptimizer::checkStetRule(ExprNode *node, const Area *area)
{
	Logger::trace << "checking for STET rule " << rule2string(db, cube, node) << endl;

	if (!node->isValid()) {
		Logger::trace << "rule is invalid, stopping STET rule check" << endl;
		return;
	}

	// the rule should be "[] = IF(...,STET(),...)" or "[] = IF(...,...,STET())"
	Node::NodeType t = node->getNodeType();

	if (t != Node::NODE_FUNCTION_IF) {
		Logger::trace << "no IF clause, stopping STET rule check" << endl;
		return;
	}

	FunctionNode * ifNode = dynamic_cast<FunctionNode*>(node);
	ExprNode* clause = dynamic_cast<ExprNode*>(ifNode->getParameters()->at(0));

	if (clause == 0) {
		Logger::warning << "something is wrong, corrupted rule " << rule2string(db, cube, node) << endl;
		return;
	}

	Node* trueNode = ifNode->getParameters()->at(1);
	Node* falseNode = ifNode->getParameters()->at(2);

	// either true or false node must be STET
	Node* nonStetNode = 0;
	bool isInclusive = false;

	if (trueNode->getNodeType() == Node::NODE_FUNCTION_STET) {
		nonStetNode = falseNode;
		isInclusive = false; // use complementary clause-set
	} else if (falseNode->getNodeType() == Node::NODE_FUNCTION_STET) {
		nonStetNode = trueNode;
		isInclusive = true; // use clause-set
	} else {
		Logger::trace << "no STET as true or false value, stopping STET rule check" << endl;
		return;
	}

	// check if clause is a dimension restriction
	Logger::trace << "checking if-clause " << rule2string(db, cube, clause) << endl;

	IdentifierType dimensionId;
	bool restriction = clause->isDimensionRestriction(cube, &dimensionId);

	if (!restriction) {
		Logger::trace << "if-clause is no dimension restriction, stopping STET rule check" << endl;
		return;
	}

	CPDimension dimension = db->lookupDimension(dimensionId, false);
	if (dimension == 0) {
		Logger::trace << "restricted dimension cannot be found, id: " << dimensionId << endl;
		return;
	}

	// ok find the dimension restriction
	set<Element*> elements = clause->computeDimensionRestriction(db, cube);

	// find dimension position
	int pos = 0;
	const IdentifiersType *dimensionIds = cube->getDimensions();

	for (IdentifiersType::const_iterator iter = dimensionIds->begin(); iter != dimensionIds->end(); ++iter, pos++) {
		if (*iter == dimensionId) {
			break;
		}
	}

	Logger::trace << "dimension restriction for dimension '" << dimensionId << "', position " << pos << endl;

	// find existing restriction
	set<IdentifierType> computedRestriction;

	// intersect with computed restriction
	if (isInclusive) {

		// no given restriction
		if (!area->elemCount(pos)) {
			for (set<Element*>::iterator iter = elements.begin(); iter != elements.end(); ++iter) {
				Element* element = *iter;
				IdentifierType id = element->getIdentifier();

				computedRestriction.insert(id);
				Logger::trace << "dimension element " << element->getName(dimension->getElemNamesVector()) << endl;
			}
		} else {
			// restriction given in rule
			for (set<Element*>::iterator iter = elements.begin(); iter != elements.end(); ++iter) {
				Element* element = *iter;
				IdentifierType id = element->getIdentifier();

				if (area->find(pos, id) != area->elemEnd(pos)) {
					computedRestriction.insert(id);
					Logger::trace << "dimension element " << element->getName(dimension->getElemNamesVector()) << endl;
				}
			}
		}
	} else {
		// intersect with complement of computed restriction

		// no given restriction
		if (!area->elemCount(pos)) {
			ElementsType allElements = dimension->getElements(PUser(), false);

			for (ElementsType::iterator iter = allElements.begin(); iter != allElements.end(); ++iter) {
				Element* element = *iter;

				if (elements.find(element) == elements.end()) {
					IdentifierType id = element->getIdentifier();
					computedRestriction.insert(id);
					Logger::trace << "dimension element " << element->getName(dimension->getElemNamesVector()) << endl;
				}
			}
		} else {
			// restriction given in rule
			set<IdentifierType> idList;

			for (set<Element*>::iterator iter = elements.begin(); iter != elements.end(); ++iter) {
				Element* element = *iter;
				IdentifierType id = element->getIdentifier();

				idList.insert(id);
			}

			for (Area::ConstElemIter iter = area->elemBegin(pos); iter != area->elemEnd(pos); ++iter) {
				IdentifierType id = *iter;

				if (idList.find(id) == idList.end()) {
					computedRestriction.insert(id);
					Logger::trace << "dimension element identifier " << id << endl;
				}
			}
		}
	}

	restrictedRule = dynamic_cast<ExprNode*>(nonStetNode->clone());
	restrictedDimension = dimension->getId();
	restrictedIdentifiers = computedRestriction;

	Logger::trace << "using rule " << rule2string(db, cube, restrictedRule) << " for restricted area" << endl;
}
}
