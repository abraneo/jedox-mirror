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

#ifndef OLAP_DFILTER_PROCESSOR_H
#define OLAP_DFILTER_PROCESSOR_H 1

#include "palo.h"
#include "Engine/EngineBase.h"
#include "Engine/AggregationProcessor.h"

namespace palo {
class Condition;

class SERVER_CLASS DFilterQuantificationProcessor : public ProcessorBase {
public:
	DFilterQuantificationProcessor(PEngineBase engine, CPPlanNode node);
	virtual ~DFilterQuantificationProcessor() {}

	virtual bool next();
	const CellValue &getValue();
	virtual double getDouble() {
		return getValue().getNumeric();
	}
	virtual const IdentifiersType &getKey() const {
		return key;
	}
	virtual void reset();

private:
	enum CalcNodeType { //preferred order of PlanNode processing
		CALC_UNDEF = 0, CALC_SOURCE, CALC_CACHE, CALC_AGGREGATION, CALC_RULE
	};
	static bool nextCalcNodeType(CalcNodeType &calcType, bool skipAggregation);
	static bool matchingPlanNode(CalcNodeType calcType, PlanNodeType planType, bool &isAggr, bool &isRule);

	void processAggregation(CPPlanNode planNode, PArea rarea, bool &isComplete);
	void processReducedArea(CPPlanNode planNode, PArea rarea, AggregationMap *aggrMap, bool isRule, bool &isComplete, bool isNumeric);
	void processEmptyCells();
	bool checkValue(IdentifierType id, const CellValue &value, bool &isComplete, bool isNumeric);
	void insertId(IdentifierType id, const CellValue &val, bool &isComplete);
	void increaseCounter(IdentifierType id, bool cond, bool isNumeric);
	void checkLimit() const;
	void getCounts(Dimension *dim, IdentifierType elemId, double &numCellCount, double &strCellCount) const;

	PCubeArea area;
	CPCubeArea numericArea;
	bool init;
	QuantificationPlanNode::QuantificationType quantType;
	uint32_t filteredDim;
	uint32_t filteredDimSize;
	bool calcRules;
	CellValue cellValue;
	IdentifiersType key;
	uint64_t maxElemCount;
	const Condition *condition;
	bool isVirtual;
	double numCellsPerElement;
	double strCellsPerElement;
	set<IdentifierType> subset;
	set<IdentifierType> complement;
	set<IdentifierType>::iterator pos; // iterator in subset
	map<IdentifierType, double> counterNumTrue;
	map<IdentifierType, double> counterNumFalse;
	map<IdentifierType, double> counterStrTrue;
	map<IdentifierType, double> counterStrFalse;
	map<IdentifierType, CellValue> values;
	bool validValue;
};

}

#endif
