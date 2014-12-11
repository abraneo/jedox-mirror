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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

//#define PROFILE_AGGREGATION
//#define PROFILE_INDEX

#include "palo.h"
#include "Olap/Context.h"
#include "Logger/Logger.h"

#include <iostream>

#include "InputOutput/FileUtils.h"
#include "Exceptions/ErrorException.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp> //include all types plus i/o

#include "Olap/Server.h"
#include "Engine/Legacy/Engine.h"

#include "AggregationProcessor.h"
#include "CombinationProcessor.h"
#include "StorageCpu.h"

using namespace boost::posix_time;

namespace palo {

static INSTR *pINSTR;

StorageCpu::StorageCpu(PPathTranslator pathTranslator) :
	StorageBase(pathTranslator), valCount(0), emptySpace(0), nopPages(new unordered_map<uint32_t, uint32_t>), index(new set<Bookmark>),
	delCount(0), indexEnabled(true), pageList(new SlimVector<uint8_t>(STORAGE_PAGE_SIZE))
{
	if (!pINSTR) {
		pINSTR = new INSTR[256];
		for (int i = 0; i < 256; i++) {
			pINSTR[i].elementIdType = (i & ELEMENTMASK);
			pINSTR[i].valueType = (i & VALUEMASK);
		}
	}
}

StorageCpu::StorageCpu(const StorageCpu &storage) :
	StorageBase(storage), endStack(storage.endStack), valCount(storage.valCount), emptySpace(storage.emptySpace), nopPages(storage.nopPages), index(storage.index),
	delCount(0), indexEnabled(storage.indexEnabled), changedCells(storage.changedCells), changeNodes(storage.changeNodes), pageList(storage.pageList), longJumps(storage.longJumps)
{
}

StorageCpu::~StorageCpu()
{
	//cout << "StorageCpu::~StorageCpu called!" << endl;
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

ostream& operator<<(ostream& ostr, const StorageCpu::Bookmark &bookmark)
{
	ostr << "{Pos: " << bookmark.position << " Key: " << bookmark.key << " Jumps: " << bookmark.jumps <<  "}";
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

void StorageCpu::clear(CPCommitable oldPageList)
{
	pageList.reset(new SlimVector<uint8_t>(STORAGE_PAGE_SIZE));
	pageList->setOld(oldPageList);
	endStack.clear();
	valCount = 0;
	emptySpace = 0;
	nopPages.reset(new unordered_map<uint32_t, uint32_t>);
	index.reset(new set<Bookmark>);
}

void StorageCpu::buildIndex()
{
	checkCheckedOut();
	if (indexEnabled) {
		set<Bookmark> empty;
		const set<Bookmark> &oldidx = ccInfo.step && oldIndex ? *oldIndex : empty;
		index.reset(new set<Bookmark>);

#ifdef PROFILE_INDEX
		ptime indexingStart = microsec_clock::local_time();
#endif
		Processor reader(*this, pageList.get(), longJumps, PPathTranslator());
		reader.indexStorage(*index, oldidx);
#ifdef PROFILE_INDEX
    	ptime indexingEnd(microsec_clock::local_time());
    	size_t indexingTime = (indexingEnd-indexingStart).total_microseconds();
    	Logger::info << "Stream indexed in " << indexingTime/1000 << " ms. Total index entries: " << this->valCount << " pages: " << index->size() << endl;
#endif
	}
}

PCellStream StorageCpu::getCellValues(CPArea area, const CellValue *defaultValue)
{
	Processor *processor = 0;
	if (area) {
		if (!index->empty()) {
			// find bookmark
			Bookmark bookmark;
			bookmark.key = *area->pathBegin();
			set<Bookmark>::iterator bit = index->lower_bound(bookmark);
			if (bit != index->begin()) {
				--bit;
				processor = new Processor(*this, pageList.get(), longJumps, area, defaultValue, 0, &*bit);
			}
		}
		if (!processor) {
			processor = new Processor(*this, pageList.get(), longJumps, area, defaultValue);
		}
	} else {
		processor = new Processor(*this, pageList.get(), longJumps, pathTranslator);
	}
	return PCellStream(processor);
}

CellValue StorageCpu::getCellValue(const IdentifiersType &key)
{
	CellValue result;
	if (!index->empty()) {
		Bookmark bookmark;
		bookmark.key = key;
		set<Bookmark>::iterator bit = index->lower_bound(bookmark);
		if (bit != index->begin()) {
			--bit;
			Processor reader(*this, pageList.get(), longJumps, PPathTranslator(), 0, &*bit);
			reader.setFilter(&key[0], (int)key.size(), true);
			if (reader.next() && key==reader.getKey()) {
				result = reader.getValue();
			} else {
				result = CellValue::NullNumeric;
			}
			return result;
		}
	}

	Processor reader(*this, pageList.get(), longJumps, PPathTranslator()); // translator not needed in this processor
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
		} else {
			convertIndex(fromPosition);
//			if (false && !index.empty()) {
//				set<Bookmark> indexPushBack;
//				index.swap(indexPushBack);
//				Logger::trace << "indexing started! pages: " << pageList.get()->pageCount() << endl;
//				buildIndex();
//				Logger::trace << "indexing done! indexes: " << index.size() << endl;
//				set<Bookmark>::iterator i1 = indexPushBack.begin();
//				set<Bookmark>::iterator i2 = index.begin();
//				uint64_t tstPos = STORAGE_PAGE_SIZE;
//				while (i2 != index.end() && i1 != indexPushBack.end()) {
//					if (i1->jumps != i2->jumps || i1->position != i2->position || i1->key != i2->key) {
//						// different indexes
//						cout << *i1 << "!=" << *i2 << endl;
////						exit(0);
//					}
//					if (i1->position != tstPos) {
//						// missing indexPushBack bookmark
//						cout << "missing indexPushBack bookmark: " << tstPos << ' ' << *i1 << endl;
////						exit(0);
//					} else if (i2->position != tstPos) {
//						// missing index bookmark
//						cout << "missing index bookmark: " << tstPos << ' ' << *i2 << endl;
////						exit(0);
//					}
//
//					int compare = i1->compare(*i2);
//					if (!compare) {
//						++i1; ++i2;
//					} else if (compare < 0) {
//						++i1;
//					} else {
//						++i2;
//					}
//					tstPos += STORAGE_PAGE_SIZE;
//				}
//				if (i2 != index.end() || i1 != indexPushBack.end()) {
//					cout << "Index verification failed!" << endl;
////					exit(0);
//				}
//			}
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
		baseNodes.push_back(PPlanNode(new SourcePlanNode(getId(), area, 0)));
		baseNodes.push_back(PPlanNode()); // empty plan activating the constant multiplication
		changeNodes.push_back(PPlanNode(new PlanNode(MULTIPLICATION, area, 0, baseNodes, 0, PCube(), 0)));
	} else if (opType == MULTIPLY_EXISTING) {
		// multiply existing
		vector<PPlanNode> baseNodes;
		baseNodes.push_back(PPlanNode(new SourcePlanNode(getId(), area, 0)));
		baseNodes.push_back(PPlanNode()); // empty plan activating the constant multiplication
		changeNodes.push_back(PPlanNode(new PlanNode(MULTIPLICATION, area, 0, baseNodes, 0, PCube(), value.getNumeric())));
	} else if (opType == SET) {
		if (area->getSize() == 1) {
			setCellValue(*area->pathBegin(), value);
		} else {
			changeNodes.push_back(PPlanNode(new PlanNode(CONSTANT, area, &value, vector<PPlanNode>(), 0, CPCube())));
		}
	} else if (opType == ADD_ALL) {
		if (area->getSize() == 1) {
			Area::PathIterator pit = area->pathBegin();
			if (changedCells) {
				changedCells->add(*pit, value.getNumeric());
			} else {
				setCellValue(*pit, value);
			}
			//double val;
			//if (changedCells && changedCells->get(*pit, val)) {
			//	cellValue = val + value.getNumeric();
			//	setCellValue(*pit, cellValue);
			//} else {
			//	setCellValue(*pit, value);
			//}
		} else {
			changeNodes.push_back(PPlanNode(new PlanNode(CONSTANT, area, &value, vector<PPlanNode>(), 0, CPCube())));
		}
	}
}

bool StorageCpu::setCellValue(CPPlanNode plan, PEngineBase engine)
{
	bool result = false;
	size_t dimCount = plan->getArea()->dimCount();
	PDoubleCellMap spFinalTargetBatch(CreateDoubleCellMap(dimCount));
	ICellMap<double> *finalTargetBatch = spFinalTargetBatch.get();

	// erase target area first
	PCellStream oldData = getCellValues(plan->getArea(), 0);

	// store old values in temporary storage as zeros
	while (oldData->next()) {
		finalTargetBatch->set(oldData->getKey(), 0);
	}

	// create processor with new data
	PCellStream targetData = engine->createProcessor(plan);

	// move new value to temporary storage
	while (targetData->next()) {
		const CellValue &sourceValue = targetData->getValue();
		if (sourceValue.isNumeric()) {
			finalTargetBatch->set(targetData->getKey(), sourceValue.getNumeric());
		} else if (sourceValue.isEmpty()) {
			// not sure if it can go here
		}
	}
	PCellStream changedValues = dynamic_cast<ICellMapStream *>(spFinalTargetBatch.get())->getValues();

	// move content of temporary storage to actual storage
	while (changedValues->next()) {
		double value = changedValues->getDouble();
		if (value == 0) {
			result = true;
		}
		setCellValue(changedValues->getKey(), CellValue(value));
	}
	return result;
}

PCellStream StorageCpu::commitChanges(bool checkLocks, bool add, bool disjunctive)
{
	bool singleValue = false;
	PCellStream st;
	if (changedCells && changedCells->size()) {
		disjunctive = false;
		changeNodes.push_back(PPlanNode(new CellMapPlanNode(changedCells)));
		singleValue = changeNodes.size() == 1 && changedCells->size() == 1;
	}
	if (changeNodes.size()) {
		PEngineBase engine = Context::getContext()->getServerCopy()->getEngine(EngineBase::CPU, true);
		if (changeNodes.size() == 1) {
			if (changedCells) {
				Logger::trace << "commitChanges, changeNodes.size() == 1, changedCells->size(): " << changedCells->size() << endl;
			} else {
				Logger::trace << "commitChanges, changeNodes.size() == 1, changeNodes[0]->getArea()->getSize(): " << changeNodes[0]->getArea()->getSize() << ", " << *changeNodes[0]->getArea() << endl;
			}
			st = engine->createProcessor(changeNodes[0], false);
		} else {
			if (add) {
				Logger::trace << "commitChanges, ADDITION, changeNodes->size(): " << changeNodes.size() << endl;
				if (changedCells) {
					Logger::trace << "commitChanges, ADDITION, changedCells->size(): " << changedCells->size() << endl;
				}
				CPPlanNode node(new PlanNode(ADDITION, changeNodes[0]->getArea(), 0, changeNodes, 0, CPCube()));
				st = engine->createProcessor(node, false);
			} else {
				Logger::trace << "commitChanges, COMBINATION, disjunctive: " << disjunctive << ", changeNodes->size(): " << changeNodes.size() << endl;
				if (changedCells) {
					Logger::trace << "commitChanges, COMBINATION, disjunctive: " << disjunctive << ", changedCells->size(): " << changedCells->size() << endl;
				}
				CPPlanNode node(new PlanNode(COMBINATION, changeNodes[0]->getArea(), 0, changeNodes, 0, CPCube()));
				for (size_t i = 0; i < changeNodes.size(); i++) {
					if (changeNodes[i]->skipEmpty()) {
						disjunctive = false;
						break;
					}
				}
				if (disjunctive) {
					st = PCellStream(new DisjunctiveCombinationProcessor(engine, node));
				} else {
					st = PCellStream(new CombinationProcessor(engine, node));
				}
			}
		}
	}

	PCellStream result;
	if (st) {
		result = commitExternalChanges(checkLocks, st, singleValue, add);
	}
	changedCells.reset();
	changeNodes.clear();
	return result;
}

PCellStream StorageCpu::commitExternalChanges(bool checkLocks, PCellStream changes, bool singleValue, bool add)
{
//	ptime indexingStart = microsec_clock::local_time();
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
	if (checkLocks) {
		res = CreateDoubleCellMap(changes->getKey().size());
	}
	if (!append) {
		IdentifiersType endKey;
		for (vector<ElemRestriction>::const_iterator it = endStack.begin(); it != endStack.end(); ++it) {
			endKey.push_back(it->elementId);
		}
		append = CellValueStream::compare(changes->getKey(), endKey) > 0;
	}
	changes->reset();
	bool forcerebuild = false;
	bool rebuild = false;
	if (append) {
		if (!pageList->isCheckedOut()) {
			pageList = COMMITABLE_CAST(SlimVector<uint8_t>, pageList->copy());
		}
		Writer w(*this, false);
		while (changes->next()) {
			double val = changes->getDouble();
			if (val) {
				w.push_back(changes->getKey(), val);
			}
			if (checkLocks) {
				res->set(changes->getKey(), 0);
			}
		}
		buildIndex();
	} else {
		boost::shared_ptr<SlimVector<uint8_t> > oldPageList = pageList;
		vector<size_t> oldLongJumps = longJumps;
		size_t vals = valCount;
		if (singleValue) {
			forcerebuild = true;
			rebuild = vals < 4000 || (pageList->pageCount() > 5 && 2 * emptySpace > (size_t)pageList->pageCount() * pageList->maxPageSize());
			if (!rebuild && indexEnabled) {
				ccInfo.init(pageList->pageCount());
			}
		} else {
			if (vals < 4000 || (pageList->pageCount() > 5 && 2 * emptySpace > (size_t)pageList->pageCount() * pageList->maxPageSize()) || (!longJumps.empty() && pageList->size() < LONG_JUMP_START_VALUE)) {
				rebuild = true;
				forcerebuild = true;
			}
		}
		if (forcerebuild && rebuild) {
			longJumps.clear();
		}

		size_t oldEmpty = emptySpace;
		vector<ElemRestriction> oldEndStack = endStack;
		oldnopPages = nopPages;
		oldIndex = index;
		clear(oldPageList->getOld() ? oldPageList->getOld() : oldPageList);
		Processor p(*this, oldPageList.get(), oldLongJumps, CPArea(), 0);
		Processor p1(p);
		Writer w(*this, rebuild && forcerebuild);
		bool contPrev = p.next();
		bool contComp = add ? false : p1.next();
		bool diffValue = false;
		bool bDel = false;
		while (changes->next()) {
			const IdentifiersType &key = changes->getKey();
			if (ccInfo.step) {
				ccInfo.newKey = key;
			}
			double val = changes->getDouble();
			if (contPrev) {
				bool found = false;
				double p1val = 0;
				if (contComp) {
					if (CellValueStream::compare(key, p1.getKey()) == 0) {
						p1val = p1.getDouble();
						contComp = p1.next();
						found = true;
					} else {
						contComp = p1.move(key, &found);
						if (found) {
							p1val = p1.getDouble();
							contComp = p1.next();
						} else {
							if (!val) {
								found = true;
							}
						}
					}
				}
				if (found) {
					if (p1val == val || (isNumeric() && compareDouble(p1val, val))) {
						continue;
					}
				}
				diffValue = true;
				contPrev = w.writeUntil(&p, &key, rebuild);
			}
			if (ccInfo.step) {
				ccInfo.step = 2;
			}
			if (contPrev && CellValueStream::compare(key, p.getKey()) == 0) {
				if (checkLocks) {
					res->set(p.getKey(), p.getDouble());
				}
				if (add) {
					val += p.getDouble();
				}
				if (val) {
					w.push_back(key, val);
				} else {
					bDel = true;
					if (diffValue) {
						delCount++;
					}
					--vals;
				}
				if (ccInfo.step) {
					if (bDel) {
						ccInfo.newValueOldPos = ccInfo.nextValueOldPos;
						ccInfo.nextValueNewPos = pageList->size();
					}
					ccInfo.nextValueOldPos = p.getPositionJumpNOP();
				}
				contPrev = p.next();
			} else {
				if (val) {
					++vals;
					w.push_back(key, val);
				} else {
					bDel = true;
					if (diffValue) {
						delCount++;
					}
				}
				if (checkLocks) {
					res->set(key, 0);
				}
				if (bDel && ccInfo.step) {
					ccInfo.newValueOldPos = ccInfo.nextValueOldPos;
				}
			}
			if (!forcerebuild) {
				if (emptySpace > 0.4 * pageList->size()) {
					rebuild = true;
				} else {
					rebuild = false;
				}
			}
		}
		if (diffValue) {
			if (contPrev) {
				if (ccInfo.step) {
					ccInfo.step = 3;
				}
				w.writeUntil(&p, 0, rebuild);
				if (ccInfo.step) {
					ccInfo.newLastPos = pageList->size();
				}
			}
			if (ccInfo.step && bDel) {
				ccInfo.newValueNewPos = ccInfo.nextValueNewPos;
			}
			if (rebuild && forcerebuild) {
				convertIndex(0);
			} else {
				buildIndex();
			}
		} else {
			nopPages = oldnopPages;
			pageList = oldPageList;
			emptySpace = oldEmpty;
			endStack = oldEndStack;
			index = oldIndex;
		}
		valCount = vals;
		oldnopPages.reset();
#ifdef _DEBUG
		if (ccInfo.step || (rebuild && forcerebuild)) {
			boost::shared_ptr<set<Bookmark> > idx = index;
			ccInfo.clear(false);
			buildIndex();
			if (*idx != *index) {

				set<Bookmark>::iterator itIdx = idx->begin();
				set<Bookmark>::iterator itIndex = index->begin();
				while (itIdx != idx->end()) {
					if (*itIdx != *itIndex) {
						Logger::error << *itIdx << endl;
						Logger::error << *itIndex << endl;
					} else {
						Logger::info << *itIdx << endl;
					}
					++itIdx;
					++itIndex;
				}

				throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::commitExternalChanges - different index");
			}
		}
#endif
		oldIndex.reset();
	}
	if (checkLocks) {
		result = dynamic_cast<ICellMapStream *>(res.get())->getValues();
	}
	ccInfo.clear(false);
	if (false && !validate(true)) {
		if (changes) {
			changes->reset();
			if (changes->next()) {
				cout << keytoString(changes->getKey()) << endl;
			}
		}
	}
//	Logger::trace << "storage value: " << this->valuesCount() << " size: " << this->pageList->size() << " long jumps: " << longJumps.size() << endl;

//	ptime indexingEnd(microsec_clock::local_time());
//	size_t indexingTime = (indexingEnd-indexingStart).total_microseconds();
//	Logger::info << "commitExternalChanges took " << indexingTime/1000 << " ms append: " << append << " forcerebuild " << forcerebuild << endl;
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
	mergeint(o);
	CPStorageCpu storage = CONST_COMMITABLE_CAST(StorageCpu, o);
	if (pageList->isCheckedOut()) {
		ret = pageList->merge(o != 0 ? storage->pageList : PCommitable(), shared_from_this());
	} else {
		if (storage) {
			pageList = storage->pageList;
			valCount = storage->valCount;
			emptySpace = storage->emptySpace;
			nopPages = storage->nopPages;
			endStack = storage->endStack;
			index = storage->index;
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

	file->getRaw((char *)&i, sizeof(uint64_t));
	nopPages->clear();
	for (uint64_t j = 0; j < i; j++) {
		file->getRaw((char *)&i1, sizeof(uint32_t));
		file->getRaw((char *)&i2, sizeof(uint32_t));
		nopPages->insert(make_pair(i1, i2));
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
	}

	if (oldnopPages) {
		oldnopPages->clear();
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

	i = nopPages->size();
	file->appendRaw((const char *)&i, sizeof(uint64_t));
	for (unordered_map<uint32_t, uint32_t>::iterator it = nopPages->begin(); it != nopPages->end(); ++it) {
		uint32_t j = it->first;
		file->appendRaw((const char *)&j, sizeof(uint32_t));
		j = it->second;
		file->appendRaw((const char *)&j, sizeof(uint32_t));
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
	Logger::info << "Validation started." << endl;

	Processor p(*this, pageList.get(), longJumps, CPArea(), 0);
	Processor pm(*this, pageList.get(), longJumps, CPArea(), 0);
	size_t checkedValues = 0;
	IdentifiersType lastKey;
	int lastDepth = 0;
	bool first = true;
	bool err = false;
	try {
		while (p.next()) {
			++checkedValues;
			const IdentifiersType &k = p.getKey();
			if (p.depth + 1 != (int32_t)k.size()) {
				Logger::error << "Key size of " << keytoString(k) << " differs from depth " << p.depth << endl;
				err = true;
			}
			for (int32_t i = 0; i < p.depth - 1; ++i) {
				const SlimVector<uint8_t> &sv = *pageList.get();
				if (sv[p.offset[i]] & JUMP) {
					size_t distance = getJump(&sv[p.offset[i]+1], longJumps);
//					memcpy((uint8_t *)&distance, &sv[p.offset[i] + 1], 4);
					if (p.offset[i] + distance < sv.size() && sv[p.offset[i] + distance] == NOP) {
						Logger::error << "Key " << keytoString(k) << " index " << i << " jumps to NOP" << endl;
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
					Logger::error << "Key size of " << keytoString(k) << " differs from last key size " << keytoString(lastKey) << endl;
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
					if (CellValueStream::compare(pm.getKey(), lastKey) != 0) {
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
	} catch (ErrorException &e) {
		Logger::error << "Got exception: " << e.getMessage() << endl;
		err = true;
	}
	if (err) {
		Logger::info << "Validation finished with errors." << endl;
	} else {
		Logger::info << "Validation finished OK." << endl;
	}
	return !err;
}

StorageCpu::Bookmark StorageCpu::endBookMark()
{
	Bookmark bookmark;
	bookmark.jumps.reserve(endStack.size());
	bookmark.key.reserve(endStack.size());
	bookmark.position = pageList.get()->size();

	for (vector<ElemRestriction>::iterator si = endStack.begin(); si != endStack.end(); ++si) {
		if (si->elementId == NO_IDENTIFIER) {
			break;
		}
		bookmark.key.push_back(si->elementId);
		bookmark.jumps.push_back(si->iOffset);
	}
	return bookmark;
}

// -jj- LONG JUMP
void StorageCpu::convertIndex(size_t fromPosition)
{
	if (index->empty()) {
		return;
	}
	if (endStack.empty()) {
		Logger::error << "endStack.empty() in StorageCpu::convertIndex" << endl;
		index->clear();
	}
	vector<size_t> lastOffset(endStack.size(), NO_OFFSET);
	vector<size_t> offsetJump(endStack.size(), 0);
	SlimVector<uint8_t> &sv = *pageList.get();

	for (set<Bookmark>::iterator bit = index->begin(); bit != index->end(); ++bit) {
		if (bit->position < fromPosition) {
			continue;
		}
		// for each bookmark
		vector<size_t>::iterator lastOffsetIt = lastOffset.begin();
		vector<size_t>::iterator offsetJumpIt = offsetJump.begin();
		vector<size_t> &jumps = const_cast<vector<size_t> &>(bit->jumps);
		for (vector<size_t>::iterator jit = jumps.begin(); jit != jumps.end(); ++jit, ++lastOffsetIt, ++offsetJumpIt) {
			// for each bookmark dimension
			// convert offset to jump
			if (*jit != NO_OFFSET && (sv[*jit] & JUMP)) {
				if (*jit == *lastOffsetIt) {
					// check conversion cache
					*jit = *offsetJumpIt;
					continue;
				}
				// save offset to the cache
				*lastOffsetIt = *jit;

				size_t distance = getJump(&sv[*jit+1], longJumps);
//				uint8_t *mem = &sv[*jit+1];
//				memcpy((uint8_t *)&distance, mem, 4);
				*jit += distance;
				*offsetJumpIt = *jit;
			} else {
				*jit = bit->position;
			}
		}
	}
}

ostream& operator<<(ostream& ostr, const IdentifiersType& v)
{
	bool first = true;
	ostr << "[" << dec;
	for (vector<uint32_t>::const_iterator it = v.begin(); it != v.end(); ++it) {
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
	ostr << "]";
	return ostr;
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
	if (!area || !defaultValue) {
		// S->V | E
		// no filter or no default value
		return nextIntern();
	} else  {
		if (isDefaultValue()) {
			if (nextRepetition()) {
				// R
				return true;
			} else {
				// use current existing V
				return !endReached;
			}
		} else {
			IdentifiersType prevKey(vkey);

			// search for next existing value (V | R | E)
			while (!endReached) {
				bool result = nextIntern();
				if (result) {
					ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, &getKey());
					return true;
				}
			}
			return 0 != ConstantRepeater::setRepetition(prevKey.empty() ? 0 : &prevKey, 0);
		}
	}
}

bool StorageCpu::Processor::nextIntern()
{
//	int tests = 0;
//	int pagesused = 0;
//	int nopCount = 0;
	bool valueReached = false;
	int ValueType = 0;
	size_t nextPagePos = 0;
	const uint8_t *p = 0;
	do {
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
				p = &(*pageList)[pos];
//				pagesused++;
			}
			uint8_t instruction = p[instrPos++];
			if (instruction == NOP) {
//				nopCount++;
				pos++;
				// find the limit position
				size_t stopPos = nextPagePos;
				if (depth >= 0 && jump[depth] < stopPos) {
					stopPos = jump[depth];
				}
				const uint8_t *px = p+instrPos;
				while (pos < stopPos && *px++ == NOP) {
//					nopCount++;
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
				jumpPosition = StorageCpu::getJump(p+instrPos, longJumps); instrPos += 4;
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
				if (!pFilter || compareKey(key, pFilter, depth) < 0) {
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
						for (instruction = (*pageList)[pos + nextPos]; instruction == NOP && pos + nextPos < endp; instruction = (*pageList)[pos + ++nextPos]);
					}
					if (depth > 0 ? (pos + nextPos < jump[depth - 1]) : (pos + nextPos < endp)) {
						uint32_t nInstr = 0;
						size_t njump = 0;
						const uint8_t *p1 = &(*pageList)[pos + nextPos];
						if (instruction & JUMP) {
							njump = StorageCpu::getJump(p1 + nInstr + 1, longJumps); nInstr += 4;
//							pval8 = (uint8_t *)&njump;
//							memcpy(pval8, p1 + nInstr + 1, 4); nInstr += 4;
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
							key[depth] = nelementId;
							if (compareKey(key, pFilter, depth) <= 0) {
								instrPos = nextPos;
								ValueType = 0;
								key[depth] = elementId;
								if (setSingle[depth] == NO_IDENTIFIER && (size_t)depth < setRange.size()) {
									setRange[depth] = setEndRange[depth];
								}
								jump[depth--] = pos + nextPos + njump;
							} else {
								key[depth] = elementId;
							}
						} else {
							instrPos = nextPos;
							ValueType = 0;
							if (setSingle[depth] == NO_IDENTIFIER && (size_t)depth < setRange.size()) {
								setRange[depth] = setEndRange[depth];
							}
							jump[depth--] = pos + nextPos + njump;
						}
					}
				} else if (compareKey(key, pFilter, depth) > 0) {
					while (prevKey[depth] == NO_IDENTIFIER || !pFilter[depth]) {
						--depth;
					}
					memset(pFilter + depth + 1, 255, (DIM_MAX - depth - 1) * sizeof(uint32_t));
					memset(key + depth, 255, (DIM_MAX - depth) * sizeof(uint32_t));
					pFilter[depth] = key[depth] < pFilter[depth] ? key[depth] - 1 : pFilter[depth] - 1;
					if (depth) {
						--depth;
						pos = offset[depth];
						offset[depth] = prevOffset[depth];
						key[depth] = prevKey[depth];
					} else {
						pos = prevOffset[depth];
						key[depth] = prevKey[depth] == NO_IDENTIFIER ? NO_IDENTIFIER : prevKey[depth] - 1;
						offset[depth] = 0;
					}
					--depth;
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
//					tests++;
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
//	if (tests) {
//		cout << value << '\t' << tests << '\t' << pagesused << endl;
//	}
	return valueReached;
}

//static int myCount = 0;
//static int myLimit = -1;

void StorageCpu::Processor::buildIndex(const set<Bookmark> &oldIndex)
{
//	cout << "buildIndex" << endl;
	if (!indexptr || before || area || pFilter) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Invalid call of StorageCpu::Processor::buildIndex()!");
	}

	uint32_t oldPage = 0;
	set<Bookmark>::const_iterator oldIt = oldIndex.begin();

	int ValueType = 0;
	size_t nextPagePos = 0;
	const uint8_t *p = 0;
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
				p = &(*pageList)[pos];
				if (pos >= STORAGE_PAGE_SIZE) {
					uint32_t iPage = (uint32_t)(pos / STORAGE_PAGE_SIZE) - 1;
					if (oldIndex.empty() || !storage.ccInfo.step || storage.ccInfo.build[iPage]) {
						Bookmark bookmark = getBookmark();
						indexptr->insert(bookmark);

//						cout << bookmark << endl;
//						myCount++;
//						if (myCount == myLimit) {
//							cout << myLimit << endl;
//						}

						// should always be beginning of the page
						if (pos % STORAGE_PAGE_SIZE) {
							Logger::debug << "Bookmark: " << bookmark << " not at the page beginning! Page size: " << STORAGE_PAGE_SIZE << endl;
							throw ErrorException(ErrorException::ERROR_INTERNAL, "Bookmark not at the page beginning!");
						}
					} else {
						Bookmark bookmark;
						uint32_t maxPage = storage.ccInfo.newToOld[iPage];
						for (; oldPage < maxPage; oldPage++) {
							++oldIt;
						}
						size_t size = oldIt->key.size();
						while (oldPage < oldIndex.size() && !storage.ccInfo.build[iPage]) {
							bookmark.key = oldIt->key;
							size = bookmark.key.size();
							bookmark.jumps.resize(size);
							bool jumpToNew = false;
							for (size_t i = 0; i < size; i++) {
								if (oldIt->jumps[i] < storage.ccInfo.newValueOldPos) {
									bookmark.jumps[i] = oldIt->jumps[i] + storage.ccInfo.diff1 * (int64_t)STORAGE_PAGE_SIZE;
								} else if (oldIt->jumps[i] > storage.ccInfo.newValueOldPos) {
									if (oldIt->jumps[i] == storage.ccInfo.nextValueOldPos) {
										bookmark.jumps[i] = storage.ccInfo.nextValueNewPos;
									} else if (oldIt->jumps[i] == storage.ccInfo.oldLastPos) {
										bookmark.jumps[i] = storage.ccInfo.newLastPos;
									} else {
										bookmark.jumps[i] = oldIt->jumps[i] + (storage.ccInfo.diff1 + storage.ccInfo.diff2) * (int64_t)STORAGE_PAGE_SIZE;
									}
								} else {
									if (oldIt->key[i] == storage.ccInfo.newKey[i] && !jumpToNew) {
										bookmark.jumps[i] = storage.ccInfo.nextValueNewPos;
									} else {
										bookmark.jumps[i] = storage.ccInfo.newValueNewPos;
										jumpToNew = true;
									}
								}
							}
							bookmark.position = bookmark.jumps[size - 1];
							indexptr->insert(bookmark);

//							cout << bookmark << endl;
//							myCount++;
//							if (myCount == myLimit) {
//								cout << myLimit << endl;
//							}

							iPage++;
							oldPage++;
							++oldIt;
						}
						if (bookmark.key.size() != size) {
							Logger::error << "StorageCpu::Processor::buildIndex optimization failed. Rebuilding index. Key: " << storage.ccInfo.newKey << endl;
							indexptr->clear();
							reset();
							return buildIndex(set<Bookmark>());
						}

						pos = bookmark.position;
						currentPos = pos;
						instrPos = 0;

						for (size_t i = 0; i < size; i++) {
							key[i] = bookmark.key[i];
							jump[i] = bookmark.jumps[i];
						}
						depth = (int)size - 1;
						nextPagePos = ((pos / STORAGE_PAGE_SIZE)+1)*STORAGE_PAGE_SIZE;
						p = &(*pageList)[pos];
						continue;
					}
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
				jumpPosition = StorageCpu::getJump(p+instrPos, longJumps); instrPos += 4;
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
	IdentifierType bMinId[DIM_MAX];
	IdentifierType bMaxId[DIM_MAX];
	IdentifierType cMaxId[DIM_MAX];
	const IdentifierType *b2PArray[DIM_MAX];
	const double *b2PWeightsArray[DIM_MAX];
	const IdentifierType *multiParents[DIM_MAX];
	const IdentifierType *currentParent[DIM_MAX];
	const double *multiWeights[DIM_MAX];
	const double *currentWeight[DIM_MAX];
	uint8_t multiDim[DIM_MAX];
	int multiCount = 0;

#ifdef PROFILE_AGGREGATION
	double sum = 0;
	size_t values = 0;
	ptime aggregationStart = microsec_clock::local_time();
	size_t nodesVisited = 0;
	size_t pagesVisited = 0;
	size_t setIterIncr = 0;
	size_t setIterEndTest = 0;
	size_t setIterReset = 0;
#endif

	for (size_t dim = 0; dim < hashStorage->dimCount; dim++) {
		parentMaps[dim]->getBaseParams(bMinId[dim], bMaxId[dim], cMaxId[dim], b2PArray[dim], b2PWeightsArray[dim]);
		singleParent[dim] = parentMaps[dim]->getSingleParent() != NO_IDENTIFIER;
		offsets[dim] = 0;
		hasMultiMap[dim] = parentMaps[dim]->hasMultiMap();
		multiParents[dim] = 0;
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
			if (!singleParent[dim]) {
				IdentifierType parentId = b2PArray[dim][key[dim]-bMinId[dim]];
				if (parentId != NO_IDENTIFIER) {
					if (hasMultiMap[dim]) {
						if (parentId > cMaxId[dim]) {
							// element has multiple parents in resultset
							multiDim[multiCount] = (uint8_t)dim;
							size_t startOffset = parentId-cMaxId[dim]+bMaxId[dim]-bMinId[dim];
							multiParents[dim] = b2PArray[dim]+startOffset;
							currentParent[dim] = multiParents[dim];
							multiWeights[dim] = b2PWeightsArray[dim] ? b2PWeightsArray[dim]+startOffset : 0;
							currentWeight[dim] = multiWeights[dim];
							multiCount++;
							if (weightSlot[dim]) {
								weights[weightSlot[dim]-1] = 1;
							}
						} else {
							// single parent
							offsets[dim] = hashStorage->offsets[dim][parentId-hashStorage->firstIds[dim]];
							cachedOffset += offsets[dim];
							if (weightSlot[dim]) {
								weights[weightSlot[dim]-1] = b2PWeightsArray[dim][key[dim]-bMinId[dim]];
							}
						}
					} else {
		//				cout << pos << "\tpar: " << parentId << endl;
						offsets[dim] = hashStorage->offsets[dim][parentId-hashStorage->firstIds[dim]];
		//				cout << pos << "\toff: " << offsets[dim] << endl;
						cachedOffset += offsets[dim];
						if (weightSlot[dim]) {
							weights[weightSlot[dim]-1] = b2PWeightsArray[dim][key[dim]-bMinId[dim]];
						}
					}
				} else {
					Logger::error << "StorageCpu::Processor::aggregate: parentId == NO_IDENTIFIER" << endl;
				}
			} else if (weightSlot[dim]) {
				weights[weightSlot[dim]-1] = b2PWeightsArray[dim][key[dim]-bMinId[dim]];
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
				if (multiParents[depth]) {
					if (multiCount && multiDim[multiCount-1] == (uint8_t)depth) {
						multiCount--;
						currentParent[depth] = multiParents[depth] = 0;
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
				p = &(*pageList)[pos];
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
				jumpPosition = StorageCpu::getJump(p+instrPos, longJumps); instrPos += 4;
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
//						setRange[depth] = sets[depth]->rangeBegin();
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
					if (!singleParent[depth]) {
						IdentifierType parentId = b2PArray[depth][elementId-bMinId[depth]];
						if (hasMultiMap[depth]) {
							if (multiParents[depth]) {
								multiCount--;
								currentParent[depth] = multiParents[depth] = 0;
							} else {
								cachedOffset -= offsets[depth];
							}
							if (parentId > cMaxId[depth]) {
								// element has multiple parents in resultset
								multiDim[multiCount] = (uint8_t)depth;
								size_t startOffset = parentId-cMaxId[depth]+bMaxId[depth]-bMinId[depth];
								multiParents[depth] = b2PArray[depth]+startOffset;
								currentParent[depth] = multiParents[depth];
								multiWeights[depth] = b2PWeightsArray[depth] ? b2PWeightsArray[depth]+startOffset : 0;
								currentWeight[depth] = multiWeights[depth];
								multiCount++;
								if (weightSlot[depth]) {
									weights[weightSlot[depth]-1] = 1;
								}
							} else {
								// single parent
								offsets[depth] = hashStorage->offsets[depth][parentId-hashStorage->firstIds[depth]];
								cachedOffset += offsets[depth];
								if (weightSlot[depth]) {
									weights[weightSlot[depth]-1] = b2PWeightsArray[depth][elementId-bMinId[depth]];
								}
							}
						} else {
							// single parent
							cachedOffset -= offsets[depth];
							offsets[depth] = hashStorage->offsets[depth][parentId-hashStorage->firstIds[depth]];
							cachedOffset += offsets[depth];
							if (weightSlot[depth]) {
								weights[weightSlot[depth]-1] = b2PWeightsArray[depth][elementId-bMinId[depth]];
							}
						}
					} else if (weightSlot[depth]) {
						weights[weightSlot[depth]-1] = b2PWeightsArray[depth][elementId-bMinId[depth]];
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
							offsets[currentDim] = hashStorage->offsets[currentDim][*currentParent[currentDim]-hashStorage->firstIds[currentDim]];
							multiOffset += offsets[currentDim];
							if (currentWeight[currentDim]) {
								multiValue *= *currentWeight[currentDim];
							}
						}
						hashStorage->resultSet[multiOffset] = 1;
						hashStorage->results[multiOffset] += multiValue;

						changeMultiDim = multiCount - 1;
						bool endOfIteration = false;
						while (!endOfIteration && changeMultiDim >= 0 && changeMultiDim < multiCount) {
							size_t currentDim = multiDim[changeMultiDim];
							++currentParent[currentDim];
							if (currentWeight[currentDim]) {
								currentWeight[currentDim]++;
							}
							if (*currentParent[currentDim] == NO_IDENTIFIER) {
								currentParent[currentDim] = multiParents[currentDim];
								currentWeight[currentDim] = multiWeights[currentDim];
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
	bool cont = false;
	if (depth > 0 ? (pos < jump[depth - 1]) : (pos < endp)) {
		const uint8_t *p1 = &(*pageList)[pos];
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
	if (isDefaultValue()) {
		return ConstantRepeater::getValue();
	} else {
		storage.convertToCellValue(cellvalue, value);
		return cellvalue;
	}
}

double StorageCpu::Processor::getDouble()
{
	if (isDefaultValue()) {
		return ConstantRepeater::getDouble();
	} else {
		return value;
	}
}

const IdentifiersType &StorageCpu::Processor::getKey() const
{
	if (isDefaultValue()) {
		return ConstantRepeater::getKey();
	} else {
		return vkey;
	}
}

StorageCpu::Processor::Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, vector<size_t> &longJumps, CPArea area, const CellValue *defaultValue, const size_t endPos, const Bookmark *bookmark) :
	ConstantRepeater(area, defaultValue), pageList(pageList), longJumps(longJumps), pstorage(storage.shared_from_this()), storage(storage), mtCallback(0), indexptr(0)
{
	reset();
	endp = endPos ? endPos : pageList->size();
	if (bookmark) {
		stackSize = (int)bookmark->key.size();
		pos = bookmark->position;
		currentPos = bookmark->position;
		for (int dim = 0; dim < stackSize; dim++) {
			key[dim] = bookmark->key[dim];
			jump[dim] = bookmark->jumps[dim];
		}
		depth = stackSize-1;
		if (area) {
			Area::ConstDimIter dimIt = area->dimBegin();
			for (int newDepth = 0; newDepth < depth; newDepth++, ++dimIt) {
				if ((*dimIt)->find(key[newDepth]) == (*dimIt)->end()) {
					currentPos = pos = jump[newDepth];
					depth = newDepth-1;
					break;
				}
			}
		}
	}
}

StorageCpu::Processor::Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, vector<size_t> &longJumps, PPathTranslator pathTranslator, const size_t endPos, const Bookmark *bookmark) :
	ConstantRepeater(pathTranslator), pageList(pageList), longJumps(longJumps), pstorage(storage.shared_from_this()), storage(storage), mtCallback(0), indexptr(0)
{
	reset();
	endp = endPos ? endPos : pageList->size();
	if (bookmark) {
		stackSize = (int)bookmark->key.size();
		pos = bookmark->position;
		currentPos = bookmark->position;
		for (int dim = 0; dim < stackSize; dim++) {
			key[dim] = bookmark->key[dim];
			jump[dim] = bookmark->jumps[dim];
		}
		depth = stackSize-1;
	}
}

StorageCpu::Processor::Processor(const Processor &other) :
	ConstantRepeater(other), value(other.value), vkey(other.vkey), depth(other.depth), prevDepth(other.prevDepth), stackSize(other.stackSize),
	pFilter(other.pFilter ? filterKey : 0), pageList(other.pageList), longJumps(other.longJumps), pos(other.pos), currentPos(other.currentPos),
	endReached(other.endReached), endp(other.endp), before(other.before), pstorage(other.pstorage), storage(other.storage),
	sets(other.sets), setRange(other.setRange), setEndRange(other.setEndRange), mtCallback(0), indexptr(other.indexptr)
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
		ConstantRepeater::reset();
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
	offset[0] = -1;
	sets.clear();
	setRange.clear();
	setEndRange.clear();
	if (area) {
		sets.reserve(area->dimCount());
		setRange.reserve(area->dimCount());
		setEndRange.reserve(area->dimCount());

		size_t dim = 0;
		for (Area::ConstDimIter dimIt = area->dimBegin(); dimIt != area->dimEnd(); ++dimIt, ++dim) {
			sets.push_back((*dimIt).get());
			setRange.push_back((*dimIt)->rangeEnd());
			setEndRange.push_back((*dimIt)->rangeEnd());
			if ((*dimIt)->size() == 1) {
				setSingle[dim] = *(*dimIt)->begin();
			} else {
				setSingle[dim] = NO_IDENTIFIER;
			}
		}
	} else {
		memset(setSingle, 255, sizeof(setSingle));
	}
	ConstantRepeater::reset();
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
//		IdentifiersType thisKey(this->key, this->key + key.size());
		int cmp = CellValueStream::compare(key, vkey/*thisKey*/);
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
		*found = key == getKey();
	}
	return result;
}

void StorageCpu::Processor::moveBefore(const IdentifiersType *key)
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
		for (size_t i = 0; i < key->size(); i++) {
			if (key->at(i) != this->key[i]) {
				depth = (int)i - 1;
				break;
			}
		}
		pos = offset[depth + 1];
		this->key[depth + 1] -= 1;
	} else {
		setFilter(0, 0, false);
		pos = offset[0];
		this->key[0] -= 1;
	}
	nextIntern();
	before = false;
}

StorageCpu::Processor::~Processor()
{
}

size_t StorageCpu::Processor::getPositionJumpNOP() const
{
	size_t epos = getPosition();

	uint32_t pageSize = pageList->maxPageSize();
	uint32_t epage = (uint32_t)(epos / pageSize);
	uint32_t epagepos = epos % pageSize;

	const vector<Slim<uint8_t>::PSlimPage> &procPages = pageList->getPages();
	Slim<uint8_t>::CPSlimPage page = procPages[epage];
	uint32_t count = page->count();

	if (epage < procPages.size() - 1 || epagepos < count) { //epos is not the end of storage
		uint32_t i;
		for (i = epagepos; i < count; ++i) {
			if ((*page)[i] == NOP) {
				epos++;
			} else {
				break;
			}
		}
		if (i == count) {
			epage++;
			if (epage < procPages.size()) {
				page = procPages[epage];
				count = page->count();
				for (i = 0; i < count; ++i) {
					if ((*page)[i] == NOP) {
						epos++;
					} else {
						break;
					}
				}
			}
		}
		if (i == count) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageCpu::Processor::getPositionJumpNOP - empty page");
		}
	}
	return epos;
}

StorageCpu::Bookmark StorageCpu::Processor::getBookmark() const
{
	Bookmark bookmark;
	bookmark.position = currentPos;

	int maxDim = min(depth+2, stackSize);
	bookmark.jumps.reserve(maxDim);
	bookmark.key.reserve(maxDim);
	for (int dim = 0; dim < maxDim; dim++) {
		bookmark.key.push_back(key[dim]);
		bookmark.jumps.push_back(max(jump[dim], currentPos));
	}
	return bookmark;
}

void StorageCpu::Processor::indexStorage(set<Bookmark> &index, const set<Bookmark> &oldIndex)
{
	indexptr = &index;
	buildIndex(oldIndex);
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

size_t StorageCpu::getJump(const uint8_t *p, const vector<size_t> &longJumps)
{
	uint32_t distance;
	memcpy((uint8_t *)&distance, p, 4);
	if (distance >= LONG_JUMP_START_VALUE) {
		size_t longJump = longJumps[distance - LONG_JUMP_START_VALUE];
		if (!longJump) {
			Logger::debug << "Long Jump distance 0 index: " << (int)(distance - LONG_JUMP_START_VALUE) << endl;
		}
		return longJump;
	} else {
		return (size_t)distance;
	}
}

void StorageCpu::Writer::setJump(uint8_t *p, size_t jumpPosition, size_t jumpValue)
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
			ds.longJumps[shortDistance - LONG_JUMP_START_VALUE] = jumpValue;
//			if (ds.longJumps[shortDistance - LONG_JUMP_START_VALUE].first != jumpPosition) {
//				Logger::debug << "StorageCpu saved longJump position differs " << ds.longJumps[shortDistance - LONG_JUMP_START_VALUE].first << " " << jumpPosition << endl;
//			}
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
			// long -> short
			Logger::debug << "StorageCpu converting longJump to shortJump " << ds.longJumps[shortDistance - LONG_JUMP_START_VALUE] << " -> " << jumpValue << endl;
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
			bookmark = ds.endBookMark();
			bookmark.position = position;
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
		size_t newPos = 0;
		if (pageAdded && position) {
			if (addBookmark) {
				// new page -> save bookmark
				ds.index->insert(bookmark);
			}
			newPos = position;
		} else {
			newPos = sizeBefore;
		}
		if (ds.ccInfo.step == 2) {
			if (pageAdded) {
				ds.ccInfo.clear(true);
				ds.ccInfo.step = 0;
			} else {
				ds.ccInfo.build[newPos / STORAGE_PAGE_SIZE] = true;
				ds.ccInfo.newValueNewPos = newPos;
				ds.ccInfo.nextValueNewPos = sv.size();
				if (newPos + buffer.size() != sv.size()) {
					Logger::error << "last pos + buffer.size != storage size, rebuilding storage" << endl;
					ds.ccInfo.clear(true);
					ds.ccInfo.step = 0;
				}
			}
		} else if (ds.ccInfo.step == 3) {
			ds.ccInfo.build[newPos / STORAGE_PAGE_SIZE] = true;
			ds.ccInfo.nextValueNewPos = newPos;
		}
		if (nopsAdded) {
			for (uint8_t dimension = testDepth; dimension < dimensions; dimension++) {
				ds.endStack[dep].iOffset += nopsAdded;
				ds.endStack[dep].pOffset = NULL; // probably not necessary
				dep--;
			}
			for (size_t *currentOffset = oldOffsets; currentOffset < oldOffset; currentOffset++) {
				uint8_t *p = &sv[*currentOffset];
				if (*p & JUMP) {
					setJump(p+1, *currentOffset+1, StorageCpu::getJump(p+1, ds.longJumps)+nopsAdded);
//					size_t distance;
//					memcpy((uint8_t *)&distance, p + 1, 4);
//					if (distance >= LONG_JUMP_START_VALUE) {
//						size_t &longDistance = ds.longJumps[distance - LONG_JUMP_START_VALUE];
//						longDistance += nopsAdded;
//					} else {
//						distance += nopsAdded;
//						memcpy(p + 1, (uint8_t *)&distance, 4);
//					}
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
				setJump(si->pOffset+1, si->iOffset+1, currentSize - si->iOffset);
//				size_t distance = currentSize - si->iOffset;
//				uint32_t shortDistance;
//				if (distance >= LONG_JUMP_START_VALUE) {
//					memcpy((uint8_t *)&shortDistance, si->pOffset + 1, 4);
//					if (shortDistance >= LONG_JUMP_START_VALUE) {
//						// update longJump
//						ds.longJumps[shortDistance - LONG_JUMP_START_VALUE] = distance;
//					} else {
//						// create new longJump
//						ds.longJumps.push_back(distance);
//						shortDistance = (uint32_t)ds.longJumps.size();
//						memcpy(si->pOffset + 1, (uint8_t *)&shortDistance, 4);
//					}
//				} else {
//					shortDistance = distance;
//					memcpy(si->pOffset + 1, (uint8_t *)&shortDistance, 4);
//				}
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
			movePages(p, key);
			if (ds.ccInfo.step == 1) {
				ds.ccInfo.nextValueOldPos = p->getPositionJumpNOP();
			}
			return p->next();
		}
	} else {
		movePages(p, 0);
		return false;
	}
}

void StorageCpu::Writer::movePages(Processor *p, const IdentifiersType *key)
{
	size_t spos = p->getPosition();
	vector<ElemRestriction> sstack = p->getStack(false);
	IdentifiersType firstkey = p->getKey();
	double val = p->getValue().getNumeric();

	p->moveBefore(key);
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
		uint32_t spage = (uint32_t)(spos / pageSize);
		uint32_t spagepos = spos % pageSize;
		uint32_t epage = (uint32_t)(epos / pageSize);
		uint32_t epagepos = epos % pageSize;

		size_t currSize = ds.pageList->size();

		vector<Slim<uint8_t>::PSlimPage> &pages = ds.pageList->getPages();
		const vector<Slim<uint8_t>::PSlimPage> &procPages = p->pageList->getPages();
		uint32_t nopcount = 0;
		Slim<uint8_t>::PSlimPage pagelast = pages[pages.size() - 1];
		nopcount = pagelast->endSpace();
		pagelast->fillRest(NOP);
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
			if (i == spage || i == epage) {
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
					size_t distance = StorageCpu::getJump(&svr[sstack[i].iOffset + 1], p->longJumps);

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

StringStorageCpu::StringStorageCpu(PPathTranslator pathTranslator) : StorageCpu(pathTranslator), strings(new StringVector()), pushed(0), cleared(0), onlyDouble(false), stringMap(0)
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
	Processor reader(*this, pageList.get(), longJumps, PPathTranslator()); // translator not needed in this processor
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
		PCellStream cs = getCellValues(area, 0);
		while (cs->next()) {
			cs->setValue(CellValue::NullString);
		}
	}
}

bool StringStorageCpu::setCellValue(CPPlanNode plan, PEngineBase engine)
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

	strings.reset(new StringVector());
	strings->load(file);
	if (!strings->pageCount()) {
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

	Processor p(*this, pageList.get(), longJumps, CPArea(), 0);

	if (changedCells) {
		PCellStream cells = dynamic_cast<ICellMapStream *>(changedCells.get())->getValues();
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
	Processor p(*this, pageList.get(), longJumps, CPArea(), 0);
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

CellMapProcessor::CellMapProcessor(PEngineBase engine, CPPlanNode node) : area(node->getArea()), dimCount(-1)
{
	const CellMapPlanNode *pn = static_cast<const CellMapPlanNode *>(node.get());
	PStorageBase storage = engine->getStorage(pn->getStorageId());
	StorageCpu *st = dynamic_cast<StorageCpu *>(storage.get());
	if (st) {
		PDoubleCellMap changedCells = st->getChangedCells();
		if (changedCells) {
			valueStream = dynamic_cast<ICellMapStream *>(changedCells.get())->getValues();
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

StorageCpu::CellChangeInfo::CellChangeInfo() : maxSize(0), newToOld(0), build(0), step(0), newValueOldPos(0), newValueNewPos(0),
	nextValueOldPos(0), nextValueNewPos(0), oldLastPos(0), newLastPos(0), oldepage(0), newepage(0), diff1(0), diff2(0)
{
}

StorageCpu::CellChangeInfo::~CellChangeInfo()
{
	clear(true);
}

void StorageCpu::CellChangeInfo::init(uint32_t pageCount)
{
	if (maxSize < pageCount + 3) { //at most 3 new pages
		clear(true);
		maxSize = pageCount + 10; // to have a buffer
		newToOld = new uint32_t[maxSize];
		build = new char[maxSize];
	} else {
		clear(false);
	}
	memset(newToOld, 0, maxSize * sizeof(uint32_t));
	memset(build, 0, maxSize * sizeof(char));
	step = 1;
}

void StorageCpu::CellChangeInfo::clear(bool del)
{
	if (del) {
		delete []newToOld;
		newToOld = NULL;
		delete []build;
		build = NULL;
		maxSize = 0;
	}
	if (step) {
		step = 0;
		newKey.clear();
		newValueOldPos = 0;
		newValueNewPos = 0;
		nextValueOldPos = 0;
		nextValueNewPos = 0;
		oldLastPos = 0;
		newLastPos = 0;
		oldepage = 0;
		newepage = 0;
		diff1 = 0;
		diff2 = 0;
	}
}

uint64_t StorageCpu::getLastDeletionCount()
{
	uint64_t d = delCount;
	delCount = 0;
	return d;
}

}
