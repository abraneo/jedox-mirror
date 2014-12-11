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

/*@file : another sorting filter that removes children if the parent is removed,
no matter whether the children pass the filter or not */

#include "TreePartialSortingFilter.h"
#include "SortingFilter.h"
#include <algorithm>
#include <boost/ref.hpp>
#include <boost/bind.hpp>

using namespace jedox::palo;

namespace jedox {
	namespace palo {

		class SDTreePartialSortingFilter::CopyToSet {
		public:
			CopyToSet( SubSet::id_set_type& given_names ): m_given_names( given_names ) {
				;
			}
			void operator()( const ELEMENT_INFO_EXT& einf ) {
				m_given_names.insert( einf.get_id() );
			}
			SubSet::id_set_type& m_given_names;
		};

		/*@brief: do nothing */

		void  SDTreePartialSortingFilter::finalize( ElementExList& elemlist ) {
			;
		}

		/*@brief: sort according to names, eventually reverse if reverse is set */

		void  SDTreePartialSortingFilter::processChildren( ElementExList& elems ) {
			elems.sort( boost::bind( &AbstractComparison::operator(),boost::ref( m_comp ), _1, _2 ) );
			if ( m_filter.m_parents_below ) {
				elems.reverse();
			}
		}

		SDTreePartialSortingFilter::SDTreePartialSortingFilter( ElementExList& given_elements, const SortingFilter& filter,
		                                                    AbstractComparison& comp ): m_given_elements( given_elements ), m_filter( filter ), m_comp( comp ) {
			std::for_each( m_given_elements.begin(), m_given_elements.end(), CopyToSet( m_given_names ) );
		}


		class TreePartialSortingFilter::CopyToSet {
		public:
			CopyToSet( SubSet::id_set_type& given_names ): m_given_names( given_names ) {
				;
			}
			void operator()( const ELEMENT_INFO_EXT& einf ) {
				m_given_names.insert( einf.get_id() );
			}
			SubSet::id_set_type& m_given_names;
		};

		/*@brief remove duplicates */

		void  TreePartialSortingFilter::finalize( ElementExList& elemlist ) {
			std::set<long> processed_ids;
			for (ElementExList::iterator it = elemlist.begin(); it != elemlist.end();) {
				if (processed_ids.find(it->get_id()) == processed_ids.end()) {
					processed_ids.insert(it->get_id());
					++it;
				} else {
					it = elemlist.erase(it);
				}
			}
		}

		/*@brief: sort according to names, eventually reverse if reverse is set */

		void  TreePartialSortingFilter::processChildren( ElementExList& elems ) {
			elems.sort( boost::bind( &AbstractComparison::operator(),boost::ref( m_comp ), _1, _2 ) );
			if ( m_filter.m_parents_below ) {
				elems.reverse();
			}
		}

		TreePartialSortingFilter::TreePartialSortingFilter( ElementExList& given_elements, const SortingFilter& filter,
			AbstractComparison& comp ): m_given_elements( given_elements ), m_filter( filter ), m_comp( comp ) {
				std::for_each( m_given_elements.begin(), m_given_elements.end(), CopyToSet( m_given_names ) );
		}

	}							 //palo
}								 //jedox
