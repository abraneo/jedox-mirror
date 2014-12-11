/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

bool AreaJob::checkElement(CPDimension &dim, Element *e, vector<User::RoleCubeRight>& rcRights, bool checkPermissions)
{
	bool result = true;
	if (checkPermissions) {
		RightsType elementPermission = cube->getElementAccessRight(database, user, dim, e->getIdentifier(), rcRights);
		if (elementPermission < RIGHT_READ) {
			result = false;
		}
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

double AreaJob::fillEmptyDim(vector<User::RoleCubeRight>& rcRights, bool checkPermissions)
{
	const IdentifiersType *dimensions = cube->getDimensions();
	for (size_t i = 0; i < jobRequest->area->size(); i++) {
		if (!jobRequest->area->at(i).size()) {
			CPDimension dim = database->lookupDimension(dimensions->at(i), false);
			ElementsType elements = dim->getElements(PUser(), false);

			if (dim->getDimensionType() == Dimension::VIRTUAL && elements.empty()) {
				jobRequest->area->at(i).push_back(ALL_IDENTIFIERS);
			} else {
				for (ElementsType::iterator j = elements.begin(); j != elements.end(); ++j) {
					if (!database->getHideElements() || checkElement(dim, *j, rcRights, checkPermissions)) {
						jobRequest->area->at(i).push_back((*j)->getIdentifier());
					}
				}
			}
		}
	}
	double size = 1.0;
	for (size_t i = 0; i < jobRequest->area->size(); i++) {
		if (jobRequest->area->at(i).size() == 1 && jobRequest->area->at(i).at(0) == ALL_IDENTIFIERS) {
			size *= double(IdentifierType(-1));
		} else {
			size *= jobRequest->area->at(i).size();
		}
	}
	return size;
}

PCubeArea AreaJob::checkRights(vector<User::RoleCubeRight>& rcRights, bool checkPermissions, CPArea area, bool* hasStringElem)
{
	const IdentifiersType *dimensions = cube->getDimensions();
	size_t dimCount = dimensions->size();
	noPermission.reset(new Area(dimCount));
	unknown.reset(new Area(dimCount));
	isNoPermission = false;
	isUnknown = false;

	if (hasStringElem) {
		*hasStringElem = false;
	}
	PCubeArea calcArea(new CubeArea(database, cube, dimCount));
	for (size_t i = 0; i < dimCount; i++) {
		PSet s(new Set(*area->getDim(i).get()));

		CPDimension dim = database->lookupDimension(dimensions->at(i), false);
		dims.push_back(dim);

		PSet np(new Set);
		PSet unk(new Set);

		if (calcArea->elemCount(i)) {
			bool changed = false;
			for (Set::Iterator it = s->begin(); it != s->end();) {
				Element *elem = dim->lookupElement(*it, false);
				if (elem) {
					if (!checkElement(dim, elem, rcRights, checkPermissions)) {
						if (database->getHideElements()) {
							unk->insert(*it);
							isUnknown = true;
						} else {
							np->insert(*it);
							isNoPermission = true;
						}
						it = s->erase(it);
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
					unk->insert(*it);
					isUnknown = true;
					it = s->erase(it);
					changed = true;
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

		noPermission->insert((IdentifierType)i, np);
		unknown->insert((IdentifierType)i, unk);
		calcArea->insert(i, s);
	}
	return calcArea;
}

bool AreaJob::loop(CPArea area, PCubeArea calcArea, PCellStream cs, uint64_t *maxCount, bool generate, PCellStream props, bool skipEmpty, vector<User::RoleCubeRight>& rcRights, IdentifiersType* lastKey)
{
	uint64_t freeCount = (maxCount ? *maxCount : UNLIMITED_COUNT);
	bool ret = false;
	Area::PathIterator it = area->pathBegin();
	Area::PathIterator end = area->pathEnd();

	IdentifiersType newKey;
	double areaSize = calcArea->getSize();
	if (areaSize) {
		User::RightSetting rs(User::checkCellDataRightCube(database, cube));

		bool getCellRight = jobRequest->properties && User::checkUser(user);
		bool isReadableArea = getCellRight ? false : isReadable(calcArea, rs);

		while ((ret = cs->next())) {
			newKey = cs->getKey();
			if (generate) {
				if (isUnknown) {
					generateErrors(it, end, &newKey, freeCount, unknown, ErrorException::ERROR_ELEMENT_NOT_FOUND);
				}
				if (isNoPermission) {
					generateErrors(it, end, &newKey, freeCount, noPermission, ErrorException::ERROR_NOT_AUTHORIZED);
				}
			}
			if (!freeCount) {
				break;
			}
			freeCount--;

			RightsType r = RIGHT_DELETE;
			bool isReadableCell;

			if (getCellRight) {
				r = user->getCellRight(database, cube, newKey, rcRights);
				isReadableCell = r >= RIGHT_READ;
			} else {
				if (isReadableArea) {
					isReadableCell = true;
				} else {
					PCubeArea cp(new CubeArea(database, cube, newKey));
					isReadableCell = isReadable(cp, rs);
				}
			}
			if (isReadableCell) {
				vector<CellValue> prop_vals;
				if (jobRequest->properties) {
					fillProps(prop_vals, newKey, props, *jobRequest->properties, r);
				}
				const CellValue &cellValue = cs->getValue();
				if (checkCondition(cellValue)) {
					appendValue(newKey, cellValue, prop_vals);
				} else {
					freeCount++;
				}
			} else {
				vector<CellValue> prop_vals;
				if (jobRequest->properties) {
					prop_vals.resize(jobRequest->properties->size());
				}
				appendValue(newKey, CellValue(ErrorException::ERROR_NOT_AUTHORIZED), prop_vals);
			}
			++it;
			if (areaSize == 1.0) {
				ret = false;
				break;
			}
		}
	}
	if (generate) {
		if (isUnknown) {
			generateErrors(it, end, 0, freeCount, unknown, ErrorException::ERROR_ELEMENT_NOT_FOUND);
		}
		if (isNoPermission) {
			generateErrors(it, end, 0, freeCount, noPermission, ErrorException::ERROR_NOT_AUTHORIZED);
		}
	}
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

void AreaJob::generateErrors(Area::PathIterator &curr, const Area::PathIterator &end, const IdentifiersType *newKey, uint64_t &freeCount, PArea &restriction, ErrorException::ErrorType error)
{
	if (newKey) {
		while (freeCount && curr != *newKey) {
			generateError(curr, freeCount, restriction, error);
		}
	} else {
		while (freeCount && curr != end) {
			generateError(curr, freeCount, restriction, error);
		}
	}
}

void AreaJob::generateError(Area::PathIterator &curr, uint64_t &freeCount, PArea &restriction, ErrorException::ErrorType error)
{
	if (!freeCount) {
		return;
	}
	for (size_t i = 0; i < restriction->dimCount(); i++) {
		if (restriction->find(i, (*curr)[i]) != restriction->elemEnd(i)) {
			vector<CellValue> prop_vals;
			if (jobRequest->properties) {
				prop_vals.resize(jobRequest->properties->size());
			}
			appendValue(*curr, CellValue(error), prop_vals);
			if (freeCount != UNLIMITED_COUNT) {
				freeCount--;
			}
			break;
		}
	}
	++curr;
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

bool AreaJob::isReadable(PCubeArea area, User::RightSetting& rs) const
{
	bool isReadable = true;
	try {
		cube->checkAreaAccessRight(database, user, area, rs, false, RIGHT_READ);
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
