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

#include "HttpsServer/PaloHttpsServer.h"

#include "Olap/Server.h"

#include "Programs/extension.h"

///////////////////
// module extension
///////////////////

#if defined(ENABLE_HTTPS_MODULE)
using namespace palo;

static HttpServer* ConstructHttpsServer(const string& rootfile,
        const string& keyfile,
        const string& password,
        const string& dhfile,
        const string& templateDirectory)
{
	return new PaloHttpsServer(rootfile, keyfile, password, dhfile, templateDirectory);
}

static const char *desc = "palo https interface";

static bool PaloInterface(ServerInfo_t* info)
{

	// check version
	if (strcmp(info->version, Server::getVersion()) == 0) {
		//    Logger::info << "version " << Server::getVersion() << " match in PaloInterface" << endl;
	} else {
		//    Logger::warning << "version " << Server::getVersion() << " mismatch in PaloInterface" << endl;
		return false;
	}

	// check revision
	if (strcmp(info->revision, Server::getRevision()) == 0) {
		//    Logger::info << "revision " << Server::getRevision() << " match in PaloInterface" << endl;
	} else {
		Logger::warning << "revision " << Server::getRevision() << " mismatch in PaloInterface" << endl;
		return false;
	}

	// return info about extension
	info->description = desc;
	info->httpsInterface = &ConstructHttpsServer;

	return true;
}

INIT_EXTENSION(PaloInterface);

#endif
