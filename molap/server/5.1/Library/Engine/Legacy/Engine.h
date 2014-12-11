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

#ifndef ENGINE_H
#define ENGINE_H

#include "palo.h"
#include "Olap/Context.h"
#include "Olap/Cube.h"
#include "Engine/Legacy/SimpleCache.h"

#ifdef _MSC_VER
#undef max
#endif

using namespace palo;

namespace paloLegacy {

/****************************************************************************
 **
 *T  EElementId  . . . . . . . . . . . . . . . . . . . . .  type of element ID
 **
 **  'ElementId' is the type of an element identifier.
 */
typedef uint32_t EElementId;

/****************************************************************************
 **
 *T  EDimension  . . . . . . . . . . . . . . . . . . . . . type of a dimension
 **
 **  dimension->database
 */
class EDimension {
public:
	EDimension(CPDimension dimension);
	~EDimension();
	void init();

	uint32_t size;
	uint32_t nrCons;
	union {
		EElementId *pconss;			// valid if nrCons > 1
		EElementId 	singleConsId;	// valid if nrCons == 1
	};
	uint32_t nrBase;
	union {
		EElementId *pbases;			// valid if nrBase > 1
		EElementId 	singleBaseId;	// valid if nrBase == 1
	};
	char *type;
	uint32_t maximal;
	const Dimension  *adimension;

	const EElementId *firstCons() const {
		return nrCons > 1 ? pconss : &singleConsId;
	};
	const EElementId *firstBase() const {
		return nrBase > 1 ? pbases : &singleBaseId;
	};
};

/****************************************************************************
 **
 *T  EBorder . . . . . .  type of the border of an area, subset of a dimension
 **
 */
class EBorder {
public:
	EBorder(EDimension *dim);
	~EBorder();

	EDimension	*dimension;

	uint32_t 	size; 				// nrCons + nrBase

	uint32_t 	nrCons;
	union {
		EElementId 	*pconss; 		// valid if nrCons > 1 and nrCons < all
		EElementId 	singleConsId;	// valid if nrCons == 1
	};
	uint32_t 	nrBase;
	union {
		EElementId 	*pbases; 		// empty if nrBase > 1 and nrBase < all
		EElementId 	singleBaseId;	// valid if nrBase == 1
	};

	const EElementId *firstCons() const {
		return nrCons > 1 ? pconss : &singleConsId;
	};
	const EElementId *firstBase() const {
		return nrBase > 1 ? pbases : &singleBaseId;
	};

};

#define MAX_DIMENSIONS      256

typedef EElementId EPath[MAX_DIMENSIONS];

/****************************************************************************
 **
 *T  ECube . . . . . . . . . . . . . . . . . . . . . . . . . .  type of a cube
 **
 */
class ERule;
class ECube {
public:
	ECube(uint32_t nrDimensions, uint32_t nrRules, const Cube &cube, const Database	&database);
	~ECube();
	ERule *findRule(IdentifierType ruleId);
	uint32_t 	nrDimensions;
	EDimension	*dimensions[MAX_DIMENSIONS];
	uint32_t	nrRules;
	list<ERule*> rules_c;
	list<ERule*> rules_n;
	const Cube *acube;
	const Database *database;
	PStorageBase numStorage;
	PStorageBase strStorage;
};

/****************************************************************************
 **
 *T  EArea . . . . . . . . . . . . . . . . . . . . . . . . . . .  type of area
 **
 */
class EArea {
public:
	EArea(ECube *cube);
	~EArea();
	ECube	*cube;
	EBorder **borders;
};

/****************************************************************************
 **


 *T  ERule . . . . . . . . . . . . . . . . . . . . . . . . . .  type of a rule
 **
 */
typedef uint16_t Bytecode;

class ERule {
public:
	ERule(ECube *cube, CPRule rule);
	~ERule();
	void init();

	ECube		*cube;

	IdentifierType	nr_rule;

	EArea		*dest_area;

	uint8_t		is_base;
	uint8_t		is_cons;

	uint8_t		marker_flag; /* 1: has a marker                 */

	uint8_t		ubm_flag; /* 1: has a unique bijective marker  */
	uint8_t		ubm_rulesTested; /* 1: test for next rules in chain was made  */
	uint8_t		ubm_noMoreRules; /* 1: no source cell can be rules calculated  */
	EPath 		ubm_mask; /*[i] == 1: this dim is restricted */
	EPath 		ubm_dest;
	EPath 		ubm_dest_is_base;
	EPath 		ubm_source;
	EPath 		ubm_source_is_base;

	Bytecode	*bytecode;
	double		*dbl_consts;
	string		*str_consts;
	uint8_t		*source_precalc;
	EElementId	**copy_mask;
	EElementId	**copy_source;

	uint32_t	gc_bc_nr;
	uint32_t	gc_bc_max;
	uint32_t	gc_dbl_const_nr;
	uint32_t	gc_dbl_const_max;
	uint32_t	gc_str_const_nr;
	uint32_t	gc_str_const_max;
	uint32_t	gc_copy_nr;

	uint8_t		precalcStet;

	const Rule *arule;
private:
	static const size_t MAX_MASK_SIZE = 256;
};

struct EngineStackEntry
{
	EngineStackEntry() : cube(0), rule(0) {}
	EngineStackEntry(CPCubeArea cubeArea, const Rule *rule) : cubeArea(cubeArea), cube(0), rule(rule) {}
	EngineStackEntry(const Cube	*cube, const Rule *rule, const IdentifierType *path) : cube(cube), rule(rule), path(path, path+cube->getDimensions()->size()) {}
	CPCubeArea		cubeArea;
	const Cube		*cube;
	const Rule		*rule;
	IdentifiersType	path;
};

class Recursion_Stack : private std::vector<EngineStackEntry> {
	static const unsigned int MAX_STACK_SIZE = 50 * 1024;

public:
	void push(const Cube *cube, const Rule *rule, const IdentifierType *path);
	ErrorException::ErrorType push(CPCubeArea cubeArea, const Rule *rule);
	void pop();
	size_t size();
	void clean(size_t toDepth);
};

}

namespace palo {

/****************************************************************************
 **
 *
 *memory allocation and stack corruption check
 */
class RulesContext {
private:
	paloLegacy::SimpleCache::query_cache_type m_querycache;
	size_t m_qc_hitcount;
	size_t m_qc_lookups;
	bool m_query_cache_full;
	size_t m_querycache_writes;
	size_t m_qc_limit;
	bool rules;
public:
	paloLegacy::Recursion_Stack m_recursion_stack;
	Context *context;

	RulesContext() {
		clear();
	}
	~RulesContext();

	bool readQueryCache(CPCube acube, const IdentifierType *cellkey, double &val, bool &isString);
	void writeQueryCache(CPCube acube, const IdentifierType *cellkey, const double val, bool isString);

	void activateRules(bool activate) {
		rules = activate;
	}
	bool areRulesActive() {
		return rules;
	}
	void clear() {
		context = Context::getContext();
		m_querycache.clear();
		m_qc_hitcount = 0;
		m_qc_lookups = 0;
		m_query_cache_full = false;
		m_querycache_writes = 0;
		m_qc_limit = 10000000;
		rules = true;
	}
};

}

namespace paloLegacy {

/****************************************************************************
 **
 *F  NewEntryCube(cube,user) . . . . . . . . .  allocate a new cube upon entry
 **
 */
ECube *NewEntryCube(const Cube &cube, const Database &db, const EngineBase &engine, Context *context);

/****************************************************************************
 **

 *F  IsPathInArea(area,path) . . . . . . . .  test whether a path is in an area
 **
 */
uint8_t IsPathInArea(EArea *area, EPath path);

/****************************************************************************
 **
 *F  IsBasePathInArea(area,path) . . . . test whether a base path is in an area
 **
 */
uint8_t IsBasePathInArea(EArea *area, EPath path);
}

#endif
