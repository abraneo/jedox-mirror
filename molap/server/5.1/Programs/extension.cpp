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

#include "extension.h"

#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>
#endif

#include "Collections/StringUtils.h"
#include "InputOutput/FileUtils.h"
#include "Logger/Logger.h"
#include "Olap/Server.h"

namespace palo {
using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// @brief suffix for extension files
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
static const string ExtensionSuffix = ".dll";
#else
static const string ExtensionSuffix = ".palo.so";
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief try to load an external module
////////////////////////////////////////////////////////////////////////////////

static ServerInfo_t * openExternalModule(const string& path, const string& modules)
{

#if defined(_MSC_VER)

	// try to open library
	HINSTANCE handle = LoadLibrary(path.c_str());

	if (handle == 0) {
		Logger::debug << "cannot open extension file '" << path << "'" << endl;
		Logger::debug << windows_error(GetLastError()) << endl;
		return 0;
	}

	Logger::debug << "open external module '" << path << "'" << endl;

	// try to find init function pointer
	void * init = (void*) GetProcAddress(handle, "InitExtension");

	if (init == 0) {
		Logger::warning << "cannot find 'InitExtension' in '" << path << "'" << endl;
		Logger::info << windows_error(GetLastError()) << endl;
		return 0;
	}

	Logger::debug << "found 'InitExtension' in '" << path << "'" << endl;

#else

	// try to open library
	void * handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);

	if (handle == 0) {
		Logger::debug << "cannot open extension file '" << path << "'" << endl;
		Logger::debug << dlerror() << endl;
		return 0;
	}

	Logger::debug << "open external module '" << path << "'" << endl;

	// try to find init function pointer
	void * init = dlsym(handle, "InitExtension");

	if (init == 0) {
		Logger::warning << "cannot find 'InitExtension' in '" << path << "'" << endl;
		return 0;
	}

	Logger::debug << "found 'InitExtension' in '" << path << "'" << endl;

#endif

	// call init function
	InitExtension_fptr func = (InitExtension_fptr) * (void**)init;

	if (func == 0) {
		Logger::warning << "'InitExtension' is empty in '" << path << "'" << endl;
		return 0;
	}

	ServerInfo_t * info = new ServerInfo_t;
	memset(info, 0, sizeof(ServerInfo_t));

	info->revision = Server::getRevision();
	info->version = Server::getVersion();
	info->modules = ::strdup(modules.c_str());
	info->handle = handle;

	bool ok = func(info);

	if (ok) {
		Logger::info << "using extension '" << info->description << "'" << endl;
	} else {
#if defined(_MSC_VER)
#else
		dlclose(handle);
#endif

		delete info;
		Logger::debug << "cannot use extension '" << path << "'" << endl;
		return 0;
	}

	if (info->httpInterface != 0) {
		Logger::debug << "extensions defines an http interface" << endl;
	}

	if (info->httpsInterface != 0) {
		Logger::debug << "extensions defines an https interface" << endl;
	}

	if (info->jobAnalyser != 0) {
		Logger::debug << "extensions defines a job analyser" << endl;
	}

	return info;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief vector of external modules
////////////////////////////////////////////////////////////////////////////////

vector<ServerInfo_t*> ExternalModules;

////////////////////////////////////////////////////////////////////////////////
/// @brief check external modules
////////////////////////////////////////////////////////////////////////////////

vector<ServerInfo_t*> OpenExternalModules(const string& directory)
{
	ServerInfo_t * info = 0;

	// check extensions files
	vector<string> files = FileUtils::listFiles(directory);

	for (vector<string>::iterator i = files.begin(); i != files.end(); ++i) {
		string file = *i;

		if (!StringUtils::isSuffix(file, ExtensionSuffix)) {
			continue;
		}

		info = openExternalModule(directory + "/" + file, directory);

		if (info != 0) {
			ExternalModules.push_back(info);
		}
	}

	return ExternalModules;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief close external modules
////////////////////////////////////////////////////////////////////////////////

void CloseExternalModules()
{
	for (vector<ServerInfo_t*>::iterator iter = ExternalModules.begin(); iter != ExternalModules.end(); ++iter) {
		ServerInfo_t* info = *iter;

		if (info->closeExtension != 0) {
			Logger::debug << "closing extension '" << info->description << "'" << endl;
			info->closeExtension();
		}
	}

#if defined(_MSC_VER)
#else
	for (vector<ServerInfo_t*>::iterator iter = ExternalModules.begin(); iter != ExternalModules.end(); ++iter) {
		ServerInfo_t* info = *iter;
		Logger::debug << "unloading extension '" << info->description << "'" << endl;
		dlclose(info->handle);
	}
#endif
	for (vector<ServerInfo_t*>::iterator iter = ExternalModules.begin(); iter != ExternalModules.end(); ++iter) {
		ServerInfo_t* info = *iter;
		free(info->modules);
		delete info;
	}
}
}

