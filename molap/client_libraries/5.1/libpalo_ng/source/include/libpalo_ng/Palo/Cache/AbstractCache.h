/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#ifndef ABSTRACTCACHE_H
#define ABSTRACTCACHE_H

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4251)
#endif

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "../../config_ng.h"

namespace jedox {
namespace palo {

class Dimension;

class LIBPALO_NG_CLASS_EXPORT CacheItemBase {
public:
	CacheItemBase(unsigned int sequenceNumber) : m_SequenceNumber(sequenceNumber) {}
	virtual ~CacheItemBase() {}
	unsigned int getSequenceNumber() const {return m_SequenceNumber;}
	void setSequenceNumber(unsigned int sequenceNumber) {m_SequenceNumber = sequenceNumber;}
	virtual void forceNextUpdate() {};

private:
	unsigned int m_SequenceNumber;
};

template<class C>
class LIBPALO_NG_CLASS_EXPORT CacheListBase {
public:
	class CacheIterator : public boost::iterator_facade<CacheIterator, C const, boost::forward_traversal_tag> {
	public:
		CacheIterator(const typename std::vector<C>::const_iterator &iter, const typename std::vector<C>::const_iterator &iter_end, boost::shared_ptr<const CacheItemBase> l) : it(iter), end_it(iter_end), m_List(l) {}
		CacheIterator(const CacheIterator &other) : it(other.it), end_it(other.end_it), m_List(other.m_List) {}
		bool operator==(const CacheIterator &other) const {return it == other.it;}
		bool operator!=(const CacheIterator &other) const {return it != other.it;}
		bool end() const {return it == end_it;}
	private:
		friend class boost::iterator_core_access;
		void increment() {if (it != end_it) ++it;}
		C const & dereference() const {return *it;}

		typename std::vector<C>::const_iterator it;
		typename std::vector<C>::const_iterator end_it;
		boost::shared_ptr<const CacheItemBase> m_List;
	};

	virtual std::unique_ptr<CacheIterator> getIterator() const = 0;
	virtual boost::shared_ptr<C> operator[](const std::string &name) const = 0;
	virtual boost::shared_ptr<C> operator[](unsigned int id) const = 0;
	boost::shared_ptr<C> get(const std::string &name) const {return (*this)[name];}
	boost::shared_ptr<C> get(unsigned int id) const {return (*this)[id];}
};

} /* palo */
} /* jedox */
#endif							 // ABSTRACTCACHE_H
