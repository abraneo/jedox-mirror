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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Marek Pikulski, marek@pikulski.net
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "palo.h"

#include "Logger/Logger.h"
#include "Olap/Server.h"
#include "PaloDispatcher/PaloJobAnalyser.h"

#include "MTPaloInterface.h"
#include "PaloLoader.h"
#include "PaloOptions.h"
#include "AutosaveTimer.h"
#include "Worker/WorkersCreator.h"
#include "InputOutput/ZipUtils.h"

#include "extension.h"

#ifdef ENABLE_GPU_SERVER
#include "Programs/GpuIdleWorker.h"
#endif

typedef void (*callback)();
static int startPalo(palo::PaloOptions& options, callback sink);
static void initiatePaloStop();
#if defined(_MSC_VER)
static void paloServerSave();
#endif

#if defined(_MSC_VER)
#include "Win32ServiceWrapper.h"
#include <signal.h>
#endif

#include "DumpHandler/DumpHandler.h"

using namespace std;
using namespace palo;

////////////////////////////////////////////////////////////////////////////////
/// @brief the one and only interface
////////////////////////////////////////////////////////////////////////////////
PaloHttpInterface* iface = NULL;

////////////////////////////////////////////////////////////////////////////////
/// @ctrl-c handlers
////////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)

SERVICE_STATUS_HANDLE Palo_Win32_Service::service_status = 0;
PaloOptions* Palo_Win32_Service::PaloServiceOptions;
Win32StatusUpdater Palo_Win32_Service::StatusUpdater;

BOOL WINAPI signalHandler(DWORD dwCtrlType)
{
	Logger::trace << "signal handler: " << dwCtrlType << endl;
	initiatePaloStop();
	return TRUE;
}

void WINAPIV signalHandlerSVS(int type)
{
	Logger::trace << "signal handler SVS: " << type << endl;
	if (Palo_Win32_Service::service_status) {
		Palo_Win32_Service::StatusUpdater.BeginUpdateForStop();
	}
	initiatePaloStop();
}

#else

#ifndef _DEBUG
static void linuxCrashHandler(int signal)
{
	time_t tt = time(0);
	struct tm *t = localtime(&tt);
	int max = 32;
	char *s = new char[max];
	strftime(s, max, "%Y-%m-%d-%H-%M-%S", t);
	string fileName(s);
	delete[] s;

	try {
		PaloSession::requestRecord(fileName, "txt");
	} catch (...) {
		Logger::error << "Can't create crashed requests file." << endl;
	}

	zipFile z = zipOpen((fileName + ".zip").c_str(), APPEND_STATUS_CREATE);
	if (z) {
		ZipUtils::addToZip(z, fileName + ".txt");
		ZipUtils::addToZip(z, DumpHandler::m_palo_ini);

		int errclose = zipClose(z, NULL);
		if (errclose == ZIP_OK) {
			FileName fnTXT("", fileName, "txt");
			FileUtils::remove(fnTXT);
		} else {
			Logger::error << "Can't close ZIP file." << endl;
		}
	} else {
		Logger::error << "Can't open ZIP file." << endl;
	}

	Logger::error << "Crash detected, server shutdown, report saved into " << fileName << ".zip" << endl;
	exit(1);
}
#endif

static void signalHandler(int signal)
{
	initiatePaloStop();
	Logger::info << "signaled to close" << endl;
}

#endif

static void initiatePaloStop()
{
	if (NULL != iface) {
		iface->handleShutdown();
	}
}

#if defined(_MSC_VER)
static void paloServerSave()
{
	bool bOK = false;
	try {
		Context *con = Context::getContext();
		if (con) {
			PServer s = con->getServer();
			if (s) {
				s->commitAndSave();
				Context::reset();
				bOK = true;
			}
		}
	} catch (ErrorException &e) {
		Logger::error << e.getMessage() << endl;
	}
	if (!bOK) {
		Logger::error << "Save failed." << endl;
	} else {
		Logger::info << "Save successfull." << endl;
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief starts the server and blocks on select
////////////////////////////////////////////////////////////////////////////////
static int startPalo(PaloOptions& options, callback sink)
{
	int result = 0;
	JobAnalyser* analyser = NULL;
	CreateServer_fptr createServer = 0;

	DumpHandler::register_handler(options.initFile);

	FileName serverFileName(options.dataDirectory, "palo", "csv");
	try {
#if defined(ENABLE_HTTPS) && ! defined(ENABLE_HTTPS_MODULE)
		Logger::info << "HTTPS support built in" << endl;
#endif
		// check for external modules
		vector<ServerInfo_t*> extensions = palo::OpenExternalModules(options.extensionsDirectory);
		for (vector<ServerInfo_t*>::iterator i = extensions.begin(); i != extensions.end(); ++i) {
			ServerInfo_t * info = *i;

			if (NULL != info->httpInterface) {
				options.externalHttpInterface = info->httpInterface;
			}

			if (NULL != info->httpsInterface) {
				options.externalHttpsInterface = info->httpsInterface;
			}

			if (NULL != info->jobAnalyser) {
				options.externalJobAnalyser = info->jobAnalyser;
			}
			if (NULL != info->createServer) {
				createServer = info->createServer;
			}
 		}

		// create server
		if (createServer) {
			createServer(serverFileName, options);
		} else {
			Server::create(serverFileName);
		}

		// update global settings
		options.updateGlobals();

		// activate gpu engine
		if (options.enableGpu) {
			PServer server = Context::getContext()->getServerCopy();
			options.enableGpu = server->activateGpuEngine(options.device_ids);
			server->commit();
			Context::reset();
		}

		// create job analyser
		if (options.externalJobAnalyser) {
			analyser = (*options.externalJobAnalyser)();
		} else {
			analyser = new PaloJobAnalyser();
		}

		if (NULL == analyser) {
			Logger::error << "cannot construct job analyser" << endl;
			return 1;
		}

		{
			// load server from disk
			PaloLoader loader(options.autoCommit, options.dataDirectory);

			loader.load(options.autoLoadDb, options.autoAddDb);
			loader.finalize(options.useFakeSession);
		}

		Logger::info << "user login is required" << endl;

		// create http interface
		iface = new MTPaloHttpInterface(&options, analyser);

		iface->addServers();

		//if ( NULL != sink ) { sink(); }//give a chance to the caller to update state

		AutosaveTimer autosave_timer(options.autosave_definition, iface);

#ifdef ENABLE_GPU_SERVER
		GpuIdleWorker idleWorker(iface);
#endif

		WorkersCreator creator(sink);

		if (!iface->servers.empty()) {
			Logger::info << "Starting to listen..." << endl;
#ifdef ENABLE_TEST_MODE
            cout << "Starting to listen..." << endl;
#endif
		}

		//block on run
		iface->run();

		//starting the shutdown sequence
#ifdef ENABLE_GOOGLE_CPU_PROFILER
	ProfilerStop();
#endif
#ifdef ENABLE_GPU_SERVER
		idleWorker.stopWorker();
#endif
		Logger::info << "sockets closed. shutting down continues..." << endl;
		autosave_timer.stop();
		if (options.autoCommit) {
			PServer server = Context::getContext()->getServer();
			server->commitAndSave();
			Context::reset();
		}

		if (NULL != sink) {
			sink(); //give a chance to the caller to update state
		}
	} catch (const ErrorException& e) {
		Logger::error << "exception '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
		result = 1;
	} catch (std::bad_alloc&) {
		Logger::error << "running out of memory, please reduce the number of loaded databases or cache sizes" << endl;
		result = 1;
	} catch (...) {
		Logger::error << "unexpected error occurred, please check if the system is not running out of memory" << endl;
		result = 1;
	}

	if (NULL != iface) {
		delete iface;
	}
	if (NULL != analyser) {
		delete analyser;
	}

	Context::reset();
	Server::destroy();
	Server::physicalRenamesNoLock(serverFileName);
	palo::CloseExternalModules();

	Logger::info << "finished" << endl;

#if defined(WIN32) && !defined(_DEBUG)
	DumpHandler::disarm();
#endif
	return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief runs palo
////////////////////////////////////////////////////////////////////////////////
static int runPalo(int argc, char* argv[])
{
	// initialize random module
#if defined(WIN32)
	srand((int)time(NULL) + (int)GetCurrentThreadId());
#else
	srand((int)time(NULL) + (int)pthread_self());
#endif

	UTF8Comparer::setDefault();

	// initialize logger
	Logger::setLogLevel("error");

	// parse options
	PaloOptions options(argc, argv);

	try {
		// handle command line options and init file
		options.parseOptions();

		if (options.showUsage) {
			options.usage();
		}

		if (options.showVersion) {
			options.version();
		}

		// change into data directory
		if (options.changeDirectory) {
			if (0 != chdir(options.dataDirectory.c_str()))

#if !defined(_MSC_VER)
				// on a case sensitive FS also try lower & upper case dir names
				if (0 != chdir(StringUtils::tolower(options.dataDirectory).c_str()))
					if (0 != chdir(StringUtils::toupper(options.dataDirectory).c_str())) {
#else
						{
#endif
						Logger::error << "cannot change into directory '" << options.dataDirectory << "'" << endl;
						return 1;
					}

			Logger::info << "changed into directory '" << options.dataDirectory << "'" << endl;

			options.dataDirectory = ".";
		} else {
			if (!options.initFile.empty()) {
				if (options.initFile[0] != '/' && options.initFile[0] != '\\') {
					options.initFile = options.dataDirectory + "/" + options.initFile;
				}
			}
		}

		DirLock dirlock(options.dataDirectory);

		// load init file
		if (options.useInitFile && !options.initFile.empty()) {
			options.parseOptions(options.initFile);
		}

		options.checkOptions();

		Logger::setLogFile(options.logFile, options.outputToStdout);
		Logger::setLogLevel(options.logLevel);

		// log version number
		Logger::info << "starting Jedox OLAP " << Server::getVersionRevision() << endl;

		if (options.encryptionType == ENC_NONE && !options.httpsPorts.empty()) {
			Logger::info << "Encryption type 'NONE', but https ports given. Https will be disabled." << endl;
			options.httpsPorts.clear();
		}

#if defined(_MSC_VER)
		signal(SIGTERM, &signalHandlerSVS);

		// start as service or start as normal program
		if (options.startAsService) {
			return Palo_Win32_Service::startPaloService(options);
		} else {
			SetConsoleCtrlHandler(&signalHandler, TRUE);
			return startPalo(options, NULL);
		}
#else
		return startPalo(options, NULL);
#endif
	} catch (const ErrorException& e) {
		Logger::error << "exception '" << e.getMessage() << "' (" << e.getDetails() << ")" << endl;
		return 1;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// @brief main
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)

extern "C" SERVER_FUNC int PaloMain(int argc, char * argv[])
{
	int ret_code = 0;
	_setmaxstdio(2048); // this is a hard limit in windows

	if (argc >= 3 && strcmp(argv[argc-2], "install_srv") == 0) {
		PaloOptions options(argc - 2, argv);
		options.parseOptions();

		Palo_Win32_Service::InstallService(options, argv[argc-1]);
	} else if (argc >= 2 && strcmp(argv[argc-1], "delete_srv") == 0) {
		PaloOptions options(argc - 1, argv);
		options.parseOptions();

		Palo_Win32_Service::DeleteService(options);
	} else {
		WSADATA wsaData;
		WSAStartup(MAKEWORD(1, 1), &wsaData);

		ret_code = runPalo(argc, argv);

		WSACleanup();
	}

	return ret_code;
}

#else

int main(int argc, char * argv[])
{
	signal(SIGABRT, &signalHandler);
	signal(SIGTERM, &signalHandler);
	signal(SIGQUIT, &signalHandler);
	signal(SIGINT, &signalHandler);
#ifndef _DEBUG
	signal(SIGSEGV, &linuxCrashHandler);
#endif

	return runPalo(argc, argv);
}

#endif
