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

#ifndef PALO_DOCUMENTATION_BROWSER_DOCUMENTATION_H
#define PALO_DOCUMENTATION_BROWSER_DOCUMENTATION_H 1

#include "palo.h"

#include "Olap/Element.h"
#include "PaloDocumentation/Documentation.h"

#include "Olap/NormalDimension.h"
#include "Olap/SystemCube.h"
#include "Olap/SystemDimension.h"
#include "Olap/Database.h"


namespace palo {
class Cube;
class Dimension;

////////////////////////////////////////////////////////////////////////////////
/// @brief provides data documentation
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS BrowserDocumentation : public Documentation {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a data based documentation
	////////////////////////////////////////////////////////////////////////////////

	BrowserDocumentation(const string& message);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if a documentation entry was found
	////////////////////////////////////////////////////////////////////////////////

	bool hasDocumentationEntry(const string&);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a vector of values to a key word
	////////////////////////////////////////////////////////////////////////////////

	const vector<string>& getDocumentationEntries(const string&);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns one value to a keyword
	////////////////////////////////////////////////////////////////////////////////

	const string& getDocumentationEntry(const string& name, size_t index = 0);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts identifier list to string
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(const IdentifiersType*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts element type to string
	////////////////////////////////////////////////////////////////////////////////

	static string convertElementTypeToString(Element::Type);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts status to string
	////////////////////////////////////////////////////////////////////////////////

	static string convertStatusToString(CPDatabase);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts time in seconds to string
	////////////////////////////////////////////////////////////////////////////////

	string convertSecondsToString(size_t seconds);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts status to string
	////////////////////////////////////////////////////////////////////////////////

	static string convertStatusToString(CPCube);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts item type to string
	////////////////////////////////////////////////////////////////////////////////

	static string convertTypeToString(ItemType type);
	static string convertTypeToString(CPDimension dimension);
	static string convertTypeToString(CPCube cube);

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines database values
	////////////////////////////////////////////////////////////////////////////////

	void defineDatabase(CPDatabase database);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines database values
	////////////////////////////////////////////////////////////////////////////////

	void defineDatabases(const vector<CPDatabase> * databases);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines dimension values
	////////////////////////////////////////////////////////////////////////////////

	void defineDimension(CPDimension dimension);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines dimension values
	////////////////////////////////////////////////////////////////////////////////

	void defineDimensions(const vector<CPDimension> * dimensions);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines cube values
	////////////////////////////////////////////////////////////////////////////////

	void defineCube(CPCube cube);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines cube values
	////////////////////////////////////////////////////////////////////////////////

	void defineCubes(const vector<CPCube> * cubes);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines element values
	////////////////////////////////////////////////////////////////////////////////

	void defineElement(CPDimension, Element* element, const string& prefix);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief defines element values
	////////////////////////////////////////////////////////////////////////////////

	template<typename T> void defineElements(CPDimension dimension, T elementIdsBegin, T elementIdsEnd, const string &prefix)
	{
		vector<string> identifier;
		vector<string> name;
		vector<string> position;
		vector<string> level;
		vector<string> indent;
		vector<string> depths;
		vector<string> parents;
		vector<string> children;
		vector<string> type;
		vector<string> strCons;

		for (T i = elementIdsBegin; i != elementIdsEnd; ++i) {
			Element *element = dimension->lookupElement(*i, false);

			if (element) {
				identifier.push_back(StringUtils::convertToString(element->getIdentifier()));
				name.push_back(StringUtils::escapeHtml(element->getName(dimension->getElemNamesVector())));
				position.push_back(StringUtils::convertToString(element->getPosition()));
				level.push_back(StringUtils::convertToString(element->getLevel()));
				indent.push_back(StringUtils::convertToString(element->getIndent()));
				depths.push_back(StringUtils::convertToString(element->getDepth()));
				children.push_back(StringUtils::convertToString((int32_t)element->getChildrenCount()));
				parents.push_back(StringUtils::convertToString((int32_t)element->getParentsCount()));
				type.push_back(convertElementTypeToString(element->getElementType()));
				strCons.push_back(element->isStringConsolidation() ? "yes" : "no");
			}
		}

		values[prefix + "_identifier"] = identifier;
		values[prefix + "_name"] = name;
		values[prefix + "_position"] = position;
		values[prefix + "_level"] = level;
		values[prefix + "_indent"] = indent;
		values[prefix + "_depth"] = depths;
		values[prefix + "_parents"] = parents;
		values[prefix + "_children"] = children;
		values[prefix + "_type"] = type;
		values[prefix + "_string_consolidation"] = strCons;
	}

protected:
	map<string, vector<string> > values;
};

}

#endif
