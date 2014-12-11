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
 * 
 *
 */

#ifndef OLAP_ARITHMETIC_PROCESSORS_H
#define OLAP_ARITHMETIC_PROCESSORS_H 1

#include "palo.h"
#include "Engine/EngineBase.h"

namespace palo {
class SERVER_CLASS ArithmeticProcessor : public ProcessorBase {
public:
	ArithmeticProcessor(PEngineBase engine, CPPlanNode node);
	ArithmeticProcessor(const vector<PProcessorBase> &streams);
	virtual ~ArithmeticProcessor() {}

	virtual bool next() = 0;
	virtual const CellValue &getValue() {return value;}
	virtual double getDouble() {return value.getNumeric();}
	virtual void setValue(const CellValue &value) {throw ErrorException(ErrorException::ERROR_INTERNAL, "ArithmeticProcessor::setValue not supported!");}
	virtual const IdentifiersType &getKey() const {return key;}
	virtual const GpuBinPath &getBinKey() const {pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath)); return binPath;}
	virtual void reset();
	virtual bool move(const IdentifiersType &key, bool *found) = 0;

protected:
	void createInputProcessors();
	vector<CellValueStream *> *nextAll();
	vector<CellValueStream *> *nextAny();
	vector<CellValueStream *> *moveAll(const IdentifiersType &key, bool *found);
	vector<CellValueStream *> *moveAny(const IdentifiersType &key, bool *found);
	CellValue value;
	IdentifiersType key;
	CellValue constValue;
private:
	vector<PProcessorBase> streams;
	vector<CellValueStream *> activeOperands;
	CPPathTranslator pathTranslator;
	GpuBinPath binPath;
	vector<bool> hasNext;
	size_t operandsCount;
	CPPlanNode node;
};

class SERVER_CLASS MultiplicationProcessor : public ArithmeticProcessor {
public:
	MultiplicationProcessor(PEngineBase engine, CPPlanNode node);
	virtual ~MultiplicationProcessor() {}

	virtual bool next();
	virtual bool move(const IdentifiersType &key, bool *found);
};

class SERVER_CLASS DivisionProcessor : public ArithmeticProcessor {
public:
	DivisionProcessor(PEngineBase engine, CPPlanNode node) : ArithmeticProcessor(engine, node) {}
	virtual ~DivisionProcessor() {}

	virtual bool next();
	virtual bool move(const IdentifiersType &key, bool *found);
};

class SERVER_CLASS AdditionProcessor : public ArithmeticProcessor {
public:
	AdditionProcessor(PEngineBase engine, CPPlanNode node) : ArithmeticProcessor(engine, node) {}
	AdditionProcessor(const vector<PProcessorBase> &streams) : ArithmeticProcessor(streams) {}
	virtual ~AdditionProcessor() {}

	virtual bool next();
	virtual bool move(const IdentifiersType &key, bool *found);
};

class SERVER_CLASS SubtractionProcessor : public ArithmeticProcessor {
public:
	SubtractionProcessor(PEngineBase engine, CPPlanNode node) : ArithmeticProcessor(engine, node) {}
	SubtractionProcessor(const vector<PProcessorBase> &streams) : ArithmeticProcessor(streams) {}
	virtual ~SubtractionProcessor() {}

	virtual bool next();
	virtual bool move(const IdentifiersType &key, bool *found);
};

}

#endif
