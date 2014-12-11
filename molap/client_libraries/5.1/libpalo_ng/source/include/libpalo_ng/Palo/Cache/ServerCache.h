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

#ifndef SERVERCACHE_H
#define SERVERCACHE_H

#include "libpalo_ng/Palo/types.h"
#include "libpalo_ng/Palo/Cache/AbstractCache.h"
#include "libpalo_ng/Palo/Cache/DatabaseCache.h"
#include "libpalo_ng/Palo/Exception/DatabaseNotFoundException.h"

namespace jedox {
namespace palo {

class LIBPALO_NG_CLASS_EXPORT ServerCacheB : public SERVER_INFO, public CacheItemBase {
public:
	ServerCacheB() : CacheItemBase(0) {}
};

class ServerCacheBase : public ServerCacheB, public CacheListBase<DatabaseCacheB> {
	friend class ServerCache;
public:
	ServerCacheBase(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient) : m_ServerImpl(s), m_PaloClient(paloClient) {}
	virtual ~ServerCacheBase() {}
	virtual std::unique_ptr<CacheIterator> getIterator() const;
	virtual boost::shared_ptr<DatabaseCacheB> operator[](const std::string &name) const;
	virtual boost::shared_ptr<DatabaseCacheB> operator[](unsigned int id) const;

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
};

class LIBPALO_NG_CLASS_EXPORT ServerCache {
public:
	friend class ServerImpl;
	class CacheIterator : public boost::iterator_facade<CacheIterator, DatabaseCache const, boost::forward_traversal_tag> {
	public:
		CacheIterator(const ServerCacheBase::CacheIterator &iter, boost::shared_ptr<ServerCacheBase> base) : iter(iter), base(base), item(iter.end() ? 0 : new DatabaseCache(*iter, boost::shared_ptr<DimensionsCache>(new DimensionsCache(base->m_ServerImpl, base->m_PaloClient, iter->database)),boost::shared_ptr<CubesCache>(new CubesCache(base->m_ServerImpl, base->m_PaloClient, iter->database)))) {}
		CacheIterator(const CacheIterator &other) : iter(other.iter) {}
		bool operator==(const CacheIterator &other) const {return iter == other.iter;}
		bool operator!=(const CacheIterator &other) const {return iter != other.iter;}
		bool end() const {return iter.end();}
	private:
		friend class boost::iterator_core_access;
		void increment() {++iter; item.reset(iter.end() ? 0 : new DatabaseCache(*iter, boost::shared_ptr<DimensionsCache>(new DimensionsCache(base->m_ServerImpl, base->m_PaloClient, iter->database)),boost::shared_ptr<CubesCache>(new CubesCache(base->m_ServerImpl, base->m_PaloClient, iter->database))));}
		DatabaseCache const & dereference() const {return *item;}

		ServerCacheBase::CacheIterator iter;
		boost::shared_ptr<ServerCacheBase> base;
		boost::shared_ptr<DatabaseCache> item;
	};
	ServerCache(boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient) : m_Base(new ServerCacheBase(s, paloClient)) {}
	std::unique_ptr<CacheIterator> getIterator() const;
	boost::shared_ptr<DatabaseCache> operator[](const std::string &name) const;
	boost::shared_ptr<DatabaseCache> operator[](unsigned int id) const;

private:
	boost::shared_ptr<ServerCacheBase> m_Base;
};

} /* palo */
} /* jedox */

#endif							 // SERVERCACHE_H
