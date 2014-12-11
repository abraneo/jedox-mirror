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

#ifndef DIMENSIONCACHE_H
#define DIMENSIONCACHE_H

#include "libpalo_ng/Palo/types.h"
#include "libpalo_ng/Palo/Cache/AbstractCache.h"
#include "libpalo_ng/Palo/Cache/ElementCache.h"
#include "libpalo_ng/Palo/Exception/ElementNotFoundException.h"

namespace jedox {
namespace palo {

class ServerImpl;
class PaloClient;

class LIBPALO_NG_CLASS_EXPORT DimensionCacheB : public DIMENSION_INFO, public CacheItemBase {
public:
	DimensionCacheB() : CacheItemBase(0) {}
};

class LIBPALO_NG_CLASS_EXPORT DimensionCache : public DimensionCacheB, public CacheListBase<ElementCache> {
public:
	DimensionCache(const DimensionCacheB &dim, boost::shared_ptr<ServerImpl> s, boost::shared_ptr<PaloClient> paloClient, unsigned int db) : DimensionCacheB(dim), m_ServerImpl(s), m_PaloClient(paloClient), m_db(db) {}
	virtual ~DimensionCache() {}
	virtual std::unique_ptr<CacheIterator> getIterator() const;
	virtual boost::shared_ptr<ElementCache> operator[](const std::string &name) const;
	virtual boost::shared_ptr<ElementCache> operator[](unsigned int id) const;

private:
	boost::shared_ptr<ServerImpl> m_ServerImpl;
	boost::shared_ptr<PaloClient> m_PaloClient;
	unsigned int m_db;
};

} /* palo */
} /* jedox */

#endif							 // DIMENSIONCACHE_H
