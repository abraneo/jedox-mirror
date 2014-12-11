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
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#include <libpalo_ng/Palo/Database.h>
#include <libpalo_ng/Util/StringUtils.h>

#include "Filter.h"
#include "SubSet.h"
#include "ListBasicException.h"
#include "PickList.h"
#include "TextFilter.h"
#include "SortingFilter.h"
#include "DataFilter.h"
#include "AliasFilter.h"
#include "StructuralFilter.h"

using namespace jedox::palo;
using namespace std;

namespace jedox {
namespace palo {

struct SubSet::SubSet_Impl {
	std::vector<Filter*> m_filters;
	jedox::palo::Server* server;
	unsigned int m_dimToken;
	unsigned int m_attrDimToken;
	std::string dbname;
	std::string dimname;
	Database db;
	boost::scoped_ptr<Cube> attr_cube;
	Dimension dim;
	boost::scoped_ptr<Dimension> attr_dim;

	/* List with elements in hierarchical or positional order,
	 eventually sorted. Multi elements possible if order is hierarchical*/
	ElementExList concrete_subset;

	/* list of elements considered to be on top of the hierarchy
	 the hierarchical tree is build below these elements*/
	ElementExList topelements;

	/* this is a copy of the topelements in most cases, if the structural
	 filter is used with an above/below bounding element, this list will hold that element
	 and will be used instead of the top elements */
	ElementExList elem_bound;
	attribute_dimension_map_type attribute_dimension;

	// should the list be cached or deleted after apply() is called
	unsigned long m_global_flags;
	map<long, ELEMENT_INFO_EXT*> concrete_ids;

	// should the topelements be fixed, used if structural filter is used with an elem_bound
	void refresh();
	void reload_dim();
	void reload_attr_dim();

	void updateTop(bool nochildren, bool bound);
	void updateTopIntern(ElementExList& list, bool nochildren);
	bool checkPath(const ELEMENT_INFO_EXT &elem);

	// Apply the bounds given by StructuralFilterSettings (i.e. only show elements with indent > 1
	void applyStructuralBounds(bool picklist_merge = false);

	SubSet_Impl(jedox::palo::Server *serv, const std::string& db_name, const std::string& dim_name) :
		m_filters(Filter::TOTAL_NUMBER), server(serv), dbname(db_name), dimname(dim_name), db((*(serv))[db_name]), dim(db.dimension[dim_name]), m_global_flags(0)
	{
	}
};

SubSet::SubSet(jedox::palo::Server* serv, const std::string& db, const std::string& dim) :
	m_impl(new SubSet_Impl(serv, db, dim))
{

	SubSet_Impl& impl = *m_impl;

	std::vector<Filter*>::iterator pos1;
	for (pos1 = impl.m_filters.begin(); pos1 != impl.m_filters.end(); ++pos1) {
		*pos1 = NULL;
	}
	std::unique_ptr<DimensionCache::CacheIterator> it = impl.dim.getIterator();
	DimensionCache::CacheIterator& it_ref = *it;
	ElementExList& concrete_subset = impl.concrete_subset;
	ElementExList& top_elems = impl.topelements;

	//this set is needed to check if we can access the parents of an element,
	//if not we have to use it as a new topelement
	id_set_type tmp_elem_lookup;

	while (!(it_ref).end()) {
		ELEMENT_INFO_EXT elem(*it_ref);
		if (elem.m_einf.parents.size() == 0) {
			top_elems.push_back(elem);
		}
		tmp_elem_lookup.insert(elem.get_id());

		concrete_subset.push_back(elem);
		++(it_ref);
	}
	{
		id_set_type::const_iterator tmp_end(tmp_elem_lookup.end());
		//id_set_type::const_iterator tmp_top_end(tmp_top_elems_lookup.end());

		for (ElementExList::const_iterator sub_it(concrete_subset.begin()), sub_end(concrete_subset.end()); sub_it != sub_end; ++sub_it) {
			bool no_parent_found = !sub_it->m_einf.parents.empty();
			for (ELEMENT_LIST::const_iterator p_it(sub_it->m_einf.parents.begin()), p_end(sub_it->m_einf.parents.end()); no_parent_found && p_it != p_end; ++p_it) {
				if (tmp_elem_lookup.find(*p_it) != tmp_end) {
					no_parent_found = false;
				}
			}
			if (no_parent_found) {
				top_elems.push_back(*sub_it);
			}
		}
	}

	impl.m_dimToken = impl.dim.getSequenceNumberFromCache();

	if (m_impl->dim.getCacheData().assoc_dimension != -1) {
		it = getAttributeDimension().getIterator();
		DimensionCache::CacheIterator& attr_it_ref = *it;
		while (!(attr_it_ref).end()) {
			impl.attribute_dimension[(*attr_it_ref).nelement] = (*attr_it_ref).element;
			++(attr_it_ref);
		}
		impl.m_attrDimToken = getAttributeDimension().getSequenceNumberFromCache();
	}
}

/*@brief deletes all filters and keeps Meta-Information*/
void SubSet::clear()
{
	std::vector<Filter*>::iterator pos1;
	for (pos1 = m_impl->m_filters.begin(); pos1 != m_impl->m_filters.end(); ++pos1) {
		if ((*pos1) != NULL) {
			delete *pos1;
			*pos1 = NULL;
		}
	}
	m_impl->m_global_flags = 0;
}

SubSet::~SubSet()
{
	clear();
}

//Add a new filter component. If the operation fails,
//the filter type is not compatible with other filters, does not exist, or has failed some other check.
//The constructor for the filter is called implicitly. SubSet acts like a factory.

void SubSet::addFilter(Filter::FilterType type, unsigned int flags)
{
	if (m_impl->m_filters[type] != NULL) {
		delete m_impl->m_filters[type];
	}
	{
		switch (type) {
		case Filter::PICKLIST:
			m_impl->m_filters[type] = new PickList(*this, flags);
			break;
		case Filter::TEXT:
			m_impl->m_filters[type] = new TextFilter(*this, flags);
			break;
		case Filter::SORTING:
			m_impl->m_filters[type] = new SortingFilter(*this, flags);
			break;
		case Filter::ALIAS:
			m_impl->m_filters[type] = new AliasFilter(*this, flags);
			break;
		case Filter::STRUCTURAL:
			m_impl->m_filters[type] = new StructuralFilter(*this, flags);
			break;
		case Filter::DATA:
			m_impl->m_filters[type] = new DataFilter(*this, flags);
			break;
		default:
			throw ListBasicException("The given filter type does not exist", "Filter undefined.");
		}
	}
	return;
}

//remove the Filter(s). If there is no such filter
//, the operation throws.
void SubSet::removeFilter(unsigned int type)
{
	if (m_impl->m_filters[type] == NULL) {
		return;
	}
	delete m_impl->m_filters[type];
	m_impl->m_filters[type] = NULL;
}

ElementExList* SubSet::applyFilters()
{
	SubSet_Impl& impl = *m_impl;
	//TODO: changed should be true whenever a filter is modified in
	//some way. ATM we always refresh, even when we only read filter properties etc.
	//update dimension if it has changed on the server
	if (impl.dim.getSequenceNumberFromCache() != impl.m_dimToken) {
		impl.reload_dim();
	}
	//update attribute dimension if it has changed on the server
	if (impl.attr_dim && getAttributeDimension().getSequenceNumberFromCache() != impl.m_attrDimToken) {
		impl.reload_attr_dim();
	}

	//we always need a sorting filter
	if (impl.m_filters[Filter::SORTING] == NULL) {
		impl.m_filters[Filter::SORTING] = new SortingFilter(*this, (SortingFilter::FLAT_HIERARCHY | SortingFilter::POSITION));
	}
	for (int i = 0; i < Filter::TOTAL_NUMBER; ++i) {
		if (impl.m_filters[i] != NULL) {
			impl.m_filters[i]->apply();
		}
	}

	return &(impl.concrete_subset);
}

struct SubSet::checkPickList {
	checkPickList(std::list<ELEMENT_INFO_EXT>::const_iterator begin, std::list<ELEMENT_INFO_EXT>::const_iterator end) :
		begin(begin), end(end)
	{
	}

	bool operator()(ELEMENT_INFO_EXT& e) const
	{
		return std::binary_search(begin, end, e);
	}
	ElementExList::const_iterator begin;
	ElementExList::const_iterator end;

};
void SubSet::SubSet_Impl::applyStructuralBounds(bool picklist_merge)
{
	if (NULL != m_filters[Filter::STRUCTURAL]) {
		StructuralFilter* sFilter = static_cast<StructuralFilter*> (m_filters[Filter::STRUCTURAL]);
		const unsigned int start = sFilter->m_level_begin;
		const unsigned int end = sFilter->m_level_end;

		ElementExList::iterator it = concrete_subset.begin();
		if (picklist_merge) {
			PickList* picklist = static_cast<PickList*> (m_filters[Filter::PICKLIST]);
			ElementExList pick_elems = picklist->getElements();
			pick_elems.sort();
			checkPickList check(pick_elems.begin(), pick_elems.end());
			switch (sFilter->m_indent) {
			//case 1 == default...
			case 2:
				while (it != concrete_subset.end()) {
					if ((it->get_level() < start || it->get_level() > end) && check(*it)) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			case 3:
				while (it != concrete_subset.end()) {
					if ((it->m_einf.depth < start || it->m_einf.depth > end) && check(*it)) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			default:
				while (it != concrete_subset.end()) {
					if ((it->m_einf.indent < start || it->m_einf.indent > end) && check(*it)) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			}

		} else {
			switch (sFilter->m_indent) {
			//case 1 == default...
			case 2:
				while (it != concrete_subset.end()) {
					if (it->get_level() < start || it->get_level() > end) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			case 3:
				while (it != concrete_subset.end()) {
					if (it->m_einf.depth < start || it->m_einf.depth > end) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			default:
				while (it != concrete_subset.end()) {
					if (it->m_einf.indent < start || it->m_einf.indent > end) {
						it = concrete_subset.erase(it);
					} else {
						++it;
					}
				}
				break;
			}

		}
	}

}

void SubSet::SubSet_Impl::reload_dim()
{
	concrete_subset.clear();
	topelements.clear();
	std::unique_ptr<DimensionCache::CacheIterator> it = dim.getIterator();
	DimensionCache::CacheIterator& it_ref = *it;
	while (!it_ref.end()) {
		const ELEMENT_INFO& e_info = *it_ref;
		concrete_subset.push_back(ELEMENT_INFO_EXT(e_info));
		if ((e_info).parents.size() == 0) {
			topelements.push_back(ELEMENT_INFO_EXT(e_info));
		}
		++it_ref;
	}
	m_dimToken = dim.getSequenceNumberFromCache();
}

void SubSet::SubSet_Impl::reload_attr_dim()
{
	attribute_dimension.clear();
	std::unique_ptr<DimensionCache::CacheIterator> it = attr_dim->getIterator();
	DimensionCache::CacheIterator& it_ref = *it;
	while (!it_ref.end()) {
		const ELEMENT_INFO& e_info = *it_ref;
		attribute_dimension[e_info.nelement] = e_info.element;
		++(it_ref);
	}
	m_attrDimToken = attr_dim->getSequenceNumberFromCache();
}

void SubSet::SubSet_Impl::updateTop(bool nochildren, bool bound)
{
	if (bound) {
		updateTopIntern(elem_bound, nochildren);
	} else {
		updateTopIntern(topelements, nochildren);
	}
}

void SubSet::SubSet_Impl::updateTopIntern(ElementExList& list, bool nochildren)
{
	for (ElementExList::iterator it = concrete_subset.begin(); it != concrete_subset.end(); ++it) {
		concrete_ids.insert(make_pair(it->get_id(), &*it));
	}
	for (ElementExList::iterator it = list.begin(); it != list.end();) {
		map<long, ELEMENT_INFO_EXT*>::iterator itid = concrete_ids.find(it->get_id());
		if (itid == concrete_ids.end()) {
			if (nochildren || !checkPath(*it)) {
				it = list.erase(it);
			} else {
				++it;
			}
		} else {
			*it = *itid->second;
			++it;
		}
	}
}

bool SubSet::SubSet_Impl::checkPath(const ELEMENT_INFO_EXT &elem)
{
	if (concrete_ids.find(elem.get_id()) != concrete_ids.end()) {
		return true;
	}
	for (ELEMENT_LIST::const_iterator it = elem.m_einf.children.begin(); it != elem.m_einf.children.end(); ++it) {
		try {
			ELEMENT_INFO_EXT e = dim[*it].getCacheData();
			if (concrete_ids.find(e.get_id()) != concrete_ids.end()) {
				return true;
			}
			if (checkPath(e)) {
				return true;
			}
		} catch (PaloException &) {
		}
	}
	return false;
}

/**TODO: we set changed to true whenever the client gets a
 * filter. This can be optimized. Changed= true for only
 * those filter functions that really do change something. */
//get the text filter from this subset
//e.g. to add an existing text filter to another subset.
TextFilter* SubSet::getTextFilter()
{

	if (dynamic_cast<TextFilter*> (m_impl->m_filters[Filter::TEXT])) {
		return dynamic_cast<TextFilter*> (m_impl->m_filters[Filter::TEXT]);
	} else
		throw ListBasicException("Tried to get uninitialized Text filter", "Filter not initialized");
}

SortingFilter* SubSet::getSortingFilter()
{
	if (dynamic_cast<SortingFilter*> (m_impl->m_filters[Filter::SORTING])) {
		return dynamic_cast<SortingFilter*> (m_impl->m_filters[Filter::SORTING]);
	} else
		throw ListBasicException("Tried to get uninitialized Sorting filter", "Filter not initialized");
}

PickList* SubSet::getPicklist()
{
	if (dynamic_cast<PickList*> (m_impl->m_filters[Filter::PICKLIST])) {
		return dynamic_cast<PickList*> (m_impl->m_filters[Filter::PICKLIST]);
	} else
		throw ListBasicException("Tried to get uninitialized Basic filter", "Filter not initialized");
}

AliasFilter* SubSet::getAliasFilter()
{
	if (dynamic_cast<AliasFilter*> (m_impl->m_filters[Filter::ALIAS])) {
		return dynamic_cast<AliasFilter*> (m_impl->m_filters[Filter::ALIAS]);
	} else
		throw ListBasicException("Tried to get uninitialized Alias filter", "Filter not initialized");
}

StructuralFilter* SubSet::getStrucuralFilter()
{
	if (dynamic_cast<StructuralFilter*> (m_impl->m_filters[Filter::STRUCTURAL])) {
		return dynamic_cast<StructuralFilter*> (m_impl->m_filters[Filter::STRUCTURAL]);
	} else
		throw ListBasicException("Tried to get uninitialized Structural filter", "Filter not initialized");
}

DataFilter* SubSet::getDataFilter()
{
	if (dynamic_cast<DataFilter*> (m_impl->m_filters[Filter::DATA])) {
		return dynamic_cast<DataFilter*> (m_impl->m_filters[Filter::DATA]);
	} else
		throw ListBasicException("Tried to get uninitialized Data filter", "Filter not initialized");
}

ElementExList& SubSet::getConcreteSubset()
{
	return m_impl->concrete_subset;
}

const ElementExList& SubSet::getTopelements() const
{
	return m_impl->topelements;
}

const ElementExList& SubSet::getElemBound() const
{
	return m_impl->elem_bound;
}

Dimension& SubSet::getDimension()
{
	return m_impl->dim;
}

const ELEMENT_INFO& SubSet::getDimElement(long id)
{
	return m_impl->dim[id].getCacheData();
}

Dimension& SubSet::getAttributeDimension()
{
	if (!m_impl->attr_dim) {
		m_impl->attr_dim.reset(new Dimension(m_impl->db.dimension[m_impl->dim.getCacheData().assoc_dimension]));
	}
	return *m_impl->attr_dim;
}

Cube& SubSet::getAttributeCube()
{
	if (!m_impl->attr_cube) {
		m_impl->attr_cube.reset(new Cube(m_impl->db.cube["#_" + m_impl->dimname]));
	}
	return *m_impl->attr_cube;
}

Database& SubSet::getDatabase()
{
	return m_impl->db;
}

void SubSet::swapConcreteSet(ElementExList& other_set)
{
	m_impl->concrete_subset = other_set;
}

void SubSet::swapConcreteSet(std::list<ELEMENT_INFO>& other_set)
{
	m_impl->concrete_subset.clear();
	for (std::list<ELEMENT_INFO>::iterator it_beg = other_set.begin(); it_beg != other_set.end(); ++it_beg) {
		m_impl->concrete_subset.push_back(*it_beg);
	}
}

long SubSet::validateAttribute(const std::string& attr)
{
	attribute_dimension_map_type::const_iterator it = m_impl->attribute_dimension.find(attr);
	if (it != m_impl->attribute_dimension.end()) {
		return it->second;
	}
	throw ListBasicException("Attempt to use non-existing attribute '" + attr + "'", "Invalid attribute");
}

void SubSet::setGlobalFlag(GlobalFlag f)
{
	m_impl->m_global_flags = (m_impl->m_global_flags | (unsigned long)f);
}

bool SubSet::queryGlobalFlag(GlobalFlag f)
{
	return ((m_impl->m_global_flags & (unsigned long)f) != 0);
}

void SubSet::applyStructuralBounds()
{
	m_impl->applyStructuralBounds(queryGlobalFlag(PICKLIST_MERGE));
}

void SubSet::addElemBound(ELEMENT_INFO_EXT elem)
{
	m_impl->elem_bound.push_back(elem);
}

void SubSet::updateTop(bool nochildren, bool bound)
{
	m_impl->updateTop(nochildren, bound);
}

bool SubSet::checkId(long id)
{
	return m_impl->concrete_ids.find(id) != m_impl->concrete_ids.end();
}

bool SubSet::checkPath(const ELEMENT_INFO_EXT &elem)
{
	return m_impl->checkPath(elem);
}

const ELEMENT_INFO_EXT *SubSet::getElem(long id)
{
	map<long, ELEMENT_INFO_EXT *>::const_iterator it = m_impl->concrete_ids.find(id);
	return it == m_impl->concrete_ids.end() ? 0 : it->second;
}

} //palo
} //jedox
