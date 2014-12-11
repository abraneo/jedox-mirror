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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_PLANNER_H
#define OLAP_PLANNER_H 1

#include "palo.h"
#include "Parser/RuleNode.h"

namespace palo {

class SERVER_CLASS Planner {
public:
	typedef map<IdentifierType, SubCubeList> RulesAreas;
public:
	Planner(CPCube cube, PCubeArea area, Planner *parentPlanner = 0);
	~Planner();
	PPlanNode createPlan(CubeArea::CellType cellType, RulesType rulesType, bool skipEmpty, uint64_t blockSize);
	void createRuleNodes(vector<PPlanNode> &nodes, RulesAreas &rulesAreas, const CellValue *outerDefaultValue, bool completeRuleId, bool allowMarkers);
	bool extractRules(SubCubeList &areas, RuleNode::RuleOption ruleType, RulesAreas *rulesAreas, RulesAreas *markedRulesAreas);
	void setContinueRule(CPRule rule) {continueRule = rule;}
	void setCurrentRule(CPRule rule) {currentRule = rule;}
private:
	bool extractCached(SubCubeList &areas, RulesAreas &cached, const ValueCache::CPCachedAreas &cache, IdentifierType ruleIdFilter);
	bool extractQueryCached(SubCubeList &areas, RulesAreas &cached, IdentifierType ruleIdFilter);
    void extractAreas(SubCubeList &areas, SubCubeList &inputAreas, bool &result, RulesAreas &cached, IdentifierType ruleId);
	PPlanNode createRulePlan(CPCubeArea area, CPRule rule, double &constResult, bool &supported, Node *node = 0);
	size_t saveNode(PPlanNode node);
	PPlanNode checkInfiniteRecursion();
private:
	Planner *getRootPlanner()
	{
		Planner *result = this;
		while(result->parentPlanner && result->parentPlanner != result) {
			result = result->parentPlanner;
		}
		return result;
	}
	CPCube cube;
	PCubeArea area;
	RulesType useRulesType;
	Planner *parentPlanner;
	vector<pair<size_t, PPlanNode> > allNodes;
	vector<PRule> activeRules;
	CPRule continueRule;
	CPRule currentRule;
};

}

#endif
