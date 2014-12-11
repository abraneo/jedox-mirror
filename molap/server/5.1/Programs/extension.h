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

#ifndef PROGRAMS_EXTENSION_H
#define PROGRAMS_EXTENSION_H 1

#include "palo.h"
#include "Programs/PaloHttpInterface.h"
#include "Programs/PaloOptions.h"

namespace palo {
class HttpServer;
class LegacyServer;
class Server;


////////////////////////////////////////////////////////////////////////////////
/// @brief close extension function
////////////////////////////////////////////////////////////////////////////////

typedef void (*CloseExtension_fptr)();
typedef void (*CreateServer_fptr)(const FileName &fileName, PaloOptions &options);

////////////////////////////////////////////////////////////////////////////////
/// @brief information about the server
////////////////////////////////////////////////////////////////////////////////

struct ServerInfo_t {
	const char * revision; // input
	const char * version; // input
	char * modules; // input

#if defined(_MSC_VER)
	HINSTANCE handle; // input (handle to DLL)
#else
	void * handle; // input (handle to shared library)
#endif

	const char * description; // output
	InitHttpInterface_fptr httpInterface; // output
	InitHttpsInterface_fptr httpsInterface; // output
	InitJobAnalyser_fptr jobAnalyser; // output
	CloseExtension_fptr closeExtension; // output
	CreateServer_fptr createServer;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief init function
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#define INIT_EXTENSION(a) extern "C" __declspec(dllexport) InitExtension_fptr InitExtension = &a
#else
#define INIT_EXTENSION(a) InitExtension_fptr InitExtension = &a
#endif

typedef bool (*InitExtension_fptr)(ServerInfo_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks the existance of external modules
////////////////////////////////////////////////////////////////////////////////

vector<ServerInfo_t*> OpenExternalModules(const string& directory);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks the existance of external modules
////////////////////////////////////////////////////////////////////////////////

void CloseExternalModules();

}

#endif
