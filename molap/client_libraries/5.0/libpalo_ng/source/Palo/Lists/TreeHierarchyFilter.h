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
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * 
 *
 */

#ifndef __TREEHIERARCHY_FILTER_H_INCL__
#define __TREEHIERARCHY_FILTER_H_INCL__

#include <list>
#include <set>
#include "StructuralFilter.h"
#include "SubSet.h"

namespace jedox {
namespace palo {

class TreeHierarchyFilter {
public:

	class CopyToMultiSet;

	bool check(const ELEMENT_INFO_EXT& elem);

	void finalize(ElementExList& elemlist);
	/*@brief: do nothing */
	void processChildren(ElementExList& elems);

	TreeHierarchyFilter(ElementExList& given_elements, const StructuralFilter& filter, const StructuralFilter::InRange& check);
private:
	/*@brief Set of unique element names which are found inside the tree
	 we are building */
	ElementExList& m_given_elements;
	const StructuralFilter& m_filter;
	const StructuralFilter::InRange& m_check;
	SubSet::id_set_type m_found_elems;
	SubSet::id_set_type m_given_names;
	//std::multiset<std::string> m_found_elems;
};

} //palo
} //jedox
#endif
