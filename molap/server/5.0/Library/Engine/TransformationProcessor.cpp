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
 * 
 *
 */

#include "Logger/Logger.h"
#include "Engine/TransformationProcessor.h"

namespace palo {

TransformationProcessor::TransformationProcessor(PEngineBase engine, CPPlanNode node) : engine(engine), node(node), transformationPlanNode(dynamic_cast<const TransformationPlanNode *>(node.get())), pathTranslator(node->getArea()->getPathTranslator()), ruleId(node->getRuleId())
{
	CPArea targetArea = transformationPlanNode->getArea();
	CPArea sourceArea = transformationPlanNode->getChildren()[0]->getArea();
	Area::ConstDimIter targetSet = targetArea->dimBegin();
	Area::ConstDimIter sourceSet = sourceArea->dimBegin();
	moveToInKey = *sourceArea->pathBegin();

	outKey.resize(targetArea->dimCount());
	size_t lastExpand = 0;
	size_t dimOrdinal;
	for (dimOrdinal = 0;targetSet != targetArea->dimEnd(); ++targetSet, ++sourceSet, ++dimOrdinal) {
		if ((*targetSet)->size() == 1) {
			// target restricted to 1 element
			outKey[dimOrdinal] = *(*targetSet)->begin();
			expansions.push_back(pair<const Set *, Set::Iterator>((const Set *)0, Set::Iterator()));
		} else if (*targetSet == *sourceSet || **targetSet == **sourceSet) {
			// target dimension is source dimension
			dimMapping.push_back(DimMapping(dimOrdinal, dimOrdinal));
			expansions.push_back(pair<const Set *, Set::Iterator>((const Set *)0, Set::Iterator()));
			lastExpand = 0;
		} else if ((*sourceSet)->size() == 1 && (*targetSet)->size() > 1) {
			// expansion
			if (!lastExpand) {
				expansionRanges.push_back(make_pair(make_pair(dimOrdinal, dimOrdinal), moveToInKey));
			} else {
				expansionRanges.back().second = dimOrdinal;
			}
			Set::Iterator setBegin = (*targetSet)->begin();
			expansions.push_back(pair<const Set *, Set::Iterator>((*targetSet).get(), setBegin));
			outKey[dimOrdinal] = *setBegin;
			lastExpand = dimOrdinal;
		} else {
			const SetMultimaps *setMultiMaps = transformationPlanNode->getSetMultiMaps();
			if (!setMultiMaps || setMultiMaps->empty() || !setMultiMaps->at(dimOrdinal)) {
				// unsupported transformation
				Logger::error << "Unsupported transformation type in TransformationProcessor!" << endl;
				throw ErrorException(ErrorException::ERROR_INTERNAL, "Unsupported transformation type in TransformationProcessor!");
			} else {
				// multimapping in this dimension
				dimMapping.push_back(DimMapping(dimOrdinal, dimOrdinal));
				lastExpand = 0;
			}
		}
	}
	if (lastExpand) {
		// extend last expansionRange to the end
		expansionRanges.back().second = dimOrdinal-1;
	}
	factor = transformationPlanNode->getFactor();
	childSP.reset();
	child = 0;
}

//bool printOut = 1;

bool TransformationProcessor::next()
{
	bool hasNext = true;
	bool newStart = false;

	if (!child) {
		childSP = engine->createProcessor(transformationPlanNode->getChildren()[0]);
		child = childSP.get();
		bool found;
		hasNext = child->move(moveToInKey, &found);
		if (hasNext && !found) {
			moveToInKey = child->getKey();
		}
		lastInKey.clear();
		newStart = true;
	} else {
		// TODO: -jj- expand tail
		hasNext = child->next();
	}

	size_t nextMoveToFirstDim = outKey.size();
	const IdentifiersType *inKey = 0;

	if (hasNext || !newStart) {
		if (hasNext) {
			inKey = &child->getKey();
			const IdentifiersType compareToKey = lastInKey.empty() ? moveToInKey : lastInKey;
			// find first changed dimension of inKey
			for (size_t dim = 0; dim < inKey->size(); dim++) {
				if ((*inKey)[dim] != compareToKey[dim]) {
					nextMoveToFirstDim = dim;
					break;
				}
			}
		} else {
			nextMoveToFirstDim = 0;
		}

		// compare nextMoveToFirstDim to expansionRanges
		ExpansionRangesType::iterator eRFirstIt;
		for (eRFirstIt = expansionRanges.begin(); eRFirstIt != expansionRanges.end(); ++eRFirstIt) {
			if (nextMoveToFirstDim <= eRFirstIt->first.first) {
				break;
			}
		}
		if (eRFirstIt != expansionRanges.end()) {
			ExpansionRangesType::iterator eRIt = --expansionRanges.end();
			for (;;--eRIt) {
				// expand ranges from eRFirstIt to the end of expansionRanges in reverse order
				for (size_t expandDim = eRIt->first.second; expandDim >= eRIt->first.first; expandDim--) {
					if (expansions[expandDim].first) {
						++expansions[expandDim].second;
						if (expansions[expandDim].second == expansions[expandDim].first->end()) {
							expansions[expandDim].second = expansions[expandDim].first->begin();
							outKey[expandDim] = *expansions[expandDim].second;
							if (expandDim == 0) {
								return false;
							}
							// continue to next dimension
						} else {
							moveToInKey = eRIt->second;
							while (++eRIt != expansionRanges.end()) {
								eRIt->second = moveToInKey;
							}
							outKey[expandDim] = *expansions[expandDim].second;
//							if (inKey != moveToInKey) {
								// restart
								childSP.reset();
								child = 0;
//							} else {
								// TODO: -jj- optimize - do not reset child processor - just use the last key and value
//							}
							return next();
						}
					}
				}
				if (eRIt == eRFirstIt) {
					break;
				}
			}
			if (!hasNext) {
				return false;
			}
			if (eRIt == eRFirstIt) {
				// dimensions expanded completely - continue with current value
				moveToInKey = *inKey;
				eRIt->second = moveToInKey;
				while (++eRIt != expansionRanges.end()) {
					eRIt->second = moveToInKey;
				}
			}
		}

		if (!hasNext) {
			return false;
		}
		// transform the key here
		for (DimsMappingType::const_iterator transIt = dimMapping.begin(); transIt != dimMapping.end(); ++transIt) {
			outKey[transIt->first] = (*inKey)[transIt->second];
		}
		lastInKey = *inKey;
		// if the sub-area needs to be replicated
//		if (printOut) {
//			Logger::info << "out " << getKey() << " " << getDouble() << " " << moveToInKey << endl;
//		}
		return true;
	}
	return false;
}

const CellValue &TransformationProcessor::getValue()
{
	if (factor == 1.0) {
		return child->getValue();
	}
	const CellValue &val = child->getValue();
	if (val.isNumeric() && !val.isEmpty()) {
		value = val.getNumeric() * factor;
		value.setRuleId(ruleId);
		return value;
	} else if (ruleId != NO_IDENTIFIER) {
		value = val;
		value.setRuleId(ruleId);
		return value;
	} else {
		return val;
	}
}

double TransformationProcessor::getDouble()
{
	return getValue().getNumeric();
}

void TransformationProcessor::setValue(const CellValue &value)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::setValue not supported");
}

const IdentifiersType &TransformationProcessor::getKey() const
{
	return child ? outKey : EMPTY_KEY;
}

const GpuBinPath &TransformationProcessor::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void TransformationProcessor::reset()
{
	child->reset();
}

TransformationMapProcessor::TransformationMapProcessor(PEngineBase engine, CPPlanNode node) : TransformationProcessor(engine, node), multiMaps(0), endOfMultiMapping(true), node(node)
{
	const TransformationPlanNode *transformationPlanNode = dynamic_cast<const TransformationPlanNode *>(node.get());
	multiMaps = transformationPlanNode->getSetMultiMaps();
}

bool TransformationMapProcessor::next()
{
	bool result = true;
	if (mapOperations.empty()) {
		result = TransformationProcessor::next();
		mapOperations.clear();
		if (result) {
			size_t dimOrdinal = 0;
			for (SetMultimaps::const_iterator mm = multiMaps->begin(); mm != multiMaps->end(); ++mm, dimOrdinal++) {
				if (!*mm) {
					continue;
				}
				mapOperation mop;
				mop.end = (*mm)->end();
				mop.sourceId = getKey().at(dimOrdinal);
				mop.current = mop.begin = (*mm)->find(mop.sourceId);
				if (mop.begin == mop.end) {
					continue;
				}
				mop.dimOrdinal = dimOrdinal;
				outKey[dimOrdinal] = mop.current->second;
				++mop.current;
				if (mop.current == mop.end || mop.current->first != mop.sourceId) {
					// single mapping
					continue;
				}
				mop.current = mop.begin;
				mop.multiMap = mm->get();
				mapOperations.push_front(mop);
				endOfMultiMapping = false;
			}
		}
	}
	if (result) {
		// change the key
		for (MapOperations::const_iterator mop = mapOperations.begin(); mop != mapOperations.end(); ++mop) {
			outKey[mop->dimOrdinal] = mop->current->second;
		}
		if (endOfMultiMapping) {
			mapOperations.clear();
		} else {
			// next iteration - set multiMapping when last iteration reached
			MapOperations::iterator mop;
			for (mop = mapOperations.begin(); mop != mapOperations.end(); ++mop) {
				++(mop->current);
				if (mop->current == mop->end || mop->current->first != mop->sourceId) {
					mop->current = mop->begin;
				} else {
					break;
				}
			}
			if (mop == mapOperations.end()) {
				endOfMultiMapping = true;
			}
		}
	}
	return result;
}

}
