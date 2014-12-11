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

#include "Engine/SequenceProcessor.h"

namespace palo {

SequenceProcessor::SequenceProcessor(PEngineBase engine, CPPlanNode node) : ProcessorBase(false, engine), engine(engine), children(node->getChildren()), currentStream(0), moveToNext(true), index(-1), pathTranslator(node->getArea()->getPathTranslator())
{
}

bool SequenceProcessor::next()
{
	bool result = false;
	do {
		if (moveToNext) {
			currentStream = 0;
			pStream.reset();
			if (index + 1 < (int)children.size()) {
				++index;
				pStream = createProcessor(children[index], false);
				currentStream = pStream.get();
			} else {
				return false;
			}
			moveToNext = false;
		}
		if (currentStream) {
			result = currentStream->next();
			if (!result) {
				moveToNext = true;
			}
		}
	} while (moveToNext);
	return result;
}

const CellValue &SequenceProcessor::getValue()
{
	if (!currentStream) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid path in SequenceProcessor::getValue()");
	}
	return currentStream->getValue();
}

double SequenceProcessor::getDouble()
{
	return getValue().getNumeric();
}

void SequenceProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No setValue for SequenceProcessor");
}

const IdentifiersType &SequenceProcessor::getKey() const
{
	if (!currentStream) {
		return EMPTY_KEY; // next was not called yet
	}
	return currentStream->getKey();
}

const GpuBinPath &SequenceProcessor::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void SequenceProcessor::reset()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No reset for SequenceProcessor");
}

}
