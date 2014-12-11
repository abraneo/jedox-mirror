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

#include "HttpServer/HttpFileRequestHandler.h"

#include <iostream>
#include "InputOutput/FileUtils.h"

#include "HttpServer/DirectHttpResponse.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

HttpFileRequestHandler::HttpFileRequestHandler(const string& file, const string& contentType) :
	filename(file), contentType(contentType)
{
}

// /////////////////////////////////////////////////////////////////////////////
// HttpRequestHandler methods
// /////////////////////////////////////////////////////////////////////////////

HttpJobRequest * HttpFileRequestHandler::handleHttpRequest(HttpRequest * request, const HttpServerTask *task)
{
	FileUtils::paloifstream file(filename.c_str());

	if (!file) {
		HttpResponse * response = new HttpResponse(HttpResponse::NOT_FOUND);
		response->getBody().appendText("file not found");
		return new DirectHttpResponse(request->getRequestPath(), response);
	}

	HttpResponse * response = new HttpResponse(HttpResponse::OK);
	response->setContentType(contentType);

	int max = 1000;
	char* c = new char[max];

	while (!file.eof()) {
		file.read(c, max);
		response->getBody().appendText(c, (size_t)file.gcount());
	}

	delete[] c;

	return new DirectHttpResponse(request->getRequestPath(), response);
}
}
