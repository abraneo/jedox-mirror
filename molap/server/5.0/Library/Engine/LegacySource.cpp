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

#include "Engine/LegacySource.h"
#include "Engine/Legacy/Engine.h"
#include "Engine/Legacy/VirtualMachine.h"
#include "Engine/EngineCpu.h"
#include "InputOutput/Statistics.h"
#include "Olap/Database.h"
#include "Olap/Rule.h"
#include "Olap/RuleMarker.h"

using namespace paloLegacy;

namespace palo {

LegacyRule::LegacyRule(PEngineBase engine, CPPlanNode node) :
	engine(engine), isValidPath(false), node(node)
{
	Context *context = Context::getContext();
	const LegacyRulePlanNode *legacyRulePlanNode = dynamic_cast<const LegacyRulePlanNode *>(node.get());
	cube = legacyRulePlanNode->getCube();
	db = CONST_COMMITABLE_CAST(Database, context->getParent(cube));
	mem_context = context->getRulesContext();
	ECube *ecube = NewEntryCube(*cube, *db, *engine, context);
	CPRule rule = legacyRulePlanNode->getRule();
	generateEmptyResults = node->getDefaultValue() != 0;

	area.reset(new CubeArea(db, cube, *node->getArea().get()));
	erule = ecube->findRule(rule->getId());
	if (erule && erule->arule) {
		generateSources();
	} else {
		stringstream msg;
		msg << "rule " << rule->getId() << " not found in database " <<  db->getName() << ", cube " << cube->getName() << ", erule";
		if (erule) {
			msg << "->nr_rule: " << erule->nr_rule << ", erule->arule: NULL";
		} else {
			msg << ": NULL";
		}
		Logger::error << msg.str() << endl;
		throw ParameterException(ErrorException::ERROR_RULE_NOT_FOUND, "rule not found in cube", "cube", (int)cube->getId(), "rule", (int)(erule ? erule->nr_rule : NO_IDENTIFIER));
	}
}

LegacyRule::~LegacyRule()
{
}

void LegacyRule::generateSources()
{
	// if caching enabled
	if (cube->getCacheBarrier() > 0) {
		Context *context = Context::getContext();
		PServer server = context->getServer();
		// precalc markers and store results in cache
		const vector<PRuleMarker>& markers = erule->arule->getMarkers();
		for (vector<PRuleMarker>::const_iterator markerIt = markers.begin(); markerIt != markers.end(); ++markerIt) {
			dbID_cubeID dbCube = (*markerIt)->getFromDbCube();
			if ((*markerIt)->isMultiplicating() || (dbCube.first == db->getId() && dbCube.second == cube->getId())) {
				continue;
			}
			CPDatabase sourceDb = server->lookupDatabase(dbCube.first, false);
			if (sourceDb) {
				CPCube sourceCube = sourceDb->lookupCube(dbCube.second, false);
				if (sourceCube) {
					size_t targetDimmCount = cube->getDimensions()->size();

					PCubeArea sourceCubeArea(new CubeArea(sourceDb, sourceCube, *(*markerIt)->getSourceArea()));	// TODO: -jj- getFromBase->getSourceArea
					Logger::trace << "Prefetching Marker From Base: " << *(*markerIt)->getFromBase() << endl;
					// transform target area into source and calculate source
					const int16_t *permutation = (*markerIt)->getPermutations();
					const RuleMarker::PMappingType *mapping = (*markerIt)->getMapping();

					for (size_t dim = 0; dim < targetDimmCount; dim++) {
						if (permutation[dim] != -1) {
							if (mapping[dim] == 0) {
								sourceCubeArea->insert(permutation[dim], area->getDim(dim));
							} else {
								PSet sourceSet(new Set());
								const Set &targetSet = *area->getDim(dim);
								// remap the set
	//							for (RuleMarker::MappingType::iterator mit = mapping[dim]->begin(); mit != mapping[dim]->end(); ++mit) {
	//								if (targetSet.find(mit->second) != targetSet.end()) {
	//									sourceSet->insert(mit->first);
	//								}
	//							}
								// use the reverse map
								map<uint32_t, uint32_t>& reverseMap = mapping[dim]->getReverseMap();
								for (Set::Iterator sit = targetSet.begin(); sit != targetSet.end(); ++sit) {
									map<uint32_t, uint32_t>::iterator rmit = reverseMap.find(*sit);
									if (rmit != reverseMap.end()) {
										sourceSet->insert(rmit->second);
									}
								}

								if (sourceSet->size()) {
									sourceCubeArea->insert(permutation[dim], sourceSet);
								} else {
									// do not calculate this source
									sourceCubeArea.reset();
									break;
								}
							}
						}
					}
					if (sourceCubeArea) {
						uint64_t barrier = (uint64_t)cube->getCacheBarrier();
						Logger::trace << "Target area: " << *area << endl;
						Logger::trace << "Source area: " << *sourceCubeArea << endl;
						// calculate the area
						PCellStream src = sourceCube->calculateArea(sourceCubeArea, CubeArea::ALL, ALL_RULES, true, barrier);
						if (src) {
							uint64_t srcCount = 0;
							while (src->next() && srcCount < barrier) {
								srcCount++;
							}
							Logger::trace << "Source values: " << srcCount << endl;
						}
					}
				}
			}
		}
	}

	sourceStreamsSP.resize(erule->gc_copy_nr+1);
	sourceStreams.resize(erule->gc_copy_nr+1);
	sourceStreamsNext.resize(erule->gc_copy_nr+1);
	sourceCubeAreas.resize(erule->gc_copy_nr+1);
	for (uint32_t source = 0; source < erule->gc_copy_nr; source++) {
		if (!erule->source_precalc[source]) {
			continue;
		}
		// transform source area
		IdentifiersType transformationMask(erule->cube->nrDimensions, NO_IDENTIFIER);
		SetMultimaps transformationMultiMap;
		sourceCubeAreas[source] = PCubeArea(new CubeArea(*area));
		for (size_t dim = 0; dim < erule->cube->nrDimensions; dim++) {
			if (erule->copy_mask[source][dim] == AreaNode::ABS_RESTRICTION) {
				IdentifierType sourceId = erule->copy_source[source][dim];
				PSet s(new Set);
				s->insert(sourceId);
//				CPSet targetSet = area->getDim(dim);
//				if (targetSet->size() > 1) {
//					// multimap
//					if (transformationMultiMap.empty()) {
//						transformationMultiMap.resize(erule->cube->nrDimensions);
//					}
//					if (!transformationMultiMap[dim]) {
//						transformationMultiMap[dim] = PSetMultimap(new SetMultimap());
//					}
//					for (Set::Iterator targetId = targetSet->begin(); targetId != targetSet->end(); ++targetId) {
//						transformationMultiMap[dim]->insert(SetMultimap::value_type(sourceId, *targetId));
//					}
//				} else {
//					// single element
//					transformationMask[dim] = *(targetSet->begin());
//				}
				sourceCubeAreas[source]->insert(dim, s);
			}
		}
		ErrorException::ErrorType stackeError = mem_context->m_recursion_stack.push(sourceCubeAreas[source], erule->arule);
		if (stackeError) {
			// all cells of the area have error result
			globalError = CellValue(stackeError);
			globalError.setRuleId(erule->nr_rule);
			break;
		}

		PPlanNode sourcePlan = cube->createPlan(sourceCubeAreas[source], CubeArea::ALL, ALL_RULES, true, UNLIMITED_SORTED_PLAN);
		sourcePlan = PPlanNode(new TransformationPlanNode(area, sourcePlan, transformationMask, transformationMultiMap, 1.0));
//		Logger::trace << "Transformed Plan: " << sourcePlan->toXML() << endl;
		PCellStream sourceStream = cube->evaluatePlan(sourcePlan, EngineBase::ANY);
		sourceStreamsSP[source] = sourceStream;
		sourceStreamsNext[source] = sourceStream->next();
		if (!sourceStreamsNext[source]) {
			sourceStreamsSP[source].reset();
		}
		sourceStreams[source] = sourceStream.get();
		mem_context->m_recursion_stack.pop();
	}
	if (erule->precalcStet && !globalError.isError()) {
		uint32_t source = erule->gc_copy_nr;
		PPlanNode sourcePlan = cube->createPlan(area, CubeArea::ALL, RulesType(INDIRECT_RULES | NOCACHE), true, UNLIMITED_SORTED_PLAN);
		PCellStream sourceStream = cube->evaluatePlan(sourcePlan, EngineBase::ANY);
		sourceStreamsSP[source] = sourceStream;
		sourceStreamsNext[source] = sourceStream->next();
		if (!sourceStreamsNext[source]) {
			sourceStreamsSP[source].reset();
		}
		sourceStreams[source] = sourceStream.get();
	}
}

void LegacyRule::computeCell()
{

	if (globalError.isError()) {
		value = globalError;
		return;
	}
	size_t size = mem_context->m_recursion_stack.size();
	try {
		double defValue = 0;
		bool notFoundStatus = true;

		// find sources
		size_t precalculatedSourcesCount = sourceStreamsSP.size();
		vector<CellValueStream *> sourceStreamsCopy(precalculatedSourcesCount, 0);
		for (uint32_t source = 0; source < precalculatedSourcesCount; source++) {
			while (sourceStreamsNext[source]) {
				const IdentifiersType &sourceKey = sourceStreams[source]->getKey();
				int keyCmp = CellValueStream::compare(sourceKey, vkey);

				if (keyCmp < 0) {
//					mem_context->m_recursion_stack.push(sourceCubeAreas[source]);
					sourceStreamsNext[source] = sourceStreams[source]->next();
					if (!sourceStreamsNext[source]) {
						sourceStreamsSP[source].reset();
						sourceStreams[source] = 0;
					}
//					mem_context->m_recursion_stack.pop();
				} else if (keyCmp == 0) {
					sourceStreamsCopy[source] = sourceStreams[source];
					break;
				} else {
					break;
				}
			}
		}

		// apply the rule
		virtual_machine machine(mem_context, &value);
		machine.setSourceStreams(&sourceStreamsCopy);
		machine.setCache(&vmCache);

		// Todo: -jj- not optimal - instead of using CellStatus::NotFoundStatus we should define LegacyUbmMarkedRule and define source processor driving the calculation
		EPath vkeyCopy;
		memcpy(vkeyCopy, &vkey[0], vkey.size() * sizeof(IdentifierType));
		machine.compute(engine.get(), &vkeyCopy[0], erule, defValue, notFoundStatus);
		if (value.isEmpty() && node->getDefaultValue()) {	// if value is empty - make sure it has correct type
			value = *node->getDefaultValue();
		}
	} catch (ErrorException& e) {
		if (e.getErrorType() == ErrorException::ERROR_STOPPED_BY_ADMIN) {
			throw e;
		}
		value = CellValue(e.getErrorType());
		value.setRuleId(erule->nr_rule);
		mem_context->m_recursion_stack.clean(size);
		return;
	}
	value.setRuleId(erule->nr_rule);
}

bool LegacyRule::next()
{
	for (;;) {
		if (isValidPath) {
			++path;
		} else {
			path = area->pathBegin();
			isValidPath = true;
		}
		if (path == area->pathEnd()) {
			sourceStreamsSP.clear();
			sourceStreams.clear();
			sourceStreamsNext.clear();
			sourceCubeAreas.clear();
			return false;
		}
		vkey = *path;
		computeCell();
		if (generateEmptyResults || !value.isEmpty()) { // if value not null
//			cout << "R\t" << vkey << "\t" << value.getNumeric() << endl;
			return true;
		}
	}
}

const CellValue &LegacyRule::getValue()
{
	if (vkey.empty()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid path in LegacyRule::getValue()");
	}
	return value;
}

double LegacyRule::getDouble()
{
	return getValue().getNumeric();
}

void LegacyRule::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No setValue for LegacyRule");
}

const IdentifiersType &LegacyRule::getKey() const
{
	return vkey;
}

const GpuBinPath &LegacyRule::getBinKey() const
{
	area->getPathTranslator()->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void LegacyRule::reset()
{
	vkey.clear();
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No reset for LegacyRule!");
}

LegacyMarkedRule::LegacyMarkedRule(PEngineBase engine, CPPlanNode node) : LegacyRule(engine, node)
{
	const LegacyRulePlanNode *legacyRulePlanNode = dynamic_cast<const LegacyRulePlanNode *>(node.get());
	PStorageBase markerStorage = engine->getStorage(legacyRulePlanNode->getCube()->getMarkerStorageId());
	if (markerStorage) {
		markers = markerStorage->getCellValues(node->getArea(), 0);
	}
}

LegacyMarkedRule::~LegacyMarkedRule()
{
}

bool LegacyMarkedRule::next()
{
	if (markers) {
		while (markers->next()) {
			vkey = markers->getKey();
			computeCell();
			if (!value.isEmpty()) { // if value not null
				return true;
			}
		}
	}
	return false;
}

}
