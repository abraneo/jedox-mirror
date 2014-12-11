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

EngineCpu::EngineCpu() : EngineBase("CPU", EngineBase::CPU)
{
}

EngineCpu::EngineCpu(const EngineCpu &s) : EngineBase(s)
{
}

EngineCpu::~EngineCpu()
{

}

//bool EngineCpu::merge(const CPCommitable &o, const PCommitable &p)
//{
//	checkCheckedOut();
//	bool ret = true;
//	CPEngineCpu eng = CONST_COMMITABLE_CAST(EngineCpu, o);
//	CPEngineCpu oldeng = CONST_COMMITABLE_CAST(EngineCpu, old);
//	if (old != 0) {
//		if (eng != 0) {
//			if (type == oldeng->type) {
//				type = eng->type;
//			}
//		}
//	}
//	if (storageList->isCheckedOut()) {
//		ret = storageList->merge(o != 0 ? eng->storageList : PStorageList(), shared_from_this());
//	} else if (o != 0) {
//		storageList = eng->storageList;
//	}
//	if (ret) {
//		commitintern();
//	}
//	return ret;
//}

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
	if (id == NO_IDENTIFIER) {
		id = storageIdHolder->getNewId();
	} else {
		storage = storageList->get(id, true);
	}
	if (storage == 0) {
		switch (type) {
		case Numeric:
			storage.reset(new StorageCpu(pathTranslator));
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

PCellStream EngineCpu::createProcessor(CPPlanNode node, bool useCache)
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
			PCellStream processor = PCellStream(new CellRightProcessor(cellRightsPlanNode->getCubeArea(), user, cellRightsPlanNode->isForPropertyCube()));
			return processor;
		}
		case LEGACY_RULE:
		{
			const LegacyRulePlanNode *legacyRulePlanNode = static_cast<const LegacyRulePlanNode *>(node.get());
			CPRule rule = legacyRulePlanNode->getRule();
			if (legacyRulePlanNode->useMarkers()) {
				PCellStream processor = PCellStream(new LegacyMarkedRule(COMMITABLE_CAST(EngineBase, shared_from_this()), node));
				if (legacyRulePlanNode->getDefaultValue()) {
					processor = PCellStream(new RepeaterProcessor(legacyRulePlanNode->getArea(), *legacyRulePlanNode->getDefaultValue(), processor));
				}
				if (useCache && legacyRulePlanNode->getCache()) {
					processor = legacyRulePlanNode->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *legacyRulePlanNode->getArea())), processor, legacyRulePlanNode->getRule()->getId());
				}
				return processor;
			} else {
				PCellStream processor = PCellStream(new LegacyRule(COMMITABLE_CAST(EngineBase, shared_from_this()), node));
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
//						cout << "D\t" << *node->getArea() << endl;
						PCellStream result = cachedAggrProc->getCalculatedValues();
						if (result) {
							return result;
						}
					}
				}
			}

//			const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(node.get());
//			if (apn->getCacheCube()) {
//				CPDatabase adb = CONST_COMMITABLE_CAST(Database, apn->getCacheCube());
//				if (adb) {
//					PCubeArea ca(new CubeArea(adb, apn->getCacheCube(), *node->getArea()));
//					PPlanNode newPlan = apn->getCacheCube()->createPlan(ca, CubeArea::CONSOLIDATED, NO_RULES, node->getDefaultValue() == NULL, UNLIMITED_SORTED_PLAN);
//					if (newPlan && newPlan->getType() != AGGREGATION) {
//						return createProcessor(newPlan, useCache);
//					}
//				}
//			}
//			PCellStream ret;
//			PPlanNode newPlan = node->getC
//			if (apn->getAggregationType() == AggregationPlanNode::SUM) {
//				ret.reset(new AggregationProcessor(thisEngine, node));
//				if (useCache && node->getCache()) {
//					ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret, NO_IDENTIFIER);
//				}
//			} else {
//				ret.reset(new AggregationFunctionProcessor(thisEngine, node));
//			}
//			return ret;
		}
		default: ;
	}
	return EngineBase::createProcessor(node);
}

bool EngineCpu::isPlanSupported(CPPlanNode node) const
{
	// cpu can calculate anything
	return true;
}

ConstantRepeater::ConstantRepeater(CPArea cpArea, const CellValue *defaultValue) : cpArea(cpArea), area(cpArea.get()), defaultValue(defaultValue), repeatCount(0)
{
	if (defaultValue) {
		defaultValueCopy = *defaultValue;
		this->defaultValue = &defaultValueCopy;
		if (area) {
			areaIt = area->pathBegin();
		}
	}
	if (area) {
		pathTranslator = area->getPathTranslator();
	}
}

ConstantRepeater::ConstantRepeater(const ConstantRepeater &other) : cpArea(other.cpArea), area(cpArea.get()), defaultValue(other.defaultValue), areaIt(other.areaIt), repeatCount(other.repeatCount)
{
	if (defaultValue) {
		defaultValueCopy = *defaultValue;
		defaultValue = &defaultValueCopy;
	}
}

const GpuBinPath &ConstantRepeater::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

uint64_t ConstantRepeater::setRepetition(const IdentifiersType *afterKey, const IdentifiersType *toKey)
{
	if (defaultValue && areaIt != area->pathEnd()) { // only if default value exists and repetition to the end was not set yet
		if (!afterKey) {
			// starting from beginning
			areaIt = area->pathBegin();
		} else {
			if (*areaIt == *afterKey && toKey) { //check quickly distance 0
				++areaIt;
				if (*areaIt == *toKey) {
					repeatCount = 0;
					return repeatCount;
				}
			}
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

void ConstantRepeater::reset()
{
	repeatCount = 0;
	if (area && defaultValue) {
		areaIt = area->pathBegin();
	}
}

RepeaterProcessor::RepeaterProcessor(CPArea area, const CellValue &defaultValue, PCellStream sourceProcessor)
	: area(area), defaultValue(defaultValue), sourceProcessor(sourceProcessor), source(sourceProcessor.get()), repeatCount(0), sourceEnded(false), bFirst(true)
{
	areaIt = area->pathBegin();
}

bool RepeaterProcessor::next()
{
	if (isDefaultValue()) {
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

void RepeaterProcessor::reset()
{
	repeatCount = 0;
	areaIt = area->pathBegin();
}

const IdentifiersType &RepeaterProcessor::getKey() const
{
	if (isDefaultValue()) {
		return *areaIt;
	} else {
		return source->getKey();
	}
}

const GpuBinPath &RepeaterProcessor::getBinKey() const
{
	area->getPathTranslator()->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

const CellValue &RepeaterProcessor::getValue()
{
	if (isDefaultValue()) {
		return defaultValue;
	} else {
		return source->getValue();
	}
}

double RepeaterProcessor::getDouble()
{
	return getValue().getNumeric();
}

uint64_t RepeaterProcessor::setRepetition(const IdentifiersType *afterKey, const IdentifiersType *toKey)
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

string LegacyRulePlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " Cube='" << cube->getId() << "' RuleId='" << rule->getId() << "' Markers='" << (useMarkers() ? "yes" : "no") << "'";
	return ss.str();
}

}
