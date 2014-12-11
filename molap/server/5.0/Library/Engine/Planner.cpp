/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#include "Olap/Server.h"
#include "Olap/SubCubeList.h"
#include "Olap/Rule.h"
#include "Engine/EngineCpu.h"

#include "Parser/SourceNode.h"
#include "Parser/DoubleNode.h"
#include "Parser/FunctionNodeSimple.h"

#include "Engine/Planner.h"

namespace palo {

Planner::Planner(CPCube cube, PCubeArea area) : cube(cube), area(area)
{
//	if (!area->validate()) {
//		Logger::debug << "Planner area: " << *area << endl;
//	}
}

bool Planner::extractRules(SubCubeList &areas, RuleNode::RuleOption ruleType, RulesAreas *rulesAreas, RulesAreas *markedRulesAreas)
{
	CPRuleList rules = cube->getRuleList(false);
	SubCubeList inputAreas = areas;
	SubCubeList nextAreas;
	bool result = false;
	for (RuleList::ConstIterator iter = rules->const_begin(); iter != rules->const_end(); ++iter) {
		CPRule rule = CONST_COMMITABLE_CAST(Rule, *iter);
		const Area *ruleDestinationArea = rule->getDestinationArea();
		if (rule->isActive() && (ruleType == RuleNode::NONE || rule->getRuleOption() == RuleNode::NONE || rule->getRuleOption() == ruleType) && ruleDestinationArea) {
			for (SubCubeList::const_iterator inputArea = inputAreas.begin(); inputArea != inputAreas.end(); ++inputArea) {
				if (inputArea->second->isOverlapping(*ruleDestinationArea)) {
					if (!rulesAreas) {
						// just rule check -> no modifications
						return true;
					}
					PCubeArea intersectionArea;
					inputArea->second->intersection(*ruleDestinationArea, &intersectionArea, &nextAreas);
//					Logger::trace << "found rule(" << rule->getId() << ") " << (**inputArea) << "-" << *ruleDestinationArea << " = " << *intersectionArea << "(+" << nextAreas << ")" << endl;
					result = true;
					if (intersectionArea) {
						if (markedRulesAreas && rule->hasMarkers()) {
							(*markedRulesAreas)[rule->getId()].insertAndMerge(intersectionArea);
						} else {
							(*rulesAreas)[rule->getId()].insertAndMerge(intersectionArea);
						}
					}
				} else {
					nextAreas.push_back(*inputArea);
				}
			}
			inputAreas = nextAreas;
			nextAreas.clear();
		}
	}
	if (result) {
		areas = inputAreas;
	}
	return result;
}

void Planner::extractAreas(SubCubeList &areas, SubCubeList &inputAreas, bool &result, RulesAreas &cached, IdentifierType ruleId)
{
    for (SubCubeList::const_iterator it = areas.begin(); it != areas.end(); ++it){
    	SubCubeList nextAreas;
        for (SubCubeList::const_iterator it2 = inputAreas.begin(); it2 != inputAreas.end(); ++it2){
            if (it2->second->isOverlapping(*(it->second))) {
                PCubeArea intersectionArea;
                it2->second->intersection(*(it->second), &intersectionArea, &nextAreas);
                result = true;
                if (intersectionArea) {
//                	if (!intersectionArea->validate()) {
//                		Logger::debug << *it2->second << " X "<< *it->second << " = " << *intersectionArea << endl;
//                	}
                    cached[ruleId].insertAndMerge(intersectionArea);
                }
            } else {
                nextAreas.push_back(*it2);
            }
        }
        inputAreas = nextAreas;
    }
}

bool Planner::extractCached(SubCubeList &areas, RulesAreas &cached, const ValueCache::CPCachedAreas &cache, IdentifierType ruleIdFilter)
{
	SubCubeList inputAreas = areas;
	bool result = false;
	if (ruleIdFilter == ALL_IDENTIFIERS) {
		for (ValueCache::CachedAreas::const_iterator ruleAreas = cache->begin(); ruleAreas != cache->end(); ++ruleAreas) {
			IdentifierType ruleId = ruleAreas->first;
			extractAreas(*ruleAreas->second, inputAreas, result, cached, ruleId);
		}
	} else {
		ValueCache::CachedAreas::const_iterator ruleAreas = cache->find(ruleIdFilter);
		if (ruleAreas != cache->end()) {
			extractAreas(*ruleAreas->second, inputAreas, result, cached, ruleIdFilter);
		}
	}
	if (result) {
		areas = inputAreas;
	}
	return result;
}

bool Planner::extractQueryCached(SubCubeList &areas, RulesAreas &cached, IdentifierType ruleIdFilter)
{
	bool result = false;
	boost::shared_ptr<ValueCache::QueryCache> queryCache = Context::getContext()->getQueryCache(cube);
	if (queryCache) {
		result = extractCached(areas, cached, queryCache->getAreas(), ruleIdFilter);
	}
	return result;
}

PPlanNode Planner::createRulePlan(CPCubeArea area, CPRule rule, double &constResult, bool &valid, Node *node)
{
	PPlanNode result;
	valid = true;

	constResult = 0;

	if (!node && rule->isActive() && rule->rule->getExprNode()) {
		node = rule->rule->getExprNode();
	}
	if (node) {
		switch (node->getNodeType()) {
			case Node::NODE_DOUBLE_NODE:
				constResult = dynamic_cast<DoubleNode*>(node)->getDoubleValue();
				return result;
			case Node::NODE_SOURCE_NODE: {
				SourceNode *src = dynamic_cast<SourceNode*>(node);

				if (src->canBeConsolidation(area->getDatabase())) {
					Logger::trace << "Rule source can be consolidated - not yet supported by processors! Cube: \"" << area->getCube()->getName() << "\" Rule: " << rule->getId() << endl;
					valid = false;
					return result;
				}

				vector<uint8_t> &restriction = *src->getRestriction();
				size_t dimCount = area->dimCount();
				IdentifiersType transformationMask(dimCount, NO_IDENTIFIER);
				const CubeArea &targetCubeArea = *area;
				PCubeArea sourceArea = PCubeArea(new CubeArea(targetCubeArea));
				const Area *ruleDstArea = rule->getDestinationArea();
				const IdentifiersType *sourceIds = src->getElementIDs();

				for(size_t dimOrdinal = 0; dimOrdinal < dimCount; dimOrdinal++) {
					if (restriction[dimOrdinal] == SourceNode::ABS_RESTRICTION) {
						CPSet targetSet = ruleDstArea->getDim(dimOrdinal);
#ifdef GPU_RULES_DIFF_DIM
						if (!targetSet) {
							// if rule is not restricted in target dim
							// if query only addresses one element in this dim, treat as transformation (if element != restricted rule element)
							CPSet querySet = area->getDim(dimOrdinal);
							IdentifierType elementId = (*src->getElementIDs())[dimOrdinal];
							if(querySet->size() == 1 && *querySet->begin() != elementId){
								transformationMask[dimOrdinal] = *querySet->begin(); 
							}
						} else if(targetSet->size() == 1) {
							transformationMask[dimOrdinal] = *targetSet->begin();
						}
#else
						if (!targetSet || targetSet->size() != 1) {
							// error -> exploding source
							Logger::debug << "invalid simple rule in Planner::createRulePlan" << endl;
							valid = false;
							return result;
						}
						transformationMask[dimOrdinal] = *targetSet->begin();
#endif
						PSet dimSet(new Set());
						dimSet->insert(sourceIds->at(dimOrdinal));
						sourceArea->insert(dimOrdinal, dimSet);
					}
				}
				SubCubeList sourceAreas(sourceArea);
				bool anyRulesOnSource = extractRules(sourceAreas, RuleNode::BASE, 0, 0);
				if (!anyRulesOnSource) {
					PPlanNode source = PPlanNode(new SourcePlanNode(area->getCube()->getNumericStorageId(), sourceArea, 0));
//					Logger::trace << "NODE_SOURCE_NODE area: " << *source->getArea().get() << endl;
					result = PPlanNode(new TransformationPlanNode(area, source, transformationMask, SetMultimaps(), 1.0));
//					Logger::trace << "Transformed NODE_SOURCE_NODE area: " << result->toXML() << endl;
				} else {
					// detected rule targeting the source area - not supported yet => aborting
					Logger::trace << "Chain of rules - not yet supported by processors! Cube: \"" << area->getCube()->getName() << "\" Rule: " << rule->getId() << endl;
					valid = false;
				}
				break;
			}
			case Node::NODE_FUNCTION_MULT: case Node::NODE_FUNCTION_DIV: case Node::NODE_FUNCTION_ADD: case Node::NODE_FUNCTION_SUB: {
				// recursive call for function operand nodes
				FunctionNodeSimple *fNode = dynamic_cast<FunctionNodeSimple*>(node);
				vector<PPlanNode> sourceNodes(2);
				double leftConst = 0;
				double rightConst = 0;
				sourceNodes[0] = createRulePlan(area, rule, leftConst, valid, fNode->getLeftNode());
				if (valid) {
					sourceNodes[1] = createRulePlan(area, rule, rightConst, valid, fNode->getRightNode());
				}
				if (!valid) {
					return result;
				}
				if (!sourceNodes[0] && !sourceNodes[1]) {
					// both sources are constants
					switch (node->getNodeType()) {
						case Node::NODE_FUNCTION_MULT:
							constResult = leftConst * rightConst;
							break;
						case Node::NODE_FUNCTION_DIV:
							constResult = rightConst == 0 ? 0 : leftConst / rightConst;
							break;
						case Node::NODE_FUNCTION_ADD:
							constResult = leftConst + rightConst;
							break;
						case Node::NODE_FUNCTION_SUB:
							constResult = leftConst - rightConst;
							break;
						default:
							constResult = 0;
							break;
					}
				} else {
					PlanNodeType planType = SOURCE; // just to identify supported operation type
					double otherConst = 0;
					switch (node->getNodeType()) {
						case Node::NODE_FUNCTION_MULT: planType = MULTIPLICATION; break;
						case Node::NODE_FUNCTION_DIV: planType = DIVISION; break;
						case Node::NODE_FUNCTION_ADD: planType = ADDITION; break;
						case Node::NODE_FUNCTION_SUB: planType = SUBTRACTION; break;
						default:
							valid = false;
							Logger::trace << "unexpected rule node type in Planner::createRulePlan! Cube: \"" << area->getCube()->getName() << "\" Rule: " << rule->getId() << endl;
							return result;
					}
					// one of the sources is constant
					// only multiplication and division is supported
					if (sourceNodes[0] && sourceNodes[1]) {
						// both sources are variable
					} else {
						if (planType == ADDITION || planType == SUBTRACTION) {
							planType = SOURCE; // signal that this combination is not supported
						}
						// only multiplication and division is supported with const
						if (!sourceNodes[0]) {
							otherConst = leftConst;
						} else if (!sourceNodes[1]) {
							otherConst = rightConst;
						}
					}
					if (planType != SOURCE) {
						result = PPlanNode(new PlanNode(planType, area, 0, sourceNodes, cube->getCache(), cube, otherConst));
					}
				}
				break;
			}
			default:
				break;
		}
	}
	if (result) {
		result->setRuleId(rule->getId());
	}
	return result;
}

void Planner::createRuleNodes(vector<PPlanNode> &nodes, RulesAreas &rulesAreas, const CellValue *outerDefaultValue, bool tryProcessors, bool allowMarkers)
{
	for (RulesAreas::const_iterator ruleIt = rulesAreas.begin(); ruleIt != rulesAreas.end(); ++ruleIt) {
		CellValue defaultValue;
		CellValue *ruleDefaultValue = 0;
		if (outerDefaultValue) {
			ruleDefaultValue = &defaultValue;
			defaultValue = *outerDefaultValue;
			defaultValue.setRuleId(ruleIt->first);
		}
		for (SubCubeList::const_iterator ruleArea = ruleIt->second.begin(); ruleArea != ruleIt->second.end(); ++ruleArea) {
			PPlanNode rulePlanNode;
			CPRule rule = cube->findRule(ruleIt->first);

			if (tryProcessors) {
				// try to build a rule plan, use LegacyRule if building fails
				Rule::Variability varDimensions;
				bool simpleRule = rule ? rule->isSimpleRule(ruleArea->second->getDatabase(), varDimensions) : false;

				if (simpleRule /* && !varDimensions.empty() */) {
					double constResult = 0;
					bool valid;
					// try to build the plan here, insert it into nodes and continue the loop
					rulePlanNode = createRulePlan(ruleArea->second, rule, constResult, valid);
					// Todo: -jj- set related rule identification for performance counter in processor
				}
			}
			if (!rulePlanNode) {
				rulePlanNode = PPlanNode(new LegacyRulePlanNode(cube, ruleArea->second, rule, ruleDefaultValue, cube->getCache(), allowMarkers && rule->hasMarkers()));
			}
			nodes.push_back(rulePlanNode);
		}
	}
}

PPlanNode Planner::createPlan(CubeArea::CellType cellType, RulesType useRulesType, bool skipEmpty, uint64_t blockSize)
{
	RulesType calcRules = RulesType(useRulesType & ALL_RULES);
	if (calcRules != NO_RULES && !cube->hasActiveRule()) {
		calcRules = NO_RULES;
	}

	const CellValue *defaultNumValue = skipEmpty ? (const CellValue *)0 : &CellValue::NullNumeric;
	const CellValue *defaultStrValue = skipEmpty ? (const CellValue *)0 : &CellValue::NullString;
	vector<PPlanNode> spns;
	bool baseNumeric = (cellType & CubeArea::BASE_NUMERIC) == CubeArea::BASE_NUMERIC;
	bool baseStrings = (cellType & CubeArea::BASE_STRING) == CubeArea::BASE_STRING;
	SubCubeList stringAreas;
	SubCubeList numericAreas;
	SubCubeList consolidatedAreas;
	RulesAreas cachedAreas;
	RulesAreas queryCachedAreas;
	RulesAreas numericBaseRulesAreas;
	RulesAreas numericConsRulesAreas;
	RulesAreas numericMarkedRulesAreas;
	RulesAreas stringRulesAreas;
	ValueCache::PCachedAreas cachedArs;
	PStorageCpu cacheStorage;
	bool suppressEmptyPlanMessage = false;

	bool useCache = !(useRulesType & NOCACHE) && ((useRulesType & ALL_RULES) != NO_RULES);

	if (useCache) {
		cube->getCache()->getAreasAndStorage(cacheStorage, cachedArs);
	}

	if (CubeArea::baseOnly(cellType) && calcRules == NO_RULES && skipEmpty) {
		// No Rules just base values - filter whole area
		stringAreas.push_back(area);
		numericAreas.push_back(area);
	} else {
		// split areas by type into string, consolidated and numeric
		area->splitbyTypes(stringAreas, numericAreas, consolidatedAreas);
		// if only base values -> clear the list of consolidated areas
		if (CubeArea::baseOnly(cellType)) {
			if (stringAreas.empty() && numericAreas.empty()) {
				suppressEmptyPlanMessage = true;
			}
			consolidatedAreas.clear();
		}
	}
	size_t subareas = stringAreas.size() + numericAreas.size() + consolidatedAreas.size();

	bool anyConsolidation = consolidatedAreas.size() > 0;
	bool anyDirectRules = false;
	bool anyCached = false;
	bool anyQueryCached = false;
	if (!subareas) {
		if (!suppressEmptyPlanMessage) {
			Logger::error << "Planner::createPlan generated empty plan!" << endl;
		}
	} else {
		if (calcRules & DIRECT_RULES) {
			anyDirectRules |= extractRules(stringAreas, RuleNode::BASE, &stringRulesAreas, &stringRulesAreas);
			anyDirectRules |= extractRules(numericAreas, RuleNode::BASE, &numericBaseRulesAreas, &numericMarkedRulesAreas);
			anyDirectRules |= extractRules(consolidatedAreas, RuleNode::CONSOLIDATION, &numericConsRulesAreas, &numericConsRulesAreas);

			// check intersection of all lists with rules
			if (anyDirectRules) {
				size_t totalRuleAreas = 0;
				for (RulesAreas::const_iterator rule = stringRulesAreas.begin(); rule != stringRulesAreas.end(); ++rule) {
					totalRuleAreas += rule->second.size();
//					Logger::trace << "Rule: " << rule->first << endl << rule->second << endl;
				}
				for (RulesAreas::iterator rule = numericConsRulesAreas.begin(); rule != numericConsRulesAreas.end(); ++rule) {
					if (useCache) {
						anyCached |= extractCached(rule->second, cachedAreas, cachedArs, rule->first);
						anyQueryCached |= extractQueryCached(rule->second, queryCachedAreas, rule->first);
					}
					totalRuleAreas += rule->second.size();
//					Logger::trace << "Rule: " << rule->first << endl << rule->second << endl;
				}
				for (RulesAreas::iterator rule = numericBaseRulesAreas.begin(); rule != numericBaseRulesAreas.end(); ++rule) {
					if (useCache) {
						anyCached |= extractCached(rule->second, cachedAreas, cachedArs, rule->first);
						anyQueryCached |= extractQueryCached(rule->second, queryCachedAreas, rule->first);
					}
					totalRuleAreas += rule->second.size();
//					Logger::trace << "Rule: " << rule->first << endl << rule->second << endl;
				}
				for (RulesAreas::iterator rule = numericMarkedRulesAreas.begin(); rule != numericMarkedRulesAreas.end(); ++rule) {
					if (useCache) {
						anyCached |= extractCached(rule->second, cachedAreas, cachedArs, rule->first);
						anyQueryCached |= extractQueryCached(rule->second, queryCachedAreas, rule->first);
					}
					totalRuleAreas += rule->second.size();
//					Logger::trace << "Rule: " << rule->first << endl << rule->second << endl;
				}
				Logger::trace << "Planner::createPlan found " << numericMarkedRulesAreas.size() + numericConsRulesAreas.size() + numericBaseRulesAreas.size() + stringRulesAreas.size() << " active direct rules in " << totalRuleAreas << " areas!" << endl;
			}
		}

		if (baseStrings) {
			for (SubCubeList::iterator it = stringAreas.begin(); it != stringAreas.end(); ++it) {
				spns.push_back(PPlanNode(new SourcePlanNode(cube->getStringStorageId(), it->second, defaultStrValue)));
			}
		}
		if (baseNumeric) {
			for (SubCubeList::iterator it = numericAreas.begin(); it != numericAreas.end(); ++it) {
				spns.push_back(PPlanNode(new SourcePlanNode(cube->getNumericStorageId(), it->second, defaultNumValue)));
			}
		}
		if (anyConsolidation) {
			if (useCache) {
				anyCached |= extractCached(consolidatedAreas, cachedAreas, cachedArs, NO_IDENTIFIER);
				anyQueryCached |= extractQueryCached(consolidatedAreas, queryCachedAreas, NO_IDENTIFIER);
			}
			for (SubCubeList::const_iterator it = consolidatedAreas.begin(); it != consolidatedAreas.end(); ++it) {
				PAggregationMaps aggregationMaps(new AggregationMaps());
				// TODO -jj- place for optimization - expanding the same aggregated subsets generates the same aggregationMaps - should be checked here
				// TODO -jj- expansion cache?
				PCubeArea initialBaseArea = it->second->expandBase(aggregationMaps.get());
				SubCubeList baseAreas;
				RulesAreas baseRulesAreas;
				baseAreas.push_back(initialBaseArea);

				size_t totalRuleAreas = 0;
				if ((calcRules & INDIRECT_RULES) && extractRules(baseAreas, RuleNode::BASE, &baseRulesAreas, &baseRulesAreas)) {
					for (RulesAreas::iterator ruleIt = baseRulesAreas.begin(); ruleIt != baseRulesAreas.end(); ++ruleIt) {
						totalRuleAreas += ruleIt->second.size();
					}
					Logger::trace << "Planner::createPlan found " << baseRulesAreas.size() << " indirect rules in " <<  totalRuleAreas << " rule areas!" << endl;
					for (RulesAreas::const_iterator rule = baseRulesAreas.begin(); rule != baseRulesAreas.end(); ++rule) {
						Logger::trace << "Rule: " << rule->first << endl << rule->second << endl;
					}
				}

				vector<PPlanNode> baseNodes;

				baseNodes.reserve(baseAreas.size()+totalRuleAreas);
				for (SubCubeList::iterator baseAreaIt = baseAreas.begin(); baseAreaIt != baseAreas.end(); ++baseAreaIt) {
					baseNodes.push_back(PSourcePlanNode(new SourcePlanNode(cube->getNumericStorageId(), baseAreaIt->second, 0)));
				}
				createRuleNodes(baseNodes, baseRulesAreas, 0, true, true);
				PPlanNode aggregationNode = PPlanNode(new AggregationPlanNode(it->second, defaultNumValue, baseNodes, aggregationMaps, useCache ? cube->getCache() : 0, useCache ? cube : CPCube(), AggregationPlanNode::SUM, blockSize));
				spns.push_back(aggregationNode);
			}
		}
		if (anyDirectRules) {
			createRuleNodes(spns, numericConsRulesAreas, defaultNumValue, false, false);
			createRuleNodes(spns, numericBaseRulesAreas, defaultNumValue, skipEmpty, false);
			createRuleNodes(spns, numericMarkedRulesAreas, defaultNumValue, false, true);
			createRuleNodes(spns, stringRulesAreas, defaultStrValue, false, true);
		}
		if (anyCached) {
			double found = 0;
			for (RulesAreas::const_iterator ruleIt = cachedAreas.begin(); ruleIt != cachedAreas.end(); ++ruleIt) {
				CellValue ruleDefaultValue;
				const CellValue *paramDefaultValue = defaultNumValue;
				if (defaultNumValue && ruleIt->first != NO_IDENTIFIER) {
					ruleDefaultValue = CellValue::NullNumeric;
					ruleDefaultValue.setRuleId(ruleIt->first);
					paramDefaultValue = &ruleDefaultValue;
				}
				for (SubCubeList::const_iterator it = ruleIt->second.begin(); it != ruleIt->second.end(); ++it) {
					PPlanNode cacheNode = PPlanNode(new CachePlanNode(it->second, paramDefaultValue, cacheStorage, ruleIt->first));
					spns.push_back(cacheNode);
					found += it->second->getSize();
				}
			}
			if (found) {
				cube->getCache()->increaseFound(found);
			}
		}
		if (anyQueryCached) {
			for (RulesAreas::const_iterator ruleIt = queryCachedAreas.begin(); ruleIt != queryCachedAreas.end(); ++ruleIt) {
				CellValue ruleDefaultValue;
				const CellValue *paramDefaultValue = defaultNumValue;
				if (defaultNumValue && ruleIt->first != NO_IDENTIFIER) {
					ruleDefaultValue = CellValue::NullNumeric;
					ruleDefaultValue.setRuleId(ruleIt->first);
					paramDefaultValue = &ruleDefaultValue;
				}
				for (SubCubeList::const_iterator it = ruleIt->second.begin(); it != ruleIt->second.end(); ++it) {
					PPlanNode cacheNode = PPlanNode(new QueryCachePlanNode(it->second, paramDefaultValue, ruleIt->first, cube));
					spns.push_back(cacheNode);
				}
			}
		}
	}
//	Logger::trace << "subareas S/C/N " << stringAreas.size() << '/' << consolidatedAreas.size() << '/' << numericAreas.size() << endl;
//	if (stringAreas.size()) Logger::trace << "S:" << stringAreas << endl;
//	if (consolidatedAreas.size()) Logger::trace << "C:" << consolidatedAreas << endl;
//	if (numericAreas.size()) Logger::trace << "N:" << numericAreas << endl;

	if (spns.size() == 1) {
		// just one processor
		return spns[0];
	} else if (spns.size() > 1) {
		// more than one processor - make sequence or combination
		return PPlanNode(new PlanNode(blockSize ? COMBINATION : SEQUENCE, area, 0, spns, 0, CPCube()));
	} else {
		return PPlanNode();
	}
}

}
