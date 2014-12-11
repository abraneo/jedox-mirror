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

#ifndef DIMENSIONSCACHE_H
#define DIMENSIONSCACHE_H

#include "libpalo_ng/Palo/types.h"
#include "libpalo_ng/Palo/Cache/AbstractCache.h"
#include "libpalo_ng/Palo/Cache/DimensionCache.h"
#include "libpalo_ng/Palo/Exception/DimensionNotFoundException.h"

namespace jedox {
namespace palo {

class ServerImpl;

class LIBPALO_NG_CLASS_EXPORT DimensionsCacheB : public CacheListBase<DimensionCacheB> {
	friend class DimensionsCache;
public:
	DimensionsCacheB(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient, unsigned int db) : m_ServerImpl(s), m_PaloClient(paloClient), m_db(db) {}
	virtual ~DimensionsCacheB() {}
	virtual std::unique_ptr<CacheIterator> getIterator() const;
	virtual boost::shared_ptr<DimensionCacheB> operator[](const std::string &name) const;
	virtual boost::shared_ptr<DimensionCacheB> operator[](unsigned int id) const;

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	unsigned int m_db;
};

class LIBPALO_NG_CLASS_EXPORT DimensionsCache {
public:
	friend class ServerImpl;
	class CacheIterator : public boost::iterator_facade<CacheIterator, DimensionCache const, boost::forward_traversal_tag> {
	public:
		CacheIterator(const DimensionsCacheB::CacheIterator &iter, boost::shared_ptr<DimensionsCacheB> base) : iter(iter), base(base), item(iter.end() ? 0 : new DimensionCache(*iter, base->m_ServerImpl, base->m_PaloClient, base->m_db)) {}
		CacheIterator(const CacheIterator &other) : iter(other.iter) {}
		bool operator==(const CacheIterator &other) const {return iter == other.iter;}
		bool operator!=(const CacheIterator &other) const {return iter != other.iter;}
		bool end() const {return iter.end();}
	private:
		friend class boost::iterator_core_access;
		void increment() {++iter; item.reset(iter.end() ? 0 : new DimensionCache(*iter, base->m_ServerImpl, base->m_PaloClient, base->m_db));}
		DimensionCache const & dereference() const {return *item;}

		DimensionsCacheB::CacheIterator iter;
		boost::shared_ptr<DimensionsCacheB> base;
		boost::shared_ptr<DimensionCache> item;
	};
	DimensionsCache(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient, unsigned int db) : m_Base(new DimensionsCacheB(s, paloClient, db)) {}
	std::unique_ptr<CacheIterator> getIterator() const;
	boost::shared_ptr<DimensionCache> operator[](const std::string &name) const;
	boost::shared_ptr<DimensionCache> operator[](unsigned int id) const;

private:
	boost::shared_ptr<DimensionsCacheB> m_Base;
};

} /* palo */
} /* jedox */

#endif							 // DIMENSIONSCACHE_H
