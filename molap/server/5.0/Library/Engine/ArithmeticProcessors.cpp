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

#include "Engine/ArithmeticProcessors.h"

namespace palo {

ArithmeticProcessor::ArithmeticProcessor(PEngineBase engine, CPPlanNode node) : constValue(node->getConstValue()), engine(engine), pathTranslator(node->getArea()->getPathTranslator()), ruleId(node->getRuleId())
{
	const vector<PPlanNode> &children = node->getChildren();
	operandsCount = children.size();
	streams.resize(operandsCount);
	for (size_t i = 0; i < operandsCount; i++) {
		if (children[i]) {
			streams[i] = engine->createProcessor(children[i]);
		}
	}
	hasNext = vector<bool>(streams.size(), true);
	activeOperands = vector<CellValueStream *>(operandsCount, 0);
}

void ArithmeticProcessor::reset()
{
	key.clear();
	for (size_t i = 0; i < operandsCount; i++) {
		hasNext [i] = true;
		activeOperands[i] = 0;
		if (streams[i]) {
			streams[i]->reset();
		}
	}
}

vector<CellValueStream *> *ArithmeticProcessor::nextAll()
{
	size_t operandOrdinal;
	activeOperands = vector<CellValueStream *>(operandsCount, 0);
	// call next for each stream and remember maximum
	key.clear();

	for (operandOrdinal = 0; operandOrdinal < operandsCount; operandOrdinal++) {
		if (!streams[operandOrdinal]) {
			continue;
		}
		if (hasNext[operandOrdinal]) {
			hasNext[operandOrdinal] = streams[operandOrdinal]->next();
		}
		if (!hasNext[operandOrdinal]) {
			return 0;
		}
		activeOperands[operandOrdinal] = streams[operandOrdinal].get();
		if (key.empty() || CellValueStream::compare(streams[operandOrdinal]->getKey(), key) > 0) {
			key = streams[operandOrdinal]->getKey();
		}
	}
	// align all streams to the current maximum
	for (;;) {
		for (operandOrdinal = 0; operandOrdinal < operandsCount; operandOrdinal++) {
			if (!streams[operandOrdinal]) {
				continue;
			}
			// Todo: -jj- here we should use stream::move instead of loop of stream::next
			int diff;
			while ((diff = CellValueStream::compare(streams[operandOrdinal]->getKey(), key)) < 0) {
				if (!(hasNext[operandOrdinal] = streams[operandOrdinal]->next())) {
					return 0;
				}
			}
			if (diff > 0) {
				// new maximum found restart from first dimension
				key = streams[operandOrdinal]->getKey();
				break;
			} else {
				// operand aligned - go to next
			}
		}
		if (operandOrdinal == operandsCount) {
			return &activeOperands;
		}
	}
}

vector<CellValueStream *> *ArithmeticProcessor::nextAny()
{
	bool anyOperandActive = false;
	bool firstCall = key.empty();
	key.clear();
	// call next for each stream and remember maximum
	for (size_t operandOrdinal = 0; operandOrdinal < operandsCount; operandOrdinal++) {
		if (firstCall || (hasNext[operandOrdinal] && activeOperands[operandOrdinal])) {
			hasNext[operandOrdinal] = streams[operandOrdinal]->next();
		}
		// select minimal key
		if (hasNext[operandOrdinal]) {
			if (key.empty()) {
				// minimal key is not yet initialized
				key = streams[operandOrdinal]->getKey();
			} else {
				// compare current with minimal
				int diff = CellValueStream::compare(streams[operandOrdinal]->getKey(), key);
				if (diff < 0) {
					key = streams[operandOrdinal]->getKey();
				}
			}
		}
	}
	for (size_t operandOrdinal = 0; operandOrdinal < operandsCount; operandOrdinal++) {
		if (hasNext[operandOrdinal] && CellValueStream::compare(streams[operandOrdinal]->getKey(), key) == 0) {
			anyOperandActive = true;
			activeOperands[operandOrdinal] = streams[operandOrdinal].get();
		} else {
			activeOperands[operandOrdinal] = 0;
		}
	}
	return anyOperandActive ? &activeOperands : 0;
}

MultiplicationProcessor::MultiplicationProcessor(PEngineBase engine, CPPlanNode node) : ArithmeticProcessor(engine, node)
{
	if (constValue == 0) {
		value = CellValue::NullNumeric;
	}
}

bool MultiplicationProcessor::next()
{
	vector<CellValueStream *> *operands = nextAll();
	if (operands) {
		double numValue = 1;
		for (vector<CellValueStream *>::const_iterator operand = operands->begin(); operand != operands->end(); ++operand) {
			const CellValue &operandVal = (*operand) ? (*operand)->getValue() : constValue;
			if (operandVal.isNumeric()) {
				numValue *= operandVal.getNumeric();
			} else {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "MultiplicationProcessor::next invalid operand type!");
			}
		}
		value = numValue;
	}
	return (operands != 0);
}

bool DivisionProcessor::next()
{
	vector<CellValueStream *> *operands = nextAll();
	if (operands) {
		double numValue = 0;
		for (vector<CellValueStream *>::const_iterator operand = operands->begin(); operand != operands->end(); ++operand) {
			const CellValue &operandVal = (*operand) ? (*operand)->getValue() : constValue;
			if (operandVal.isEmpty()) {
				numValue = 0;
				break;
			} else if (operandVal.isNumeric()) {
				if (numValue == 0) {
					numValue = operandVal.getNumeric();
				} else {
					numValue /= operandVal.getNumeric();
				}
			} else {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "DivisionProcessor::next invalid operand type!");
			}
		}
		value = numValue;
	}
	return (operands != 0);
}

bool AdditionProcessor::next()
{
	vector<CellValueStream *> *operands = nextAny();
	if (operands) {
		double numValue = 0;
		for (vector<CellValueStream *>::const_iterator operand = operands->begin(); operand != operands->end(); ++operand) {
			if (!(*operand)) {
				continue;
			}
			const CellValue &operandVal = (*operand)->getValue();
			if (operandVal.isNumeric()) {
				numValue += operandVal.getNumeric();
			} else {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "AdditionProcessor::next invalid operand type!");
			}
		}
		value = numValue;
	}
	return (operands != 0);
}

bool SubtractionProcessor::next()
{
	vector<CellValueStream *> *operands = nextAny();
	if (operands) {
		double numValue = 0;
		for (vector<CellValueStream *>::const_iterator operand = operands->begin(); operand != operands->end(); ++operand) {
			if (!(*operand)) {
				continue;
			}
			const CellValue &operandVal = (*operand)->getValue();
			if (operandVal.isNumeric()) {
				if (operand == operands->begin()) {
					numValue = operandVal.getNumeric();
				} else {
					numValue -= operandVal.getNumeric();
				}
			} else {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "SubtractionProcessor::next invalid operand type!");
			}
		}
		value = numValue;
	}
	return (operands != 0);
}

}
