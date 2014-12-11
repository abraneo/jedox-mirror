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

#ifndef SLIMMAPITERATOR_H_
#define SLIMMAPITERATOR_H_


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::Content::Content(TKey *key, TValue *value, const Compare &cmp)
	: key(key), value(value), cmp(cmp)
{
	if (key) {
		localKey = *key;
		localValue = *value;
	}
}


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::Content::Content(const typename SlimMap<TKey, TValue, Compare>::Content &c)
	: key(c.key), value(c.value), localKey(c.localKey), localValue(c.localValue), cmp(c.cmp)
{
}


template <typename TKey, typename TValue, typename Compare> bool SlimMap<TKey, TValue, Compare>::Content::operator<(const typename SlimMap<TKey, TValue, Compare>::Content &c) const
{
	//cout << "operator< Content " << localKey << ";" << c.localKey << endl;
	return cmp(localKey, c.localKey);
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::Content & SlimMap<TKey, TValue, Compare>::Content::operator=(const typename SlimMap<TKey, TValue, Compare>::Content &c)
{
	//cout << "operator= Content to " << *key << ";" << *value << " from " << c.localKey << ";" << c.localValue << endl;
	*key = c.localKey;
	*value = c.localValue;
	localKey = c.localKey;
	localValue = c.localValue;
	return *this;
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::Content::set(TKey *key, TValue *value)
{
	this->key = key;
	this->value = value;
	this->localKey = *key;
	this->localValue = *value;
}


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::base_iterator::base_iterator(const SlimMap<TKey, TValue, Compare> &parentMap, uint32_t page, uint32_t offset) :
	parentMap(parentMap), page(page), offset(offset), content(0, 0, parentMap.key_compare)
{
}


template <typename TKey, typename TValue, typename Compare> SlimMap<TKey, TValue, Compare>::base_iterator::base_iterator(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter) :
	parentMap(iter.parentMap), page(iter.page), offset(iter.offset), content(0, 0, iter.parentMap.key_compare)
{
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::base_iterator & SlimMap<TKey, TValue, Compare>::base_iterator::operator=(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter)
{
	//cout << "operator= to " << page << ";" << offset << " from " << iter.page << ";" << iter.offset << endl;
	page = iter.page;
	offset = iter.offset;
	//content = iter.content;
	return *this;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator & SlimMap<TKey, TValue, Compare>::const_iterator::operator++()
{
	const_iterator::increment();
	return *this;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator & SlimMap<TKey, TValue, Compare>::const_iterator::operator--()
{
	const_iterator::decrement();
	return *this;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::const_iterator::operator++(int i)
{
	typename SlimMap<TKey, TValue, Compare>::const_iterator ret = *this;
	this->increment();
	return ret;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::const_iterator::operator--(int i)
{
	typename SlimMap<TKey, TValue, Compare>::const_iterator ret = *this;
	this->decrement();
	return ret;
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::base_iterator::increment(ptrdiff_t n)
{
	uint32_t pc = parentMap.pageCount();
//	if (page >= pc) {
//		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid page in a SlimMapIterator (increment)");
//	}

	offset += (uint32_t)n; // TODOMD, 32bit only
	while (offset >= parentMap.pageSize(page)) {
		if (parentMap.hasFullPages() && page != pc - 1) {
			uint32_t ps = parentMap.pagesSize();
			uint32_t pageDiff = offset / ps;
			offset -= pageDiff * ps;
			page += pageDiff;
			break;
		} else {
			offset -= parentMap.pageSize(page);
			page++;
			if (page >= pc) {
				if (offset != 0) {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid page in a SlimMapIterator (increment)");
				} else {
					break;
				}
			}
		}
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::base_iterator::increment()
{
	offset++;
	if (offset >= parentMap.pageSize(page)) {
		offset = 0;
		page++;
	}
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::base_iterator::decrement()
{
	if (offset > 0) {
		offset--;
	} else {
		page--;
		offset = parentMap.pageSize(page) - 1;
	}
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::base_iterator & SlimMap<TKey, TValue, Compare>::base_iterator::operator--()
{
//	cout << "operator--" << endl;
	decrement(1);
	return *this;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::base_iterator SlimMap<TKey, TValue, Compare>::base_iterator::operator--(int i)
{
	//cout << "operator--i" << endl;
	typename SlimMap<TKey, TValue, Compare>::base_iterator ret = *this;
	decrement(1);
	return ret;
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::base_iterator::decrement(ptrdiff_t n)
{
	if (page == 0 && offset == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid page in a SlimMap::base_iterator (decrement)");
	}

	while (n > 0) {
		if (n <= (ptrdiff_t)offset) {
			offset -= (uint32_t)n;
			break;
		} else {
			if (page == 0) {
				throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid page in a SlimMap::base_iterator (increment)");
			}
			n -= (offset + 1);
			page--;
			offset = parentMap.pageSize(page) - 1;
		}
	}
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::const_iterator::operator-(ptrdiff_t n)
{
	typename SlimMap<TKey, TValue, Compare>::const_iterator ret = *this;
	return ret += -n;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator SlimMap<TKey, TValue, Compare>::const_iterator::operator+(ptrdiff_t n)
{
//	cout << "operator+ " << n << endl;
	typename SlimMap<TKey, TValue, Compare>::const_iterator ret = *this;
	return ret += n;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::const_iterator & SlimMap<TKey, TValue, Compare>::const_iterator::operator+=(ptrdiff_t n)
{
	if (n > 0) {
		base_iterator::increment(n);
	} else if (n < 0) {
		base_iterator::decrement(-n);
	}
	return *this;
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::base_iterator & SlimMap<TKey, TValue, Compare>::base_iterator::operator-=(ptrdiff_t n)
{
//	cout << "operator-=" << n << "calling +=" << endl;
	return *this += -n;
}


template <typename TKey, typename TValue, typename Compare> ptrdiff_t SlimMap<TKey, TValue, Compare>::base_iterator::operator-(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter) const
{
	if (page == iter.page) {
		return offset - iter.offset;
	}

	ptrdiff_t diff = 0;
	if (page > iter.page) {
		diff += parentMap.pageSize(iter.page) - iter.offset;
		if (parentMap.hasFullPages() && page - iter.page > 1) {
			diff += parentMap.pagesSize() * (page - iter.page - 2);
			diff += parentMap.pageSize(page - 1); // maybe it's last page which is not full
		} else {
			for (uint32_t i = iter.page + 1; i < page; i++) {
				diff += parentMap.pageSize(i);
			}
		}
		diff += offset;
	} else {
		diff -= parentMap.pageSize(page) - offset;
		if (parentMap.hasFullPages() && iter.page - page > 1) {
			diff -= parentMap.pagesSize() * (iter.page - page - 2);
			diff += parentMap.pageSize(iter.page - 1); // maybe it's last page which is not full
		} else {
			for (uint32_t i = page + 1; i < iter.page; i++) {
				diff -= parentMap.pageSize(i);
			}
		}
		diff -= iter.offset;
	}
	return diff;
}


template <typename TKey, typename TValue, typename Compare> bool SlimMap<TKey, TValue, Compare>::base_iterator::operator==(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter) const
{
//	cout << "operator==" << endl;
	return page == iter.page && offset == iter.offset;
}


template <typename TKey, typename TValue, typename Compare> bool SlimMap<TKey, TValue, Compare>::base_iterator::operator!=(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter) const
{
//	cout << "operator!= page;offset " << page << ";" << offset << " " << iter.page << ";" << iter.offset << endl;
	return page != iter.page || offset != iter.offset;
}


template <typename TKey, typename TValue, typename Compare> bool SlimMap<TKey, TValue, Compare>::base_iterator::operator<(const typename SlimMap<TKey, TValue, Compare>::base_iterator &iter) const
{
	//cout << "operator< " <<  page << ";" << offset << " " << iter.page << ";" << iter.offset << endl;
	if (page == iter.page) {
		return offset < iter.offset;
	} else {
		return page < iter.page;
	}
}


template <typename TKey, typename TValue, typename Compare> typename SlimMap<TKey, TValue, Compare>::Content & SlimMap<TKey, TValue, Compare>::base_iterator::operator*() const
{
	//cout << "operator* on " << page << ";" << offset << endl;
	typename SlimMap<TKey, TValue, Compare>::Content &c = const_cast<typename SlimMap<TKey, TValue, Compare>::Content &>(content);
	setCurrentContentTo(c);
	return c;
}


template <typename TKey, typename TValue, typename Compare> void SlimMap<TKey, TValue, Compare>::base_iterator::setCurrentContentTo(typename SlimMap<TKey, TValue, Compare>::Content &c) const
{
	c.set(&parentMap.keys->at(page, offset), &parentMap.values->at(page, offset));
}


#endif /* SLIMMAPITERATOR_H_ */
