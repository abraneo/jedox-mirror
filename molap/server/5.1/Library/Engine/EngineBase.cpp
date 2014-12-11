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

#include "Collections/StringUtils.h"
#include "Collections/StringBuffer.h"
#include "Olap/Context.h"
#include "Olap/Server.h"
#include "Engine/CombinationProcessor.h"
#include "Engine/SequenceProcessor.h"
#include "Engine/AggregationProcessor.h"
#include "Engine/TransformationProcessor.h"
#include "Engine/ArithmeticProcessors.h"
#include "Engine/DFilterProcessor.h"

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

bool CellValue::operator<(const CellValue &b) const
{
	if (isNumeric() && b.isNumeric()) {
		return getNumeric() < b.getNumeric();
	} else {
		return UTF8Comparer::compare(*this, b) < 0;
	}
}

double CellValue::getNumeric() const
{
	return value;
}

CellValue::operator double() const
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

ostream& operator<<(ostream& ostr, const CellValue& value)
{
	ostr << value.toString();
	return ostr;
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
	mergeint(o,p);
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

PProcessorBase EngineBase::createProcessor(CPPlanNode node, bool sortedOutput, bool useCache)
{
	PEngineBase thisEngine = COMMITABLE_CAST(EngineBase, shared_from_this());
	switch (node->getType()) {
	case UNION:
		if (sortedOutput) {
			return PProcessorBase(new CombinationProcessor(thisEngine, node->getChildren(), node->getArea()->getPathTranslator()));
		} else {
			return PProcessorBase(new SequenceProcessor(thisEngine, node));
		}
	case SOURCE: {
		const SourcePlanNode *pn = static_cast<const SourcePlanNode *>(node.get());
		PStorageBase storage = getStorage(pn->getStorageId());
		return storage->getCellValues(pn->getArea());
	}
	case AGGREGATION: {
		PProcessorBase ret;
		const AggregationPlanNode *apn = dynamic_cast<const AggregationPlanNode *>(node.get());
		if (apn->getAggregationType() == AggregationPlanNode::SUM) {
			ret.reset(new AggregationProcessor(thisEngine, node));
			if (useCache && node->getCache()) {
				ret = node->getCache()->getWriter(node->getCacheCube(), PCubeArea(new CubeArea(CPDatabase(), CPCube(), *node->getArea())), ret, NO_IDENTIFIER);
			}
		} else {
			ret.reset(new AggregationFunctionProcessor(thisEngine, node));
		}
		if (apn->getCondition()) {
			ret.reset(new FilteredReader(thisEngine, ret, apn->getCondition(), apn->getArea()));
		}
		return ret;
	}
	case TRANSFORMATION: {
		PProcessorBase ret;
		const TransformationPlanNode *transformationPlanNode = dynamic_cast<const TransformationPlanNode *>(node.get());
		if (transformationPlanNode->getSourceCubeId().first != NO_IDENTIFIER) {
			Context::getContext()->setCacheDependence(transformationPlanNode->getSourceCubeId());
		}
		if (transformationPlanNode->getSetMultiMaps() && !transformationPlanNode->getSetMultiMaps()->isTrivialMapping()) {
			boost::shared_ptr<TransformationMapProcessor> tmp(new TransformationMapProcessor(thisEngine, node, sortedOutput));
			if (tmp->isBrokenOrder()) {
				ret = tmp->getSortedResults();
			} else {
				ret = tmp;
			}
		} else {
			ret = PProcessorBase(new TransformationProcessor(thisEngine, node));
		}
		return ret;
	}
	case MULTIPLICATION: {
		PProcessorBase ret(new MultiplicationProcessor(thisEngine, node));
		return ret;
	}
	case DIVISION: {
		PProcessorBase ret(new DivisionProcessor(thisEngine, node));
		return ret;
	}
	case ADDITION: {
		PProcessorBase ret(new AdditionProcessor(thisEngine, node));
		return ret;
	}
	case SUBTRACTION: {
		PProcessorBase ret(new SubtractionProcessor(thisEngine, node));
		return ret;
	}
	case CACHE: {
		const CachePlanNode *pn = dynamic_cast<const CachePlanNode *>(node.get());
		PProcessorBase ret(pn->getCacheStorage()->getCellValues(pn->getArea()));

		if (pn->getDefaultValue() || pn->getRuleId() != NO_IDENTIFIER) {
			// complete the info - ruleId and/or defaultValue
			ret = PProcessorBase(new CompleteProcessor(pn->getArea(), pn->getDefaultValue(), pn->getRuleId(), ret));
		}
		return ret;
	}
	case QUERY_CACHE: {
		const QueryCachePlanNode *pn = dynamic_cast<const QueryCachePlanNode *>(node.get());
		PProcessorBase ret(Context::getContext()->getQueryCache(pn->getCube())->getFilteredValues(node->getArea(), pn->getDefaultValue()));
		if (pn->getRuleId() != NO_IDENTIFIER && pn->getDefaultValue()) {
			ret = PProcessorBase(new CompleteProcessor(pn->getArea(), pn->getDefaultValue(), pn->getRuleId(), ret));
		}
		return ret;
	}
	case CONSTANT: {
		const ConstantPlanNode *cpn = dynamic_cast<const ConstantPlanNode *>(node.get());
		return PProcessorBase(new ConstantProcessor(cpn->getArea(), cpn->getDefaultValue()));
	}
	case CELLMAP: {
		const CellMapPlanNode *cmpn = dynamic_cast<const CellMapPlanNode *>(node.get());
		PDoubleCellMap cellMap = cmpn->getCellMap();
		if (cellMap) {
			return cmpn->getCellMap()->getValues();
		} else {
			return PProcessorBase(new CellMapProcessor(thisEngine, node));
		}
	}
	case COMPLETE: {
		const CompletePlanNode *cpn = dynamic_cast<const CompletePlanNode *>(node.get());
		PProcessorBase ret(new CompleteProcessor(thisEngine, cpn->getArea(), cpn->getDefaultValue(), cpn->getRuleId(), cpn->getChildren()[0]));
		return ret;
	}
	case QUANTIFICATION: {
		return PProcessorBase(new DFilterQuantificationProcessor(thisEngine, node));
	}
	default:
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Unsupported plan node type");
	}
}

SourcePlanNode::SourcePlanNode(IdentifierType storageId, CPArea area, uint64_t revision) :
		PlanNode(SOURCE, area, vector<PPlanNode>(), 0, CPCube()), storageId(storageId), revision(revision)
{
}

void SourcePlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// write source storage Identification including revision here
	w.appendString(" ",0);
	// TODO: -jj- get persistent storage Id
	w.appendIdentifier(getStorageId(),'/');
	w.appendIdentifier((IdentifierType)getRevision(),0);  // TODO: -jj- serialize uint64_t
}

bool SourcePlanNode::isEqual(const PlanNode& b) const
{
	const SourcePlanNode &o = dynamic_cast<const SourcePlanNode &>(b);
	return storageId == o.storageId && revision == o.revision;
}

string SourcePlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " StorageId='" << getStorageId() << "'";
	return ss.str();
}

AggregationPlanNode::AggregationPlanNode(CPArea area, const vector<PPlanNode> &children, CPAggregationMaps aggregationMaps, ValueCache *cache, CPCube cacheCube, AggregationType aggrType, uint64_t maxCount, PCondition cond, int dfDimIndex, double dfCellsPerElement) :
	PlanNode(AGGREGATION, area, children, cache, cacheCube), aggregationMaps(aggregationMaps), aggrType(aggrType), maxCount(maxCount), cond(cond), dfDimIndex(dfDimIndex), dfCellsPerElement(dfCellsPerElement)
{
}

string AggregationPlanNode::getFunctionTypeString(bool code) const
{
	string result = code ? "?" : "Unknown";
	switch (getAggregationType()) {
	case SUM:
		result = code ? "S" : "Sum";
		break;
	case AVG:
		result = code ? "A" : "Avg";
		break;
	case COUNT:
		result = code ? "C" : "Count";
		break;
	case MAX:
		result = code ? "X" : "Max";
		break;
	case MIN:
		result = code ? "N" : "Min";
		break;
	default:
		break;
	}
	return result;
}

void AggregationPlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// TODO: -jj- write Aggregation Function and AggregationMap here
	w.appendString(getFunctionTypeString(true),0);
	w.appendAggregationMaps(getAggregationMaps());
}

bool AggregationPlanNode::isEqual(const PlanNode& b) const
{
	const AggregationPlanNode &o = dynamic_cast<const AggregationPlanNode &>(b);
	return (aggregationMaps == o.aggregationMaps ||
			(aggregationMaps && o.aggregationMaps && aggregationMaps->size() == o.aggregationMaps->size() &&
			equal(aggregationMaps->begin(), aggregationMaps->end(), o.aggregationMaps->begin()))) &&
			aggrType == o.aggrType && maxCount == o.maxCount;
}

string AggregationPlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " Function='" << getFunctionTypeString() << "'";
	return ss.str();
}

string AggregationPlanNode::getXMLContent() const
{
	stringstream ss;
	ss << PlanNode::getXMLContent();
	// print the aggregation map
	return ss.str();
}

QuantificationPlanNode::QuantificationPlanNode(PCubeArea dfArea, CPCubeArea numericArea, const vector<PPlanNode> &children, QuantificationType quantType, PCondition cond, int dimIndex, bool isVirt, double cellsPerElement, bool calcRules, uint64_t maxCount) :
	PlanNode(QUANTIFICATION, dfArea, children, 0, PCube()), dfArea(dfArea), numericArea(numericArea), quantType(quantType), cond(cond), dimIndex(dimIndex), isVirt(isVirt), cellsPerElement(cellsPerElement), calcRules(calcRules), maxCount(maxCount)
{
}

QuantificationPlanNode::QuantificationPlanNode(PCubeArea dfArea, const vector<PPlanNode> &children, int dimIndex, bool isVirt, bool calcRules, uint64_t maxCount) :
	PlanNode(QUANTIFICATION, dfArea, children, 0, PCube()), dfArea(dfArea), quantType(EXISTENCE), dimIndex(dimIndex), isVirt(isVirt), cellsPerElement(0), calcRules(calcRules), maxCount(maxCount)
{
}

string QuantificationPlanNode::getQuantificationTypeString(bool code) const
{
	string result = code ? "?" : "Unknown";
	switch (getQuantificationType()) {
	case ALL:
		result = code ? "A" : "All";
		break;
	case ANY_NUM:
		result = code ? "N" : "AnyNumeric";
		break;
	case ANY_STR:
		result = code ? "S" : "AnyString";
		break;
	case EXISTENCE:
		result = code ? "X" : "Existence";
		break;
	default:
		break;
	}
	return result;
}

string QuantificationPlanNode::getXMLAttributes() const
{
	stringstream ss;
	ss << " Function='" << getQuantificationTypeString() << "'";
	return ss.str();
}

string QuantificationPlanNode::getXMLContent() const
{
	stringstream ss;
	return ss.str();
}

TransformationPlanNode::TransformationPlanNode(CPArea area, PPlanNode &child, const SetMultimaps &setMultiMaps, double factor, const vector<uint32_t> &dimensionMapping) :
	PlanNode(TRANSFORMATION, area, vector<PPlanNode>(1, child), 0, CPCube()), setMultiMaps(setMultiMaps),
	pSetMultiMaps(0), factor(factor), dimensionMapping(dimensionMapping), sourceCubeId(NO_IDENTIFIER, NO_IDENTIFIER)
{
	if (setMultiMaps.size()) {
		for (SetMultimaps::const_iterator smm = setMultiMaps.begin(); smm != setMultiMaps.end(); ++smm) {
			if (*smm) {
				pSetMultiMaps = &this->setMultiMaps;
			}
		}
	}
}

void TransformationPlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// TODO: -jj- write multiplication factor and dimension map here
	if (getFactor() != 1.0) {
		w.appendDouble(getFactor(),0);
	}
}

bool TransformationPlanNode::isEqual(const PlanNode& b) const
{
	const TransformationPlanNode &o = dynamic_cast<const TransformationPlanNode &>(b);
	return setMultiMaps == o.setMultiMaps; // TODO: -jj- or if content of multimaps is identical
}

string TransformationPlanNode::getXMLContent() const
{
	stringstream ss;
	ss << PlanNode::getXMLContent();
	//vector<uint32_t> dimensionMapping
	if (dimensionMapping.size()) {
		ss << "<DimMap>";
		for(vector<uint32_t>::const_iterator dmit = dimensionMapping.begin(); dmit != dimensionMapping.end(); ++dmit) {
			if (dmit != dimensionMapping.begin()) {
				ss << ", ";
			}
			ss << *dmit++;
			if (dmit != dimensionMapping.end()) {
				ss << '-' << *dmit;
			}
		}
		ss << "</DimMap>";
	}
	if (pSetMultiMaps) {
		size_t dimOrdinal = 0;
		for (SetMultimaps::const_iterator mmaps = pSetMultiMaps->begin(); mmaps != pSetMultiMaps->end(); ++mmaps, ++dimOrdinal) {
			if (*mmaps) {
				ss << "<KeyMap dimension='" << dimOrdinal << "'>";
				bool firstOut = true;
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

void ConstantPlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// write Constant CellValue here
	w.appendString(getDefaultValue()->toString(),0);
}

bool ConstantPlanNode::isEqual(const PlanNode& b) const
{
	const ConstantPlanNode &o = dynamic_cast<const ConstantPlanNode &>(b);
	return CompleteNodeInfo::isEqual(o);
}

CellMapPlanNode::CellMapPlanNode(PDoubleCellMap cellMap) :
		PlanNode(CELLMAP, PArea(), vector<PPlanNode>(), 0, PCube()), cellMap(cellMap), storageId(NO_IDENTIFIER)
{
}

CellMapPlanNode::CellMapPlanNode(IdentifierType storageId, CPArea area) :
		PlanNode(CELLMAP, area, vector<PPlanNode>(), 0, PCube()), storageId(storageId)
{
}

void CellMapPlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// TODO: -jj- write CellMap content here
}

bool CellMapPlanNode::isEqual(const PlanNode& b) const
{
//	const CellMapPlanNode &o = dynamic_cast<const CellMapPlanNode &>(b);
	return cellMap && storageId; // TODO: -jj- compare content of cellmap?
}

PlanNode::PlanNode(PlanNodeType type, CPArea area, const vector<PPlanNode> &children, ValueCache *cache, CPCube cacheCube, double constValue) :
		type(type), area(area), children(children), cache(cache), cacheCube(cacheCube), constValue(constValue)
{
}

PlanNodeType PlanNode::getType() const
{
	return type;
}

string PlanNode::getTypeString(bool code) const
{
	switch (getType()) {
	case UNION:
		return code ? "U" : "Union";
	case SOURCE:
		return code ? "S" : "Source";
	case AGGREGATION:
		return code ? "A" : "Aggregation";
	case ADDITION:
		return code ? "+" : "Addition";
	case SUBTRACTION:
		return code ? "-" : "Subtraction";
	case MULTIPLICATION:
		return code ? "*" : "Multiplication";
	case DIVISION:
		return code ? "/" : "Division";
	case LEGACY_RULE:
		return code ? "R" : "LegacyRule";
	case TRANSFORMATION:
		return code ? "T" : "Transformation";
	case CACHE:
		return code ? "C" : "Cache";
	case QUERY_CACHE:
		return code ? "Q" : "QueryCache";
	case COMPLETE:
		return code ? "E" : "Complete";
	case ROUNDCORRECT:
		return code ? "O" : "RoundCorrect";
	case QUANTIFICATION:
		return code ? "X" : "Quantification";
	case CONSTANT:
		return code ? "N" : "Constant";
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

void PlanNode::write(FileWriter &w, CPArea parentArea) const
{
	w.appendString(getTypeString(true),0);
	w.appendAreaCompact(getArea(), parentArea);
	if (!children.empty()) {
		bool firstChild = true;
		CPArea nextParentArea = getArea() ? getArea() : parentArea;
		w.appendString("(",0);
		for (vector<PPlanNode>::const_iterator chit = children.begin(); chit != children.end(); ++chit) {
			if (firstChild) {
				firstChild = false;
			} else {
				w.appendString(",",0);
			}
			w.appendString("(",0);
			if (*chit) {
				w.appendPlan(*chit, nextParentArea);
			}
			w.appendString(")",0);
		}
		w.appendString(")",0);
	}
}

bool nodeEqual(const PPlanNode &a, const PPlanNode &b)
{
	return a == b || (a && b && *a == *b);
}

bool PlanNode::operator==(const PlanNode& b) const
{
	return type == b.type && cache == b.cache && constValue == b.constValue &&
			(area == b.area || (area != 0 && b.area != 0 && area->dimCount() == b.area->dimCount() && *area == *b.area)) && children.size() == b.children.size() &&
			equal(children.begin(), children.end(), b.children.begin(), nodeEqual) && isEqual(b);
}



string PlanNode::getXMLAttributes() const
{
	stringstream ss;
//	if (defaultValue) {
//		ss << " Default='" << defaultValue->toString() << "'";
//	}
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

bool UnionPlanNode::canHaveDuplicates() const
{
	bool anyIntersection = false;
	for (vector<PPlanNode>::const_iterator child = children.begin(); !anyIntersection && child != children.end(); ++child) {
		vector<PPlanNode>::const_iterator otherChild = child;
		++otherChild;
		while (otherChild != children.end()) {
			CPArea area1, area2;
			if (*child) {
				area1 = (*child)->getArea();
			}
			if (*otherChild) {
				area2 = (*otherChild)->getArea();
			}
			if (area1 && area2 && area1->intersection(*area2)) {
				anyIntersection = true;
				break;
			}
			++otherChild;
		}
	}
	return anyIntersection;
}

bool UnionPlanNode::isEqual(const PlanNode& b) const
{
	const UnionPlanNode &o = dynamic_cast<const UnionPlanNode &>(b);
	return ruleId == o.ruleId;
}

void CompletePlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// write DefaultValue and RuleId here
	w.appendString("(",0);
	if (getDefaultValue()) {
		w.appendString(getDefaultValue()->toString(),0);
	}
	w.appendString(")",0);
	if (getRuleId() != NO_RULE){
		w.appendIdentifier(getRuleId(),0);
	}
}

bool CompletePlanNode::isEqual(const PlanNode& b) const
{
	const CompletePlanNode &o = dynamic_cast<const CompletePlanNode &>(b);
	return CompleteNodeInfo::isEqual(o);
}

string CompletePlanNode::getXMLAttributes() const
{
	stringstream ss;
	if (ruleId != NO_RULE) {
		ss << " rule='" << ruleId << "'";
	}
	if (getDefaultValue()) {
		ss << " value='" << *getDefaultValue() << "'";
	}
	return ss.str();
}

void RoundCorrrectPlanNode::write(FileWriter &w, CPArea parentArea) const
{
	PlanNode::write(w, parentArea);
	// TODO: -jj- write targetValue unitValue and AggregationMap here
}

bool RoundCorrrectPlanNode::isEqual(const PlanNode& b) const
{
	const RoundCorrrectPlanNode &o = dynamic_cast<const RoundCorrrectPlanNode &>(b);
	return targetValue == o.targetValue && unitValue == o.unitValue && aggregationMaps == o.aggregationMaps; // TODO: -jj- or if content of aggregationMaps is identical
}

string CachePlanNode::getXMLAttributes() const
{
	stringstream ss;
	if (ruleId != NO_IDENTIFIER) {
		ss << " RuleId='" << ruleId << "'";
	}
	return ss.str();
}

bool CachePlanNode::isEqual(const PlanNode& b) const
{
	const CachePlanNode &o = dynamic_cast<const CachePlanNode &>(b);
	return cacheStorage == o.cacheStorage && CompleteNodeInfo::isEqual(o);
}

bool QueryCachePlanNode::isEqual(const PlanNode& b) const
{
	const QueryCachePlanNode &o = dynamic_cast<const QueryCachePlanNode &>(b);
	return cube == o.cube && CompleteNodeInfo::isEqual(o);
}

string ConstantPlanNode::getXMLAttributes() const
{
	stringstream ss;
	if (getDefaultValue()) {
		ss << " value='" << *getDefaultValue() << "'";
	}
	return ss.str();
}


PEngineBase ProcessorBase::selectEngine(CPPlanNode plan)
{
	Context *context = Context::getContext();
	PServer server = context->getServer();
	PEngineBase selectedEngine;

	const set<EngineBase::Type> &engines = context->getAvailableEngines();
	// find engine
	for (set<EngineBase::Type>::const_reverse_iterator engineIt = engines.rbegin(); engineIt != engines.rend(); ++engineIt) {
		selectedEngine = server->getEngine(*engineIt);
		if (selectedEngine->isPlanSupported(plan)) {
			break;
		}
		selectedEngine.reset();
	}
	return selectedEngine;
}

PProcessorBase ProcessorBase::createProcessor(CPPlanNode plan, bool sortedOutput, bool useCache)
{
	PEngineBase selectedEngine;
	if (isEngineLocked()) {
		selectedEngine = getEngine();
	} else {
		selectedEngine = selectEngine(plan);
	}
	return selectedEngine->createProcessor(plan, sortedOutput, useCache);
}

}
