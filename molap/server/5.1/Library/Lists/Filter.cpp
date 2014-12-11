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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Filter.h"
#include "SubSet.h"

namespace palo {

Filter::Filter(SubSet& subset, unsigned long flags, int max) :
	m_subset_ref(subset), filter_flags(flags), max_flag(max)
{
	if (!checkFlags(flags)) {
		throw ErrorException(ErrorException::ERROR_INVALID_TYPE, "Invalid Filter Flags passed to this filter.");
	}
}

bool Filter::queryFlag(const unsigned long int & flag) const
{
	return (filter_flags & flag) != 0;
}

bool Filter::checkFlags(const unsigned long int& flags)
{
	if ((flags >> max_flag) != 0) {
		return false;
	}
	return true;
}

void Filter::clearFlags()
{
	this->filter_flags = 0;
}

void Filter::setFlag(unsigned long int f)
{
	this->filter_flags |= f;
}

void Filter::resetFlag(unsigned long int f)
{
	this->filter_flags &= ~f;
}

} //palo
