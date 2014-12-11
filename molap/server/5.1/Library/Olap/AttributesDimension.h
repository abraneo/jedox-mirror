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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_ATTRIBUTES_DIMENSION_H
#define OLAP_ATTRIBUTES_DIMENSION_H 1

#include "palo.h"

#include "Olap/SystemDimension.h"
#include "Olap/AttributedDimension.h"

namespace palo {

class NormalDimension;

////////////////////////////////////////////////////////////////////////////////
/// @brief alias OLAP dimension
///
/// An OLAP dimension is an ordered list of elements
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS AttributesDimension : public SystemDimension, public DRCubeDimension {
public:
	static const string PREFIX_ATTRIBUTE_DIMENSION;
	static const string SUFFIX_ATTRIBUTE_DIMENSION;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates new dimension with given identifier
	////////////////////////////////////////////////////////////////////////////////

	AttributesDimension(const string& name) :
		SystemDimension(name, Dimension::ATTRIBUTES) {
	}

	//inherited from Dimension
	virtual void notifyAddDimension(PServer server, PDatabase database, IdentifierType *attrDimId, IdentifierType *attrCubeId, IdentifierType *rightsCubeId, IdentifierType *dimDimElemId, bool useDimWorker);
	virtual void beforeRemoveDimension(PServer server, PDatabase database, bool useDimWorker);
	virtual void notifyRenameDimension(PServer server, PDatabase database, const string& oldName, bool useDimWorker);

	CPNormalDimension getNormalDimension() const;

	virtual PCommitable copy() const;
};

}

#endif
