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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef STRINGMAP_H_
#define STRINGMAP_H_


#include "SlimMap.h"
#include "StringVector.h"
#include "Collections/StringUtils.h"

namespace palo {


struct ElementStringIdCmp {
	bool operator()(const StringVector::StringId &id1, const StringVector::StringId &id2) const {
		return strVector->casecmp(id1, id2);
	}

	bool operator()(const string &str1, const string &str2) const {
		return UTF8Comparer::compare(str1, str2) < 0;
	}

	void setStringVector(CPStringVector v) {
		strVector = v;
	}
	CPStringVector getStringVector() const {
		return strVector;
	}

private:
	CPStringVector strVector;
};


class NameIdSlimMap : public SlimMap<StringVector::StringId, IdentifierType, ElementStringIdCmp> {
public:
	NameIdSlimMap(uint32_t blockSize) : SlimMap<StringVector::StringId, IdentifierType, ElementStringIdCmp>(blockSize), strVector(PStringVector(new StringVector(4096))) {
		setStringVector(strVector);
	}

	const_iterator find(const string &str) const;
	virtual void print() const;
	void setStringVector(PStringVector strVector) {
		key_compare.setStringVector(strVector);
	}
	StringVector & getStringVector() {
		return *strVector;
	}
	StringVector::StringId pushToVector(const string &name) {
		return strVector->push(name);
	}
	void clear() {
		strVector->clear();
		SlimMap<StringVector::StringId, IdentifierType, ElementStringIdCmp>::clear();
	}
	void erase(StringVector::StringId id) {
		strVector->erase(id);
		SlimMap<StringVector::StringId, IdentifierType, ElementStringIdCmp>::erase(id);
	}

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	PStringVector strVector;

protected:
	NameIdSlimMap(const NameIdSlimMap &map) : SlimMap<StringVector::StringId, IdentifierType, ElementStringIdCmp>(map), strVector(boost::dynamic_pointer_cast<StringVector, Commitable>(map.strVector->copy())) {
		setStringVector(strVector);
	}

	const_iterator find(const_iterator &itBegin, const_iterator &itEnd, const string &str, bool returnClosest) const;
};


typedef boost::shared_ptr<NameIdSlimMap> PNameIdSlimMap;
typedef boost::shared_ptr<const NameIdSlimMap> CPNameIdSlimMap;



}


#endif /* STRINGMAP_H_ */
