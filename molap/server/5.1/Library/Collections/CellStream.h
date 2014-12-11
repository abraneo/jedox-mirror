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
 * 
 *
 */

#ifndef CELLSTREAM_H
#define CELLSTREAM_H

#include "palo.h"
#include "Engine/Area.h"
#include "Exceptions/ErrorException.h"

using namespace palo;

namespace palo {

template <typename ValueType>
class CellStream {
public:
	virtual ~CellStream() {}
	virtual bool next() = 0;
	virtual void setValue(const ValueType &value) {throw ErrorException(ErrorException::ERROR_INTERNAL, "CellStream::setValue not supported!");}
	virtual const ValueType &getValue() = 0;
	virtual const IdentifiersType &getKey() const = 0;
	virtual const GpuBinPath &getBinKey() const {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "getBinKey not supported!");
	}
	virtual void reset() = 0;
	virtual bool move(const Area::PathIterator &path) = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns -1 if key1 < key2, 0 if key1 == key2, +1 if key1 > key2
	////////////////////////////////////////////////////////////////////////////////
	static int compare(const IdentifiersType &key1, const IdentifiersType &key2) {
		size_t size = key1.size();
		if (size != key2.size()) {
			throw ErrorException(ErrorException::ERROR_INTERNAL, "different key dimensionality");
		}
		for (size_t i = 0; i < size; i++) {
			if (key1[i] < key2[i]) {
				return -1;
			} else if (key1[i] > key2[i]) {
				return 1;
			}
		}
		return 0;
	}
};
}
#endif
