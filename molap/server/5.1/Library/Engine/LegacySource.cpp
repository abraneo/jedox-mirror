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
		ProcessorBase(true, engine), engine(engine), isValidPath(false), node(node)
{
	Context *context = Context::getContext();
	legacyRulePlanNode = dynamic_cast<const LegacyRulePlanNode *>(node.get());
	cube = legacyRulePlanNode->getCube();
	db = legacyRulePlanNode->getDatabase();
	mem_context = context->getRulesContext();
	ECube *ecube = NewEntryCube(*cube, *db, *engine, context);
	CPRule rule = legacyRulePlanNode->getRule();
	generateEmptyResults = legacyRulePlanNode->getDefaultValue() != 0;

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

	boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
	user = session ? session->getUser() : PUser();
}

LegacyRule::~LegacyRule()
{
}

void LegacyRule::generateSources()
{
	boost::shared_ptr<PaloSession> session = Context::getContext()->getSession();
	PUser user = session ? session->getUser() : PUser();
	// if caching enabled
	if (cube->getCacheBarrier() > 0 && area->getSize() > 0) {
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
					if (sourceCubeArea && sourceCubeArea->getSize()) {
						bool skip = false;
						if (erule->arule->isCustom()) {
							try {
								if (User::checkUser(user)) {
									User::RightSetting rs(User::checkCellDataRightCube(sourceDb, sourceCube));
									cube->checkAreaAccessRight(sourceDb, user, sourceCubeArea, rs, false, RIGHT_READ, 0);
								}
							} catch (ErrorException &) {
								skip = true;
							}
						} else {
							try {
								User::checkRuleDatabaseRight(user.get(), erule->cube->database->getId(), sourceDb->getId());
							} catch (ErrorException &) {
								skip = true;
							}
						}
						uint64_t barrier = (uint64_t)cube->getCacheBarrier();
						Logger::trace << "Target area: " << *area << endl;
						Logger::trace << "Source area: " << *sourceCubeArea << endl;
						// calculate the area
						if (!skip) {
							PCellStream src = sourceCube->calculateArea(sourceCubeArea, CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, barrier);
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
		sourceCubeAreas[source] = PCubeArea(new CubeArea(*area));
		for (size_t dim = 0; dim < erule->cube->nrDimensions; dim++) {
			if (erule->copy_mask[source][dim] == AreaNode::ABS_RESTRICTION) {
				IdentifierType sourceId = erule->copy_source[source][dim];
				PSet s(new Set);
				s->insert(sourceId);
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

		PPlanNode sourcePlan;
		if (erule->arule->isCustom()) {
			try {
				if (User::checkUser(user)) {
					User::RightSetting rs(User::checkCellDataRightCube(db, cube));
					cube->checkAreaAccessRight(db, user, sourceCubeAreas[source], rs, false, RIGHT_READ, 0);
				}
			} catch (ErrorException &e) {
				CellValue c(e.getErrorType());
				sourcePlan.reset(new ConstantPlanNode(sourceCubeAreas[source], c));
			}
		} // area and sourceCubeAreas[source] are from the same database therefore User::checkRuleDatabaseRight doesn't have to be called
		if (!sourcePlan) {
			sourcePlan = cube->createPlan(sourceCubeAreas[source], CubeArea::ALL, RulesType(ALL_RULES | NO_RULE_IDS), true, UNLIMITED_SORTED_PLAN);
		}
		sourcePlan = PPlanNode(new TransformationPlanNode(area, sourcePlan, SetMultimaps(), 1.0, vector<uint32_t>()));
//		Logger::trace << "Transformed Plan: " << sourcePlan->toXML() << endl;
		PProcessorBase sourceStream = cube->evaluatePlan(sourcePlan, EngineBase::ANY, true);
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
		PProcessorBase sourceStream = cube->evaluatePlan(sourcePlan, EngineBase::ANY, true);
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
			if (!sourceStreamsNext[source]) {
				continue;
			}
			bool exactMatch = false;
			sourceStreamsNext[source] = sourceStreams[source]->move(vkey, &exactMatch);
			if (sourceStreamsNext[source]) {
				// something found
				if (exactMatch) {
					sourceStreamsCopy[source] = sourceStreams[source];
				}
			} else {
				sourceStreamsSP[source].reset();
				sourceStreams[source] = 0;
			}
		}

		// apply the rule
		virtual_machine machine(mem_context, &value);
		machine.setSourceStreams(&sourceStreamsCopy);
		machine.setCache(&vmCache);
		machine.setUser(user);

		// Todo: -jj- not optimal - instead of using CellStatus::NotFoundStatus we should define LegacyUbmMarkedRule and define source processor driving the calculation
		EPath vkeyCopy;
		memcpy(vkeyCopy, &vkey[0], vkey.size() * sizeof(IdentifierType));
		machine.compute(engine.get(), &vkeyCopy[0], erule, defValue, notFoundStatus);
		if (value.isEmpty() && generateEmptyResults) {	// if value is empty - make sure it has correct type
			value = *legacyRulePlanNode->getDefaultValue();
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
			return false;
		}
		vkey = *path;
		computeCell();
		if (generateEmptyResults || !value.isEmpty()) { // if value not null
//			cout << "R\t" << vkey << "\t" << value.getNumeric() << endl;
			return true;
		}
	}
	return false;
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
	isValidPath = false;
	vkey.clear();
	for (size_t src = 0; src < sourceStreams.size(); ++src) {
		if(sourceStreams[src]) {
			sourceStreams[src]->reset();
			sourceStreamsNext[src] = sourceStreams[src]->next();
			if (sourceStreamsNext[src]) {
				sourceStreams[src]->reset();
			}
		}
	}
	//throw ErrorException(ErrorException::ERROR_INTERNAL, "No reset for LegacyRule!");
}

LegacyMarkedRule::LegacyMarkedRule(PEngineBase engine, CPPlanNode node) : LegacyRule(engine, node)
{
	legacyRulePlanNode = dynamic_cast<const LegacyRulePlanNode *>(node.get());
	PStorageBase markerStorage = engine->getStorage(legacyRulePlanNode->getCube()->getMarkerStorageId());
	if (markerStorage) {
		markers = markerStorage->getCellValues(node->getArea());
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

void LegacyMarkedRule::reset()
{
	if (markers) {
		markers->reset();
	}
	LegacyRule::reset();
}

bool LegacyMarkedRule::move(const IdentifiersType &key, bool *found)
{
	if (markers) {
		if (markers->move(key, found)) {
			do {
				vkey = markers->getKey();
				computeCell();
				if (!value.isEmpty()) { // if value not null
					return true;
				}
				if (found) {
					*found = false;
				}
			} while (markers->next());
		}
	}
	return false;
}

}
