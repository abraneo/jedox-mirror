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
 * 
 *
 */

#ifndef OLAP_ENGINE_BASE_H
#define OLAP_ENGINE_BASE_H 1

#include "palo.h"
#include "Engine/Area.h"
#include "Engine/Streams.h"
#include "Olap/Commitable.h"
#include "Olap/CommitableList.h"
#include "Exceptions/ErrorException.h"

namespace palo {

class FileWriter;

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
	bool operator<(const CellValue &b) const;

	double getNumeric() const;
	operator double() const;

	ErrorException::ErrorType getError() const;
	bool isNumeric() const;
	bool isString() const;
	bool isError() const;
	bool isEmpty() const;
	void setEmpty(bool empty) {isEmp = empty;}
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

	friend ostream& operator<<(ostream&, const CellValue&);
};

ostream& operator<<(ostream&, const CellValue&);

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
	PlanNode(PlanNodeType type, CPArea area, const vector<PPlanNode> &children, ValueCache *cache, CPCube cacheCube, double constValue = 0.0);
	virtual ~PlanNode() {}
	PlanNodeType getType() const;
	string getTypeString(bool code = false) const;
	void setArea(CPArea area);
	CPArea getArea() const;
	const vector<PPlanNode> &getChildren() const;
	virtual string toXML() const;
	ValueCache *getCache() const {return cache;}
	CPCube getCacheCube() const {return cacheCube;}
	double getConstValue() const {return constValue;}
	void setChildren(const vector<PPlanNode> &children) {this->children = children;}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	bool operator==(const PlanNode &b) const;
	virtual bool isEqual(const PlanNode &b) const {return true;}
protected:
	virtual string getXMLAttributes() const;
	virtual string getXMLContent() const;
	PlanNodeType type;
	CPArea area;
	vector<PPlanNode> children;
	ValueCache *cache;
	CPCube cacheCube;
	double constValue;
};

class SERVER_CLASS CompleteNodeInfo {
public:
	CompleteNodeInfo(const CellValue *defaultValue, IdentifierType ruleId)
	: defaultValue(defaultValue ? &defaultValueCopy : 0), defaultValueCopy(defaultValue ? *defaultValue : CellValue::NullNumeric), ruleId(ruleId) {}
	virtual ~CompleteNodeInfo() {}
	const CellValue *getDefaultValue() const {return defaultValue;}
	void setRuleId(IdentifierType ruleId) {this->ruleId = ruleId;}
	IdentifierType getRuleId() const {return ruleId;}
	bool isEqual(const CompleteNodeInfo &b) const {return ruleId == b.ruleId && ((!defaultValue && !b.defaultValue) || (defaultValue && b.defaultValue && *defaultValue == *b.defaultValue));}
	bool skipEmpty() const {return !defaultValue;}
protected:
	const CellValue *defaultValue;
	CellValue defaultValueCopy;
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
/// @brief OLAP processor base class
///
/// abstract OLAP processor base class
////////////////////////////////////////////////////////////////////////////////
class ProcessorBase;
typedef boost::shared_ptr<ProcessorBase> PProcessorBase;
typedef boost::shared_ptr<const ProcessorBase> CPProcessorBase;

class SERVER_CLASS  ProcessorBase : public CellValueStream, public boost::enable_shared_from_this<ProcessorBase> {
public:
	ProcessorBase(bool sorted, PEngineBase engine, bool engineLocked = true) : sorted(sorted), engine(engine), engineLocked(engineLocked) {}
	ProcessorBase(const ProcessorBase &other) : sorted(other.sorted), engine(other.engine), engineLocked(other.engineLocked) {}
	virtual ~ProcessorBase() {}
	bool isSorted() {return sorted;}
	bool isEngineLocked() {return engineLocked;}
	void setEngineLocked(bool locked) {engineLocked = locked;}
	PProcessorBase createProcessor(CPPlanNode node, bool sortedOutput, bool useCache = true);
	PEngineBase getEngine() {return engine;}
	static PEngineBase selectEngine(CPPlanNode node);
protected:
	bool sorted;
	PEngineBase engine;
	bool engineLocked;
};

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
	virtual ~StorageBase() {}
	virtual PProcessorBase getCellValues(CPArea area) = 0;
	virtual bool setCellValue(PCellStream stream) = 0;
	virtual void setCellValue(CPArea area, const CellValue &value, OperationType opType) = 0;
	virtual bool setCellValue(PPlanNode plan, PEngineBase engine) = 0;
	virtual void setCellValue(CPPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType) = 0;

	// Asynchronous calls for splashing
	virtual PAsyncResult setCellValueBegin(PCellStream stream) {
        setCellValue(stream); 
        return PAsyncResult(new FakeAsyncResult());
    }

	virtual PAsyncResult setCellValueBegin(CPArea area, const CellValue &value, OperationType opType) {
        setCellValue(area, value, opType); 
        return PAsyncResult(new FakeAsyncResult());
    }

	virtual PAsyncResult setCellValueBegin(PPlanNode plan, PEngineBase engine) {
        setCellValue(plan, engine);
        return PAsyncResult(new FakeAsyncResult());
    }

    virtual PAsyncResult setCellValueBegin(PPlanNode plan, PEngineBase engine, CPArea area, const CellValue &value, OperationType opType) {
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
	SourcePlanNode(IdentifierType storageId, CPArea area, uint64_t revision);
	virtual ~SourcePlanNode() {}
	IdentifierType getStorageId() const {return storageId;}
	uint64_t getRevision() const {return revision;}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
private:
	virtual string getXMLAttributes() const;
	IdentifierType storageId;
	uint64_t revision;
};

typedef boost::shared_ptr<SourcePlanNode> PSourcePlanNode;
typedef boost::shared_ptr<const SourcePlanNode> CPSourcePlanNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine aggregation node class
///
/// OLAP Data Engine Aggregation node base class.
/// Implements aggregation of children nodes.
////////////////////////////////////////////////////////////////////////////////

const int32_t NO_DFILTER = (int32_t)-1; //plan for a 'normal' aggregation (NOT for data filter)

class SERVER_CLASS AggregationPlanNode : public PlanNode {
public:
	enum AggregationType {
		SUM = 0, AVG, COUNT, MAX, MIN
	};
	AggregationPlanNode(CPArea area, const vector<PPlanNode> &children, CPAggregationMaps aggregationMaps, ValueCache *cache, CPCube cacheCube, AggregationType aggrType, uint64_t maxCount = 0, PCondition cond = PCondition(), int dfDimIndex = NO_DFILTER, double dfCellsPerElement = 0.0);
	virtual ~AggregationPlanNode() {}
	CPAggregationMaps getAggregationMaps() const {return aggregationMaps;}
	AggregationType getAggregationType() const {return aggrType;}
	string getFunctionTypeString(bool code = false) const;
	uint64_t getMaxCount () const {return maxCount;}
	CPCondition getCondition() const {return cond;}
	int getDimIndex() const {return dfDimIndex;}
	double getCellsPerElement() const {return dfCellsPerElement;}

	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;

private:
	virtual string getXMLAttributes() const;
	virtual string getXMLContent() const;
	CPAggregationMaps aggregationMaps;
	AggregationType aggrType;
	uint64_t maxCount;
	CPCondition cond;
	int dfDimIndex;
	double dfCellsPerElement;
};

class SERVER_CLASS QuantificationPlanNode : public PlanNode {
public:
	enum QuantificationType {
		ALL = 0, ANY_NUM, ANY_STR, EXISTENCE
	};
	QuantificationPlanNode(PCubeArea dfArea, CPCubeArea numericArea, const vector<PPlanNode> &children, QuantificationType quantType, PCondition cond, int dimIndex, bool isVirt, double cellsPerElement, bool calcRules, uint64_t maxCount);
	QuantificationPlanNode(PCubeArea dfArea, const vector<PPlanNode> &children, int dimIndex, bool isVirt, bool calcRules, uint64_t maxCount);
	virtual ~QuantificationPlanNode() {}
	CPCubeArea getNumericArea() const {return numericArea;}
	QuantificationType getQuantificationType() const {return quantType;}
	string getQuantificationTypeString(bool code = false) const;
	uint64_t getMaxCount () const {return maxCount;}
	CPCondition getCondition() const {return cond;}
	int getDimIndex() const {return dimIndex;}
	bool isVirtual() const {return isVirt;}
	double getCellCount() const {return cellsPerElement;}
	bool getCalcRules() const {return calcRules;}
	PCubeArea getFilteredArea() const {return dfArea;}

#ifdef ENABLE_GPU_SERVER
	CPAggregationMaps getAggregationMaps() const { return aggregationMaps; }
#endif

private:
	virtual string getXMLAttributes() const;
	virtual string getXMLContent() const;
	PCubeArea dfArea; // the whole area
	CPCubeArea numericArea; // only numeric cells
	QuantificationType quantType;
	CPCondition cond;
	int dimIndex;
	bool isVirt;
	double cellsPerElement;
	bool calcRules;
	uint64_t maxCount;

#ifdef ENABLE_GPU_SERVER
	CPAggregationMaps aggregationMaps;
#endif
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine transformation plan node class
///
/// OLAP Data transformation node
/// Implements transformation of child nodes into required targets
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS TransformationPlanNode : public PlanNode {
public:
	TransformationPlanNode(CPArea area, PPlanNode &child, const SetMultimaps &setMultiMaps, double factor, const vector<uint32_t> &dimensionMapping);
	virtual ~TransformationPlanNode() {}
	const SetMultimaps *getSetMultiMaps() const {return pSetMultiMaps;}
	double getFactor() const {return factor;}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
	const vector<uint32_t> &getDimMapping()  const {return dimensionMapping;}
	const dbID_cubeID &getSourceCubeId() const {return sourceCubeId;}
	void setSourceCubeId(const dbID_cubeID &sourceCubeId) {this->sourceCubeId = sourceCubeId;}
private:
	virtual string getXMLContent() const;
	SetMultimaps setMultiMaps;
	SetMultimaps *pSetMultiMaps;
	double factor;
	vector<uint32_t> dimensionMapping;
	dbID_cubeID sourceCubeId;
};


////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine Union plan node
///
/// OLAP Data merge node
/// Describes merge functionality into one result-set
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS UnionPlanNode : public PlanNode {
public:
	UnionPlanNode(CPArea area, const vector<PPlanNode> &children, IdentifierType ruleId) : PlanNode(UNION, area, children, 0, CPCube()), ruleId(ruleId) {}
	virtual ~UnionPlanNode() {}
	bool canHaveDuplicates() const;
	virtual bool isEqual(const PlanNode& b) const;
private:
	IdentifierType ruleId;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine Complete plan node
///
/// OLAP Data complete info
/// Adds missing values and/or sets rule information to produced data
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS CompletePlanNode : public PlanNode, public CompleteNodeInfo {
public:
	CompletePlanNode(CPArea area, PPlanNode child, const CellValue *defaultValue, IdentifierType ruleId)
	: PlanNode(COMPLETE, area, vector<PPlanNode>(1, child), 0, CPCube()), CompleteNodeInfo(defaultValue, ruleId) {}
	virtual ~CompletePlanNode() {}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
private:
	virtual string getXMLAttributes() const;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine RoundAndCorrect plan node
///
/// OLAP Data
/// Rounds the input to closest number divisible by specified number and
// continuously calculates correction
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS RoundCorrrectPlanNode : public PlanNode {
public:
	RoundCorrrectPlanNode(CPArea area, PPlanNode child, CPAggregationMaps aggregationMaps, double targetValue, double unitValue = 0) : PlanNode(ROUNDCORRECT, area, vector<PPlanNode>(1, child), 0, CPCube()), aggregationMaps(aggregationMaps), targetValue(targetValue), unitValue(unitValue) {}
	virtual ~RoundCorrrectPlanNode() {}
	CPAggregationMaps getAggregationMaps() const {
		return aggregationMaps;
	}
	double getTargetValue() const {return targetValue;}
	double getUnitValue() const {return unitValue;}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
private:
	//virtual string getXMLAttributes() const;
	CPAggregationMaps aggregationMaps;
	double targetValue;
	double unitValue;
};

typedef boost::shared_ptr<RoundCorrrectPlanNode> PRoundCorrrectPlanNode;
typedef boost::shared_ptr<const RoundCorrrectPlanNode> CPRoundCorrrectPlanNode;

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine cache plan node class
///
/// OLAP Cache node
/// Stores actual cache
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS CachePlanNode : public PlanNode, public CompleteNodeInfo {
public:
	CachePlanNode(CPArea area, const CellValue *defaultValue, PStorageCpu cacheStorage, IdentifierType ruleId)
	: PlanNode(CACHE, area, vector<PPlanNode>(), 0, CPCube()), CompleteNodeInfo(defaultValue, ruleId), cacheStorage(cacheStorage) {}
	virtual ~CachePlanNode() {}
	const PStorageCpu &getCacheStorage() const {return cacheStorage;}
	virtual bool isEqual(const PlanNode& b) const;
protected:
	string getXMLAttributes() const;
private:
	PStorageCpu cacheStorage;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine cache plan node class
///
/// OLAP Cache node
/// Stores actual cache
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS QueryCachePlanNode : public PlanNode, public CompleteNodeInfo {
public:
	QueryCachePlanNode(CPArea area, const CellValue *defaultValue, IdentifierType ruleId, CPCube cube)
	: PlanNode(QUERY_CACHE, area, vector<PPlanNode>(), 0, CPCube()), CompleteNodeInfo(defaultValue, ruleId), cube(cube) {}
	virtual ~QueryCachePlanNode() {}
	CPCube getCube() const {return cube;}
	virtual bool isEqual(const PlanNode& b) const;
private:
	CPCube cube;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP Engine constant node class
///
/// OLAP Data Engine Constant node
////////////////////////////////////////////////////////////////////////////////
class SERVER_CLASS ConstantPlanNode : public PlanNode, public CompleteNodeInfo {
public:
	ConstantPlanNode(CPArea area, const CellValue &value) : PlanNode(CONSTANT, area, vector<PPlanNode>(), 0, CPCube())
	, CompleteNodeInfo(value.isEmpty() ? 0 : &value, NO_RULE) {}
	virtual ~ConstantPlanNode() {}
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
protected:
	virtual string getXMLAttributes() const;
};

typedef boost::shared_ptr<ConstantPlanNode> PConstantPlanNode;
typedef boost::shared_ptr<const ConstantPlanNode> CPConstantPlanNode;

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
	virtual void write(FileWriter &w, CPArea parentArea) const;
	virtual bool isEqual(const PlanNode& b) const;
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
	CellRightsPlanNode(PCubeArea cubeArea, bool forPropertyCube) : PlanNode(CELL_RIGHTS, cubeArea, vector<PPlanNode>(), 0, CPCube()), cubeArea(cubeArea), forPropertyCube(forPropertyCube) {}
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
	virtual void deleteStorage(IdentifierType id);				// removes and releases a storage of given id
	virtual void recreateStorage(IdentifierType id, PPathTranslator pathTranslator, StorageType type); // removes a storage of given id and creates empty storage

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new processor providing data
	////////////////////////////////////////////////////////////////////////////////
	virtual PProcessorBase createProcessor(CPPlanNode node, bool sortedOutput, bool useCache = true);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief test of plan support
	////////////////////////////////////////////////////////////////////////////////
	virtual bool isPlanSupported(CPPlanNode node) const;

protected:
	PStorageList getStorageList(bool write);

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
