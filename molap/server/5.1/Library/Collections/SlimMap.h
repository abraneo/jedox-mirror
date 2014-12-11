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

#ifndef SLIMMAP_H_
#define SLIMMAP_H_


#include "palo.h"
#include "Olap/Commitable.h"
#include <iostream>

#include "SlimVector.h"

namespace palo {


template < typename TKey, typename TValue, typename Compare = less<TKey> > class SlimMap : public Commitable {

protected:
	struct Content {
		Content(TKey *key, TValue *value, const Compare &cmp);
		Content(const Content &c);
		TKey *key;
		TValue *value;
		TKey localKey;
		TValue localValue;
		const Compare &cmp;

		bool operator<(const Content &c) const;
		Content & operator=(const Content &c);
		void set(TKey *key, TValue *value);
	};


protected:
	class base_iterator : public std::iterator<random_access_iterator_tag, Content, ptrdiff_t, Content *, Content &> {
	public:
		base_iterator(const base_iterator &iter);
		virtual ~base_iterator() {}

		base_iterator & operator=(const base_iterator &iter);
		base_iterator & operator--();
		base_iterator operator--(int i);
		base_iterator & operator-=(ptrdiff_t n);
		ptrdiff_t operator-(const base_iterator &iter) const;
		bool operator==(const base_iterator &iter) const;
		bool operator!=(const base_iterator &iter) const;
		bool operator<(const base_iterator &iter) const;
		Content & operator*() const;
		TKey & first() {
			setCurrentContentTo(content);
			return *content.key;
		}
		TValue & second() {
			setCurrentContentTo(content);
			return *content.value;
		}
		void print() {
			setCurrentContentTo(content);
			cout << *content.key << " " << *content.value << endl;
		}
	protected:
		base_iterator(const SlimMap<TKey, TValue, Compare> &parentMap, uint32_t page, uint32_t offset);

		void increment(ptrdiff_t n);
		void increment();
		void decrement(ptrdiff_t n);
		void decrement();
		void setCurrentContentTo(Content &c) const;

		const SlimMap<TKey, TValue, Compare> &parentMap;
		uint32_t page;
		uint32_t offset;
		Content content;
	};

public:
	class iterator : public base_iterator {
		friend class SlimMap<TKey, TValue, Compare>;
		friend class NameIdSlimMap;
	public:
		iterator(const iterator &iter) : base_iterator(iter) {}

	protected:
		iterator(const SlimMap<TKey, TValue, Compare> &parentMap, uint32_t page, uint32_t offset) : base_iterator(parentMap, page, offset) {}
	};

	class const_iterator : public base_iterator {
		friend class SlimMap<TKey, TValue, Compare>;
		friend class NameIdSlimMap;
	public:
		const_iterator(const const_iterator &iter) : base_iterator(iter) {}
		const_iterator(const base_iterator &iter) : base_iterator(iter) {}
		const_iterator operator+(ptrdiff_t n);
		const_iterator operator-(ptrdiff_t n);
		const_iterator & operator++();
		const_iterator & operator--();
		const_iterator & operator+=(ptrdiff_t n);
		const_iterator operator++(int i);
		const_iterator operator--(int i);
		ptrdiff_t operator-(const const_iterator &iter) const {
			return base_iterator::operator-(iter);
		}

	protected:
		const_iterator(const SlimMap<TKey, TValue, Compare> &parentMap, uint32_t page, uint32_t offset) : base_iterator(parentMap, page, offset) {}
	};


public:
	SlimMap(uint32_t blockSize);
	virtual ~SlimMap();

	virtual TValue & operator[](const TKey &key);
	virtual const TValue & operator[](const TKey &key) const;
	virtual void clear();
	virtual TValue & insert(const TKey &key, TValue value, bool sort);
	virtual void sort(bool afterLoad);
	virtual iterator begin();
	virtual iterator end();
	virtual const_iterator begin() const;
	virtual const_iterator end() const;
	virtual void erase(const TKey &key);
	virtual void erase(const_iterator &it);
	virtual const_iterator find(const TKey &key);
	virtual uint64_t size() const;
	virtual void print() const;

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

protected:
	uint32_t blockSize;
	uint64_t mapSize;
	bool sorted;
	bool fullPages;
	uint32_t fullPageSize;
	TKey lastKey;
	Compare key_compare;

	typename Slim<TKey>::PSlimVector keys;
	typename Slim<TValue>::PSlimVector values;

	SlimMap(const SlimMap<TKey, TValue, Compare> &map);

	const_iterator find(const_iterator &itBegin, const_iterator &itEnd, const TKey &key, bool returnClosest) const;
	TValue & insert(const TKey &key, TValue value, const_iterator &currSearchResult);
	uint32_t pageCount() const {
		return keys->pageCount();
	}
	uint32_t pageSize(uint32_t page) const {
		return keys->pageSize(page);
	}
	uint32_t pagesSize() const;
	bool hasFullPages() const {
		return fullPages;
	}
};

//template <typename TKey, typename TValue, typename Compare> Compare SlimMap<TKey, TValue, Compare>::Content::key_compare;

#include "SlimMapIterator.h"


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::SlimMap(uint32_t blockSize) :
	Commitable(""), blockSize(blockSize), mapSize(0), sorted(true), fullPages(true), fullPageSize(0)
{
	uint32_t maxKeys = blockSize / sizeof(TKey);
	uint32_t maxValues = blockSize / sizeof(TValue);
	uint32_t maxInBlock = min(maxKeys, maxValues);

	keys = typename Slim<TKey>::PSlimVector(new SlimVector<TKey>(maxInBlock * sizeof(TKey)));
	values = typename Slim<TValue>::PSlimVector(new SlimVector<TValue>(maxInBlock * sizeof(TValue)));
}


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::SlimMap(const SlimMap<TKey, TValue, Compare> &map) :
	Commitable(map), blockSize(map.blockSize), mapSize(map.mapSize), sorted(map.sorted), fullPages(map.fullPages), fullPageSize(map.fullPageSize), lastKey(map.lastKey)
{
	keys = boost::dynamic_pointer_cast<SlimVector<TKey>, Commitable>(map.keys->copy());
	values = boost::dynamic_pointer_cast<SlimVector<TValue>, Commitable>(map.values->copy());
}


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::~SlimMap()
{
	checkOut();
	clear();
}


template <typename TKey, typename TValue, typename Compare> TValue & SlimMap<TKey, TValue, Compare>::operator[](const TKey &key)
{
	const SlimMap<TKey, TValue, Compare> &cm = *this;
	const_iterator itBegin = cm.begin();
	const_iterator itEnd = cm.end();
	const_iterator it = find(itBegin, itEnd, key, true);

	if (it != end() && it.first() == key) {
		return it.second();
	} else {
		return insert(key, TValue(), it);
	}
}


template <typename TKey, typename TValue, typename Compare> const TValue & SlimMap<TKey, TValue, Compare>::operator[](const TKey &key) const
{
	const_iterator itBegin = begin();
	const_iterator itEnd = end();
	const_iterator it = find(itBegin, itEnd, key, true);

	if (it != end() && it.first() == key) {
		return it.second();
	} else {
		//return insert(key, TValue(), it);
		throw ErrorException(ErrorException::ERROR_INTERNAL, "key not found in operator[] const");
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::erase(const TKey &key)
{
	const_iterator it = find(key);
	if (it != end()) {
		erase(it);
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::erase(const_iterator &it)
{
	checkCheckedOut();
	keys->erase(it.page, it.offset);
	values->erase(it.page, it.offset);
	mapSize--;
	fullPages = false;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::find(const TKey &key)
{
	const SlimMap<TKey, TValue, Compare> &cm = *this;
	const_iterator itBegin = cm.begin();
	const_iterator itEnd = cm.end();
	return find(itBegin, itEnd, key, false);
}


template <typename TKey, typename TValue, typename Compare> uint64_t SlimMap<TKey, TValue, Compare>::size() const
{
	return mapSize;
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::print() const
{
	for (const_iterator it = begin(); it != end(); ++it) {
		cout << "PAGE " << it.page << ":" << it.first() << " " << it.second() << endl;
	}
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::find(const_iterator &itBegin, const_iterator &itEnd, const TKey &key, bool returnClosest) const
{
	if (itBegin == itEnd) {
		if (itEnd.page != values->pageCount()) {
			if (itEnd.first() == key) {
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
		if (itBegin.page == itEnd.page) {
			ptrdiff_t diff = (itEnd - itBegin) / 2;
			const_iterator itMiddle = itBegin + diff;

			if (key_compare(itMiddle.first(), key)) {
				return find(++itMiddle, itEnd, key, returnClosest);
			} else {
				return find(itBegin, itMiddle, key, returnClosest);
			}
		} else {
			uint32_t pageDiff = (itEnd.page - itBegin.page) / 2;
			const_iterator itMiddle = const_iterator(itBegin.parentMap, itBegin.page + pageDiff, itBegin.parentMap.pageSize(itBegin.page + pageDiff) - 1);

			if (key_compare(itMiddle.first(), key)) {
				return find(++itMiddle, itEnd, key, returnClosest);
			} else {
				return find(itBegin, itMiddle, key, returnClosest);
			}
		}
	}
}


template <typename TKey, typename TValue, typename Compare> TValue & SlimMap<TKey, TValue, Compare>::insert(const TKey &key, TValue value, typename SlimMap<TKey, TValue, Compare>::const_iterator &currSearchResult)
{
	if (currSearchResult.page == keys->pageCount()) {
		// empty map or all keys are smaller, push to end
		return insert(key, value, false);
	} else {
		if (key_compare(currSearchResult.first(), key)) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "inserting after currSearchResult was not supposed to happen");
		} else {
			if (key == currSearchResult.first()) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "key already present in map");
			}
			//cout << key << " to be inserted before " << currSearchResult.first() << endl;
			keys->insertBefore(currSearchResult.page, currSearchResult.offset, key);
			TValue & insertedValue = values->insertBefore(currSearchResult.page, currSearchResult.offset, value);
			mapSize++;
			fullPages = false;
			return insertedValue;
		}
	}
}


template <typename TKey, typename TValue, typename Compare> uint32_t SlimMap<TKey, TValue, Compare>::pagesSize() const
{
	if (keys->pageCount() > 1) {
		return keys->pageSize(0);
	} else {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "SlimMap::pagesSize invalid request");
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::clear()
{
	checkCheckedOut();
	keys->clear();
	values->clear();
	mapSize = 0;
	fullPages = true;
}


template <typename TKey, typename TValue, typename Compare> TValue & SlimMap<TKey, TValue, Compare>::insert(const TKey &key, TValue value, bool sort)
{
	checkCheckedOut();
	if (!sort) {
		if (mapSize > 0 && sorted) {
			if (key_compare(lastKey, key)) {
				lastKey = key;
			} else {
				sorted = false;
			}
		} else if (mapSize == 0) {
			lastKey = key;
			sorted = true;
		}

		mapSize++;
		keys->push_back(key);
		//cout << "inserted key " << key << " with value " << value << " into map, pages used: " << keys.pageCount() << endl;
		return values->push_back(value);
	} else {
		return (*this)[key] = value;
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::sort(bool afterLoad)
{
//	cout << "SORTING MAP, size = " << mapSize << endl;
//	print();

	checkCheckedOut();

	if (!sorted) {
		const SlimMap<TKey, TValue, Compare> &cm = *this;
		const_iterator itBegin = cm.begin();
		const_iterator itEnd = cm.end();
		std::sort(itBegin, itEnd);

		sorted = true;
		if (mapSize > 0) {
			lastKey = (--itEnd).first();
		}
	}

//	cout << "SUCCESSFULLY SORTED, size = " << mapSize << endl;
//	print();
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::iterator SlimMap<TKey, TValue, Compare>::begin()
{
	iterator b(*this, 0, 0);
	return b;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::iterator SlimMap<TKey, TValue, Compare>::end()
{
	iterator e(*this, keys->pageCount(), 0);
	return e;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::begin() const
{
	const_iterator b(*this, 0, 0);
	return b;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::end() const
{
	const_iterator e(*this, keys->pageCount(), 0);
	return e;
}


template <typename TKey, typename TValue, typename Compare> bool SlimMap<TKey, TValue, Compare>::merge(const CPCommitable &o, const PCommitable &p)
{
	bool ret = true;
	checkCheckedOut();
	mergeint(o,p);

	typename Slim2<TKey, TValue, Compare>::CPSlimMap oldMap = boost::dynamic_pointer_cast<const SlimMap<TKey, TValue, Compare>, const Commitable>(old);
	typename Slim2<TKey, TValue, Compare>::CPSlimMap map = boost::dynamic_pointer_cast<const SlimMap<TKey, TValue, Compare>, const Commitable>(o);

	if (ret) {
		// return false if any conflict in keys or values
		if (map && oldMap) {
			if (keys != map->keys && map->keys != oldMap->keys) {
				ret = false;
			}
			if (values != map->values && map->values != oldMap->values) {
				ret = false;
			}
		}
	}

	if (ret && keys->isCheckedOut()) {
		ret = keys->merge(map ? map->keys : typename Slim<TKey>::PSlimVector(), shared_from_this());
	}

	if (ret && values->isCheckedOut()) {
		ret = values->merge(map ? map->values : typename Slim<TValue>::PSlimVector(), shared_from_this());
	}

	if (ret) {
		commitintern();
	}

	return ret;
}


template<typename TKey, typename TValue, typename Compare> PCommitable SlimMap<TKey, TValue, Compare>::copy() const
{
	checkNotCheckedOut();
	//typename Slim2<TKey, TValue, Compare>::PSlimMap newMap(new SlimMap<TKey, TValue, Compare>(*this));
	boost::shared_ptr<SlimMap<TKey, TValue, Compare> > newMap(new SlimMap<TKey, TValue, Compare>(*this));
	return newMap;
}


}

#endif /* SLIMMAP_H_ */
