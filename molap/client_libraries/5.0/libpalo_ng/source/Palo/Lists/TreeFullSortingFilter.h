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

#ifndef __TREEFULLSORTINGFILTER_H_INCL__
#define __TREEFULLSORTINGFILTER_H_INCL__

#include <string>
#include <set>
#include <list>
#include "SortingFilter.h"

namespace jedox {
namespace palo {

class SortingFilter;

class NameComparison;
class PositionalComparison;

/**@brief Used as an algorithm by TreeBuilder
 * SD == Show Duplicates */
class SDTreeFullSortingFilter {
public:
	bool check(const ELEMENT_INFO_EXT& elem);

	/*@brief: do nothing */
	void finalize(ElementExList& elemlist);

	/*@brief: sort according to names, evtl. reverse if reverse is set */
	void processChildren(ElementExList& elems);

	SDTreeFullSortingFilter(SubSet& concrete_subset, const SortingFilter&, AbstractComparison& comp);

private:
	SubSet& concrete_subset;
	const SortingFilter& m_filter;
	AbstractComparison& m_comp;

};

/**@brief Used as an algorithm by TreeBuilder
 *
 * same as SDTreeFullSortingFilter, but instead of showing duplicates,
 * we gonna hide them */

class TreeFullSortingFilter {
public:
	bool check(const ELEMENT_INFO_EXT& elem);
	/*@brief: do nothing */
	void finalize(ElementExList& elemlist);

	/*@brief: sort according to names, evtl. reverse if reverse is set */
	void processChildren(ElementExList& elems);

	TreeFullSortingFilter(SubSet& concrete_subset, const SortingFilter&, AbstractComparison& comp);

private:
	SubSet& concrete_subset;
	const SortingFilter& m_filter;
	AbstractComparison& m_comp;

};

} //palo
} //jedox
#endif							 // __TREEFULLSORTINGFILTER_H_INCL__
