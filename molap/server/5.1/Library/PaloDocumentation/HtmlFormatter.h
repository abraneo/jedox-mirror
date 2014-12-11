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

#ifndef PALO_DOCUMENTATION_SERVER_HTML_FORMATTER_H
#define PALO_DOCUMENTATION_SERVER_HTML_FORMATTER_H 1

#include "palo.h"

#include "Collections/StringBuffer.h"

#include "PaloDocumentation/Documentation.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief formats a documentation object using a html template
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS HtmlFormatter {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new formatter from a template
	///
	/// Objects of this class generate a html page from a template file and a
	/// documentation object. A template file consists of html code and placeholders.
	/// A HtmlFormatter object replaces this placeholders by data from a documentation
	/// object.
	////////////////////////////////////////////////////////////////////////////////

	HtmlFormatter(const string& templateName);

	~HtmlFormatter() {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief formats documentation
	///
	/// Replaces the placeholders of the template file by data from a documentation
	/// object and returns the generated html documentation.
	////////////////////////////////////////////////////////////////////////////////

	const StringBuffer& getDocumentation(Documentation * documentation);

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates the documetation
	///
	/// The methods reads the template line by line and uses method "processTemplateLine"
	/// to replace the placeholders by data of the documentation object.
	////////////////////////////////////////////////////////////////////////////////

	void generateDocumentation(Documentation*);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief formats one line
	////////////////////////////////////////////////////////////////////////////////

	void processTemplateLine(Documentation *, const string& line);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief formats the lines of a loop
	////////////////////////////////////////////////////////////////////////////////

	void processTemplateLine(Documentation *, const string& line, size_t maxLoops);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief replaces the placeholders by data from the documentation object
	////////////////////////////////////////////////////////////////////////////////

	void processTemplateLine(Documentation *, const string& line, bool withLoop, size_t maxLoops);

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief list of template lines
	////////////////////////////////////////////////////////////////////////////////

	vector<string> templateBuffer;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief buffer for the html result
	////////////////////////////////////////////////////////////////////////////////

	StringBuffer htmlDocumentation;
};

}

#endif
