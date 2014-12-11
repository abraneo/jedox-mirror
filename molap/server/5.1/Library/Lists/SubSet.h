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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef _SUBSET_H
#define _SUBSET_H

#include <string>

#include "Filter.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {

class TextFilter;
class SortingFilter;
class PickList;
class StructuralFilter;
class AliasFilter;
class DataFilter;
class PostProcessor;
class AbstractComparison;

struct SubElem {
	Element *elem;
	IndentType ind;
	DepthType dep;
	string path;
	SubElem() : elem(0), ind(NO_IDENTIFIER), dep(NO_IDENTIFIER) {}
	SubElem(Element *elem, IndentType ind, DepthType dep, const string &path) : elem(elem), ind(ind), dep(dep), path(path) {}
};

class SubSet {
public:
	enum GlobalFlag {
		DATA_ONLY_LEAVES = 0x1,
		DATA_ONLY_CONSOLIDATED = 0x2,
		DATA_FILTER_ACTIVE = 0x4,
		ALIAS_FILTER_ACTIVE = 0x8,
		PICKLIST_MERGE = 0x10,
		PICKLIST_FRONT = 0x20,
		PICKLIST_BACK = 0x40,
		PICKLIST_SUB = 0x80,
		REVOLVE = 0x100,
		PICKLIST_DFILTER = 0x200,
		DATA_STRING = 0x400,
		REVERSE = 0x800,
		DONT_SHOW_DUPLICATES = 0x1000,
		LEVEL_BOUNDS = 0x2000,
		BELOW_EXCLUSIVE = 0x4000,
		BELOW_INCLUSIVE = 0x8000,
		STRUCTURAL_FILTER_ACTIVE = 0x10000

	};

	class ItBase;
	class MapIter;
	class FinalIter;
	class ChildrenIter;
	class ParentsIter;
	class VirtVectorIter;
	class ElementsIter;
	friend struct SortElem;

	class Iterator {
	public:
		Iterator(ItBase *impl);
		Iterator(const Iterator &it);
		Iterator();
		~Iterator();
		bool operator!=(const Iterator &it) const;
		bool operator==(const Iterator &it) const;
		Iterator &operator++();
		Iterator operator++(int);
		Iterator &operator=(const Iterator &it);
		IdentifierType getId() const;
		size_t getChildrenCount() const;
		const CellValue &getValue() const;
		IndentType getIndent() const;
		LevelType getLevel() const;
		DepthType getDepth() const;
		string getName() const;
		PositionType getPosition() const;
		CellValue getSearchAlias(bool name) const;
		IdentifierType getConsOrder() const;
		Element *getElement() const;
		string getPath() const;
		bool end() const;
	private:
		ItBase *m_impl;
	};

	SubSet(PDatabase db, PDimension dim, PUser user, vector<BasicFilterSettings> &basic, TextFilterSettings &text, SortingFilterSettings &sorting, AliasFilterSettings &alias, FieldFilterSettings &field, vector<StructuralFilterSettings> &structural, vector<DataFilterSettings> &data);

	const CellValue &getSearchAlias(IdentifierType id, const CellValue &def);
	const CellValue &getSortingAlias(IdentifierType id, const CellValue &def);
	const CellValue &getValue(IdentifierType id);
	void setSearchAlias(IdentifierType id, const CellValue &alias);
	void setSortingAlias(IdentifierType id, const CellValue &alias);
	void setValue(IdentifierType id, const CellValue &val);

	void setChildren(IdentifierType id, boost::shared_ptr<ElementsWeightType> ch);
	boost::shared_ptr<ElementsWeightType> getChildren(IdentifierType id);

	bool queryGlobalFlag(GlobalFlag f);
	void setGlobalFlag(GlobalFlag f);
	void mergePicklist();
	IdentifierType validateAttribute(const string& attr);
	bool checkId(Element *el);
	bool checkPath(Element *elem);
	PDimension getDimension();
	PUser getUser();
	PDatabase getDatabase();
	Iterator begin(bool showDuplicates);
	Iterator topbegin(bool nochildren);
	Iterator pickbegin(PickListBase::PickListFlag f);
	Iterator childrenbegin(Element *el);
	Iterator parentsbegin(Element *el);
	Iterator vectorbegin(ElementsType &vec);
	size_t size();
	PSet getSet(bool incPick);
	void addElemBound(Element *el);
	void apply();

private:
	void updateMap(const ElementsType &list);
	void makeFinal(bool usePath);

	PDatabase db;
	PDimension dim;
	PUser user;
	unsigned long m_global_flags;
	vector<BasicFilterSettings> &basic;
	TextFilterSettings &text;
	SortingFilterSettings &sorting;
	AliasFilterSettings &alias;
	FieldFilterSettings &field;
	vector<StructuralFilterSettings> &structural;
	vector<DataFilterSettings> &data;

	map<IdentifierType, CellValue> values;
	map<IdentifierType, CellValue> searchAlias;
	map<IdentifierType, CellValue> sortAlias;
	map<IdentifierType, boost::shared_ptr<ElementsWeightType> > childrenMap;
	bool bound;
	ElementsType elemBound;
	ElementsType topElems;
	set<Element *> elemsMap;
	bool shrinked;
	bool final;
	bool topFilled;
	ElementsType shrinkedList;
	vector<SubElem> finalList;
	ElementsType pickFront;
	ElementsType pickMerge;
	ElementsType pickBack;
	ElementsType pickSub;
	ElementsType pickDFilter;
};

struct SortElem {
	SubSet *sub;
	Element *el;
	DepthType dep;
	IndentType ind;
	IdentifierType cons;
	U_NAMESPACE_QUALIFIER UnicodeString uname;
	CellValue val;

	SortElem() : sub(0), el(0), dep(NO_IDENTIFIER), ind(NO_IDENTIFIER), cons(0) {}
	SortElem(SubSet *sub, Element *el, IndentType ind, DepthType dep, IdentifierType cons) : sub(sub), el(el), dep(dep), ind(ind), cons(cons) {}
	const U_NAMESPACE_QUALIFIER UnicodeString &getUName() const {if (uname.isEmpty()) const_cast<SortElem *>(this)->uname = U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(el->getName(sub->dim->getElemNamesVector()).c_str()); return uname;}
	const CellValue &getSearchAlias() const {if (val.isEmpty()) {const_cast<SortElem *>(this)->val = el->getName(sub->dim->getElemNamesVector()); const_cast<SortElem *>(this)->val = sub->getSearchAlias(el->getIdentifier(), val);} return val;}
	const CellValue &getValue() const {return sub->getValue(el->getIdentifier());}
	const CellValue &getSortingAlias() const {if (val.isEmpty()) {const_cast<SortElem *>(this)->val = el->getName(sub->dim->getElemNamesVector()); const_cast<SortElem *>(this)->val = sub->getSortingAlias(el->getIdentifier(), val);} return val;}
};

typedef boost::shared_ptr<SubSet> PSubSet;

} //palo
#endif
