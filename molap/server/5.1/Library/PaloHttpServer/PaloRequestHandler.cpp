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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "PaloHttpServer/PaloRequestHandler.h"

#include "Collections/StringUtils.h"
#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpResponse.h"
#include "HttpServer/DirectHttpResponse.h"
#include "PaloDispatcher/PaloJobRequest.h"

namespace palo {

// ////////////////
// static constants
// ////////////////

const string PaloRequestHandler::ACTIVATE = "activate";
const string PaloRequestHandler::ACTION = "action";
const string PaloRequestHandler::ADD = "add";
const string PaloRequestHandler::BASE_ONLY = "base_only";
const string PaloRequestHandler::BLOCKSIZE = "blocksize";
const string PaloRequestHandler::COMMENT = "comment";
const string PaloRequestHandler::COMPLETE = "complete";
const string PaloRequestHandler::CONDITION = "condition";
const string PaloRequestHandler::DEFINITION = "definition";
const string PaloRequestHandler::EXTERN_PASSWORD = "extern_password";
const string PaloRequestHandler::EXTERNAL_IDENTIFIER = "external_identifier";
const string PaloRequestHandler::EVENT = "event";
const string PaloRequestHandler::EVENT_PROCESSOR = "event_processor";
const string PaloRequestHandler::FUNCTIONS = "functions";
const string PaloRequestHandler::ID_AREA = "area";
const string PaloRequestHandler::ID_CHILDREN = "children";
const string PaloRequestHandler::ID_CUBE = "cube";
const string PaloRequestHandler::ID_DATABASE = "database";
const string PaloRequestHandler::ID_DIMENSION = "dimension";
const string PaloRequestHandler::ID_DIMENSIONS = "dimensions";
const string PaloRequestHandler::ID_ELEMENT = "element";
const string PaloRequestHandler::ID_ELEMENTS = "elements";
const string PaloRequestHandler::ID_LOCK = "lock";
const string PaloRequestHandler::ID_MODE = "mode";
const string PaloRequestHandler::ID_PARENT = "parent";
const string PaloRequestHandler::ID_PATH = "path";
const string PaloRequestHandler::ID_PATHS = "paths";
const string PaloRequestHandler::ID_LOCKED_PATHS = "locked_paths";
const string PaloRequestHandler::ID_PATH_TO = "path_to";
const string PaloRequestHandler::ID_RULE = "rule";
const string PaloRequestHandler::ID_TYPE = "type";
const string PaloRequestHandler::ID_TYPES = "types";
const string PaloRequestHandler::NAME_AREA = "name_area";
const string PaloRequestHandler::NAME_CHILDREN = "name_children";
const string PaloRequestHandler::NAME_CUBE = "name_cube";
const string PaloRequestHandler::NAME_DATABASE = "name_database";
const string PaloRequestHandler::NAME_DIMENSION = "name_dimension";
const string PaloRequestHandler::NAME_DIMENSIONS = "name_dimensions";
const string PaloRequestHandler::NAME_ELEMENT = "name_element";
const string PaloRequestHandler::NAME_ELEMENTS = "name_elements";
const string PaloRequestHandler::NAME_PATH = "name_path";
const string PaloRequestHandler::NAME_PATHS = "name_paths";
const string PaloRequestHandler::NAME_PATH_TO = "name_path_to";
const string PaloRequestHandler::NAME_USER = "user";
const string PaloRequestHandler::NEW_NAME = "new_name";
const string PaloRequestHandler::NUM_STEPS = "steps";
const string PaloRequestHandler::PASSWORD = "password";
const string PaloRequestHandler::POSITION = "position";
const string PaloRequestHandler::POSITIONS = "positions";
const string PaloRequestHandler::SID = "sid";
const string PaloRequestHandler::SHOW_ATTRIBUTE = "show_attribute";
const string PaloRequestHandler::SHOW_NORMAL = "show_normal";
const string PaloRequestHandler::SHOW_RULE = "show_rule";
const string PaloRequestHandler::SHOW_RULES = "show_rules";
const string PaloRequestHandler::SHOW_SYSTEM = "show_system";
const string PaloRequestHandler::SHOW_INFO = "show_info";
const string PaloRequestHandler::SHOW_GPUTYPE = "show_gputype";
const string PaloRequestHandler::SKIP_EMPTY = "skip_empty";
const string PaloRequestHandler::SOURCE = "source";
const string PaloRequestHandler::SPLASH = "splash";
const string PaloRequestHandler::USE_IDENTIFIER = "use_identifier";
const string PaloRequestHandler::USE_RULES = "use_rules";
const string PaloRequestHandler::VALUE = "value";
const string PaloRequestHandler::VALUES = "values";
const string PaloRequestHandler::WEIGHTS = "weights";
const string PaloRequestHandler::SHOW_LOCK_INFO = "show_lock_info";
const string PaloRequestHandler::FUNCTION = "function";
const string PaloRequestHandler::EXPAND = "expand";
const string PaloRequestHandler::SHOW_PERMISSION = "show_permission";
const string PaloRequestHandler::VIEW_SUBSETS = "view_subsets";
const string PaloRequestHandler::VIEW_AXES = "view_axes";
const string PaloRequestHandler::VIEW_AREA = "view_area";

const string PaloRequestHandler::X_PALO_SERVER = "X-PALO-SV";
const string PaloRequestHandler::X_PALO_DATABASE = "X-PALO-DB";
const string PaloRequestHandler::X_PALO_DIMENSION = "X-PALO-DIM";
const string PaloRequestHandler::X_PALO_CUBE = "X-PALO-CB";
const string PaloRequestHandler::X_PALO_CUBE_CLIENT_CACHE = "X-PALO-CC";

// /////////////////////////////////////////////////////////////////////////////
// HttpRequestHandler methods
// /////////////////////////////////////////////////////////////////////////////

HttpJobRequest * PaloRequestHandler::handleHttpRequest(HttpRequest* request, const HttpServerTask *task)
{
	HttpJobRequest* jobRequest = generateJobRequest(request);
	PaloJobRequest* paloJobRequest = dynamic_cast<PaloJobRequest*>(jobRequest);

	if (paloJobRequest != 0) {
		paloJobRequest->hasSession = false;
	}

	return jobRequest;
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

HttpJobRequest * PaloRequestHandler::generateJobRequest(HttpRequest* request)
{
	PaloHttpRequest* paloRequest = dynamic_cast<PaloHttpRequest*>(request);

	if (paloRequest == 0) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "not an palo request");
	}

	if (!enabled) {
		HttpResponse * response = new HttpResponse(HttpResponse::BAD);

		response->getBody().appendInteger((uint32_t)ErrorException::ERROR_HTTP_DISABLED);
		response->getBody().appendText(";" + StringUtils::escapeString(ErrorException::getDescriptionErrorType(ErrorException::ERROR_HTTP_DISABLED)) + ";");
		response->getBody().appendText(StringUtils::escapeString("please use secure communication"));
		response->getBody().appendEol();

		return new DirectHttpResponse("disabled", response);
	}

	PaloJobRequest* job = paloRequest->releasePaloJobRequest();
	job->httpRequest = job->getName() + paloRequest->getHttpParams()->str();
	job->httpsPort = httpsPort;

	return job;
}
}
