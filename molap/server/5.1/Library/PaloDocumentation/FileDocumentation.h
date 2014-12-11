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

#ifndef PALO_DOCUMENTATION_SERVER_FILE_DOCUMENTATION_H
#define PALO_DOCUMENTATION_SERVER_FILE_DOCUMENTATION_H 1

#include "palo.h"

#include "PaloDocumentation/Documentation.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief provides documentation from a file
///
/// The class reads documentation blocks from a file. Each block is started by
/// "@param", "@result", "@example", "@param_type", "@param_description",
/// "@result_type", "@result_description", or "@example_description".
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS FileDocumentation : public Documentation {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new file-based documentation
	////////////////////////////////////////////////////////////////////////////////

	FileDocumentation(const string& filename);

public:
	bool hasDocumentationEntry(const string& name);

	const vector<string>& getDocumentationEntries(const string& name);

	const string& getDocumentationEntry(const string& name, size_t index = 0);

private:
	void processDocumentationEntry(const string& name, const string& value);
	ssize_t addDocumentationEntry(const string& name, const string& value);
	void replaceDocumentationEntry(const string& name, const string& value, size_t position);

private:
	map<string, vector<string> > docEntries;

	ssize_t lastParamId;
	ssize_t lastResultId;
	ssize_t lastExampleId;

	ssize_t lastServerId;
	ssize_t lastDatabaseId;
	ssize_t lastDimensionId;
	ssize_t lastElementId;
	ssize_t lastCubeId;
	ssize_t lastCellId;
};

}

#endif
