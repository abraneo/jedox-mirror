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

#ifndef INPUT_OUTPUT_CONDITION_H
#define INPUT_OUTPUT_CONDITION_H 1

#include "palo.h"
#include "../Olap/Cube.h"

#include <functional>

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief condition on double
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Condition {
public:
	template<class _Tp>
	struct logical_xor : public binary_function<_Tp, _Tp, bool> {
		bool operator()(const _Tp& x, const _Tp& y) const {
			return (x || y) && !(x && y);
		}
	};

public:
	static Condition * parseCondition(const string& desc);

public:
	virtual ~Condition() {
	}

public:
	virtual bool check(const CellValue &value) const {
		bool ret = false;

		if (!value.isError()) {
			if (value.isNumeric()) {
				ret = check(value.getNumeric());
			} else {
				ret = check((const string &)value);
			}
		}

		return ret;
	}
	virtual bool check(double value) const = 0;
	virtual bool check(const string& value) const = 0;
};

}

#endif
