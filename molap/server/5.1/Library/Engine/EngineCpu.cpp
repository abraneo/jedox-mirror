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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "palo.h"
#include "Olap/Commitable.h"
#include "Olap/Rule.h"
#include "Engine/EngineCpu.h"
#include "Engine/StorageCpu.h"
#include "Engine/LegacySource.h"
#include "Engine/CellRightProcessor.h"
#include "Engine/AggregationProcessor.h"

namespace palo {

EngineCpu::EngineCpu() :
		EngineBase("CPU", EngineBase::CPU)
{
}

EngineCpu::EngineCpu(const EngineCpu &s) :
		EngineBase(s)
{
}

EngineCpu::~EngineCpu()
{

}

PCommitable EngineCpu::copy() const
{
	checkNotCheckedOut();
	PEngineBase newd(new EngineCpu(*this));
	return newd;
}

PStorageBase EngineCpu::getCreateStorage(IdentifierType &id, PPathTranslator pathTranslator, StorageType type)
{
	checkCheckedOut();
	PCommitable storage;
	if (id == NO_IDENTIFIER ) {
		id = storageIdHolder->getNewId();
	} else {
		storage = storageList->get(id, true);
	}
	if (storage == 0) {
		switch (type) {
		case Numeric:
			storage.reset(new StorageCpu(pathTranslator, true));
			break;
		case String:
			storage.reset(new StringStorageCpu(pathTranslator));
			break;
		case Marker:
			storage.reset(new MarkerStorageCpu(pathTranslator));
			break;
		}
		storage->setID(id);
		getStorageList(true)->add(storage, false);
	} else {
		getStorageList(true)->set(storage);
	}
	return COMMITABLE_CAST(StorageBase, storage);
}

PProcessorBase EngineCpu::createProcessor(CPPlanNode node, bool sortedOutput, bool useCache)
{
	PEngineBase thisEngine = COMMITABLE_CAST(EngineBase, shared_from_this());

	switch (node->getType()) {
		case CELL_RIGHTS:
		{
			const CellRightsPlanNode *cellRightsPlanNode = static_cast<const CellRightsPlanNode *>(node.get());
			PUser user;
			boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
			if (session) {
				user = session->getUser();
			}
			PProcessorBase processor = PProcessorBase(new CellRightProcessor(cellRightsPlanNode->getCubeArea(), user, cellRightsPlanNode->isForPropertyCube()));
			return processor;
		}
		case LEGACY_RULE:
		{
			CPLegacyRulePlanNode legacyRulePlanNode = boost::dynamic_pointer_cast<const LegacyRulePlanNode, const PlanNode>(node);
			CPRule rule = legacyRulePlanNode->getRule();
			if (legacyRulePlanNode->useMarkers()) {
				PProcessorBase processor = PProcessorBase(new LegacyMarkedRule(thisEngine, legacyRulePlanNode));
				if (useCache && legacyRulePlanNode->getCache()) {
					processor = legacyRulePlanNode->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *legacyRulePlanNode->getArea())), processor, legacyRulePlanNode->getRule()->getId());
				}
				if (legacyRulePlanNode->getDefaultValue()) {
					processor = PProcessorBase(new CompleteProcessor(legacyRulePlanNode->getArea(), legacyRulePlanNode->getDefaultValue(), NO_RULE, processor));
				}
				return processor;
			} else {
				PProcessorBase processor = PProcessorBase(new LegacyRule(thisEngine, legacyRulePlanNode));
				if (useCache && legacyRulePlanNode->getCache()) {
					processor = legacyRulePlanNode->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *legacyRulePlanNode->getArea())), processor, legacyRulePlanNode->getRule()->getId());
				}
				return processor;
			}
		}
		case AGGREGATION: {
			const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(node.get());
			Context *context = Context::getContext();
			for (Context::CacheDependences::const_iterator cacheIt = context->getCacheDependences().begin(); cacheIt != context->getCacheDependences().end(); ++cacheIt) {
				ValueCache::CacheWriteProcessor *cacheProc = (*cacheIt);
				if (cacheProc->getCubeArea()->getCube() == apn->getCacheCube() && *node->getArea() == *cacheProc->getCubeArea()) {
					PCellStream cacheInput = cacheProc->getInputProcessor();
					AggregationProcessor *cachedAggrProc = dynamic_cast<AggregationProcessor*>(cacheProc->getInputProcessor().get());
					if (cachedAggrProc) {
						PProcessorBase result = cachedAggrProc->getCalculatedValues();
						if (result) {
//							cout << "D\t" << *node->getArea() << endl;
							return result;
						}
					}
				}
			}
		}
		default: ;
	}
	return EngineBase::createProcessor(node, sortedOutput, useCache);
}

bool EngineCpu::isPlanSupported(CPPlanNode node) const
{
	// cpu can calculate anything
	return true;
}

CompleteProcessor::CompleteProcessor(CPArea area, const CellValue *defaultValue, IdentifierType ruleId, PProcessorBase sourceProcessor) :
	ProcessorBase(true, PEngineBase()), area(area), defaultValue(defaultValue ? &defaultValueCopy : 0), defaultValueCopy(defaultValue ? *defaultValue : CellValue::NullNumeric),
	sourceProcessor(sourceProcessor), source(sourceProcessor.get()), repeatCount(0), sourceEnded(false), bFirst(true), ruleId(ruleId)
{
	areaIt = area->pathBegin();
}

CompleteProcessor::CompleteProcessor(PEngineBase engine, CPArea area, const CellValue *defaultValue, IdentifierType ruleId, CPPlanNode child) :
	ProcessorBase(true, engine, false), area(area), defaultValue(defaultValue ? &defaultValueCopy : 0),
	defaultValueCopy(defaultValue ? *defaultValue : CellValue::NullNumeric), repeatCount(0), sourceEnded(false), bFirst(true), ruleId(ruleId)
{
	areaIt = area->pathBegin();
	sourceProcessor = createProcessor(child, true);
	source = sourceProcessor.get();
}

bool CompleteProcessor::next()
{
	if (!area || !defaultValue) {
		return source->next();
	} else if (isDefaultValue()) {
		bFirst = false;
		if (nextRepetition()) {
			// R
			return true;
		} else {
			// use current existing V
			return !sourceEnded;
		}
	} else {
		IdentifiersType prevKey;
		if (source) {
			// search for next existing value (V | R | E)
			bool result = false;
			if (bFirst) {
				bFirst = false;
			} else {
				prevKey = source->getKey();
			}

			while (!sourceEnded) {
				result = source->next();
				if (result) {
					setRepetition(prevKey.empty() ? 0 : &prevKey, &getKey());
					return true;
				} else {
					sourceEnded = true;
				}
			}
		} else {
			bFirst = false;
		}
		return 0 != setRepetition(prevKey.empty() ? 0 : &prevKey, 0);
	}
}

void CompleteProcessor::reset()
{
	repeatCount = 0;
	areaIt = area->pathBegin();
}

const IdentifiersType &CompleteProcessor::getKey() const
{
	if (isDefaultValue()) {
		return *areaIt;
	} else {
		return source->getKey();
	}
}

const GpuBinPath &CompleteProcessor::getBinKey() const
{
	area->getPathTranslator()->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

const CellValue &CompleteProcessor::getValue()
{
	if (isDefaultValue()) {
		return *defaultValue;
	} else if (ruleId != NO_RULE) {
		tmpValue = source->getValue();
		tmpValue.setRuleId(ruleId);
		return tmpValue;
	} else {
		return source->getValue();
	}
}

double CompleteProcessor::getDouble()
{
	return getValue().getNumeric();
}

uint64_t CompleteProcessor::setRepetition(const IdentifiersType *afterKey, const IdentifiersType *toKey)
{
	if (areaIt != area->pathEnd()) { // only if repetition to the end was not set yet
		if (!afterKey) {
			// starting from beginning
			areaIt = area->pathBegin();
		} else {
			// get start from last key
			areaIt = area->find(*afterKey);
			++areaIt;
		}
		// convert newKey to PathIterator
		Area::PathIterator endIt;
		if (toKey) {
			endIt = area->find(*toKey);
		} else {
			endIt = area->pathEnd();
		}
		//calculate distance
		double distance = endIt - areaIt;
		repeatCount = (uint64_t)distance;
		if (distance != (double)repeatCount) {
			repeatCount = 0;
			throw ErrorException(ErrorException::ERROR_INTERNAL, "too many generated constant values!");
		}
		return repeatCount;
	} else {
		return 0;
	}

}

LegacyRulePlanNode::LegacyRulePlanNode(CPDatabase db, CPCube cube, CPArea area, CPRule rule, const CellValue *defaultValue, ValueCache *cache, bool markers)
: PlanNode(LEGACY_RULE, area, vector<PPlanNode>(), cache, cube), CompleteNodeInfo(defaultValue, rule->getId()),
  db(db), cube(cube), rule(rule), markers(markers)
{
}

void LegacyRulePlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// write rule identification including revision here
	w.appendIdentifier(getDatabase()->getId(),'/');
	w.appendIdentifier(getCube()->getId(),'/');
	w.appendIdentifier(getRule()->getId(),'/');
	w.appendIdentifier((IdentifierType)getDatabase()->getObjectRevision(),0); // TODO: -jj- serialize uint64_t
}

bool LegacyRulePlanNode::isEqual(const PlanNode& b) const
{
	const LegacyRulePlanNode &o = dynamic_cast<const LegacyRulePlanNode &>(b);
	return db == o.db && cube == o.cube && rule == o.rule && markers == o.markers && CompleteNodeInfo::isEqual(o);
}

string LegacyRulePlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " Cube='" << cube->getId() << "' RuleId='" << rule->getId() << "' Markers='" << (useMarkers() ? "yes" : "no") << "'";
	return ss.str();
}

}
