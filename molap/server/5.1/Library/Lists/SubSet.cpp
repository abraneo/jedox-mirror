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
 * \author Frieder Hofmann , Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Filter.h"
#include "SubSet.h"
#include "PickList.h"
#include "TextFilter.h"
#include "SortingFilter.h"
#include "DataFilter.h"
#include "AliasFilter.h"
#include "StructuralFilter.h"

#include "Olap/AttributedDimension.h"
#include "Olap/AttributesDimension.h"

namespace palo {

class SubSet::ItBase
{
public:
	ItBase(SubSet *sub) : sub(sub) {}
	ItBase() : sub(0) {}
	ItBase(const ItBase &o) : sub(o.sub) {}
	virtual ~ItBase() {}
	virtual ItBase *clone() = 0;
	virtual bool end() = 0;
	virtual IdentifierType getId() = 0;
	virtual void next() = 0;
	virtual size_t getChildrenCount() = 0;
	virtual IndentType getIndent() = 0;
	virtual LevelType getLevel() = 0;
	virtual DepthType getDepth() = 0;
	virtual string getName() = 0;
	virtual PositionType getPosition() = 0;
	virtual IdentifierType getConsOrder() = 0;
	virtual Element *getElement() = 0;
	virtual string getPath() = 0;
protected:
	friend class Iterator;
	SubSet *sub;
};

class SubSet::MapIter : public SubSet::ItBase
{
public:
	MapIter(set<Element *>::iterator beg, set<Element *>::iterator end, SubSet *sub) : ItBase(sub), beg(beg), endit(end) {}
	virtual ~MapIter() {}
	virtual ItBase *clone()
	{
		return new MapIter(beg, endit, sub);
	}
	virtual bool end()
	{
		return beg == endit;
	}
	virtual IdentifierType getId()
	{
		return (*beg)->getIdentifier();
	}
	virtual void next()
	{
		++beg;
	}
	virtual size_t getChildrenCount()
	{
		return (*beg)->getChildrenCount();
	}
	virtual IndentType getIndent()
	{
		return (*beg)->getIndent();
	}
	virtual LevelType getLevel()
	{
		return (*beg)->getLevel();
	}
	virtual DepthType getDepth()
	{
		return (*beg)->getDepth();
	}
	virtual string getName()
	{
		return (*beg)->getName(sub->dim->getElemNamesVector());
	}
	virtual PositionType getPosition()
	{
		return (*beg)->getPosition();
	}
	virtual IdentifierType getConsOrder()
	{
		return NO_IDENTIFIER;
	}
	virtual Element *getElement()
	{
		return (*beg);
	}
	virtual string getPath()
	{
		return "";
	}
private:
	set<Element *>::iterator beg;
	set<Element *>::iterator endit;
};

class SubSet::FinalIter : public SubSet::ItBase
{
public:
	FinalIter(SubSet *sub) : ItBase(sub), it(sub->finalList.begin())
	{
	}
	FinalIter(const FinalIter &o) : ItBase(o), it(o.it)
	{
	}
	virtual ~FinalIter() {}
	virtual ItBase *clone()
	{
		return new FinalIter(*this);
	}
	virtual bool end()
	{
		return it == sub->finalList.end();
	}
	virtual IdentifierType getId()
	{
		return it->elem->getIdentifier();
	}
	virtual void next()
	{
		++it;
	}
	virtual size_t getChildrenCount()
	{
		return it->elem->getChildrenCount();
	}
	virtual IndentType getIndent()
	{
		return it->ind;
	}
	virtual LevelType getLevel()
	{
		return it->elem->getLevel();
	}
	virtual DepthType getDepth()
	{
		return it->dep;
	}
	virtual string getName()
	{
		return it->elem->getName(sub->dim->getElemNamesVector());
	}
	virtual PositionType getPosition()
	{
		return it->elem->getPosition();
	}
	virtual IdentifierType getConsOrder()
	{
		return NO_IDENTIFIER;
	}
	virtual Element *getElement()
	{
		return it->elem;
	}
	virtual string getPath()
	{
		return it->path;
	}
private:
	vector<SubElem>::iterator it;
};

class SubSet::ChildrenIter : public SubSet::ItBase
{
public:
	ChildrenIter(Element *el, SubSet *sub) : ItBase(sub), i(0)
	{
		children = sub->getChildren(el->getIdentifier());
		if (!children) {
			children.reset(new ElementsWeightType(sub->dim->getChildren(sub->user, el)));
			sub->setChildren(el->getIdentifier(), children);
		}
	}
	ChildrenIter(const ChildrenIter &o) : ItBase(o), i(o.i), children(o.children)
	{
	}
	virtual ~ChildrenIter() {}
	virtual ItBase *clone()
	{
		return new ChildrenIter(*this);
	}
	virtual bool end()
	{
		return i >= children->size();
	}
	virtual IdentifierType getId()
	{
		return children->at(i).first->getIdentifier();
	}
	virtual void next()
	{
		if (!end()) {
			++i;
		}
	}
	virtual size_t getChildrenCount()
	{
		return children->at(i).first->getChildrenCount();
	}
	virtual IndentType getIndent()
	{
		return children->at(i).first->getIndent();
	}
	virtual LevelType getLevel()
	{
		return children->at(i).first->getLevel();
	}
	virtual DepthType getDepth()
	{
		return children->at(i).first->getDepth();
	}
	virtual string getName()
	{
		return children->at(i).first->getName(sub->dim->getElemNamesVector());
	}
	virtual PositionType getPosition()
	{
		return children->at(i).first->getPosition();
	}
	virtual IdentifierType getConsOrder()
	{
		return (IdentifierType)i;
	}
	virtual Element *getElement()
	{
		return children->at(i).first;
	}
	virtual string getPath()
	{
		return "";
	}
private:
	size_t i;
	boost::shared_ptr<ElementsWeightType> children;
};

class SubSet::ParentsIter : public SubSet::ItBase
{
public:
	ParentsIter(Element *el, SubSet *sub) : ItBase(sub), parents(new ParentsType(sub->dim->getParents(sub->user, el))), i(0)
	{
	}
	ParentsIter(const ParentsIter &o) : ItBase(o), parents(o.parents), i(o.i)
	{
	}
	virtual ~ParentsIter() {}
	virtual ItBase *clone()
	{
		return new ParentsIter(*this);
	}
	virtual bool end()
	{
		return i >= parents->size();
	}
	virtual IdentifierType getId()
	{
		return parents->at(i)->getIdentifier();
	}
	virtual void next()
	{
		if (!end()) {
			++i;
		}
	}
	virtual size_t getChildrenCount()
	{
		return parents->at(i)->getChildrenCount();
	}
	virtual IndentType getIndent()
	{
		return parents->at(i)->getIndent();
	}
	virtual LevelType getLevel()
	{
		return parents->at(i)->getLevel();
	}
	virtual DepthType getDepth()
	{
		return parents->at(i)->getDepth();
	}
	virtual string getName()
	{
		return parents->at(i)->getName(sub->dim->getElemNamesVector());
	}
	virtual PositionType getPosition()
	{
		return parents->at(i)->getPosition();
	}
	virtual IdentifierType getConsOrder()
	{
		return NO_IDENTIFIER;
	}
	virtual Element *getElement()
	{
		return parents->at(i);
	}
	virtual string getPath()
	{
		return "";
	}
private:
	boost::shared_ptr<ParentsType> parents;
	size_t i;
};

class SubSet::VirtVectorIter : public SubSet::ItBase
{
public:
	VirtVectorIter(ElementsType::iterator beg, ElementsType::iterator end, SubSet *sub) : ItBase(sub), beg(beg), endit(end)
	{
	}
	VirtVectorIter(const VirtVectorIter &o) : ItBase(o), beg(o.beg), endit(o.endit)
	{
	}
	virtual ~VirtVectorIter() {}
	virtual ItBase *clone()
	{
		return new VirtVectorIter(*this);
	}
	virtual bool end()
	{
		return beg == endit;
	}
	virtual IdentifierType getId()
	{
		return (IdentifierType)(size_t)*beg;
	}
	virtual void next()
	{
		++beg;
	}
	virtual size_t getChildrenCount()
	{
		return 0;
	}
	virtual IndentType getIndent()
	{
		return 0;
	}
	virtual LevelType getLevel()
	{
		return 0;
	}
	virtual DepthType getDepth()
	{
		return 0;
	}
	virtual string getName()
	{
		return StringUtils::convertToString((IdentifierType)(size_t)*beg);
	}
	virtual PositionType getPosition()
	{
		return 0;
	}
	virtual IdentifierType getConsOrder()
	{
		return NO_IDENTIFIER;
	}
	virtual Element *getElement()
	{
		return (Element *)(size_t)*beg;
	}
	virtual string getPath()
	{
		return "";
	}
private:
	ElementsType::iterator beg;
	ElementsType::iterator endit;
};

class SubSet::ElementsIter : public SubSet::ItBase
{
public:
	ElementsIter(ElementsType::iterator beg, ElementsType::iterator end, SubSet *sub) : ItBase(sub), beg(beg), endit(end)
	{
	}
	ElementsIter(const ElementsIter &o) : ItBase(o), beg(o.beg), endit(o.endit)
	{
	}
	virtual ~ElementsIter() {}
	virtual ItBase *clone()
	{
		return new ElementsIter(*this);
	}
	virtual bool end()
	{
		return beg == endit;
	}
	virtual IdentifierType getId()
	{
		return (*beg)->getIdentifier();
	}
	virtual void next()
	{
		++beg;
	}
	virtual size_t getChildrenCount()
	{
		return (*beg)->getChildrenCount();
	}
	virtual IndentType getIndent()
	{
		return (*beg)->getIndent();
	}
	virtual LevelType getLevel()
	{
		return (*beg)->getLevel();
	}
	virtual DepthType getDepth()
	{
		return (*beg)->getDepth();
	}
	virtual string getName()
	{
		return (*beg)->getName(sub->dim->getElemNamesVector());
	}
	virtual PositionType getPosition()
	{
		return (*beg)->getPosition();
	}
	virtual IdentifierType getConsOrder()
	{
		return 0;
	}
	virtual Element *getElement()
	{
		return *beg;
	}
	virtual string getPath()
	{
		return "";
	}
private:
	ElementsType::iterator beg;
	ElementsType::iterator endit;
};

SubSet::Iterator::Iterator(ItBase *impl) :
	m_impl(impl)
{
}

SubSet::Iterator::Iterator(const SubSet::Iterator &it) :
	m_impl(it.m_impl ? it.m_impl->clone() : 0)
{
}

SubSet::Iterator::Iterator() :
	m_impl(0)
{
}

SubSet::Iterator::~Iterator()
{
	delete m_impl;
}

bool SubSet::Iterator::operator!=(const Iterator &it) const
{
	return !operator ==(it);
}

bool SubSet::Iterator::operator==(const Iterator &it) const
{
	bool end1 = m_impl->end();
	bool end2 = it.m_impl->end();
	if (end1 || end2) {
		return end1 == end2;
	}
	return m_impl->getId() == it.m_impl->getId();
}

SubSet::Iterator &SubSet::Iterator::operator++()
{
	m_impl->next();
	return *this;
}

SubSet::Iterator SubSet::Iterator::operator++(int)
{
	Iterator ret(*this);
	m_impl->next();
	return ret;
}

SubSet::Iterator &SubSet::Iterator::operator=(const Iterator &it)
{
	if (m_impl) {
		delete m_impl;
	}
	m_impl = it.m_impl->clone();
	return *this;
}

IdentifierType SubSet::Iterator::getId() const
{
	return m_impl->getId();
}

size_t SubSet::Iterator::getChildrenCount() const
{
	return m_impl->getChildrenCount();
}

const CellValue &SubSet::Iterator::getValue() const
{
	return m_impl->sub->getValue(m_impl->getId());
}

IndentType SubSet::Iterator::getIndent() const
{
	return m_impl->getIndent();
}

LevelType SubSet::Iterator::getLevel() const
{
	return m_impl->getLevel();
}

DepthType SubSet::Iterator::getDepth() const
{
	return m_impl->getDepth();
}

string SubSet::Iterator::getName() const
{
	return m_impl->getName();
}

PositionType SubSet::Iterator::getPosition() const
{
	return m_impl->getPosition();
}

CellValue SubSet::Iterator::getSearchAlias(bool name) const
{
	CellValue c;
	if (name) {
		c = m_impl->getName();
	}
	return m_impl->sub->getSearchAlias(m_impl->getId(), c);
}

IdentifierType SubSet::Iterator::getConsOrder() const
{
	return m_impl->getConsOrder();
}

Element *SubSet::Iterator::getElement() const
{
	return m_impl->getElement();
}

string SubSet::Iterator::getPath() const
{
	return m_impl->getPath();
}

bool SubSet::Iterator::end() const
{
	return m_impl->end();
}

SubSet::SubSet(PDatabase db, PDimension dim, PUser user, vector<BasicFilterSettings> &basic, TextFilterSettings &text, SortingFilterSettings &sorting, AliasFilterSettings &alias, FieldFilterSettings &field, vector<StructuralFilterSettings> &structural, vector<DataFilterSettings> &data) :
	db(db), dim(dim), user(user), m_global_flags(0), basic(basic), text(text), sorting(sorting), alias(alias), field(field), structural(structural), data(data), bound(false), shrinked(false), final(false), topFilled(false)
{
}

const CellValue &SubSet::getSearchAlias(IdentifierType id, const CellValue &def)
{
	map<IdentifierType, CellValue>::iterator it = searchAlias.find(id);
	if (it != searchAlias.end()) {
		return it->second;
	} else {
		return def;
	}
}

const CellValue &SubSet::getSortingAlias(IdentifierType id, const CellValue &def)
{
	map<IdentifierType, CellValue>::iterator it = sortAlias.find(id);
	if (it != sortAlias.end()) {
		return it->second;
	} else {
		return def;
	}
}

const CellValue &SubSet::getValue(IdentifierType id)
{
	map<IdentifierType, CellValue>::iterator it = values.find(id);
	if (it != values.end()) {
		return it->second;
	} else {
		return CellValue::NullNumeric;
	}
}

void SubSet::setSearchAlias(IdentifierType id, const CellValue &alias)
{
	searchAlias[id] = alias;
}

void SubSet::setSortingAlias(IdentifierType id, const CellValue &alias)
{
	sortAlias[id] = alias;
}

void SubSet::setValue(IdentifierType id, const CellValue &val)
{
	values[id] = val;
}

void SubSet::setChildren(IdentifierType id, boost::shared_ptr<ElementsWeightType> ch)
{
	childrenMap[id] = ch;
}

boost::shared_ptr<ElementsWeightType> SubSet::getChildren(IdentifierType id)
{
	map<IdentifierType, boost::shared_ptr<ElementsWeightType> >::iterator it = childrenMap.find(id);
	if (it != childrenMap.end()) {
		return it->second;
	} else {
		return boost::shared_ptr<ElementsWeightType>();
	}
}

bool SubSet::queryGlobalFlag(GlobalFlag f)
{
	return ((m_global_flags & (unsigned long)f) != 0);
}

void SubSet::setGlobalFlag(GlobalFlag f)
{
	m_global_flags = (m_global_flags | (unsigned long)f);
}

void SubSet::mergePicklist()
{
	if (final) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "Merge after final!");
	}
	for (ElementsType::iterator it = pickMerge.begin(); it != pickMerge.end(); ++it) {
		if (elemsMap.find(*it) == elemsMap.end()) {
			elemsMap.insert(*it);
		}
		if (shrinked || dim->getDimensionType() == Dimension::VIRTUAL) {
			shrinkedList.push_back(*it);
		}
	}
	pickMerge.clear();
}

IdentifierType SubSet::validateAttribute(const string& attr)
{
	IdentifierType id = NO_IDENTIFIER;
	CPAttributesDimension ad = AttributedDimension::getAttributesDimension(db, dim->getName());
	if (ad) {
		id = ad->findElementByName(attr, user.get(), false)->getIdentifier();
	} else {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Attempt to use non-existing attribute '" + attr + "'");
	}
	return id;
}

bool SubSet::checkId(Element *el)
{
	return elemsMap.find(el) != elemsMap.end();
}

bool SubSet::checkPath(Element *elem)
{
	if (checkId(elem)) {
		return true;
	}
	for (Iterator it = childrenbegin(elem); !it.end(); ++it) {
		if (checkId(it.getElement())) {
			return true;
		}
		if (checkPath(it.getElement())) {
			return true;
		}
	}
	return false;
}

PDimension SubSet::getDimension()
{
	return dim;
}

PUser SubSet::getUser()
{
	return user;
}

PDatabase SubSet::getDatabase()
{
	return db;
}

SubSet::Iterator SubSet::begin(bool showDuplicates)
{
	if (dim->getDimensionType() == Dimension::VIRTUAL) {
		return Iterator(new VirtVectorIter(shrinkedList.begin(), shrinkedList.end(), this));
	} else if (final) {
		return Iterator(new FinalIter(this));
	} else if (shrinked && !showDuplicates) {
		return Iterator(new ElementsIter(shrinkedList.begin(), shrinkedList.end(), this));
	} else {
		return Iterator(new MapIter(elemsMap.begin(), elemsMap.end(), this));
	}
}

SubSet::Iterator SubSet::topbegin(bool nochildren)
{
	if (dim->getDimensionType() == Dimension::VIRTUAL) {
		return Iterator(new VirtVectorIter(shrinkedList.begin(), shrinkedList.end(), this));
	}
	if (!bound && ! topFilled) {
		topFilled = true;
		topElems = dim->getElements(user, true, 0);
	}
	ElementsType &curr = bound ? elemBound : topElems;
	for (ElementsType::iterator it = curr.begin(); it != curr.end();) {
		if (checkId(*it)) {
			++it;
		} else {
			if (nochildren || !checkPath(*it)) {
				it = curr.erase(it);
			} else {
				++it;
			}
		}
	}
	return Iterator(new ElementsIter(curr.begin(), curr.end(), this));
}

SubSet::Iterator SubSet::pickbegin(PickListBase::PickListFlag f)
{
	switch (f) {
	case PickListBase::INSERT_BACK:
		return Iterator(new ElementsIter(pickBack.begin(), pickBack.end(), this));
	case PickListBase::MERGE:
		return Iterator(new ElementsIter(pickMerge.begin(), pickMerge.end(), this));
	case PickListBase::SUB:
		return Iterator(new ElementsIter(pickSub.begin(), pickSub.end(), this));
	case PickListBase::INSERT_FRONT:
	default:
		return Iterator(new ElementsIter(pickFront.begin(), pickFront.end(), this));
	}
}

SubSet::Iterator SubSet::childrenbegin(Element *el)
{
	return Iterator(new ChildrenIter(el, this));
}

SubSet::Iterator SubSet::parentsbegin(Element *el)
{
	return Iterator(new ParentsIter(el, this));
}

SubSet::Iterator SubSet::vectorbegin(ElementsType &vec)
{
	return Iterator(new ElementsIter(vec.begin(), vec.end(), this));
}

size_t SubSet::size()
{
	size_t ret = 0;
	if (final) {
		ret = finalList.size();
	} else if (shrinked) {
		ret = shrinkedList.size();
	} else {
		ret = elemsMap.size();
	}
	return ret;
}

PSet SubSet::getSet(bool incPick)
{
	PSet s(new Set());
	for (Iterator it = begin(false); !it.end(); ++it) {
		s->insert(it.getId());
	}
	if (!final && incPick) {
		for (ElementsType::iterator it = pickBack.begin(); it != pickBack.end(); ++it) {
			s->insert((*it)->getIdentifier());
		}
		for (ElementsType::iterator it = pickFront.begin(); it != pickFront.end(); ++it) {
			s->insert((*it)->getIdentifier());
		}
		for (ElementsType::iterator it = pickMerge.begin(); it != pickMerge.end(); ++it) {
			s->insert((*it)->getIdentifier());
		}
		for (ElementsType::iterator it = pickBack.begin(); it != pickBack.end(); ++it) {
			s->insert((*it)->getIdentifier());
		}
	}
	return s;
}

void SubSet::addElemBound(Element *el)
{
	bound = true;
	elemBound.push_back(el);
}

void SubSet::apply()
{
	boost::posix_time::ptime tStart = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime tSt = boost::posix_time::microsec_clock::local_time();
	ElementsType els = dim->getElements(user, false, 0);
	elemsMap.insert(els.begin(), els.end());
	els.clear();
	size_t dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Init " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	bool worked = false;
	for (vector<BasicFilterSettings>::iterator it = basic.begin(); it != basic.end(); ++it) {
		if (it->active) {
			PickList p(*this, *it);
			ElementsType res = p.apply();
			if (p.queryFlag(PickListBase::INSERT_BACK)) {
				pickBack.insert(pickBack.end(), res.begin(), res.end());
			} else if (p.queryFlag(PickListBase::MERGE)) {
				pickMerge.insert(pickMerge.end(), res.begin(), res.end());
			} else if (p.queryFlag(PickListBase::SUB)) {
				worked = true;
				pickSub.insert(pickSub.end(), res.begin(), res.end());
			} else if (p.queryFlag(PickListBase::DFILTER)) {
				pickDFilter.insert(pickDFilter.end(), res.begin(), res.end());
			} else {
				pickFront.insert(pickFront.end(), res.begin(), res.end());
			}
		}
	}
	if (worked) {
		updateMap(pickSub);
		worked = false;
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Picklist " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	vector<SubElem> stru;
	for (vector<StructuralFilterSettings>::iterator it = structural.begin(); it != structural.end(); ++it) {
		if (it->active) {
			StructuralFilter s(*this, *it);
			vector<SubElem> r = s.apply(worked);
			stru.insert(stru.end(), r.begin(), r.end());
		}
	}
	set<Element *> strset;
	if (worked) {
		shrinked = true;
		elemsMap.clear();
		for (vector<SubElem>::iterator it = stru.begin(); it != stru.end(); ++it) {
			elemsMap.insert(it->elem);
			shrinkedList.push_back(it->elem);
		}
		strset = elemsMap;
		stru.clear();
		worked = false;
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Structural " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	if (alias.active || field.active) {
		AliasFilter a(*this, alias, field);
		ElementsType res = a.apply(worked);
		if (worked) {
			shrinked = true;
			shrinkedList = res;
			updateMap(shrinkedList);
			worked = false;
		}
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Alias " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	if (text.active) {
		TextFilter t(*this, text, alias.active ? TextFilter::ALIAS : TextFilter::BASIC, "");
		ElementsType res = t.apply();
		shrinked = true;
		shrinkedList = res;
		updateMap(shrinkedList);
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Text " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	if (queryGlobalFlag(SubSet::PICKLIST_DFILTER)) {
		shrinked = true;
		shrinkedList.swap(pickDFilter);
		updateMap(shrinkedList);
	}
	ElementsType df_result;
	for (vector<DataFilterSettings>::iterator it = data.begin(); it != data.end(); ++it) {
		if (it->active) {
			DataFilter d(*this, *it);
			ElementsType r = d.apply();
			df_result.insert(df_result.end(), r.begin(), r.end());
			worked = true;
		}
	}
	if (worked) {
		worked = false;
		shrinked = true;
		shrinkedList = df_result;
		updateMap(shrinkedList);
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Data " << dur << "ms" << endl;
	tSt = boost::posix_time::microsec_clock::local_time();
	if (queryGlobalFlag(SubSet::PICKLIST_MERGE)) {
		mergePicklist();
	}
	if (!sorting.active) {
		sorting.active = true;
		sorting.flags = SortingFilter::FLAT_HIERARCHY | SortingFilter::POSITION;
		sorting.limit_count = 0;
	}
	if (dim->getDimensionType() != Dimension::VIRTUAL) {
		SortingFilter srt(*this, sorting);
		if (!srt.queryFlag(SortingFilterBase::NOSORT)) {
			finalList = srt.apply();
			final = true;
		}
		if (queryGlobalFlag(LEVEL_BOUNDS)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			for (vector<SubElem>::iterator itIds = finalList.begin(); itIds != finalList.end();) {
				if (strset.find(itIds->elem) == strset.end()) {
					vector<StructuralFilterSettings>::iterator it = structural.begin();
					for (; it != structural.end(); ++it) {
						const unsigned int start = it->level_start;
						const unsigned int end = it->level_end;
						bool br = false;
						switch (it->indent) {
						case 2:
							{
								if (itIds->elem->getLevel() >= start && itIds->elem->getLevel() <= end) {
									br = true;
								}
							}
							break;
						case 3:
							if (itIds->dep >= start && itIds->dep <= end) {
								br = true;
							}
							break;
						default:
							if (itIds->ind >= start && itIds->ind <= end) {
								br = true;
							}
							break;
						}
						if (br) {
							break;
						}
					}
					if (it == structural.end()) {
						itIds = finalList.erase(itIds);
					} else {
						++itIds;
					}
				} else {
					++itIds;
				}
			}
		}
		if (queryGlobalFlag(PICKLIST_FRONT)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			vector<SubElem> tmpIds;
			for (Iterator it = pickbegin(PickListBase::INSERT_FRONT); !it.end(); ++it) {
				tmpIds.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), srt.queryFlag(SortingFilter::PATH) ? StringUtils::convertToString(it.getId()) : ""));
			}
			tmpIds.insert(tmpIds.end(), finalList.begin(), finalList.end());
			finalList = tmpIds;
		}
		if (queryGlobalFlag(PICKLIST_BACK)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			for (Iterator it = pickbegin(PickListBase::INSERT_BACK); !it.end(); ++it) {
				finalList.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), srt.queryFlag(SortingFilter::PATH) ? StringUtils::convertToString(it.getId()) : ""));
			}
		}
		if (queryGlobalFlag(PICKLIST_SUB)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			set<IdentifierType> pick;
			for (Iterator it = pickbegin(PickListBase::SUB); !it.end(); ++it) {
				pick.insert(it.getId());
			}
			for (vector<SubElem>::iterator itIds = finalList.begin(); itIds != finalList.end();) {
				if (pick.find(itIds->elem->getIdentifier()) == pick.end()) {
					itIds = finalList.erase(itIds);
				} else {
					++itIds;
				}
			}
		}
		if (queryGlobalFlag(REVOLVE)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			for (vector<StructuralFilterSettings>::iterator it = structural.begin(); it != structural.end(); ++it) {
				if ((it->flags & StructuralFilter::REVOLVING) || (it->flags  & StructuralFilter::REVOLVE_ADD_ABOVE) || (it->flags & StructuralFilter::REVOLVE_ADD_BELOW)) {
					unsigned int count = it->revolve_count;
					size_t index = finalList.size();
					if (index >= count) {
						finalList.resize(count);
					} else {
						std::back_insert_iterator<vector<SubElem> > back_it(finalList);
						size_t stop_it = finalList.size();
						size_t repeat_it = 0;
						if (repeat_it != stop_it) {
							while (index < count) {
								*back_it = finalList[repeat_it];
								++back_it;
								++index;
								++repeat_it;
								if (repeat_it == stop_it) {
									repeat_it = 0;
								}
							}
						}
					}
					break;
				}
			}
		}

		if (srt.queryFlag(SortingFilter::REVERSE_TOTAL_EX) ^ srt.m_parents_below) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			vector<SubElem> tmpIds;
			tmpIds.insert(tmpIds.end(), finalList.rbegin(), finalList.rend());
			finalList = tmpIds;
		}

		if (srt.queryFlag(SortingFilter::LIMIT)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			vector<SubElem> tmpIds;
			if (srt.m_limit_start > finalList.size()) {
				srt.m_limit_start = (unsigned int)finalList.size();
			}
			if (srt.m_limit_start + srt.m_limit_count > finalList.size()) {
				srt.m_limit_count = (unsigned int)finalList.size() - srt.m_limit_start;
			}
			tmpIds.insert(tmpIds.end(), finalList.begin() + srt.m_limit_start, finalList.begin() + srt.m_limit_start + srt.m_limit_count);
			finalList = tmpIds;
		}
		if (srt.queryFlag(SortingFilter::FLAT_HIERARCHY) || !srt.queryFlag(SortingFilter::WHOLE)) {
			makeFinal(srt.queryFlag(SortingFilter::PATH));
			for (vector<SubElem>::iterator it = finalList.begin(); it != finalList.end(); ++it) {
				it->ind = 0;
			}
		}
	}
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tSt).total_milliseconds();
	Logger::debug << "Sorting " << dur << "ms" << endl;
	dur = (size_t)(boost::posix_time::microsec_clock::local_time() - tStart).total_milliseconds();
	Logger::debug << "Subset " << dur << "ms" << endl;
}

void SubSet::updateMap(const ElementsType &list)
{
	set<Element *> tmpMap;
	for (ElementsType::const_iterator it = list.begin(); it != list.end(); ++it) {
		set<Element *>::iterator sit = elemsMap.find(*it);
		if (sit != elemsMap.end()) {
			tmpMap.insert(*sit);
		}
	}
	elemsMap = tmpMap;
}

void SubSet::makeFinal(bool usePath)
{
	if (!final) {
		for (Iterator it = begin(true); !it.end(); ++it) {
			finalList.push_back(SubElem(it.getElement(), it.getIndent(), it.getDepth(), usePath ? StringUtils::convertToString(it.getId()) : ""));
		}
		final = true;
	}
}

} //palo
