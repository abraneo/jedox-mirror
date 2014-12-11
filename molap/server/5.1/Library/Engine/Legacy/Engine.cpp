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
 * \author Martin Schoenert, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "palo.h"

#include "Collections/CellMap.h"
#include "Olap/Cube.h"
#include "Olap/Context.h"
#include "Olap/Rule.h"
#include "Parser/RuleNode.h"
#include "Engine/Legacy/Engine.h"
#include "Engine/Legacy/VirtualMachine.h"
#include "Engine/LegacySource.h"

/* #define DEBUG_ENGINE */
#ifdef DEBUG_ENGINE
#define DEBUG_PRINTF(x) printf x
#else
#define DEBUG_PRINTF(x)
#endif

using namespace paloLegacy;

namespace palo {

static size_t s_null_values = 0;

bool RulesContext::readQueryCache(CPCube acube, const IdentifierType *cellkey, double &val, bool &isString)
{
	if (!(size_t)Cube::getCacheBarrier()) {
		return false;
	}

	bool found = false;

	paloLegacy::SimpleCache::query_cache_key qk(context->getParent(acube)->getId(), acube->getId());
	paloLegacy::SimpleCache::query_cache_iterator qci = m_querycache.find(qk);
	if (qci != m_querycache.end()) {
		m_qc_lookups++;

		SimpleCache::cache_value_type cache_value;
		found = qci->second->get(cellkey, cache_value);
		if (found) {
			val = cache_value.first;
			isString = cache_value.second ? true : false;
			m_qc_hitcount++;
		} else if (m_query_cache_full) { // value not found and cache is full
			if (m_qc_lookups % 1000 == 0 && 100 * m_qc_hitcount / m_qc_lookups < 10) { // succees ratio < 10 %
				Logger::debug << "Query cache success rate low. Cache cleanup." << endl;
				m_querycache.clear();
				m_qc_hitcount = m_qc_lookups = 0;
				m_query_cache_full = false;
			}
		}
	}
	return found;
}

void RulesContext::writeQueryCache(CPCube acube, const IdentifierType *cellkey, const double val, bool isString)
{
	if (!(size_t)Cube::getCacheBarrier()) {
		return;
	}

	if (!m_query_cache_full) {
		paloLegacy::SimpleCache::query_cache_key qk(context->getParent(acube)->getId(), acube->getId());
		paloLegacy::SimpleCache::query_cache_iterator qci = m_querycache.find(qk);
#ifdef _DEBUG
		if (qci != m_querycache.end()) {
			paloLegacy::SimpleCache::cache_value_type cachedValue;

			if (qci->second->get(cellkey, cachedValue)) {
				if (val != cachedValue.first || isString != (cachedValue.second != 0)) {
					int q = 1; // something wrong
					q++;
				}
			}
		}
#endif
		if (qci == m_querycache.end()) {
			SimpleCache::PQueryCache spcache(CreateCellMap<SimpleCache::cache_value_type>(acube->getDimensions()->size()));
			m_querycache[qk] = spcache;
			qci = m_querycache.find(qk);
		}
		if (qci != m_querycache.end()) {
			qci->second->set(cellkey, SimpleCache::cache_value_type(val, isString ? 1 : 0));
		}
		if (val == 0) {
			s_null_values++;
		}
		if (!(m_querycache_writes++ % 1000)) {
			size_t total_size = 0;

			// check total size of query cache
			for (paloLegacy::SimpleCache::query_cache_type::iterator qci = m_querycache.begin(); qci != m_querycache.end(); ++qci) {
				total_size += qci->second->size();
			}
			if (total_size > m_qc_limit) {
				Logger::debug << "Per query cache full! Success ratio: " << (m_qc_lookups ? 100 * m_qc_hitcount / m_qc_lookups : 0) << " % null values: " << s_null_values << " double writes: " << m_querycache_writes * 100 / total_size - 100 << " %" << endl;
				m_query_cache_full = true;
				m_querycache_writes = s_null_values = 0;
			}
		}
	}
}

RulesContext::~RulesContext()
{
	if (Logger::isDebug() && m_querycache.size()) {
		stringstream ss;
		uint32_t totalcells = 0;
		ss << "Query Cache: ";
		for (paloLegacy::SimpleCache::query_cache_iterator qci = m_querycache.begin(); qci != m_querycache.end(); ++qci) {
			totalcells += (uint32_t)qci->second->size();
			ss << qci->second->size() << "[" << qci->first.first << "," << qci->first.second << "];";
		}
		ss << "Total: " << totalcells << " hits: " << m_qc_hitcount << " lookups: " << m_qc_lookups;
		if (m_qc_lookups) {
			ss << " " << 100.0 * m_qc_hitcount / m_qc_lookups << " %";
		}
		ss << endl;
		Logger::debug << ss.str();
	}
}

}

using namespace palo;

namespace paloLegacy {

EBorder::EBorder(EDimension *dim) : dimension(dim), size(0), nrCons(0), pconss(0), nrBase(0), pbases(0)
{
}

EBorder::~EBorder()
{
	if (nrCons > 1 && nrCons < dimension->nrCons && pconss) {
		delete []pconss;
	}
	if (nrBase > 1 && nrBase < dimension->nrBase && pbases) {
		delete []pbases;
	}
}

EArea::EArea(ECube *cube) : cube(cube)
{
	borders = new EBorder *[cube->nrDimensions];
}

EArea::~EArea()
{
	for (uint32_t i = 0; i < cube->nrDimensions; i++) {
		if (borders[i]) {
			delete borders[i];
		}
	}
	delete []borders;
}

ECube::ECube(uint32_t nrDim, uint32_t nrRules, const Cube &cube, const Database &database) : nrDimensions(nrDim), nrRules(nrRules), acube(&cube), database(&database)
{
	memset(dimensions, 0, sizeof(dimensions[0]) * nrDim);
}

ECube::~ECube()
{
	for (list<ERule*>::iterator it = rules_c.begin(); it != rules_c.end(); ++it) {
		if (!(*it)->is_base) {
			delete *it;
		}
	}
	for (list<ERule*>::iterator it = rules_n.begin(); it != rules_n.end(); ++it) {
		delete *it;
	}
	for (uint32_t i = 0; i < nrDimensions; i++) {
		if (dimensions[i]) {
			delete dimensions[i];
		}
	}
}

ERule *ECube::findRule(IdentifierType ruleId)
{
	for (list<ERule*>::iterator rit = rules_c.begin(); rit != rules_c.end(); ++rit) {
		if ((*rit)->nr_rule == ruleId) {
			return *rit;
		}
	}
	for (list<ERule*>::iterator rit = rules_n.begin(); rit != rules_n.end(); ++rit) {
		if ((*rit)->nr_rule == ruleId) {
			return *rit;
		}
	}
	return 0;
}


ERule::ERule(ECube *cube, CPRule rule) : cube(cube), nr_rule(rule->getId()), dest_area(0), is_base(0), is_cons(0), marker_flag(0), ubm_flag(0),
	ubm_rulesTested(0), ubm_noMoreRules(0),	bytecode(0), dbl_consts(0), str_consts(0), source_precalc(0), copy_mask(0),
	copy_source(0),	gc_bc_nr(0), gc_bc_max(0), gc_dbl_const_nr(0), gc_dbl_const_max(0), gc_str_const_nr(0), gc_str_const_max(0),
	gc_copy_nr(0), precalcStet(0), arule(rule.get())
{
}

void ERule::init()
{
	if (arule->getRuleOption() == RuleNode::NONE) {
		is_base = 1;
		is_cons = 1;
	} else if (arule->getRuleOption() == RuleNode::CONSOLIDATION) {
		is_base = 0;
		is_cons = 1;
	} else if (arule->getRuleOption() == RuleNode::BASE) {
		is_base = 1;
		is_cons = 0;
	}

	//uninitialized
	//ubm_flag;
	//ubm_mask;
	//ubm_dest;
	//ubm_dest_is_base;
	//ubm_source_is_base;

	memset(ubm_source, 0, sizeof(ubm_source));

	/* allocate bytecode */
	gc_bc_nr = 0;
	gc_bc_max = 25600;
	bytecode = new Bytecode[gc_bc_max];
	memset(bytecode, 0, sizeof(Bytecode) * gc_bc_max);

	/* constants */
	gc_dbl_const_nr = 0;
	gc_dbl_const_max = 16;
	gc_str_const_nr = 0;
	gc_str_const_max = 16;
	dbl_consts = new double[gc_dbl_const_max];
	memset(dbl_consts, 0, sizeof(double) * gc_dbl_const_max);
	str_consts = new string[gc_str_const_max];

	/* copy mask */
	gc_copy_nr = 0;
	source_precalc = new uint8_t[MAX_MASK_SIZE];
	copy_mask = new EElementId*[MAX_MASK_SIZE];
	memset(copy_mask, 0, sizeof(EElementId*) * MAX_MASK_SIZE);
	copy_source = new EElementId*[MAX_MASK_SIZE];
	memset(copy_source, 0, sizeof(EElementId*) * MAX_MASK_SIZE);

	bytecode_generator generator(*this);

	/* a rules has no marker if the conversion failed                      */
	/* (from the format in the rule node to the format in the cube struct) */
	/* this happens if the left hand side has a unspecified dimension      */
	/* that does not appear as variable in PALO.MARKER()                   */
	if (!arule->hasMarkers()) {
		marker_flag = 0;
	} else {
		marker_flag = 1;
	}

	/* rules with a marker are automatically N-Rules */
	if (marker_flag) {
		is_base = 1;
		is_cons = 0;
	}

	arule->genCode(generator);
}

ERule::~ERule()
{
	delete dest_area;
	delete [] bytecode;
	delete [] dbl_consts;
	delete [] str_consts;
	if (copy_mask) {
		for (size_t i = 0; i < MAX_MASK_SIZE; i++) {
			delete [] copy_mask[i];
			delete [] copy_source[i];
		}
	}
	delete [] source_precalc;
	delete [] copy_mask;
	delete [] copy_source;
}

void Recursion_Stack::push(const Cube *cube, const Rule *rule, const IdentifierType *path)
{
	size_t nrDims = cube->getDimensions()->size();
	for (reverse_iterator it = rbegin(); it != rend(); ++it) {
		if (it->cubeArea) {
			continue;
		}
		const Cube *it_cube = it->cube;
		if (it_cube->getId() != cube->getId()) {
			continue;
		}

		const Rule *it_rule = it->rule;
		if (rule) {
			if (it_rule) {
				if (it_rule->getId() != rule->getId()) {
					continue;
				}
			} else {
				continue;
			}
		} else {
			if (it_rule) {
				continue;
			}
		}

		bool match = true;
		// check path
		for (uint32_t j = 0; j < nrDims; j++) {
			if (it->path[j] != path[j]) {
				match = false;
				break;
			}
		}
		if (!match) {
			continue;
		}

		Logger::warning << "recursion in rule found (cube " << cube->getId() << ")" << endl;
		if (rule) {
			throw ErrorException(ErrorException::ERROR_RULE_HAS_CIRCULAR_REF, "cube=" + StringUtils::convertToString(cube->getId()) + "&rule=" + StringUtils::convertToString(rule->getId()), rule->getId());
		} else {
			throw ErrorException(ErrorException::ERROR_RULE_HAS_CIRCULAR_REF, "cube=" + StringUtils::convertToString(cube->getId()));
		}
	}
	EngineStackEntry stackEntry(cube, rule, path);

	push_back(stackEntry);

	if (MAX_STACK_SIZE < size()) {
		if (rule) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "Stack Overflow while evaluating: cube=" + StringUtils::convertToString(cube->getId()) + "&rule=" + StringUtils::convertToString(rule->getId()), rule->getId());
		} else {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "Stack Overflow while evaluating: cube=" + StringUtils::convertToString(cube->getId()));
		}
	}
}

ErrorException::ErrorType Recursion_Stack::push(CPCubeArea cubeArea, const Rule *rule)
{
	for (reverse_iterator it = rbegin(); it != rend(); ++it) {
		if (!it->cubeArea) {
			continue;
		}
		if (it->rule != rule) {
			continue;
		}
		if (*cubeArea == *it->cubeArea) {
			Logger::warning << "Endless recursion in rule found! Database: " << cubeArea->getDatabase()->getName() << " Cube:" << cubeArea->getCube()->getName() << " RuleId: " << rule->getId() <<  endl;
			return ErrorException::ERROR_RULE_HAS_CIRCULAR_REF;
		}
	}
	EngineStackEntry stackEntry(cubeArea, rule);
	push_back(stackEntry);

	if (MAX_STACK_SIZE < size()) {
		Logger::warning << "Rule stack overflow!" << endl;
		return ErrorException::ERROR_INTERNAL;
	}
	return ErrorException::ErrorType(0);
}

void Recursion_Stack::pop()
{
	pop_back();
}

size_t Recursion_Stack::size()
{
	return vector<EngineStackEntry>::size();
}

void Recursion_Stack::clean(size_t toDepth)
{
	if (size() > toDepth) {
		toDepth = size() - toDepth;
		for (size_t i = 0; i < toDepth; i++) {
			pop();
		}
	}
}

uint8_t IsPathInArea(EArea *area, EPath path)
{
	EElementId e;
	uint32_t d;
	uint32_t i;

	for (d = 0; d < area->cube->nrDimensions; d++) {
		e = path[d];
		if (e == NO_IDENTIFIER) {
			continue;
		}
		if (area->borders[d]->dimension->type[e] == 'C') {
			if (area->borders[d]->nrCons == 0) {
				return 0;
			} else if (area->borders[d]->nrCons == 1) {
				if (e != area->borders[d]->firstCons()[0]) {
					return 0;
				}
			}
		} else {
			if (area->borders[d]->nrBase == 0) {
				return 0;
			} else if (area->borders[d]->nrBase == 1) {
				if (e != area->borders[d]->firstBase()[0]) {
					return 0;
				}
			}
		}
	}

	for (d = 0; d < area->cube->nrDimensions; d++) {
		e = path[d];
		if (e == NO_IDENTIFIER) {
			continue;
		}
		if (area->borders[d]->dimension->type[e] == 'C') {
			if (area->borders[d]->nrCons < area->borders[d]->dimension->nrCons) {
				const EElementId *p = area->borders[d]->firstCons();
				for (i = 0; i < area->borders[d]->nrCons; i++) {
					if (e == p[i]) {
						break;
					} else if (e < p[i]) {
						return 0;
					}
				}
				if (i == area->borders[d]->nrCons) {
					return 0;
				}
			}
		} else {
			if (area->borders[d]->nrBase < area->borders[d]->dimension->nrBase) {
				const EElementId *p = area->borders[d]->firstBase();
				for (i = 0; i < area->borders[d]->nrBase; i++) {
					if (e == p[i]) {
						break;
					} else if (e < p[i]) {
						return 0;
					}
				}
				if (i == area->borders[d]->nrBase) {
					return 0;
				}
			}
		}
	}
	return 1;
}

uint8_t IsBasePathInArea(EArea *area, EPath path)
{
	EElementId e;
	uint32_t d;
	uint32_t i;

	for (d = 0; d < area->cube->nrDimensions; d++) {
		e = path[d];
		if (area->borders[d]->nrBase == 0) {
			return 0;
		} else if (area->borders[d]->nrBase == 1) {
			if (e != area->borders[d]->firstBase()[0]) {
				return 0;
			}
		}
	}

	for (d = 0; d < area->cube->nrDimensions; d++) {
		e = path[d];
		if (area->borders[d]->nrBase < area->borders[d]->dimension->nrBase) {
			const EElementId *idSet = area->borders[d]->firstBase();
			for (i = 0; i < area->borders[d]->nrBase; i++) {
				if (e == idSet[i]) {
					break;
				} else if (e < idSet[i]) {
					return 0;
				}
			}
			if (i == area->borders[d]->nrBase) {
				return 0;
			}
		}
	}
	return 1;
}

EDimension::EDimension(CPDimension dimension) : size(0), nrCons(0), pconss(0), nrBase(0), pbases(0), type(0), maximal(0), adimension(dimension.get())
{
}

void EDimension::init()
{
	maximal = adimension->getMaximalIdentifier();

	const PElementList elementList = adimension->getElementList();
	for (ElementList::const_iterator it = elementList->begin(); it != elementList->end(); ++it) {
		const Element &element = *it;
		if (element.getElementType() == Element::UNDEFINED) {
			continue;
		}
		if (element.getElementType() == Element::CONSOLIDATED) {
			nrCons++;
		} else if (element.getElementType() == Element::NUMERIC || element.getElementType() == Element::STRING) {
			nrBase++;
		}
	}

	size = nrCons + nrBase;

	EElementId *pc, *pb;
	if (nrCons > 1) {
		pconss = new EElementId[nrCons];
		memset(pconss, 0, sizeof(EElementId) * nrCons);
		pc = pconss;
	} else {
		pc = &singleConsId;
	}
	if (nrBase > 1) {
		pbases = new EElementId[nrBase];
		memset(pbases, 0, sizeof(EElementId) * nrBase);
		pb = pbases;
	} else {
		pb = &singleBaseId;
	}
	type = new char[maximal + 1];
	memset(type, 0, sizeof(char) *(maximal + 1));

	for (ElementList::const_iterator it = elementList->begin(); it != elementList->end(); ++it) {
		const Element &element = *it;
		if (element.getElementType() == Element::UNDEFINED) {
			continue;
		}
		IdentifierType i = element.getIdentifier();
		if (element.getElementType() == Element::CONSOLIDATED && !element.isStringConsolidation()) {
			*pc++ = i;
			type[i] = 'C';
		} else if (element.getElementType() == Element::NUMERIC) {
			*pb++ = i;
			type[i] = 'N';
		} else if (element.getElementType() == Element::STRING || element.isStringConsolidation()) {
			type[i] = 'S';
		}
	}
}

EDimension::~EDimension()
{
	if (nrCons > 1 && pconss) {
		delete []pconss;
	}
	if (nrBase > 1 && pbases) {
		delete []pbases;
	}
	if (type) {
		delete [] type;
	}
}

ECube *NewEntryCube(const Cube &acube, const Database &db, const EngineBase &engine, Context *context)
{
	ECube 		*result;
	ERule 		*rule;
	uint32_t 	i;

	result = context->getEngineCube(make_pair(db.getId(), acube.getId()));
	if (result) {
		return result;
	}

	vector<PRule> activeRules = acube.getRules(PUser(), true);
#ifdef SORT_RULES_BY_POSITION
	sort(activeRules.begin(), activeRules.end(), rulePositionCompare);
#endif

	//memory allocated here is referred on the Cube.cpp level. It does not qualify for autodestruction
	result = new ECube((uint32_t)acube.getDimensions()->size(), (uint32_t)activeRules.size(), acube, db);

	try {
		for (i = 0; i < result->nrDimensions; i++) {
			result->dimensions[i] = new EDimension(CONST_COMMITABLE_CAST(Database, context->getParent(acube.shared_from_this()))->lookupDimension(acube.getDimensions()->at(i), false));
			result->dimensions[i]->init();
		}

		for (i = 0; i < result->nrRules; i++) {
			CPRule arule = activeRules[i];
			if (arule->isActive()) {
				rule = new ERule(result, arule);
				try {
					rule->init();
				} catch(...) {
					delete rule;
					throw;
				}
				if (rule->is_cons) {
					result->rules_c.push_back(rule);
				}
				if (rule->is_base) {
					result->rules_n.push_back(rule);
				}
			}
		}
		result->numStorage = engine.getStorage(acube.getNumericStorageId());
		result->strStorage = engine.getStorage(acube.getStringStorageId());

		context->setEngineCube(result);
	} catch (...) {
		delete result;
		throw;
	}

	return result;
}

}
