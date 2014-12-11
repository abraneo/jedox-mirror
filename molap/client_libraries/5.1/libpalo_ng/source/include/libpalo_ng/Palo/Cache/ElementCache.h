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

#ifndef ELEMENTCACHE_H
#define ELEMENTCACHE_H

#include "libpalo_ng/Palo/types.h"
#include "libpalo_ng/Palo/Cache/AbstractCache.h"
#include "libpalo_ng/Util/StringUtils.h"

namespace jedox {
namespace palo {

class LIBPALO_NG_CLASS_EXPORT ElementCache : public ELEMENT_INFO {
public:
	ElementCache() {}
	ElementCache(unsigned int id) {
		element = id;
		nelement = jedox::util::StringUtils::Numeric2String(id);
		position = element;
		level = 0;
		indent = 1;
		depth = 0;
		type = ELEMENT_INFO::NUMERIC;
		number_parents = 0;
		//ELEMENT_LIST parents;
		number_children = 0;
		//ELEMENT_LIST children;
		//ELEMENT_WEIGHT_LIST weights;
	}
};

} /* palo */
} /* jedox */

#endif							 // ELEMENTCACHE_H
