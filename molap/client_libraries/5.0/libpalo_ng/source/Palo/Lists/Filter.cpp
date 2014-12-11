/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 *
 */

#include<map>
#include "Filter.h"
#include "SubSet.h"
#include "ListBasicException.h"

using std::map;
using std::string;

namespace jedox {
namespace palo {

bool Filter::initialized(false);

//create a new filter containing a reference to the subset it refers to.
//Set all flags for this filter
//: myset(SubSet),filter_flags(flags),max_flag(max),initialized(false){
Filter::Filter(SubSet& subset, unsigned long flags, int max, unsigned int type) :
	filter_flags(flags), m_subset_ref(subset), type(type), max_flag(max)
{
	if (!checkFlags(flags)) {
		throw ListBasicException("Invalid Filter Flags passed to this filter.", "Filter Flags invalid");
	}
	if (!initialized) {
		initialized = true;

	}
}

bool Filter::queryFlag(const unsigned long int & flag) const
{
	return (filter_flags & flag) != 0;
}

bool Filter::checkFlags(const unsigned long int& flags)
{
	if ((flags >> max_flag) != 0) {
		return false;
	}
	return true;
}

void Filter::clearFlags()
{
	this->filter_flags = 0;
}

void Filter::setFlag(unsigned long int f)
{
	this->filter_flags |= f;
}

void Filter::resetFlag(unsigned long int f)
{
	this->filter_flags &= ~f;
}

unsigned int Filter::getType()
{
	return this->type;
}

SubSet& Filter::getSubsetRef()
{
	return m_subset_ref;
}

} //palo
} //jedox
