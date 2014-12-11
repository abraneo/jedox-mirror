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

#include "Engine/RoundCorrectProcessor.h"

namespace palo {

RoundCorrectProcessor::RoundCorrectProcessor(PEngineBase engine, CPRoundCorrrectPlanNode node) : ProcessorBase(true, engine), engine(engine),
		child(node->getChildren().at(0)), moveToNext(false), targetValue(node->getTargetValue()), unitValue(node->getUnitValue()), error(0),
		referenceValue(0), pathTranslator(node->getArea()->getPathTranslator())
{
}

bool RoundCorrectProcessor::next()
{
	bool result = false;
	if (!pStream) {
		pStream = createProcessor(child, true);
		if (!pStream) {
			return false;
		}
		moveToNext = pStream->next();
		if (moveToNext) {
			pStream.reset();
			return false;
		}
	}
	lastKey = pStream->getKey();
	lastValue = pStream->getValue();

	if (moveToNext) {
		result = true;
		moveToNext = pStream->next();
	}

	// round and correct lastValue
	if (moveToNext || !targetValue) {
		// round
		if (unitValue) {
			double correctedValue = lastValue.getNumeric() + error;
			int64_t multiplier = (int64_t)(correctedValue / unitValue);
			lastValue = unitValue * (double)multiplier;
			error = lastValue.getNumeric() - correctedValue;
		}
		referenceValue += lastValue.getNumeric(); // TODO -jj- multiply by weight from AggregationMap
	} else if (targetValue) {
		// correct - last value target set
		lastValue = targetValue - referenceValue;
	}

	return result;
}

const CellValue &RoundCorrectProcessor::getValue()
{
	return lastValue;
}

const IdentifiersType &RoundCorrectProcessor::getKey() const
{
	return lastKey;
}

double RoundCorrectProcessor::getDouble()
{
	return lastValue.getNumeric();
}

void RoundCorrectProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No setValue for RoundCorrectProcessor");
}

const GpuBinPath &RoundCorrectProcessor::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void RoundCorrectProcessor::reset()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No reset for RoundCorrectProcessor");
}

}
