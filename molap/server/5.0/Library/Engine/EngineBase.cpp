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

#include "Collections/StringUtils.h"
#include "Collections/StringBuffer.h"
#include "Olap/Context.h"
#include "Engine/CombinationProcessor.h"
#include "Engine/SequenceProcessor.h"
#include "Engine/AggregationProcessor.h"
#include "Engine/TransformationProcessor.h"
#include "Engine/ArithmeticProcessors.h"

#include "Engine/EngineBase.h"
#include "Engine/Cache.h"

namespace palo {

PIdHolder EngineBase::storageIdHolder(new IdHolder());

const CellValue CellValue::NullString(false);
const CellValue CellValue::NullNumeric(true);
const CellValue CellValue::MarkerValue(1.0);

CellValue::CellValue(bool isNumeric) :
		value(0.0), isNum(isNumeric), isEmp(true), ruleId(NO_RULE), status(0), err((ErrorException::ErrorType)0)
{
}

CellValue::CellValue(const string &str) :
		string(str), value(0.0), isNum(false), isEmp(false), ruleId(NO_RULE), status(0), err((ErrorException::ErrorType)0)
{
}

CellValue::CellValue(double value) :
		value(value), isNum(true), isEmp(value == 0.0), ruleId(NO_RULE), status(0), err((ErrorException::ErrorType)0)
{
}

CellValue::CellValue(double value, const string &str) :
		string(str), value(value), isNum(false), isEmp(false), ruleId(NO_RULE), status(0), err((ErrorException::ErrorType)0)
{
}

CellValue::CellValue(ErrorException::ErrorType err) :
		value(0.0), isNum(false), isEmp(false), ruleId(NO_RULE), status(0), err(err)
{
}

CellValue &CellValue::operator=(const CellValue &val)
{
	string::operator=(val);
	value = val.value;
	isNum = val.isNum;
	isEmp = val.isEmp;
	ruleId = val.ruleId;
	status = val.status;
	err = val.err;
	return *this;
}

CellValue &CellValue::operator=(const string &str)
{
	string::operator=(str);
	value = 0.0;
	isNum = false;
	isEmp = string::empty();
	ruleId = NO_RULE;
	status = 0;
	err = (ErrorException::ErrorType)0;
	return *this;
}

CellValue &CellValue::operator=(const char *str)
{
	string::operator=(str);
	value = 0.0;
	isNum = false;
	isEmp = false;
	ruleId = NO_RULE;
	status = 0;
	err = (ErrorException::ErrorType)0;
	return *this;
}

CellValue &CellValue::operator=(double val)
{
	clear();
	value = val;
	isNum = true;
	isEmp = value == 0.0;
	ruleId = NO_RULE;
	status = 0;
	err = (ErrorException::ErrorType)0;
	return *this;
}

bool CellValue::operator==(const CellValue &b) const
{
	return (isNumeric() && b.isNumeric() && getNumeric() == b.getNumeric()) || (isString() && b.isString() && (string &)*this == (string &)b);
}

bool CellValue::operator!=(const CellValue &b) const
{
	return !(*this == b);
}

double CellValue::getNumeric() const
{
	return value;
}

ErrorException::ErrorType CellValue::getError() const
{
	return err;
}

bool CellValue::isNumeric() const
{
	return isError() ? false : isNum;
}

bool CellValue::isString() const
{
	return isError() ? false : !isNum;
}

bool CellValue::isError() const
{
	return err != 0;
}

bool CellValue::isEmpty() const
{
	return isEmp;
}

void CellValue::setRuleId(IdentifierType id)
{
	ruleId = id;
}

IdentifierType CellValue::getRuleId() const
{
	return ruleId;
}

string CellValue::toString() const
{
	double val = value;
	if (isNum) {
		StringBuffer s;
		s.appendDecimal(val);
		return s.str();
	} else {
		return StringUtils::escapeString(*this);
	}
}

PCommitableList StorageList::createnew(const CommitableList& l) const
{
	return PCommitableList(new StorageList(dynamic_cast<const StorageList &>(l)));
}

EngineBase::EngineBase(const string &engineName, Type type) :
		Commitable(engineName), storageList(PStorageList(new StorageList(storageIdHolder))), type(type)
{
	setID(type);
}

EngineBase::EngineBase(const EngineBase &e) :
		Commitable(e), storageList(e.storageList), type(e.type)
{
}

EngineBase::~EngineBase()
{
}

bool EngineBase::merge(const CPCommitable &o, const PCommitable &p)
{
	checkCheckedOut();
	mergeint(o);
	bool ret = true;
	CPEngineBase eng = CONST_COMMITABLE_CAST(EngineBase, o);
	CPEngineBase oldeng = CONST_COMMITABLE_CAST(EngineBase, old);
	if (old != 0) {
		if (eng != 0) {
			if (type == oldeng->type) {
				type = eng->type;
			}
		}
	}
	if (storageList->isCheckedOut()) {
		ret = storageList->merge(o != 0 ? eng->storageList : PStorageList(), shared_from_this());
	} else if (o != 0) {
		storageList = eng->storageList;
	}
	if (ret) {
		commitintern();
	}
	return ret;
}

PStorageBase EngineBase::getStorage(IdentifierType id) const
{
	PCommitable storage;
	storage = storageList->get(id, false);
	return COMMITABLE_CAST(StorageBase, storage);
}

void EngineBase::deleteStorage(IdentifierType storageId)
{
	checkCheckedOut();
	getStorageList(true)->remove(storageId);
}

void EngineBase::recreateStorage(IdentifierType id, PPathTranslator pathTranslator, StorageType type)
{
	checkCheckedOut();
	PStorageBase oldStorage = getStorage(id);
	deleteStorage(id);
	PStorageBase newStorage = getCreateStorage(id, pathTranslator, type);
	newStorage->updateOld(oldStorage);
}

PStorageList EngineBase::getStorageList(bool write)
{
	checkCheckedOut();
	if (write) {
		if (!storageList->isCheckedOut()) {
			storageList = COMMITABLE_CAST(StorageList, storageList->copy());
		}
	}
	return storageList;
}

bool EngineBase::isPlanSupported(CPPlanNode node) const
{
	return true;
}

PCellStream EngineBase::createProcessor(CPPlanNode node, bool useCache)
{
	PEngineBase thisEngine = COMMITABLE_CAST(EngineBase, shared_from_this());
	switch (node->getType()) {
	case COMBINATION:
		return PCellStream(new CombinationProcessor(thisEngine, node));
	case SEQUENCE:
		return PCellStream(new SequenceProcessor(thisEngine, node));
	case SOURCE: {
		const SourcePlanNode *pn = static_cast<const SourcePlanNode *>(node.get());
		PStorageBase storage = getStorage(pn->getStorageId());
		return storage->getCellValues(pn->getArea(), pn->getDefaultValue());
	}
	case AGGREGATION: {
		PCellStream ret;
		const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(node.get());
		if (apn->getAggregationType() == AggregationPlanNode::SUM) {
			ret.reset(new AggregationProcessor(thisEngine, node));
			if (useCache && node->getCache()) {
				ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret, NO_IDENTIFIER);
			}
		} else {
			ret.reset(new AggregationFunctionProcessor(thisEngine, node));
		}
		return ret;
	}
	case TRANSFORMATION: {
		const TransformationPlanNode *transformationPlanNode = dynamic_cast<const TransformationPlanNode *>(node.get());
		if (transformationPlanNode->getSetMultiMaps()) {
			return PCellStream(new TransformationMapProcessor(thisEngine, node));
		} else {
			return PCellStream(new TransformationProcessor(thisEngine, node));
		}
	}
	case MULTIPLICATION: {
		PCellStream ret(new MultiplicationProcessor(thisEngine, node));
		if (useCache && node->getCache()) {
			// Todo: -jj- should never happen "new" rules results are not cached yet. Rule Id must be provided
//			ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret);
		}
		return ret;
	}
	case DIVISION: {
		PCellStream ret(new DivisionProcessor(thisEngine, node));
		if (useCache && node->getCache()) {
			// Todo: -jj- should never happen "new" rules results are not cached yet. Rule Id must be provided
//			ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret);
		}
		return ret;
	}
	case ADDITION: {
		PCellStream ret(new AdditionProcessor(thisEngine, node));
		if (useCache && node->getCache()) {
			// Todo: -jj- should never happen "new" rules results are not cached yet. Rule Id must be provided
//			ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret);
		}
		return ret;
	}
	case SUBTRACTION: {
		PCellStream ret(new SubtractionProcessor(thisEngine, node));
		if (useCache && node->getCache()) {
			// Todo: -jj- should never happen "new" rules results are not cached yet. Rule Id must be provided
//			ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret);
		}
		return ret;
	}
	case CACHE: {
		const CachePlanNode *pn = dynamic_cast<const CachePlanNode *>(node.get());
		PCellStream ret(pn->getCacheStorage()->getCellValues(node->getArea(), node->getDefaultValue()));
		if (pn->getRuleId() != NO_IDENTIFIER && node->getDefaultValue()) {
			ret = PCellStream(new RuleCacheProcessor(ret, pn->getRuleId()));
		}
		return ret;
	}
	case QUERY_CACHE: {
		const QueryCachePlanNode *pn = dynamic_cast<const QueryCachePlanNode *>(node.get());
		PCellStream ret(Context::getContext()->getQueryCache(pn->getCube())->getFilteredValues(node->getArea(), node->getDefaultValue()));
		if (pn->getRuleId() != NO_IDENTIFIER && node->getDefaultValue()) {
			ret = PCellStream(new RuleCacheProcessor(ret, pn->getRuleId()));
		}
		return ret;
	}
	case CONSTANT:
		return PCellStream(new ConstantProcessor(node));
	case CELLMAP: {
		const CellMapPlanNode *cmpn = dynamic_cast<const CellMapPlanNode *>(node.get());
		PDoubleCellMap cellMap = cmpn->getCellMap();
		if (cellMap) {
			return dynamic_cast<ICellMapStream *>(cmpn->getCellMap().get())->getValues();
		} else {
			return PCellStream(new CellMapProcessor(thisEngine, node));
		}
	}
	default:
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Unsupported plan node type");
	}
}

SourcePlanNode::SourcePlanNode(IdentifierType storageId, CPArea area, const CellValue *defaultValue) :
		PlanNode(SOURCE, area, defaultValue, vector<PPlanNode>(), 0, CPCube()), storageId(storageId)
{
}

string SourcePlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " StorageId='" << getStorageId() << "'";
	return ss.str();
}

IdentifierType SourcePlanNode::getStorageId() const
{
	return storageId;
}

AggregationPlanNode::AggregationPlanNode(CPArea area, const CellValue *defaultValue, const vector<PPlanNode> &children, CPAggregationMaps aggregationMaps, ValueCache *cache, CPCube cacheCube, AggregationType aggrType, uint64_t maxCount) :
		PlanNode(AGGREGATION, area, defaultValue, children, cache, cacheCube), aggregationMaps(aggregationMaps), aggrType(aggrType), maxCount(maxCount)
{
}

string AggregationPlanNode::getXMLContent() const
{
	stringstream ss;
	ss << PlanNode::getXMLContent();
	// print the aggregation map
	return ss.str();
}

TransformationPlanNode::TransformationPlanNode(CPArea area, PPlanNode &child, const IdentifiersType &transformationMask, const SetMultimaps &setMultiMaps, double factor) :
		PlanNode(TRANSFORMATION, area, 0, vector<PPlanNode>(1, child), 0, CPCube()), transformationMask(transformationMask), setMultiMaps(setMultiMaps), pSetMultiMaps(0), factor(factor)
{
	if (setMultiMaps.size()) {
		for (SetMultimaps::const_iterator smm = setMultiMaps.begin(); smm != setMultiMaps.end(); ++smm) {
			if (*smm) {
				pSetMultiMaps = &this->setMultiMaps;
			}
		}
	}
}

string TransformationPlanNode::getXMLContent() const
{
	stringstream ss;
	ss << PlanNode::getXMLContent();
	ss << "<KeyTransformation>";
	size_t dimOrdinal = 0;
	bool firstOut = true;
	for (IdentifiersType::const_iterator targetKey = transformationMask.begin(); targetKey != transformationMask.end(); ++targetKey, ++dimOrdinal) {
		if (*targetKey != NO_IDENTIFIER) {
			if (!firstOut) {
				ss << ',';
			}
			ss << '[' << dimOrdinal << "]=" << *targetKey;
			firstOut = false;
		}
	}
	ss << "</KeyTransformation>";
	if (pSetMultiMaps) {
		dimOrdinal = 0;
		for (SetMultimaps::const_iterator mmaps = pSetMultiMaps->begin(); mmaps != pSetMultiMaps->end(); ++mmaps, ++dimOrdinal) {
			if (*mmaps) {
				ss << "<KeyMap dimension='" << dimOrdinal << "'>";
				firstOut = true;
				for (SetMultimap::const_iterator mmap = (*mmaps)->begin(); mmap != (*mmaps)->end(); ++mmap) {
					if (!firstOut) {
						ss << ", ";
					}
					ss << mmap->first << "=>" << mmap->second;
					firstOut = false;
				}
				ss << "</KeyMap>";
			}
		}
	}
	return ss.str();
}

CellMapPlanNode::CellMapPlanNode(PDoubleCellMap cellMap) :
		PlanNode(CELLMAP, PArea(), 0, vector<PPlanNode>(), 0, PCube()), cellMap(cellMap), storageId(NO_IDENTIFIER)
{
}

CellMapPlanNode::CellMapPlanNode(IdentifierType storageId, CPArea area) :
		PlanNode(CELLMAP, area, 0, vector<PPlanNode>(), 0, PCube()), storageId(storageId)
{
}

PlanNode::PlanNode(PlanNodeType type, CPArea area, const CellValue *defaultValue, const vector<PPlanNode> &children, ValueCache *cache, CPCube cacheCube, double constValue) :
		type(type), area(area), defaultValue(defaultValue), children(children), cache(cache), cacheCube(cacheCube), constValue(constValue), ruleId(NO_IDENTIFIER)
{
	setDefaultValue(defaultValue);
}

PlanNodeType PlanNode::getType() const
{
	return type;
}

string PlanNode::getTypeString() const
{
	switch (getType()) {
	case COMBINATION:
		return "Combination";
	case SEQUENCE:
		return "Sequence";
	case SOURCE:
		return "Source";
	case AGGREGATION:
		return "Aggregation";
	case ADDITION:
		return "Addition";
	case SUBTRACTION:
		return "Subtraction";
	case MULTIPLICATION:
		return "Multiplication";
	case DIVISION:
		return "Division";
	case LEGACY_RULE:
		return "LegacyRule";
	case TRANSFORMATION:
		return "Transformation";
	case CACHE:
		return "Cache";
	case QUERY_CACHE:
		return "QueryCache";
	default:
		return "Unknown";
	}
}

CPArea PlanNode::getArea() const
{
	return area;
}

void PlanNode::setArea(CPArea area)
{
	this->area = area;
}

void PlanNode::setDefaultValue(const CellValue *defaultValue)
{
	if (defaultValue) {
		this->defaultValueCopy = *defaultValue;
		this->defaultValue = &this->defaultValueCopy;
	} else {
		this->defaultValue = 0;
	}
}

const CellValue *PlanNode::getDefaultValue() const
{
	return defaultValue;
}

const vector<PPlanNode> &PlanNode::getChildren() const
{
	return children;
}

string PlanNode::toXML() const
{
	stringstream ss;
	string ElementName = getTypeString();
	ss << "<" << ElementName << getXMLAttributes() << ">";
	ss << getXMLContent();
	ss << "</" << ElementName << ">";
	return ss.str();
}

string PlanNode::getXMLAttributes() const
{
	stringstream ss;
	if (defaultValue) {
		ss << " Default='" << defaultValue->toString() << "'";
	}
	return ss.str();
}

string PlanNode::getXMLContent() const
{
	stringstream ss;
	if (area) {
		// serialize the area
		ss << "<Area Size='" << area->getSize() << "'>" << *area << "</Area>";
	}
	if (children.size()) {
		// serialize children
		ss << "<Children>";
		for (vector<PPlanNode>::const_iterator child = children.begin(); child != children.end(); ++child) {
			if (*child) {
				ss << (*child)->toXML();
			} else {
				ss << "<Constant value='" << getConstValue() << "'/>";
			}
		}
		ss << "</Children>";
	}
	return ss.str();
}

string CachePlanNode::getXMLAttributes() const
{
	stringstream ss;
	if (ruleId != NO_IDENTIFIER) {
		ss << " RuleId='" << ruleId << "'";
	}
	return ss.str();
}

}
