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

#ifndef PALO_HTTP_SERVER_PALO_REQUEST_HANDLER_H
#define PALO_HTTP_SERVER_PALO_REQUEST_HANDLER_H 1

#include "palo.h"

#include "HttpServer/HttpRequestHandler.h"
#include "PaloHttpServer/PaloHttpRequest.h"

namespace palo {
class PaloJobRequest;

////////////////////////////////////////////////////////////////////////////////
/// @brief http request handler for palo objects
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloRequestHandler : public HttpRequestHandler {
public:
	static const string ACTIVATE;
	static const string ACTION;
	static const string ADD;
	static const string BASE_ONLY;
	static const string BLOCKSIZE;
	static const string COMMENT;
	static const string COMPLETE;
	static const string CONDITION;
	static const string DEFINITION;
	static const string EXTERN_PASSWORD;
	static const string EXTERNAL_IDENTIFIER;
	static const string FUNCTIONS;
	static const string EVENT;
	static const string EVENT_PROCESSOR;
	static const string ID_AREA;
	static const string ID_CHILDREN;
	static const string ID_CUBE;
	static const string ID_DATABASE;
	static const string ID_DIMENSION;
	static const string ID_DIMENSIONS;
	static const string ID_ELEMENT;
	static const string ID_ELEMENTS;
	static const string ID_LOCK;
	static const string ID_MODE;
	static const string ID_PARENT;
	static const string ID_PATH;
	static const string ID_PATHS;
	static const string ID_LOCKED_PATHS;
	static const string ID_PATH_TO;
	static const string ID_RULE;
	static const string ID_TYPE;
	static const string ID_TYPES;
	static const string NAME_AREA;
	static const string NAME_CHILDREN;
	static const string NAME_CUBE;
	static const string NAME_DATABASE;
	static const string NAME_DIMENSION;
	static const string NAME_DIMENSIONS;
	static const string NAME_ELEMENT;
	static const string NAME_ELEMENTS;
	static const string NAME_PATH;
	static const string NAME_PATHS;
	static const string NAME_PATH_TO;
	static const string NAME_USER;
	static const string NEW_NAME;
	static const string NUM_STEPS;
	static const string PASSWORD;
	static const string POSITION;
	static const string POSITIONS;
	static const string SID;
	static const string SHOW_ATTRIBUTE;
	static const string SHOW_NORMAL;
	static const string SHOW_RULES;
	static const string SHOW_RULE;
	static const string SHOW_SYSTEM;
	static const string SHOW_USER_INFO;
	static const string SHOW_INFO;
	static const string SHOW_GPUTYPE;
	static const string SKIP_EMPTY;
	static const string SOURCE;
	static const string SPLASH;
	static const string USE_IDENTIFIER;
	static const string USE_RULES;
	static const string VALUE;
	static const string VALUES;
	static const string WEIGHTS;
	static const string SHOW_LOCK_INFO;
	static const string FUNCTION;
	static const string EXPAND;
	static const string SHOW_PERMISSION;
	static const string VIEW_SUBSETS;
	static const string VIEW_AXES;
	static const string VIEW_AREA;

	static const string X_PALO_SERVER;
	static const string X_PALO_DATABASE;
	static const string X_PALO_DIMENSION;
	static const string X_PALO_CUBE;
	static const string X_PALO_CUBE_CLIENT_CACHE;

	enum CommandIdentifier {
		CMD_ACTCODE,
		CMD_ACTIVATE,
		CMD_ACTION,
		CMD_ADD,
		CMD_BASE_ONLY,
		CMD_BLOCKSIZE,
		CMD_COMMENT,
		CMD_COMPLETE,
		CMD_CONDITION,
		CMD_CONTENT_LENGTH,
		CMD_DEFINITION,
		CMD_EXTERN_PASSWORD,
		CMD_EXTERNAL_IDENTIFIER,
		CMD_FUNCTIONS,
		CMD_EVENT,
		CMD_EVENT_PROCESSOR,
		CMD_ID_AREA,
		CMD_ID_CHILDREN,
		CMD_ID_CUBE,
		CMD_ID_DATABASE,
		CMD_ID_DIMENSION,
		CMD_ID_DIMENSIONS,
		CMD_ID_ELEMENT,
		CMD_ID_ELEMENTS,
		CMD_ID_LIMIT,
		CMD_ID_LOCK,
		CMD_ID_LOCKED_PATHS,
		CMD_ID_MODE,
		CMD_ID_PARENT,
		CMD_ID_PATH,
		CMD_ID_PATHS,
		CMD_ID_PATH_TO,
		CMD_ID_RULE,
		CMD_ID_TYPE,
		CMD_ID_TYPES,
		CMD_LICKEY,
		CMD_NAME_AREA,
		CMD_NAME_CHILDREN,
		CMD_NAME_CUBE,
		CMD_NAME_DATABASE,
		CMD_NAME_DIMENSION,
		CMD_NAME_DIMENSIONS,
		CMD_NAME_ELEMENT,
		CMD_NAME_ELEMENTS,
		CMD_NAME_PATH,
		CMD_NAME_PATHS,
		CMD_NAME_PATH_TO,
		CMD_NAME_USER,
		CMD_NEW_NAME,
		CMD_NUM_STEPS,
		CMD_PASSWORD,
		CMD_POSITION,
		CMD_POSITIONS,
		CMD_PROPERTIES,
		CMD_SID,
		CMD_SHOW_ATTRIBUTE,
		CMD_SHOW_NORMAL,
		CMD_SHOW_RULES,
		CMD_SHOW_RULE,
		CMD_SHOW_SYSTEM,
		CMD_SHOW_USER_INFO,
		CMD_SHOW_INFO,
		CMD_SHOW_GPUTYPE,
		CMD_SKIP_EMPTY,
		CMD_SOURCE,
		CMD_SPLASH,
		CMD_USE_IDENTIFIER,
		CMD_USE_RULES,
		CMD_VALUE,
		CMD_VALUES,
		CMD_WEIGHTS,
		CMD_SHOW_LOCK_INFO,
		CMD_FUNCTION,
		CMD_EXPAND,
		CMD_MACHINE,
		CMD_REQUIRED,
		CMD_OPTIONAL,
		CMD_SHOW_PERMISSION,
		CMD_VIEW_SUBSETS,
		CMD_VIEW_AXES,
		CMD_VIEW_AREA,

		CMD_X_PALO_SERVER,
		CMD_X_PALO_DATABASE,
		CMD_X_PALO_DIMENSION,
		CMD_X_PALO_CUBE,
		CMD_X_PALO_CUBE_CLIENT_CACHE,
	};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloRequestHandler(bool enabled, int httpsPort = 0) :
		enabled(enabled), httpsPort(httpsPort) {
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	HttpRequest * createHttpRequest(const string& path) {
		return new PaloHttpRequest(path, this);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	HttpJobRequest * handleHttpRequest(HttpRequest*, const HttpServerTask*);

protected:
	HttpJobRequest * generateJobRequest(HttpRequest* request);

protected:
	const bool enabled;
	int httpsPort;
};
}

#endif
