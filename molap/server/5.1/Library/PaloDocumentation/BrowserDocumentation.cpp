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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloDocumentation/BrowserDocumentation.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>

namespace palo {
BrowserDocumentation::BrowserDocumentation(const string& message)
{
	vector<string> m;
	m.push_back(message);
	values["@message"] = m;
	vector<string> x;
	x.push_back("Copyright 2006-2013 Jedox AG");
	values["@footerText"] = x;
}

bool BrowserDocumentation::hasDocumentationEntry(const string& name)
{
	map<string, vector<string> >::const_iterator i = values.find(name);
	return (i == values.end()) ? false : true;
}

const vector<string>& BrowserDocumentation::getDocumentationEntries(const string& name)
{
	static vector<string> error;
	map<string, vector<string> >::const_iterator i = values.find(name);
	return (i == values.end()) ? error : i->second;
}

const string& BrowserDocumentation::getDocumentationEntry(const string& name, size_t index)
{
	static string error = "";

	map<string, vector<string> >::const_iterator i = values.find(name);

	if (i == values.end()) {
		return error;
	}

	const vector<string> * entries = &(i->second);

	if (index < 0 || entries->size() <= index) {
		return error;
	}

	return (*entries)[index];
}

string BrowserDocumentation::convertToString(const IdentifiersType* elements)
{
	ostringstream s;
	bool start = true;

	for (IdentifiersType::const_iterator i = elements->begin(); i != elements->end(); ++i) {
		if (start) {
			start = false;
		} else {
			s << ",";
		}

		s << (IdentifierType)(*i);
	}

	return s.str();
}

string BrowserDocumentation::convertElementTypeToString(Element::Type elementType)
{
	switch (elementType) {
	case Element::NUMERIC:
		return "numeric";

	case Element::STRING:
		return "string";

	case Element::CONSOLIDATED:
		return "consolidated";

	default:
		return "undefined";
	}
}

string BrowserDocumentation::convertStatusToString(CPDatabase database)
{
	switch (database->getStatus()) {
	case Database::UNLOADED:
		return "not loaded";

	case Database::LOADED:
		return "loaded/saved";

	case Database::CHANGED:
		return "changed/new";

	default:
		return "error";
	}
}

string BrowserDocumentation::convertSecondsToString(size_t seconds)
{
	ostringstream ss;
	if (seconds / (60*60)) {
		ss << seconds / (60*60) << ":";
		seconds = seconds % (60*60);
	}
	ss.width(2);
	ss.fill('0');
	ss << seconds / 60 << ":";
	seconds = seconds % 60;
	ss.width(2);
	ss.fill('0');
	ss << seconds;
	return ss.str();
}

string BrowserDocumentation::convertStatusToString(CPCube cube)
{
	switch (cube->getStatus()) {
	case Cube::UNLOADED:
		return "not loaded";

	case Cube::LOADED:
		return "loaded/saved";

	case Cube::CHANGED:
		return "changed/new";

	default:
		return "error";
	}
}

string BrowserDocumentation::convertTypeToString(ItemType type)
{
	switch (type) {
	case NORMALTYPE:
		return "normal";

	case GPUTYPE:
		return "gpu type";

	case SYSTEMTYPE:
		return "system";

	case USER_INFOTYPE:
		return "user info";

	default:
		return "error";
	}
}

string BrowserDocumentation::convertTypeToString(CPDimension dimension)
{
	Dimension::SaveType type = dimension->getDimensionType();

	string typeString;

	switch (type) {
	case Dimension::NORMAL:
		typeString = "Normal";
		break;
	case Dimension::RIGHTS:
		typeString = "System";
		break;
	case Dimension::ALIAS:
		typeString = "Alias";
		break;
	case Dimension::ATTRIBUTES:
		typeString = "Attributes";
		break;
	case Dimension::CUBE:
		typeString = "Cubes";
		break;
	case Dimension::CONFIG:
		typeString = "Configuration";
		break;
	case Dimension::DIMENSION:
		typeString = "Dimensions";
		break;
	case Dimension::SUBSETVIEW:
		typeString = "Subsets";
		break;
	case Dimension::USERINFO:
		typeString = "UserInfo";
		break;
	case Dimension::CELLPROPS:
		typeString = "CellProperties";
		break;
	case Dimension::VIRTUAL:
		typeString = "SystemId";
		break;
	default:
		typeString = "Unknown";
		break;
	}
	return typeString;
}

string BrowserDocumentation::convertTypeToString(CPCube cube)
{
	ItemType type = cube->getType();
	if (type == NORMALTYPE) {
		return "normal";
	} else {
		if (type == GPUTYPE)
			return "gpu type";
		else if (type == USER_INFOTYPE) {
			return "user info";
		} else if (type == SYSTEMTYPE) {
			CPSystemCube c = CONST_COMMITABLE_CAST(SystemCube, cube);
			if (c->getCubeType() == Cube::ATTRIBUTES) {
				return "attributes";
			}
			return "system";
		}
	}
	return "error";
}

void BrowserDocumentation::defineDatabase(CPDatabase database)
{
	vector<CPDatabase> databases(1, database);
	defineDatabases(&databases);
}

void BrowserDocumentation::defineDatabases(const vector<CPDatabase> * databases)
{
	vector<string> idenfier;
	vector<string> name;
	vector<string> status;
	vector<string> type;
	vector<string> dimensions;
	vector<string> cubes;

	for (vector<CPDatabase>::const_iterator i = databases->begin(); i != databases->end(); ++i) {
		CPDatabase database = *i;

		idenfier.push_back(StringUtils::convertToString(database->getIdentifier()));
		name.push_back(StringUtils::escapeHtml(database->getName()));
		status.push_back(convertStatusToString(database));
		type.push_back(convertTypeToString(database->getType()));
		dimensions.push_back(StringUtils::convertToString((uint32_t)database->sizeDimensions()));
		cubes.push_back(StringUtils::convertToString((uint32_t)database->sizeCubes()));
	}

	values["@database_identifier"] = idenfier;
	values["@database_name"] = name;
	values["@database_status"] = status;
	values["@database_type"] = type;
	values["@database_num_dimensions"] = dimensions;
	values["@database_num_cubes"] = cubes;
}

void BrowserDocumentation::defineDimension(CPDimension dimension)
{
	vector<CPDimension> dimensions(1, dimension);
	defineDimensions(&dimensions);
}

void BrowserDocumentation::defineDimensions(const vector<CPDimension> * dimensions)
{
	vector<string> identifier;
	vector<string> name;
	vector<string> numElements;
	vector<string> maxLevel;
	vector<string> maxIndent;
	vector<string> maxDepth;
	vector<string> type;
	vector<string> numBaseElements;
	vector<string> numCells;

	double nCells = 1;
	for (vector<CPDimension>::const_iterator i = dimensions->begin(); i != dimensions->end(); ++i) {
		CPDimension dimension = *i;

		identifier.push_back(StringUtils::convertToString(dimension->getId()));
		name.push_back(StringUtils::escapeHtml(dimension->getName()));
		numElements.push_back(StringUtils::convertToString((int32_t)dimension->sizeElements()));
		maxLevel.push_back(StringUtils::convertToString((int32_t)dimension->getLevel()));
		maxIndent.push_back(StringUtils::convertToString((int32_t)dimension->getIndent()));
		maxDepth.push_back(StringUtils::convertToString((int32_t)dimension->getDepth()));
		type.push_back(convertTypeToString(dimension));

		int32_t num = 0;
		ElementsType el = dimension->getElements(PUser(), false);
		for (ElementsType::iterator j = el.begin(); j != el.end(); ++j) {
			if ((*j)->getElementType() == Element::NUMERIC) {
				num++;
			}
		}
		numBaseElements.push_back(StringUtils::convertToString(num));
		nCells *= num;
	}
	numCells.push_back(StringUtils::convertToString(nCells));

	values["@dimension_identifier"] = identifier;
	values["@dimension_name"] = name;
	values["@dimension_num_elements"] = numElements;
	values["@dimension_max_level"] = maxLevel;
	values["@dimension_max_indent"] = maxIndent;
	values["@dimension_max_depth"] = maxDepth;
	values["@dimension_type"] = type;
	values["@dimension_numeric_elements"] = numBaseElements;
	values["@dimension_numeric_cells"] = numCells;
}

void BrowserDocumentation::defineCube(CPCube cube)
{
	vector<CPCube> cubes(1, cube);
	defineCubes(&cubes);
}

void BrowserDocumentation::defineCubes(const vector<CPCube> * cubes)
{
	vector<string> identifier;
	vector<string> name;
	vector<string> dimensions;
	vector<string> status;
	vector<string> type;
	vector<string> rules;
	vector<string> size;
	vector<string> marked_cells;
	vector<string> cached_areas;
	vector<string> cached_cells;
	vector<string> cached_values_limit;
	vector<string> cached_values_found;
	vector<string> cache_time_info;

	for (vector<CPCube>::const_iterator i = cubes->begin(); i != cubes->end(); ++i) {
		CPCube cube = *i;

		identifier.push_back(StringUtils::convertToString(cube->getId()));
		name.push_back(StringUtils::escapeHtml(cube->getName()));

		// convert dimension list into string
		const IdentifiersType* dims = cube->getDimensions();

		dimensions.push_back(convertToString(dims));
		status.push_back(convertStatusToString(cube));
		type.push_back(convertTypeToString(cube));
		size_t rulesCount = cube->getRules(PUser(), false).size();
		rules.push_back(rulesCount ? StringUtils::convertToString(rulesCount) : "");
		size.push_back(StringUtils::convertToString(cube->sizeFilledCells()));
		marked_cells.push_back(StringUtils::convertToString(cube->sizeFilledMarkerCells()));

		size_t areasCount = 0;
		size_t valuesCount = 0;
		double cellCount = 0;
		double cellLimit = 0;
		double foundCellsCount = 0;
		ValueCache *cache = cube->getCache();
		if (cache) {
			PStorageCpu cacheStorage;
			ValueCache::PCachedAreas cacheAreas;
			double cacheFound = cache->getAreasAndStorage(cacheStorage, cacheAreas);
			if (cacheStorage && cacheAreas) {
				cache->getStatistics(cacheStorage, cacheAreas, cacheFound, areasCount, valuesCount, cellCount, foundCellsCount);
			}
			cellLimit = cache->getBarrier();

			time_t invalidationTime = cache->getInvalidationTime();
			time_t currentTime = time(0);
			size_t age = size_t(difftime(currentTime, invalidationTime));
			cache_time_info.push_back(convertSecondsToString(age)+"/"+StringUtils::convertToString(cache->getGeneration()));
		} else {
			cache_time_info.push_back("");
		}
		cached_areas.push_back(StringUtils::convertToString(areasCount));
		cached_cells.push_back(UTF8Comparer::doubleToString(cellCount, 0, 0));
		cached_values_limit.push_back(StringUtils::convertToString(valuesCount)+"/"+UTF8Comparer::doubleToString(cellLimit,0,0)+" ("+(cellLimit ? UTF8Comparer::doubleToString(100*valuesCount/cellLimit,0,2) : "0")+"%)");
		cached_values_found.push_back(UTF8Comparer::doubleToString(foundCellsCount,0,0));
	}

	values["@cube_identifier"] = identifier;
	values["@cube_name"] = name;
	values["@cube_status"] = status;
	values["@cube_type"] = type;
	values["@cube_rules"] = rules;
	values["@cube_dimensions"] = dimensions;
	values["@cube_size"] = size;
	values["@marked_cells"] = marked_cells;
	values["@cached_areas"] = cached_areas;
	values["@cached_cells"] = cached_cells;
	values["@cached_values_limit"] = cached_values_limit;
	values["@cached_values_found"] = cached_values_found;
	values["@cache_time_info"] = cache_time_info;
}

void BrowserDocumentation::defineElement(CPDimension dimension, Element* element, const string& prefix)
{
	IdentifiersType elementIds(1, element->getIdentifier());
	defineElements(dimension, elementIds.begin(), elementIds.end(), prefix);
}

}
