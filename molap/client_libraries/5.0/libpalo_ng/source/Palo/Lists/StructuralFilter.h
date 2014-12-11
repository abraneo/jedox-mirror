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

#ifndef __STRUCTURALFILTER_H_INCL__
#define __STRUCTURALFILTER_H_INCL__

#include <string>
# include <boost/scoped_ptr.hpp>

#include <libpalo_ng/config_ng.h>

#include "Filter.h"

using namespace std;

namespace jedox {
namespace palo {

// STRUCTURAL_FILTER_NUMFLAGS is defined int types.h
// StructuralFilterBase is defined in types.h

class LIBPALO_NG_CLASS_EXPORT StructuralFilter : public StructuralFilterBase, public Filter {
public:
	class InRange;

private:

	class InLevelRange;
	class InDepthRange;
	class InIndentRange;
	class CheckIsNotParentLevel;
	class CheckIsNotChildLevel;
	class CheckIsNotLevel;

	// Constructor
	StructuralFilter(SubSet&, unsigned long int flag);

	// Destructor
	virtual ~StructuralFilter();

	virtual void apply();
	unsigned int m_level_begin, m_level_end;
	//unsigned int m_revolve_begin, m_revolve_end;
	long m_elem_bound;
	long m_elem_revolve;

	unsigned int m_revolve_count;
	unsigned int m_indent;

	void CheckNotInRange(const ElementExList & elem_list);
	void CheckNotInLevelRange(const ElementExList & elem_list);
	void CheckNotInDepthRange(const ElementExList & elem_list);
	void CheckNotInIndentRange(const ElementExList & elem_list);

public:
	friend class TreeHierarchyFilter;
	friend class SubSet;

	/*@brief define the name of the element that bounds the selection from above or below.
	 used in conjunction with flags BELOW INCLUSIVE and ABOVE INCLUSIVE */
	void setBound(const std::string& element_name);

	/*@brief Precondition: start <= end. set interval of aggregation levels
	 the elements are picked from...*/
	void setLevels(const std::string& start, const std::string& end);
	/*@brief Return the start level bound */
	//			const unsigned int getLevelStart() const;
	/*@brief Return the end level bound */
	//			const unsigned int getLevelEnd() const;

	/*@brief Precondition: start <= end. set interval of aggregation levels
	 the elements are picked from... */
	void setLevels(const unsigned int& start, const unsigned int& end);

	/*@brief set the name of the element to start the revolving list with and define the
	 maximum number of elements inside the list. */
	void setRevolveParams(const std::string& elemname, const unsigned int& count);
	unsigned int getRevolveCount();
	/**@brief Set the indent used for the aggregation levels */
	void setIndent(const unsigned int indent);
	//const unsigned int getIndent()  const;
};

class StructuralFilter::InRange {
public:

	InRange(unsigned int lower_bound, unsigned int upper_bound) :
		m_upper(upper_bound), m_lower(lower_bound)
	{
		;
	}
	virtual const bool do_check(const ELEMENT_INFO_EXT& e) const
	{
		return true;
	}
	;
protected:
	unsigned int m_upper;
	unsigned int m_lower;
};

class StructuralFilter::InLevelRange : public StructuralFilter::InRange {
public:

	InLevelRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
		;
	}
	const bool do_check(const ELEMENT_INFO_EXT& e) const
	{
		if ((e.get_level() < m_lower) || (e.get_level() > m_upper)) {
			return false;
		}
		return true;
	}
};

class StructuralFilter::InIndentRange : public StructuralFilter::InRange {
public:

	InIndentRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
		;
	}
	const bool do_check(const ELEMENT_INFO_EXT& e) const
	{
		if ((e.m_einf.indent < m_lower) || (e.m_einf.indent > m_upper)) {
			return false;
		}
		return true;
	}
};

class StructuralFilter::InDepthRange : public StructuralFilter::InRange {
public:

	InDepthRange(unsigned int lower_bound, unsigned int upper_bound) :
		InRange(lower_bound, upper_bound)
	{
		;
	}
	const bool do_check(const ELEMENT_INFO_EXT& e) const
	{
		if ((e.m_einf.depth < m_lower) || (e.m_einf.depth > m_upper)) {
			return false;
		}
		return true;
	}
};

} //palo
} //jedox
#endif
