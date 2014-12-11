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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Olap/AliasDimension.h"

#include "InputOutput/FileWriter.h"

#include "Olap/Database.h"
#include "Olap/Context.h"

namespace palo {

uint32_t AliasDimension::getToken() const {
	return alias(false)->getToken();
}

bool AliasDimension::merge(const CPCommitable &o, const PCommitable &p)
{
	if (o && old) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "alias dimension shouldn't change", "dimension", getName());
	} else {
		// new instance
		commitintern();
		return true;
	}

}

PCommitable AliasDimension::copy() const
{
	throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "alias dimension cannot be copied", "dimension", getName());
}

PDimension AliasDimension::alias(bool write) const
{
	if (write) {
		throw ParameterException(ErrorException::ERROR_DIMENSION_UNCHANGABLE, "alias dimension shouldn't change", "dimension", getName());
	}
	PDatabase db = Context::getContext()->getServer()->lookupDatabase(dbId, write);
	return db->lookupDimension(aliasId, write);
};

}
