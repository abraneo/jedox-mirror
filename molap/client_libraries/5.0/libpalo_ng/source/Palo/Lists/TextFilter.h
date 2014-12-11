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

#ifndef __TEXTFILTER_H_INCL__
#define __TEXTFILTER_H_INCL__

#include <list>

#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>

#include <libpalo_ng/config_ng.h>

#include "Filter.h"

using namespace boost;
using namespace std;

namespace jedox {
namespace palo {

// TEXT_FILTER_NUMFLAGS is defined int types.h
// TextFilterBase is defined in types.h
class LIBPALO_NG_CLASS_EXPORT TextFilter : public TextFilterBase, public Filter {

public:
	enum ColumnType {
		BASIC = 0, // use only the basic dimension for searching the regex (default)
		ALIAS = 1, // use aliases as defined by the alias filter for searching the regex
		// use the attribute columns for searching -- ADDITIONAL_FIELDS must be != 0
		ATTRIBUTE = 2
	};

	friend class SubSet;

	/**@brief add a regular expression to the pool. It is applied to all
	 elements of the (alias|attribute)column. The matches are combined using
	 OR. E.g. if an element matches regex A but not B, it is inside the final subset.
	 @param r extended regular expression
	 @param t defines the column to which the regex is applied
	 @param a if applied to an attribute-column, this string says which one. */
	void setRegex(const std::string& s, ColumnType t = TextFilter::ALIAS, const std::string& attribute = "");

private:
	virtual ~TextFilter();

	/**@brief Initialize a filter that uses regular expressions on element
	 names/aliases/attributes. */
	TextFilter(SubSet& s, unsigned long int flags);

	/**@brief apply this filter*/
	virtual void apply();

	struct TextFilterImpl;
	boost::scoped_ptr<TextFilterImpl> m_Impl;
};
} //palo
} //jedox
#endif
