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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloJobs/AreaJob.h"

#include "Engine/EngineBase.h"
#include "InputOutput/Statistics.h"
#include "Olap/AttributesCube.h"

namespace palo {

size_t AreaJob::s_max_cell_count;

bool AreaJob::checkElement(CPDimension &dim, Element *e, vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, PDatabase &database, PUser &user)
{
	bool result = true;
	if (checkPermissions && User::checkUser(user)) {
		result = user->getElementRight(database->getId(), dim->getId(), e->getIdentifier(), vRights, true) >= RIGHT_READ;
	}
	return result;
}

void AreaJob::appendValue(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals)
{
	StringBuffer *sb = &response->getBody();
	if (value.isError()) {
		sb->appendCsvInteger((int32_t)99);
		sb->appendCsvInteger((int32_t)value.getError());
		sb->appendCsvString(StringUtils::escapeString(ErrorException::getDescriptionErrorType(value.getError())));
	} else {
		sb->appendCsvInteger(value.isNumeric() ? (int32_t)1 : (int32_t)2);
		if (!value.isEmpty()) {
			sb->appendCsvString("1");
			sb->appendCsvString(value.toString());
		} else {
			sb->appendCsvString("0;");
		}
	}
	for (size_t i = 0; i < key.size(); i++) {
		if (0 < i) {
			sb->appendChar(',');
		}
		sb->appendInteger(key[i]);
	}
	sb->appendChar(';');
	if (jobRequest->showRule) {
		if (value.getRuleId() != NO_RULE) {
			sb->appendCsvInteger((uint32_t)value.getRuleId());
		} else {
			sb->appendChar(';');
		}
	}
	if (jobRequest->showLockInfo) {
		if (value.isError()) {
			sb->appendCsvInteger(0);
		} else {
			Cube::CellLockInfo lockInfo = 0;
			lockInfo = cube->getCellLockInfo(key, user ? user->getId() : 0);
			sb->appendCsvInteger((uint32_t)lockInfo);
		}
	}
	if (!prop_vals.empty()) {
		for (vector<CellValue>::const_iterator it = prop_vals.begin(); it != prop_vals.end(); ++it) {
			if (it != prop_vals.begin()) {
				sb->appendChar(',');
			}
			if (!it->isEmpty()) {
				sb->appendText(it->toString());
			}
		}
		sb->appendChar(';');
	}
	sb->appendEol();
}

double AreaJob::fillEmptyDim(vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, vector<IdentifiersType> &area, PCube &cube, PDatabase &database, PUser &user)
{
	const IdentifiersType *dimensions = cube->getDimensions();
	for (size_t i = 0; i < area.size(); i++) {
		if (!area.at(i).size()) {
			CPDimension dim = database->lookupDimension(dimensions->at(i), false);
			ElementsType elements = dim->getElements(PUser(), false);

			if (dim->getDimensionType() == Dimension::VIRTUAL && elements.empty()) {
				area.at(i).push_back(ALL_IDENTIFIERS);
			} else {
				for (ElementsType::iterator j = elements.begin(); j != elements.end(); ++j) {
					if (!database->getHideElements() || checkElement(dim, *j, vRights, checkPermissions, database, user)) {
						Element *elem = *j;
						area.at(i).push_back(elem->getIdentifier());
					}
				}
			}
		}
	}
	double size = 1.0;
	for (size_t i = 0; i < area.size(); i++) {
		if (area.at(i).size() == 1 && area.at(i).at(0) == ALL_IDENTIFIERS) {
			size *= double(IdentifierType(-1));
		} else {
			size *= area.at(i).size();
		}
	}
	return size;
}

PCubeArea AreaJob::checkRights(vector<User::RoleDbCubeRight> &vRights, bool checkPermissions, CPArea area, bool* hasStringElem, PCube &cube, PDatabase &database, PUser &user, bool reduceCalcArea, PArea &noPermission, bool &isNoPermission, vector<CPDimension> &dims)
{
	const IdentifiersType *dimensions = cube->getDimensions();
	size_t dimCount = dimensions->size();
	noPermission.reset(new Area(dimCount));
	isNoPermission = false;

	if (hasStringElem) {
		*hasStringElem = false;
	}
	PCubeArea calcArea(new CubeArea(database, cube, dimCount));
	for (size_t i = 0; i < dimCount; i++) {
		PSet s(new Set(*area->getDim(i).get()));
		calcArea->insert(i, s);

		CPDimension dim = database->lookupDimension(dimensions->at(i), false);
		dims.push_back(dim);

		PSet np(new Set);

		if (calcArea->elemCount(i) && dim->getDimensionType() != Dimension::VIRTUAL) {
			bool changed = false;
			for (Set::Iterator it = s->begin(); it != s->end();) {
				Element *elem = dim->lookupElement(*it, false);
				if (elem) {
					if (reduceCalcArea && !checkElement(dim, elem, vRights, checkPermissions, database, user)) {
						np->insert(*it);
						isNoPermission = true;
						it = s->erase(it); // elements without permission are removed from calcArea to make the calculation faster
						changed = true;
					} else {
						++it;
						if (hasStringElem) {
							if (elem->getElementType() == Element::STRING || elem->isStringConsolidation()) {
								*hasStringElem = true;
							}
						}
					}
				} else {
					throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid element in AreaJob::checkRights");
				}
			}
			if (changed) {
				//update of calcArea->areaSize
				CPSet s = calcArea->getDim(i);
				if (s->size()) {
					calcArea->insert(i, s);
				} else {
					calcArea->insert(i, PSet());
				}
			}
		}

		if (reduceCalcArea) {
			noPermission->insert((IdentifierType)i, np);
		}
	}
	return calcArea;
}

bool AreaJob::loop(CPArea area, PCubeArea calcArea, PCellStream cs, uint64_t *maxCount, PCellStream props, vector<User::RoleDbCubeRight> &vRights, IdentifiersType *lastKey, bool emptyValues)
{
	uint64_t freeCount = (maxCount ? *maxCount : UNLIMITED_COUNT);
	bool ret = false;
	Area::PathIterator it = area->pathBegin();
	Area::PathIterator end = area->pathEnd();

	IdentifiersType newKey;
	uint64_t validCount = freeCount;

	bool checkCellDataRightCube = User::checkCellDataRightCube(database, cube); // true if groupCellDataCube contains values or rules
	User::RightSetting rs(false);
	bool getCellRight = jobRequest->properties && User::checkUser(user);
	bool isReadableArea = (getCellRight || checkCellDataRightCube) ? false : isReadable(calcArea, rs, 0);
	rs.checkCells = checkCellDataRightCube;

	double areaSize = calcArea->getSize();
	if (areaSize) {
		while ((ret = cs->next())) {
			newKey = cs->getKey();

			generateMissingValues(it, end, &newKey, props, emptyValues, getCellRight, vRights, isReadableArea, rs, freeCount);
			generateValue(newKey, cs->getValue(), props, getCellRight, vRights, isReadableArea, rs, freeCount);

			if (areaSize == 1.0) {
				++it;
				ret = false;
				break;
			}
			if (!freeCount || !validCount) {
				break;
			}
			validCount--;
			++it;
		}
	}
	generateMissingValues(it, end, 0, props, emptyValues, getCellRight, vRights, isReadableArea, rs, freeCount);

	if (lastKey) {
		bool copyIter;
		IdentifiersType iterKey = *it;
		if (!newKey.empty()) {
			copyIter = CellValueStream::compare(newKey, iterKey) < 0;
		} else {
			copyIter = true;
		}
		for (size_t i = 0; i < area->dimCount(); i++) {
			(*lastKey)[i] = copyIter ? iterKey[i] : newKey[i];
		}
	}
	if (maxCount) {
		*maxCount = freeCount;
	}
	return ret;
}

void AreaJob::generateMissingValues(Area::PathIterator &curr, const Area::PathIterator &end, const IdentifiersType *newKey, PCellStream props, bool emptyValues, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount)
{
	if (freeCount && (isNoPermission || emptyValues)) {
		if (newKey) {
			while (freeCount && curr != *newKey) {
				generateMissingValues_intern(curr, props, emptyValues, getCellRight, vRights, isReadableArea, rs, freeCount);
			}
		} else {
			while (freeCount && curr != end) {
				generateMissingValues_intern(curr, props, emptyValues, getCellRight, vRights, isReadableArea, rs, freeCount);
			}
		}
	}
}

void AreaJob::generateMissingValues_intern(Area::PathIterator &curr, PCellStream props, bool emptyValues, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount)
{
	bool added = false;
	if (isNoPermission) {
		added = generateError(curr, freeCount, noPermission, database->getHideElements() ? ErrorException::ERROR_ELEMENT_NOT_FOUND : ErrorException::ERROR_NOT_AUTHORIZED);
	}
	if (emptyValues && !added) {
		generateValue(*curr, CellValue::NullNumeric, props, getCellRight, vRights, isReadableArea, rs, freeCount);
	}
	++curr;
}

void AreaJob::generateValue(const IdentifiersType &key, const CellValue value, PCellStream props, bool getCellRight, vector<User::RoleDbCubeRight> &vRights, bool isReadableArea, User::RightSetting rs, uint64_t &freeCount)
{
	if (freeCount) {
		bool defaultUsed = false;
		bool isReadableCell = true;
		RightsType r = RIGHT_DELETE;

		if (getCellRight) {
			r = user->getCellRight(database, cube, key, vRights, &defaultUsed);
			isReadableCell = r >= RIGHT_READ;
		} else {
			if (isReadableArea) {
				isReadableCell = true;
			} else {
				PCubeArea cp(new CubeArea(database, cube, key));
				isReadableCell = isReadable(cp, rs, &defaultUsed);
			}
		}

		bool generated = false;
		vector<CellValue> prop_vals;
		if (isReadableCell) {
			if (checkCondition(value)) {
				if (jobRequest->properties) {
					fillProps(prop_vals, key, props, *jobRequest->properties, r);
				}
				appendValue(key, value, prop_vals);
				generated = true;
			}
		} else {
			if (jobRequest->properties) {
				prop_vals.resize(jobRequest->properties->size());
			}
			appendValue(key, CellValue(database->getHideElements() && !defaultUsed ? ErrorException::ERROR_ELEMENT_NOT_FOUND : ErrorException::ERROR_NOT_AUTHORIZED), prop_vals);
			generated = true;
		}

		if (generated && freeCount != UNLIMITED_COUNT) {
			freeCount--;
		}
	}
}

bool AreaJob::generateError(Area::PathIterator &curr, uint64_t &freeCount, PArea &restriction, ErrorException::ErrorType error)
{
	bool generated = false;
	if (freeCount) {
		for (size_t i = 0; i < restriction->dimCount(); i++) {
			if (restriction->find(i, (*curr)[i]) != restriction->elemEnd(i)) {
				vector<CellValue> prop_vals;
				if (jobRequest->properties) {
					prop_vals.resize(jobRequest->properties->size());
				}
				appendValue(*curr, CellValue(error), prop_vals);
				generated = true;
				if (freeCount != UNLIMITED_COUNT) {
					freeCount--;
				}
				break;
			}
		}
	}
	return generated;
}

PCellStream AreaJob::getCellPropsStream(CPDatabase db, CPCube cube, CPCubeArea area, const IdentifiersType &properties)
{
	CPAttributesCube propCube = CONST_COMMITABLE_CAST(AttributesCube, db->findCubeByName(SystemCube::PREFIX_CELL_PROPS_DATA + cube->getName(), PUser(), true, false));
	PArea a(new Area(area->dimCount() + 1));
	size_t i = 0;
	for (; i <  area->dimCount(); i++) {
		a->insert(i, area->getDim(i));
	}
	PSet s(new Set);
	for (IdentifiersType::const_iterator it = properties.begin(); it != properties.end(); ++it) {
		if (*it) {
			s->insert(*it);
		}
	}
	a->insert(i, s);
	PCubeArea propArea(new CubeArea(db, propCube, *a));

	for (i = 0; i < properties.size(); i++) {
		map<IdentifierType, vector<size_t> >::iterator it = propPositions.find(properties[i]);
		if (it == propPositions.end()) {
			vector<size_t> positions;
			positions.push_back(i);
			propPositions.insert(make_pair(properties[i], positions));
		} else {
			it->second.push_back(i);
		}
	}
	firstProp = true;
	PCellStream ret;

	if (propArea->getSize()) {
		ret = propCube->calculatePropertiesArea(db, cube, propArea);
	}
	propsFinish = !ret;
	return ret;
}

void AreaJob::fillProps(vector<CellValue> &result, const IdentifiersType &key, PCellStream &propStream, const IdentifiersType &properties, RightsType right)
{
	result.resize(properties.size());
	map<IdentifierType, vector<size_t> >::iterator it = propPositions.find(0);
	if (it != propPositions.end()) {
		for (vector<size_t>::const_iterator iti = it->second.begin(); iti != it->second.end(); ++iti) {
			result[*iti] = User::rightsTypeToString(right);
		}
	}
	if (!propsFinish) {
		if (firstProp) {
			propsFinish = !propStream->next();
			firstProp = false;
		}
		const IdentifiersType *propKey = &propStream->getKey();
		while (!propsFinish && keyCompareProp(key, *propKey)) {
			insertProperties(result, propStream, *propKey);
			if (!(propsFinish = !propStream->next())) {
				propKey = &propStream->getKey();
			}
		}
	}
}

void AreaJob::insertProperties(vector<CellValue> &result, PCellStream &propStream, const IdentifiersType &key)
{
	IdentifierType ind = key[key.size() - 1];
	map<IdentifierType, vector<size_t> >::iterator it = propPositions.find(ind);
	const CellValue &val = propStream->getValue();
	for (vector<size_t>::const_iterator iti = it->second.begin(); iti != it->second.end(); ++iti) {
		result[*iti] = val;
	}
}

bool AreaJob::keyCompareProp(const IdentifiersType &key, const IdentifiersType &keyProp)
{
	for (size_t i = 0; i < key.size(); i++) {
		if (key[i] != keyProp[i]) {
			return false;
		}
	}
	return true;
}

bool AreaJob::isReadable(PCubeArea area, User::RightSetting& rs, bool *defaultUsed) const
{
	bool isReadable = true;
	try {
		cube->checkAreaAccessRight(database, user, area, rs, false, RIGHT_READ, defaultUsed);
	} catch (const ErrorException& e) {
		if (e.getErrorType() == ErrorException::ERROR_NOT_AUTHORIZED) {
			isReadable = false;
		} else {
			throw;
		}
	}
	return isReadable;
}

}
