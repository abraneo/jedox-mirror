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

#ifndef __FILTER_H_INCL__
#define __FILTER_H_INCL__

#include <vector>
#include <map>
#include <string>
#include <set>

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Palo/types.h>

namespace jedox {
namespace palo {

class SubSet;

// FilterBase is defined in types.h

//Basic filter class defining possible Flags and basic operations
class LIBPALO_NG_CLASS_EXPORT Filter {
public:

	//List of filter types used to distinguish between different
	//filters and for introspection. (Has a SubSet two filters of the same kind -> error)
	enum FilterType {
		PICKLIST = 0, STRUCTURAL, ALIAS, TEXT, DATA,
		//Sorting filter is the last filter applied and does always exist (there has to be some default order)
		SORTING,
		TOTAL_NUMBER
	};

	virtual void apply()
	{
		//m_set.clear();
	}

	//create a new filter containing a reference to the subset.
	//Set all flags for this filter
	Filter(SubSet& subset, unsigned long int flags, int max, unsigned int type);

	/** The subset MUST NOT be deleted inside the destructor,
	 *  since we are adding and removing filters to/from existing
	 *  subsets. */
	virtual ~Filter()
	{
		;
	}
	//query a specific flag for a filter.
	//True if the flag is set, false if not.
	//Exception if the flag does not exist for this filter.
	bool queryFlag(const unsigned long int & flag) const;

	//Check if the flags passed to the constructor are sane
	bool checkFlags(const unsigned long int& flags);

	/**@brief set all flags of this filter to 0*/
	void clearFlags();

	std::map<unsigned int, std::string>* get_filter_map()
	{
		// HACK
		//return &s_filter_names;
		return NULL;
	}

	/**@brief Returns the type of this filter, e.g. DATA*/
	unsigned int getType();
	SubSet& getSubsetRef();
	void setFlag(unsigned long int);
	void resetFlag(unsigned long int);
protected:
	unsigned long int filter_flags;
	SubSet& m_subset_ref;

	//std::set<ELEMENT_INFO_EXT> m_set;
	// HACK
	//static std::map<unsigned int, std::string> s_filter_names;
	static bool initialized;
	unsigned int type;

private:
	int max_flag;
};

} //palo
} //jedox
#endif
