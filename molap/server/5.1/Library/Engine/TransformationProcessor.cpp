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

#include "Logger/Logger.h"
#include "Engine/StorageCpu.h"
#include "Engine/CombinationProcessor.h"
#include "Engine/TransformationProcessor.h"

namespace palo {

class CellValueStorage : public StorageCpu
{
public:
	CellValueStorage(PPathTranslator pathTranslator) : StorageCpu(pathTranslator, true) {}
	virtual ~CellValueStorage() {}

	virtual double convertToDouble(const CellValue &value);
	virtual void convertToCellValue(CellValue &value, double d) const;
	virtual bool isNumeric() const {return false;}

	virtual PCellStream commitChanges(bool checkLocks, bool add, bool disjunctive) {return PCellStream();};
	bool merge(const CPCommitable &o, const PCommitable &p) {return false;}
	PCommitable copy() const {return PCommitable();}

private:
	vector<CellValue> internedValues;
	CellValue value;
};

class MixedStorage : public StorageBase {
public:
	MixedStorage(PPathTranslator pathTranslator) : StorageBase(pathTranslator), valueStorage(new CellValueStorage(pathTranslator)), numericStorage(new StorageCpu(pathTranslator, true)) {}
	virtual ~MixedStorage() {}
	virtual PProcessorBase getCellValues(CPArea area);
	virtual bool setCellValue(PCellStream stream);
	virtual size_t valuesCount() const {return valueStorage->valuesCount() + numericStorage->valuesCount();}
	virtual bool merge(const CPCommitable &o, const PCommitable &p) {return false;}
	virtual PCommitable copy() const {return PCommitable();}

	virtual void setCellValue(CPArea area, const CellValue &value, OperationType opType) {}
	virtual bool setCellValue(PPlanNode plan, PEngineBase engine) {return false;}
	virtual void setCellValue(CPPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType) {}
	virtual PCellStream commitChanges(bool checkLocks, bool add, bool disjunctive) {return PCellStream();};
private:
	PStorageCpu valueStorage;
	PStorageCpu numericStorage;
};

PProcessorBase MixedStorage::getCellValues(CPArea area)
{
	if (!valueStorage->valuesCount()) {
		return numericStorage->getCellValues(area);
	} else if (!numericStorage->valuesCount()) {
		return valueStorage->getCellValues(area);
	} else {
		vector<PProcessorBase> nodes;
		nodes.push_back(numericStorage->getCellValues(area));
		nodes.push_back(valueStorage->getCellValues(area));
		return PProcessorBase(new CombinationProcessor(PEngineBase(), nodes, CPPathTranslator()));
	}
}

bool MixedStorage::setCellValue(PCellStream stream)
{
	StorageCpu::Writer numW(*numericStorage, true);
	StorageCpu::Writer valW(*valueStorage, true);
	bool numOrdered = true;
	bool valOrdered = true;
	IdentifiersType lastNum;
	IdentifiersType lastVal;
	while (stream->next()) {
		const IdentifiersType &key = stream->getKey();
		const CellValue &value = stream->getValue();
		IdentifiersType &last = value.isNumeric() ? lastNum : lastVal;
		bool &ordered = value.isNumeric() ? numOrdered : valOrdered;
		StorageCpu::Writer &sw = value.isNumeric() ? numW : valW;
		if (ordered && last.size()) {
			if (CellValueStream::compare(last, key) >= 0) {
				ordered = false;
			}
		}
		last = key;
		if (ordered) {
			sw.push_back(key, sw.getStorage().convertToDouble(value));
		} else {
			sw.getStorage().setCellValue(key, value);
		}
	}
	if (!numOrdered) {
		numericStorage->commitChanges(false, false, false);
	}
	if (!valOrdered) {
		valueStorage->commitChanges(false, false, false);
	}
	return !numOrdered || !valOrdered;
}

double CellValueStorage::convertToDouble(const CellValue &value)
{
	internedValues.push_back(value);
	return (double)internedValues.size();
}

void CellValueStorage::convertToCellValue(CellValue &value, double d) const
{
	value = internedValues[size_t(d-1)];
}

class SERVER_CLASS RearrangeProcessor : public ProcessorBase {
public:
	RearrangeProcessor(PProcessorBase inputSP, const vector<uint32_t> &dimensionMapping, CPArea targetArea, CPArea sourceArea);
	virtual ~RearrangeProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual const IdentifiersType &getKey() const;
	virtual const GpuBinPath &getBinKey() const;
	virtual void reset();
	virtual bool move(const IdentifiersType &key, bool *found);
private:
	void cacheInput();
	PProcessorBase inputSP;
	vector<uint32_t> dimensionMapping;
	CPArea targetArea;
	CPArea sourceArea;
	IdentifiersType outKey;
	vector<uint32_t> target2Source;
	vector<uint32_t> source2Target;
	struct MisplacedDimension {
		MisplacedDimension() {}
		MisplacedDimension(int targetOrdinal, int sourceOrdinal, const Set &set) : targetOrdinal(targetOrdinal), sourceOrdinal(sourceOrdinal), setCurrent(set.begin()), setBegin(set.begin()), setEnd(set.end()), set(&set) {}
		int targetOrdinal;
		int sourceOrdinal;
		Set::Iterator setCurrent;
		Set::Iterator setBegin;
		Set::Iterator setEnd;
		const Set *set;
	};
	typedef map<int, MisplacedDimension> MPDM;
	MPDM misplacedDimensions;
	PStorageBase storage;
	PProcessorBase cachedInputSP;
	ProcessorBase *cachedInput;
};

static ostream& operator<<(ostream& ostr, const IdentifiersType& v)
{
	bool first = true;
	ostr << dec;
	for (vector<IdentifierType>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if (!first) {
			ostr << ",";
		}
		if (*it == NO_IDENTIFIER) {
			ostr << "*";
		} else {
			ostr << *it;
		}
		first = false;
	}
	return ostr;
}

size_t rearrangeNextCount = 0;
size_t rearrangeMoveCount = 0;

RearrangeProcessor::RearrangeProcessor(PProcessorBase inputSP, const vector<uint32_t> &dimensionMapping, CPArea targetArea, CPArea sourceArea) :
		ProcessorBase(true, PEngineBase()), inputSP(inputSP), dimensionMapping(dimensionMapping),
		targetArea(targetArea), sourceArea(sourceArea), cachedInput(0)
{
	target2Source.resize(targetArea->dimCount());
	source2Target.resize(sourceArea->dimCount());
	vector<uint32_t>::const_iterator dmit = dimensionMapping.begin();
	while (dmit != dimensionMapping.end()) {
		uint32_t sourceOrdinal = *dmit++;
		if (dmit != dimensionMapping.end()) {
			uint32_t targetOrdinal = *dmit++;
			target2Source[targetOrdinal] = sourceOrdinal+1;
			source2Target[sourceOrdinal] = targetOrdinal+1;
		}
	}
	double iterationsCount = 1;
	for (vector<uint32_t>::const_iterator stit = source2Target.begin(); stit != source2Target.end(); ++stit) {
		if (*stit) {
			for (vector<uint32_t>::const_iterator stit2 = source2Target.begin(); stit2 != stit; ++stit2) {
				if (*stit2 && *stit < *stit2) {
					int sourceOrdinal = int(stit - source2Target.begin());
					int targetOrdinal = *stit - 1;
					MisplacedDimension mdr(targetOrdinal, sourceOrdinal, *sourceArea->getDim(sourceOrdinal));
					misplacedDimensions[targetOrdinal] = mdr;
					iterationsCount *= sourceArea->getDim(sourceOrdinal)->size();
					break;
				}
			}
		}
	}
	if (iterationsCount && Logger::isTrace()) {
		Logger::trace << "RearrangeProcessor created with " << iterationsCount << " iterations in " << misplacedDimensions.size() << " dimensions" << endl;
	}
}

bool RearrangeProcessor::next()
{
	rearrangeNextCount++;
	if (outKey.empty()) {
		cacheInput();
	}
	for(;;) {
		if (!cachedInput) {
			PArea selectArea(new Area(*sourceArea));
			// create new reader for current batch
			for(MPDM::const_iterator mdit = misplacedDimensions.begin(); mdit != misplacedDimensions.end(); ++mdit) {
				PSet s(new Set());
				outKey[mdit->first] = *mdit->second.setCurrent;
				s->insert(*mdit->second.setCurrent);
				selectArea->insert(mdit->second.sourceOrdinal, s);
			}
			cachedInputSP = storage->getCellValues(selectArea);
			cachedInput = cachedInputSP.get();
		}
		if (cachedInput->next()) {
			int sourceOrdinal = 0;
			IdentifiersType inKey = cachedInput->getKey();
			for(vector<uint32_t>::const_iterator stit = source2Target.begin(); stit != source2Target.end(); ++stit, ++sourceOrdinal) {
				if (*stit) {
					outKey[*stit-1] = inKey[sourceOrdinal];
				}
			}
//			if (Logger::isTrace()) {
//				Logger::trace << "RearrangeProcessor::next() out:" << outKey << " in: " << inKey << endl; // << outKey
//			}
			return true;
		}
		cachedInputSP.reset();
		cachedInput = 0;
		// iterate over misplacedDimensions
		MPDM::reverse_iterator mdit = misplacedDimensions.rbegin();
		while (mdit != misplacedDimensions.rend()) {
			++mdit->second.setCurrent;
			if (mdit->second.setCurrent != mdit->second.setEnd) {
//				if (Logger::isTrace()) {
//					Logger::trace << "change at " << (mdit - misplacedDimensions.rbegin()) << endl;
//				}
				break;
			}
			mdit->second.setCurrent = mdit->second.setBegin;
			++mdit;
		}
		if (mdit == misplacedDimensions.rend()) {
//			if (Logger::isTrace()) {
//				Logger::trace << "no more combinations in RearrangeProcessor" << endl;
//			}
			return false;
		}
	}
	return false;
}

bool RearrangeProcessor::move(const IdentifiersType &key, bool *found)
{
	rearrangeMoveCount++;
	if (outKey.empty()) {
		cacheInput();
	}
	// prepare key to move in source
	IdentifiersType moveToSourceKey(sourceArea->dimCount(), NO_IDENTIFIER);
	size_t targetDim = 0;
	for (vector<uint32_t>::const_iterator t2sit = target2Source.begin(); t2sit != target2Source.end(); ++t2sit, targetDim++) {
		if (*t2sit) {
			moveToSourceKey[*t2sit-1] = key[targetDim];
		}
	}
	bool bulkChanged = !cachedInput;

//	return ProcessorBase::move(key, found);

	// prepare source batch
	for (MPDM::iterator mdit = misplacedDimensions.begin(); mdit != misplacedDimensions.end(); ++mdit) {
		Set::Iterator newCurrent;
		if (key[mdit->first]) {
			newCurrent = mdit->second.set->find(key[mdit->first]);
			if (newCurrent == mdit->second.setEnd) {
				// problem
				throw ErrorException(ErrorException::ERROR_INTERNAL, "RearrangeProcessor::move(): key outside the area.");
			}
		} else {
			newCurrent = mdit->second.setBegin;
		}
		if (newCurrent != mdit->second.setCurrent) {
			mdit->second.setCurrent = newCurrent;
			bulkChanged = true;
		}
	}
	if (bulkChanged) {
		if (cachedInput) {
			cachedInputSP.reset();
			cachedInput = 0;
		}
		PArea selectArea(new Area(*sourceArea));
		// create new reader for current batch
		for(MPDM::const_iterator mdit = misplacedDimensions.begin(); mdit != misplacedDimensions.end(); ++mdit) {
			PSet s(new Set());
			outKey[mdit->first] = *mdit->second.setCurrent;
			s->insert(*mdit->second.setCurrent);
			selectArea->insert(mdit->second.sourceOrdinal, s);
		}
		cachedInputSP = storage->getCellValues(selectArea);
		cachedInput = cachedInputSP.get();
	}
	if (!cachedInput->move(moveToSourceKey, found)) {
		bool result = next();
		// nothing found in this bulk
//		if (Logger::isTrace()) {
//			Logger::trace << "RearrangeProcessor::move() out:" << (result ? outKey : EMPTY_KEY) << " move to key: " << key << endl; // << outKey
//		}
		return result;
	}
	int sourceOrdinal = 0;
	IdentifiersType inKey = cachedInput->getKey();
	for(vector<uint32_t>::const_iterator stit = source2Target.begin(); stit != source2Target.end(); ++stit, ++sourceOrdinal) {
		if (*stit) {
			outKey[*stit-1] = inKey[sourceOrdinal];
		}
	}
//	if (Logger::isTrace()) {
//		Logger::trace << "RearrangeProcessor::move() out:" << outKey << " move to key: " << key << endl; // << outKey
//	}
	return true;
}

const CellValue &RearrangeProcessor::getValue()
{
	return cachedInput->getValue();
}

double RearrangeProcessor::getDouble()
{
	return cachedInput->getDouble();
}

const IdentifiersType &RearrangeProcessor::getKey() const
{
	if (cachedInput) {
		return outKey;
	} else {
		return EMPTY_KEY;
	}
}

const GpuBinPath &RearrangeProcessor::getBinKey() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "Unsupported method RearrangeProcessor::getBinKey()");
}

void RearrangeProcessor::reset()
{
	for(MPDM::iterator mdit = misplacedDimensions.begin(); mdit != misplacedDimensions.end(); ++mdit) {
		mdit->second.setCurrent = mdit->second.setBegin;
	}
	cachedInputSP.reset();
	cachedInput = 0;
}

void RearrangeProcessor::cacheInput()
{
	outKey = *targetArea->pathBegin();
	PCellStream inStream(boost::dynamic_pointer_cast<CellValueStream, ProcessorBase>(inputSP));
	storage = PStorageBase(new MixedStorage(PPathTranslator()));
	storage->setCellValue(inStream);
	if (Logger::isTrace()) {
		Logger::trace << "RearrangeProcessor cached " << storage->valuesCount() << " values" << endl;
	}
}

TransformationProcessor::TransformationProcessor(PEngineBase engine, CPPlanNode node, bool sortedOutput)
 : ProcessorBase(sortedOutput, engine), node(node), transformationPlanNode(dynamic_cast<const TransformationPlanNode *>(node.get())),
   pathTranslator(node->getArea()->getPathTranslator()), nextResult(false)
{
	CPArea targetArea = transformationPlanNode->getArea();
	vector<uint32_t> target2Source;

	CPArea sourceArea = transformationPlanNode->getChildren()[0]->getArea();
	size_t targetDimOrd = 0;

	targetStartKey = *targetArea->pathBegin();

	const vector<uint32_t> &dimensionMapping = transformationPlanNode->getDimMapping();
	if (dimensionMapping.size()) {
		sourceStartKey = targetStartKey;

		target2Source.resize(targetArea->dimCount());
		vector<uint32_t>::const_iterator dmit = dimensionMapping.begin();
		while (dmit != dimensionMapping.end()) {
			uint32_t sourceOrdinal = *dmit++;
			if (dmit != dimensionMapping.end()) {
				uint32_t targetOrdinal = *dmit++;
				target2Source[targetOrdinal] = sourceOrdinal+1;
			}
		}
	} else {
		sourceStartKey = *sourceArea->pathBegin();
	}
	moveToInKey = sourceStartKey;

	mappedDimensions.resize(targetArea->dimCount());
	outKey.resize(targetArea->dimCount());
	expansions.resize(targetArea->dimCount());
	size_t dimOrdinal;
	for (dimOrdinal = 0; targetDimOrd < targetArea->dimCount(); targetDimOrd++, dimOrdinal++) {
//		if (*targetSet != *sourceSet && **targetSet != **sourceSet) {
//			if ((*sourceSet)->size() == 1 && (*targetSet)->size() > 1) {
//				// expansion
//				Logger::trace << "Expansion in dim: " << dimOrdinal << endl;
//			} else if ((*sourceSet)->size() == 1 && (*targetSet)->size() == 1) {
//				// mapping 1:1
//				Logger::trace << "Mapping in dim: " << dimOrdinal << endl;
//			} else {
//				// mapping N:M, 1:M
//				Logger::error << "Undefined in dim: " << dimOrdinal << endl;
//			}
//		} else {
//			// identical subset
////			Logger::trace << "Identical dim: " << dimOrdinal << endl;
//		}
		CPSet targetSet = targetArea->getDim(targetDimOrd);
		CPSet sourceSet;

		if (target2Source.size()) {
			if (target2Source[targetDimOrd]) {
				sourceSet = sourceArea->getDim(target2Source[targetDimOrd]-1);
			}
		} else if (targetArea->dimCount() == sourceArea->dimCount()) {
			// only if dimensionality is identical compare  source and target sets
			sourceSet = sourceArea->getDim(targetDimOrd);
		}
		if (targetSet->size() == 1) {
			// target restricted to 1 element
			outKey[dimOrdinal] = *targetSet->begin();
		} else if (sourceSet && (targetSet == sourceSet || *targetSet == *sourceSet)) {
			// target dimension is source dimension
			dimMapping.push_back(dimOrdinal);
			mappedDimensions[targetDimOrd] = true;
		} else if ((!sourceSet || sourceSet->size() == 1) && targetSet->size() > 1) {
			// expansion
			Set::Iterator setBegin = targetSet->begin();
			expansions[dimOrdinal]=ExpansionState(targetSet.get(), &moveToInKey);
			outKey[dimOrdinal] = *setBegin;
		} else {
			// more than one element in target - not identical to source, source has more than one element -> mapping N:M
			const SetMultimaps *setMultiMaps = transformationPlanNode->getSetMultiMaps();
			if (!setMultiMaps || setMultiMaps->empty() || !setMultiMaps->at(dimOrdinal)) {
				// unsupported transformation
				Logger::error << "Unsupported transformation type in TransformationProcessor!" << endl;
				throw ErrorException(ErrorException::ERROR_INTERNAL, "Unsupported transformation type in TransformationProcessor!");
			} else {
				// multimapping in this dimension
				dimMapping.push_back(dimOrdinal);
			}
		}
	}
	factor = transformationPlanNode->getFactor();
	childSP.reset();
	child = 0;
//	if (Logger::isTrace()) {
//		Logger::trace << this << " TransformationProcessor created" << endl; // << outKey
//	}
}

bool TransformationProcessor::nextIntern(int dimStart)
{
	int dim;
	bool calledFromMove;
	if (dimStart == int(outKey.size())) {
		dim = dimStart-1;
		calledFromMove = false;
	} else {
		dim = dimStart;
		calledFromMove = true;
	}
	bool result = false;
	while (dim >= -1) {
		if (dim == -1 || mappedDimensions[dim]) {
			// mapped dimension
//			int restartDim = dim;
			bool nextInput;
			int firstChangedInputDimension = -1;
			const IdentifiersType *foundInput = 0;
			if (!child) {
				if (!childSP) {
					childSP = createProcessor(transformationPlanNode->getChildren()[0], true);
					if (transformationPlanNode->getDimMapping().size()) {
						childSP = PProcessorBase(new RearrangeProcessor(childSP, transformationPlanNode->getDimMapping(), transformationPlanNode->getArea(), transformationPlanNode->getChildren()[0]->getArea()));
					}
				}
				child = childSP.get();
				bool found;
				//0, 8, 365, 32, 31, 5, 0, 0, 0
//				if (lastInKey.size() == 9 && lastInKey[0] == 0 && lastInKey[1] == 8 && lastInKey[2] == 365 && lastInKey[3] == 32 && lastInKey[4] == 31 && lastInKey[5] == 5 && lastInKey[6] == 0 && lastInKey[7] == 0 && lastInKey[8] == 0) {
//					int breakhere = 1;
//				}
				nextInput = child->move(moveToInKey, &found);
				if (nextInput) {
					foundInput = &child->getKey();
					int relativeReqInput = foundInput->compare(moveToInKey);
					if (relativeReqInput < 0) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::next() shouldn't happen #3!");
					} else if (relativeReqInput == 0) {
						// ok
						firstChangedInputDimension = outKey.size();
						if (!found) {
							throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::next() shouldn't happen #4!");
						}
					} else {
						if (nextResult) {
							firstChangedInputDimension = relativeReqInput-1;
						} else {
							// first value -> do not check if previous have to be repeated
							firstChangedInputDimension = dim;
						}
					}
					if (!nextResult) { // first move or next
						moveToInKey = *foundInput;
						for (int expandDim = dim; expandDim < int(outKey.size()); expandDim++) {
							if (expandDim >=0 && expansions[expandDim].set) {
								expansions[expandDim].resetKey = moveToInKey;
							}
						}
					}
				} else {
					if (nextResult) {
						if (calledFromMove) {
							// move called - repeat anything?
							firstChangedInputDimension = -1;
						} else {
							// regular next
							throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::next() shouldn't happen #2!");
						}
					} else {
						// no values in the child
						firstChangedInputDimension = dim;
					}
				}
			} else {
				nextInput = child->next();
				if (nextInput) {
					foundInput = &child->getKey();
					int relativeToLastInput = foundInput->compare(lastInKey);
					if (relativeToLastInput <= 0) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::next() shouldn't happen!");
					}
					firstChangedInputDimension = relativeToLastInput-1;
				}
			}

			bool restart = false;
			while (--dim >= firstChangedInputDimension) {
				if (dim < 0) {
					break;
				}
				if (expansions[dim].set) {
					if (++expansions[dim].sit == expansions[dim].set->end()) {
						// last iteration in the expanded dimension
						expansions[dim].sit = expansions[dim].set->begin();
						outKey[dim] = *expansions[dim].sit;
						if (foundInput) {
							for (int expandDim = dim; expandDim < int(outKey.size()); expandDim++) {
								if (expansions[expandDim].set) {
									expansions[expandDim].resetKey = *foundInput;
								}
							}
						}
						// check higher dimensions
					} else {
						// another expansion possible
						outKey[dim] = *expansions[dim].sit;
						// restart input
						if (calledFromMove) {
							for (MappedDimsType::const_iterator transIt = dimMapping.begin(); transIt != dimMapping.end(); ++transIt) {
								if (*transIt > dim) {
									moveToInKey[*transIt] = sourceStartKey[*transIt];
								} else if (*transIt <= firstChangedInputDimension) {
									// keep moveToInKey[*transIt]
								}
							}
							for (int mapDim = 0; mapDim < dim; mapDim++) {
								if (mappedDimensions[mapDim]) {
									if (expansions[dim].resetKey[mapDim] > moveToInKey[mapDim]) {
										for (;mapDim < dim; mapDim++) {
											moveToInKey[mapDim] = expansions[dim].resetKey[mapDim];
										}
									}
								}
							}
						} else {
							moveToInKey = expansions[dim].resetKey;
						}
						for (int expandDim = dim+1; expandDim < int(outKey.size()); expandDim++) {
							if (expansions[expandDim].set) {
								expansions[expandDim].resetKey = moveToInKey;
								expansions[expandDim].sit = expansions[expandDim].set->begin();
								outKey[expandDim] = *expansions[expandDim].sit;
							}
						}
						child->reset();
						child = 0;
						nextInput = false;
						dim--;
						restart = true;
						break;
					}
				}
			}
			if (nextInput) {
				for (MappedDimsType::const_iterator transIt = dimMapping.begin(); transIt != dimMapping.end(); ++transIt) {
					outKey[*transIt] = (*foundInput)[*transIt];
				}
				lastInKey = *foundInput;
				if (calledFromMove) {
					for (int expandDim = firstChangedInputDimension+1; expandDim < int(outKey.size()); expandDim++) {
						if (expansions[expandDim].set) {
							expansions[expandDim].resetKey = moveToInKey;
							expansions[expandDim].sit = expansions[expandDim].set->begin();
							outKey[expandDim] = *expansions[expandDim].sit;
						}
					}
				}
				result = true;
			}
			if (!restart) {
				break;
			}
		} else if (child && expansions[dim].set) {
			// expanded dimension
			if (++expansions[dim].sit != expansions[dim].set->end()) {
				outKey[dim] = *expansions[dim].sit;
				result = true;
				break;
			}
			// last element in the expanded dimension=> reset
			expansions[dim].sit = expansions[dim].set->begin();
			outKey[dim] = *expansions[dim].sit;
			dim--;
		} else {
			// single target element
			// nothing to do
			dim--;
		}
	}
	nextResult = result;
//	if (Logger::isTrace()) {
//		if (result) {
//			Logger::trace << this << " TransformationProcessor::next() out:" << outKey << endl; // << outKey
//		} else {
//			Logger::trace << this << " TransformationProcessor::next() end." << endl; // << outKey
//		}
//	}
	return result;
}

bool TransformationProcessor::next()
{
//	if (tproc == this) {
//		int breakhere = 1;
//	}
	bool result = nextIntern(outKey.size());
	nextResult = result;
//	if (Logger::isTrace()) {
//		if (result) {
//			Logger::info << this << " TransformationProcessor::next() out:" << outKey << endl; // << outKey
//		} else {
//			Logger::info << this << " TransformationProcessor::next() end." << endl; // << outKey
//		}
//	}
	return result;
}

bool TransformationProcessor::move(const IdentifiersType &key, bool *found)
{
	bool result = false;

//	if (tproc == this) {
//		int breakhere = 1;
//	}
	// 12,8,0,0,0,0,915,91
//	if ((key.size() == 8 && key[0] == 12 && key[1] == 8 && key[2] == 0 && key[3] == 0 && key[4] == 0 && key[5] == 0 && key[6] == 915 && key[7] == 91 /*&& key[8] == 0*/) /*&&
//		(outKey.size() == 9 && outKey[0] == 0 && outKey[1] == 8 && outKey[2] == 317 && outKey[3] == 197 && outKey[4] == 30 && outKey[5] == 4 && outKey[6] == 0 && outKey[7] == 0 && outKey[8] == 0)*/) {
//		int breakhere = 1;
//	}

#ifdef undefined
	result = CellValueStream::move(key, found);
#else
	int dim = outKey.size()-1;;
//	size_t firstOutputChange = key.size();
	bool outOfBoundaries = false;
	if (nextResult) {
		// check if not on the position or after
		const IdentifiersType &currentKey = getKey();
		int diff = currentKey.compare(key);
		if (diff >= 0) {
			if (found) {
				*found = diff == 0;
			}
			result = true;
		} else {
//			firstOutputChange = -diff-1;
		}
	}
	if (!result) {
		dim = -1;
		size_t outOfBoundariesDim = key.size();
		for (size_t verifyDim = 0; verifyDim < key.size(); verifyDim++) {
			IdentifierType reqElem = verifyDim > outOfBoundariesDim ? targetStartKey[verifyDim] : key[verifyDim];
			if (mappedDimensions[verifyDim]) {
				// mapped dimension
				if (outKey[verifyDim] != reqElem && dim == -1) {
					dim = int(verifyDim); // main loop starts from last mapped dimension
				}
				moveToInKey[verifyDim] = reqElem;
			} else {
				if (verifyDim > outOfBoundariesDim) {
					if (expansions[verifyDim].set) {
						expansions[verifyDim].sit = expansions[verifyDim].set->begin();
					}
					outKey[verifyDim] = reqElem;
				} else {
					if (expansions[verifyDim].set) {
						// expanded target
						expansions[verifyDim].sit = expansions[verifyDim].set->lowerBound(reqElem);
						if (expansions[verifyDim].sit == expansions[verifyDim].set->end()) {
							// no such value in area
							outOfBoundaries = true;
							break;
						}
						outKey[verifyDim] = *expansions[verifyDim].sit;
					} else {
						// single target
						if (outKey[verifyDim] > key[verifyDim]) {
							outOfBoundariesDim = verifyDim;
						} else if (outKey[verifyDim] < key[verifyDim]) {
							// no such value in area
							outOfBoundaries = true;
							break;
						}
					}
				}
			}
		}
		for (size_t verifyDim = 0; verifyDim < key.size(); verifyDim++) {
			if (expansions[verifyDim].set) {
				for (size_t mapDim = 0; mapDim < verifyDim; mapDim++) {
					expansions[verifyDim].resetKey[mapDim] = moveToInKey[mapDim];
				}
			}
		}

		if (childSP) {
			int relativeToInput = moveToInKey.compare(lastInKey);
			if (!relativeToInput) {
				// exact input
				if (found) {
					*found = outOfBoundariesDim == key.size();
				}
				result = true;
			} else {
				if (relativeToInput < 0) {
					// move back
					childSP->reset();
				} else {
					// move forward
				}
				child = 0;
			}
		}
	}

	if (!result && !outOfBoundaries) {
		result = nextIntern(dim);
	}
	if (result && found) {
		*found = outKey == key;
	}
	nextResult = result;
#endif
//	if (Logger::isTrace()) {
//		if (result) {
//			Logger::info << this << " TransformationProcessor::move() out:" << outKey << " for " << key <<  endl; // << outKey
//			int diff = outKey.compare(key);
//			if (diff < 0) {
//				Logger::info << this << " TransformationProcessor::move() out is invalid!" << endl; // << outKey
//				throw ErrorException(ErrorException::ERROR_INTERNAL, "TransformationProcessor::move() out is invalid!");
//			}
//		} else {
//			Logger::info << this << " TransformationProcessor::move() end." << " for " << key << endl; // << outKey
//		}
//	}
	return result;
}

const CellValue &TransformationProcessor::getValue()
{
	if (factor == 1.0) {
		return child->getValue();
	}
	value = child->getValue();
	if (value.isNumeric() && !value.isEmpty()) {
		value = value.getNumeric() * factor;
	}
	return value;
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
	if (child && nextResult) {
		return outKey;
	} else {
		return EMPTY_KEY;
	}
}

const GpuBinPath &TransformationProcessor::getBinKey() const
{
	pathTranslator->pathToBinPath(getKey(), const_cast<GpuBinPath &>(binPath));
	return binPath;
}

void TransformationProcessor::reset()
{
	if (child) {
		child->reset();
		child = 0;
	}
	for (size_t dim = 0; dim < outKey.size(); dim++) {
		if (expansions[dim].set) {
			expansions[dim].sit = expansions[dim].set->begin();
			expansions[dim].resetKey = sourceStartKey;
		}
	}
	outKey = targetStartKey;
	lastInKey.clear();
	nextResult = false;
//	Logger::info << this << " TransformationProcessor::reset" << endl; // << outKey
}

TransformationMapProcessor::TransformationMapProcessor(PEngineBase engine, CPPlanNode node, bool sortedOutput)
: TransformationProcessor(engine, node, sortedOutput), multiMaps(0), endOfMultiMapping(true), node(node), brokenOrder(false)
{
	const TransformationPlanNode *transformationPlanNode = dynamic_cast<const TransformationPlanNode *>(node.get());
	multiMaps = transformationPlanNode->getSetMultiMaps();
	if (sortedOutput && multiMaps) {
		// test the same sorting order in source and target area
		for (SetMultimaps::const_iterator smmit = multiMaps->begin(); !brokenOrder && smmit != multiMaps->end(); ++smmit) {
			if (!*smmit) {
				continue;
			}
			const SetMultimap &smm = **smmit;
			IdentifierType lastMax = 0;
			IdentifierType currentId = NO_IDENTIFIER;
			IdentifierType currentMax = 0;
			for (SetMultimap::const_iterator smit = smm.begin(); !brokenOrder && smit != smm.end(); ++smit) {
				if (smit->first != currentId) {
					currentId = smit->first;
					lastMax = currentMax;
					currentMax = smit->second;
				} else if (smit->second > currentMax) {
					currentMax = smit->second;
				}
				if (smit->second < lastMax) {
					Logger::debug << "TransformationMapProcessor: source and target area has different sorting!" << endl;
					brokenOrder = true;
					break;
				}
			}
		}
	}
}

PProcessorBase TransformationMapProcessor::getSortedResults()
{
	PProcessorBase result;
	if (multiMaps) {
		boost::shared_ptr<ICellMap<CellValue> > cellMap = CreateCellMap<CellValue>(multiMaps->size());
		while (next()) {
			cellMap->set(getKey(), getValue());
		}
		result = cellMap->getValues();
	}
	return result;
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
