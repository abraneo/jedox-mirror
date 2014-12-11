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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Christoffer Anselm, Jedox AG, Freiburg, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Tobias Lauer, Jedox AG, Freiburg, Germany
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef OLAP_ELEMENT_H
#define OLAP_ELEMENT_H 1

#include "palo.h"
#include "Commitable.h"
#include "CommitableList.h"
#include "Collections/StringVector.h"
#include "Engine/Area.h"

namespace palo {
class Dimension;

class Parents {
public:
	class const_iterator : public std::iterator<random_access_iterator_tag, IdentifierType, ptrdiff_t, IdentifierType *, IdentifierType &> {
	private:
		const_iterator(const Parents *parents, uint32_t index) :
			parents(parents), index(index)
		{
		}
	public:
		const_iterator(const const_iterator &it) : parents(it.parents), index(it.index)
		{
		}
		bool operator!=(const const_iterator &it) const
		{
			return index != it.index;
		}
		bool operator==(const const_iterator &it) const
		{
			return index == it.index;
		}
		bool operator<(const const_iterator &it) const
		{
			return index < it.index;
		}
		ptrdiff_t operator-(const const_iterator &it) const
		{
			return index - it.index;
		}
		const_iterator operator+(ptrdiff_t i)
		{
			index += (uint32_t)i;
			return *this;
		}
		const_iterator & operator++()
		{
			index++;
			return *this;
		}
		IdentifierType operator*()
		{
			return parents->at(index);
		}
	private:
		const Parents *parents;
		uint32_t index;
		friend class Parents;
	};

public:
	Parents() :
		parents(NULL), parentCount(0)
	{
	}
	Parents(const Parents &par) : singleParent(par.singleParent), parentCount(par.parentCount)
	{
		if (par.parents) {
			parents = new IdentifiersType(*par.parents);
		} else {
			parents = NULL;
		}
	}
	virtual ~Parents()
	{
		delete parents;
	}
	uint32_t size() const
	{
		return parentCount;
	}
	bool empty() const
	{
		return parentCount == 0;
	}
	IdentifierType at(uint32_t index) const;
	const_iterator begin() const
	{
		return const_iterator(this, 0);
	}
	const_iterator end() const
	{
		return const_iterator(this, parentCount);
	}
	void clear()
	{
		parentCount = 0;
		delete parents;
		parents = NULL;
	}
	void erase(const_iterator it);
	void push_back(IdentifierType id);

private:
	IdentifierType singleParent;
	IdentifiersType *parents;
	uint32_t parentCount;
};

typedef boost::shared_ptr<Parents> PParents;
typedef boost::shared_ptr<const Parents> CPParents;

////////////////////////////////////////////////////////////////////////////////
/// @brief element relations
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Relations {
public:
	Relations() :
		children(NULL), parents(), baseElements(NULL), isStringConsolidation(0), checkedOut(true)
	{
	}
	virtual ~Relations()
	{
		delete children;
		delete baseElements;
	}
	static PRelations checkOut(PRelations current);
	bool isCheckedOut() const
	{
		return checkedOut;
	}
	void checkCheckedOut() const
	{
		if (!isCheckedOut())
			throw CommitException(ErrorException::ERROR_COMMIT_OBJECTNOTCHECKEDOUT, "Object not checked out");
	}
	void commit()
	{
		checkedOut = 0;
	}
private:
	Relations(const Relations &r);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief all children internal Ids and weights
	////////////////////////////////////////////////////////////////////////////////
	IdentifiersWeightType *children;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief all parents internal Ids
	////////////////////////////////////////////////////////////////////////////////
	PParents parents;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief base elements
	/// all base elements intervals and their weights
	////////////////////////////////////////////////////////////////////////////////

	WeightedSet *baseElements;

	static BaseRangesWeightType emptyRanges;
	static PWeightedSet emptyBaseElements;

	friend class Element;

	static IdentifiersWeightType emptyChildren;
	static PParents emptyParents;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief 1 if consolidated element has string descendant
	////////////////////////////////////////////////////////////////////////////////
	uint8_t isStringConsolidation : 1;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief 1 if object is checked-out
	////////////////////////////////////////////////////////////////////////////////
	uint8_t checkedOut : 1;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief palo element
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Element {
public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief Element types
	////////////////////////////////////////////////////////////////////////////////

	enum Type {
		UNDEFINED = 0, NUMERIC = 1, STRING = 2, CONSOLIDATED = 4,
	};

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new element
	////////////////////////////////////////////////////////////////////////////////

	Element() :
		identifier(NO_IDENTIFIER), position(NO_IDENTIFIER), type(UNDEFINED), level(0), depth(0)
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new element with a identifier
	////////////////////////////////////////////////////////////////////////////////

	Element(IdentifierType identifier) :
		identifier(identifier), position(0), type(UNDEFINED), level(0), depth(0)
	{
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets identifier of element
	////////////////////////////////////////////////////////////////////////////////

	void setIdentifier(IdentifierType identifier)
	{
		this->identifier = identifier;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets identifier of element
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getIdentifier() const
	{
		return identifier;
	}
	;

	//	////////////////////////////////////////////////////////////////////////////////
	//	/// @brief sets or rename element
	//	////////////////////////////////////////////////////////////////////////////////
	//
	//	void setName(const string& name)
	//	{
	//		this->name = name;
	//	}
	//	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets element name
	////////////////////////////////////////////////////////////////////////////////

	void setName(const StringVector::StringId &id)
	{
		this->nameId = id;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets element name
	////////////////////////////////////////////////////////////////////////////////

	//	const string& getName() const
	//	{
	//		return name;
	//	}
	string getName(const StringVector &v) const
	{
		if (nameId.notSet()) {
			return "";
		} else {
			return v.getString(nameId);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets element name StringId
	////////////////////////////////////////////////////////////////////////////////

	StringVector::StringId getNameId() const
	{
		return nameId;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets element type
	////////////////////////////////////////////////////////////////////////////////

	void setElementType(Element::Type type)
	{
		this->type = type;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets element type
	////////////////////////////////////////////////////////////////////////////////

	Element::Type getElementType() const
	{
		return (Type)type;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets position
	////////////////////////////////////////////////////////////////////////////////

	void setPosition(PositionType position)
	{
		this->position = position;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets position
	////////////////////////////////////////////////////////////////////////////////

	PositionType getPosition() const
	{
		return position;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets level
	////////////////////////////////////////////////////////////////////////////////

	void setLevel(LevelType level)
	{
		this->level = level;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets level
	////////////////////////////////////////////////////////////////////////////////

	LevelType getLevel() const
	{
		return level;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets indent
	////////////////////////////////////////////////////////////////////////////////

	void setIndent(IndentType indent)
	{
		this->indent = indent;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets indent
	////////////////////////////////////////////////////////////////////////////////

	IndentType getIndent() const
	{
		return indent;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets depth
	////////////////////////////////////////////////////////////////////////////////

	void setDepth(DepthType depth)
	{
		this->depth = depth;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets depth
	////////////////////////////////////////////////////////////////////////////////

	DepthType getDepth() const
	{
		return depth;
	}
	;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief computes ranges of base elements this element consists of
	///
	/// Computes contiguous ranges of base elements for consolidated elements
	/// prototype implementation by Tobias Lauer modified by Christoffer Anselm
	/// This code creates an ascending ordered vector of ranges
	///
	/// @author Tobias Lauer
	////////////////////////////////////////////////////////////////////////////////

	CPParents getParents() const;
	PParents getParents(bool writable);
	void setParents(PParents parents);
	size_t getParentsCount() const;

	const IdentifiersWeightType *getChildren() const;
	IdentifiersWeightType *getChildren(bool writable);
	size_t getChildrenCount() const;

	const WeightedSet * getBaseElements() const;
	WeightedSet * getBaseElements(bool writable);
	size_t getBaseElementsCount() const;
	WeightedSet::const_iterator baseElementsBegin() const;
	WeightedSet::const_iterator baseElementsEnd() const;
	void baseElementsClear();
	void setBaseElements(WeightedSet *ws);

	bool isStringConsolidation() const;
	void setStringConsolidation(bool);

	void release();
private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the identifier of the element
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType identifier;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the name of the element
	////////////////////////////////////////////////////////////////////////////////

	//string name;
	StringVector::StringId nameId;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the position of the element in the list of elements
	////////////////////////////////////////////////////////////////////////////////

	PositionType position;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the type of the element
	////////////////////////////////////////////////////////////////////////////////

	unsigned type :3;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the level of the element
	////////////////////////////////////////////////////////////////////////////////

	unsigned level :9;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the depth of the element
	////////////////////////////////////////////////////////////////////////////////

	unsigned depth :9;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the indent of the element
	////////////////////////////////////////////////////////////////////////////////

	unsigned indent :9;

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief the relations description of the element
	////////////////////////////////////////////////////////////////////////////////

	PRelations relations;

	friend class ElementList;
};

#define ELEMENTS_PER_PAGE 100

class SERVER_CLASS ElementPage {
public:
	ElementPage() :
		size(0)
	{
	}
	;
	PElementPage copy() const
	{
		return PElementPage(new ElementPage(*this));
	}
	;
private:
	Element elements[ELEMENTS_PER_PAGE];
	size_t size;

	friend class ElementList;
};

class SERVER_CLASS ElementList : public Commitable {
public:
	ElementList() :
		Commitable(""), idh(new IdHolder())
	{
	}
	;

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	void checkOut(IdentifierType id);

	void setNewIDStart(IdentifierType newstart)
	{
		idh->setStart(newstart);
	}
	IdentifierType getLastId()
	{
		return idh->getLastId();
	}

	void clear()
	{
		freeInternals.clear();
		pages.clear();
	}
	;
	size_t size() const
	{
		return pages.size() * ELEMENTS_PER_PAGE;
	}
	;
	IdentifierType addElement(const Element &, PDatabase db, const Dimension *dim);
	void deleteElement(IdentifierType internalId);
	const Element& operator[](const size_t& pos) const;
	Element& operator[](const size_t& pos);

	class const_iterator {
		friend class ElementList;
	public:
		const_iterator();
		const_iterator operator=(const const_iterator &iter);
		bool operator==(const const_iterator &iter) const;
		bool operator!=(const const_iterator &iter) const;
		const_iterator &operator++(); //++o
		const_iterator operator++(int i); //o++
		Element &operator*() const;
	private:
		const_iterator(const vector<PElementPage> &pgs, vector<PElementPage>::const_iterator &i);

		const vector<PElementPage> *pages;
		vector<PElementPage>::const_iterator it;
		size_t pagePos;
	};

	const_iterator begin() const;
	const_iterator end() const;

private:
	IdentifiersType freeInternals; // free internal slots for recycling
	std::vector<PElementPage> pages;
	PIdHolder idh;

	friend class Dimension;
};

typedef boost::shared_ptr<ElementList> PElementList;
typedef boost::shared_ptr<const ElementList> CPElementList;

}

#endif
