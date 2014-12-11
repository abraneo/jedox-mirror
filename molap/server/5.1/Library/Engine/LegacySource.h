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

#ifndef OLAP_LEGACYSOURCE_H
#define OLAP_LEGACYSOURCE_H 1

#include "palo.h"
#include "Engine/Area.h"
#include "Engine/Legacy/Engine.h"
#include "Engine/Legacy/VirtualMachine.h"

using namespace paloLegacy;

namespace paloLegacy {
class AreaStringResultStorage;
class CellPath;
}

namespace palo {

class LegacyRulePlanNode;

class LegacyRule : public ProcessorBase {
public:
	LegacyRule(PEngineBase engine, CPPlanNode node);
	virtual ~LegacyRule();

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual void setValue(const CellValue &value);
	virtual const IdentifiersType &getKey() const;
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset();
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance
private:
	PEngineBase engine;
	CPDatabase db;
	CPCube cube;
	PCubeArea area;

	bool isValidPath;
	Area::PathIterator path;
	GpuBinPath binPath;
	vector<PCubeArea> sourceCubeAreas;
	vector<PProcessorBase> sourceStreamsSP;
	vector<ProcessorBase *> sourceStreams;
	vector<bool> sourceStreamsNext;
	VMCache vmCache;
private:
	// Engine
	RulesContext *mem_context;
	ERule *erule;
	bool generateEmptyResults;
	void generateSources();
protected:
	void computeCell();
	IdentifiersType vkey;
	CellValue value;
	CPPlanNode node;
	CellValue globalError;
	const LegacyRulePlanNode *legacyRulePlanNode;
	PUser user;
};

class LegacyMarkedRule : public LegacyRule {
public:
	LegacyMarkedRule(PEngineBase engine, CPPlanNode node);
	virtual ~LegacyMarkedRule();
	virtual bool next();
	virtual void reset();
	virtual bool move(const IdentifiersType &key, bool *found);
private:
	PCellStream markers;
};

}

#endif
