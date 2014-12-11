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

#ifndef OLAP_STORAGE_CPU_H_
#define OLAP_STORAGE_CPU_H_

#include "EngineCpu.h"
#include "Collections/SlimVector.h"
#include "Collections/CellMap.h"
#include "Collections/StringVector.h"

namespace palo {

class HashValueStorage;

template<typename TIN, typename TOUT> bool canCompress(TIN valIn, TOUT &valOut)
{
	valOut = (TOUT)valIn;
	return (TIN)valOut == valIn;
}

enum Instruction {
	JUMP = 1,
	ELEMENT8 = 1 << 1,
	ELEMENT16 = 2 << 1,
	ELEMENT32 = 3 << 1,
	ELEMENTMASK = ELEMENT32,
	VALUEU8 = 1 << 3,
	VALUE32 = 2 << 3,
	VALUE64 = 3 << 3,
	VALUEF32 = 4 << 3,
	VALUEMASK = 7 << 3,
	NOP = 0xFF
};

struct INSTR {
	int elementIdType;
	int valueType;
};

const uint32_t DIM_MAX = 256;
const uint32_t STORAGE_PAGE_SIZE = 16 * 1024;
#define NO_OFFSET ((size_t) ~(size_t)0)

class StorageCpu : public StorageBase {
private:
	struct ElemRestriction {
		ElemRestriction(uint32_t elementId = 0, size_t offset = 0) : elementId(elementId), iOffset(offset), pOffset(NULL) {};
		uint32_t elementId;
		size_t iOffset;
		uint8_t *pOffset;
	};

	class Writer;
public:
	class Processor;
	class ProcessorCallback {
	public:
		virtual ~ProcessorCallback() {}
		virtual void readCallback() = 0;
	};

	struct Bookmark {
		Bookmark() : position(0)  {}
		bool operator<(const Bookmark &o) const {
			return compare(o) < 0;
		}
		bool operator==(const Bookmark &o) const {
			return position == o.position && key == o.key && jumps == o.jumps;
		}
		bool operator!=(const Bookmark &o) const {
			return !operator==(o);
		}
		int compare(const Bookmark &o) const {
			IdentifiersType::const_iterator i1 = key.begin();
			IdentifiersType::const_iterator i2 = o.key.begin();
			for (;i1 != key.end() && i2 != o.key.end(); ++i1, ++i2) {
				if (*i1 < *i2) {
					return -1;
				} else if (*i1 > *i2) {
					return 1;
				}
			}
			if (i1 == key.end() && i2 == o.key.end()) {
				return 0;
			} else if (i1 == key.end()) {
				return 1;
			} else {
				return -1;
			}
		}
		size_t position;
		IdentifiersType key;
		vector<size_t> jumps;
	};

	struct CellChangeInfo {
	public:
		CellChangeInfo();
		~CellChangeInfo();
		void init(uint32_t pageCount);
		void clear(bool del);

		uint32_t maxSize;
		uint32_t *newToOld;
		char *build;
		uint32_t step;
		IdentifiersType newKey;
		size_t newValueOldPos;
		size_t newValueNewPos;
		size_t nextValueOldPos;
		size_t nextValueNewPos;
		size_t oldLastPos;
		size_t newLastPos;
		uint32_t oldepage;
		uint32_t newepage;
		int diff1;
		int diff2;
	};

	class Processor : public ConstantRepeater {
		friend class Writer;
		friend class AggregationProcessorWorker;
		friend class AggregationProcessorMT;
		friend class AggregationProcessor;
	public:
		Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, vector<size_t> &longJumps, CPArea area, const CellValue *defaultValue, const size_t endPos = 0, const Bookmark *bookmark = 0);
		Processor(StorageCpu &storage, const SlimVector<uint8_t> *pageList, vector<size_t> &longJumps, PPathTranslator pathTranslator, const size_t endPos = 0, const Bookmark *bookmark = 0);
		Processor(const Processor &other);

		void setFilter(const uint32_t *filter, const int dimensions, bool changeDepth);
		virtual ~Processor();

		bool end() const {
			return endReached;
		};

		size_t getPosition() const {
			return pos;
		}

		size_t getPositionJumpNOP() const;

		size_t getEndPosition() const {
			return endp;
		}

		Bookmark getBookmark() const;
		void indexStorage(set<Bookmark> &index, const set<Bookmark> &oldIndex);

		vector<StorageCpu::ElemRestriction> getStack(bool previous);

		// Current value
		double value;
		CellValue cellvalue;
		IdentifiersType vkey;

		// object state
		int depth;
		int prevDepth;
		uint32_t prevKey[DIM_MAX];
		uint32_t key[DIM_MAX];
		size_t offset[DIM_MAX];
		size_t prevOffset[DIM_MAX];
		size_t jump[DIM_MAX];

		friend ostream& operator<<(ostream& ostr, const StorageCpu::Processor& sr);

		// CellStream interface
		virtual bool next();
		virtual void setValue(const CellValue &value);
		virtual const CellValue &getValue();
		virtual double getDouble();
		virtual const IdentifiersType &getKey() const;
		virtual void reset();
		virtual bool move(const IdentifiersType &key, bool *found);
		void moveBefore(const IdentifiersType *key);
		bool nextValid(size_t pos);
	private:
		bool nextIntern();
		void buildIndex(const set<Bookmark> &oldIndex);
		int compareKey(const IdentifierType *key1, const IdentifierType *key2, int depth) {
			for (int i = 0; i <= depth; ++i) {
				if (key1[i] < key2[i]) {
					return -1;
				} else if (key1[i] > key2[i]) {
					return 1;
				}
			}
			return 0;
		}

		int stackSize;
		uint32_t filterKey[DIM_MAX];
		uint32_t *pFilter;
		const SlimVector<uint8_t> *pageList;
		const vector<size_t> &longJumps;
		size_t pos;
		size_t currentPos;
		bool endReached;
		size_t endp;
		bool before;
		PCommitable pstorage;
		StorageCpu &storage;

		vector<const Set *> sets;
		vector<Set::range_iterator> setRange;
		vector<Set::range_iterator> setEndRange;
		IdentifierType setSingle[DIM_MAX];
		ProcessorCallback *mtCallback;
		set<Bookmark> *indexptr;

		void aggregate(HashValueStorage *storage, const AggregationMap **parentMaps);
	};

	StorageCpu(PPathTranslator pathTranslator);
	virtual ~StorageCpu();
	friend ostream& operator<<(ostream& ostr, const StorageCpu& ds);
	void clear(CPCommitable oldPageList);
	void buildIndex();

	// StorageBase
	virtual PCellStream getCellValues(CPArea area, const CellValue *defaultValue);
	virtual CellValue getCellValue(const IdentifiersType &key);

	virtual bool setCellValue(PCellStream stream);
	virtual void setCellValue(CPArea area, const CellValue &value, OperationType opType);
	virtual bool setCellValue(CPPlanNode plan, PEngineBase engine);
    virtual void setCellValue(CPPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType){}

	virtual PCellStream commitChanges(bool checkLocks, bool add, bool disjunctive);
	PCellStream commitExternalChanges(bool checkLocks, PCellStream changes, bool singleValue, bool add);
	size_t valuesCount() const {
		return valCount;
	}
	uint64_t getLastDeletionCount();

	// Commitable
	bool merge(const CPCommitable &o, const PCommitable &p);
	PCommitable copy() const;

	virtual void load(FileReader *file, uint32_t fileVersion);
	virtual void save(FileWriter *file) const;

	static string keytoString(const IdentifiersType& key);

	virtual void convertToCellValue(CellValue &value, double d) const {
		value = d;
	}

	virtual PDoubleCellMap getChangedCells() const;
	virtual void updateOld(CPStorageBase o);
	static size_t getJump(const uint8_t *p, const vector<size_t> &longJumps);

protected:
	StorageCpu(const StorageCpu &storage);
	virtual void setCellValue(const IdentifiersType &key, const CellValue &val);
	virtual double convertToDouble(const CellValue &value) {
		return value.getNumeric();
	}
	virtual bool isNumeric() const {
		return true;
	}

private:
	static const char FILE_TAG[];

	vector<ElemRestriction> endStack;
	void setStack(vector<ElemRestriction> &newStack);
	bool validate(bool thorough);
	Bookmark endBookMark();
	void convertIndex(size_t fromPosition);

	size_t valCount;
	size_t emptySpace;
	boost::shared_ptr<unordered_map<uint32_t, uint32_t> > nopPages;
	boost::shared_ptr<unordered_map<uint32_t, uint32_t> > oldnopPages;

	boost::shared_ptr<set<Bookmark> > index;
	boost::shared_ptr<set<Bookmark> > oldIndex;
	CellChangeInfo ccInfo;
	uint64_t delCount;
	const static size_t LONG_JUMP_START_VALUE = 4000000000ul; // 1; //80000000000ul; //16 * 1024;

protected:
	bool indexEnabled;
	PDoubleCellMap changedCells;
	vector<PPlanNode> changeNodes;
	boost::shared_ptr<SlimVector<uint8_t> > pageList;
	vector<size_t> longJumps;

private:
	static bool compareDouble(double d1, double d2);

	class Writer {
	public:
		Writer(StorageCpu &ds, bool addBookmark) : ds(ds), depth(-1), addBookmark(addBookmark) {};
		void push_back(const uint32_t *key, int dimensions, double value);
		void push_back(const vector<uint32_t> &key, double value);
		bool writeUntil(Processor *p, const IdentifiersType *key, bool rebuild);
	private:
		bool writeUntilCopy(Processor *p, const IdentifiersType *key);
		bool writeUntilRebuild(Processor *p, const IdentifiersType *key);
		void movePages(Processor *p, const IdentifiersType *key);
		static void storeInstruction(uint32_t elementId, vector<ElemRestriction> &endStack, int &depth, vector<uint8_t> &buffer, bool doCompress, const double * const value = 0);
		void setJump(uint8_t *p, size_t jumpPosition, size_t jumpValue);
		StorageCpu &ds;
		int depth;
		friend class Processor;
		bool addBookmark;
	};
};

class StringStorageCpu : public StorageCpu {
public:
	StringStorageCpu(PPathTranslator pathTranslator);
	virtual ~StringStorageCpu();

	virtual CellValue getCellValue(const IdentifiersType &key);
	virtual bool setCellValue(PCellStream stream);
	virtual void setCellValue(CPArea area, const CellValue &value, OperationType opType);
	virtual bool setCellValue(CPPlanNode plan, PEngineBase engine);
	virtual PCellStream commitChanges(bool checkLocks, bool add, bool disjunctive);

	virtual void load(FileReader *file, uint32_t fileVersion);
	virtual void save(FileWriter *file) const;

	virtual double convertToDouble(const CellValue &value);
	virtual void convertToCellValue(CellValue &value, double d) const;
	virtual bool isNumeric() const {return false;}
	virtual void updateOld(CPStorageBase o);

	bool merge(const CPCommitable &o, const PCommitable &p);
	PCommitable copy() const;
	bool validateStrings(const string &prefix);

private:
	static const size_t MAX_MAP_SIZE = 1000;

	StringStorageCpu(const StringStorageCpu &st);
	void createStringMap();
	void destroyStringMap();
	bool rebuildStrings(StringVector *newStrings);
	double copyString(StringVector *newStrings, double d);

	PStringVector strings;
	size_t pushed;
	size_t cleared;
	bool onlyDouble;
	map<string, StringVector::StringId> *stringMap;
};

class MarkerStorageCpu : public StorageCpu {
public:
	MarkerStorageCpu(PPathTranslator pathTranslator) : StorageCpu(pathTranslator) {indexEnabled = false;}
	virtual void setCellValue(const IdentifiersType &key, const CellValue &val);
	bool setMarker(const IdentifiersType &key);
	virtual PDoubleCellMap getChangedCells() const {return changedCells;}

	virtual PCommitable copy() const;
};

class SERVER_CLASS CellMapProcessor : public CellValueStream {
public:
	CellMapProcessor(PEngineBase engine, CPPlanNode node);
	virtual ~CellMapProcessor() {}

	virtual bool next();
	virtual const CellValue &getValue();
	virtual double getDouble();
	virtual const IdentifiersType &getKey() const;
	virtual void reset();
	//virtual bool move(const IdentifiersType &key, bool *found); // TODO: -jj implement for better performance if needed

private:
	bool nextAreaKey(int32_t dim, const IdentifiersType &key);
	CPArea area;
	PCellStream valueStream;
	int32_t dimCount;
	vector<Area::ConstElemIter> areaKey;
};

}

#endif /* OLAP_STORAGE_CPU_H_ */
