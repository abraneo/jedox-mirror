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

#ifndef PALO_HTTP_SERVER_DOCUMENTATION_HANDLER_H
#define PALO_HTTP_SERVER_DOCUMENTATION_HANDLER_H 1

#include "palo.h"

#include "HttpServer/DirectHttpResponse.h"
#include "HttpServer/HttpJobRequest.h"
#include "HttpServer/HttpRequest.h"
#include "HttpServer/HttpRequestHandler.h"
#include "PaloDocumentation/FileDocumentation.h"
#include "PaloDocumentation/HtmlFormatter.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief api documentation
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS DocumentationHandler : public HttpRequestHandler {
public:
	DocumentationHandler(const string& templateFile, const string& documentation) :
		documentation(documentation), templateFile(templateFile), contentType("text/html;charset=utf-8") {
	}
	;

	DocumentationHandler(const string& templateFile, const string& documentation, const string& contentType) :
		documentation(documentation), templateFile(templateFile), contentType(contentType) {
	}
	;

public:
	HttpJobRequest * handleHttpRequest(HttpRequest *request, const HttpServerTask *task) {
		HttpResponse * response = new HttpResponse(HttpResponse::OK);

		FileDocumentation fd(documentation);
		HtmlFormatter hd(templateFile);

		response->getBody().copy(hd.getDocumentation(&fd));
		response->setContentType(contentType);

		return new DirectHttpResponse(request->getRequestPath(), response);
	}

private:
	string documentation;
	string templateFile;
	string contentType;
};

}

#endif
