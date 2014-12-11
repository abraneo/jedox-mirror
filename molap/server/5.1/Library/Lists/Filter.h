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

#ifndef __FILTER_H_INCL__
#define __FILTER_H_INCL__

#include <vector>
#include <map>
#include <string>
#include <set>

#include "Engine/EngineBase.h"

namespace palo {

class SubSet;

class Filter {
public:
	Filter(SubSet& subset, unsigned long int flags, int max);
	bool queryFlag(const unsigned long int & flag) const;
	bool checkFlags(const unsigned long int& flags);
	void clearFlags();

	unsigned int getType();
	void setFlag(unsigned long int);
	void resetFlag(unsigned long int);
	SubSet &m_subset_ref;
protected:
	unsigned long int filter_flags;
private:
	int max_flag;
};

} //palo
#endif
