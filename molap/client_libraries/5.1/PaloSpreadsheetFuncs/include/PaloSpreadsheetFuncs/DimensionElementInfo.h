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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef DIMENSION_ELEMENT_INFO_H
#define DIMENSION_ELEMENT_INFO_H

#include <string>
#include <vector>

#include "DimensionElementType.h"

namespace Palo {
namespace Types {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Stores most important information about a dimension element.
 */
struct DimensionElementInfoSimple {
	std::string name;
	DimensionElementType type;
	long int identifier;

	DimensionElementInfoSimple(const std::string& n, const DimensionElementType t, const long int id) :
			name(n), type(t), identifier(id)
	{
	}

	DimensionElementInfoSimple() :
			name(""), type(DimensionElementType::Numeric), identifier(-1)
	{
	}
};

inline const bool operator<(const DimensionElementInfoSimple& lhs, const DimensionElementInfoSimple& rhs)
{
	return lhs.identifier < rhs.identifier;
}

inline const bool operator==(const DimensionElementInfoSimple& lhs, const DimensionElementInfoSimple& rhs)
{
	return lhs.identifier == rhs.identifier;
}

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Stores information about a consolidation element.
 */
struct ConsolidationElementInfo : public DimensionElementInfoSimple {

	ConsolidationElementInfo(const std::string& n, const DimensionElementType t, const long int id, double w) :
			DimensionElementInfoSimple(n, t, id), weight(w)
	{
	}

	ConsolidationElementInfo() :
			DimensionElementInfoSimple(), weight(1.0)
	{
	}

	ConsolidationElementInfo(const DimensionElementInfoSimple& deis, double w) :
			DimensionElementInfoSimple(deis), weight(w)
	{
	}
	double weight;
};

/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Stores information about a dimension element
 *         (including parents and children).
 */
struct DimensionElementInfo : public DimensionElementInfoSimple {
	typedef std::vector<DimensionElementInfoSimple> ParentsList;
	typedef std::vector<ConsolidationElementInfo> ChildrenList;

	unsigned int level;
	unsigned int indent;
	unsigned int depth;
	unsigned int position;
	size_t num_parents;
	size_t num_children;
	ParentsList parents;
	ChildrenList children;
	bool locked;

	DimensionElementInfo() :
			level(0), indent(0), depth(0), position(0), num_parents(0), num_children(0), locked(false)
	{
	}
};

struct DimensionElementInfoPerm : public DimensionElementInfo {
	std::string permission;
};

/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief Stores information about a parent element
 */
struct ParentElementInfo {
	long int identifier;

	ParentElementInfo() :
			identifier(-1)
	{
	}

	ParentElementInfo(long int id) :
			identifier(id)
	{
	}

};

/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief Stores information about a child element
 */
struct ChildElementInfo {
	long int identifier;
	double weight;

	ChildElementInfo() :
			identifier(-1), weight(0)
	{
	}

	ChildElementInfo(long int id, double w) :
			identifier(id), weight(w)
	{
	}

};

/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief ParentElementInfoArray.
 */
typedef std::vector<ParentElementInfo> ParentElementInfoArray;

/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief ChildElementInfoArray.
 */
typedef std::vector<ChildElementInfo> ChildElementInfoArray;

/*! \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 *  \brief Stores information about a dimension element
 *         (including parents and children).
 */
struct DimensionElementInfoReduced : public DimensionElementInfoSimple {

	long level;
	long indent;
	long depth;
	long position;
	size_t num_parents;
	size_t num_children;
	ParentElementInfoArray parents;
	ChildElementInfoArray children;

	DimensionElementInfoReduced() :
			level(0), indent(0), depth(0), position(0), num_parents(0), num_children(0)
	{
	}

	DimensionElementInfoReduced(const jedox::palo::ELEMENT_INFO &ei) :
			DimensionElementInfoSimple(ei.nelement, ei.type, ei.element)
	{
		size_t i;
		level = ei.level;
		indent = ei.indent;
		depth = ei.depth;
		position = ei.position;
		num_parents = ei.parents.size();
		num_children = ei.children.size();

		parents.resize(num_parents);
		for (i = 0; i < num_parents; i++) {
			parents[i] = ei.parents[i];
		}

		children.resize(num_children);
		for (i = 0; i < num_children; i++) {
			children[i].identifier = ei.children[i];
			children[i].weight = ei.weights[i];
		}

	}

};
}
}
#endif
