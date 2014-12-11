/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "PaloHttpServer/PaloHttpRequest.h"

#include <iostream>

#include "Collections/StringBuffer.h"
#include "Collections/StringUtils.h"
#include "PaloDispatcher/PaloJobRequest.h"
#include "PaloHttpServer/PaloCommands.h"
#include "Olap/PaloSession.h"
#include "Olap/Server.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloHttpRequest::PaloHttpRequest(const string& path, HttpRequestHandler* httpRequestHandler) :
	HttpRequest(path, httpRequestHandler)
{
	paloJobRequest = new PaloJobRequest(path);
	loginRequest = (path == "/server/login");
}

PaloHttpRequest::~PaloHttpRequest()
{
	if (paloJobRequest != 0) {
		delete paloJobRequest;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// extraction
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpRequest::extractHeader(char* begin, char* end)
{
#ifdef ENABLE_TRACE_OPTION
	headerString = string(begin, end);

	if (Logger::isTrace()) {
		Logger::trace << "header " << headerString << endl;
	}
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

					// check for request type (GET/POST in line 0),
					// path and parameters
					if (lineNum == 0) {
						if (start + 4 == colon && strncmp(start, "POST", 4) == 0) {
							type = HTTP_REQUEST_POST;
						} else if (start + 3 == colon && strncmp(start, "GET", 3) == 0) {
							type = HTTP_REQUEST_GET;
						}

						if (type != HTTP_REQUEST_ILLEGAL) {
							char* reqe = space + 1;

							// delete "HTTP/1.1" from request
							for (; reqe < endnl && *reqe != SPC; reqe++) {
							}

							// split requestPath and parameters
							char* parm = space + 1;

							for (; parm < reqe && *parm != '?'; parm++) {
							}

							if (parm < reqe) {
								setKeyValues(parm + 1, reqe);
							}
						}
					} else {
						*colon = '\0';
						*endnl = '\0';
						++space;

						if (loginRequest) { // has to save HTTP headers
							string key = string(start, colon);
							string value = string(space, endnl);
							headerFields[key] = value;
						}

						const struct CommandOption * option = Perfect_Hash::PaloValue(start, (unsigned int)(colon - start));

						if (option != 0) {
							switch (option->code) {
							case PaloRequestHandler::CMD_X_PALO_SERVER:
								fillToken(paloJobRequest->serverToken, space, endnl);
								break;

							case PaloRequestHandler::CMD_X_PALO_DATABASE:
								fillToken(paloJobRequest->databaseToken, space, endnl);
								break;

							case PaloRequestHandler::CMD_X_PALO_DIMENSION:
								fillToken(paloJobRequest->dimensionToken, space, endnl);
								break;

							case PaloRequestHandler::CMD_X_PALO_CUBE:
								fillToken(paloJobRequest->cubeToken, space, endnl);
								break;

							case PaloRequestHandler::CMD_X_PALO_CUBE_CLIENT_CACHE:
								fillToken(paloJobRequest->clientCacheToken, space, endnl);
								break;

							case PaloRequestHandler::CMD_CONTENT_LENGTH: {
								char *p;
								long int result = ::strtol(space, &p, 10);

								if (*p == '\0') {
									contentLength = result;
								}

								break;
							}

							default:
								break;
							}
						}
					}
				}
			} else {
			}

			start = end + 1;
		}

		start = findnl + 1;
		lineNum++;
	}
}

void PaloHttpRequest::extractBody(char* begin, char* end)
{
#ifdef ENABLE_TRACE_OPTION
	bodyString = string(begin, end);

	if (Logger::isTrace()) {
		Logger::trace << "body " << bodyString << endl;
	}
#endif

	setKeyValues(begin, end);
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpRequest::setKeyValues(char* begin, char* end)
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

			*keyPtr = '\0';
			valueStart = buffer + 1;
			valuePtr = valueStart;

			continue;
		} else if (next == AMB) {
			phase = KEY;

			*valuePtr = '\0';
			setKeyValue(keyStart, keyPtr, valueStart, valuePtr);

			keyStart = buffer + 1;
			keyPtr = keyStart;

			valueStart = buffer + 1;
			valuePtr = valueStart;

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
		*keyPtr = '\0';
		*valuePtr = '\0';
		setKeyValue(keyStart, keyPtr, valueStart, valuePtr);
	}
}

void PaloHttpRequest::setKeyValue(char * keyStart, char * keyPtr, char * valueStart, char * valuePtr)
{
	const struct CommandOption * option = Perfect_Hash::PaloValue(keyStart, (unsigned int)(keyPtr - keyStart));

	if (Server::flightRecorderEnabled()) {
		httpParams.seekg(0, ios::end);
		std::streamoff size = httpParams.tellg();
		httpParams.seekg(0, ios::beg);

		if (size == 0) {
			httpParams << "?";
		} else {
			httpParams << "&";
		}

		httpParams.write(keyStart, keyPtr - keyStart);
		httpParams << '=';

		if (option != 0 && option->code == PaloRequestHandler::CMD_SID) {
			httpParams << PaloSession::shortenSid(string(valueStart, valuePtr));
		} else if (valuePtr - valueStart > 1000000) {
			httpParams.write(valueStart, 1000000);
			httpParams << "...(argument total " << valuePtr-valueStart << " bytes)";
		} else {
			httpParams.write(valueStart, valuePtr - valueStart);
		}
	}

	if (option != 0) {
		switch (option->code) {
		case PaloRequestHandler::CMD_SID:
			fillSid(paloJobRequest->sid, valueStart, valuePtr);
			paloJobRequest->hasSession = true;
			break;

		case PaloRequestHandler::CMD_ID_CUBE:
			fillIdentifier(paloJobRequest->cube, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_DATABASE:
			fillIdentifier(paloJobRequest->database, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_DIMENSION:
			fillIdentifier(paloJobRequest->dimension, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_ELEMENT:
			fillIdentifier(paloJobRequest->element, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_LIMIT: {
			IdentifiersType *limitValues = 0;
			fillVectorIdentifier(limitValues, valueStart, valuePtr);
			if (!limitValues) {
				break;
			}
			if (limitValues->size() > 1) {
				paloJobRequest->limitCount = (*limitValues)[1];
			}
			if (limitValues->size()) {
				paloJobRequest->limitStart = (*limitValues)[0];
			}
			delete limitValues;
		}
		break;

		case PaloRequestHandler::CMD_ID_LOCK:
			fillIdentifier(paloJobRequest->lock, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_LOCKED_PATHS:
			fillIdentifier(paloJobRequest->lock, valueStart, valuePtr);
			fillVectorVectorIdentifier(paloJobRequest->lockedPaths, valueStart, valuePtr, ':', ',');
			break;

		case PaloRequestHandler::CMD_ID_RULE:
			fillVectorIdentifier(paloJobRequest->rules, valueStart, valuePtr);
			if (paloJobRequest->rules && paloJobRequest->rules->size()) {
				paloJobRequest->rule = paloJobRequest->rules->at(0);
			}
			break;

		case PaloRequestHandler::CMD_ACTIVATE:
			fillUint(paloJobRequest->activate, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ADD:
			fillBoolean(paloJobRequest->add, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_BASE_ONLY:
			fillBoolean(paloJobRequest->baseOnly, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_COMPLETE:
			fillBoolean(paloJobRequest->complete, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_EVENT_PROCESSOR:
			fillBoolean(paloJobRequest->eventProcess, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_ATTRIBUTE:
			fillBoolean(paloJobRequest->showAttribute, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_INFO:
			fillBoolean(paloJobRequest->showInfo, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_GPUTYPE:
			fillBoolean(paloJobRequest->showGputype, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_LOCK_INFO:
			fillBoolean(paloJobRequest->showLockInfo, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_NORMAL:
			fillBoolean(paloJobRequest->showNormal, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_RULE:
			fillBoolean(paloJobRequest->showRule, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_SYSTEM:
			fillBoolean(paloJobRequest->showSystem, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SHOW_USER_INFO:
			fillBoolean(paloJobRequest->showUserInfo, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SKIP_EMPTY:
			fillBoolean(paloJobRequest->skipEmpty, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_USE_IDENTIFIER:
			fillBoolean(paloJobRequest->useIdentifier, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_USE_RULES:
			fillBoolean(paloJobRequest->useRules, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ACTION:
			fillString(paloJobRequest->action, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_COMMENT:
			fillString(paloJobRequest->comment, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_CONDITION:
			fillString(paloJobRequest->condition, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NAME_CUBE:
			fillString(paloJobRequest->cubeName, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NAME_DATABASE:
			fillString(paloJobRequest->databaseName, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_DEFINITION:
			fillString(paloJobRequest->definition, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NAME_DIMENSION:
			fillString(paloJobRequest->dimensionName, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NAME_ELEMENT:
			fillString(paloJobRequest->elementName, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_EVENT:
			fillString(paloJobRequest->event, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_EXTERN_PASSWORD:
			fillString(paloJobRequest->externPassword, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_EXTERNAL_IDENTIFIER:
			fillString(paloJobRequest->externalIdentifier, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_FUNCTIONS:
			fillString(paloJobRequest->functions, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NEW_NAME:
			fillString(paloJobRequest->newName, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_PASSWORD:
			fillString(paloJobRequest->password, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SOURCE:
			fillString(paloJobRequest->source, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NAME_USER:
			fillString(paloJobRequest->user, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_VALUE:
			fillString(paloJobRequest->value, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_BLOCKSIZE:
			fillUint(paloJobRequest->blockSize, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_MODE:
			fillUint(paloJobRequest->mode, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_POSITION:
			fillUint(paloJobRequest->position, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_PROPERTIES:
			fillVectorIdentifier(paloJobRequest->properties, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_SPLASH:
			fillUint(paloJobRequest->splash, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_NUM_STEPS:
			fillUint(paloJobRequest->steps, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_TYPE:
			fillUint(paloJobRequest->type, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_TYPES:
			fillVectorUint(paloJobRequest->types, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_DIMENSIONS:
			fillVectorIdentifier(paloJobRequest->dimensions, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_PARENT:
			if (valueStart == valuePtr) {
				// empty parameter -> no parent specified - root
				paloJobRequest->parent = NO_IDENTIFIER;
			} else {
				fillIdentifier(paloJobRequest->parent, valueStart, valuePtr);
			}
			break;

		case PaloRequestHandler::CMD_ID_PATH:
			fillVectorIdentifier(paloJobRequest->path, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_PATH_TO:
			fillVectorIdentifier(paloJobRequest->pathTo, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_ID_ELEMENTS:
			fillVectorIdentifier(paloJobRequest->elements, valueStart, valuePtr);
			break;

		case PaloRequestHandler::CMD_WEIGHTS:
			fillVectorVectorDouble(paloJobRequest->weights, valueStart, valuePtr, ':', ',');
			break;

		case PaloRequestHandler::CMD_NAME_DIMENSIONS:
			fillVectorString(paloJobRequest->dimensionsName, valueStart, valuePtr, ',');
			break;

		case PaloRequestHandler::CMD_NAME_ELEMENTS:
			fillVectorStringQuote(paloJobRequest->elementsName, valueStart, valuePtr, ',');
			break;

		case PaloRequestHandler::CMD_NAME_PATH:
			fillVectorString(paloJobRequest->pathName, valueStart, valuePtr, ',');
			break;

		case PaloRequestHandler::CMD_NAME_PATH_TO:
			fillVectorString(paloJobRequest->pathToName, valueStart, valuePtr, ',');
			break;

		case PaloRequestHandler::CMD_VALUES:
			fillVectorStringQuote(paloJobRequest->values, valueStart, valuePtr, ':');
			break;

		case PaloRequestHandler::CMD_ID_AREA:
			fillVectorVectorIdentifier(paloJobRequest->area, valueStart, valuePtr, ',', ':');
			break;

		case PaloRequestHandler::CMD_ID_CHILDREN:
			fillVectorVectorIdentifier(paloJobRequest->children, valueStart, valuePtr, ':', ',');
			break;

		case PaloRequestHandler::CMD_ID_PATHS:
			fillVectorVectorIdentifier(paloJobRequest->paths, valueStart, valuePtr, ':', ',');
			break;

		case PaloRequestHandler::CMD_NAME_AREA:
			fillVectorVectorString(paloJobRequest->areaName, valueStart, valuePtr, ',', ':');
			break;

		case PaloRequestHandler::CMD_NAME_CHILDREN:
			fillVectorVectorStringQuote(paloJobRequest->childrenName, valueStart, valuePtr, ':', ',');
			break;

		case PaloRequestHandler::CMD_NAME_PATHS:
			fillVectorVectorString(paloJobRequest->pathsName, valueStart, valuePtr, ':', ',');
			break;
		case PaloRequestHandler::CMD_FUNCTION:
			fillUint(paloJobRequest->function, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_EXPAND:
			fillVectorUint(paloJobRequest->expand, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_ACTCODE:
			fillString(paloJobRequest->actcode, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_LICKEY:
			fillString(paloJobRequest->lickey, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_MACHINE:
			fillString(paloJobRequest->machineString, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_REQUIRED:
			fillString(paloJobRequest->requiredFeatures, valueStart, valuePtr);
			break;
		case PaloRequestHandler::CMD_OPTIONAL:
			fillString(paloJobRequest->optionalFeatures, valueStart, valuePtr);
			break;
		}
	}
}

static inline uint32_t parseUint32(char* valueStart, char* valueEnd)
{
	uint32_t id = 0;

	for (; valueStart < valueEnd; ++valueStart) {
		if ('0' <= *valueStart && *valueStart <= '9') {
			id = id * 10 + (uint32_t)(*valueStart - '0');
		} else {
			return 0;
		}
	}

	return id;
}

void PaloHttpRequest::fillToken(uint32_t*& identifier, char* valueStart, char* valueEnd)
{
	identifier = new uint32_t;
	*identifier = parseUint32(valueStart, valueEnd);
}

void PaloHttpRequest::fillIdentifier(IdentifierType& identifier, char* valueStart, char* valueEnd)
{
	if (valueEnd != valueStart) {
		uint32_t id = parseUint32(valueStart, valueEnd);
		identifier = (IdentifierType)id;
	}
}

void PaloHttpRequest::fillBoolean(bool& flag, char* valueStart, char* valueEnd)
{
	flag = (valueStart + 1 == valueEnd && *valueStart == '1');
}

void PaloHttpRequest::fillString(string*& text, char* valueStart, char* valueEnd)
{
	text = new string(valueStart, valueEnd);
}

void PaloHttpRequest::fillSid(string& text, char* valueStart, char* valueEnd)
{
	text = string(valueStart, valueEnd);
}

void PaloHttpRequest::fillUint(uint32_t& identifier, char* valueStart, char* valueEnd)
{
	uint32_t id = parseUint32(valueStart, valueEnd);
	identifier = (uint32_t)id;
}

void PaloHttpRequest::fillVectorIdentifier(IdentifiersType*& identifiers, char* valueStart, char* valueEnd)
{
	identifiers = new IdentifiersType();
	const char* p = valueStart;
	const char* e = valueEnd;
	identifiers->reserve((e - p) / 3);

	if (p == e) {
		return;
	}

	IdentifierType id = 0;

	for (; p < e; p++) {
		if (*p == ',') {
			identifiers->push_back(id);
			id = 0;
		} else {
			if ('0' <= *p && *p <= '9') {
				id = id * 10 + (IdentifierType)(*p - '0');
			} else {
				delete identifiers;
				identifiers = 0;
				return;
			}
		}
	}

	identifiers->push_back(id);
}

void PaloHttpRequest::fillVectorUint(vector<uint32_t>*& ints, char* valueStart, char* valueEnd)
{
	ints = new vector<uint32_t> ();
	const char* p = valueStart;
	const char* e = valueEnd;

	if (p == e) {
		return;
	}

	uint32_t id = 0;

	for (; p < e; p++) {
		if (*p == ',') {
			ints->push_back(id);
			id = 0;
		} else {
			if ('0' <= *p && *p <= '9') {
				id = id * 10 + (uint32_t)(*p - '0');
			} else {
				delete ints;
				ints = 0;
				return;
			}
		}
	}

	ints->push_back(id);
}

void PaloHttpRequest::fillVectorDouble(vector<double>*& doubles, char* valueStart, char* valueEnd)
{
	doubles = new vector<double> ();
	string buffer = valueStart;
	size_t size = valueEnd - valueStart;
	size_t pos = 0;

	while (pos < size) {
		string s = StringUtils::getNextElement(buffer, pos, ',');
		double v = StringUtils::stringToDouble(s);
		doubles->push_back(v);
	}
}

void PaloHttpRequest::fillVectorString(vector<string>*& strings, char* valueStart, char* valueEnd, char separator)
{
	strings = new vector<string> ();
	string buffer = valueStart;
	size_t size = valueEnd - valueStart;
	size_t pos = 0;

	while (pos < size) {
		string s = StringUtils::getNextElement(buffer, pos, separator);
		strings->push_back(s);
	}
}

void PaloHttpRequest::fillVectorStringQuote(vector<string>*& strings, char* valueStart, char* valueEnd, char separator)
{
	strings = new vector<string> ();
	string buffer = valueStart;
	size_t size = valueEnd - valueStart;
	size_t pos = 0;

	while (pos < size) {
		string s = StringUtils::getNextElement(buffer, pos, separator, true);
		strings->push_back(s);
	}
}

void PaloHttpRequest::fillVectorVectorIdentifier(vector<IdentifiersType>*& identifiers, char* valueStart, char* valueEnd, char first, char second)
{
	identifiers = new vector<IdentifiersType> ;
	const char* p = valueStart;
	const char* e = valueEnd;

	for (; p < e; p++) {
		identifiers->push_back(IdentifiersType());

		if (*p == '*') {
			p++;

			if (p < e && *p != first) {
				delete identifiers;
				identifiers = 0;
				return;
			}
		} else if (*p == first) {
		} else {
			IdentifiersType& vec = (*identifiers)[identifiers->size() - 1];
			IdentifierType id = 0;

			for (; p < e && *p != first; p++) {
				if (*p == second) {
					vec.push_back(id);
					id = 0;
				} else {
					if ('0' <= *p && *p <= '9') {
						if ((IdentifierType) - 1 != id) {
							id = id * 10 + (IdentifierType)(*p - '0');
						}
					} else {
						id = (IdentifierType) - 1;
					}
				}
			}

			vec.push_back(id);
		}
	}

	// last char was delimiter, add one empty dimension for it
	if (valueStart < valueEnd && *(valueEnd - 1) == first) {
		identifiers->push_back(IdentifiersType());
	}
}

void PaloHttpRequest::fillVectorVectorString(vector<vector<string> >*& strings, char* valueStart, char* valueEnd, char first, char second)
{
	strings = new vector<vector<string> > ;
	string buffer = valueStart;
	size_t size = valueEnd - valueStart;
	size_t pos = 0;

	while (pos < size) {
		string s = StringUtils::getNextElement(buffer, pos, first);

		if (s.size() == 0 || s == "*") {
			vector<string> v = vector<string> ();
			strings->push_back(v);
		} else {
			vector<string> v = vector<string> ();
			size_t pos2 = 0;

			while (pos2 < s.size()) {
				string s2 = StringUtils::getNextElement(s, pos2, second);
				v.push_back(s2);
			}

			strings->push_back(v);
		}
	}
}

void PaloHttpRequest::fillVectorVectorStringQuote(vector<vector<string> >*& strings, char* valueStart, char* valueEnd, char first, char second)
{

	strings = new vector<vector<string> > ();
	StringUtils::splitString2(valueStart, valueEnd, strings, first, second, true);
	for (size_t i = 0; i < strings->size(); i++)
		if ((*strings)[i].size() == 1 && (*strings)[i][0] == "*")
			(*strings)[i] = vector<string> ();
}

void PaloHttpRequest::fillVectorVectorDouble(vector<vector<double> >*& doubles, char* valueStart, char* valueEnd, char first, char second)
{
	doubles = new vector<vector<double> > ;
	string buffer = valueStart;
	size_t size = valueEnd - valueStart;
	size_t pos = 0;

	while (pos < size) {
		string s = StringUtils::getNextElement(buffer, pos, first);

		if (s.size() == 0 || s == "*") {
			vector<double> v = vector<double> ();
			doubles->push_back(v);
		} else {
			vector<double> v = vector<double> ();
			size_t pos2 = 0;

			while (pos2 < s.size()) {
				string s2 = StringUtils::getNextElement(s, pos2, second);
				double d = StringUtils::stringToDouble(s2);
				v.push_back(d);
			}

			doubles->push_back(v);
		}
	}
}
}
