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

#include "Olap/Rule.h"
#include "Olap/RuleMarker.h"
#include "Engine/EngineBase.h"

#include "Exceptions/ParameterException.h"
#include "Exceptions/DefragmentationException.h"

#include "Parser/FunctionNodePaloMarker.h"
#include "Parser/RuleNode.h"
#include "Parser/SourceNode.h"
#include "Parser/RuleOptimizer.h"
#include "Parser/RuleParserDriver.h"
#include "Parser/FunctionNodeSimple.h"

namespace palo {

Rule::~Rule()
{
	restrictedRule.reset();
}

// Accepts base rules with binary operators +, -, *, and / on double sources
// e.g: {4@0} = B: {4@3} * {4@4}
// or {12@0,4@0} = B: {12@1,4@1} * ({12@2} / {4@3})
bool Rule::isSimpleRule(CPCubeArea area, Variability &varDimensions) const
{
	bool result = false;
//	if (rule) {
	if (rule && !hasMarkers()) {
		//return rule ? !hasMarkers() && checkSimpleRuleNode(db, rule->getExprNode(), varDimensions) : false;
		result = rule->getExprNode()->isPlanCompatible(area, varDimensions);
//		Variability varDimensions2;
//		bool result2 = checkSimpleRuleNode(db, rule->getExprNode(), varDimensions2);
//		if (result != result2 || varDimensions != varDimensions2) {
//			throw;
//		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// destination related methods
////////////////////////////////////////////////////////////////////////////////

const Area *Rule::getDestinationArea() const
{
	if (rule) {
		return rule->getDestinationArea();
	} else {
		return 0;
	}
}

bool Rule::withinArea(const Area *cellPath) const
{
	return Cube::isInArea(cellPath, rule->getDestinationArea());
}

bool Rule::withinRestricted(const Area *cellPath) const
{
	return Cube::isInArea(cellPath, restrictedArea.get());
}

bool Rule::within(const CubeArea *cellPath) const
{
	// use destination area of definition
	if (!Cube::isInArea(cellPath, rule->getDestinationArea())) {
		return false;
	}

	// Check the rule type of the original rule. Linear Base Rules which are
	// applied to consolidations are handled at a latter stage in Cube.cpp.
	RuleNode::RuleOption option = rule->getRuleOption();

	if (option == RuleNode::NONE) {
		return true;
	}
	if (option == RuleNode::CONSOLIDATION) {
		return !cellPath->isBase(cellPath->pathBegin());
	} else if (option == RuleNode::BASE) {
		return cellPath->isBase(cellPath->pathBegin());
	}
	return false;
}

bool Rule::contains(const Area *cellPath) const
{
	return Cube::isInArea(cellPath, containsArea.get());
}

bool Rule::containsRestricted(const Area *cellPath) const
{
	return Cube::isInArea(cellPath, containsRestrictedArea.get());
}

bool Rule::setActive(bool isActive, PServer server, PDatabase db, PCube cube, string* errMsg)
{
	checkCheckedOut();
	if (isActive == false) {
		if (activeRule && rule) {
			definition = getTextRepresentation(db, cube);
		}
		activeRule = false;
		rule.reset();
	} else {
		try {
			RuleParserDriver driver;
			driver.parse(definition);
			rule = PRuleNode(driver.getResult());

			if (rule) {
				string errMsgLocal;
				if (!errMsg) {
					errMsg = &errMsgLocal;
				}
				bool ok = rule->validate(server, db, cube, *errMsg);

				if (!ok) {
					rule.reset();
					return false;
				}
			} else {
				if (errMsg) {
					*errMsg = driver.getErrorMessage();
				}
				return false;
			}

			activeRule = true;
		} catch (...) {
			rule.reset();
			return false;
		}
	}
	return true;
}

bool Rule::isLinearRule() const
{
	return linearRule;
}

bool Rule::isRestrictedRule() const
{
	return isOptimized;
}

void Rule::optimizeRule(CPDatabase db, CPCube cube)
{
	// reset optimization
	if (isOptimized) {
		restrictedRule.reset();
		restrictedDimension = 0;
		restrictedIdentifiers.clear();
	}

	isOptimized = false;
	linearRule = false;

	// only optimize rules without markers
	if (!markers.empty()) {
		Logger::trace << "cannot optimize rules with markers" << endl;
		return;
	}

	RuleOptimizer ruleOptimizer(db, cube);

	// try to optimize the rule
	isOptimized = ruleOptimizer.optimize(rule.get());

	// update parameter
	if (isOptimized) {
		restrictedRule = PExprNode(ruleOptimizer.getRestrictedRule());
		restrictedDimension = ruleOptimizer.getRestrictedDimension();
		restrictedIdentifiers = ruleOptimizer.getRestrictedIdentifiers();

		restrictedArea.reset(new Area(*rule->getDestinationArea()));

		int pos = 0;
		const IdentifiersType *dimensions = cube->getDimensions();

		for (IdentifiersType::const_iterator iter = dimensions->begin(); iter != dimensions->end(); ++iter, pos++) {
			if (*iter == restrictedDimension) {
				PSet s(new Set);
				s->insert(restrictedIdentifiers.begin(), restrictedIdentifiers.end());
				restrictedArea->insert(pos, s);
				break;
			}
		}
	}

	linearRule = ruleOptimizer.isLinearRule();
}

////////////////////////////////////////////////////////////////////////////////
// auxillary functions
////////////////////////////////////////////////////////////////////////////////

void Rule::onCubeChange(CPDatabase db, CPCube cube)
{
	if (isActive()) {
		optimizeRule(db, cube);
		computeContains(db, cube);

		if (isOptimized) {
			computeContainsRestricted(db, cube);
		}
	}
}

void Rule::computeContains(CPDatabase db, CPCube cube)
{
	const Area *area = rule->getDestinationArea();
	containsArea.reset(new Area(area->dimCount()));
	IdentifiersType::const_iterator dims = cube->getDimensions()->begin();

	for (size_t i = 0; i < area->dimCount(); i++, ++dims) {
		if (area->elemCount(i)) {
			CPDimension d = db->lookupDimension(*dims, false);

			for (Area::ConstElemIter iter = area->elemBegin(i); iter != area->elemEnd(i); ++iter) {
				IdentifierType id = *iter;
				Element * e = d->findElement(id, 0, false);
				set<Element*> ac = d->ancestors(e);

				PSet s(new Set);
				for (set<Element*>::iterator res = ac.begin(); res != ac.end(); ++res) {
					Element * re = *res;
					s->insert(re->getIdentifier());
				}
				containsArea->insert(i, s);
			}
		}
	}
}

void Rule::computeContainsRestricted(CPDatabase db, CPCube cube)
{
	containsRestrictedArea.reset(new Area(restrictedArea->dimCount()));
	IdentifiersType::const_iterator dims = cube->getDimensions()->begin();

	for (size_t i = 0; i < restrictedArea->dimCount(); i++, ++dims) {
		if (restrictedArea->elemCount(i)) {
			CPDimension d = db->lookupDimension(*dims, false);

			for (Area::ConstElemIter iter = restrictedArea->elemBegin(i); iter != restrictedArea->elemEnd(i); ++iter) {
				IdentifierType id = *iter;
				Element * e = d->findElement(id, 0, false);
				set<Element*> ac = d->ancestors(e);

				PSet s(new Set);
				for (set<Element*>::iterator res = ac.begin(); res != ac.end(); ++res) {
					Element * re = *res;
					s->insert(re->getIdentifier());
				}
				containsRestrictedArea->insert(i, s);
			}
		}
	}
}

PRuleMarker Rule::convertMarker(CPCube cube, const Area* destination, Node* node)
{
	CPDatabase db = CONST_COMMITABLE_CAST(Database, Context::getContext()->getParent(cube));
	if (Node::NODE_SOURCE_NODE == node->getNodeType()) {
		SourceNode* source = dynamic_cast<SourceNode*> (node);
		const Area* area = source->getNodeArea();

		PRuleMarker marker = PRuleMarker(new RuleMarker(make_pair(db->getId(), cube->getId()), area, destination, getId()));
		Logger::trace << "using " << *marker << endl;

		return marker;
	} else if (Node::NODE_FUNCTION_PALO_MARKER == node->getNodeType()) {
		FunctionNodePaloMarker* source = dynamic_cast<FunctionNodePaloMarker*> (node);
		string databaseName = source->getDatabaseName();
		string cubeName = source->getCubeName();
		const vector<Node*>& path = source->getPath();

		if (databaseName.empty()) {
			databaseName = db->getName();
		}
		if (cubeName.empty()) {
			cubeName = cube->getName();
		}

		PDatabase database = Context::getContext()->getServer()->findDatabaseByName(databaseName, PUser(), true, false);
		PCube fromCube = database->findCubeByName(cubeName, PUser(), true, false);

		if (fromCube->getDimensions()->size() != path.size()) {
			throw ParameterException(ErrorException::ERROR_INVALID_COORDINATES, "length of path does not match dimension of cube", "number dimensions", (int)fromCube->getDimensions()->size());
		}

		PRuleMarker marker = PRuleMarker(new RuleMarker(make_pair(database->getId(), fromCube->getId()), make_pair(db->getId(), cube->getId()), path, destination, getId()));
		Logger::trace << "using " << *marker << endl;

		return marker;
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "unknown node type as marker");
	}
}

void Rule::removeMarkers()
{
	checkCheckedOut();
	for (vector<PRuleMarker>::iterator i = markers.begin(); i != markers.end(); ++i) {
		PRuleMarker marker = *i;

		PServer server = Context::getContext()->getServer();
		dbID_cubeID fromDbCube = marker->getFromDbCube();
		PCube fromCube = Cube::addCubeToDatabase(server, fromDbCube);
		if (fromCube) {
			fromCube->removeFromMarker(marker);
		}

		dbID_cubeID toDbCube = marker->getToDbCube();
		PCube toCube = Cube::addCubeToDatabase(server, toDbCube);
		if (toCube) {
			toCube->removeToMarker(server, marker);
		}
	}

	markers.clear();
}

void Rule::constructMarkers(const vector<Node*>& markerList, const Area *destinationArea)
{
	checkCheckedOut();
	if (!markerList.size()) {
		return;
	}
	// construct the list of markers, push markers to cube
	Context *context = Context::getContext();
	PServer server = context->getServer();
	bool generateMarkers = Context::getContext()->getChangedMarkerCubes().empty(); //true when server is started (load of cubes)

	set<PCube> changedCubes;
	CPCube toC = CONST_COMMITABLE_CAST(Cube, context->getParent(shared_from_this()));
	Logger::trace << "Cube: " << toC->getName() << "(Id: " << toC->getId() << ") Rule Id:" << this->getId() << " found markers " <<  markerList.size() << endl;

	for (vector<Node*>::const_iterator i = markerList.begin(); i != markerList.end(); ++i) {
		try {
			PRuleMarker marker = convertMarker(toC, destinationArea, *i);

			dbID_cubeID fromDbCube = marker->getFromDbCube();
			PCube fromCube = Cube::addCubeToDatabase(server, fromDbCube);
			if (fromCube) {
				fromCube->addFromMarker(marker, generateMarkers ? &changedCubes : 0);
			}

			dbID_cubeID toDbCube = marker->getToDbCube();
			PCube toCube = Cube::addCubeToDatabase(server, toDbCube);
			if (toCube) {
				toCube->addToMarker(marker);
			}

			markers.push_back(marker);
		} catch (const DefragmentationException&) {
			throw;
		} catch (const ErrorException& ex) {
			Logger::warning << "cannot convert marker: " << ex.getMessage() << " (" << ex.getDetails() << ")" << endl;
		}
	}
	if (!changedCubes.empty()) {
		(*changedCubes.begin())->commitChanges(false, PUser(), changedCubes, false);
	}
}

void Rule::computeMarkers()
{
	// remove old markers, start with a fresh list
	removeMarkers();

	if (activeRule && rule) {
		const Area *destArea = rule->getDestinationArea();
		if (destArea) {
			constructMarkers(rule->getExternalMarkers(), destArea);
			constructMarkers(rule->getInternalMarkers(), destArea);
		}
	}
}

bool Rule::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);

	CPRule lastrule = CONST_COMMITABLE_CAST(Rule, o);
	CPRule oldrule = CONST_COMMITABLE_CAST(Rule, old);
	if (oldrule != 0) {
		if (lastrule != 0 && lastrule != oldrule) {
			if (rule == oldrule->rule) {
				rule = lastrule->rule;
			}
			if (comment == oldrule->comment) {
				comment = lastrule->comment;
			}
			if (external == oldrule->external) {
				external = lastrule->external;
			}
			if (timestamp == oldrule->timestamp) {
				timestamp = lastrule->timestamp;
			}
			if (activeRule == oldrule->activeRule) {
				activeRule = lastrule->activeRule;
			}
			if (isOptimized == oldrule->isOptimized) {
				isOptimized = lastrule->isOptimized;
			}
			if (restrictedRule == oldrule->restrictedRule) {
				restrictedRule = lastrule->restrictedRule;
			}
			if (restrictedDimension == oldrule->restrictedDimension) {
				restrictedDimension = lastrule->restrictedDimension;
			}
			if (restrictedIdentifiers == oldrule->restrictedIdentifiers) {
				restrictedIdentifiers = lastrule->restrictedIdentifiers;
			}
			if (linearRule == oldrule->linearRule) {
				linearRule = lastrule->linearRule;
			}
			if (restrictedArea == oldrule->restrictedArea) {
				restrictedArea = lastrule->restrictedArea;
			}
			if (containsArea == oldrule->containsArea) {
				containsArea = lastrule->containsArea;
			}
			if (containsRestrictedArea == oldrule->containsRestrictedArea) {
				containsRestrictedArea = lastrule->containsRestrictedArea;
			}
			if (markers == oldrule->markers) {
				markers = lastrule->markers;
			}
			if (evalCounter == oldrule->evalCounter) {
				evalCounter = lastrule->evalCounter;
			}
			if (evalNullCounter == oldrule->evalNullCounter) {
				evalNullCounter = lastrule->evalNullCounter;
			}
		}
	}

	if (ret) {
		commitintern();
	}
	return ret;
}

Rule::Rule(const Rule &other) : Commitable(other)
{
	rule = other.rule;
	comment = other.comment;
	external = other.external;
	definition = other.definition;
	timestamp = other.timestamp;
	activeRule = other.activeRule;
	isOptimized = other.isOptimized;
	restrictedRule = other.restrictedRule;
	restrictedDimension = other.restrictedDimension;
	restrictedIdentifiers = other.restrictedIdentifiers;
	linearRule = other.linearRule;
	restrictedArea = other.restrictedArea;
	containsArea = other.containsArea;
	containsRestrictedArea = other.containsRestrictedArea;
	markers = other.markers;
	evalCounter = other.evalCounter;
	evalNullCounter = other.evalNullCounter;
	position = other.position;
	custom = other.custom;
}

PCommitable Rule::copy() const
{
	checkNotCheckedOut();
	PRule newr(new Rule(*this));
	return newr;
}

bool rulePositionCompare(PRule i,PRule j)
{
	return (i->getPosition()<j->getPosition() || (i->getPosition()==j->getPosition() && i->getId()<j->getId()));
}

}
