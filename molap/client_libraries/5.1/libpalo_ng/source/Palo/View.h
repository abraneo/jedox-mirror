/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#ifndef LIBPALO_NG_VIEW_H_
#define LIBPALO_NG_VIEW_H_

#include "libpalo_ng/Palo/types.h"
#include <boost/thread/mutex.hpp>
#include <set>
#include <boost/thread/thread_time.hpp>

namespace jedox {
namespace palo {

class Server;

struct VIEW_SUBSET {
	std::string database;
	std::string dimension;
	int indent;
	std::vector<BasicFilterSettings> basic;
	TextFilterSettings text;
	SortingFilterSettings sorting;
	AliasFilterSettings alias;
	FieldFilterSettings field;
	std::vector<StructuralFilterSettings> structural;
	std::vector<DataFilterSettings> data;

	std::string ser_ident;
	std::string ser_str;
	void Serialize(Server *server);
};

struct VIEW_AXIS_SUBSET {
	VIEW_AXIS_SUBSET(const std::string &subsetHandle, boost::shared_ptr<VIEW_SUBSET> sub, const std::string &parent, const std::string &calculation, bool zeroSup) : subsetHandle(subsetHandle), sub(sub), parent(parent), calculation(calculation), zeroSup(zeroSup) {}
	std::string subsetHandle;
	boost::shared_ptr<VIEW_SUBSET> sub;
	std::string parent;
	std::string calculation;
	bool zeroSup;
};

struct VIEW_AXIS {
	std::string database;
	int axisId;
	std::vector<VIEW_AXIS_SUBSET> as;

	std::string ser_ident;
	std::string ser_str;
	void Serialize();
};

struct VIEW_CALC_AXIS {
	std::vector<std::unordered_map<std::string, std::pair<size_t, ELEMENT_INFO::TYPE> > > nameIndex;
	std::vector<ElementExList> tuples;
};

struct VIEW_CALC {
	std::unordered_map<ELEMENT_LIST, CELL_VALUE_PROPS, boost::hash<ELEMENT_LIST> > values;
	std::vector<std::pair<size_t, size_t> > dimonaxis;
	std::map<std::string, size_t> dim2axis;
	std::vector<boost::shared_ptr<VIEW_CALC_AXIS> > axes;
	unsigned int sequence;
	bool virt;
};

struct VIEW_AREA {
	typedef std::unordered_map<ELEMENT_LIST, CELL_VALUE_PROPS, boost::hash<ELEMENT_LIST> >::const_iterator valiter;
	std::string database;
	std::string cube;
	std::vector<std::pair<std::string, boost::shared_ptr<VIEW_AXIS> > > axes;
	std::vector<std::string> properties;
	boost::shared_ptr<VIEW_CALC> calc;

	std::string ser_ident;
	std::string ser_str;
	void Serialize();
};

class View {
public:
	View(Server *server);
	std::string defineViewSubset(const std::string &database, const std::string &dimension, int indent, const std::vector<BasicFilterSettings> &basic, const TextFilterSettings &text, const SortingFilterSettings &sorting, const AliasFilterSettings &alias, const FieldFilterSettings &field, const std::vector<StructuralFilterSettings> &structural, const std::vector<DataFilterSettings> &data);
	std::string defineViewAxis(const std::string &database, int axisId, const AxisSubsets &as);
	std::string defineViewArea(const std::string &database, const std::string &cube, const std::vector<std::string> &axes, const std::vector<std::string> &properties);
	size_t getViewAxisSize(const std::string &database, const std::string &viewHandle, int axisId);
	ELEMENT_INFO_EXT getViewSubset(const std::string &database, const std::string &viewHandle, int axisId, size_t subsetPos, size_t elemPos, UINT &indent);
	AxisElement getViewSubset(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos);
	CELL_VALUE_PROPS getViewArea(const std::string &database, const std::string &viewHandle, const std::vector<std::string> &coord, bool cubeException, std::vector<std::string> *&properties);
	std::vector<AxisElement> getViewSubsetTop(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, size_t start, size_t limit);
	std::vector<AxisElement> getViewSubsetChildren(const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos, const std::string &element);

private:
	static const int OBSOLETE_OBJECT_MINUTES = 5;
	void calculateView(boost::shared_ptr<VIEW_AREA> view_area);
	void updateParentChild(boost::shared_ptr<VIEW_CALC_AXIS> curr, size_t start, size_t ind, std::vector<std::map<IdentifierType, size_t> > &knownElems);
	void deleteOld();
	bool hasSameParentSubsets(const std::vector<ELEMENT_INFO_EXT> &v1, const std::vector<ELEMENT_INFO_EXT> &v2, size_t count);
	boost::shared_ptr<VIEW_CALC_AXIS> getAxisAndPos(std::pair<size_t, size_t> &axpos, IdentifierType &indent, const std::string &database, const std::string &viewHandle, const std::string &dim, size_t elemPos);

	Server *m_Server;
	std::unordered_map<std::string, std::pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_SUBSET> > > m_Subsets;
	std::unordered_map<std::string, std::pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AXIS> > > m_Axes;
	std::unordered_map<std::string, std::pair<boost::posix_time::ptime, boost::shared_ptr<VIEW_AREA> > > m_Areas;

	std::unordered_map<std::string, std::string> m_SubsetsCache;
	std::unordered_map<std::string, std::string> m_AxisCache;
	std::unordered_map<std::string, std::string> m_AreasCache;

	boost::mutex m_Lock;
};

}
}

#endif /* LIBPALO_NG_VIEW_H_ */
