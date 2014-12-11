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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Engine/CubeFileStream.h"
#include "InputOutput/FileReader.h"
#include "Exceptions/FileFormatException.h"
#include "Olap/Cube.h"
#include "Olap/Database.h"

namespace palo {

CubeFileStream::CubeFileStream(PDatabase db, PCube cube, FileReader *file, bool isString, vector<map<IdentifierType, IdentifierType> *> *aliasMaps) :
	file(file), numeric(!isString), cube(cube), first(true), aliasMaps(aliasMaps), anyReMap(false)
{
	const IdentifiersType *d = cube->getDimensions();
	for (IdentifiersType::const_iterator it = d->begin(); it != d->end(); ++it) {
		dims.push_back(db->lookupDimension(*it, false).get());
	}
	file->nextLine();

	const IdentifiersType *dimensions = cube->getDimensions();
	for (IdentifiersType::const_iterator dimId = dimensions->begin(); dimId != dimensions->end(); ++dimId) {
		ElementOld2NewMap *dimMap = db->getDimensionMap(*dimId);
		dimReMap.push_back(dimMap);
		if (dimMap) {
			anyReMap = true;
		}
	}
}

bool CubeFileStream::next()
{
	bool failed;
	do {
		if (file->isDataLine()) {
			path = file->getDataIdentifiers(0);
			failed = false;

			if (anyReMap) {
				for (size_t i = 0; i < dims.size(); i++) {
					if (dimReMap[i]) {
						IdentifierType newId = dimReMap[i]->translate(path[i]);
						if (newId != NO_IDENTIFIER) {
							path[i] = newId;
						} else {
							// error
							string msg = (string)"error in " + (numeric ? "numeric" : "string") + " cell path";
							Logger::debug << msg << " of cube '" << cube->getName() << "', skipping entry " << path[i] << " in dimension '" << dims[i]->getName() << endl;
							failed = true;
							break;
						}
					}
				}
			}

			if (!failed) {
				if (first) {
					prev = path;
				}
				if (numeric) {
					value = file->getDataDouble(1);
				} else {
					value = file->getDataString(1);
				}

				if (dims.size() != path.size()) {
					string msg = (string)"error in " + (numeric ? "numeric" : "string") + " cell path";
					Logger::error << msg << " of cube '" << cube->getName() << "'" << endl;
					throw FileFormatException(msg, file);
				}
			}

			if (!failed) {
				for (size_t i = 0; i < dims.size(); i++) {
					if (aliasMaps && aliasMaps->at(i) != NULL) {
						map<IdentifierType, IdentifierType>::iterator it = aliasMaps->at(i)->find(path[i]);
						if (it == aliasMaps->at(i)->end()) {
							failed = true;
							break;
						} else {
							prev[i] = path[i];
							path[i] = it->second;
						}
					} else if (first || prev[i] != path[i]) {
						const Dimension *dim = dims[i];
						Element *elem = dim->lookupElement(path[i], false);
						if (!elem || (numeric && elem->getElementType() == Element::CONSOLIDATED)) {
							if (!elem) {
								string msg = (string)"error in " + (numeric ? "numeric" : "string") + " cell path";
								Logger::debug << msg << " of cube '" << cube->getName() << "', skipping entry " << path[i] << " in dimension '" << dim->getName() << endl;
							} else {
								Logger::debug << "consolidation in numeric cell path of cube '" << cube->getName() << "', skipping entry " << path[i] << " in dimension '" << dim->getName() << endl;
							}

							failed = true;
							break;
						}
						prev[i] = path[i];
					}
				}
			}
			if (!failed) {
				first = false;
			}
			file->nextLine();
		} else {
			return false;
		}
	} while (failed);
	return true;
}

const CellValue &CubeFileStream::getValue()
{
	return value;
}

double CubeFileStream::getDouble()
{
	return value.getNumeric();
}

const IdentifiersType &CubeFileStream::getKey() const
{
	return path;
}

void CubeFileStream::reset()
{
	throw ErrorException(ErrorException::ERROR_INTERNAL, "No reset for file stream");
}

}
