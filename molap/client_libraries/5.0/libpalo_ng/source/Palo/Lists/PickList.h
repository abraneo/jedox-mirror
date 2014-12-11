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

#ifndef __PICKLIST_H_INCL__
#define __PICKLIST_H_INCL__

#include <list>
#include <vector>
#include <string>
#include <set>

#include <boost/scoped_ptr.hpp>

#include <libpalo_ng/config_ng.h>

#include "Filter.h"
#include "SubSet.h"

namespace jedox {
namespace palo {

using namespace std;

class SubSet;

// PICKLIST_FILTER_NUMFLAGS is defined int types.h
// PickListBase is defined in types.h

class LIBPALO_NG_CLASS_EXPORT PickList : public PickListBase, public Filter {

public:
	friend class SubSet;
	class CheckElementsInStrings;

	/*@brief define the list of elements of our pick list.
	 @param elems The names of the hand-picked elements.
	 */
	void setPicklist(const std::list<std::string>& elems);

	/**@brief Get the list of elements we are using as our pick list*/
	ElementExList& getElements();
private:
	/**@brief Apply this Pick list*/
	virtual void apply();
	virtual ~PickList();

	PickList(SubSet&, unsigned long int flags);
	struct PickListImpl;
	boost::scoped_ptr<PickListImpl> m_Impl;

	/**@brief Build a list of real elements from the strings passed to this filter*/
	void buildElemlist(void);
};

} //palo
} //jedox
#endif
