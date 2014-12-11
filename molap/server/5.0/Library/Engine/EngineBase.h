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
 * 
 *
 */

#ifndef OLAP_ENGINE_BASE_H
#define OLAP_ENGINE_BASE_H 1

#include "palo.h"
#include "Engine/Area.h"
#include "Olap/Commitable.h"
#include "Olap/CommitableList.h"
#include "Exceptions/ErrorException.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief "splash mode" for setting numeric values in aggregations
///
/// DISABLED: do not set the value<br>
/// DEFAULT:  <br>
///           1. value = 0.0 <br>
///              clears all base path cells<br>
///           2. value <> 0.0 and old_value = 0.0<br>
///              compute a "splash value" and distribute this value to all
///              base path cells<br>
///           3. value <> 0.0 and old_value <> 0.0<br>
///              compute a scale factor and recalculate all base cells<br>
/// SET_BASE: sets all base path elements to the same value<br>
/// ADD_BASE: adds value to all base path elements
////////////////////////////////////////////////////////////////////////////////

enum SplashMode {
	DISABLED, DEFAULT, SET_BASE, ADD_BASE
};

class CellValue : public std::string {
public:
	CellValue(bool isNumeric = true);
	CellValue(const string &str);
	CellValue(double value);
	CellValue(double value, const string &str);
	CellValue(ErrorException::ErrorType type);

	CellValue &operator=(const CellValue &val);
	CellValue &operator=(const string &str);
	CellValue &operator=(const char *str);
	CellValue &operator=(double val);
	bool operator==(const CellValue& b) const;
	bool operator!=(const CellValue& b) const;

	double getNumeric() const;
	ErrorException::ErrorType getError() const;
	bool isNumeric() const;
	bool isString() const;
	bool isError() const;
	bool isEmpty() const;
	void setRuleId(IdentifierType id);
	IdentifierType getRuleId() const;
	string toString() const;

	static const CellValue NullString;
	static const CellValue NullNumeric;
	static const CellValue MarkerValue;
private:
	double value;
	bool isNum;
	bool isEmp;
	union {
		IdentifierType ruleId;
		SplashMode splashMode;
	};
	uint16_t status;
	ErrorException::ErrorType err;
};

class SERVER_CLASS StorageList : public CommitableList {
public:
	StorageList(const PIdHolder &newidh) : CommitableList(newidh, true) {}
	StorageList(const StorageList &l) : CommitableList(l) {};
	virtual PCommitableList createnew(const CommitableList& l) const;
};
typedef boost::shared_ptr<StorageList> PStorageList;
typedef boost::shared_ptr<const StorageList> CPStorageList;

class SERVER_CLASS ValueCache;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine plan node class
///
/// OLAP Data Engine plan node class
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS PlanNode {
public:
	PlanNode(PlanNodeType type, CPArea area, const CellValue *defaultValue, const vector<PPlanNode> &children, ValueCache *cache, CPCube cacheCube, double constValue = 0.0);
	virtual ~PlanNode() {}
	PlanNodeType getType() const;
	string getTypeString() const;
	void setArea(CPArea area);
	CPArea getArea() const;
	void setDefaultValue(const CellValue *defaultValue);
	const CellValue *getDefaultValue() const;
	const vector<PPlanNode> &getChildren() const;
	virtual string toXML() const;
	ValueCache *getCache() const {return cache;}
	CPCube getCacheCube() const {return cacheCube;}
	double getConstValue() const {return constValue;}
	void setChildren(const vector<PPlanNode> &children) {this->children = children;}
	void setRuleId(IdentifierType ruleId) {this->ruleId = ruleId;}
	IdentifierType getRuleId() const {return ruleId;}
	bool skipEmpty() const {return !defaultValue;}
protected:
	virtual string getXMLAttributes() const;
	virtual string getXMLContent() const;
	PlanNodeType type;
	CPArea area;
	const CellValue *defaultValue;
	CellValue defaultValueCopy;
	vector<PPlanNode> children;
	ValueCache *cache;
	CPCube cacheCube;
	double constValue;
	IdentifierType ruleId;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Interface of asynchronous call result
///
/// Interface of asynchronous call result
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS AsyncResult
{
public:
	virtual ~AsyncResult() {}
	virtual void wait() = 0; // blocking - wait for async operation to be executed
			// Throws an exception if anything failed in the async function
};
typedef boost::shared_ptr<AsyncResult> PAsyncResult;
typedef vector<PAsyncResult> AsyncResults;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine storage base class
///
/// OLAP Data Engine storage base class
////////////////////////////////////////////////////////////////////////////////
class StorageBase;
typedef boost::shared_ptr<StorageBase> PStorageBase;
typedef boost::shared_ptr<const StorageBase> CPStorageBase;

class SERVER_CLASS StorageBase : public Commitable {
public:
	enum OperationType {
		SET = 0, MULTIPLY_EXISTING, ADD_ALL, SET_BASE
	};

	StorageBase(PPathTranslator pathTranslator) : Commitable(""), pathTranslator(pathTranslator) {}
	virtual PCellStream getCellValues(CPArea area, const CellValue *defaultValue) = 0;
	virtual bool setCellValue(PCellStream stream) = 0;
	virtual void setCellValue(CPArea area, const CellValue &value, OperationType opType) = 0;
	virtual bool setCellValue(CPPlanNode plan, PEngineBase engine) = 0;
	virtual void setCellValue(CPPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType) = 0;

    virtual void rightshiftBin(uint32_t binIdx, uint32_t bitPosition, uint32_t numBits, map<PGpuDevice, vector<PGpuPage> > gdpMap = map<PGpuDevice, vector<PGpuPage> > ()){};
    virtual void rightshiftBins(uint32_t firstBin, uint32_t numBinsToChange, vector<uint32_t> numBitsToMove, vector<uint32_t> numEmptyBits, bool addNewBin, uint32_t bitPosition, uint32_t numBits, uint32_t lastDimResizedBits){};

	// Asynchronous calls for splashing
	virtual PAsyncResult setCellValueBegin(PCellStream stream) {
        setCellValue(stream); 
        return PAsyncResult(new FakeAsyncResult());
    }

	virtual PAsyncResult setCellValueBegin(CPArea area, const CellValue &value, OperationType opType) {
        setCellValue(area, value, opType); 
        return PAsyncResult(new FakeAsyncResult());
    }

	virtual PAsyncResult setCellValueBegin(CPPlanNode plan, PEngineBase engine) {
        setCellValue(plan, engine);
        return PAsyncResult(new FakeAsyncResult());
    }

    virtual PAsyncResult setCellValueBegin(CPPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType) {
        setCellValue(plan, engine, area, value, opType);
        return PAsyncResult(new FakeAsyncResult());
    }

	virtual PCellStream commitChanges(bool checkLocks, bool add, bool disjunctive) = 0;
	virtual size_t valuesCount() const = 0;
	//only for legacy calculation
	virtual CellValue getCellValue(const IdentifiersType &key) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "StorageBase::getCellValue not implemented");
	}
	virtual void updateOld(CPStorageBase o) {
		checkCheckedOut();
		old = o->old ? o->old : o;
	}
	void updatePathTranslator(PPathTranslator pt){
			checkCheckedOut();
			pathTranslator = pt;
	}
protected:
	StorageBase(const StorageBase &base) : Commitable(base), pathTranslator(base.pathTranslator) {}
	virtual ~StorageBase() {}
	PPathTranslator pathTranslator;
private:
	class FakeAsyncResult : public AsyncResult
	{
	public:
		virtual ~FakeAsyncResult() {}
		virtual void wait() {}
	};
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine source node class
///
/// OLAP Data Engine Source node base class. Implements data filter on storage.
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS SourcePlanNode : public PlanNode {
public:
	SourcePlanNode(IdentifierType storageId, CPArea area, const CellValue *defaultValue);
	virtual ~SourcePlanNode() {}
	IdentifierType getStorageId() const;
private:
	virtual string getXMLAttributes() const;
	IdentifierType storageId;
};

typedef boost::shared_ptr<SourcePlanNode> PSourcePlanNode;
typedef boost::shared_ptr<const SourcePlanNode> CPSourcePlanNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine aggregation node class
///
/// OLAP Data Engine Aggregation node base class.
/// Implements aggregation of children nodes.
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS AggregationPlanNode : public PlanNode {
public:
	enum AggregationType {
		SUM = 0, AVG, COUNT, MAX, MIN
	};
	AggregationPlanNode(CPArea area, const CellValue *defaultValue, const vector<PPlanNode> &children, CPAggregationMaps aggregationMaps, ValueCache *cache, CPCube cacheCube, AggregationType aggrType, uint64_t maxCount = 0);
	virtual ~AggregationPlanNode() {}
	CPAggregationMaps getAggregationMaps() const {
		return aggregationMaps;
	}
	AggregationType getAggregationType() const {
		return aggrType;
	}
	uint64_t getMaxCount () const {return maxCount;}
private:
	virtual string getXMLContent() const;
	CPAggregationMaps aggregationMaps;
	AggregationType aggrType;
	uint64_t maxCount;
};


////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine transformation plan node class
///
/// OLAP Data transformation node
/// Implements transformation of child nodes into required targets
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS TransformationPlanNode : public PlanNode {
public:
	TransformationPlanNode(CPArea area, PPlanNode &child, const IdentifiersType &transformationMask, const SetMultimaps &setMultiMaps, double factor);
	virtual ~TransformationPlanNode() {}
	const IdentifiersType &getTransformationMask() const {return transformationMask;}
	const SetMultimaps *getSetMultiMaps() const {return pSetMultiMaps;}
	double getFactor() const {return factor;}
private:
	virtual string getXMLContent() const;
	IdentifiersType transformationMask;
	SetMultimaps setMultiMaps;
	SetMultimaps *pSetMultiMaps;
	double factor;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine cache plan node class
///
/// OLAP Cache node
/// Stores actual cache
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS CachePlanNode : public PlanNode {
public:
	CachePlanNode(CPArea area, const CellValue *defaultValue, PStorageCpu cacheStorage, IdentifierType ruleId) : PlanNode(CACHE, area, defaultValue, vector<PPlanNode>(), 0, CPCube()), cacheStorage(cacheStorage), ruleId(ruleId) {}
	virtual ~CachePlanNode() {}
	const PStorageCpu &getCacheStorage() const {return cacheStorage;}
	IdentifierType getRuleId() const {return ruleId;}
protected:
	string getXMLAttributes() const;
private:
	PStorageCpu cacheStorage;
	IdentifierType ruleId;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine cache plan node class
///
/// OLAP Cache node
/// Stores actual cache
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS QueryCachePlanNode : public PlanNode {
public:
	QueryCachePlanNode(CPArea area, const CellValue *defaultValue, IdentifierType ruleId, CPCube cube) : PlanNode(QUERY_CACHE, area, defaultValue, vector<PPlanNode>(), 0, CPCube()), ruleId(ruleId), cube(cube) {}
	virtual ~QueryCachePlanNode() {}
	IdentifierType getRuleId() const {return ruleId;}
	CPCube getCube() const {return cube;}
private:
	IdentifierType ruleId;
	CPCube cube;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine double cell map node class
///
/// OLAP Data Engine DoubleCellMap node
/// Stores cell map of doubles
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS CellMapPlanNode : public PlanNode {
public:
	CellMapPlanNode(PDoubleCellMap cellMap);
	CellMapPlanNode(IdentifierType storageId, CPArea area);
	virtual ~CellMapPlanNode() {}
	PDoubleCellMap getCellMap() const {return cellMap;}
	IdentifierType getStorageId() const {return storageId;}
private:
	PDoubleCellMap cellMap;
	IdentifierType storageId;
};

typedef boost::shared_ptr<CellMapPlanNode> PCellMapPlanNode;
typedef boost::shared_ptr<const CellMapPlanNode> CPCellMapPlanNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine cell rights plan node
///
/// OLAP Data Engine cell rights plan node
/// calculates cell rights
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS CellRightsPlanNode : public PlanNode {
public:
	CellRightsPlanNode(PCubeArea cubeArea, bool forPropertyCube) : PlanNode(CELL_RIGHTS, cubeArea, 0, vector<PPlanNode>(), 0, CPCube()), cubeArea(cubeArea), forPropertyCube(forPropertyCube) {}
	virtual ~CellRightsPlanNode() {}
	PCubeArea getCubeArea() const {return cubeArea;}
	bool isForPropertyCube() const {return forPropertyCube;}
private:
	PCubeArea cubeArea;
	bool forPropertyCube;
};

typedef boost::shared_ptr<CellRightsPlanNode> PCellRightsPlanNode;
typedef boost::shared_ptr<const CellRightsPlanNode> CPCellRightsPlanNode;


////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine base class
///
/// abstract OLAP Data Engine base class
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS EngineBase : public Commitable {
public:
	enum Type {
		ANY = -1, CPU = 0, GPU = 1
	};
	enum StorageType {
		Numeric, String, Marker
	};

	EngineBase(const string &engineName, Type type);
	EngineBase(const EngineBase &s);
	virtual ~EngineBase();

	virtual bool merge(const CPCommitable &o, const PCommitable &p);

	virtual PStorageBase getCreateStorage(IdentifierType &id, PPathTranslator pathTranslator, StorageType type) = 0;	// create new storage of given type or check out existing storage for writing
	virtual PStorageBase getStorage(IdentifierType id) const;	// returns a storage of given id
	virtual void deleteStorage(IdentifierType id);				// removes and releases
	virtual void recreateStorage(IdentifierType id, PPathTranslator pathTranslator, StorageType type); // removes a storage of given id and creates empty storage

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new processor providing data
	////////////////////////////////////////////////////////////////////////////////
	virtual PCellStream createProcessor(CPPlanNode node, bool useCache = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief test of plan support
	////////////////////////////////////////////////////////////////////////////////
	virtual bool isPlanSupported(CPPlanNode node) const;

	PStorageList getStorageList(bool write);
protected:
	static PIdHolder storageIdHolder;
	PStorageList storageList;
	Type type;
};

class SERVER_CLASS EngineList : public CommitableList {
public:
	EngineList(const PIdHolder &newidh) : CommitableList(newidh) {}
	EngineList() {}
	EngineList(const EngineList &l) : CommitableList(l) {};
	virtual PCommitableList createnew(const CommitableList& l) const {
		return PCommitableList(new EngineList(dynamic_cast<const EngineList &>(l)));
	};
};

typedef boost::shared_ptr<EngineList> PEngineList;
typedef boost::shared_ptr<const EngineList> CPEngineList;

}

#endif
