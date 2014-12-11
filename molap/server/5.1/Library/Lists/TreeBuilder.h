/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Frieder Hofmann , Jedox AG, Freiburg, Germany
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef __TREEBUILDER_H_INCL__
#define __TREEBUILDER_H_INCL__

#include <list>
#include <set>
#include <string>

#include <boost/shared_ptr.hpp>

#include "SubSet.h"

namespace palo {

template<class Algorithm>
class TreeBuilder {
public:
	TreeBuilder(Algorithm& alg, SubSet::Iterator &it_beg, SubSet &subset);
	virtual ~TreeBuilder();
	void build(vector<SubElem> &result, bool usePath);
	void process(vector<SubElem> &result, const SortElem &elem, const string &path);
	void setReverse(bool b = true);
private:
	bool m_reverse_active;
	Algorithm &m_alg;
	SubSet::Iterator &m_beg;
public:
	SubSet &m_subset_ref;

};

} //palo
#endif							 // __TREEBUILDER_H_INCL__
