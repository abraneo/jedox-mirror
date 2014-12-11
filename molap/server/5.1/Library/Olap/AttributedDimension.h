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

#ifndef OLAP_ATTRIBUTED_DIMENSION_H
#define OLAP_ATTRIBUTED_DIMENSION_H 1

#include "palo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief class for dimensions with attribute cube
///
/// class for dimensions which have attribute dimension and attribute cube
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS AttributedDimension {
public:
	AttributedDimension() {};

	static void addDimension(PServer server, PDatabase database, const Dimension *dimension, bool lookup, IdentifierType *attributesDimId, IdentifierType *attributesCubeId, bool useDimWorker);
	static void removeDimension(PServer server, PDatabase database, const string &name, bool useDimWorker);
	static void renameDimension(PServer server, PDatabase database, const string &newName, const string &oldName, bool useDimWorker);

	static CPAttributesDimension getAttributesDimension(CPDatabase database, const string &name);
	static CPSystemCube getAttributesCube(CPDatabase database, const string &name);
};

////////////////////////////////////////////////////////////////////////////////
/// @brief class for dimensions with rights cube
///
/// class for dimensions which have dimension data rights cube
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DRCubeDimension {
public:
	DRCubeDimension() {};

	static void addCube(PServer server, PDatabase database, const Dimension *dimension, bool lookup, IdentifierType *rightsCubeId, bool useDimWorker);
	static void removeCube(PServer server, PDatabase database, const string &dimName, bool useDimWorker);
	static void renameCube(PServer server, PDatabase database, const string &newDimName, const string &oldDimName, bool useDimWorker);

	static CPRightsCube getRightsCube(CPDatabase database, const string &dimName);
};

}

#endif
