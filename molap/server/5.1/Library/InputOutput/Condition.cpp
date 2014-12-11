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

#include "InputOutput/Condition.h"

#include <functional>
#include <iostream>

#include "InputOutput/BinaryCondition.h"
#include "InputOutput/ConstantCondition.h"
#include "InputOutput/UnaryCondition.h"

namespace palo {
	enum COMPARE {
		OP_LT, OP_LE, OP_EQ, OP_GT, OP_GE, OP_NEQ
	};

static Condition * descendCompare(COMPARE op, const char * & ptr, const char * end)
{
	while (ptr < end && (*ptr == ' ' || *ptr == '\t')) {
		ptr++;
	}

	if (*ptr == '"') {
		string value;

		ptr++;

		while (ptr < end) {
			if (*ptr == '"') {
				ptr++;

				if (*ptr == '"') {
					value.push_back('"');
					ptr++;
				} else {
					break;
				}
			} else {
				value.push_back(*ptr++);
			}
		}

		switch (op) {
		case OP_LT:
			return new UnaryCondition<less<double> , less<string> > (value);
		case OP_LE:
			return new UnaryCondition<less_equal<double> , less_equal<string> > (value);
		case OP_EQ:
			return new UnaryCondition<equal_to<double> , equal_to<string> > (value);
		case OP_GT:
			return new UnaryCondition<greater<double> , greater<string> > (value);
		case OP_GE:
			return new UnaryCondition<greater_equal<double> , greater_equal<string> > (value);
		case OP_NEQ:
			return new UnaryCondition<not_equal_to<double> , not_equal_to<string> > (value);
		}
	} else {
		double d = strtod(ptr, (char**)&ptr);

		switch (op) {
		case OP_LT:
			return new UnaryCondition<less<double> , less<string> > (d);
		case OP_LE:
			return new UnaryCondition<less_equal<double> , less_equal<string> > (d);
		case OP_EQ:
			return new UnaryCondition<equal_to<double> , equal_to<string> > (d);
		case OP_GT:
			return new UnaryCondition<greater<double> , greater<string> > (d);
		case OP_GE:
			return new UnaryCondition<greater_equal<double> , greater_equal<string> > (d);
		case OP_NEQ:
			return new UnaryCondition<not_equal_to<double> , not_equal_to<string> > (d);
		}
	}

	return new ConstantCondition<false> ();
}

static Condition * descend(Condition * left, const char * & ptr, const char * end)
{
	while (ptr < end && (*ptr == ' ' || *ptr == '\t')) {
		ptr++;
	}

	if (ptr == end) {
		return left == 0 ? new ConstantCondition<true> () : left;
	}

	// handle and, or, xor
	Condition * right = 0;

	try {
		if (tolower(ptr[0]) == 'a' && tolower(ptr[1]) == 'n' && tolower(ptr[2]) == 'd') {
			ptr += 3;
			right = descend(0, ptr, end);

			return new BinaryCondition<logical_and<bool> > (left, right);
		} else if (tolower(ptr[0]) == 'x' && tolower(ptr[1]) == 'o' && tolower(ptr[2]) == 'r') {
			ptr += 3;
			right = descend(0, ptr, end);

			return new BinaryCondition<Condition::logical_xor<bool> > (left, right);
		} else if (tolower(ptr[0]) == 'o' && tolower(ptr[1]) == 'r') {
			ptr += 2;
			right = descend(0, ptr, end);

			return new BinaryCondition<logical_or<bool> > (left, right);
		}
	} catch (...) {
		if (right != 0) {
			delete right;
		}

		throw ;
	}

	if (*ptr == ')') {
		return left == 0 ? new ConstantCondition<true> () : left;
	}

	// cannot handle a left value for comparison
	if (left != 0) {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "malformed expression", "condition", ptr);
	}

	// the order is important in order to catch "<=" before "<"
	if (*ptr == '(') {
		ptr += 1;
		left = descend(0, ptr, end);

		if (*ptr != ')') {
			throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "malformed expression", "condition", ptr);
		}

		ptr += 1;
	} else if (ptr[0] == '>' && ptr[1] == '=') {
		ptr += 2;
		left = descendCompare(OP_GE, ptr, end);
	} else if (ptr[0] == '>') {
		ptr += 1;
		left = descendCompare(OP_GT, ptr, end);
	} else if (ptr[0] == '<' && ptr[1] == '=') {
		ptr += 2;
		left = descendCompare(OP_LE, ptr, end);
	} else if (ptr[0] == '<' && ptr[1] == '>') {
		ptr += 2;
		left = descendCompare(OP_NEQ, ptr, end);
	} else if (ptr[0] == '<') {
		ptr += 1;
		left = descendCompare(OP_LT, ptr, end);
	} else if (ptr[0] == '=' && ptr[1] == '=') {
		ptr += 2;
		left = descendCompare(OP_EQ, ptr, end);
	} else if (ptr[0] == '=') {
		ptr += 1;
		left = descendCompare(OP_EQ, ptr, end);
	} else if (ptr[0] == '!' && ptr[1] == '=') {
		ptr += 2;
		left = descendCompare(OP_NEQ, ptr, end);
	} else {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "malformed expression", "condition", ptr);
	}

	// descend
	try {
		return descend(left, ptr, end);
	} catch (...) {
		if (left != 0) {
			delete left;
		}

		throw ;
	}
}

Condition * Condition::parseCondition(const string& desc)
{
	const char * ptr = desc.c_str();
	const char * end = ptr + desc.size();

	return descend(0, ptr, end);
}
}
