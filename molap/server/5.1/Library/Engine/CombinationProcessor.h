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

#ifndef OLAP_COMBINATION_PROCESSOR_H
#define OLAP_COMBINATION_PROCESSOR_H 1

#include "palo.h"
#include "Engine/EngineBase.h"
#include "Engine/Streams.h"

namespace palo {

class SERVER_CLASS CombinationProcessorBase : public ProcessorBase {
public:
	CombinationProcessorBase(PEngineBase engine, CPPathTranslator pathTranslator, const vector<PPlanNode> &children);
	virtual ~CombinationProcessorBase() {}

	virtual bool next() = 0;
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual const IdentifiersType &getKey() const;
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset();
	void createInputProcessors(const vector<PPlanNode> &rchildren);
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance

protected:
	int index;
	vector<PProcessorBase> inputs;
	CPPathTranslator pathTranslator;
	GpuBinPath binPath;
	const vector<PPlanNode> children;
};

class SERVER_CLASS CombinationProcessor : public CombinationProcessorBase {
public:
	CombinationProcessor(PEngineBase engine, const vector<PPlanNode> &children, CPPathTranslator pathTranslator);
	CombinationProcessor(PEngineBase engine, vector<PProcessorBase> inputs, CPPathTranslator pathTranslator);
	virtual ~CombinationProcessor() {}
	virtual bool next();
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed

private:
	bool hasSameKey;
	vector<bool> isSame;
	vector<bool> hasNext;
};

class SERVER_CLASS DisjunctiveCombinationProcessor : public CombinationProcessorBase {
public:
	DisjunctiveCombinationProcessor(PEngineBase engine, const vector<PPlanNode> &rchildren, CPPathTranslator pathTranslator);
	virtual ~DisjunctiveCombinationProcessor() {}
	virtual bool next();

private:
	void init();

	uint32_t dimCount;
	vector<uint32_t> setSize;
	vector<char> first;
	vector<uint32_t> firstDiff;
	vector<int32_t> pos;
	vector<vector<uint32_t> > counter;
};

class SERVER_CLASS ConstantProcessor : public ProcessorBase {
public:
	ConstantProcessor(CPArea area, const CellValue *value) : ProcessorBase(true, PEngineBase()), value(value ? *value : CellValue()), area(area), path(area->pathBegin()), first(true) {}
	virtual ~ConstantProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue() {return value;}
	virtual double getDouble() {return value.getNumeric();}
	virtual const IdentifiersType &getKey() const {return first ? EMPTY_KEY : *path;}
	virtual void reset() {first = true;}
	virtual bool move(const IdentifiersType &key, bool *found);

private:
	const CellValue value;
	CPArea area;
	Area::PathIterator path;
	bool first;
};

}

#endif
