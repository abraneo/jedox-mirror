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

#include "Collections/NameIdSlimMap.h"
#include "Collections/StringUtils.h"


namespace palo {


NameIdSlimMap::const_iterator NameIdSlimMap::find(const string &str) const
{
//	print();
	const_iterator itBegin = begin();
	const_iterator itEnd = end();
	return find(itBegin, itEnd, str, false);
}


NameIdSlimMap::const_iterator NameIdSlimMap::find(NameIdSlimMap::const_iterator &itBegin, NameIdSlimMap::const_iterator &itEnd, const string &str, bool returnClosest) const
{
	if (itBegin == itEnd) {
		if (itEnd.page != values->pageCount()) {
			if (!UTF8Comparer::compare(strVector->getString(itEnd.first()).c_str(), str.c_str())) {
				return itEnd;
			} else {
				if (returnClosest) {
					return itBegin;
				} else {
					return end();
				}
			}
		} else {
			// not found, itEnd == end()
			if (returnClosest) {
				return itBegin;
			} else {
				return end();
			}
		}
	} else {
		ptrdiff_t diff = (itEnd - itBegin) / 2;
		if (diff) {
			const_iterator itMiddle = itBegin + diff;
			if (key_compare(str, strVector->getString(itMiddle.first()))) {
				return find(itBegin, itMiddle, str, returnClosest);
			} else {
				return find(itMiddle, itEnd, str, returnClosest);
			}
		} else {
			return find(itBegin, itBegin, str, returnClosest);
		}
	}
}


void NameIdSlimMap::print() const
{
	for (const_iterator it = begin(); it != end(); ++it) {
		cout << "PAGE " << it.page << ":" << " " << key_compare.getStringVector()->getString(it.first()) << " " << it.second() << endl;
	}
	cout << "SIZE: " << this->size() << endl;
}


bool NameIdSlimMap::merge(const CPCommitable &o, const PCommitable &p)
{
	bool ret = true;
	checkCheckedOut();
	mergeint(o, p);

	CPNameIdSlimMap oldMap = boost::dynamic_pointer_cast<const NameIdSlimMap, const Commitable>(old);
	CPNameIdSlimMap map = boost::dynamic_pointer_cast<const NameIdSlimMap, const Commitable>(o);

	if (ret) {
		// return false if any conflict in keys or values
		if (map && oldMap) {
			if (keys != map->keys && map->keys != oldMap->keys) {
				ret = false;
			}
			if (values != map->values && map->values != oldMap->values) {
				ret = false;
			}
			if (strVector != map->strVector && map->strVector != oldMap->strVector) {
				ret = false;
			}
		}
	}

	if (ret && keys->isCheckedOut()) {
		ret = keys->merge(map ? map->keys : Slim<StringVector::StringId>::PSlimVector(), shared_from_this());
	}

	if (ret && values->isCheckedOut()) {
		ret = values->merge(map ? map->values : Slim<IdentifierType>::PSlimVector(), shared_from_this());
	}

	if (ret && strVector->isCheckedOut()) {
		ret = strVector->merge(map ? map->strVector : PStringVector(), shared_from_this());
	}

	if (ret) {
		commitintern();
	}

	return ret;
}


PCommitable NameIdSlimMap::copy() const
{
	checkNotCheckedOut();
	boost::shared_ptr<NameIdSlimMap> newMap(new NameIdSlimMap(*this));
	return newMap;
}


}
