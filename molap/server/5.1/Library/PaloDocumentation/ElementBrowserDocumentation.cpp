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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * 
 *
 */

#include "PaloDocumentation/ElementBrowserDocumentation.h"

#include <functional>
#include <iostream>
#include <sstream>

#include "Olap/Dimension.h"

namespace palo {
ElementBrowserDocumentation::ElementBrowserDocumentation(PDatabase database, PDimension dimension, Element *element, const string &message) :
	BrowserDocumentation(message)
{
	vector<string> ids;
	ids.push_back(StringUtils::convertToString(database->getId()));
	values["@database_identifier"] = ids;

	vector<string> did;
	did.push_back(StringUtils::convertToString(dimension->getId()));
	values["@dimension_identifier"] = did;

	defineElement(dimension, element, "@element");

	// the parents of the element
	CPParents parents = element->getParents();
	if (parents) {
		defineElements(dimension, parents->begin(), parents->end(), "@parent");
	}

	// the children of the element
	const IdentifiersWeightType *children = element->getChildren();

	IdentifiersType childrenElements;

	if (children) {
		childrenElements.reserve(children->size());
		for (IdentifiersWeightType::const_iterator i = children->begin(); i != children->end(); ++i) {
			childrenElements.push_back((*i).first);
		}
	}

	defineElements(dimension, childrenElements.begin(), childrenElements.end(), "@child");

	// and their weight
	vector<string> weight;

	if (children) {
		for (IdentifiersWeightType::const_iterator i = children->begin(); i != children->end(); ++i) {
			weight.push_back(StringUtils::convertToString((*i).second));
		}
	}

	values["@child_weight"] = weight;
}
}
