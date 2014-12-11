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

#include <algorithm>
#include "Engine/CombinationProcessor.h"

namespace palo {

CombinationProcessorBase::CombinationProcessorBase(CPPlanNode node) : index(-1), pathTranslator(node->getArea()->getPathTranslator())
{
	streams.resize(node->getChildren().size());
}

const CellValue &CombinationProcessorBase::getValue()
{
	if (index < 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid path in CombinationProcessorBase::getValue()");
	}
	return streams[index]->getValue();
}

double CombinationProcessorBase::getDouble()
{
	return getValue().getNumeric();
}

const IdentifiersType &CombinationProcessorBase::getKey() const
{
	if (index < 0) {
		return CellValueStream::EMPTY_KEY;
	}
	return streams[index]->getKey();
}

const GpuBinPath &CombinationProcessorBase::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void CombinationProcessorBase::reset()
{
	index = -1;
	size_t size = streams.size();
	for (size_t i = 0; i < size; i++) {
		streams[i]->reset();
	}
}

CombinationProcessor::CombinationProcessor(PEngineBase engine, CPPlanNode node) :
	CombinationProcessorBase(node), hasSameKey(false)
{
	const vector<PPlanNode> &children = node->getChildren();
	size_t size = children.size();
	isSame.resize(size);
	hasNext.resize(size);
	for (size_t i = 0; i < size; i++) {
		streams[i] = engine->createProcessor(children[i]);
	}
}

bool CombinationProcessor::next()
{
	size_t size = streams.size();
	if (index < 0) {
		hasSameKey = false;
		for (size_t i = 0; i < size; i++) {
			isSame[i] = false;
			hasNext[i] = streams[i]->next();
		}
	} else {
		hasNext[index] = streams[index]->next();
		if (hasSameKey) {
			for (size_t i = index + 1; i < size; i++) {
				if (isSame[i]) {
					isSame[i] = false;
					hasNext[i] = streams[i]->next();
				}
			}
		}
	}

	index = -1;
	for (size_t i = 0; i < size; i++) {
		if (hasNext[i]) {
			index = (int)i;
			break;
		}
	}
	if (index < 0) {
		return false;
	}

	hasSameKey = false;
	for (size_t i = index + 1; i < size; i++) {
		if (hasNext[i]) {
			int diff = CellValueStream::compare(streams[index]->getKey(), streams[i]->getKey());
			if (diff == 0) {
				isSame[i] = true;
				hasSameKey = true;
			} else if (diff > 0) {
				index = (int)i;
				for (size_t j = index + 1; j < i; j++) {
					isSame[j] = false;
				}
				hasSameKey = false;
			}
		}
	}
	return hasNext[index];
}

bool lessDCP(PPlanNode left, PPlanNode right)
{
	const Area *la = left->getArea().get();
	const Area *ra = right->getArea().get();

	for (size_t i = 0; i < la->dimCount(); i++) {
		CPSet ls = la->getDim(i);
		if (ls->size() == 1) {
			CPSet rs = ra->getDim(i);
			IdentifierType l = *ls->begin();
			IdentifierType r = *rs->begin();
			if (l < r) {
				return true;
			} else if (l > r) {
				return false;
			}
		}
	}
	return false;
}

DisjunctiveCombinationProcessor::DisjunctiveCombinationProcessor(PEngineBase engine, CPPlanNode node) :
	CombinationProcessorBase(node), dimCount(0)
{
	vector<PPlanNode> children = node->getChildren();
	size_t size = children.size();

	if (size) {
		std::stable_sort(children.begin(), children.end(), lessDCP);
		const Area *area = (*children.begin())->getArea().get();
		dimCount = (uint32_t)area->dimCount();
		setSize.resize(dimCount);
		for (size_t i = 0; i < dimCount; i++) {
			setSize[i] = (uint32_t)area->getDim(i)->size();
		}
		first.resize(size);
		firstDiff.resize(size - 1);
		pos.resize(size);
		counter.resize(size);
		for (size_t i = 0; i < size; i++) {
			counter[i].resize(dimCount);
		}
	}

	for (size_t i = 0; i < size; i++) {
		streams[i] = engine->createProcessor(children[i]);
	}
}

bool DisjunctiveCombinationProcessor::next()
{
	size_t size = streams.size();
	if (!dimCount || !size) {
		return false;
	}

	if (index < 0) {
		init();
		index = 0;
	}

	bool hasNext = false;
	while (index < (int)size) {
		if (pos[index] == (int32_t)dimCount) {
			if (first[index]) {
				hasNext = true;
				first[index] = false;
			} else {
				hasNext = streams[index]->next();
				if (!hasNext) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "internal error in DisjunctiveCombinationProcessor::next!");
				}
			}
			pos[index]--;
			break;
		}

		int32_t iDim;
		for (iDim = (int32_t)dimCount - 1; iDim >= 0; iDim--) {
			counter[index][iDim]++;
			if (counter[index][iDim] < setSize[iDim]) {
				break;
			}
		}
		pos[index] = iDim;
		if (iDim < 0) { // all keys from this stream are already returned
			index++;
		} else {
			if (index >= (int)firstDiff.size() || iDim > (int)firstDiff[index]) { // there is a set after firstDiff -> go back, if it is possible, otherwise continue with the same stream
				int i;
				for (i = index; i >= 0; i--) {
					if (index >= (int)firstDiff.size()) {
						if (pos[i] == -1) {
							break;
						}
					} else {
						if (pos[i] < (int32_t)firstDiff[index]) {
							break;
						}
					}
				}
				for (int j = i + 1; j <= index; j++) {
					for (size_t k = pos[j] + 1; k < dimCount; k++) {
						counter[j][k] = 0;
					}
					pos[j] = (int32_t)dimCount;
				}
				index = i + 1;
			} else { // there is a set before firstDiff -> check the next stream
				index++;
			}
		}
	}
	return hasNext;
}

void DisjunctiveCombinationProcessor::init()
{
	size_t size = streams.size();
	if (size) {
		streams[0]->next();
		for (size_t i = 0; i < size; i++) {
			first[i] = true;
			pos[i] = (int32_t)dimCount;
			for (size_t j = 0; j < dimCount; j++) {
				counter[i][j] = 0;
			}

			if (i) {
				const IdentifiersType &key = streams[i - 1]->getKey();
				streams[i]->next();
				const IdentifiersType &key2 = streams[i]->getKey();

				for (size_t j = 0; j < dimCount; j++) {
					if (setSize[j] == 1 && key[j] != key2[j]) {
						firstDiff[i - 1] = (uint32_t)j;
						break;
					}
				}
			}
		}
	}
}

ConstantProcessor::ConstantProcessor(CPPlanNode node) :	value(*node->getDefaultValue()), area(node->getArea()), path(area->pathBegin()), first(true)
{
}

bool ConstantProcessor::next()
{
	if (first) {
		path = area->pathBegin();
		first = false;
	} else {
		++path;
	}
	return (path != area->pathEnd());
}

}
