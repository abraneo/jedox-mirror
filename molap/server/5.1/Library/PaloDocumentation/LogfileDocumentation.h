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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef PALO_DOCUMENTATION_LOGFILE_BROWSER_DOCUMENTATION_H
#define PALO_DOCUMENTATION_LOGFILE_BROWSER_DOCUMENTATION_H 1

#include "palo.h"

#include "PaloDocumentation/BrowserDocumentation.h"
#include "Olap/SystemDatabase.h"
#include "Engine/Streams.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief provides server data documentation
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS LogFileBrowserDocumentation : public BrowserDocumentation {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a new server data based documentation
	////////////////////////////////////////////////////////////////////////////////

	LogFileBrowserDocumentation(PCellStream logData, const string& message)
	: BrowserDocumentation(message) {
		vector<string> logfile_date(1,"2011-11-22");
		vector<string> logfile_time(1,"15:59:43");
		vector<string> logfile_level(1,"INFO");
		vector<string> logfile_message(1,"created CubeDimension");
		vector<vector<string>*> columns;
		columns.push_back(&logfile_date);
		columns.push_back(&logfile_time);
		columns.push_back(&logfile_level);
		columns.push_back(&logfile_message);

		IdentifierType lastLine = NO_IDENTIFIER;
		while (logData && logData->next()) {
			const IdentifiersType &key= logData->getKey();
			if (lastLine != key[0]) {
				lastLine = key[0];
				for (vector<vector<string>*>::iterator cit = columns.begin(); cit != columns.end(); ++cit) {
					(*cit)->push_back("");
				}
//				logfile_date.push_back("");
//				logfile_time.push_back("");
//				logfile_level.push_back("");
//				logfile_message.push_back("");
			}
			columns[key[1]]->back() = logData->getValue();
			// ordered like in const string SystemDatabase::MESSAGE_ITEMS
			// "date", "time", "level", "message"
			//logfile_date[key-startLine] =
		}
		values["@logfile_date"] = logfile_date;
		values["@logfile_time"] = logfile_time;
		values["@logfile_level"] = logfile_level;
		values["@logfile_message"] = logfile_message;
	}
};

}

#endif
