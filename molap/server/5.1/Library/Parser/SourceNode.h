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

#ifndef PARSER_SOURCE_NODE_H
#define PARSER_SOURCE_NODE_H

#include "palo.h"

#include <string>
#include <vector>
#include <iostream>

#include "Engine/Planner.h"
#include "Parser/AreaNode.h"
#include "Parser/ExprNode.h"
#include "Olap/Rule.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief parser source node
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS SourceNode : public AreaNode, public ExprNode {
public:
	SourceNode(RPVpsval *elements, RPVpival *elementsIds);

	SourceNode(RPVpsval *elements, RPVpival *elementsIds, bool marker);

	virtual ~SourceNode();

	Node * clone();

public:
	NodeType getNodeType() const {
		return NODE_SOURCE_NODE;
	}

	Node::ValueType getValueType();

	RuleValueType getValue(const CubeArea* cellPath, bool* isCachable, RulesContext* mem_context);

	bool hasElement(CPDimension, IdentifierType) const;

	bool isSimple() const {
		return true;
	}

	virtual bool isPlanCompatible(CPCubeArea area, Variability &varDimensions) const {
#ifndef GPU_COMPLEX_RULES
		if (canBeConsolidation(db)) {
			return false;
		}
#endif

		// checking for offset restriction - relative source addressing is not yet supported
		bool simpleRestrictions = true;

		const vector<uint8_t> *restriction = getRestriction();
		vector<uint8_t>::const_iterator srcIt = restriction->begin();

		// vectors srcRestrictions and destRestrictions have the same size as they refer to the same cube
		// we don't allow PaloData Nodes so src and dest must be from the same cube
		for(size_t dimOrdinal = 0; srcIt != restriction->end(); ++srcIt, dimOrdinal++) {
			if (*srcIt != SourceNode::ABS_RESTRICTION) {
				varDimensions.set(dimOrdinal);
			}
			// check if each restricted dim in source is also restricted in destination
			if (*srcIt == SourceNode::OFFSET_RESTRICTION) {
				simpleRestrictions = false;
				break;
			}
		}
		return simpleRestrictions && isSimple();
	}

	virtual PPlanNode createPlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, Planner *parentPlanner) const
	{
		PPlanNode result;
		valid = true;
		constResult = 0;
		bool complexSource = false;

#ifndef GPU_COMPLEX_RULES
		if (canBeConsolidation(area->getDatabase())) {
			Logger::trace << "Rule source can be consolidated - not yet supported by processors! Cube: \"" << area->getCube()->getName() << "\" Rule: " << rule->getId() << endl;
			valid = false;
			return result;
		}
#else
		complexSource = true;
#endif

		const vector<uint8_t> &restriction = *getRestriction();
		size_t dimCount = area->dimCount();
		const CubeArea &targetCubeArea = *area;
		PCubeArea sourceArea = PCubeArea(new CubeArea(targetCubeArea));
		const IdentifiersType *sourceIds = getElementIDs();

		for(size_t dimOrdinal = 0; dimOrdinal < dimCount; dimOrdinal++) {
			if (restriction[dimOrdinal] == SourceNode::ABS_RESTRICTION) {
				PSet dimSet(new Set());
				dimSet->insert(sourceIds->at(dimOrdinal));
				sourceArea->insert(dimOrdinal, dimSet);
			}
		}
		if (rule->isCustom()) {
			CPCube cube = sourceArea->getCube();
			boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
			PUser user = session ? session->getUser() : PUser();
			CPDatabase database = sourceArea->getDatabase();
			try {
				if (User::checkUser(user)) {
					User::RightSetting rs(User::checkCellDataRightCube(database, cube));
					cube->checkAreaAccessRight(database, user, sourceArea, rs, false, RIGHT_READ, 0);
				}
			} catch (ErrorException &e) {
				CellValue c(e.getErrorType());
				return PPlanNode(new ConstantPlanNode(area, c));
			}
		} // targetCubeArea and sourceArea are from the same database therefore User::checkRuleDatabaseRight doesn't have to be called
		SubCubeList sourceAreas(sourceArea);
		bool anyRulesOnSource = true;
		if (!complexSource) {
			anyRulesOnSource = parentPlanner->extractRules(sourceAreas, RuleNode::BASE, 0, 0);
		}
		if (complexSource || !anyRulesOnSource) {
			PPlanNode source;
			CPCube cube = area->getCube();
			if (complexSource) {
				Planner planner(cube, sourceArea, parentPlanner);
				planner.setCurrentRule(rule);
				source = planner.createPlan(CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
			} else {
				source = PPlanNode(new SourcePlanNode(cube->getNumericStorageId(), sourceArea, cube->getObjectRevision()));
			}
//					Logger::trace << "NODE_SOURCE_NODE area: " << *source->getArea().get() << endl;
			result = PPlanNode(new TransformationPlanNode(area, source, SetMultimaps(), 1.0, vector<uint32_t>()));
//					Logger::trace << "Transformed NODE_SOURCE_NODE area: " << result->toXML() << endl;
		} else {
#ifndef GPU_COMPLEX_RULES
			// detected rule targeting the source area - not supported yet => aborting
			Logger::trace << "Chain of rules - not yet supported by processors! Cube: \"" << area->getCube()->getName() << "\" Rule: " << rule->getId() << endl;
			valid = false;
#endif
		}
		return result;
	}

	bool isMarker() const {
		return marker;
	}

	void appendRepresentation(StringBuffer* sb, CPDatabase db, CPCube cube) const {
		AreaNode::appendRepresentation(sb, db, cube, marker);
	}

	uint32_t guessType(uint32_t level);

	bool genCode(bytecode_generator& generator, uint8_t want) const;

	void appendXmlRepresentation(StringBuffer*, int, bool);

	Area* getSourceArea() {
		return nodeArea.get();
	}

	bool validate(PServer server, PDatabase database, PCube cube, Node* node, string& err) {
		return AreaNode::validate(server, database, cube, node, err);
	}

	void collectMarkers(vector<Node*>& markers) {
		if (marker) {
			markers.push_back(this);
		}
	}

protected:
	bool validateNames(PServer, PDatabase, PCube, Node*, string&);

	bool validateIds(PServer, PDatabase, PCube, Node*, string&);

private:
	IdentifierType databaseid;
	IdentifierType cubeid;
	PCubeArea fixedCellPath;
	bool marker;
};
}
#endif

