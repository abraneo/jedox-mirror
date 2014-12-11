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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "CommitableList.h"
#include "Thread/WriteLocker.h"
#include "Context.h"

namespace palo {

IdentifierType IdHolder::getNewId()
{
	WriteLocker wl(&idlock);
	IdentifierType ret = id;
	id++;
	return ret;
}

void IdHolder::setStart(IdentifierType newid)
{
	WriteLocker wl(&idlock);
	id = newid;
}

CommitableList::Iterator::Iterator(const map<IdentifierType, PCommitable>::iterator &i) :
	it(i)
{
}

CommitableList::Iterator CommitableList::Iterator::operator=(const CommitableList::Iterator &iter)
{
	it = iter.it;
	return *this;
}

bool CommitableList::Iterator::operator==(const CommitableList::Iterator &iter) const
{
	return it == iter.it;
}

bool CommitableList::Iterator::operator!=(const CommitableList::Iterator &iter) const
{
	return it != iter.it;
}

CommitableList::Iterator CommitableList::Iterator::operator++(int i)
{
	Iterator ret = *this;
	++it;
	return ret;
}

CommitableList::Iterator CommitableList::Iterator::operator++()
{
	++it;
	return *this;
}

const PCommitable &CommitableList::Iterator::operator*() const
{
	return it->second;
}

PCommitable CommitableList::Iterator::getCopy()
{
	if (it->second->isCheckedOut()) {
		return it->second;
	} else {
		return it->second->copy();
	}
}

CommitableList::ConstIterator::ConstIterator(const map<IdentifierType, PCommitable>::const_iterator &i) :
	it(i)
{
}

CommitableList::ConstIterator CommitableList::ConstIterator::operator=(const CommitableList::ConstIterator &iter)
{
	it = iter.it;
	return *this;
}

bool CommitableList::ConstIterator::operator==(const CommitableList::ConstIterator &iter) const
{
	return it == iter.it;
}

bool CommitableList::ConstIterator::operator!=(const CommitableList::ConstIterator &iter) const
{
	return it != iter.it;
}

CommitableList::ConstIterator CommitableList::ConstIterator::operator++(int i)
{
	ConstIterator ret = *this;
	++it;
	return ret;
}

CommitableList::ConstIterator CommitableList::ConstIterator::operator++()
{
	++it;
	return *this;
}

CPCommitable CommitableList::ConstIterator::operator*() const
{
	return it->second;
}

CommitableList::CommitableList(const PIdHolder &newidh, bool ignoreNames) :
	Commitable(""), idh(newidh), ignoreNames(ignoreNames)
{
	if (idh.get() == 0) {
		idh.reset(new IdHolder());
	}
}

CommitableList::CommitableList(bool ignoreNames) :
	Commitable(""), idh(new IdHolder()), ignoreNames(ignoreNames)
{
}

CommitableList::CommitableList(const CommitableList &l) :
	Commitable(l), idh(l.idh), ids(l.ids), names(l.names), ignoreNames(l.ignoreNames)
{
}

void CommitableList::add(const PCommitable &item, bool newid)
{
	checkCheckedOut();
	if (newid) {
		item->setID(idh->getNewId());
	}
	ids.insert(pair<IdentifierType, PCommitable>(item->getId(), item));
	if (!ignoreNames) {
		names.insert(pair<string, IdentifierType>(item->getName(), item->getId()));
	}
}

void CommitableList::clear()
{
	checkCheckedOut();
	ids.clear();
	names.clear();
}

IdentifierType CommitableList::size() const
{
	return (IdentifierType)ids.size();
}

PCommitable CommitableList::get(IdentifierType id, bool write) const
{
	PCommitable ret;
	map<IdentifierType, PCommitable>::const_iterator it = ids.find(id);
	if (it != ids.end()) {
		ret = it->second;
		if (write && !ret->isCheckedOut()) {
			ret = ret->copy();
		}
	}
	return ret;
}

PCommitable CommitableList::copy() const
{
	checkNotCheckedOut();
	return createnew(*this);
}

PCommitable CommitableList::get(const string &name, bool write) const
{
	if (ignoreNames) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot search by name in this list");
	}

	PCommitable ret;
	map<string, IdentifierType, UTF8Comparer>::const_iterator nit = names.find(name);
	if (nit != names.end()) {
		map<IdentifierType, PCommitable>::const_iterator it = ids.find(nit->second);
		if (it == ids.end()) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "CommitableList::get item not found!");;
		}
		ret = it->second;
		if (write && !ret->isCheckedOut()) {
			ret = ret->copy();
		}
	}
	return ret;
}

void CommitableList::set(const PCommitable &item)
{
	checkCheckedOut();
	map<IdentifierType, PCommitable>::iterator it = ids.find(item->getId());
	if (it == ids.end()) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "CommitableList::set item not found!");;
	}
	it->second = item;
}

void CommitableList::remove(IdentifierType id)
{
	checkCheckedOut();
	map<IdentifierType, PCommitable>::const_iterator it = ids.find(id);
	if (!ignoreNames) {
		names.erase(it->second->getName());
	}
	ids.erase(id);
}

void CommitableList::erase(Iterator it)
{
	remove((*it)->getId());
}

template<typename TKey, typename TValue>
struct ExtractPairFirst : public unary_function<pair<TKey, TValue> , const TKey> {
	typename unary_function<pair<TKey, TValue> , const TKey>::result_type operator()(const typename unary_function<pair<TKey, TValue> , const TKey>::argument_type &arg) const {
		return arg.first;
	}
};

bool CommitableList::merge(const CPCommitable &o, const PCommitable &p)
{
	bool ret = true;
	mergeint(o,p);
	CPCommitableList list = CONST_COMMITABLE_CAST(CommitableList, o);
	if (list != 0) {
		list->checkNotCheckedOut();
	}
	for (map<IdentifierType, PCommitable>::iterator it = ids.begin(); it != ids.end(); ++it) {
		if (it->second->isCheckedOut()) {
			if (list != 0) {
				map<IdentifierType, PCommitable>::const_iterator itl = list->ids.find(it->first);
				if (itl != list->ids.end()) {
					ret = it->second->merge(itl->second, p);
				} else {
					ret = it->second->merge(CPCommitable(), p);
				}
			} else {
				ret = it->second->merge(CPCommitable(), p);
			}
			if (!ret) {
				break;
			}
		} else {
			if (list != 0) {
				map<IdentifierType, PCommitable>::const_iterator itl = list->ids.find(it->first);
				if (itl != list->ids.end()) {
					it->second = itl->second;
				}
			}
		}
	}
	if (ret && list != 0) {
		if (old != 0) {
			CPCommitableList oldlist = CONST_COMMITABLE_CAST(CommitableList, old);
			if (list != oldlist) {
				std::set<IdentifierType> newitems, removed, myremoved, ls, os, ms;

				transform(list->ids.begin(), list->ids.end(), inserter(ls, ls.end()), ExtractPairFirst<IdentifierType, PCommitable>());
				transform(oldlist->ids.begin(), oldlist->ids.end(), inserter(os, os.end()), ExtractPairFirst<IdentifierType, PCommitable>());
				transform(ids.begin(), ids.end(), inserter(ms, ms.end()), ExtractPairFirst<IdentifierType, PCommitable>());

				set_difference(ls.begin(), ls.end(), os.begin(), os.end(), inserter(newitems, newitems.begin()));
				set_difference(os.begin(), os.end(), ls.begin(), ls.end(), inserter(removed, removed.begin()));
				set_difference(os.begin(), os.end(), ms.begin(), ms.end(), inserter(myremoved, myremoved.begin()));

				for (std::set<IdentifierType>::iterator it = newitems.begin(); it != newitems.end(); ++it) {
					if (myremoved.find(*it) == myremoved.end()) {
						map<IdentifierType, PCommitable>::const_iterator item = list->ids.find(*it);
						ret = item->second->checkBeforeInsertToMergedList(p);
						if (!ret) {
							break;
						}
						ids.insert(*item);
					}
				}
				if (ret) {
					for (std::set<IdentifierType>::iterator it = removed.begin(); it != removed.end(); ++it) {
						if (myremoved.find(*it) == myremoved.end()) {
							ids.erase(*it);
						}
					}
				}
			}
		}
	}
	if (ret && !ignoreNames) {
		names.clear();
		for (map<IdentifierType, PCommitable>::iterator it = ids.begin(); it != ids.end(); ++it) {
			names.insert(pair<string, IdentifierType>(it->second->getName(), it->first));
		}
	}
	if (ret) {
		commitintern();
	}
	return ret;
}

void CommitableList::rename(IdentifierType id, const string &newname)
{
	checkCheckedOut();
	if (ignoreNames) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "cannot search by name in this list");
	}

	PCommitable o = ids[id];
	if (!o->isCheckedOut()) {
		o->checkCheckedOut();
	}
	names.erase(o->getName());
	o->setName(newname);
	names.insert(pair<string, IdentifierType>(newname, id));
}

CommitableList::Iterator CommitableList::begin()
{
	return Iterator(ids.begin());
}

CommitableList::Iterator CommitableList::end()
{
	return Iterator(ids.end());
}

CommitableList::ConstIterator CommitableList::const_begin() const
{
	return ConstIterator(ids.begin());
}

CommitableList::ConstIterator CommitableList::const_end() const
{
	return ConstIterator(ids.end());
}

}
