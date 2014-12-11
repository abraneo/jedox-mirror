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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "PaloDocumentation/CubeBrowserDocumentation.h"

#include <iostream>
#include <sstream>

#include "Olap/Dimension.h"
#include "Olap/Rule.h"

namespace palo {
CubeBrowserDocumentation::CubeBrowserDocumentation(PDatabase database, PCube cube, const string& pathString, const string& message, const vector<string> &identifiers, const vector<string> &elemNames, const vector<string> &type, const vector<string> &rule, const vector<string> &value) :
	BrowserDocumentation(message)
{
	vector<string> ids(1, StringUtils::convertToString(database->getIdentifier()));
	values["@database_identifier"] = ids;

	defineCube(cube);
	const IdentifiersType *dims = cube->getDimensions();
	vector<CPDimension> dimensions;
	for (IdentifiersType::const_iterator it = dims->begin(); it != dims->end(); ++it) {
		dimensions.push_back(database->lookupDimension(*it, false));
	}
	defineDimensions(&dimensions);

	vector<string> cellPathsS(1, pathString);
	values["@cell_path_value"] = cellPathsS;
	values["@cell_path"] = identifiers;
	values["@cell_pathName"] = elemNames;
	values["@cell_type"] = type;
	values["@cell_value"] = value;
	values["@cell_rule"] = rule;
}

}
