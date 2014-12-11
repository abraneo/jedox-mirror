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

//TODO refactoring: add filter-communicator as a class inherited virtually
// by all filters. The class handles data and functionality shared by filters.

#ifndef _SUBSET_H
#define _SUBSET_H

#include <list>
#include <string>
#include <string.h>

#include <boost/scoped_ptr.hpp>

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Palo/Server.h>
#include <libpalo_ng/Palo/Dimension.h>
#include <libpalo_ng/Palo/Cube.h>

#include "Filter.h"

using namespace jedox::palo;

namespace jedox {
namespace palo {

class TextFilter;
class SortingFilter;
class PickList;
class StructuralFilter;
class AliasFilter;
class DataFilter;
class PostProcessor;

class LIBPALO_NG_CLASS_EXPORT SubSet {
private:
	typedef std::map<std::string, long, util::UTF8Comparer> attribute_dimension_map_type;
	struct checkPickList;
	struct SubSet_Impl;
	boost::scoped_ptr<SubSet_Impl> m_impl;

	struct SubSetStorage {
		typedef ElementExList list_type;
		typedef std::vector<int> look_up_type;
	};

public:

	typedef std::set<long> id_set_type;

	enum GlobalFlag {
		DATA_ONLY_LEAVES = 0x1, DATA_ONLY_CONSOLIDATED = 0x2, DATA_FILTER_ACTIVE = 0x4, ALIAS_FILTER_ACTIVE = 0x8, PICKLIST_MERGE = 0x10, PICKLIST_FRONT = 0x20, PICKLIST_BACK = 0x40, PICKLIST_SUB = 0x80, REVOLVE = 0x100, PICKLIST_ACTIVE = 0x200, DATA_STRING = 0x400, REVERSE = 0x800,
		//If Datafilter Top n or some kind of percentage
		//limit is set we won't show duplicates
		DONT_SHOW_DUPLICATES = 0x1000,
		//Structural filter with level bounds is active
		LEVEL_BOUNDS = 0x2000,
		BELOW_EXCLUSIVE = 0x4000,
		BELOW_INCLUSIVE = 0x8000,
		//structural filter is active
		STRUCTURAL_FILTER_ACTIVE = 0x10000

	};
public:
	//constructor
	SubSet(jedox::palo::Server* serv, const std::string& dim, const std::string& db);
	//destructor
	virtual ~SubSet();
private:

public:
	/**@brief Add a new filter component. If the operation fails,
	 the filter type is either already present or not
	 compatible with other filters or has failed some other check.
	 An exception is thrown.
	 The constructor for the filter is called implicitly. SubSet acts like a factory. */
	void addFilter(Filter::FilterType, unsigned int flags);

	/**@brief remove the Filter(s). If there is no such filter
	 the operation throws an exception*/
	void removeFilter(unsigned int type);

	/**@brief evaluate the filters and return the Element-List
	 if no filter has changed since the last invocation
	 of apply, nothing is recomputed. */
	ElementExList* applyFilters();
public:
	/*@brief remove all filters, keep dimension*/
	void clear();

	TextFilter* getTextFilter();
	SortingFilter* getSortingFilter();
	PickList* getPicklist();
	AliasFilter* getAliasFilter();
	StructuralFilter* getStrucuralFilter();
	DataFilter* getDataFilter();

	Dimension& getAttributeDimension();
	Dimension& getDimension();
	const ELEMENT_INFO& getDimElement(long id);
	Cube& getAttributeCube();
	Database& getDatabase();

	ElementExList& getConcreteSubset();

	//TODO : move this to filter_manager - class
	const ElementExList& getTopelements() const;

	//this is to pass the bounding element from the structural filter, to the sorting filter
	const ElementExList& getElemBound() const;
	void addElemBound(ELEMENT_INFO_EXT elem);

	void swapConcreteSet(std::list<ELEMENT_INFO>& other_set);
	void swapConcreteSet(ElementExList& other_set);
	//will return the id of the element in question
	long validateAttribute(const std::string& attr);

	void setGlobalFlag(GlobalFlag f);
	bool queryGlobalFlag(GlobalFlag f);
	/**@brief Apply the bounds given by StructuralFilterSettings (i.e. only show elements with indent > 1 */
	void applyStructuralBounds();
	void updateTop(bool nochildren, bool bound);
	bool checkId(long id);
	bool checkPath(const ELEMENT_INFO_EXT &elem);
	const ELEMENT_INFO_EXT *getElem(long id);
};

} //palo
} //jedox
#endif
