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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "HttpServer/HttpRequest.h"

#include <iostream>

#include "Collections/StringBuffer.h"

#include "Logger/Logger.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

HttpRequest::HttpRequest(const string& path, HttpRequestHandler* httpRequestHandler) :
	httpRequestHandler(httpRequestHandler), type(HTTP_REQUEST_ILLEGAL), requestPath(path), contentLength(0)
{
}

// /////////////////////////////////////////////////////////////////////////////
// extraction
// /////////////////////////////////////////////////////////////////////////////

void HttpRequest::extractHeader(char* begin, char* end)
{
#ifdef ENABLE_TRACE_OPTION
	headerString = string(begin, end);
#endif

	// 1. split header into lines at "\r\n"
	// 2. split lines at " "
	// 3. split GET/POST etc. requests

	//
	// check for '\n' (we check for '\r' later)
	//

	static const char CR = '\r';
	static const char NL = '\n';
	static const char SPC = ' ';

	int lineNum = 0;

	for (char* start = begin; start < end;) {
		char* findnl = start;

		for (; findnl < end && *findnl != NL; ++findnl) {
		}

		if (findnl == end) {
			break;
		}

		char* endnl = findnl;

		//
		// check for '\r'
		//

		if (endnl > start && *(endnl - 1) == CR) {
			endnl--;
		}

		if (endnl > start) {

			//
			// split line at spaces
			//

			char* space = start;

			for (; space < endnl && *space != SPC; ++space) {
			}

			if (space < endnl) {
				if (space > start) {
					char* colon = space;

					if (*(colon - 1) == ':') {
						--colon;
					}

					string key(start, colon);

					// check for request type (GET/POST in line 0),
					// path and parameters
					if (lineNum == 0) {
						if (key == "POST") {
							type = HTTP_REQUEST_POST;
						} else if (key == "GET") {
							type = HTTP_REQUEST_GET;
						}

						if (type != HTTP_REQUEST_ILLEGAL) {
							char* reqe = space + 1;

							// delete "HTTP/1.1" from request
							for (; reqe < endnl && *reqe != SPC; reqe++) {
							}

							string value(space + 1, reqe);
							headerFields[key] = value;

							// split requestPath and parameters
							char* parm = space + 1;

							for (; parm < reqe && *parm != '?'; parm++) {
							}

							if (parm < reqe) {
								setKeyValues(parm + 1, reqe);
							}
						}
					} else {
						string value(space + 1, endnl);
						headerFields[key] = value;

						if (key == "Content-Length") {
							char *p;
							long int result = strtol(value.c_str(), &p, 10);

							if (*p == '\0') {
								contentLength = result;
							}
						}
					}
				}
			} else {
				string key(start, endnl);
				headerFields[key] = "";
			}

			start = end + 1;
		}

		start = findnl + 1;
		lineNum++;
	}
}

void HttpRequest::extractBody(char* begin, char* end)
{
#ifdef ENABLE_TRACE_OPTION
	bodyString = string(begin, end);
#endif

	setKeyValues(begin, end);
}

const string& HttpRequest::getHeader(const string& field) const
{
	static const string error = "";

	map<string, string>::const_iterator i = headerFields.find(field);

	if (i != headerFields.end()) {
		return i->second;
	} else {
		return error;
	}
}

const map<string, string>& HttpRequest::getHeaders() const
{
	return headerFields;
}

const string& HttpRequest::getValue(const string& key) const
{
	static const string error = "";

	map<string, string>::const_iterator i = requestFields.find(key);

	if (i != requestFields.end()) {
		return i->second;
	} else {
		return error;
	}
}

const string& HttpRequest::getValue(const string& key, bool& found) const
{
	static const string error = "";

	map<string, string>::const_iterator i = requestFields.find(key);

	if (i != requestFields.end()) {
		found = true;
		return i->second;
	} else {
		found = false;
		return error;
	}
}

const map<string, string>& HttpRequest::getValues() const
{
	return requestFields;
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void HttpRequest::setKeyValues(char* begin, char* end)
{
	enum {
		KEY, VALUE
	} phase = KEY;
	enum {
		NORMAL, HEX1, HEX2
	} reader = NORMAL;

	int hex = 0;

	const char AMB = '&';
	const char EQUAL = '=';
	const char PERCENT = '%';
	const char PLUS = '+';
	const char SPC = ' ';

	char * buffer = begin;
	char * keyStart = buffer;
	char * keyPtr = keyStart;
	char * valueStart = buffer;
	char * valuePtr = valueStart;

	for (; buffer < end; ++buffer) {
		char next = *buffer;

		if (phase == KEY && next == EQUAL) {
			phase = VALUE;

			valueStart = buffer + 1;
			valuePtr = valueStart;

			continue;
		} else if (next == AMB) {
			phase = KEY;

			string keyStr(keyStart, keyPtr);
			keyStart = buffer + 1;
			keyPtr = keyStart;

			string valueStr(valueStart, valuePtr);
			valueStart = buffer + 1;
			valuePtr = valueStart;

			requestFields[keyStr] = valueStr;

			continue;
		} else if (next == PERCENT) {
			reader = HEX1;
			continue;
		} else if (reader == HEX1) {
			hex = hex2int(next) * 16;
			reader = HEX2;
			continue;
		} else if (reader == HEX2) {
			hex += hex2int(next);
			reader = NORMAL;
			next = (char)hex;
		} else if (next == PLUS) {
			next = SPC;
		}

		if (phase == KEY) {
			*keyPtr++ = next;
		} else {
			*valuePtr++ = next;
		}
	}

	if (keyStart < keyPtr) {
		string keyStr(keyStart, keyPtr);
		string valueStr(valueStart, valuePtr);

		requestFields[keyStr] = valueStr;
	}
}
}
