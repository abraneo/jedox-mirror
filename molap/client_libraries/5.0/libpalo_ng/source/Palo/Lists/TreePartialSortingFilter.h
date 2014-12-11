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

#ifndef __TREEPARTIALSORTINGFILTER_H_INCL__
#define __TREEPARTIALSORTINGFILTER_H_INCL__

#include <string>
#include <set>
#include <list>
#include "SubSet.h"
#include "SortingFilter.h"

namespace jedox {
namespace palo {

class SortingFilter;

class NameComparison;
class PositionalComparison;

/**@brief Used as an algorithm by TreeBuilder
 * SD == Show Duplicates */

class SDTreePartialSortingFilter {
public:
	class CopyToSet;
	/*@brief The check fails if the element has already been filtered (== is not in
	 m_given_names) and, if yes, mark it for deletion */
	inline bool check(const ELEMENT_INFO_EXT& elem)
	{
		return m_given_names.find(elem.get_id()) != m_given_names.end();
	}
	/*@brief: do nothing */
	void finalize(ElementExList& elemlist);

	/*@brief: sort according to names, evtl. reverse if reverse is set */
	void processChildren(ElementExList& elems);

	SDTreePartialSortingFilter(ElementExList& given_elements, const SortingFilter&, AbstractComparison& comp);

private:
	/*@brief Set of names of elements that have passed previous filters
	 A set lets us validate the elements faster than a list
	 (logN to N) */
	std::set<long> m_given_names;
	ElementExList& m_given_elements;
	std::list<ElementExList::iterator> m_deletions;
	const SortingFilter& m_filter;
	AbstractComparison& m_comp;
};

/** @brief  Used as an algorithm by TreeBuilder
 *
 * same as SDTreePartialSortingFilter, but instead of showing duplicates,
 * we gonna hide them */

class TreePartialSortingFilter {
public:
	class CopyToSet;
	/*@brief The check fails if the element has already been filtered (== is not in
	 m_given_names) and, if yes, mark it for deletion */
	inline bool check(const ELEMENT_INFO_EXT& elem)
	{
		return m_given_names.find(elem.get_id()) != m_given_names.end();
	}

	/*@brief: remove duplicates */
	void finalize(ElementExList& elemlist);

	/*@brief: sort according to names, evtl. reverse if reverse is set */
	void processChildren(ElementExList& elems);

	TreePartialSortingFilter(ElementExList& given_elements, const SortingFilter&, AbstractComparison& comp);

private:
	std::set<long> m_given_names;
	ElementExList& m_given_elements;
	std::list<ElementExList::iterator> m_deletions;
	const SortingFilter& m_filter;
	AbstractComparison& m_comp;
};

} //palo
} //jedox
#endif							 // __TREEPARTIALSORTINGFILTER_H_INCL__
