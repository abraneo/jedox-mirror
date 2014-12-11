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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef GENERIC_CONTEXT_H
#define GENERIC_CONTEXT_H

#include <PaloSpreadsheetFuncs/Conversion.hpp>

#include "ErrorInfo.h"

namespace Palo {
namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Abstract settings class.
 *
 *  Used to pass different options to SpreadsheetFuncs and SpreadsheetFuncsBase.
 */
class GenericContext {
public:
	virtual ~GenericContext()
	{
	}

	/*! \brief Check if splashing is allowed.
	 *
	 *  What the name suggests.
	 */
	virtual bool splashingAllowed() = 0;

	virtual void setError(const Palo::Types::ErrorInfo& ei)
	{
		this->ei = ei;
	}

	virtual const Palo::Types::ErrorInfo getError() const
	{
		return ei;
	}

	struct Conversions {
		Conversions(const jedox::i18n::Conversion& from, const jedox::i18n::Conversion& to) :
				from(from), to(to)
		{
		}

		void operator=(const Conversions& other)
		{
			from = other.from;
			to = other.to;
		}

		jedox::i18n::Conversion from;
		jedox::i18n::Conversion to;
	};

	virtual Conversions& getConversions() = 0;

protected:
	Palo::Types::ErrorInfo ei;
};
}
}
#endif
