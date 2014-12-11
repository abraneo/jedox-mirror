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

#include "Olap/Cube.h"
#include "Olap/Database.h"
#include "InputOutput/Condition.h"
#include "InputOutput/ConstantCondition.h"
#include "Engine/DFilterProcessor.h"

namespace palo {

DFilterQuantificationProcessor::DFilterQuantificationProcessor(PEngineBase engine, CPPlanNode node) :
	ProcessorBase(true, engine), init(true), numCellsPerElement(0), strCellsPerElement(0), validValue(false)
{
	const QuantificationPlanNode *plan = dynamic_cast<const QuantificationPlanNode *>(node.get());
	area = plan->getFilteredArea();
	numericArea = plan->getNumericArea();
	quantType = plan->getQuantificationType();
	filteredDim = plan->getDimIndex();
	if (filteredDim == (uint32_t)NO_DFILTER) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "DFilterQuantificationProcessor: invalid filtered dimension");
	}
	filteredDimSize = (uint32_t)area->getDim(filteredDim)->size();
	calcRules = plan->getCalcRules();
	key = *area->pathBegin();
	maxElemCount = plan->getMaxCount();
	condition = plan->getCondition().get();
	if (quantType != QuantificationPlanNode::EXISTENCE && !condition) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "DFilterQuantificationProcessor: invalid condition");
	}
	isVirtual = plan->isVirtual();
	if (!isVirtual && quantType != QuantificationPlanNode::EXISTENCE) {
		if (numericArea->getSize() > 0) {
			numCellsPerElement = 1;
			for (size_t i = 0; i < numericArea->dimCount(); i++) {
				if (i != filteredDim) {
					numCellsPerElement *= numericArea->getDim(i)->size();
				}
			}
		}
		strCellsPerElement = plan->getCellCount() - numCellsPerElement;
	}
}

bool DFilterQuantificationProcessor::next()
{
	if (init) {
		bool bCalc;
		// for virtual cubes calculate the whole area (don't call processEmptyCells)
		// for EXISTENCE skip empty values and don't call processEmptyCells
		// for ALL, ANY_NUM and ANY_STR skip empty values and call processEmptyCells

		if (isVirtual || quantType == QuantificationPlanNode::EXISTENCE) {
			bCalc = true;
		} else {
			bCalc = numCellsPerElement + strCellsPerElement > 0;
		}
		if (bCalc) {
			if (quantType == QuantificationPlanNode::ALL) {
				CPSet filteredSet = area->getDim(filteredDim);
				for (Set::Iterator fit = filteredSet->begin(); fit != filteredSet->end(); ++fit) {
					subset.insert(*fit);
				}
			}

			bool isNumeric = false; // the first area in 'areaList' is numeric, not used for virtual cubes and QuantificationPlanNode::EXISTENCE
			SubCubeList areaList;
			if (isVirtual || quantType == QuantificationPlanNode::EXISTENCE) {
				areaList.push_back(area);
			} else {
				if (numericArea->getSize()) {
					isNumeric = true;
					areaList.push_back(numericArea);

					PCubeArea intersectionArea;
					SubCubeList stringAreas;
					area->intersection(*numericArea.get(), &intersectionArea, &stringAreas);

					for (SubCubeList::iterator it = stringAreas.begin(); it != stringAreas.end(); ++it) {
						areaList.push_back(*it);
					}
				} else {
					areaList.push_back(area);
				}
			}

			bool isComplete = false;
			for (SubCubeList::iterator it = areaList.begin(); it != areaList.end(); ++it) {
				PCubeArea a = PCubeArea(new CubeArea(area->getDatabase(), area->getCube(), *it->second.get()));
				RulesType rulesType = calcRules ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES;

				PPlanNode plan = area->getCube()->createPlan(a, CubeArea::ALL, rulesType, true, UNLIMITED_SORTED_PLAN);
				vector<PPlanNode> children;
				if (plan->getType() == UNION) {
					children = plan->getChildren();
				} else {
					children.push_back(plan);
				}

				CalcNodeType calcType = CALC_UNDEF;
				while (nextCalcNodeType(calcType, false)) {
					for (size_t i = 0; i < children.size(); i++) {
						bool isAggr;
						bool isRule;
						PlanNodeType childType = children[i]->getType();
						if (!matchingPlanNode(calcType, childType, isAggr, isRule)) {
							continue;
						}
						set<IdentifierType> &rset = quantType == QuantificationPlanNode::ALL ? complement : subset;
						PArea rarea = isVirtual ? PArea(new Area(*children[i]->getArea())) : children[i]->getArea()->reduce(filteredDim, rset);
						if (rarea->getSize()) {
							if (quantType == QuantificationPlanNode::EXISTENCE && isAggr && !isVirtual) {
								processAggregation(children[i], rarea, isComplete);
							} else {
								processReducedArea(children[i], rarea, 0, isRule, isComplete, isNumeric);
							}
						}
						if (isComplete) {
							break;
						}
					}
					if (isComplete) {
						break;
					}
				}
				isNumeric = false;
			}
			if (quantType != QuantificationPlanNode::EXISTENCE && !isComplete && !isVirtual) {
				processEmptyCells();
			}
		}
		pos = subset.begin();
		init = false;
	} else {
		++pos;
	}

	if (pos == subset.end()) {
		return false;
	} else {
		key[filteredDim] = *pos;
		validValue = quantType == QuantificationPlanNode::EXISTENCE;
		return true;
	}
}

const CellValue &DFilterQuantificationProcessor::getValue()
{
	if (!validValue) {
		map<IdentifierType, CellValue>::iterator vit = values.find(*pos);
		if (vit == values.end()) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "missing value in DFilterQuantificationProcessor!");
		} else {
			cellValue = vit->second;
			validValue = true;
		}
	}
	return cellValue;
}

void DFilterQuantificationProcessor::reset()
{
	init = true;
	subset.clear();
	complement.clear();
	counterNumTrue.clear();
	counterNumFalse.clear();
	counterStrTrue.clear();
	counterStrFalse.clear();
	values.clear();
	validValue = false;
}

void DFilterQuantificationProcessor::processAggregation(CPPlanNode planNode, PArea rarea, bool &isComplete)
{
	PCubeArea ca(new CubeArea(area->getDatabase(), area->getCube(), *rarea));
	PAggregationMaps aggrMaps(new AggregationMaps());
	ca->expandBase(aggrMaps.get(), &filteredDim);

	const vector<PPlanNode> children = planNode->getChildren();
	CalcNodeType calcType = CALC_UNDEF;

	while (nextCalcNodeType(calcType, true)) {
		for (size_t i = 0; i < children.size(); i++) {
			bool isAggr;
			bool isRule;
			if (!matchingPlanNode(calcType, children[i]->getType(), isAggr, isRule)) {
				continue;
			}

			rarea = PArea(new Area(*children[i]->getArea()->reduce(filteredDim, subset)));
			if (rarea->getSize()) {
				bool isNumeric = false; // doesn't matter for EXISTENCE
				processReducedArea(children[i], rarea, &(*aggrMaps)[filteredDim], isRule, isComplete, isNumeric);
				if (isComplete) {
					return;
				}
			}
		}
	}
}

void DFilterQuantificationProcessor::processReducedArea(CPPlanNode planNode, PArea rarea, AggregationMap *aggrMap, bool isRule, bool &isComplete, bool isNumeric)
{
	PCellStream cs;
	if (planNode->getType() == CACHE) {
		const CachePlanNode *childPlan = dynamic_cast<const CachePlanNode *>(planNode.get());
		// TODO: -jj- check if default value is needed - childPlan->getDefaultValue()
		cs = childPlan->getCacheStorage()->getCellValues(rarea);
	} else if (planNode->getType() == QUERY_CACHE) {
		const QueryCachePlanNode *childPlan = dynamic_cast<const QueryCachePlanNode *>(planNode.get());
		cs = Context::getContext()->getQueryCache(childPlan->getCube())->getFilteredValues(planNode->getArea(), childPlan->getDefaultValue());
	} else {
		PCubeArea childArea(new CubeArea(area->getDatabase(), area->getCube(), *rarea));
		RulesType rulesType = calcRules || planNode->getType() != SOURCE ? RulesType(ALL_RULES | NO_RULE_IDS) : NO_RULES;
		PPlanNode childPlan = area->getCube()->createPlan(childArea, CubeArea::ALL, rulesType, !isVirtual, UNLIMITED_SORTED_PLAN);
		if (childPlan) {
			cs = area->getCube()->evaluatePlan(childPlan, EngineBase::ANY, true);
		} else {
			return;
		}
	}

	while (cs->next()) {
		IdentifierType id = cs->getKey()[filteredDim];
		const CellValue &val = cs->getValue();
		if (aggrMap) {
			for (AggregationMap::TargetReader targets = aggrMap->getTargets(id); !targets.end(); ++targets) {
				checkValue(*targets, val, isComplete, isNumeric);
				if (isComplete) {
					break;
				}

			}
		} else {
			checkValue(id, val, isComplete, isNumeric);
		}
		if (isComplete) {
			return;
		}
	}
}

void DFilterQuantificationProcessor::processEmptyCells()
{
	bool zeroCond = condition->check(quantType == QuantificationPlanNode::ANY_STR ? CellValue::NullString : CellValue::NullNumeric);
	CPSet filteredSet = area->getDim(filteredDim);

	const IdentifiersType *dims = area->getCube()->getDimensions();
	Dimension *dim = area->getDatabase()->lookupDimension(dims->at(filteredDim), false).get();
	double numCellCount;
	double strCellCount;

	if (quantType == QuantificationPlanNode::ALL) {
		complement.clear();
		for (Set::Iterator fit = filteredSet->begin(); fit != filteredSet->end(); ++fit) {
			IdentifierType id = *fit;
			getCounts(dim, id, numCellCount, strCellCount);

			set<IdentifierType>::iterator sit = subset.find(id);
			if (sit != subset.end()) {
				if (counterNumFalse.find(id) != counterNumFalse.end() || counterStrFalse.find(id) != counterStrFalse.end()) { // the condition failed for at least one value
					values.erase(id);
					subset.erase(id);
					continue;
				}

				if (strCellCount) {
					map<IdentifierType, double>::iterator mit = counterStrTrue.find(id);
					if (mit == counterStrTrue.end() || mit->second < strCellCount) { // there is at least one empty string cell for 'id'
						values.erase(id);
						subset.erase(id);
						continue;
					}
				}

				if (numCellCount) {
					map<IdentifierType, double>::iterator mit = counterNumTrue.find(id);
					if (mit == counterNumTrue.end() || mit->second < numCellCount) { // the condition was not met for all values
						if (zeroCond) {
							values.insert(make_pair(id, CellValue::NullNumeric));
						} else {
							values.erase(id);
							subset.erase(id);
						}
					}
				}
			}
		}
		checkLimit();
	} else { // QuantificationPlanNode::ANY_NUM and ANY_STR
		if (zeroCond) {
			bool isComplete = false;
			for (Set::Iterator fit = filteredSet->begin(); fit != filteredSet->end(); ++fit) {
				IdentifierType id = *fit;
				getCounts(dim, id, numCellCount, strCellCount);

				set<IdentifierType>::iterator sit = subset.find(id);
				if (sit == subset.end()) {
					if (quantType == QuantificationPlanNode::ANY_NUM) {
						if (numCellCount) {
							map<IdentifierType, double>::iterator mit = counterNumFalse.find(id);
							if (mit == counterNumFalse.end() || mit->second < numCellCount) {
								// if there is at least one numeric #N/A there add 'id' to 'subset'
								insertId(id, CellValue::NullNumeric, isComplete);
							}
						}
					} else { // QuantificationPlanNode::ANY_STR
						if (strCellCount) {
							map<IdentifierType, double>::iterator mit = counterStrFalse.find(id);
							if (mit == counterStrFalse.end() || mit->second < strCellCount) {
								// if there is at least one string #N/A there add 'id' to 'subset'
								insertId(id, CellValue::NullString, isComplete);
							}
						}
					}
					if (isComplete) {
						break;
					}
				}
			}
		}
	}
}

bool DFilterQuantificationProcessor::checkValue(IdentifierType id, const CellValue &value, bool &isComplete, bool isNumeric)
{
	bool changed = false;
	if (quantType == QuantificationPlanNode::ALL) {
		if (subset.find(id) != subset.end()) {
			bool cond = value.isNumeric() && condition->check(value);
			increaseCounter(id, cond, isNumeric);
			if (cond) {
				values.insert(make_pair(id, value));
			} else {
				values.erase(id);
				subset.erase(id);
				complement.insert(id);
				changed = true;
			}
		}
	} else {
		bool cond = false;
		if (quantType == QuantificationPlanNode::ANY_NUM || quantType == QuantificationPlanNode::ANY_STR) {
			if (quantType == QuantificationPlanNode::ANY_NUM) {
				cond = value.isNumeric() && condition->check(value);
			} else {
				cond = value.isString() && condition->check(value);
			}
			increaseCounter(id, cond, isNumeric);
		}
		if (subset.find(id) == subset.end()) {
			if (quantType == QuantificationPlanNode::EXISTENCE || cond) {
				insertId(id, value, isComplete);
				changed = true;
			}
		}
	}
	return changed;
}

void DFilterQuantificationProcessor::insertId(IdentifierType id, const CellValue &value, bool &isComplete)
{
	checkLimit();
	subset.insert(id);
	values.insert(make_pair(id, value));
	isComplete = subset.size() == filteredDimSize;
}

void DFilterQuantificationProcessor::increaseCounter(IdentifierType id, bool cond, bool isNumeric)
{
	map<IdentifierType, double> &counter = isNumeric ? (cond ? counterNumTrue : counterNumFalse) : (cond ? counterStrTrue : counterStrFalse);
	map<IdentifierType, double>::iterator it = counter.find(id);
	if (it == counter.end()) {
		counter.insert(make_pair(id, 1.0));
	} else {
		it->second = it->second + 1.0;
	}
}

void DFilterQuantificationProcessor::checkLimit() const
{
	if (maxElemCount && subset.size() >= maxElemCount) {
		throw ErrorException(ErrorException::ERROR_MAX_ELEM_REACHED, "subset too big");
	}
}

bool DFilterQuantificationProcessor::nextCalcNodeType(CalcNodeType &calcType, bool skipAggregation)
{
	switch (calcType) {
	case CALC_UNDEF:
		calcType = CALC_SOURCE;
		break;
	case CALC_SOURCE:
		calcType = CALC_CACHE;
		break;
	case CALC_CACHE:
		calcType = skipAggregation ? CALC_RULE : CALC_AGGREGATION;
		break;
	case CALC_AGGREGATION:
		calcType = CALC_RULE;
		break;
	case CALC_RULE:
		return false;
	}
	return true;
}

bool DFilterQuantificationProcessor::matchingPlanNode(CalcNodeType calcType, PlanNodeType planType, bool &isAggr, bool &isRule)
{
	switch (planType) {
	case SOURCE:
	case CACHE:
	case QUERY_CACHE:
		isAggr = false;
		isRule = false;
		break;
	case ADDITION:
	case SUBTRACTION:
	case MULTIPLICATION:
	case DIVISION:
	case LEGACY_RULE:
	case TRANSFORMATION:
	case CONSTANT:
	case COMPLETE:
		isAggr = false;
		isRule = true;
		break;
	case AGGREGATION:
		isAggr = true;
		isRule = false;
		break;
	case CELLMAP:
	case CELL_RIGHTS:
	case ROUNDCORRECT:
	case QUANTIFICATION:
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid PlanNode type in DFilterQuantificationProcessor!");
	default:
		throw ErrorException(ErrorException::ERROR_INTERNAL, "unknown PlanNode type in DFilterQuantificationProcessor!");
	}

	switch (calcType) {
	case CALC_UNDEF:
		return false;
	case CALC_SOURCE:
		return planType == SOURCE;
	case CALC_CACHE:
		return planType == CACHE || planType == QUERY_CACHE;
	case CALC_AGGREGATION:
		return planType == AGGREGATION;
	case CALC_RULE:
		return planType != SOURCE && planType != CACHE && planType != AGGREGATION;
	}
	return false;
}

void DFilterQuantificationProcessor::getCounts(Dimension *dim, IdentifierType elemId, double &numCellCount, double &strCellCount) const
{
	Element *elem = dim->lookupElement(elemId, false);
	if (elem->getElementType() == Element::STRING) {
		numCellCount = 0;
		strCellCount = numCellsPerElement + strCellsPerElement;
	} else {
		numCellCount = numCellsPerElement;
		strCellCount = strCellsPerElement;
	}
}

}
