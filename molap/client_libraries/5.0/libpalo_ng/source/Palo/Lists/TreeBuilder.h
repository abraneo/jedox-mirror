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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#ifndef __TREEBUILDER_H_INCL__
#define __TREEBUILDER_H_INCL__

#include <list>
#include <set>
#include <string>

#include <boost/shared_ptr.hpp>

#include <libpalo_ng/Palo/types.h>

#include "SubSet.h"

namespace jedox {
namespace palo {

class SubSet;
/**@brief Strategy-Pattern. A Strategy-Algorithm used by
 * Tree-Builder needs the following methods:
 * check(ElementExList::iterator&)
 * finalize(ElementExList&)
 process_children(ElementExList&)
 The strategy builds a hierarchical tree-like structure inside
 a ElementExList and returns it as a reference.*/

template<class Algorithm>
class TreeBuilder {
public:
	/**@param li The list of top-elements we start with. All elements are
	 supposed to be on the same level
	 @param alg : we call the template methods on a concrete object to enable
	 instance specific information */
	TreeBuilder(Algorithm& alg, const ElementExList& li, SubSet& subset);
	// Destructor
	virtual ~TreeBuilder();
	/**@return The list passed to the constructor, expanded to a properly modified tree*/
	ElementExList& build();
	void process(ElementExList::iterator& head);
	/**@brief Activate building the tree structure from bottom to top.*/
	void setReverse(bool b = true);
private:
	bool m_reverse_active;

	Algorithm& m_alg;

	//list that is later returned.
	ElementExList m_elements;
public:
	SubSet& m_subset_ref;

};

} //palo
} //jedox
#endif							 // __TREEBUILDER_H_INCL__
