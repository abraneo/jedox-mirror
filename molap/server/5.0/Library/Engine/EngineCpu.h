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

#ifndef OLAP_ENGINE_CPU_H
#define OLAP_ENGINE_CPU_H 1

#include "palo.h"
#include "Engine/EngineBase.h"
#include "Engine/Streams.h"

namespace palo {

class FileWriter;

class SERVER_CLASS ConstantRepeater : public CellValueStream {
public:
	ConstantRepeater(CPArea area, const CellValue *defaultValue);
	ConstantRepeater(PPathTranslator pathTranslator) : area(0), pathTranslator(pathTranslator) {}
	ConstantRepeater(const ConstantRepeater &other);
	virtual ~ConstantRepeater() {}
	virtual const IdentifiersType &getKey() const {
		return *areaIt;
	};
	virtual const GpuBinPath &getBinKey() const;
	const CellValue &getValue() {
		return defaultValueCopy;
	};
	double getDouble() {
		return defaultValueCopy.getNumeric();
	};
protected:
	bool nextRepetition() {
		if (repeatCount) {
			++areaIt;
			return --repeatCount > 0;
		} else return false;
	}
	uint64_t setRepetition(const IdentifiersType *afterKey, const IdentifiersType *toKey);
	bool isDefaultValue() const {
		return repeatCount != 0;
	}
	void reset();
	CPArea cpArea;
	const Area *area;
	const CellValue *defaultValue;
private:
	CellValue defaultValueCopy;
	Area::PathIterator areaIt;
	uint64_t repeatCount;
	PPathTranslator pathTranslator;
	GpuBinPath binPath;
};

class SERVER_CLASS RepeaterProcessor : public CellValueStream {
public:
	RepeaterProcessor(CPArea area, const CellValue &defaultValue, PCellStream sourceProcessor);
	virtual ~RepeaterProcessor() {}
	virtual bool next();
	virtual void reset();
	// virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed

	virtual const IdentifiersType &getKey() const;
	virtual const GpuBinPath &getBinKey() const;
	virtual const CellValue &getValue();
	virtual double getDouble();
private:
	bool nextRepetition() {
		if (repeatCount) {
			++areaIt;
			return --repeatCount > 0;
		} else return false;
	}
	uint64_t setRepetition(const IdentifiersType *afterKey, const IdentifiersType *toKey);
	bool isDefaultValue() const {
		return repeatCount != 0;
	}
	CPArea area;
	const CellValue defaultValue;
	PCellStream sourceProcessor;
	CellValueStream *source;
	Area::PathIterator areaIt;
	uint64_t repeatCount;
	GpuBinPath binPath;
	bool sourceEnded;
	bool bFirst;
};

//////////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine legacy rule node class
///
/// OLAP Data Engine Legacy Rule node base class. Implements legacy rule calculation.
//////////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS LegacyRulePlanNode : public PlanNode {
public:
	LegacyRulePlanNode(CPCube cube, CPArea area, CPRule rule, const CellValue *defaultValue, ValueCache *cache, bool markers) : PlanNode(LEGACY_RULE, area, defaultValue, vector<PPlanNode>(), cache, cube), cube(cube), rule(rule), markers(markers) {}
	virtual ~LegacyRulePlanNode() {}
	CPCube getCube() const {return cube;}
	CPRule getRule() const {return rule;}
	bool useMarkers() const {return markers;}
private:
	virtual string getXMLAttributes() const;
	CPCube cube;
	CPRule rule;
	bool markers;
};

class SERVER_CLASS EngineCpu : public EngineBase {
public:
	EngineCpu();
	EngineCpu(const EngineCpu &s);
	virtual ~EngineCpu();

	//virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns storage object of given Id (and creates it if needed)
	////////////////////////////////////////////////////////////////////////////////
	virtual PStorageBase getCreateStorage(IdentifierType &id, PPathTranslator pathTranslator, StorageType type);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new processor providing data
	////////////////////////////////////////////////////////////////////////////////
	virtual PCellStream createProcessor(CPPlanNode node, bool useCache = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief test of plan support
	////////////////////////////////////////////////////////////////////////////////
	virtual bool isPlanSupported(CPPlanNode node) const;
};

}

#endif
