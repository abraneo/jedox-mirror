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

#ifndef CUBECACHE_H
#define CUBECACHE_H

namespace jedox {
namespace palo {

class LIBPALO_NG_CLASS_EXPORT CubeCache : public CUBE_INFO, public CacheItemBase {
public:

	CubeCache() : CacheItemBase(0) {}
	virtual ~CubeCache() {}
};

} /* palo */
} /* jedox */

#endif							 // CUBECACHE_H
