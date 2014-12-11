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

#ifndef __ALIASFILTER_H_INCL__
#define __ALIASFILTER_H_INCL__

#include <string>
#include <list>
#include <vector>

#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <libpalo_ng/config_ng.h>

#include "Filter.h"
#include "SubSet.h"

namespace jedox {
namespace palo {

// ALIAS_FILTER_NUMFLAGS is defined in types.h
// AliasFilterBase is defined in types.h

class LIBPALO_NG_CLASS_EXPORT AliasFilter : public AliasFilterBase, public Filter {

	struct FilterOperator;
	struct AliasFilterHelper;

	enum Optype {
		NO_OP = 0, OP_GT, OP_LESS, OP_GEQ, OP_LEQ, OP_EQU, OP_NE
	};

	typedef boost::tuple<Optype, double, boost::regex> filterexpression_type;
	typedef std::vector<filterexpression_type> filterexpression_vec_type;
	typedef std::vector<filterexpression_vec_type> filterexpression_vec_vec_type;
	typedef long coord_type;
	typedef std::vector<coord_type> coords_type;
	typedef std::vector<CELL_VALUE> cell_value_vec_type;

	// Constructor
	AliasFilter(SubSet&, unsigned long int flag);

	// Destructor
	virtual ~AliasFilter();

public:
	friend class SubSet;

	// pass filter columns. Each filter column is one column from the attribute-cube
	void setFilters(const std::vector<std::vector<std::string> >& columns);

	/* set the first and the second attribute that is to
	 be searched for aliases
	 param first : first attribute to be searched, only this one is needed if flag SEARCH_ONE is used
	 param second : second attribute to be searched. Only necessary if flag SEARCH_TWO has been set.*/
	void setAttributes(const std::string& first, const std::string& second = "");

	/* Set the attribute in which we search for the alias that is put into element.display_alias
	 TODO: implement set and apply for this feature */
	inline void setDisplayAttribute(const std::string& attr);

private:
	// apply this filter
	virtual void apply();

	// Find and insert the alias for this element
	inline bool insertAlias(const CELL_VALUE& cell_val, const ElementExList::iterator& it);

	// Find and insert the display-alias for this element
	inline bool insertDisplayAlias(const CELL_VALUE& cell_val, const ElementExList::iterator& it);
	inline void applyOneDisplayAlias(ElementExList& elemlist, cell_value_vec_type& cell_vals);
	inline void applyTwoDisplayAliases(ElementExList& elemlist, cell_value_vec_type& cell_vals);

	// Search one attribute for aliases
	inline void applyOneAlias(ElementExList& elemlist);
	inline void applyOneAlias(ElementExList& elemlist, cell_value_vec_type& cell_vals);

	// Search two attributes for aliases
	inline void applyTwoAliases(ElementExList& elemlist);
	inline void applyTwoAliases(ElementExList& elemlist, cell_value_vec_type& cell_vals);

	struct AliasFilterData;
	boost::scoped_ptr<AliasFilterData> m_data;
};

} //jedox
} //palo
#endif
