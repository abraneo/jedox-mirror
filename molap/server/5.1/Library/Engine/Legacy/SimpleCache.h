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
 * \author Radu Racariu radu@yalos-solutions.com
 * \author Radu Ialovoi ialovoi@yalos-solutions.com
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef SIMPLE_CACHE_H
#define SIMPLE_CACHE_H

#include "palo.h"
#include "Collections/CellMap.h"

using namespace palo;

namespace paloLegacy {

class SimpleCache {
public:
	class cache_value_type : public std::pair<double, uint8_t> {
	public:
		cache_value_type() {}
		cache_value_type(double d, uint8_t u) : std::pair<double, uint8_t>(d,u) {}
		cache_value_type &operator+=(const cache_value_type &acv);
		operator double() {return first;}
	};
	typedef IdentifiersType cache_key_type;
	typedef std::map<cache_key_type, cache_value_type> cache_type;
	typedef std::pair<IdentifierType, IdentifierType> query_cache_key;
	typedef boost::shared_ptr<ICellMap<cache_value_type> > PQueryCache;
	typedef std::map<query_cache_key, PQueryCache > query_cache_type;
	typedef std::map<query_cache_key, PQueryCache >::iterator query_cache_iterator;
};


}

#endif
