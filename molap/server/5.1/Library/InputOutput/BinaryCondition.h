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

#ifndef INPUT_OUTPUT_BINARY_CONDITION_H
#define INPUT_OUTPUT_BINARY_CONDITION_H 1

#include "palo.h"

#include "Exceptions/ParameterException.h"

#include "InputOutput/Condition.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief binary condition on double
////////////////////////////////////////////////////////////////////////////////

template<typename OP>
class SERVER_CLASS BinaryCondition : public Condition {
public:
#ifdef ENABLE_GPU_SERVER
	friend class EngineGpu;
	friend class GpuDFilterQuantificationProcessor;
#endif
	BinaryCondition(Condition * left, Condition * right) :
		left(left), right(right) {

		if (left == 0 || right == 0) {
			throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "expecting a left and right value for a binary condition", "condition", "binary");
		}
	}

	~BinaryCondition() {
		delete left;
		delete right;
	}

public:
	bool check(double value) const {
		return op(left->check(value), right->check(value));
	}

	bool check(const string& value) const {
		return op(left->check(value), right->check(value));
	}

private:
	OP op;

	Condition * left;
	Condition * right;
};

}

#endif
