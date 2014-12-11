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

#include "PaloHttpServer/PaloHttpServer.h"

#include "HttpServer/HttpFileRequestHandler.h"
#include "PaloHttpServer/PaloSSORequestHandler.h"

#include "PaloHttpServer/DocumentationHandler.h"
#include "PaloHttpServer/NotFoundHandler.h"
#include "PaloHttpServer/PaloBrowserHandler.h"
#include "PaloHttpServer/PaloBrowserHandlerSid.h"
#include "PaloHttpServer/PaloRequestHandler.h"
#include "PaloHttpServer/PaloRequestHandlerSid.h"
#include "Olap/Server.h"

namespace palo {

HttpRequestHandler* PaloHttpServer::HandleCreator::create(bool enabled, bool writeRequest) const
{
	return new PaloRequestHandlerSid(enabled);
}

PaloHttpServer::HandleCreator::~HandleCreator()
{
}

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloHttpServer::PaloHttpServer(const string& templateDirectory) :
	IoTask(INVALID_SOCKET, INVALID_SOCKET), HttpServer(), templateDirectory(templateDirectory)
{

	// add static items
	addStaticHandlers();

	// add error handler
	addNotFoundHandler(new NotFoundHandler(templateDirectory + "/not_found.tmpl", templateDirectory + "/home.api", "text/html;charset=utf-8"));

	// add documentation
	addDocumentationHandlers(templateDirectory + "/documentation");

	// add commands
	addCommandHandlers(false, HandleCreator());
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpServer::enableBroswer()
{
	addBrowserHandlers();
}

void PaloHttpServer::enableServerInfo(int httpsPort)
{
	addHandler("/server/info", new PaloRequestHandler(true, httpsPort));
	addHandler("/server/license", new PaloRequestHandler(true, httpsPort));
	addHandler("/server/licenses", new PaloRequestHandler(true, httpsPort));
}

void PaloHttpServer::enablePalo(const HandleCreator &handleCreator)
{
	addCommandHandlers(true, handleCreator);
}

void PaloHttpServer::enablePalo()
{
	addCommandHandlers(true, HandleCreator());
}

void PaloHttpServer::enableLogin()
{
	addHandler("/server/login", new PaloSSORequestHandler(true));
}

// /////////////////////////////////////////////////////////////////////////////
// handlers
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpServer::addCommandHandlers(bool enabled, const HandleCreator &handleCreator)
{
	// might be overwritten by enableServerInfo with a HTTPS port
	addHandler("/server/info", new PaloRequestHandler(enabled));
	addHandler("/server/license", new PaloRequestHandler(enabled));
	addHandler("/server/licenses", new PaloRequestHandler(enabled));
	addHandler("/server/login", new PaloSSORequestHandler(false));

	// sid-full
	addHandler("/cell/area_counter", handleCreator.create(enabled, false));
	addHandler("/cell/area", handleCreator.create(enabled, false));
	addHandler("/cell/copy", handleCreator.create(enabled, true));
	addHandler("/cell/drillthrough", handleCreator.create(enabled, false));
	addHandler("/cell/export", handleCreator.create(enabled, false));
	addHandler("/cell/goalseek", handleCreator.create(enabled, true));
	addHandler("/cell/replace_bulk", handleCreator.create(enabled, true));
	addHandler("/cell/replace", handleCreator.create(enabled, true));
	addHandler("/cell/value", handleCreator.create(enabled, false));
	addHandler("/cell/values", handleCreator.create(enabled, false));

	addHandler("/cube/clear_cache", handleCreator.create(enabled, false));
	addHandler("/cube/clear", handleCreator.create(enabled, true));
	addHandler("/cube/commit", handleCreator.create(enabled, true));
	addHandler("/cube/create", handleCreator.create(enabled, true));
	addHandler("/cube/convert", handleCreator.create(enabled, true));
	addHandler("/cube/destroy", handleCreator.create(enabled, true));
	addHandler("/cube/info", handleCreator.create(enabled, false));
	addHandler("/cube/load", handleCreator.create(enabled, false));
	addHandler("/cube/lock", handleCreator.create(enabled, false));
	addHandler("/cube/locks", handleCreator.create(enabled, false));
	addHandler("/cube/rename", handleCreator.create(enabled, true));
	addHandler("/cube/rollback", handleCreator.create(enabled, false));
	addHandler("/cube/rules", handleCreator.create(enabled, false));
	addHandler("/cube/save", handleCreator.create(enabled, true));
	addHandler("/cube/unload", handleCreator.create(enabled, false));

	addHandler("/database/create", handleCreator.create(enabled, true));
	addHandler("/database/cubes", handleCreator.create(enabled, false));
	addHandler("/database/destroy", handleCreator.create(enabled, true));
	addHandler("/database/dimensions", handleCreator.create(enabled, false));
	addHandler("/database/info", handleCreator.create(enabled, false));
	addHandler("/database/load", handleCreator.create(enabled, false));
	addHandler("/database/rename", handleCreator.create(enabled, true));
	addHandler("/database/save", handleCreator.create(enabled, true));
	addHandler("/database/unload", handleCreator.create(enabled, false));

	addHandler("/dimension/clear", handleCreator.create(enabled, true));
	addHandler("/dimension/create", handleCreator.create(enabled, true));
	addHandler("/dimension/cubes", handleCreator.create(enabled, false));
	addHandler("/dimension/destroy", handleCreator.create(enabled, true));
	addHandler("/dimension/element", handleCreator.create(enabled, false));
	addHandler("/dimension/elements", handleCreator.create(enabled, false));
	addHandler("/dimension/info", handleCreator.create(enabled, false));
	addHandler("/dimension/rename", handleCreator.create(enabled, true));
	addHandler("/dimension/dfilter", handleCreator.create(enabled, false));

	addHandler("/element/append", handleCreator.create(enabled, true));
	addHandler("/element/create", handleCreator.create(enabled, true));
	addHandler("/element/create_bulk", handleCreator.create(enabled, true));
	addHandler("/element/destroy", handleCreator.create(enabled, true));
	addHandler("/element/destroy_bulk", handleCreator.create(enabled, true));
	addHandler("/element/info", handleCreator.create(enabled, false));
	addHandler("/element/move", handleCreator.create(enabled, true));
	addHandler("/element/move_bulk", handleCreator.create(enabled, true));
	addHandler("/element/rename", handleCreator.create(enabled, true));
	addHandler("/element/replace", handleCreator.create(enabled, true));
	addHandler("/element/replace_bulk", handleCreator.create(enabled, true));

	addHandler("/event/begin", handleCreator.create(enabled, false));
	addHandler("/event/end", handleCreator.create(enabled, false));

	addHandler("/rule/create", handleCreator.create(enabled, true));
	addHandler("/rule/destroy", handleCreator.create(enabled, true));
	addHandler("/rule/functions", new PaloBrowserHandlerSid(templateDirectory));
	addHandler("/rule/info", handleCreator.create(enabled, false));
	addHandler("/rule/modify", handleCreator.create(enabled, true));
	addHandler("/rule/parse", handleCreator.create(enabled, false));

	addHandler("/server/databases", handleCreator.create(enabled, false));
	addHandler("/server/load", handleCreator.create(enabled, false));
	addHandler("/server/logout", handleCreator.create(enabled, false));
	addHandler("/server/save", handleCreator.create(enabled, true));
	addHandler("/server/shutdown", handleCreator.create(enabled, false));
	addHandler("/server/markers", handleCreator.create(enabled, false));
	addHandler("/server/change_password", handleCreator.create(enabled, true));
	addHandler("/server/user_info", handleCreator.create(enabled, false));
	addHandler("/server/activate_license", handleCreator.create(enabled, true));

	addHandler("/svs/info", handleCreator.create(enabled, false));
	addHandler("/svs/restart", handleCreator.create(enabled, false));

	addHandler("/statistics/clear", handleCreator.create(enabled, false));
	addHandler("/view/calculate", handleCreator.create(enabled, false));
}

void PaloHttpServer::addStaticHandlers()
{

	// add handlers for pictures and style sheets
	addHandler("/", new DocumentationHandler(templateDirectory + "/home.tmpl", templateDirectory + "/home.api"));
	addHandler("/api", new DocumentationHandler(templateDirectory + "/api_overview.tmpl", templateDirectory + "/api_overview.api"));

	addHandler("/api/example.html", new HttpFileRequestHandler(templateDirectory + "/example.html", "text/html;charset=utf-8"));
	addHandler("/favicon.ico", new HttpFileRequestHandler(templateDirectory + "/favicon.ico", "image/x-icon"));
	addHandler("/inc/background.gif", new HttpFileRequestHandler(templateDirectory + "/background.gif", "image/gif"));
	addHandler("/inc/bg.gif", new HttpFileRequestHandler(templateDirectory + "/bg.gif", "image/gif"));
	addHandler("/inc/header5.jpg", new HttpFileRequestHandler(templateDirectory + "/header5.jpg", "image/jpeg"));
	addHandler("/inc/style_palo.css", new HttpFileRequestHandler(templateDirectory + "/style_palo.css", "text/css"));
	addHandler("/inc/topheader.gif", new HttpFileRequestHandler(templateDirectory + "/topHeader.gif", "image/gif"));
	addHandler("/js/palo.html", new HttpFileRequestHandler(templateDirectory + "/palo.html", "text/html"));
	addHandler("/js/palo.js", new HttpFileRequestHandler(templateDirectory + "/palo.js", "text/javascript"));
	addHandler("/js/md5.js", new HttpFileRequestHandler(templateDirectory + "/md5.js", "text/javascript"));
	addHandler("/js/rule_lexer.js", new HttpFileRequestHandler(templateDirectory + "/rule_lexer.js", "text/javascript"));
}

void PaloHttpServer::addDocumentationHandlers(const string& tmpl)
{
	const string d1 = tmpl + ".tmpl";
	const string d2 = tmpl + "2.tmpl";

	addHandler("/api/server/databases", new DocumentationHandler(d1, templateDirectory + "/server_databases.api"));
	addHandler("/api/server/info", new DocumentationHandler(d1, templateDirectory + "/server_info.api"));
	addHandler("/api/server/licenses", new DocumentationHandler(d1, templateDirectory + "/server_licenses.api"));
	addHandler("/api/server/load", new DocumentationHandler(d1, templateDirectory + "/server_load.api"));
	addHandler("/api/server/login", new DocumentationHandler(d2, templateDirectory + "/server_login.api"));
	addHandler("/api/server/logout", new DocumentationHandler(d1, templateDirectory + "/server_logout.api"));
	addHandler("/api/server/save", new DocumentationHandler(d1, templateDirectory + "/server_save.api"));
	addHandler("/api/server/shutdown", new DocumentationHandler(d1, templateDirectory + "/server_shutdown.api"));
	addHandler("/api/server/change_password", new DocumentationHandler(d1, templateDirectory + "/server_change_password.api"));
	addHandler("/api/server/user_info", new DocumentationHandler(d1, templateDirectory + "/server_user_info.api"));
	addHandler("/api/server/activate_license", new DocumentationHandler(d1, templateDirectory + "/server_activate_license.api"));

	addHandler("/api/database/create", new DocumentationHandler(d1, templateDirectory + "/database_create.api"));
	addHandler("/api/database/cubes", new DocumentationHandler(d1, templateDirectory + "/database_cubes.api"));
	addHandler("/api/database/destroy", new DocumentationHandler(d1, templateDirectory + "/database_delete.api"));
	addHandler("/api/database/dimensions", new DocumentationHandler(d1, templateDirectory + "/database_dimensions.api"));
	addHandler("/api/database/info", new DocumentationHandler(d1, templateDirectory + "/database_info.api"));
	addHandler("/api/database/load", new DocumentationHandler(d1, templateDirectory + "/database_load.api"));
	addHandler("/api/database/rename", new DocumentationHandler(d1, templateDirectory + "/database_rename.api"));
	addHandler("/api/database/save", new DocumentationHandler(d1, templateDirectory + "/database_save.api"));
	addHandler("/api/database/unload", new DocumentationHandler(d1, templateDirectory + "/database_unload.api"));

	addHandler("/api/dimension/clear", new DocumentationHandler(d1, templateDirectory + "/dimension_clear.api"));
	addHandler("/api/dimension/create", new DocumentationHandler(d1, templateDirectory + "/dimension_create.api"));
	addHandler("/api/dimension/cubes", new DocumentationHandler(d1, templateDirectory + "/dimension_cubes.api"));
	addHandler("/api/dimension/destroy", new DocumentationHandler(d1, templateDirectory + "/dimension_delete.api"));
	addHandler("/api/dimension/element", new DocumentationHandler(d1, templateDirectory + "/dimension_element.api"));
	addHandler("/api/dimension/elements", new DocumentationHandler(d1, templateDirectory + "/dimension_elements.api"));
	addHandler("/api/dimension/info", new DocumentationHandler(d1, templateDirectory + "/dimension_info.api"));
	addHandler("/api/dimension/rename", new DocumentationHandler(d1, templateDirectory + "/dimension_rename.api"));
	addHandler("/api/dimension/dfilter", new DocumentationHandler(d1, templateDirectory + "/dimension_dfilter.api"));

	addHandler("/api/cube/clear", new DocumentationHandler(d1, templateDirectory + "/cube_clear.api"));
	addHandler("/api/cube/clear_cache", new DocumentationHandler(d1, templateDirectory + "/cube_clear_cache.api"));
	addHandler("/api/cube/commit", new DocumentationHandler(d1, templateDirectory + "/cube_commit.api"));
	addHandler("/api/cube/create", new DocumentationHandler(d1, templateDirectory + "/cube_create.api"));
	addHandler("/api/cube/convert", new DocumentationHandler(d1, templateDirectory + "/cube_convert.api"));
	addHandler("/api/cube/destroy", new DocumentationHandler(d1, templateDirectory + "/cube_delete.api"));
	addHandler("/api/cube/info", new DocumentationHandler(d1, templateDirectory + "/cube_info.api"));
	addHandler("/api/cube/load", new DocumentationHandler(d1, templateDirectory + "/cube_load.api"));
	addHandler("/api/cube/lock", new DocumentationHandler(d1, templateDirectory + "/cube_lock.api"));
	addHandler("/api/cube/locks", new DocumentationHandler(d1, templateDirectory + "/cube_locks.api"));
	addHandler("/api/cube/rename", new DocumentationHandler(d1, templateDirectory + "/cube_rename.api"));
	addHandler("/api/cube/rollback", new DocumentationHandler(d1, templateDirectory + "/cube_rollback.api"));
	addHandler("/api/cube/rules", new DocumentationHandler(d1, templateDirectory + "/cube_rules.api"));
	addHandler("/api/cube/save", new DocumentationHandler(d1, templateDirectory + "/cube_save.api"));
	addHandler("/api/cube/unload", new DocumentationHandler(d1, templateDirectory + "/cube_unload.api"));

	addHandler("/api/element/append", new DocumentationHandler(d1, templateDirectory + "/element_append.api"));
	addHandler("/api/element/create", new DocumentationHandler(d1, templateDirectory + "/element_create.api"));
	addHandler("/api/element/create_bulk", new DocumentationHandler(d1, templateDirectory + "/element_create_bulk.api"));
	addHandler("/api/element/destroy", new DocumentationHandler(d1, templateDirectory + "/element_delete.api"));
	addHandler("/api/element/destroy_bulk", new DocumentationHandler(d1, templateDirectory + "/element_delete_bulk.api"));
	addHandler("/api/element/info", new DocumentationHandler(d1, templateDirectory + "/element_info.api"));
	addHandler("/api/element/move", new DocumentationHandler(d1, templateDirectory + "/element_move.api"));
	addHandler("/api/element/move_bulk", new DocumentationHandler(d1, templateDirectory + "/element_move_bulk.api"));
	addHandler("/api/element/rename", new DocumentationHandler(d1, templateDirectory + "/element_rename.api"));
	addHandler("/api/element/replace", new DocumentationHandler(d1, templateDirectory + "/element_replace.api"));
	addHandler("/api/element/replace_bulk", new DocumentationHandler(d1, templateDirectory + "/element_replace_bulk.api"));

	addHandler("/api/cell/area", new DocumentationHandler(d1, templateDirectory + "/cell_area.api"));
	addHandler("/api/cell/copy", new DocumentationHandler(d1, templateDirectory + "/cell_copy.api"));
	addHandler("/api/cell/drillthrough", new DocumentationHandler(d1, templateDirectory + "/cell_drillthrough.api"));
	addHandler("/api/cell/export", new DocumentationHandler(d1, templateDirectory + "/cell_export.api"));
	addHandler("/api/cell/goalseek", new DocumentationHandler(d1, templateDirectory + "/cell_goalseek.api"));
	addHandler("/api/cell/replace", new DocumentationHandler(d1, templateDirectory + "/cell_replace.api"));
	addHandler("/api/cell/replace_bulk", new DocumentationHandler(d1, templateDirectory + "/cell_replace_bulk.api"));
	addHandler("/api/cell/value", new DocumentationHandler(d1, templateDirectory + "/cell_value.api"));
	addHandler("/api/cell/values", new DocumentationHandler(d1, templateDirectory + "/cell_values.api"));

	addHandler("/api/event/begin", new DocumentationHandler(d1, templateDirectory + "/event_begin.api"));
	addHandler("/api/event/end", new DocumentationHandler(d1, templateDirectory + "/event_end.api"));

	addHandler("/api/rule/create", new DocumentationHandler(d1, templateDirectory + "/rule_create.api"));
	addHandler("/api/rule/destroy", new DocumentationHandler(d1, templateDirectory + "/rule_destroy.api"));
	addHandler("/api/rule/functions", new DocumentationHandler(d1, templateDirectory + "/rule_functions.api"));
	addHandler("/api/rule/info", new DocumentationHandler(d1, templateDirectory + "/rule_info.api"));
	addHandler("/api/rule/modify", new DocumentationHandler(d1, templateDirectory + "/rule_modify.api"));
	addHandler("/api/rule/parse", new DocumentationHandler(d1, templateDirectory + "/rule_parse.api"));

	addHandler("/api/svs/info", new DocumentationHandler(d1, templateDirectory + "/svs_info.api"));
	addHandler("/api/svs/restart", new DocumentationHandler(d1, templateDirectory + "/svs_restart.api"));

	addHandler("/api/view/calculate", new DocumentationHandler(d1, templateDirectory + "/view_calculate.api"));

	// browse the error codes
	addHandler("/api/error.html", new PaloBrowserHandler(templateDirectory));

	// no browser handler
	addHandler("/browser", new DocumentationHandler(templateDirectory + "/no_browser.tmpl", templateDirectory + "/home.api"));
}

void PaloHttpServer::addBrowserHandlers()
{
	addHandler("/browser", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/cube", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/database", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/dimension", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/element", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/rule", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/analyzer", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/server", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/statistics", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/logfile", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/sessions", new PaloBrowserHandler(templateDirectory));
	addHandler("/browser/jobs", new PaloBrowserHandler(templateDirectory));
}
}
