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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Engine/Streams.h"
#include "Olap/Cube.h"
#include "Olap/Database.h"

namespace palo {

int IdentifiersType::compare(const IdentifiersType &o) const {
	size_t size = vector<IdentifierType>::size();
	size_t sizeo = o.size();
	if (!size && sizeo) {
		return -1; // empty < any
	} else if (size && !sizeo) {
		return 1; // any > empty
	} else if (size != sizeo) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "different size of IdentifiersType");
	}

	for (int i = 0; i < (int)size; i++) {
		if (at(i) < o.at(i)) {
			return -(i+1);
		} else if (at(i) > o.at(i)) {
			return i+1;
		}
	}
	return 0;
}

bool CellValueStream::move(const IdentifiersType &key, bool *found)
{
	bool result = true;
	if (found) {
		*found = false;
	}
	do {
		int compare = key.compare(getKey());
		if (!compare) {
			// found - at the searched key
			if (found) {
				*found = true;
			}
			break;
		} else if (compare < 0) {
			// not found - behind the searched key
			break;
		}
		result = next();
	} while (result);
	return result;
}

const IdentifiersType CellValueStream::EMPTY_KEY;

}
