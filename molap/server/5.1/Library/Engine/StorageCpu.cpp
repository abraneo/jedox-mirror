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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

//#define PROFILE_AGGREGATION
//#define PROFILE_NEXTINTERN
//#define TEST_MOVE_BEFORE
//#define PROFILE_INDEX

//#define PROFILE_COMMIT
//#define VALIDATE_COMMIT

#define PARALLEL_COMMIT

#include "palo.h"
#include "Olap/Context.h"
#include "Logger/Logger.h"

#include <iostream>

#include "Collections/CellBuffer.h"
#include "InputOutput/FileUtils.h"
#include "Exceptions/ErrorException.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp> //include all types plus i/o

#include "Olap/Server.h"
#include "Engine/Legacy/Engine.h"

#include "AggregationProcessor.h"
#include "CombinationProcessor.h"
#include "ArithmeticProcessors.h"
#include "StorageCpu.h"

using namespace boost::posix_time;

namespace palo {

static size_t commitCounter = 0;
#ifdef VALIDATE_COMMIT
static size_t TST_START = 0;
static int TST_CHUNK = 0; //57
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static INSTR *pINSTR;

static const int ELEM_SIZE[] = {0,1,2,4};
static const int VALUE_SIZE[] = {0,1,4,8,4};

StorageCpu::StorageCpu(PPathTranslator pathTranslator, bool indexEnabled) :
	StorageBase(pathTranslator), valCount(0), emptySpace(0), index2(indexEnabled ? new vector<Bookmark> : 0),
	delCount(0), indexEnabled(indexEnabled), pageList(new SlimVector<uint8_t>(STORAGE_PAGE_SIZE))
{
	if (!pINSTR) {
		pINSTR = new INSTR[256];
		for (int i = 0; i < 255; i++) {
			pINSTR[i].elementIdType = (i & ELEMENTMASK);
			pINSTR[i].valueType = (i & VALUEMASK);
			pINSTR[i].instructionSize = 1+(i & JUMP ? 4 : 0) + ELEM_SIZE[pINSTR[i].elementIdType >> 1] + VALUE_SIZE[pINSTR[i].valueType >> 3];
		}
		pINSTR[255].elementIdType = 0;
		pINSTR[255].valueType = 0;
		pINSTR[255].instructionSize = 1;
	}
}

StorageCpu::StorageCpu(const StorageCpu &storage) :
	StorageBase(storage), endStack(storage.endStack), valCount(storage.valCount), emptySpace(storage.emptySpace), index2(storage.index2),
	delCount(0), longJumps(storage.longJumps), indexEnabled(storage.indexEnabled), changedCells(storage.changedCells), changeNodes(storage.changeNodes), pageList(storage.pageList)
{
}

ostream& operator<<(ostream& ostr, const vector<size_t>& v)
{
	bool first = true;
	ostr << "[" << dec;
	for (vector<size_t>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if (!first) {
			ostr << ",";
		}
		if (*it == NO_OFFSET) {
			ostr << "*";
		} else {
			ostr << *it;
		}
		first = false;
	}
	ostr << "]";
	return ostr;
}

ostream& operator<<(ostream& ostr, const IdentifiersType& v)
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

StorageCpu::Bookmark::Bookmark(const IdentifiersType &key) : position(0), key(key)
{

}

StorageCpu::Bookmark::Bookmark(const StorageCpu &storage) : position(0)
{
	const vector<StorageCpu::ElemRestriction> &endStack = storage.endStack;
	offsets.reserve(endStack.size());
	key.reserve(endStack.size());
	setPosition(storage.pageList.get()->size(), 0);

	for (vector<StorageCpu::ElemRestriction>::const_iterator si = endStack.begin(); si != endStack.end(); ++si) {
		if (si->elementId == NO_IDENTIFIER) {
			break;
		}
		key.push_back(si->elementId);
		offsets.push_back(si->iOffset);
	}
}

void StorageCpu::Bookmark::setPosition(size_t position, size_t page) {
	if (page) {
		int64_t distanceFromPageStart = int64_t(position) - page * 16384;
		if (distanceFromPageStart < 0  || distanceFromPageStart >= 16384) {
			Logger::error << "Bookmark.setPosition() parameter out of range! distanceFromPageStart: " << distanceFromPageStart << endl;
		}
	}
	this->position = position;
}

ostream& operator<<(ostream& ostr, const StorageCpu::Bookmark &bookmark)
{
	ostr << "{Pos: " << bookmark.position << " Key: [" << bookmark.key << "] Offsets: " << bookmark.offsets << "}";
	return ostr;
}

const char StorageCpu::FILE_TAG[] = {'M', 'D', 'S', '1'};

ostream& operator<<(ostream& ostr, const StorageCpu& ds)
{
	enum Step {
		ReadInstruction, ReadElemId, ReadValue, ReadJump
	};

	ostr << "StorageId: " << ds.getId() << ", size: " << ds.pageList->size() << ", pageCount: " << ds.pageList->pageCount() << endl;
	vector<Slim<uint8_t>::PSlimPage> &pages = ds.pageList->getPages();

	size_t nopCount = 0;
	uint8_t instr = 0;
	Step step = ReadInstruction;

	uint8_t *pval8;
	uint16_t val16;
	uint32_t val32;
	float valfloat;

	for (size_t i = 0; i < pages.size(); i++) {
		Slim<uint8_t>::PSlimPage page = pages[i];
		ostr << "page: " << i << endl;

		for (uint32_t j = 0; j < STORAGE_PAGE_SIZE;) {
			switch (step) {
			case ReadInstruction: {
				size_t offset = i * STORAGE_PAGE_SIZE + j;
				instr = page->at(j++);
				if (instr == NOP) {
					nopCount++;
				} else {
					if (nopCount) {
						ostr << "NOP: " << nopCount << endl;
						nopCount = 0;
					}

					if (instr & JUMP) {
						ostr << "offset: " << offset << ", ";
						step = ReadJump;
					} else if (instr & ELEMENTMASK) {
						step = ReadElemId;
					} else if (instr & VALUEMASK) {
						step = ReadValue;
					}
				}
				break;
			}
			case ReadJump: {
				pval8 = (uint8_t *)&val32;
				*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
				if (val32 >= ds.LONG_JUMP_START_VALUE) {
					size_t longJump = ds.longJumps[val32 - ds.LONG_JUMP_START_VALUE];
					ostr << "jump: " << longJump << ", ";
				} else {
					ostr << "jump: " << val32 << ", ";
				}

				if (instr & ELEMENTMASK) {
					step = ReadElemId;
				} else if (instr & VALUEMASK) {
					step = ReadValue;
				} else {
					step = ReadInstruction;
				}
				break;
			}
			case ReadElemId: {
				int elementIdType = (instr & ELEMENTMASK);
				IdentifierType elemId;

				if (elementIdType == ELEMENT8) {
					elemId = page->at(j++);
				} else if (elementIdType == ELEMENT16) {
					pval8 = (uint8_t *)&val16;
					*pval8++ = page->at(j++); *pval8++ = page->at(j++);
					elemId = val16;
				} else if (elementIdType == ELEMENT32) {
					pval8 = (uint8_t *)&val32;
					*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
					elemId = val32;
				} else {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "Unknown elementIdType");
				}
				ostr << "elemId: " << elemId << ", ";

				if (instr & VALUEMASK) {
					step = ReadValue;
				} else {
					step = ReadInstruction;
				}
				break;
			}
			case ReadValue: {
				int valueType = (instr & VALUEMASK);
				double value;

				if (valueType == VALUEU8) {
					value = page->at(j++);
				} else if (valueType == VALUE32) {
					pval8 = (uint8_t *)&val32;
					*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
					value = val32;
				} else if (valueType == VALUEF32) {
					pval8 = (uint8_t *)&valfloat;
					*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
					value = valfloat;
				} else if (valueType == VALUE64) {
					pval8 = (uint8_t *)&value;
					*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
					*pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++); *pval8++ = page->at(j++);
				}
				ostr << "value: " << value << endl;
				step = ReadInstruction;
				break;
			}
			default:
				throw ErrorException(ErrorException::ERROR_INTERNAL, "Unknown instruction");
			}
		}
		if (nopCount) {
			ostr << "NOP: " << nopCount << endl;
			nopCount = 0;
		}
	}

//	ostr << "size: " << dec << ds.pageList->size() << " data: " << hex;
//	ostr.flush();
//	size_t counter = 0;
//	for (SlimVector<uint8_t>::const_iterator it = ds.pageList->begin(); it != ds.pageList->end() && counter < 512 ; ++it, ++counter) {
//		ostr.width(2);
//		ostr.fill('0');
//		ostr << (int)*it << " ";
//	}
//	ostr << endl << dec;
//	if (ds.valCount) {
//		ostr << ds.valCount;
//	}
	return ostr;
}

//void StorageCpu::clear()
//{
//	pageList.reset(new SlimVector<uint8_t>(STORAGE_PAGE_SIZE));
//	endStack.clear();
//	valCount = 0;
//	emptySpace = 0;
//	index2.reset(indexEnabled ? new vector<Bookmark> : 0);
//}

void StorageCpu::buildIndex()
{
	checkCheckedOut();
	if (indexEnabled) {
		index2.reset(new vector<Bookmark>);

#ifdef PROFILE_INDEX
		ptime indexingStart = microsec_clock::local_time();
#endif
		Processor reader(*this, pageList.get(), PPathTranslator());
		reader.indexStorage();
#ifdef PROFILE_INDEX
    	ptime indexingEnd(microsec_clock::local_time());
    	size_t indexingTime = (indexingEnd-indexingStart).total_microseconds();
    	Logger::info << "Stream indexed in " << indexingTime/1000 << " ms. Total index entries: " << this->valCount << " pages: " << index2->size() << endl;
#endif
#ifdef VALIDATE_COMMIT
		if (!validate(true)) {
			Logger::error << "Validation failed" << endl;
			throw;
		}
#endif
	}
}

PProcessorBase StorageCpu::getCellValues(CPArea area)
{
	Processor *processor = 0;
	if (area) {
		if (index2 && !index2->empty()) {
			// find bookmark
			Bookmark bookmark(*area->pathBegin());
			vector<Bookmark>::iterator bit = lower_bound (index2->begin(), index2->end(), bookmark);
			if (bit != index2->begin()) {
				--bit;
				processor = new Processor(*this, pageList.get(), area, 0, &*bit);
			}
		}
		if (!processor) {
			processor = new Processor(*this, pageList.get(), area);
		}
	} else {
		processor = new Processor(*this, pageList.get(), pathTranslator);
	}
	return PProcessorBase(processor);
}

CellValue StorageCpu::getCellValue(const IdentifiersType &key)
{
	CellValue result;
	if (index2 && !index2->empty()) {
		Bookmark bookmark(key);
		vector<Bookmark>::iterator bit = lower_bound (index2->begin(), index2->end(), bookmark);
		if (bit != index2->begin()) {
			--bit;
			Processor reader(*this, pageList.get(), PPathTranslator(), 0, &*bit);
			reader.setFilter(&key[0], (int)key.size(), true);
			if (reader.next() && key==reader.getKey()) {
				result = reader.getValue();
			} else {
				result = CellValue::NullNumeric;
			}
			return result;
		}
	}

	Processor reader(*this, pageList.get(), PPathTranslator()); // translator not needed in this processor
	reader.setFilter(&key[0], (int)key.size(), true);
	if (reader.next() && key==reader.getKey()) {
		result = reader.getValue();
	} else {
		result = CellValue::NullNumeric;
	}
	return result;
}

bool StorageCpu::setCellValue(PCellStream stream)
{
	checkCheckedOut();
	if (!pageList->isCheckedOut()) {
		pageList = COMMITABLE_CAST(SlimVector<uint8_t>, pageList->copy());
	}
	Writer sw(*this, true);
	bool ordered = true;
	IdentifiersType last;
	size_t fromPosition = pageList.get()->size();
	while (stream->next()) {
		const IdentifiersType &key = stream->getKey();
		if (ordered && last.size()) {
			if (CellValueStream::compare(last, key) >= 0) {
				ordered = false;
			}
		}
		last = key;
		if (ordered) {
			sw.push_back(key, convertToDouble(stream->getValue()));
		} else {
			setCellValue(key, stream->getValue());
		}
	}
	if (!ordered) {
		commitChanges(false, false, false);
	} else {
		if (fromPosition) {
			buildIndex();
		}
	}
	return !ordered;
}

void StorageCpu::setCellValue(const IdentifiersType &key, const CellValue &val)
{
	checkCheckedOut();
	if (!changedCells) {
		changedCells = CreateDoubleCellMap(key.size());
	}
	changedCells->set(key, convertToDouble(val));
}

void StorageCpu::setCellValue(CPArea area, const CellValue &value, OperationType opType)
{
	if (value.isString()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::setCellValue string value cannot be stored!");
	}
	CellValue cellValue;
	if ((opType == SET && value.isEmpty() && area->getSize() > 1) || (opType == MULTIPLY_EXISTING && value.getNumeric() == 0)) {
		// clear existing
		vector<PPlanNode> baseNodes;
		baseNodes.push_back(PPlanNode(new SourcePlanNode(getId(), area, getObjectRevision())));
		baseNodes.push_back(PPlanNode()); // empty plan activating the constant multiplication
		changeNodes.push_back(PPlanNode(new PlanNode(MULTIPLICATION, area, baseNodes, 0, PCube(), 0)));
	} else if (opType == MULTIPLY_EXISTING) {
		// multiply existing
		vector<PPlanNode> baseNodes;
		baseNodes.push_back(PPlanNode(new SourcePlanNode(getId(), area, getObjectRevision())));
		baseNodes.push_back(PPlanNode()); // empty plan activating the constant multiplication
		changeNodes.push_back(PPlanNode(new PlanNode(MULTIPLICATION, area, baseNodes, 0, PCube(), value.getNumeric())));
	} else if (opType == SET) {
		if (area->getSize() == 1) {
			setCellValue(*area->pathBegin(), value);
		} else {
			changeNodes.push_back(PPlanNode(new ConstantPlanNode(area, value)));
		}
	} else if (opType == ADD_ALL) {
		if (area->getSize() == 1) {
			Area::PathIterator pit = area->pathBegin();
			if (changedCells) {
				changedCells->add(*pit, value.getNumeric());
			} else {
				setCellValue(*pit, value);
			}
		} else {
			changeNodes.push_back(PPlanNode(new ConstantPlanNode(area, value)));
		}
	}
}

bool StorageCpu::setCellValue(PPlanNode plan, PEngineBase engine)
{
	bool result = false;

	// set new values
	changeNodes.push_back(plan);

	PCellStream oldData = getCellValues(plan->getArea());
	if (oldData && oldData->next()) {
		result = true;
		// delete old values
		vector<PPlanNode> baseNodes;
		baseNodes.push_back(PPlanNode(new SourcePlanNode(getId(), plan->getArea(), getObjectRevision())));
		baseNodes.push_back(PPlanNode()); // empty plan activating the constant multiplication
		changeNodes.push_back(PPlanNode(new PlanNode(MULTIPLICATION, plan->getArea(), baseNodes, 0, PCube(), 0)));
	}
	return result;
}

PCellStream StorageCpu::commitChanges(bool checkLocks, bool add, bool disjunctive)
{
	size_t valuesCount = size_t(-1);
	PProcessorBase st;
	if (changedCells && changedCells->size()) {
		disjunctive = false;
		changeNodes.push_back(PPlanNode(new CellMapPlanNode(changedCells)));
		if (changeNodes.size() == 1) {
			valuesCount =  changedCells->size();
		}
	}
	if (changeNodes.size()) {
		PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
		if (changeNodes.size() == 1) {
			st = engine->createProcessor(changeNodes[0], true, false);
		} else {
			if (add) {
				CPPlanNode node(new PlanNode(ADDITION, changeNodes[0]->getArea(), changeNodes, 0, CPCube()));
				st = engine->createProcessor(node, true, false);
			} else {
//				size_t countDisj = 0;
//				size_t countNorm = 0;
				PPathTranslator pt = changeNodes[0]->getArea()->getPathTranslator();
				if (disjunctive) {
					vector<vector<PPlanNode> > combNodes;
					vector<bool> skipEmpty;

					for (size_t i = 0; i < changeNodes.size(); i++) {
						const CompleteNodeInfo *cni = dynamic_cast<const CompleteNodeInfo *>(changeNodes[i].get());
						bool skip = !cni || cni->skipEmpty();

						if (skipEmpty.empty() || skipEmpty[skipEmpty.size() - 1] != skip) {
							combNodes.push_back(vector<PPlanNode>());
							skipEmpty.push_back(skip);
						}
						combNodes[combNodes.size() - 1].push_back(changeNodes[i]);
//						skip ? countNorm++ : countDisj++;
					}

					if (combNodes.size() == 1) {
						if (skipEmpty[0]) {
							st = PProcessorBase(new CombinationProcessor(engine, changeNodes, pt));
						} else {
							st = PProcessorBase(new DisjunctiveCombinationProcessor(engine, changeNodes, pt));
						}
					} else {
						vector<PProcessorBase> procs(combNodes.size());
						for (size_t i = 0; i < combNodes.size(); i++) {
							if (combNodes[i].size() == 1) {
								procs[i] = engine->createProcessor(combNodes[i][0], true, false);
							} else if (skipEmpty[i]) {
								procs[i] = PProcessorBase(new CombinationProcessor(engine, combNodes[i], pt));
							} else {
								procs[i] = PProcessorBase(new DisjunctiveCombinationProcessor(engine, combNodes[i], pt));
							}
						}
						st = PProcessorBase(new CombinationProcessor(engine, procs, pt));
					}
//					Logger::debug << "all: " << countDisj + countNorm << ", disj: " << countDisj << ", normal: " << countNorm << endl;
				} else {
					st = PProcessorBase(new CombinationProcessor(engine, changeNodes, pt));
				}
			}
		}
	}

	PCellStream result;
	if (st) {
		st->setEngineLocked(true);
		result = commitExternalChanges(checkLocks, st, valuesCount, add);
	}
	changedCells.reset();
	changeNodes.clear();
	return result;
}

#ifdef PROFILE_NEXTINTERN
	static size_t nextInternCount = 0;
	static size_t nextBeforeCount = 0;
	static size_t nextLoopCount = 0;
#endif

struct StorageFragments
{
	Mutex collMutex;
	map<int, pair<int, PStorageBase> > storagesStartingWith;
	map<int, pair<int, PStorageBase> > storagesEndingWith;
};

static uint32_t countNops(const uint8_t *p, size_t count)
{
	uint32_t nops = 0;
	while (count) {
		uint8_t instruction = *p;
		if (instruction == NOP) {
			nops++;
		}
		size_t instructionSize = pINSTR[instruction].instructionSize;
		if (instructionSize > count) {
			instructionSize = count;
		}
		count -= instructionSize;
		p += instructionSize;
	}
	return nops;
}

class StorageCpuCommitWorker : public ThreadPoolJob {
public:
	static PStorageBase start(StorageCpu::Processor oldDataReader, PProcessorBase changes, size_t valuesCount, bool additive, PDoubleCellMap lockedChanges);
	const static size_t MAX_BULK_CHANGES = 10000;
private:
	StorageCpuCommitWorker(PThreadPool tp, ThreadPool::ThreadGroup &tg, PProcessorBase segmentReaderSP, PProcessorBase changes, int seqNr, bool additive, StorageFragments &results)
	: ThreadPoolJob(tg), tp(tp), additive(additive), results(results), segmentReaderSP(segmentReaderSP), changes(changes), seqNr(seqNr) {}
	StorageCpuCommitWorker(PThreadPool tp, ThreadPool::ThreadGroup &tg, PProcessorBase segmentReaderSP, PProcessorBase changes, int seqNr, bool additive, StorageFragments &results, PDoubleCellMap lockedChanges)
	: ThreadPoolJob(tg), tp(tp), additive(additive), results(results), lockedChanges(lockedChanges), segmentReaderSP(segmentReaderSP), changes(changes), seqNr(seqNr) {}
	virtual void operator()();
	static PStorageBase mergeSegment(bool additive, PProcessorBase segmentReaderSP, PProcessorBase changes, int seqNr, PDoubleCellMap lockedChanges);
	PStorageBase mergeParts(PStorageBase s1, PStorageBase s2);
	void storeSegment(int seqNr, PStorageBase mergedSegmentSP);

	// globals
	PThreadPool tp;
	bool additive;
	StorageFragments &results;
	PDoubleCellMap lockedChanges;

	// active segment specific
	PProcessorBase segmentReaderSP;
	PProcessorBase changes;

	int seqNr;
};

PStorageBase StorageCpuCommitWorker::mergeParts(PStorageBase s1SP, PStorageBase s2SP)
{
	StorageCpu *s2 = dynamic_cast<StorageCpu *>(s2SP.get());
	StorageCpu *s1 = dynamic_cast<StorageCpu *>(s1SP.get());

	StorageCpu::Processor p(*s2, s2->pageList.get(), CPArea(), 0);
	size_t newCount = s1->valCount + s2->valCount;
	if (p.next()) {
		StorageCpu::Writer w(*s1, true);

		w.movePages(&p, 0);
#ifdef VALIDATE_COMMIT
//		if (commitCounter >= TST_START && seqNr >= TST_CHUNK) {
//			if (!s1->validate(false)) {
//				Logger::error << "Merge of segments failed" << endl;
////				throw;
//			} else {
////				Logger::debug << commitCounter << "-" << seqNr << " validated" << endl;
//			}
//		}
#endif
	}
	s1->valCount = newCount;
	s1->delCount += s2->delCount;

	return s1SP;
}

void StorageCpuCommitWorker::storeSegment(int seqNr, PStorageBase mergedSegmentSP)
{
	bool insert = true;
	int startSeqNr = seqNr;
	int endSeqNr = seqNr;
	while (insert) {
		insert = false;
		PStorageBase s1;
		PStorageBase s2;
		int mergedStart, mergedEnd;
		{
			WriteLocker wl(&results.collMutex);
			map<int, pair< int, PStorageBase>  >::iterator colit;

			colit = results.storagesEndingWith.find(startSeqNr-1); // find previous
			if (colit != results.storagesEndingWith.end()) {
				// append to previous
				mergedStart = colit->second.first;
				mergedEnd = endSeqNr;
				s1 = colit->second.second;
				s2 = mergedSegmentSP;
				results.storagesStartingWith.erase(mergedStart);
				results.storagesEndingWith.erase(startSeqNr-1);
//				Logger::info << "appending fragment to previous" << endl;
				insert = true;
			} else {
				colit = results.storagesStartingWith.find(endSeqNr+1); // find next
				if (colit != results.storagesStartingWith.end()) {
					// append to next
					mergedStart = startSeqNr;
					mergedEnd = colit->second.first;
					s1 = mergedSegmentSP;
					s2 = colit->second.second;
					results.storagesStartingWith.erase(endSeqNr+1);
					results.storagesEndingWith.erase(mergedEnd);
//					Logger::info << "appending fragment to next" << endl;
					insert = true;
				} else {
					// nothing to merge with, just store
					results.storagesStartingWith.insert(make_pair(startSeqNr, make_pair(endSeqNr, mergedSegmentSP)));
					results.storagesEndingWith.insert(make_pair(endSeqNr, make_pair(startSeqNr, mergedSegmentSP)));
				}
			}
		}
		if (insert) {
#ifdef VALIDATE_COMMIT
			if (startSeqNr >= TST_CHUNK && commitCounter >= TST_START) {
				StorageCpu *sg1 = dynamic_cast<StorageCpu *>(s1.get());
				StorageCpu *sg2 = dynamic_cast<StorageCpu *>(s2.get());
//				if (!sg1->validate(false)) {
//					Logger::error << "Segment1 validation failed" << endl;
//				}
//				if (!sg2->validate(false)) {
//					Logger::error << "Segment2 validation failed" << endl;
//				}
			}
#endif
			// merge two fragments and continue
			mergedSegmentSP = mergeParts(s1, s2);
#ifdef VALIDATE_COMMIT
//			StorageCpu *s1 = dynamic_cast<StorageCpu *>(mergedSegmentSP.get());
//			if (commitCounter > TST_START && /* */!s1->validate(false)) {
//				Logger::error << "Merge of segments " << startSeqNr << "-" << endSeqNr << " to " << mergedStart << "-" << mergedEnd << " failed" << endl;
//				throw;
//			} else {
//				Logger::debug << commitCounter << " " << startSeqNr << "-" << endSeqNr << " validated" << endl;
//			}
#endif
			startSeqNr = mergedStart;
			endSeqNr = mergedEnd;
		}
	}
}

void StorageCpuCommitWorker::operator()()
{
	PStorageBase mergedSegmentSP = mergeSegment(additive, segmentReaderSP, changes, seqNr, lockedChanges);
	storeSegment(seqNr, mergedSegmentSP);
}

PStorageBase StorageCpuCommitWorker::mergeSegment(bool additive, PProcessorBase segmentReaderSP, PProcessorBase changes, int seqNr, PDoubleCellMap lockedChanges)
{
	StorageCpu::Processor *segmentReader = dynamic_cast<StorageCpu::Processor *>(segmentReaderSP.get());
	StorageCpu *mergedSegment = new StorageCpu(PPathTranslator(), segmentReader->storage.indexEnabled);
	PStorageBase mergedSegmentSP(mergedSegment);

	changes->reset();
	if (!changes->next()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpuCommitWorker::mergeSegment empty change list!");
	}
	changes->reset();

	PProcessorBase toStore;
	size_t oldSegmentSize;
	if (segmentReader->getEndPosition() >= segmentReader->getPosition()) {
		oldSegmentSize = segmentReader->getEndPosition() - segmentReader->getPosition();
	} else {
		Logger::warning << "Wrong reader position. endp: " << segmentReader->getEndPosition() << " size: " << segmentReader->storage.pageList->size() << endl;
		oldSegmentSize = segmentReader->storage.pageList->size() - segmentReader->getPosition();
	}
	bool rebuild = false;
	if (lockedChanges) {
		// do not rebuild if cells are locked
	} else if (oldSegmentSize == 0) {
		toStore = changes;
		rebuild = true;
	}

	if (rebuild) {
		if (!toStore) {
			vector<PProcessorBase> inputs;
			inputs.push_back(changes);
			inputs.push_back(segmentReaderSP);
			if (additive) {
				toStore.reset(new AdditionProcessor(inputs));
			} else {
				toStore.reset(new CombinationProcessor(PEngineBase(), inputs, CPPathTranslator()));
			}
		}
		StorageCpu::Writer writer(*mergedSegment, true);
		size_t tstCount = 0;
		while(toStore->next()) {
			if (toStore->getDouble()) {
				writer.push_back(toStore->getKey(), toStore->getDouble());
			}
			tstCount++;
		}
	} else { // merge interleaved
		if (mergedSegment->index2) {
			mergedSegment->index2->reserve(oldSegmentSize/mergedSegment->pageList->maxPageSize()+16);
		}
		changes->reset();
		StorageCpu::Processor p1(*segmentReader);
		StorageCpu::Writer w(*mergedSegment, true);

		bool contPrev = segmentReader->next();
		bool contComp = additive ? false : p1.next();
		bool diffValue = false;
		size_t valsDiff = 0;

		while (changes->next()) {
			const IdentifiersType &key = changes->getKey();

			double val = changes->getDouble();
			if (contPrev) {
				bool found = false;
				double oldVal = 0;
				if (contComp) {
					if (CellValueStream::compare(key, p1.getKey()) == 0) {
						oldVal = p1.getDouble();
						contComp = p1.next();
						found = true;
					} else {
						contComp = p1.move(key, &found);
						if (found) {
							oldVal = p1.getDouble();
							contComp = p1.next();
						} else {
							if (!val) {
								found = true;
							}
						}
					}
				}
				if (found) {
					if (oldVal == val || (segmentReader->storage.isNumeric() && StorageCpu::compareDouble(oldVal, val))) {
						continue;
					}
				}
				diffValue = true;
		 		contPrev = w.writeUntil(segmentReader, &key, rebuild);
			}
			if (contPrev && CellValueStream::compare(key, segmentReader->getKey()) == 0) {
				if (lockedChanges) {
					lockedChanges->set(segmentReader->getKey(), segmentReader->getDouble());
				}
				if (additive) {
					val += segmentReader->getDouble();
				}
				if (val) {
					w.push_back(key, val);
				} else {
					if (diffValue) {
						mergedSegment->delCount++;
					}
					--valsDiff;
				}
				contPrev = segmentReader->next();
			} else {
				if (val) {
					++valsDiff;
					w.push_back(key, val);
				} else {
					if (diffValue) {
						mergedSegment->delCount++;
					}
				}
				if (lockedChanges) {
					lockedChanges->set(key, 0);
				}
			}
		}
		if (contPrev) {
			w.writeUntil(segmentReader, 0, rebuild);
		}
		// change of count of values in segment
		mergedSegment->valCount = valsDiff;
	}
	// validate end stack
	vector<StorageCpu::ElemRestriction>::iterator esit1 = mergedSegment->endStack.begin();
	size_t s1Size = mergedSegment->pageList->size();
	const SlimVector<uint8_t> &sv = *mergedSegment->pageList;

	for(; esit1 != mergedSegment->endStack.end(); ++esit1) {
		const uint8_t *p = &(sv[esit1->iOffset]);
		if (*p & JUMP) {
			size_t distance1 = mergedSegment->getJump(p+1);
			if (distance1 + esit1->iOffset != s1Size) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpuCommitWorker::mergeSegment invalid endstack!");
			}
		} else {
			// only last dim has no jump
			if (esit1+1 != mergedSegment->endStack.end()) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpuCommitWorker::mergeSegment invalid endstack (JUMP)!");
			}
		}
	}

//	static bool testNops = false;
//	if (testNops && commitCounter >= 0) {
//		const vector<Slim<uint8_t>::PSlimPage> &pages = sv.getPages();
//		size_t countedFree = 0;
//
//		for (uint32_t iPage = 0; iPage < sv.pageCount(); iPage++) {
//			uint32_t nopsCount = countNops(&sv[iPage * sv.pageSize(0)], pages[iPage]->count());
//			if (pages[iPage]->getEmptyCount() != nopsCount) {
//				Logger::debug << "invalidnopcount " << commitCounter << " / " << seqNr << " " << pages[iPage]->getEmptyCount() << " != " << nopsCount << " page " << iPage <<  endl;
//			}
//			countedFree += nopsCount;
//		}
//		if (mergedSegment->emptySpace != countedFree) {
//			Logger::debug << "invalidnopcount " << commitCounter << " / " << seqNr << " " << mergedSegment->emptySpace << " != " << countedFree <<  endl;
//		}
//	}
	return mergedSegmentSP;
}

PStorageBase StorageCpuCommitWorker::start(StorageCpu::Processor oldDataReader, PProcessorBase changes, size_t valuesCount, bool additive, PDoubleCellMap lockedChanges)
{
	bool hasNext = true;
	bool nextOld = true;
	int seqNr = 0;
	size_t prepareTime = 0;

	if (!changes->next()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu empty change list!");
	}
	size_t dimensions = changes->getKey().size();

	if (valuesCount < MAX_BULK_CHANGES) {
		changes->reset();
		StorageCpu::Processor *segmentReader = new StorageCpu::Processor(oldDataReader);
		PProcessorBase segmentReaderSP(segmentReader);
		return StorageCpuCommitWorker::mergeSegment(additive, segmentReaderSP, changes, 0, lockedChanges);
	}
	if (lockedChanges) {
		lockedChanges = PDoubleCellMap(new CellMapSync<double>(lockedChanges));
	}
	StorageFragments results;
	Context *con = Context::getContext();
	PThreadPool tp = con->getServer()->getThreadPool();
	ThreadPool::ThreadGroup tg = tp->createThreadGroup();

	while (hasNext) {
		if (!con->check(false)) {
			break;
		}
		ptime prepareStart = microsec_clock::local_time();

		size_t bulkChanges = 0;
		PCellStreamBuffer changesBuffer(new CellStreamBuffer(dimensions, MAX_BULK_CHANGES));
		IdentifiersType lastChangedInBulk;

		for(;hasNext && bulkChanges < MAX_BULK_CHANGES-1;bulkChanges++) {
//			Logger::debug << seqNr << " " << changes->getKey() << endl;
			changesBuffer->push_back(changes->getKey(), changes->getDouble());
			hasNext = changes->next();
		}
		if (hasNext) {
//			Logger::debug << seqNr << " " << changes->getKey() << endl;
			lastChangedInBulk = changes->getKey();
			changesBuffer->push_back(changes->getKey(), changes->getDouble());
			hasNext = changes->next();
			bulkChanges++;
		}
		ptime prepareEnd = microsec_clock::local_time();
		prepareTime += size_t((prepareEnd-prepareStart).total_microseconds());

		if (!bulkChanges) {
			break;
		}
		StorageCpu::Processor *segmentReader = new StorageCpu::Processor(oldDataReader);
		PProcessorBase segmentReaderSP(segmentReader);

		vector<StorageCpu::ElemRestriction> sstack = oldDataReader.getStack(false);
		if (nextOld && hasNext) {
			bool oldFound = false;
			nextOld = oldDataReader.move(lastChangedInBulk, &oldFound);
			if (!nextOld || !oldFound) {
				oldDataReader.reset();
				if (oldDataReader.moveBefore(&lastChangedInBulk)) {
//					Logger::debug << oldDataReader.getKey() << " at position " << oldDataReader.getPosition() << endl;
				}
				segmentReader->endp = oldDataReader.getPosition();
			} else if (nextOld) {
				segmentReader->endp = oldDataReader.getPosition();
			}
		}

		StorageCpuCommitWorker *sw = new StorageCpuCommitWorker(tp, tg, segmentReaderSP, changesBuffer->getValues(), seqNr, additive, results, lockedChanges);
		PThreadPoolJob w(sw);
		if (/*false && */hasNext && tp->hasFreeCore(false)) {
			tp->addJob(w);
		} else {
			(*sw)();
		}

//		prepareEnd = microsec_clock::local_time();
//		size_t prepareTime = (prepareEnd-prepareStart).total_microseconds();


//		ptime mergeEnd(microsec_clock::local_time());
//		size_t mergeTime = (mergeEnd-prepareEnd).total_microseconds();
//		Logger::info << "preparation: " << prepareTime << " us merge: "<< mergeTime << " us result: " << mergedSegment->pageList->size() << " B" << endl;

		seqNr++;
	}
//	Logger::info << "total preparation time: " << prepareTime/1000 << " ms" << endl;
	tp->join(tg);
	con->check();

	if (results.storagesStartingWith.size() != 1) {
		Logger::error << "parallel commit failed. Result storages: " << results.storagesStartingWith.size() << endl;
	}

	map<int, pair<int, PStorageBase> >::iterator resit = results.storagesStartingWith.begin();
//	Logger::info << "commited in " <<  resit->second.first << " fragments" << endl;
	return resit->second.second;
}

void validateCommit(const char *prefix, PCellStream changes, PStorageBase merged, StorageCpu::Processor &p)
{
	PCellStream cs = merged->getCellValues(PArea());
	size_t testCounter = 0;
	changes->reset();
	p.reset();
	bool nextNew = cs->next();
	bool nextChanged = changes->next();
	bool nextOld = p.next();
	size_t missingNew = 0;
	size_t missingOld = 0;
	size_t remainingLines = 100;
	for (;;) {
		if (nextNew) {
			testCounter++;
			while (nextChanged && CellValueStream::compare(changes->getKey(), cs->getKey()) < 0)  {
				if (changes->getDouble()) {
					if (remainingLines) {
						cout << prefix << " c\t" << changes->getKey() << endl;
						remainingLines--;
					}
					missingNew++;
				}
				nextChanged = changes->next();
			}
			if (nextChanged && CellValueStream::compare(changes->getKey(), cs->getKey()) == 0) {
				nextChanged = changes->next();
			}
			while (nextOld && CellValueStream::compare(p.getKey(), cs->getKey()) < 0)  {
				if (changes->getDouble()) {
					if (remainingLines) {
						cout << prefix << " o\t" << p.getKey() << endl;
						remainingLines--;
					}
					missingOld++;
				}
				nextOld = p.next();
			}
			if (nextOld && CellValueStream::compare(p.getKey(), cs->getKey()) == 0) {
				nextOld = p.next();
			}
		} else {
			if (nextChanged) {
				do {
					if (changes->getDouble() != 0) {
						if (remainingLines) {
							cout << prefix << " c\t" << changes->getKey() << endl;
							remainingLines--;
						}
						missingNew++;
					}
				} while(changes->next());
			}
//			if (nextOld) {
//				do {
//					if (p.getDouble() != 0) {
//						if (remainingLines) {
//							cout << prefix << " o\t" << p.getKey() << endl;
//							remainingLines--;
//						}
//						missingOld++;
//					}
//				} while(p.next());
//			}
			break;
		}
		nextNew = cs->next();
	}
	if (merged->valuesCount() != testCounter || missingOld || missingNew) {
		Logger::info << "Values count test failed " << merged->valuesCount() << " " << testCounter << " " << missingOld << " " << missingNew << endl;
//		throw;
	}
}

PCellStream StorageCpu::commitExternalChanges(bool checkLocks, PProcessorBase changes, size_t valuesCount, bool add)
{
	commitCounter++;
#ifdef PROFILE_COMMIT
	ptime indexingStart = microsec_clock::local_time();
#ifdef PROFILE_NEXTINTERN
	nextInternCount = 0;
	nextBeforeCount = 0;
	nextLoopCount = 0;
#endif
#endif
	PStorageBase parallelMergedStorageSP;

	checkCheckedOut();
	PCellStream result;
	PDoubleCellMap res;
	bool append = false;
	if (!changes->next()) {
		return result;
	}
	if (endStack.empty()) {
		append = true;
	}
	size_t dimensions = changes->getKey().size();
	if (checkLocks) {
		res = CreateDoubleCellMap(dimensions);
	}
	if (!append) {
		IdentifiersType endKey;
		for (vector<ElemRestriction>::const_iterator it = endStack.begin(); it != endStack.end(); ++it) {
			endKey.push_back(it->elementId);
		}
		append = CellValueStream::compare(changes->getKey(), endKey) > 0;
	}
	changes->reset();
//	bool forcerebuild = false;
		{
			Processor p(*this, pageList.get(), CPArea(), 0);

#ifdef PROFILE_COMMIT
			ptime inputReadStart = microsec_clock::local_time();
//			Logger::debug << "Changes iteration 1" << endl;
			size_t inputSize = 0;
//			while (changes->next()) {
//				inputSize++;
//			}
//			changes->reset();
//			Logger::debug << "Changes iteration 2" << endl;
//			size_t inputSize2 = 0;
//			while (changes->next()) {
//				inputSize2++;
//			}
//			Logger::debug << "Changes iteration 3" <<  inputSize << " " << inputSize2 << endl;
//			changes->reset();
#endif

#ifdef PROFILE_COMMIT
			ptime parStart = microsec_clock::local_time();
#endif
			parallelMergedStorageSP = StorageCpuCommitWorker::start(p, changes, valuesCount, add, res);

			ptime parEnd(microsec_clock::local_time());
#ifdef PROFILE_COMMIT
			size_t parTime = size_t((parEnd-parStart).total_microseconds());
#endif
			StorageCpu *mergedStorage = dynamic_cast<StorageCpu *>(parallelMergedStorageSP.get());
			PProcessorBase resValues = parallelMergedStorageSP->getCellValues(PArea());

#ifdef PROFILE_COMMIT
			size_t mergedValues = 0;
//			double mergedSum = 0;
//			while(resValues->next()) {
//				mergedValues++;
//				mergedSum += resValues->getDouble();
//			}
//			Logger::info << "parallel commit " << parTime/1000 << " ms input of " << inputSize << " fetched in " << (parStart-inputReadStart).total_microseconds()/1000 << " ms values: " << mergedValues << "(" << mergedStorage->valuesCount() << ") size: " << mergedStorage->pageList->size() << " checksum: " << mergedSum << endl;
			Logger::info << "parallel commit " << parTime/1000 << " ms input of " << inputSize << " fetched in " << (parStart-inputReadStart).total_microseconds()/1000 << " ms values: " << mergedValues << "(" << mergedStorage->valuesCount() << ") size: " << mergedStorage->pageList->size() << " pages: " << mergedStorage->pageList->pageCount() << " bookmarks: " << (mergedStorage->index2 ? mergedStorage->index2->size() : 0 ) << endl;
#endif
//			{
//				size_t mergedValues = 0;
//				while(resValues->next()) {
//					mergedValues++;
//				}
//				if (mergedValues != valCount + mergedStorage->valCount) {
//					Logger::error << "# " << commitCounter << " StorageCpu valCount test failed!" << endl;
//					changes->reset();
//					throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu valCount test failed!");
//				}
//			}

			valCount += mergedStorage->valCount;
			mergedStorage->pageList->setOld(pageList->getOld() ? pageList->getOld() : pageList);
			pageList = mergedStorage->pageList;
			emptySpace = mergedStorage->emptySpace;
			index2.swap(mergedStorage->index2);
			endStack.swap(mergedStorage->endStack);
			if (longJumps.size() && !mergedStorage->longJumps.size()) {
				Logger::debug << longJumps.size() << " jumps lost!" << endl;
			}
			longJumps.swap(mergedStorage->longJumps);
			delCount = mergedStorage->delCount;
			if (Logger::isDebug() && longJumps.size()) {
				Logger::debug << "Merged jumps: " << longJumps << endl;
			}
//			{
//				int bytesPerValue = valCount ? pageList->size() / valCount : 0;
//				Logger::debug << commitCounter << " " << valCount << " " << pageList->size() << " " << emptySpace << " " << bytesPerValue << endl;
//			}

#ifdef VALIDATE_COMMIT
//			PStorageCpu newStorage = COMMITABLE_CAST(StorageCpu, parallelMergedStorageSP);
// problems			validateCommit("p",changes, newStorage, p);
#endif

			mergedStorage = 0;
			parallelMergedStorageSP.reset();

			changes->reset();
		}

#ifdef VALIDATE_COMMIT
	if (commitCounter >= TST_START && /*false && */!validate(false)) {
//		if (changes) {
//			changes->reset();
//			if (changes->next()) {
//				cout << keytoString(changes->getKey()) << endl;
//			}
//		}
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu validation failed!");
	}
#endif

#ifdef PROFILE_COMMIT
//	PProcessorBase resValues = getCellValues(PArea());
//	size_t mergedValues = 0;
//	double mergedSum = 0;
//	while(resValues->next()) {
//		mergedValues++;
//		mergedSum += resValues->getDouble();
//	}

//	Logger::debug << "storage value: " << this->valuesCount() << " size: " << this->pageList->size() << " emptySpace: " << emptySpace << " " << (this->pageList->size() ? 100*emptySpace/this->pageList->size() : 0) /*<< " counted: " << mergedValues << " checksum: " << mergedSum */ << endl;
	if (longJumps.size()) {
		Logger::debug << "long jumps: " << longJumps.size() << endl;
	}
	ptime indexingEnd(microsec_clock::local_time());
	size_t indexingTime = size_t((indexingEnd-indexingStart).total_microseconds());
	Logger::info << "commitExternalChanges nr." << commitCounter << " took " << indexingTime/1000 << " ms" << endl;
	Logger::info << commitCounter << " " << this->valuesCount() <<  " " << this->pageList->size() << " " << longJumps.size() << " " << (this->pageList->size() ? this->pageList->size()/this->valuesCount() : 0) << endl;

#ifdef PROFILE_NEXTINTERN
//	Logger::info << "nextIntern " << nextInternCount << " nextBeforeCount " << nextBeforeCount << " nextLoopCount " << nextLoopCount << endl;
#endif
//#ifndef _DEBUG
//	if (!validate(true)) {
//		Logger::error << "validation failed!" << endl;
//	}
//#endif
#endif
	if (checkLocks) {
		result = res->getValues();
	}

	return result;
}

bool StorageCpu::compareDouble(double d1, double d2)
{
	int64_t i1; // = *(int64_t *)&d1;
	memcpy(&i1, &d1, sizeof(int64_t));
	if (i1 < 0) {
		i1 = 0x8000000000000000 - i1; //two's complement
	}

	int64_t i2; // = *(int64_t *)&d2;
	memcpy(&i2, &d2, sizeof(int64_t));
	if (i2 < 0) {
		i2 = 0x8000000000000000 - i2;
	}

	int64_t i = i1 - i2;
	if (i == LLONG_MIN) { // i is LLONG_MIN e.g. when d1 = 2 and d2 = -2
		return false;     // llabs(0x8000000000000000) cannot be represented as int64_t, therefore it is 0x8000000000000000
	} else {
		int64_t diff = llabs(i);
		return diff < 8;
	}
}

bool StorageCpu::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	bool ret = true;
	mergeint(o,p);
	CPStorageCpu storage = CONST_COMMITABLE_CAST(StorageCpu, o);
	if (pageList->isCheckedOut()) {
		ret = pageList->merge(o != 0 ? storage->pageList : PCommitable(), shared_from_this());
	} else {
		if (storage) {
			pageList = storage->pageList;
			valCount = storage->valCount;
			emptySpace = storage->emptySpace;
			endStack = storage->endStack;
			index2 = storage->index2;
			longJumps = storage->longJumps;
		}
	}
	if (ret) {
		commitintern();
	}
	return ret;
}

PCommitable StorageCpu::copy() const
{
	checkNotCheckedOut();
	PStorageCpu newStorage(new StorageCpu(*this));
	return newStorage;
}

void StorageCpu::setStack(vector<ElemRestriction> &newStack)
{
	endStack = newStack;
	for (vector<ElemRestriction>::iterator it = endStack.begin(); it != endStack.end(); ++it) {
		it->iOffset = NO_OFFSET;	// not offset to current storage
		it->pOffset = NULL;
	}
}

void StorageCpu::load(FileReader *file, uint32_t fileVersion)
{
	pageList->load(file);

	uint64_t i;
	uint32_t i1;
	uint32_t i2;
	uint64_t u64;

	file->getRaw((char *)&i, sizeof(uint64_t));
	valCount = (size_t)i;

	file->getRaw((char *)&i, sizeof(uint64_t));
	emptySpace = (size_t)i;

	vector<Slim<uint8_t>::PSlimPage> &pages = pageList->getPages();

	file->getRaw((char *)&i, sizeof(uint64_t));
	for (uint64_t j = 0; j < i; j++) {
		file->getRaw((char *)&i1, sizeof(uint32_t));
		file->getRaw((char *)&i2, sizeof(uint32_t));
		if (i1 < pages.size()) {
			pages[i1]->setEmptyCount(i2);
		}
	}

	file->getRaw((char *)&i, sizeof(uint64_t));
	endStack.clear();
	endStack.resize((size_t)i);
	for (size_t j = 0; j < (size_t)i; j++) {
		file->getRaw((char *)&i1, sizeof(uint32_t));
		if (fileVersion >= 2) {
			file->getRaw((char *)&u64, sizeof(uint64_t));
		} else {
			file->getRaw((char *)&i2, sizeof(uint32_t));
			u64 = i2;
		}
		ElemRestriction er(i1, (size_t)u64);
		endStack[j] = er;
	}
	if (fileVersion >= 2) {
		file->getRaw((char *)&i, sizeof(uint64_t));
		longJumps.clear();
		longJumps.resize((size_t)i);
		for (size_t j = 0; j < (size_t)i; j++) {
			file->getRaw((char *)&u64, sizeof(uint64_t));
			longJumps[j] = (size_t)u64;
		}
		if (Logger::isDebug() && longJumps.size()) {
			Logger::debug << "Loaded jumps: " << longJumps << endl;
		}
	}

	buildIndex();
}

void StorageCpu::save(FileWriter *file) const
{
	pageList->save(file);

	uint64_t i = valCount;
	file->appendRaw((const char *)&i, sizeof(uint64_t));

	i = emptySpace;
	file->appendRaw((const char *)&i, sizeof(uint64_t));

	const vector<Slim<uint8_t>::PSlimPage> &pages = pageList->getPages();
	i = 0;
	const uint32_t MIN_EMPTY_COUNT = 64;
	for (vector<Slim<uint8_t>::PSlimPage>::const_iterator pit = pages.begin(); pit != pages.end(); ++pit) {
		if ((*pit)->getEmptyCount() > MIN_EMPTY_COUNT) {
			i++;
		}
	}
	file->appendRaw((const char *)&i, sizeof(uint64_t));

	uint32_t j = 0;
	for (vector<Slim<uint8_t>::PSlimPage>::const_iterator pit = pages.begin(); pit != pages.end(); ++pit, ++j) {
		if ((*pit)->getEmptyCount() > MIN_EMPTY_COUNT) {
			file->appendRaw((const char *)&j, sizeof(uint32_t));
			i = (*pit)->getEmptyCount();
			file->appendRaw((const char *)&i, sizeof(uint32_t));
		}
	}

	i = endStack.size();
	file->appendRaw((const char *)&i, sizeof(uint64_t));
	for (vector<ElemRestriction>::const_iterator it = endStack.begin(); it != endStack.end(); ++it) {
		uint32_t j = it->elementId;
		file->appendRaw((const char *)&j, sizeof(uint32_t));
		i = it->iOffset;
		file->appendRaw((const char *)&i, sizeof(uint64_t));
	}
	i = longJumps.size();
	file->appendRaw((const char *)&i, sizeof(uint64_t));
	for (vector<size_t>::const_iterator it = longJumps.begin(); it != longJumps.end(); ++it) {
		i = *it;
		file->appendRaw((const char *)&i, sizeof(uint64_t));
	}
}

string StorageCpu::keytoString(const IdentifiersType& key)
{
	stringstream ret;
	ret << "(";
	for (IdentifiersType::const_iterator it = key.begin(); it != key.end(); ++it) {
		if (it != key.begin()) {
			ret << ", ";
		}
		ret << *it;
	}
	ret << ")";
	return ret.str();
}

PDoubleCellMap StorageCpu::getChangedCells() const
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::getChangedCells not implemented");
}

void StorageCpu::updateOld(CPStorageBase o)
{
	StorageBase::updateOld(o);
	CPStorageCpu st = CONST_COMMITABLE_CAST(StorageCpu, o);
	pageList->setOld(st->pageList->getOld() ? st->pageList->getOld() : st->pageList);
}

bool StorageCpu::validate(bool thorough)
{
//	Logger::info << "Validation started." << endl;
	Processor p(*this, pageList.get(), CPArea(), 0);
	Processor pm(*this, pageList.get(), CPArea(), 0);
	size_t checkedValues = 0;
	IdentifiersType lastKey;
	int lastDepth = 0;
	bool first = true;
	bool err = false;
	size_t valU8Count = 0;
	size_t val32Count = 0;
	size_t val64Count = 0;
	size_t valF32Count = 0;

	SlimVector<uint8_t> &svr = *pageList;

	try {
		// validate endstack
		{
			vector<StorageCpu::ElemRestriction>::iterator esit1 = endStack.begin();
			size_t s1Size = pageList->size();

			for(; esit1 != endStack.end(); ++esit1) {
				const uint8_t *p = &(svr[esit1->iOffset]);
				if (esit1->pOffset && esit1->pOffset != p) {
					Logger::error << "invalid endstack: wrong pOffset. commit nr: " << commitCounter << endl;
					err = true;
				}
				if (*p & JUMP) {
					size_t distance1 = getJump(p+1);
					if (distance1 + esit1->iOffset != s1Size) {
						Logger::error << "invalid endstack: jump not to end. commit nr: " << commitCounter << endl;
						err = true;
					}
				} else {
					// only last dim has no jump
					if (esit1+1 != endStack.end()) {
						Logger::error << "invalid endstack: missing jump. commit nr: " << commitCounter << endl;
						err = true;
					}
				}
			}
		}

		const SlimVector<uint8_t> &sv = *pageList.get();
		size_t lastPos=0;
		while (!err && 	p.next()) {
			++checkedValues;
			size_t currPos = p.getPosition();
			size_t nopsCount = 0;
			for (size_t pos = lastPos; pos < currPos; pos++) {
				if (sv[pos]==NOP) {
					nopsCount++;
				}
			}
			if (nopsCount > pageList->maxPageSize()) {
				Logger::error << "Too many ("<< nopsCount << ") NOPs between values " << keytoString(lastKey) <<  " - " << keytoString(p.getKey()) << " commit nr: " << commitCounter << " page " << lastPos/pageList->maxPageSize() << endl;
				err = true;
			}
			lastPos = currPos;

			uint8_t instruction = sv[p.currentPos];
			int ValueType = pINSTR[instruction].valueType;
			if (ValueType == VALUEU8) {
				valU8Count++;
			} else if (ValueType == VALUE32) {
				val32Count++;
			} else if (ValueType == VALUE64) {
				val64Count++;
			} else if (ValueType == VALUEF32) {
				valF32Count++;
			}

			const IdentifiersType &k = p.getKey();
			if (p.depth + 1 != (int32_t)k.size()) {
				Logger::error << "Key size of " << keytoString(k) << " differs from depth " << p.depth << " commit nr: " << commitCounter << endl;
				err = true;
			}
			for (int32_t i = 0; i < p.depth - 1; ++i) {
				if (sv[p.offset[i]] & JUMP) {
					size_t distance = getJump(&sv[p.offset[i]+1]);
					if (p.offset[i] + distance < sv.size() && sv[p.offset[i] + distance] == NOP) {
						Logger::error << "Key " << keytoString(k) << " index " << i << " jumps to NOP commit nr: " << commitCounter << endl;
						err = true;
					}
					if (p.offset[i] + distance > sv.size()) {
						Logger::error << "Key " << keytoString(k) << " index " << i << " jumps out of the boundaries" << endl;
						err = true;
					}
				} else {
					Logger::error << "Key " << keytoString(k) << " index " << i << " doesn't jump." << endl;
					err = true;
				}
			}
			if (!first) {
				if (lastKey.size() != k.size()) {
					Logger::error << "Key size of " << keytoString(k) << " differs from last key size " << keytoString(lastKey) << " commit nr. " << commitCounter << endl;
					err = true;
				}
				if (CellValueStream::compare(lastKey, k) >= 0) {
					Logger::error << "Key " << keytoString(k) << " is not coming after last key " << keytoString(lastKey) << endl;
					err = true;
				}
				if (lastDepth != p.depth) {
					Logger::error << "Depth " << p.depth << " differs from last depth " << lastDepth << endl;
					err = true;
				}
			}
			if (thorough) {
				pm.reset();
				bool found = false;
				if (!pm.move(k, &found)) {
					Logger::error << "Move to key " << keytoString(k) << " returned false" << endl;
				} else {
					if (!found) {
						Logger::error << "Key " << keytoString(k) << " and move key " << keytoString(pm.getKey()) << " differ." << endl;
						err = true;
					}
					if (pm.depth != p.depth) {
						Logger::error << "Depth " << p.depth << " and move depth " << pm.depth << " differ." << endl;
						err = true;
					}
				}
				if (!first) {
					pm.reset();
					pm.next();
					pm.moveBefore(&k);
					if (pm.getKey().size() != lastKey.size() || CellValueStream::compare(pm.getKey(), lastKey) != 0) {
						Logger::error << "Last key " << keytoString(lastKey) << " and move before key " << keytoString(pm.getKey()) << " differ." << endl;
						err = true;
					}
					if (pm.depth != lastDepth) {
						Logger::error << "Last depth " << lastDepth << " and move before depth " << pm.depth << " differ." << endl;
						err = true;
					}
				}
			}
			first = false;
			lastKey = k;
			lastDepth = p.depth;
		}
		if (valCount != checkedValues) {
			Logger::error << "Values: " << valCount << " values really in storage: " << checkedValues << endl;
			err = true;
		}
		if (index2 && !index2->empty()) {
			int iPage = 1;
			for (vector<Bookmark>::iterator bit = index2->begin(); !err && bit != index2->end(); ++bit, iPage++) {
//				Logger::debug << "Index page: " << iPage << " pos: " <<  bit->getPosition() << " key: " << bit->getKey() << " offsets: " << bit->getOffsets() << " jumps: " << bit->getJumps() << endl;
				if (bit->getPosition() < size_t(iPage) * pageList->maxPageSize() || bit->getPosition() > size_t(iPage+1) * pageList->maxPageSize()) {
					Logger::error << "Invalid index position. Page: " << iPage << " Position: " << bit->getPosition() << endl;
					err = true;
				}
				Processor pb(*this, pageList.get(), CPArea(), 0, &*bit);
				if (pb.next()) {
					const IdentifiersType &k = pb.getKey();
					pm.reset();
					if (pm.moveBefore(&k)) {
						IdentifiersType kb = pm.getKey();
						if (pm.next()) {
							if (pm.getKey() != k) {
								Logger::error << "Invalid index: move before.next.getKey failed. key: " << k << " Page: " << iPage << " reader.key: " << pm.getKey() << " reader pos: " << pm.getPosition() << " key before: " << kb << endl;
								err = true;
							}
						} else {
							Logger::error << "Invalid index: move before.next failed. key: " << k << " Page: " << iPage << endl;
							err = true;
						}
					} else {
						Logger::error << "Invalid index: move before failed. key: " << k << " Page: " << iPage << endl;
						err = true;
					}
				}
			}
		}
	} catch (ErrorException &e) {
		Logger::error << "Got exception: " << e.getMessage() << endl;
		err = true;
	}
	if (err) {
		Logger::info << "Validation finished with errors." << endl;
//		throw;
	} else {
//		Logger::debug << "value statistics: U8=" << valU8Count << " I32=" << val32Count << " F64=" << val64Count << " F32=" << valF32Count << endl;
//		Logger::info << "Validation finished OK." << endl;
	}
	return !err;
}

ostream& operator<<(ostream& ostr, const StorageCpu::Processor& sr)
{
	if (!sr.end()) {
		for (int dim = 0; dim <= sr.depth; dim++) {
			if (dim) {
				ostr << ',';
			}
			ostr << sr.key[dim];
		}
		CellValue v;
		sr.storage.convertToCellValue(v, sr.value);
		ostr << ';' << v << ';';
	}
	return ostr;
}

bool StorageCpu::Processor::next()
{
#ifdef PROFILE_NEXTINTERN
	int tests = 0;
	int pagesused = 0;
	int nopCount = 0;
	static size_t calls = 0;

	if (!(++calls % 100000)) {
//		Logger::debug << "calls: " <<  calls << endl;
	}
	if (before) {
		nextBeforeCount++;
	} else {
		nextInternCount++;
	}
#endif

	const SlimVector<uint8_t> &svr = *pageList;
	bool valueReached = false;
	int ValueType = 0;
	size_t nextPagePos = 0;
	const uint8_t *p = 0;
	do {
#ifdef PROFILE_NEXTINTERN
		nextLoopCount++;
#endif

		while (depth >= 0) { // terminate all open positions
			if (jump[depth] <= pos) {
				if (setSingle[depth] == NO_IDENTIFIER && (size_t)depth < setRange.size()) {
					setRange[depth] = setEndRange[depth];
				}
				depth--;
			} else {
				break;
			}
		}

		size_t lastpos = currentPos;
		currentPos = pos;
		size_t instrPos = 0;

		if (pos >= endp) {
			endReached = true;
		} else {
			if (pos < nextPagePos) {
				p += (pos - lastpos);
			} else {
				nextPagePos = ((pos / STORAGE_PAGE_SIZE)+1)*STORAGE_PAGE_SIZE;
				p = &svr[pos];
#ifdef PROFILE_NEXTINTERN
				pagesused++;
#endif
			}
			uint8_t instruction = p[instrPos++];
			if (instruction == NOP) {
#ifdef PROFILE_NEXTINTERN
				nopCount++;
#endif
				pos++;
				// find the limit position
				size_t stopPos = nextPagePos;
				if (depth >= 0 && jump[depth] < stopPos) {
					stopPos = jump[depth];
				}
				const uint8_t *px = p+instrPos;
				while (pos < stopPos && *px++ == NOP) {
#ifdef PROFILE_NEXTINTERN
				nopCount++;
#endif
					pos++;
				}
				continue;
			}
			uint32_t elementId;
			size_t jumpPosition;
			uint8_t *pval8;
			uint16_t val16;
			int32_t val32;
			float valfloat;

			// read jump value
			if (instruction & JUMP) {
				jumpPosition = storage.getJump(p+instrPos); instrPos += 4;
//				pval8 = (uint8_t *)&jumpPosition;
//				memcpy(pval8, p + instrPos, 4); instrPos += 4;
			} else {
				jumpPosition = 0;
			}
			// read elementId
			int elementIdType = pINSTR[instruction].elementIdType;
			if (elementIdType == ELEMENT8) {
				elementId = p[instrPos++];
			} else if (elementIdType == ELEMENT16) {
				pval8 = (uint8_t *)&val16;
				*pval8++ = p[instrPos++]; *pval8++ = p[instrPos++];
				elementId = val16;
			} else if (elementIdType == ELEMENT32) {
				pval8 = (uint8_t *)&elementId;
				memcpy(pval8, p + instrPos, 4); instrPos += 4;
			} else {
				elementId = NO_IDENTIFIER;
			}
			// store next dimension
			if (++depth >= stackSize) {
				if (elementId == NO_IDENTIFIER) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "Storage: Invalid code. First elementId cannot be incremented!");
				}
				key[depth] = elementId;
				prevKey[depth] = NO_IDENTIFIER;
				stackSize++;
			} else {
				prevKey[depth] = key[depth];
				if (elementId == NO_IDENTIFIER) {
					elementId = ++key[depth];
				} else {
					key[depth] = elementId;
				}
			}
			prevOffset[depth] = offset[depth];
			prevDepth = depth + 1;
			prevOffset[depth+1] = 0;
			prevKey[depth+1] = NO_IDENTIFIER;
			offset[depth] = currentPos;
			jump[depth] = jumpPosition + currentPos;
			ValueType = pINSTR[instruction].valueType;

			if (before) {
				if (!pFilter || compareKey(key, pFilter, depth) < 0) { // no filter or element before filter
					size_t nextPos = 0;
					if (jumpPosition) {
						nextPos = jumpPosition;
					} else {
						if (ValueType == VALUEU8) {
							nextPos = instrPos + 1;
						} else if (ValueType == VALUE32 || ValueType == VALUEF32) {
							nextPos = instrPos + 4;
						} else if (ValueType == VALUE64) {
							nextPos = instrPos + 8;
						}
					}
					uint8_t instruction = 0;
					if (pos + nextPos < endp) {
						for (instruction = svr[pos + nextPos]; instruction == NOP && pos + nextPos < endp; instruction = svr[pos + ++nextPos]);
					}
					if (depth > 0 ? (pos + nextPos < min(endp, jump[depth - 1])) : (pos + nextPos < endp)) {
						uint32_t nInstr = 0;
						size_t njump = 0;
						const uint8_t *p1 = &svr[pos + nextPos];
						if (instruction & JUMP) {
							njump = storage.getJump(p1 + nInstr + 1); nInstr += 4;
						}
						uint32_t nelementId;

						int nelementIdType = pINSTR[instruction].elementIdType;
						if (nelementIdType == ELEMENT8) {
							nelementId = p1[nInstr + 1];
						} else if (nelementIdType == ELEMENT16) {
							pval8 = (uint8_t *)&val16;
							*pval8++ = p1[nInstr + 1]; *pval8++ = p1[nInstr + 2];
							nelementId = val16;
						} else if (nelementIdType == ELEMENT32) {
							pval8 = (uint8_t *)&nelementId;
							memcpy(pval8, p1 + nInstr + 1, 4);
						} else {
							nelementId = elementId + 1;
						}
						if (pFilter) {
							if (nelementId <= pFilter[depth]) {
								instrPos = nextPos;
								ValueType = 0;
								jump[depth--] = pos + nextPos + njump;
							} else {
								// filter Id cannot be reached in this dim, ignore filter in following dims
								memset(pFilter + depth + 1, 255, (DIM_MAX - depth - 1) * sizeof(uint32_t));
							}
//						} else if (pos + nextPos + njump > endp) { // would leave the readers region - go deeper
//							// do nothing
//							int testbreak = 1;
						} else { // get next element
							instrPos = nextPos;
							ValueType = 0;
							jump[depth--] = min(endp, pos + nextPos + njump);
						}
					} else if (pFilter) {
						// filter Id cannot be reached in this dim, ignore filter in following dims
						memset(pFilter + depth + 1, 255, (DIM_MAX - depth - 1) * sizeof(uint32_t));
					}
				} else if (compareKey(key, pFilter, depth) > 0) {
					// only first subelement can cause this - other has lookahead
					do {
						pFilter[depth] = NO_IDENTIFIER;
						key[depth] = NO_IDENTIFIER;
						offset[depth] = 0;
						--depth;
					} while (depth >= 0  && prevKey[depth] == NO_IDENTIFIER);
					if (depth >= 0) {
						pos = prevOffset[depth];
						offset[depth] = prevOffset[depth];
						pFilter[depth] = prevKey[depth];
						key[depth] = prevKey[depth]-1;
						--depth;
					} else {
						// no prev value - set pos to beginning
						pos = 0;
						return false;
					}
					nextPagePos = 0;
					instrPos = 0;
					ValueType = 0;
				}
			} else {
				// compare element against set
				if (area) {
					bool found = false;
					bool endOfSet = true;
					if (setSingle[depth] == NO_IDENTIFIER) {
						if (setRange[depth] == setEndRange[depth]) {
							setRange[depth] = sets[depth]->rangeLowerBound(elementId);
//							setRange[depth] = sets[depth]->rangeBegin();
						}
						while (setRange[depth] != setEndRange[depth]) {
							if (elementId < setRange[depth].low()) {
								// element not in selection
								endOfSet = false;
								break;
							} else if (elementId <= setRange[depth].high()) {
								// element in selection
								found = true;
								break;
							} else {
								++setRange[depth];
							}
						}
					} else {
						if (setSingle[depth] == elementId) {
							found = true;
						} else if (setSingle[depth] > elementId) {
							endOfSet = false;
						}
					}
//					cout << elementId << '\t' << depth << '\t'<< found << '\t' << endOfSet << endl;
					if (!found) {
						// invalid element
						if (endOfSet) {
							// no more possible values in this branch -> fallback
							// after the searched element at this level - skip to next parent
							if (depth > 0) { // terminate all open positions
								instrPos = jump[--depth] - currentPos;
							} else {
								endReached = true;
							}
							ValueType = 0; // ignore the value
						} else {
							if (jumpPosition) {
								// jumpPosition == 0 => get next item - stream can never jump back
								instrPos = jumpPosition;
							} else {
								// skip value
								if (ValueType == VALUEU8) {
									instrPos++;
								} else if (ValueType == VALUE32 || ValueType == VALUEF32) {
									instrPos += 4;
								} else if (ValueType == VALUE64) {
									instrPos += 8;
								}
							}
							ValueType = 0; // ignore the value
						}
					} else {
						// valid element -> continue
					}
				}
				if (pFilter && pFilter[depth] != NO_IDENTIFIER) {
#ifdef PROFILE_NEXTINTERN
					tests++;
#endif
					if (elementId < pFilter[depth]) {
						// searched element not yet reached
						// invalid element -> skip data
						if (jumpPosition) {
							// jumpPosition == 0 => get next item - stream can never jump back
							instrPos = jumpPosition;
						} else {
							// skip value
							if (ValueType == VALUEU8) {
								instrPos++;
							} else if (ValueType == VALUE32 || ValueType == VALUEF32) {
								instrPos += 4;
							} else if (ValueType == VALUE64) {
								instrPos += 8;
							}
						}
						ValueType = 0; // ignore the value
					} else if (elementId == pFilter[depth]) {
						// element found -> continue
					} else {
						// if restricted to single cell (check previous dimensions) -> return from here
						int dim;
						for (dim = 0; dim < depth; dim++) {
							if (pFilter[depth] == NO_IDENTIFIER) {
								break;
							}
						}
						if (dim >= depth) {
							memset(pFilter + depth + 1, 255, (DIM_MAX - depth - 1) * sizeof(uint32_t));
						} else {
							// after the searched element at this level - skip to next parent
							if (depth >= 0) { // terminate all open positions
								if (setSingle[depth] == NO_IDENTIFIER && (size_t)depth < setRange.size()) {
									setRange[depth] = setEndRange[depth];
								}
								instrPos = jump[depth--] - currentPos;
							} else {
								endReached = true;
							}
							ValueType = 0; // ignore the value
						}
					}
				}
			}
			if (ValueType) {
				// read value
				if (ValueType == VALUEU8) {
					valueReached = true;
					value = p[instrPos++];
				} else if (ValueType == VALUE32) {
					valueReached = true;
					pval8 = (uint8_t *)&val32;
					memcpy(pval8, p + instrPos, 4); instrPos += 4;
					value = val32;
				} else if (ValueType == VALUEF32) {
					valueReached = true;
					pval8 = (uint8_t *)&valfloat;
					memcpy(pval8, p + instrPos, 4); instrPos += 4;
					value = valfloat;
				} else if (ValueType == VALUE64) {
					valueReached = true;
					double valdouble;
					memcpy(&valdouble, p + instrPos, 8); instrPos += 8;
					value = valdouble;
				}
			} else	if (mtCallback && !endReached) {
				mtCallback->readCallback();
			}
		}

		pos += instrPos;

	} while (!valueReached && !endReached);

	// TODO -jj- optimize, quick hack
	if (ValueType) {
		if (vkey.size() < (size_t)depth + 1) {
			vkey.resize(depth + 1);
		}
		memcpy(&vkey[0], key, (depth + 1)*sizeof(key[0]));
//	} else if (index && nopCount) {
//		Logger::info << "index nopCount: " << nopCount << endl;
	}
#ifdef PROFILE_NEXTINTERN
	if (tests) {
//		Logger::trace << value << '\t' << tests << '\t' << pagesused << endl;
	}
#endif
	return valueReached;
}

//static int myCount = 0;
//static int myLimit = -1;

void StorageCpu::Processor::buildIndex()
{
//	cout << "buildIndex" << endl;
	//set<Bookmark>::iterator insertHint = indexptr->end();

	if (!storage.index2 || before || area || pFilter) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Invalid call of StorageCpu::Processor::buildIndex()!");
	}
	storage.index2->clear();
	storage.index2->reserve(storage.pageList->pageCount());

	vector<size_t> offsetCache(storage.endStack.size(), NO_OFFSET);
	vector<size_t> jumpCache(storage.endStack.size(), 0);

//	uint32_t oldPage = 0;
//	set<Bookmark>::const_iterator oldIt = oldIndex.begin();

	int ValueType = 0;
	size_t nextPagePos = 0;
	const uint8_t *p = 0;

	const SlimVector<uint8_t> &svr = *pageList;


	do {
		while (depth >= 0) { // terminate all open positions
			if (jump[depth] <= pos) {
				depth--;
			} else {
				break;
			}
		}

		size_t lastpos = currentPos;
		currentPos = pos;
		size_t instrPos = 0;

		if (pos >= endp) {
			endReached = true;
		} else {
			if (pos < nextPagePos) {
				p += (pos - lastpos);
			} else {
				nextPagePos = ((pos / STORAGE_PAGE_SIZE)+1)*STORAGE_PAGE_SIZE;
				p = &svr[pos];
				if (pos >= STORAGE_PAGE_SIZE) {
//					uint32_t iPage = (uint32_t)(pos / STORAGE_PAGE_SIZE) - 1;
					Bookmark bookmark = getBookmark();
					if (offsetCache.empty()) {
						offsetCache.resize(bookmark.getOffsets().size(), NO_OFFSET);
						jumpCache.resize(bookmark.getOffsets().size(), 0);
					}
					storage.index2->push_back(bookmark);
				}
			}
			uint8_t instruction = p[instrPos++];
			if (instruction == NOP) {
				pos++;
				// find the limit position
				size_t stopPos = nextPagePos;
				if (depth >= 0 && jump[depth] < stopPos) {
					stopPos = jump[depth];
				}
				const uint8_t *px = p+instrPos;
				while (pos < stopPos && *px++ == NOP) {
					pos++;
				}
				continue;
			}
			uint32_t elementId;
			size_t jumpPosition;
			uint8_t *pval8;

			// read jump value
			if (instruction & JUMP) {
				jumpPosition = storage.getJump(p+instrPos); instrPos += 4;
			} else {
				jumpPosition = 0;
			}
			// read elementId
			int elementIdType = pINSTR[instruction].elementIdType;
			if (elementIdType == ELEMENT8) {
				elementId = p[instrPos++];
			} else if (elementIdType == ELEMENT16) {
				uint16_t val16;
				pval8 = (uint8_t *)&val16;
				*pval8++ = p[instrPos++]; *pval8++ = p[instrPos++];
				elementId = val16;
			} else if (elementIdType == ELEMENT32) {
				pval8 = (uint8_t *)&elementId;
				memcpy(pval8, p + instrPos, 4); instrPos += 4;
			} else {
				elementId = NO_IDENTIFIER;
			}
			// store next dimension
			if (++depth >= stackSize) {
				if (elementId == NO_IDENTIFIER) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "Storage: Invalid code. First elementId cannot be incremented!");
				}
				key[depth] = elementId;
				prevKey[depth] = NO_IDENTIFIER;
				stackSize++;
			} else {
				prevKey[depth] = key[depth];
				if (elementId == NO_IDENTIFIER) {
					elementId = ++key[depth];
				} else {
					key[depth] = elementId;
				}
			}
			prevKey[depth+1] = NO_IDENTIFIER;
			jump[depth] = jumpPosition + currentPos;
			offset[depth] = currentPos;
			ValueType = pINSTR[instruction].valueType;

			// jump closer to the end of current page
			if (jumpPosition && (currentPos+jumpPosition)/STORAGE_PAGE_SIZE == currentPos/STORAGE_PAGE_SIZE) {
				ValueType = 0; // ignore the value
				instrPos = jump[depth--] - currentPos;
			}

			if (ValueType) {
				if (ValueType == VALUEU8) {
					instrPos++;
				} else if (ValueType == VALUE32 || ValueType == VALUEF32) {
					instrPos += 4;
				} else if (ValueType == VALUE64) {
					instrPos += 8;
				}
			}
		}

		pos += instrPos;

	} while (!endReached);
}

void StorageCpu::Processor::aggregate(HashValueStorage *hashStorage, const AggregationMap **parentMaps)
{
	int ValueType = 0;
	size_t nextPagePos = 0;
	double currentValue = 0;
	const uint8_t *p = 0;
	int offsets[DIM_MAX];
	bool singleParent[DIM_MAX];
	bool hasMultiMap[DIM_MAX];
	uint8_t weightSlot[DIM_MAX];
	double weights[DIM_MAX];
	int weightsCount = 0;
	int cachedOffset = 0;
	AggregationMap::TargetReader targets[DIM_MAX];
	uint8_t multiDim[DIM_MAX];
	int multiCount = 0;
	const SlimVector<uint8_t> &svr = *pageList;

#ifdef PROFILE_AGGREGATION
	double sum = 0;
	size_t values = 0;
	ptime aggregationStart = microsec_clock::local_time();
	size_t nodesVisited = 0;
	size_t pagesVisited = 0;
#endif

	for (size_t dim = 0; dim < hashStorage->dimCount; dim++) {
		singleParent[dim] = parentMaps[dim]->getSingleTarget() != NO_IDENTIFIER;
		offsets[dim] = 0;
		hasMultiMap[dim] = parentMaps[dim]->hasMultiMap();
	}
	for (size_t dim = 0; dim < hashStorage->dimCount; dim++) {
		if (parentMaps[dim]->hasWeights()) {
			weightSlot[dim] = weightsCount+1;
			weights[weightsCount++] = 1.0;
		} else {
			weightSlot[dim] = 0;
		}
	}
//	cout << pos << "\tdepth: " << depth << endl;
	for (int dim = 0; dim < depth+1; dim++) {
//		cout << pos << "\tmp: " << multiParent[dim] << endl;
		if (sets[dim]->find(key[dim]) != sets[dim]->end()) {
			AggregationMap::TargetReader targetReader = parentMaps[dim]->getTargets(key[dim]);
			if (!singleParent[dim]) {
				if (targetReader.size() != 0) {
					if (hasMultiMap[dim]) {
						if (targetReader.size() > 1) {
							// element has multiple parents in resultset
							multiDim[multiCount] = (uint8_t)dim;
							targets[dim] = targetReader;
							multiCount++;
							if (weightSlot[dim]) {
								weights[weightSlot[dim]-1] = 1;
							}
						} else {
							// single parent
							offsets[dim] = hashStorage->offsets[dim][*targetReader-hashStorage->firstIds[dim]];
							cachedOffset += offsets[dim];
							if (weightSlot[dim]) {
								weights[weightSlot[dim]-1] = targetReader.getWeight();
							}
						}
					} else {
		//				cout << pos << "\tpar: " << parentId << endl;
						offsets[dim] = hashStorage->offsets[dim][*targetReader-hashStorage->firstIds[dim]];
		//				cout << pos << "\toff: " << offsets[dim] << endl;
						cachedOffset += offsets[dim];
						if (weightSlot[dim]) {
							weights[weightSlot[dim]-1] = targetReader.getWeight();
						}
					}
				} else {
					Logger::error << "StorageCpu::Processor::aggregate: targetReader is empty" << endl;
				}
			} else if (weightSlot[dim]) {
				weights[weightSlot[dim]-1] = targetReader.getWeight();
			}
		} else {
			pos = jump[dim];
			depth = dim-1;
		}
	}

	do {
		while (depth >= 0) { // terminate all open positions
			if (jump[depth] <= pos) {
				if (setSingle[depth] == NO_IDENTIFIER && (size_t)depth < setRange.size()) {
					setRange[depth] = setEndRange[depth];
				}
				if (targets[depth].size() > 1) {
					if (multiCount && multiDim[multiCount-1] == (uint8_t)depth) {
						multiCount--;
						targets[depth] = AggregationMap::TargetReader();
						offsets[depth] = 0;
					}
				}
				depth--;
			} else {
				break;
			}
		}

		size_t lastpos = currentPos;
		currentPos = pos;
		size_t instrPos = 0;

		if (pos >= endp) {
			endReached = true;
		} else {
#ifdef PROFILE_AGGREGATION
			++nodesVisited;
#endif
			if (pos < nextPagePos) {
				p += (pos - lastpos);
			} else {
#ifdef PROFILE_AGGREGATION
				++pagesVisited;
#endif
				nextPagePos = ((pos / STORAGE_PAGE_SIZE)+1)*STORAGE_PAGE_SIZE;
				p = &svr[pos];
			}
			uint8_t instruction = p[instrPos++];
			if (instruction == NOP) {
				pos++;
				continue;
			}
			uint32_t elementId;
			size_t jumpPosition;
			uint8_t *pval8;
			uint16_t val16;
			int32_t val32;
			float valfloat;

			// read jump value
			if (instruction & JUMP) {
				jumpPosition = storage.getJump(p+instrPos); instrPos += 4;
			} else {
				jumpPosition = 0;
			}
			// read elementId
			int elementIdType = pINSTR[instruction].elementIdType;
			if (elementIdType == ELEMENT8) {
				elementId = p[instrPos++];
			} else if (elementIdType == ELEMENT16) {
				pval8 = (uint8_t *)&val16;
				*pval8++ = p[instrPos++]; *pval8++ = p[instrPos++];
				elementId = val16;
			} else if (elementIdType == ELEMENT32) {
				pval8 = (uint8_t *)&elementId;
				memcpy(pval8, p + instrPos, 4); instrPos += 4;
			} else {
				elementId = NO_IDENTIFIER;
			}
			// store next dimension
			if (++depth >= stackSize) {
				key[depth] = elementId;
				offsets[stackSize] = 0;
				stackSize++;
			} else {
				if (elementId == NO_IDENTIFIER) {
					elementId = ++key[depth];
				} else {
					key[depth] = elementId;
				}
			}
			jump[depth] = jumpPosition + currentPos;
			ValueType = pINSTR[instruction].valueType;

			// compare element against set
			{
				bool found = false;
				bool endOfSet = true;
				if (setSingle[depth] == NO_IDENTIFIER) {
					if (setRange[depth] == setEndRange[depth]) {
						setRange[depth] = sets[depth]->rangeLowerBound(elementId);
					}
					while (setRange[depth] != setEndRange[depth]) {
						if (elementId < setRange[depth].low()) {
							// element not in selection
							endOfSet = false;
							break;
						} else if (elementId <= setRange[depth].high()) {
							// element in selection
							found = true;
							break;
						} else {
							++setRange[depth];
						}
					}
				} else {
					if (setSingle[depth] == elementId) {
						found = true;
					} else if (setSingle[depth] > elementId) {
						endOfSet = false;
					}
				}
				if (!found) {
					// invalid element
					if (endOfSet) {
						// no more possible values in this branch -> fallback
						// after the searched element at this level - skip to next parent
						if (depth > 0) { // terminate all open positions
							instrPos = jump[--depth] - currentPos;
						} else {
							endReached = true;
						}
						ValueType = 0; // ignore the value
					} else {
						if (jumpPosition) {
							// jumpPosition == 0 => get next item - stream can never jump back
							instrPos = jumpPosition;
						} else {
							// skip value
							if (ValueType == VALUEU8) {
								instrPos++;
							} else if (ValueType == VALUE32 || ValueType == VALUEF32) {
								instrPos += 4;
							} else if (ValueType == VALUE64) {
								instrPos += 8;
							}
						}
						ValueType = 0; // ignore the value
					}
				} else {
					// valid element -> continue
					AggregationMap::TargetReader targetReader = parentMaps[depth]->getTargets(elementId);
					if (!singleParent[depth]) {
						if (hasMultiMap[depth]) {
							if (targets[depth].size() > 1) {
								multiCount--;
								targets[depth] = AggregationMap::TargetReader();
							} else {
								cachedOffset -= offsets[depth];
							}
							if (targetReader.size() > 1) {
								// element has multiple parents in resultset
								targets[depth] = targetReader;
								multiDim[multiCount] = (uint8_t)depth;
								multiCount++;
								if (weightSlot[depth]) {
									weights[weightSlot[depth]-1] = 1;
								}
							} else {
								// single parent
								offsets[depth] = hashStorage->offsets[depth][*targetReader-hashStorage->firstIds[depth]];
								cachedOffset += offsets[depth];
								if (weightSlot[depth]) {
									weights[weightSlot[depth]-1] = targetReader.getWeight();
								}
							}
						} else {
							// single parent
							cachedOffset -= offsets[depth];
							offsets[depth] = hashStorage->offsets[depth][*targetReader-hashStorage->firstIds[depth]];
							cachedOffset += offsets[depth];
							if (weightSlot[depth]) {
								weights[weightSlot[depth]-1] = targetReader.getWeight();
							}
						}
					} else if (weightSlot[depth]) {
						weights[weightSlot[depth]-1] = targetReader.getWeight();
					}
				}
			}

			if (ValueType) {
				// read value
				if (ValueType == VALUEU8) {
					currentValue = p[instrPos++];
				} else if (ValueType == VALUE32) {
					pval8 = (uint8_t *)&val32;
					memcpy(pval8, p + instrPos, 4); instrPos += 4;
					currentValue = val32;
				} else if (ValueType == VALUEF32) {
					pval8 = (uint8_t *)&valfloat;
					memcpy(pval8, p + instrPos, 4); instrPos += 4;
					currentValue = valfloat;
				} else if (ValueType == VALUE64) {
					memcpy(&currentValue, p + instrPos, 8); instrPos += 8;
				}
				// aggregate the value
#ifdef PROFILE_AGGREGATION
				values++;
				sum += currentValue;
#endif

				if (multiCount) {
					// for each parent cell
					int changeMultiDim;
					for (int weight = 0; weight < weightsCount; weight++) {
						currentValue *= weights[weight];
					}
					do {
						int multiOffset = cachedOffset;
						double multiValue = currentValue;
						for (int dim = 0; dim < multiCount; dim++) {
							size_t currentDim = multiDim[dim];
							offsets[currentDim] = hashStorage->offsets[currentDim][*targets[currentDim]-hashStorage->firstIds[currentDim]];
							multiOffset += offsets[currentDim];
							multiValue *= targets[currentDim].getWeight();
						}
						hashStorage->resultSet[multiOffset] = 1;
						hashStorage->results[multiOffset] += multiValue;
//						cout << "M\t "<< multiValue << "\t" << hashStorage->results[multiOffset] << endl;

						changeMultiDim = multiCount - 1;
						bool endOfIteration = false;
						while (!endOfIteration && changeMultiDim >= 0 && changeMultiDim < multiCount) {
							size_t currentDim = multiDim[changeMultiDim];
							++targets[currentDim];
							if (targets[currentDim].end()) {
								targets[currentDim].reset();
								changeMultiDim--;
							} else {
								endOfIteration = true;
							}
						}
					} while (changeMultiDim >= 0 && changeMultiDim < multiCount);
				} else {
					for (int weight = 0; weight < weightsCount; weight++) {
						currentValue *= weights[weight];
					}
					hashStorage->resultSet[cachedOffset] = 1;
					hashStorage->results[cachedOffset] += currentValue;
//					{
//						stringstream ss;
//						IdentifiersType vkey(key, key+depth);
//						ss << currentValue << " " << hashStorage->results[cachedOffset] << " " << cachedOffset << " " << vkey << endl;
//						cout << ss.str();
//					}
//					cout << "C\t "<< currentValue << "\t" << hashStorage->results[cachedOffset] << endl;
				}
			} else if (mtCallback && !endReached) {
				mtCallback->readCallback();
			}
		}
		pos += instrPos;
	} while (!endReached);

#ifdef PROFILE_AGGREGATION
	ptime aggregationEnd(microsec_clock::local_time());
	size_t aggregationTime = (aggregationEnd-aggregationStart).total_microseconds();
	{
		stringstream ss;
		ss << "Sum: " << sum << " Values: " << values << " Time: " << aggregationTime/1000 << " ms " << nodesVisited << " " << pagesVisited << " " << hashStorage->resultSize << endl;
		cout << ss.str();
	}
#endif
}

bool StorageCpu::Processor::nextValid(size_t pos)
{
	const SlimVector<uint8_t> &svr = *pageList;
	bool cont = false;
	if (depth > 0 ? (pos < jump[depth - 1]) : (pos < endp)) {
		const uint8_t *p1 = &svr[pos];
		uint8_t instruction = *p1;
		uint32_t nInstr = 1;
		if (instruction & JUMP) {
			nInstr += 4;
		}
		uint32_t nelementId;
		uint8_t *pval8;
		uint16_t val16;

		int nelementIdType = pINSTR[instruction].elementIdType;
		if (nelementIdType == ELEMENT8) {
			nelementId = p1[nInstr];
		} else if (nelementIdType == ELEMENT16) {
			pval8 = (uint8_t *)&val16;
			*pval8++ = p1[nInstr]; *pval8++ = p1[nInstr + 1];
			nelementId = val16;
		} else if (nelementIdType == ELEMENT32) {
			pval8 = (uint8_t *)&nelementId;
			memcpy(pval8, p1 + nInstr, 4);
		} else {
			nelementId = key[depth] + 1;
		}
		Set::range_iterator it = setEndRange[depth];
		if ((--it).high() >= nelementId) {
			cont = true;
		}
	}
	return cont;
}

void StorageCpu::Processor::setValue(const CellValue &value)
{
//	if (!value.isNumeric() && !value.isEmpty()) {
//		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::NumericStorageProcessor::setValue called with invalid type!");
//	}
	storage.setCellValue(getKey(), value);
}

const CellValue &StorageCpu::Processor::getValue()
{
	storage.convertToCellValue(cellvalue, value);
	return cellvalue;
}

double StorageCpu::Processor::getDouble()
{
	return value;
}

const IdentifiersType &StorageCpu::Processor::getKey() const
{
	return vkey;
}

StorageCpu::Processor::Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, CPArea area, const size_t endPos, const Bookmark *bookmark) :
	ProcessorBase(true, PEngineBase()), area(area), pageList(pageList), pstorage(storage.shared_from_this()), storage(storage), mtCallback(0)//, indexptr(0)
{
	reset();
	endp = endPos ? endPos : pageList->size();
	if (bookmark) {
		stackSize = (int)bookmark->getKey().size();
		pos = bookmark->getPosition();
		currentPos = bookmark->getPosition();
		for (int dim = 0; dim < stackSize; dim++) {
			key[dim] = bookmark->getKey()[dim];
			size_t off = bookmark->getOffsets()[dim];
			const uint8_t *p = &(*pageList)[off];
			if (*p & JUMP) {
				size_t distance = storage.getJump(p + 1);
				jump[dim] = off + distance;
			} else {
				jump[dim] = pos;
			}
		}
		depth = stackSize-1;
		if (area) {
			size_t dim = 0;
			for (int newDepth = 0; newDepth < depth; newDepth++, dim++) {
				CPSet s = area->getDim(dim);
				if (s->find(key[newDepth]) == s->end()) {
					currentPos = pos = jump[newDepth];
					depth = newDepth-1;
					break;
				}
			}
		}
	}
}

StorageCpu::Processor::Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, PPathTranslator pathTranslator, const size_t endPos, const Bookmark *bookmark) :
	ProcessorBase(true, PEngineBase()), pageList(pageList), pstorage(storage.shared_from_this()), storage(storage), mtCallback(0)//, indexptr(0)
{
	reset();
	endp = endPos ? endPos : pageList->size();
	if (bookmark) {
		stackSize = (int)bookmark->getKey().size();
		pos = bookmark->getPosition();
		currentPos = bookmark->getPosition();
		for (int dim = 0; dim < stackSize; dim++) {
			key[dim] = bookmark->getKey()[dim];
			size_t off = bookmark->getOffsets()[dim];
			const uint8_t *p = &(*pageList)[off];
			if (*p & JUMP) {
				size_t distance = storage.getJump(p + 1);
				jump[dim] = off + distance;
			} else {
				jump[dim] = pos;
			}
		}
		depth = stackSize-1;
	}
}

StorageCpu::Processor::Processor(const Processor &other) :
	ProcessorBase(true, PEngineBase()), value(other.value), vkey(other.vkey), depth(other.depth), prevDepth(other.prevDepth), stackSize(other.stackSize),
	pFilter(other.pFilter ? filterKey : 0), area(other.area), pageList(other.pageList), pos(other.pos), currentPos(other.currentPos),
	endReached(other.endReached), endp(other.endp), before(other.before), pstorage(other.pstorage), storage(other.storage),
	sets(other.sets), setRange(other.setRange), setEndRange(other.setEndRange), mtCallback(0)//, indexptr(other.indexptr)
{
	memcpy(key, other.key, sizeof(key));
	memcpy(prevKey, other.prevKey, sizeof(prevKey));
	memcpy(jump, other.jump, sizeof(jump));
	memcpy(offset, other.offset, sizeof(offset));
	memcpy(prevOffset, other.prevOffset, sizeof(prevOffset));
	memcpy(filterKey, other.filterKey, sizeof(filterKey));
	memcpy(setSingle, other.setSingle, sizeof(setSingle));
}

void StorageCpu::Processor::setFilter(const uint32_t *filter, const int dimensions, bool changeDepth)
{
	if (filter) {
		memset(filterKey, 255, sizeof(filterKey));
		for (int dim = 0; dim < dimensions; dim++) {
			filterKey[dim] = filter[dim];
		}
		pFilter = filterKey;
		if (changeDepth) {
			for (int newDepth = 0; newDepth < depth; newDepth++) {
				if (filter[newDepth] != key[newDepth]) {
					currentPos = pos = jump[newDepth];
					depth = newDepth-1;
					break;
				}
			}
		}
	} else {
		pFilter = 0;
	}
}


void StorageCpu::Processor::reset()
{
	depth = -1;
	stackSize = 0;
	pFilter = 0;
	pos = 0;
	currentPos = 0;
	before = false;
	endReached = pageList->empty();
	memset(offset, 0, sizeof(offset));
	memset(key, 255, sizeof(key));
	offset[0] = -1;
	if (area) {
		if (setEndRange.size()) {
			setRange.clear();
			setRange.reserve(area->dimCount());
			for (size_t dim = 0; dim < area->dimCount(); dim++) {
				setRange.push_back(setEndRange[dim]);
			}
		} else {
			sets.reserve(area->dimCount());
			setRange.reserve(area->dimCount());
			setEndRange.reserve(area->dimCount());

			for (size_t dim = 0; dim < area->dimCount(); dim++) {
				CPSet s = area->getDim(dim);
				sets.push_back(s.get());
				setRange.push_back(s->rangeEnd());
				setEndRange.push_back(s->rangeEnd());
				if (s->size() == 1) {
					setSingle[dim] = *s->begin();
				} else {
					setSingle[dim] = NO_IDENTIFIER;
				}
			}
		}
	} else {
		memset(setSingle, 255, sizeof(setSingle));
	}
}

bool StorageCpu::Processor::move(const IdentifiersType &key, bool *found)
{
	if (found) {
		*found = false;
	}
	bool quickMode = true;
	if ((int)offset[0] == -1) {
		offset[0] = 0;
		quickMode = false;
	}
	if (quickMode) {
		int cmp = key.compare(vkey);
		if (cmp < 0) {
			return true;
		} else if (cmp == 0) {
			if (found) {
				*found = true;
			}
			return true;
		}
	}
	// set filter to the single cell
	setFilter(&key.at(0), (int)key.size(), false);
	// call next to get to the cell
	depth = -1;
	if (quickMode) {
		size_t i;
		for (i = 0; i < key.size(); i++) {
			if (key[i] != this->key[i]) {
				depth = (int)i - 1;
				for (size_t j = i; j < setRange.size(); j++) {
					if (setSingle[j] == NO_IDENTIFIER) {
						setRange[j] = setEndRange[j];
					}
				}
				break;
			}
		}
		pos = offset[depth + 1];
		this->key[depth + 1] -= 1;
	}
	if (!quickMode) {
		pos = offset[0];
		this->key[0] = prevKey[0];
	}
	bool result = next();
	if (result && found) {
		*found = true;
		const IdentifiersType &foundKey = getKey();
		for (size_t i = 0; i < key.size(); i++) {
			if (key[i] != NO_IDENTIFIER && key[i] != foundKey[i]) {
				*found = false;
				break;
			}
		}
	}
	return result;
}

bool StorageCpu::Processor::moveBefore(const IdentifiersType *key)
{
	before = true;
	depth = -1;
	if (key) {
		IdentifiersType prev = *key;
		for (IdentifiersType::reverse_iterator it = prev.rbegin(); it != prev.rend(); ++it) {
			if (*it) {
				(*it)--;
				break;
			} else {
				(*it) = (uint32_t)-1;
			}
		}
		setFilter(&(prev)[0], (int)prev.size(), false);
		if (pos) {
			for (size_t i = 0; i < key->size(); i++) {
				if (key->at(i) != this->key[i]) {
					depth = (int)i - 1;
					break;
				}
			}
			pos = offset[depth + 1];
			this->key[depth + 1] -= 1;
		} else {
			// start from beginning
		}
		memset(this->key + depth + 2, 255, (DIM_MAX - depth - 2) * sizeof(this->key[0]));
		memset(this->offset + depth + 2, 0, (DIM_MAX - depth - 2) * sizeof(this->offset[0]));
	} else {
		setFilter(0, 0, false);
		pos = offset[0];
		this->key[0] -= 1;
	}
	bool result = next();
	before = false;
	setFilter(0, 0, false);
	return result;
}

StorageCpu::Processor::~Processor()
{
}

StorageCpu::Bookmark StorageCpu::Processor::getBookmark() const
{
	Bookmark bookmark;
	bookmark.setPosition(currentPos, 0);
	IdentifiersType &bookmarkKey = bookmark.getKey();

	int maxDim = min(depth+2, stackSize);
	vector<size_t> &offsets = bookmark.getOffsets();

	offsets.reserve(maxDim);
	bookmarkKey.reserve(maxDim);
	for (int dim = 0; dim < maxDim; dim++) {
		bookmarkKey.push_back(key[dim]);
		offsets.push_back(offset[dim]);
	}
	return bookmark;
}

void StorageCpu::Processor::indexStorage()
{
//	indexptr = &index;
	buildIndex();
}

vector<StorageCpu::ElemRestriction> StorageCpu::Processor::getStack(bool previous)
{
	vector<ElemRestriction> result;
	uint32_t *keyBase = previous ? prevKey : key;
	size_t *offsetBase = previous ? prevOffset : offset;

	for (int d = 0; d < stackSize; d++) {
		result.push_back(ElemRestriction(keyBase[d], offsetBase[d]));
	}
	return result;
}

void StorageCpu::Writer::storeInstruction(uint32_t elementId, vector<ElemRestriction> &endStack, int &depth, vector<uint8_t> &buffer, bool doCompress, const double *value)
{
	depth++;
	size_t instructionPosition = buffer.size();
	bool storeElementId = true;
	if (endStack.size() < (size_t)depth + 1) {
		endStack.push_back(ElemRestriction(elementId));
	} else {
		// replacing existing
		if (endStack[depth].elementId != NO_IDENTIFIER && endStack[depth].elementId + 1 == elementId) {
			storeElementId = false;
		}
		endStack[depth] = ElemRestriction(elementId);
	}
	buffer.push_back(0);
	// value -> no jump offset
	if (!value) {
		buffer[instructionPosition] |= JUMP;
		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back(0);
	}
	endStack[depth].iOffset = instructionPosition;
	endStack[depth].pOffset = NULL;

	uint8_t valu8;
	uint16_t valu16;
	int32_t val32;
	float valfloat;
	uint8_t *pval8;

	// if dimension has not changed - do not store it
	// otherwise ...
	if (storeElementId) {
		if (canCompress(elementId, valu8)) {
			buffer[instructionPosition] |= ELEMENT8;
			buffer.push_back(valu8);
		} else if (canCompress(elementId, valu16)) {
			buffer[instructionPosition] |= ELEMENT16;
			pval8 = (uint8_t *)&valu16;
			buffer.push_back(pval8[0]);
			buffer.push_back(pval8[1]);
		} else  {
			buffer[instructionPosition] |= ELEMENT32;
			pval8 = (uint8_t *)&elementId;
			buffer.push_back(pval8[0]);
			buffer.push_back(pval8[1]);
			buffer.push_back(pval8[2]);
			buffer.push_back(pval8[3]);
		}
	}

	if (value) {
		if (doCompress && canCompress(*value, valu8)) {
			buffer[instructionPosition] |= VALUEU8;
			buffer.push_back(valu8);
		} else if (doCompress && canCompress(*value, val32)) {
			buffer[instructionPosition] |= VALUE32;
			pval8 = (uint8_t *)&val32;
			buffer.push_back(pval8[0]);
			buffer.push_back(pval8[1]);
			buffer.push_back(pval8[2]);
			buffer.push_back(pval8[3]);
		} else if (doCompress && canCompress(*value, valfloat)) {
			buffer[instructionPosition] |= VALUEF32;
			pval8 = (uint8_t *)&valfloat;
			buffer.push_back(pval8[0]);
			buffer.push_back(pval8[1]);
			buffer.push_back(pval8[2]);
			buffer.push_back(pval8[3]);
		} else  {
			buffer[instructionPosition] |= VALUE64;
			pval8 = (uint8_t *)value;
			size_t dblPos = buffer.size();
			buffer.resize(dblPos+8);
			memcpy(&buffer[dblPos], pval8, 8);
		}
	}
}

void StorageCpu::Writer::setJump(uint8_t *p, size_t jumpPosition, size_t jumpValue, const vector<size_t> &sourceLongJumps)
{
	uint32_t shortDistance;
	if (!p) {
		SlimVector<uint8_t> &sv = *ds.pageList.get();
		p = &sv[jumpPosition];
	}
	if (jumpValue >= LONG_JUMP_START_VALUE) {
		memcpy((uint8_t *)&shortDistance, p, 4);
		if (shortDistance >= LONG_JUMP_START_VALUE) {
			// update longJump
			if (&sourceLongJumps == &ds.longJumps) {
//				Logger::debug << "StorageCpu updating longJump at " << jumpPosition << " " << ds.longJumps[shortDistance - LONG_JUMP_START_VALUE] << " -> " << jumpValue << endl;
				ds.longJumps[shortDistance - LONG_JUMP_START_VALUE] = jumpValue;
			} else {
//				Logger::error << "LongJump update source and target jump table differs!" << endl;
				shortDistance = (uint32_t)(ds.longJumps.size()+LONG_JUMP_START_VALUE);
				memcpy(p, (uint8_t *)&shortDistance, 4);
				ds.longJumps.push_back(jumpValue);
			}
		} else {
			// create new longJump
			shortDistance = (uint32_t)(ds.longJumps.size()+LONG_JUMP_START_VALUE);
			memcpy(p, (uint8_t *)&shortDistance, 4);
			ds.longJumps.push_back(jumpValue);
			Logger::debug << "New longJump created at " << jumpPosition << " distance: " << jumpValue << " index:  " << shortDistance-LONG_JUMP_START_VALUE << endl;
		}
	} else {
		// short -> short
		memcpy((uint8_t *)&shortDistance, p, 4);
		if (shortDistance >= LONG_JUMP_START_VALUE) {
			uint64_t oldLongJumpDistance = uint64_t(-1);
			if (shortDistance - LONG_JUMP_START_VALUE < sourceLongJumps.size()) {
				oldLongJumpDistance = sourceLongJumps[shortDistance - LONG_JUMP_START_VALUE];
			}
			// long -> short
			Logger::debug << "StorageCpu converting longJump at " << jumpPosition << " to shortJump " << oldLongJumpDistance << " -> " << jumpValue << endl;
		}
		shortDistance = (uint32_t)jumpValue;
		memcpy(p, (uint8_t *)&shortDistance, 4);
	}
}

void StorageCpu::Writer::push_back(const uint32_t *key, int dimensions, double value)
{
	SlimVector<uint8_t> &sv = *ds.pageList.get();
	depth = -1;
	vector<ElemRestriction>::iterator si = ds.endStack.begin();
	int testDepth = 0;
	int endStackSize = (int)ds.endStack.size();
	for (testDepth = 0; testDepth < endStackSize ; ++si, testDepth++) {
		if (key[testDepth] < si->elementId) {
			// error
			throw ErrorException(ErrorException::ERROR_INTERNAL, "Storage: push_back - invalid key");
		} else if (key[testDepth] == si->elementId) {
			// dimension OK
			depth++;
		} else {
			break;
		}
	}
	if (depth + 1 == dimensions) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Storage: push_back - overwriting last value");
	}

	// reset dimensions from depth+1 to end
	if (si != ds.endStack.end()) {
		++si;
	}
	for (; si != ds.endStack.end(); ++si) {
		si->elementId = NO_IDENTIFIER;
	}

	vector<uint8_t> buffer;
	buffer.reserve(32);
	size_t maxBufferSize = dimensions * 9 + 4;
	size_t oldOffsets[MAX_DIMENSIONS+1];
	size_t *oldOffset = oldOffsets;

	size_t position = 0;
	Bookmark bookmark;
	size_t sizeBefore = sv.size();
	uint32_t posInPage = sizeBefore % STORAGE_PAGE_SIZE;
	if (ds.indexEnabled && sv.pageCount() && (((STORAGE_PAGE_SIZE - posInPage) < maxBufferSize/*64*/) || !posInPage)) {	// close to the end of page or beginning of the new page - remember bookmark
		position = ((sizeBefore / STORAGE_PAGE_SIZE)+1)*STORAGE_PAGE_SIZE; // set position to the new page
		if (!(sizeBefore % STORAGE_PAGE_SIZE)) {
			position -= STORAGE_PAGE_SIZE;
		}
		if (addBookmark) {
			bookmark = Bookmark(ds);
			bookmark.setPosition(position, 0);
		}
	}

	for (uint8_t dimension = testDepth; dimension < dimensions; dimension++) {
		if (ds.endStack.size() > (uint32_t)(depth + 1)) {
			*oldOffset++ = ds.endStack[depth + 1].iOffset;
		}
		// store single element and update jump offsets of previous key
		storeInstruction(key[dimension], ds.endStack, depth, buffer, ds.isNumeric(), dimension == dimensions - 1 ? &value : 0);
		ds.endStack[depth].iOffset += sv.size(); // relative offset was saved in storeInstruction
		ds.endStack[depth].pOffset = NULL;
	}
	if (!buffer.empty()) {
		int dep = depth;
		bool pageAdded = false;
		bool pageCheckout = false;
		uint32_t nopsAdded = sv.push_back(buffer, NOP, pageAdded, pageCheckout);
		if (pageCheckout) {
			for (si = ds.endStack.begin(); si != ds.endStack.end(); ++si) {
				si->pOffset = NULL;
			}
		}
//		size_t newPos = 0;
		if (pageAdded && position) {
			if (addBookmark) {
				// new page -> save bookmark
//				ds.index->insert(bookmark);
				if (ds.index2) {
					ds.index2->push_back(bookmark);
				}
			}
//			newPos = position;
		} else {
//			newPos = sizeBefore;
		}
		if (nopsAdded) {
			ds.emptySpace += nopsAdded;
			for (uint8_t dimension = testDepth; dimension < dimensions; dimension++) {
				ds.endStack[dep].iOffset += nopsAdded;
				ds.endStack[dep].pOffset = NULL; // probably not necessary
				dep--;
			}
			for (size_t *currentOffset = oldOffsets; currentOffset < oldOffset; currentOffset++) {
				uint8_t *p = &sv[*currentOffset];
				if (*p & JUMP) {
					setJump(p+1, *currentOffset+1, ds.getJump(p+1)+nopsAdded, ds.longJumps);
				}
			}
		}
	}
	ds.valCount++;

	size_t currentSize = sv.size();
	// terminate open elements and return
	for (si = ds.endStack.begin(); si != ds.endStack.end(); ++si) {
		if (si->iOffset != NO_OFFSET) {
			if (si->pOffset == NULL) {
				si->pOffset = &sv[si->iOffset];
			}
			if (*si->pOffset & JUMP) {
				setJump(si->pOffset+1, si->iOffset+1, currentSize - si->iOffset, ds.longJumps);
			}
		}
	}
}

void StorageCpu::Writer::push_back(const vector<uint32_t> &key, double value)
{
	push_back(&key[0], (int)key.size(), value);
}

bool StorageCpu::Writer::writeUntil(Processor *p, const IdentifiersType *key, bool rebuild)
{
	if (rebuild) {
		return writeUntilRebuild(p, key);
	} else {
		return writeUntilCopy(p, key);
	}
}

bool StorageCpu::Writer::writeUntilCopy(Processor *p, const IdentifiersType *key)
{
	if (key) {
		if (CellValueStream::compare(*key, p->getKey()) <= 0) {
			return true;
		} else {
			return movePages(p, key);
		}
	} else {
		return movePages(p, 0);
	}
}

void StorageCpu::Writer::translateBookmark(StorageCpu::Processor *p, size_t spos, uint32_t iPage, uint32_t spagepos)
{
	if (p->storage.index2->size() < iPage-1) {
		Logger::error << "index2 is too small. " << p->storage.index2->size() << " req page: " << iPage-1 << endl;
		ds.index2.reset();
	}
	if (ds.index2) {
		ds.index2->push_back(p->storage.index2->at(iPage-1));
		Bookmark &bm = ds.index2->back();
		int64_t shift = int64_t(iPage)*p->storage.pageList->pageSize(iPage-1)-ds.pageList->size();
		bm.setPosition(size_t(bm.getPosition()-shift), size_t(ds.index2->size()));
		vector<size_t> &offsets = bm.getOffsets();
		IdentifiersType::const_iterator eit = bm.getKey().begin();
		vector<ElemRestriction>::const_iterator esit = ds.endStack.begin();
		vector<size_t>::iterator oit = offsets.begin();
		for (; oit != offsets.end(); ++oit, ++esit, ++eit) {
			if (*oit > spos || esit->elementId != *eit) {
				break;
			}
			*oit = esit->iOffset;
		}
		for (;oit != offsets.end(); ++oit) {
			*oit = size_t(*oit-shift);
		}
//		ds.index->insert(bm);
//		Bookmark bm = p->storage.index2->at(iPage-1);
//		int64_t shift = iPage*p->storage.pageList->pageSize(iPage-1)-ds.pageList->size();
//		bm.setPosition(size_t(bm.getPosition()-shift));
//		vector<size_t> &offsets = bm.getOffsets();
//		IdentifiersType::const_iterator eit = bm.getKey().begin();
//		vector<ElemRestriction>::const_iterator esit = ds.endStack.begin();
//		vector<size_t>::iterator oit = offsets.begin();
//		for (; oit != offsets.end(); ++oit, ++esit, ++eit) {
//			if (*oit > spos || esit->elementId != *eit) {
//				break;
//			}
//			*oit = esit->iOffset;
//		}
//		for (;oit != offsets.end(); ++oit) {
//			*oit = size_t(*oit-shift);
//		}
////		ds.index->insert(bm);
//		ds.index2->push_back(bm);
	}
}

void StorageCpu::Writer::copyLongJunps(vector<size_t> &nextPositions, Processor *proc)
{
	if (ds.endStack.size() < 2) {
		// one dimensional cube, no jumps
		return;
	}
	const SlimVector<uint8_t> &sv = *ds.pageList;

	int depth;
	size_t pos = nextPositions[0];
	vector<size_t> limit(nextPositions.size(), sv.size());
	std::copy(nextPositions.begin(), nextPositions.end()-1, limit.begin()+1);

	for (depth = (int)nextPositions.size()-1; depth > 0; depth--) {
		pos = nextPositions[depth];
		if (nextPositions[depth-1]-nextPositions[depth] >= LONG_JUMP_START_VALUE) {
			break;
		}
	}

	while (depth >= 0) {
		// while the limit is not reached
		if (limit[depth] - pos < LONG_JUMP_START_VALUE) {
			depth--;
			continue;
		}
		const uint8_t *p = &sv[pos];
		uint8_t instruction = *p++;
		if (instruction == NOP) {
			// TODO: -jj- optimize, BTW this code shouldn't be executed
			pos++;
			continue;
		}
		if (instruction & JUMP) {
			uint32_t distance;
			memcpy((uint8_t *)&distance, p, 4);
			p += 4;
			size_t longDistance = distance;
			if (distance >= LONG_JUMP_START_VALUE) {
				longDistance = proc->storage.longJumps[distance - LONG_JUMP_START_VALUE];
			}
			// if current jump is short
			if (longDistance < LONG_JUMP_START_VALUE) {
				// skip jump
				pos += longDistance;
				continue;
			} else {
				// copy jump
				// checkout the page
//				Logger::debug << "Copying long jump " << longDistance << " at position " << pos << endl;
				setJump(0, pos+1, longDistance, proc->storage.longJumps);

				// go deeper if not last jump dimension
				if (depth >= (int)nextPositions.size()-1) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Writer::copyLongJunps depth out of range, limit position test failed");
				}
				depth++;
				// set new inner limit
				limit[depth] = longDistance+pos;
				// skip value
				pos += pINSTR[instruction].instructionSize;
//				throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Writer::copyLongJunps not yet implemented");
			}
		} else {
			// skip value
			// should never get here
			throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Writer::copyLongJunps no Jump");
		}
	}
}

bool StorageCpu::Writer::movePages(Processor *p, const IdentifiersType *key)
{
	size_t spos = p->getPosition();
	vector<ElemRestriction> sstack = p->getStack(false);
	IdentifiersType firstkey = p->getKey();
	double val = p->getValue().getNumeric();

//	if (23345569 == spos && commitCounter == 68) {
//		int breakhere = 1;
//	}
	Processor p2(*p);
	p->moveBefore(key);

	vector<ElemRestriction> estack = p->getStack(false);
	size_t epos = p->getPosition();

	uint32_t pageSize = p->pageList->maxPageSize();
	uint32_t startShift = 0;
	uint32_t startNOPs = 0;
	// skip all NOPs at the beginning
	const SlimVector<uint8_t> &svr = *p->pageList;

	while (spos < epos && svr[spos] == NOP) {
		spos++;
		startNOPs++;
//		startShift++;
	}

	const vector<Slim<uint8_t>::PSlimPage> &procPages = p->pageList->getPages();
	uint32_t spage = (uint32_t)(spos / pageSize);
	uint32_t spagepos = spos % pageSize;
	uint32_t epage = (uint32_t)(epos / pageSize);
	uint32_t epagepos = epos % pageSize;

	bool rebuildBlock = (epos - spos <= pageSize /*+ (pageSize - cpagepos)*/);
	if (!rebuildBlock) {
		size_t nopsInBlock = 0;
		for (uint32_t iPage = spage; iPage < epage; iPage++) {
			if (epagepos) {
				nopsInBlock += procPages[iPage]->getEmptyCount();
			}
		}
		if (nopsInBlock > 100) {
			int nopsPerc = (int)(100*nopsInBlock/(epos - spos));
//			Logger::debug << commitCounter << " nops " << nopsInBlock << " in " << epos - spos << " " << nopsPerc << " %" << endl;
			if (nopsPerc > 40) {
				rebuildBlock = true;
			}
		}
	}

	// moved block smaller than page size + remaining part of page - do push back - it wouldn't save any memory
	if (/*true || */rebuildBlock) {
		if (key) {
			p->next();
		}
		return writeUntilRebuild(&p2, key);
	}

	size_t movedSize = epos - spos;
//	if (movedSize > LONG_JUMP_START_VALUE) {
//		Logger::debug << "moving " << movedSize << " Bytes" << endl;
//	}

	push_back(firstkey, val);

	size_t currSize = ds.pageList->size();
	uint32_t cpagepos = currSize % pageSize;

	if (epos > spos) {
		if (!cpagepos) {
			ds.pageList->addPage();
			if (ds.index2) {
				StorageCpu::Bookmark bookmark(ds);
				ds.index2->push_back(bookmark);
			}
		}
		vector<Slim<uint8_t>::PSlimPage> &pages = ds.pageList->getPages();
		Slim<uint8_t>::PSlimPage pagelast = pages[pages.size() - 1];

		Slim<uint8_t>::PSlimPage pagecopy;
		uint32_t nops = 0;

		bool copyBookMark = ds.index2 && p->storage.index2 && (!ds.index2->empty() || key || spos != currSize);
//		bool copyBookMark = ds.index2 && p->storage.index2;

		//////////////////////////////
		// copy start page content
		//////////////////////////////movePages
		size_t startCount = procPages[spage]->count()-spagepos;  // should be always until end of the page
		if (cpagepos <= spagepos) {
			// change fits into current page
			if (spagepos-cpagepos > 0) {
				pagelast->fill(NOP, spagepos-cpagepos);
				nops = spagepos-cpagepos;
				startShift += nops;
			}
			const uint8_t *psrc = &svr[spos];
			uint8_t *pdst = &(*pagelast)[spagepos];
			memcpy(pdst, psrc, startCount);
			nops += countNops(psrc, startCount);
			pagelast->setSize(pageSize);
			pagelast->setEmptyCount(pagelast->getEmptyCount()+nops);
			ds.emptySpace += nops;
		} else {
			// finish current page
			nops = pagelast->endSpace();
			startShift += nops;
			pagelast->fill(NOP);
			pagelast->setEmptyCount(pagelast->getEmptyCount()+nops);
			ds.emptySpace += nops;

			// copy and modify next
			nops = 0;
			pagecopy = procPages[spage]->getCopy();
			pagecopy->setValue(0, spagepos, NOP);
			nops = spagepos;
			startShift += nops;
			nops += countNops(&svr[spos], startCount);
			pages.push_back(pagecopy);
			pagecopy->setEmptyCount(nops);
			ds.emptySpace += nops;
			if (copyBookMark && ds.index2) {
				Bookmark bm(ds);
				bm.setPosition(bm.getPosition()-pagecopy->maxCount() + spagepos, size_t(ds.index2->size()+1));
//				ds.index->insert(bm);
				ds.index2->push_back(bm);
			}
		}

		//////////////////////////////
		// reuse mid pages
		//////////////////////////////
		for (uint32_t iPage = spage+1; iPage < epage; iPage++) {
			ds.emptySpace += procPages[iPage]->getEmptyCount();;
			procPages[iPage]->checkIn();
			if (copyBookMark) {
				translateBookmark(p, spos, iPage, spagepos);
			}
			pages.push_back(procPages[iPage]);
		}

		//////////////////////////////
		// copy end page content
		//////////////////////////////
		// if last page is not empty
		if (epagepos) {
			nops = 0;
			pagecopy = procPages[epage]->getCopy();
			pagecopy->setSize(epagepos);
			nops += countNops(&svr[epage*pageSize], epagepos);
			pagecopy->setEmptyCount(nops);
			if (copyBookMark) {
				translateBookmark(p, spos, epage, spagepos);
			}
			pages.push_back(pagecopy);
			ds.emptySpace += nops;
		}

		if (ds.index2 && p->storage.index2 && ds.index2->empty() && !key && spos == currSize) {
			if (spage == 0 && p->storage.index2->at(p->storage.index2->size() - 1).getPosition() < epos) {
				ds.index2 = p->storage.index2;
			} else {
				if (epage > spage) {
					ds.index2->reserve(epage - spage);
				}
				for (vector<Bookmark>::const_iterator iti2 = p->storage.index2->begin(); iti2 != p->storage.index2->end(); ++iti2) {
					if (iti2->getPosition() >= spos && iti2->getPosition() < epos) {
						ds.index2->push_back(*iti2);
					}
				}
			}
		}

		//////////////////////////////
		// update jumps before and after the copied block
		//////////////////////////////
		const SlimVector<uint8_t> &sv = *ds.pageList.get();
		vector<size_t> nextPositions(ds.endStack.size()-1, ds.pageList->size());

		for (size_t i = 0; i < ds.endStack.size(); i++) {
			if (sv[ds.endStack[i].iOffset] & JUMP) {
				size_t distance = p->storage.getJump(&svr[sstack[i].iOffset + 1]);

				if (sstack[i].iOffset + distance >= epos) {
					distance = sv.size() - ds.endStack[i].iOffset;
				} else {
					distance = (sstack[i].iOffset + distance - spos) + currSize + startShift - ds.endStack[i].iOffset;
				}
				setJump(0, ds.endStack[i].iOffset + 1, distance, ds.longJumps);
				nextPositions[i] = ds.endStack[i].iOffset+distance;
			}
		}

		if (movedSize > LONG_JUMP_START_VALUE) {
			copyLongJunps(nextPositions, p);
		}

		for (size_t i = 0; i < ds.endStack.size(); i++) {
			if (estack[i].iOffset >= spos) {
				ds.endStack[i].iOffset = estack[i].iOffset - spos + currSize + startShift;
				ds.endStack[i].pOffset = NULL;
				ds.endStack[i].elementId = estack[i].elementId;
				if (sv[ds.endStack[i].iOffset] & JUMP) {
					size_t distance = (sv.size() - ds.endStack[i].iOffset);
					setJump(0, ds.endStack[i].iOffset+1, distance, p->storage.longJumps);
				}
			}
		}
		// validate endstack
		{
			vector<StorageCpu::ElemRestriction>::iterator esit1 = ds.endStack.begin();
			size_t s1Size = ds.pageList->size();

			for(; esit1 != ds.endStack.end(); ++esit1) {
				const uint8_t *p = &((*ds.pageList)[esit1->iOffset]);
				if (*p & JUMP) {
					size_t distance1 = ds.getJump(p+1);
					if (distance1 + esit1->iOffset != s1Size) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Writer::movePages invalid endstack!");
					}
				} else {
					// only last dim has no jump
					if (esit1+1 != ds.endStack.end()) {
						throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Writer::movePages invalid endstack (JUMP)!");
					}
				}
			}
		}
	}

	for (size_t i = 0; i < ds.endStack.size(); i++) {
		ds.endStack[i].pOffset = NULL;
	}
	if (key) {
		return p->next();
	} else {
		return false;
	}
}

#ifdef undefined
void StorageCpu::Writer::movePages(Processor *p, const IdentifiersType *key)
{
	size_t spos = p->getPosition();
	vector<ElemRestriction> sstack = p->getStack(false);
	IdentifiersType firstkey = p->getKey();
	double val = p->getValue().getNumeric();

//#ifdef TEST_MOVE_BEFORE
//	Processor pcopyBeore(*p);
//	bool failure = false;
//	if (key && CellValueStream::compare(p->getKey(), *key) > 0) {
//		Logger::error << "move pages, reader behind key!" << endl;
//		failure = true;
//	}
//	ptime moveStart = microsec_clock::local_time();
//#endif

	p->moveBefore(key);

//#ifdef TEST_MOVE_BEFORE
//	ptime moveEnd(microsec_clock::local_time());
//	size_t moveTime = (moveEnd-moveStart).total_microseconds();
//	if (moveTime > 100) {
//		Logger::info << "moveBefore took " << moveTime << " us" << endl;
////		p->reset();
////		bool found;
////		if (p->move(firstkey, &found) && found) {
////			p->moveBefore(key);
////		} else {
////			Logger::info << "reset and move failed!" << endl;
////		}
//	}
//
//	if (key && CellValueStream::compare(p->getKey(), *key) > 0) {
//		Logger::error << "after move pages, reader behind key!" << endl;
//		failure = true;
//	}
//	Processor pcopy(*p);
//	if (pcopy.next()) {
//		if (!key) {
//			Logger::error << "after move pages with no key, next returned true!" << endl;
//			failure = true;
//		} else {
//			if (CellValueStream::compare(pcopy.getKey(), *key) < 0) {
//				Logger::error << "after move pages and next, reader is not behind key!" << endl;
//				failure = true;
//			}
//		}
//	}
//	for(;failure;) {
//		Processor pcopytst(pcopyBeore);
//		IdentifiersType tstKeyBefore = pcopyBeore.getKey();
//		pcopytst.moveBefore(key);
//		IdentifiersType tstKeyAfter = pcopytst.getKey();
//		Logger::error << tstKeyBefore << " " << (key ? *key : IdentifiersType() )<< " " << tstKeyAfter << endl;
//	}
//#endif

	vector<ElemRestriction> estack = p->getStack(false);
	size_t epos = p->getPosition();

	if (ds.ccInfo.step == 1) {
		ds.ccInfo.newValueOldPos = p->getPositionJumpNOP();
	} else if (ds.ccInfo.step == 3) {
		ds.ccInfo.oldLastPos = epos;
	}

	push_back(firstkey, val);

	uint32_t pageSize = p->pageList->maxPageSize();
	if (epos > spos) {
		size_t currSize = ds.pageList->size();

		uint32_t cpagepos = currSize % pageSize;
		uint32_t spage = (uint32_t)(spos / pageSize);
		uint32_t spagepos = spos % pageSize;
		uint32_t epage = (uint32_t)(epos / pageSize);
		uint32_t epagepos = epos % pageSize;

		if (epos - spos <= pageSize) {
			Logger::debug << "moved block smaller than page size" << endl;
		}
		if (cpagepos < spagepos) {
			Logger::debug << "start page fits into current page" << endl;
		}

		vector<Slim<uint8_t>::PSlimPage> &pages = ds.pageList->getPages();
		const vector<Slim<uint8_t>::PSlimPage> &procPages = p->pageList->getPages();
		uint32_t nopcount = 0;
		Slim<uint8_t>::PSlimPage pagelast = pages[pages.size() - 1];
		nopcount = pagelast->endSpace();
		pagelast->fill(NOP);
		if (nopcount) {
			ds.emptySpace += nopcount;
			unordered_map<uint32_t, uint32_t>::iterator it = ds.nopPages->find((uint32_t)pages.size() - 1);
			if (it == ds.nopPages->end()) {
				(*ds.nopPages)[(uint32_t)pages.size() - 1] = nopcount;
			} else {
				it->second += nopcount;
			}
		}

		bool startRemoved = false;
		Slim<uint8_t>::PSlimPage pagecopy;
		for (uint32_t i = spage; i <= epage; i++) {
			// if first or last page of moved block
			if (i == spage || i == epage) {
				// if last page is empty - break
				if (i == epage && !epagepos) {
					break;
				}
				pagecopy = procPages[i]->getCopy();
				if (i == spage) {
					pagecopy->setValue(0, spagepos, NOP);
				}
				if (i == epage) {
					pagecopy->setSize(epagepos);
				}
				uint32_t count = pagecopy->count();
				uint32_t nops = 0;
				for (uint32_t ni = 0; ni < count; ++ni) {
					if ((*pagecopy)[ni] == NOP) {
						++nops;
					}
				}
				if (nops != count) {
					if (nops) {
						ds.emptySpace += nops;
						(*ds.nopPages)[(uint32_t)pages.size()] = nops;
					}
					pages.push_back(pagecopy);
				} else {
					startRemoved = (startRemoved || i == spage);
				}
				if (ds.ccInfo.step == 1) {
					if (spage == epage) {
						if (!startRemoved) {
							ds.ccInfo.build[0] = true;
							ds.ccInfo.build[1] = true;
							ds.ccInfo.diff1 = 1;
						}
					} else {
						if (i == spage) {
							if (startRemoved) {
								ds.ccInfo.newToOld[0] = 0;
							} else {
								ds.ccInfo.build[0] = true;
								ds.ccInfo.newToOld[1] = 0;
								ds.ccInfo.diff1 = 1;
							}
						} else if (i == epage) {
							ds.ccInfo.build[pages.size() - 1] = true;
						}
					}
				} else if (ds.ccInfo.step == 3) {
					if (i == spage) {
						ds.ccInfo.diff2 = (int)(pages.size() - 1 - ds.ccInfo.newepage) - (spage - ds.ccInfo.oldepage);
						if (!startRemoved) {
							ds.ccInfo.newToOld[pages.size() - 1] = i;
						}
					}
				}
			} else {
				unordered_map<uint32_t, uint32_t>::iterator it = ds.oldnopPages->find(i);
				if (it != ds.oldnopPages->end()) {
					ds.emptySpace += it->second;
					(*ds.nopPages)[(uint32_t)pages.size()] = it->second;
				}
				procPages[i]->checkIn();
				pages.push_back(procPages[i]);

				if (ds.ccInfo.step == 1 || ds.ccInfo.step == 3) {
					ds.ccInfo.newToOld[pages.size() - 1] = i;
				}
			}
		}

		if (startRemoved && spage == epage) {
			pagelast->setSize(pageSize - nopcount);
			ds.emptySpace -= nopcount;
			(*ds.nopPages)[(uint32_t)pages.size() - 1] = 0;
		} else {
			SlimVector<uint8_t> &sv = *ds.pageList.get();
			const SlimVector<uint8_t> &svr = *p->pageList;
			for (size_t i = 0; i < ds.endStack.size(); i++) {
				if (sv[ds.endStack[i].iOffset] & JUMP) {
					size_t distance = p->storage.getJump(&svr[sstack[i].iOffset + 1]);

					if (sstack[i].iOffset + distance >= epos) {
						distance = sv.size() - ds.endStack[i].iOffset;
					} else {
						distance = (sstack[i].iOffset + distance - spos) + currSize + spagepos + nopcount - ds.endStack[i].iOffset;
						if (startRemoved) {
							distance -= pageSize;
						}
					}
					setJump(0, ds.endStack[i].iOffset + 1, distance);
//					memcpy((uint8_t *)&distance, &svr[sstack[i].iOffset + 1], 4);
//					if (sstack[i].iOffset + distance >= epos) {
//						distance = (uint32_t)(sv.size() - ds.endStack[i].iOffset);
//					} else {
//						distance = (uint32_t)((sstack[i].iOffset + distance - spos) + currSize + spagepos + nopcount - ds.endStack[i].iOffset);
//						if (startRemoved) {
//							distance -= pageSize;
//						}
//					}
//					memcpy(&sv[ds.endStack[i].iOffset + 1], (uint8_t *)&distance, 4);
				}
				if (estack[i].iOffset >= spos) {
					ds.endStack[i].iOffset = estack[i].iOffset - spos + currSize + nopcount + spagepos;
					ds.endStack[i].pOffset = NULL;
					if (startRemoved) {
						ds.endStack[i].iOffset -= pageSize;
					}
					//ds.endStack[i].pOffset = &sv[ds.endStack[i].iOffset];
					ds.endStack[i].elementId = estack[i].elementId;
					if (sv[ds.endStack[i].iOffset] & JUMP) {
						size_t distance = (sv.size() - ds.endStack[i].iOffset);
						setJump(0, ds.endStack[i].iOffset+1, distance);
						//memcpy(&sv[ds.endStack[i].iOffset + 1], (uint8_t *)&distance, 4);
					}
				}
			}
		}
	}
	if (ds.ccInfo.step == 1) {
		ds.ccInfo.oldepage = (uint32_t)(epos / pageSize);
		if (!(epos % pageSize)) {
			ds.ccInfo.oldepage--;
		}
		ds.ccInfo.newepage = (uint32_t)ds.pageList->getPages().size() - 1;
	}

	for (size_t i = 0; i < ds.endStack.size(); i++) {
		ds.endStack[i].pOffset = NULL;
	}
}
#endif

bool StorageCpu::Writer::writeUntilRebuild(Processor *p, const IdentifiersType *key)
{
	bool ret = true;
	if (key) {
		while (CellValueStream::compare(*key, p->getKey()) > 0) {
			push_back(p->getKey(), p->getDouble());
			ret = p->next();
			if (!ret) {
				break;
			}
		}
	} else {
		ret = false;
		do {
			push_back(p->getKey(), p->getDouble());
		} while (p->next());
	}
	return ret;
}

StringStorageCpu::StringStorageCpu(PPathTranslator pathTranslator) :
	StorageCpu(pathTranslator, true), strings(new StringVector()), pushed(0), cleared(0), onlyDouble(false), stringMap(0)
{
	strings->push("");
}

StringStorageCpu::StringStorageCpu(const StringStorageCpu &st) : StorageCpu(st), strings(st.strings), pushed(st.pushed), cleared(st.cleared), onlyDouble(st.onlyDouble), stringMap(0)
{
}

StringStorageCpu::~StringStorageCpu()
{
	destroyStringMap();
}

CellValue StringStorageCpu::getCellValue(const IdentifiersType &key)
{
	Processor reader(*this, pageList.get(), PPathTranslator()); // translator not needed in this processor
	reader.setFilter(&key[0], (int)key.size(), true);
	if (reader.next() && key==reader.getKey()) {
		double d = reader.getDouble();
		CellValue value;
		convertToCellValue(value, d);
		return value;
	} else {
		return CellValue::NullString;
	}
}

bool StringStorageCpu::setCellValue(PCellStream stream)
{
	createStringMap();
	bool result = StorageCpu::setCellValue(stream);
	destroyStringMap();
	return result;
}

void StringStorageCpu::setCellValue(CPArea area, const CellValue &value, OperationType opType)
{
	if (value.isNumeric() || opType == MULTIPLY_EXISTING || opType == ADD_ALL) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StringStorageCpu::setCellValue numeric value cannot be stored!");
	}
	if (value.size()) {
		// set all
		for (Area::PathIterator it = area->pathBegin(); it != area->pathEnd(); ++it) {
			StorageCpu::setCellValue(*it, value);
		}
	} else {
		// clear existing
		if (area->getSize() == 1) {
			IdentifiersType key = *area->pathBegin();
			CellValue oldValue = getCellValue(key);
			if (!oldValue.isEmpty()) {
				StorageCpu::setCellValue(key, CellValue::NullString);
			}
		} else {
			PCellStream cs = getCellValues(area);
			while (cs->next()) {
				cs->setValue(CellValue::NullString);
			}
		}
	}
}

bool StringStorageCpu::setCellValue(PPlanNode plan, PEngineBase engine)
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "StringStorageCpu::setCellValue from plan not yet supported!");
}

PCellStream StringStorageCpu::commitChanges(bool checkLocks, bool add, bool disjunctive)
{
	PStringVector newStrings;
	bool changed = false;

	if (!checkLocks && (cleared > 1000 || pushed > 1000 || 0.8 * pushed > valuesCount())) {
		newStrings.reset(new StringVector());
		newStrings->setOld(strings->getOld() ? strings->getOld() : strings);
		changed = rebuildStrings(newStrings.get());
	}
	onlyDouble = true;
	PCellStream result = StorageCpu::commitChanges(checkLocks, false, disjunctive);
	onlyDouble = false;
	if (changed) {
		strings = newStrings;
	}
	return result;
}

void StringStorageCpu::load(FileReader *file, uint32_t fileVersion)
{
	StorageCpu::load(file, fileVersion);
	strings->load(file);
	if (strings->getPages().empty()) {
		strings->push("");
	}
}

void StringStorageCpu::save(FileWriter *file) const
{
	StorageCpu::save(file);
	strings->save(file);
}

double StringStorageCpu::convertToDouble(const CellValue &value)
{
	double d = 0;
	if (value.size()) {
		if (!strings->isCheckedOut()) {
			strings = boost::dynamic_pointer_cast<StringVector, Commitable>(strings->copy());
		}
		bool newString = true;
		if (stringMap) {
			map<string, StringVector::StringId>::iterator it = stringMap->find(value);
			if (it != stringMap->end()) {
				d = StringVector::decode(it->second);
				newString = false;
			}
		}
		if (newString) {
			StringVector::StringId id = strings->push(value);
			pushed++;
			d = StringVector::decode(id);
			if (stringMap && stringMap->size() < MAX_MAP_SIZE) {
				stringMap->insert(make_pair(value, id));
			}
		}
	} else {
		cleared++;
	}
	return d;
}

void StringStorageCpu::convertToCellValue(CellValue &value, double d) const
{
	if (onlyDouble) {
		value = d;
	} else {
		if (d) {
			StringVector::StringId id = StringVector::encode(d);
			value = CellValue(d, strings->getString(id));
		} else {
			value = CellValue::NullString;
		}
	}
}

bool StringStorageCpu::merge(const CPCommitable &o, const PCommitable &p)
{
	//validateStrings("before");
	checkCheckedOut();
	bool ret = true;
	CPStringStorageCpu storage = CONST_COMMITABLE_CAST(StringStorageCpu, o);
	bool sch = !strings->isCheckedOut() || !storage || strings->getOld() == storage->strings;
	bool pch = !pageList->isCheckedOut() || !storage || pageList->getOld() == storage->pageList;
	if (sch != pch) {
		ret = false;
	}
	if (ret) {
		if (strings->isCheckedOut()) {
			// new string was added or rebuildStrings was called
			ret = strings->merge(o != 0 ? storage->strings : PCommitable(), shared_from_this());
		} else if (storage) {
			strings = storage->strings;
			pushed = storage->pushed;
			cleared = storage->cleared;
			onlyDouble = storage->onlyDouble;
		}
	}
	if (ret) {
		ret = StorageCpu::merge(o, p);
	}
	//if (ret) {
	//	validateStrings("after");
	//}
	return ret;
}

PCommitable StringStorageCpu::copy() const
{
	checkNotCheckedOut();
	PCommitable newStorage(new StringStorageCpu(*this));
	return newStorage;
}

void StringStorageCpu::createStringMap()
{
	if (stringMap) {
		destroyStringMap();
	}
	stringMap = new map<string, StringVector::StringId>;
}

void StringStorageCpu::destroyStringMap()
{
	delete stringMap;
	stringMap = 0;
}

bool StringStorageCpu::rebuildStrings(StringVector *newStrings)
{
	createStringMap();
	newStrings->push("");
	bool changed = false;

	Processor p(*this, pageList.get(), CPArea(), 0);

	if (changedCells) {
		PCellStream cells = changedCells->getValues();
		while (cells->next()) {
			const IdentifiersType &key = cells->getKey();
			double d = cells->getDouble();
			if (d) {
				d = copyString(newStrings, d);
				changed = true;
				changedCells->set(key, d);
			}
		}
	}

	while (p.next()) {
		const IdentifiersType &key = p.getKey();
		if (!changedCells) {
			changedCells = CreateDoubleCellMap(key.size());
		}
		double d = copyString(newStrings, p.getDouble());
		changed = true;
		if (!changedCells->get(key, d)) {
			changedCells->set(key, d);
		}
	}
	pushed = 0;
	cleared = 0;
	destroyStringMap();
	return changed;
}

double StringStorageCpu::copyString(StringVector *newStrings, double d)
{
	StringVector::StringId id = StringVector::encode(d);
	string str = strings->getString(id);

	map<string, StringVector::StringId>::iterator it = stringMap->find(str);
	if (it != stringMap->end()) {
		id = it->second;
	} else {
		id = newStrings->push(str);
		pushed++;
		if (stringMap->size() < MAX_MAP_SIZE) {
			stringMap->insert(make_pair(str, id));
		}
	}
	return StringVector::decode(id);
}

bool StringStorageCpu::validateStrings(const string &prefix)
{
	bool ret = true;
	Processor p(*this, pageList.get(), CPArea(), 0);
	while (p.next()) {
		if (!strings->validate(p.getDouble())) {
			Logger::error << prefix << " invalid string at " << keytoString(p.getKey()) << endl;
			ret = false;
		}
	}
	return ret;
}

void StringStorageCpu::updateOld(CPStorageBase o)
{
	StorageCpu::updateOld(o);
	CPStringStorageCpu st = CONST_COMMITABLE_CAST(StringStorageCpu, o);
	strings->setOld(st->strings->getOld() ? st->strings->getOld() : st->strings);
}

void MarkerStorageCpu::setCellValue(const IdentifiersType &key, const CellValue &val)
{
	checkCheckedOut();
	bool found;
	if (!changedCells) {
		changedCells = CreateDoubleCellMap(key.size());
		found = false;
	} else {
		double d;
		found = changedCells->get(key, d);
	}
	if (!found) {
		changedCells->set(key, val.getNumeric());
	}
}

bool MarkerStorageCpu::setMarker(const IdentifiersType &key)
{
	double d;
	bool newMarker = !changedCells || !changedCells->get(key, d);
	if (newMarker) {
		setCellValue(key, CellValue::MarkerValue);
	}
	return newMarker;
}

PCommitable MarkerStorageCpu::copy() const
{
	checkNotCheckedOut();
	PCommitable newStorage(new MarkerStorageCpu(*this));
	return newStorage;
}

CellMapProcessor::CellMapProcessor(PEngineBase engine, CPPlanNode node) : ProcessorBase(true, PEngineBase()), area(node->getArea()), dimCount(-1)
{
	const CellMapPlanNode *pn = static_cast<const CellMapPlanNode *>(node.get());
	PStorageBase storage = engine->getStorage(pn->getStorageId());
	StorageCpu *st = dynamic_cast<StorageCpu *>(storage.get());
	if (st) {
		PDoubleCellMap changedCells = st->getChangedCells();
		if (changedCells) {
			valueStream = changedCells->getValues();
		}
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid storage for CellMapProcessor");
	}
}

bool CellMapProcessor::next()
{
	if (!valueStream) {
		return false;
	}
	if (dimCount < 0) {
		if (!area->getSize()) {
			return false;
		}
		dimCount = (uint32_t)area->dimCount();
		areaKey.resize(dimCount);
		for (int32_t i = 0; i < dimCount; i++) {
			areaKey[i] = area->elemBegin(i);
		}
	}
	while (valueStream->next()) {
		const IdentifiersType &key = valueStream->getKey();
		int32_t dim = 0;
		while (dim < dimCount) {
			IdentifierType id = *areaKey[dim];
			if (key[dim] < id) {
				break; // take the next key from valueStream;
			} else if (key[dim] == id) {
				dim++;
			} else {
				int32_t i;
				for (i = dim; i >= 0; i--) {
					if (nextAreaKey(i, key)) {
						id = *areaKey[i];
						break;
					}
				}
				if (i < 0) {
					return false; // no larger area key
				} else {
					for (int32_t j = i + 1; j < dimCount; j++) {
						areaKey[j] = area->elemBegin(j);
					}
					if (key[i] < id) {
						break; // take the next key from valueStream;
					}
				}
			}
		}
		if (dim == dimCount) {
			return true;
		}
	}
	return false;
}

bool CellMapProcessor::nextAreaKey(int32_t dim, const IdentifiersType &key)
{
	bool result = true;

	//try it quickly
	++areaKey[dim];
	if (areaKey[dim] == area->elemEnd(dim)) {
		result = false;
	} else if (*areaKey[dim] < key[dim]) {
		areaKey[dim] = area->getDim(dim)->lowerBound(key[dim]);
		result = areaKey[dim] != area->elemEnd(dim);
	}
	return result;
}

const CellValue &CellMapProcessor::getValue()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "CellMapProcessor::getValue not implemented");
}

double CellMapProcessor::getDouble()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "CellMapProcessor::getDouble not implemented");
}

const IdentifiersType &CellMapProcessor::getKey() const
{
	return valueStream->getKey();
}

void CellMapProcessor::reset()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "CellMapProcessor::reset not implemented");
}

uint64_t StorageCpu::getLastDeletionCount()
{
	uint64_t d = delCount;
	delCount = 0;
	return d;
}

}
