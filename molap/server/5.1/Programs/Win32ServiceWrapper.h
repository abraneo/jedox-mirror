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
 * \author Vali Nitu, vali@yalos-solutions.com
 * 
 *
 */

#ifndef PROGRAMS_WIN32_SERVICE_WRAPPER_H
#define PROGRAMS_WIN32_SERVICE_WRAPPER_H 1

#include "ServiceUpdateTimer.h"

namespace palo {

//static int startPalo( PaloOptions& options );

class Palo_Win32_Service {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief installs palo as a windows service
	////////////////////////////////////////////////////////////////////////////////
	static void InstallServiceCommand(PaloOptions& options, const string& command) {
		Logger::info << "adding service '" << options.friendlyServiceName << "' (internal '" << options.serviceName << "')" << endl;

		SC_HANDLE schSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

		if (0 == schSCManager) {

			Logger::error << "OpenSCManager failed with " << GetLastError() << endl;
			exit(1);
		}

		SC_HANDLE schService = CreateServiceA(schSCManager, // SCManager database
		        options.serviceName.c_str(), // name of service
		        options.friendlyServiceName.c_str(), // service name to display
		        SERVICE_ALL_ACCESS, // desired access
		        SERVICE_WIN32_OWN_PROCESS, // service type
		        SERVICE_AUTO_START, // start type
		        SERVICE_ERROR_NORMAL, // error control type
		        command.c_str(), // path to service's binary
		        NULL, // no load ordering group
		        NULL, // no tag identifier
		        NULL, // no dependencies
		        NULL, // account (LocalSystem); TODO use LocalService
		        NULL); // password

		CloseServiceHandle(schSCManager);

		if (schService == 0) {
			Logger::error << "CreateServiceA failed with " << GetLastError() << endl;
			exit(1);
		}

		SERVICE_DESCRIPTION description;
		description.lpDescription = (char *)options.service_description.c_str();
		ChangeServiceConfig2A(schService, SERVICE_CONFIG_DESCRIPTION, &description);

		Logger::info << "added service with command line '" << command << "'" << endl;

		CloseServiceHandle(schService);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes palo as a windows service
	////////////////////////////////////////////////////////////////////////////////
	static void DeleteService(PaloOptions& options) {
		Logger::info << "removing service '" << options.friendlyServiceName << "' (internal '" << options.serviceName << "')" << endl;

		SC_HANDLE schSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

		if (0 == schSCManager) {
			Logger::error << "OpenSCManager failed with " << windows_error(GetLastError()) << endl;
			exit(1);
		}

		SC_HANDLE schService = OpenServiceA(schSCManager, // SCManager database
		        options.serviceName.c_str(), // name of service
		        DELETE); // only need DELETE access

		CloseServiceHandle(schSCManager);

		if (0 == schService) {
			Logger::error << "OpenServiceA failed with " << windows_error(GetLastError()) << endl;
			exit(1);
		}

		if (!::DeleteService(schService)) {
			Logger::error << "DeleteService failed with " << windows_error(GetLastError()) << endl;
			exit(1);
		}

		CloseServiceHandle(schService);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief wraps the command line and calls the install service command
	////////////////////////////////////////////////////////////////////////////////
	static void InstallService(PaloOptions& options, const string& data) {
		CHAR path[MAX_PATH];

		if (!GetModuleFileNameA(NULL, path, MAX_PATH)) {
			Logger::error << "GetModuleFileNameA failed" << endl;
			exit(1);
		}

		// get working directory in wd
		CHAR wd[MAX_PATH];
		string working;

		// absolute path
		if ((3 <= data.size() && data.substr(1, 2) == ":\\") || (2 < data.size() && data.substr(0, 2) == "\\\\")) {
			working = "";
		}

		// relative
		else {
			if (getcwd(wd, MAX_PATH) == 0) {
				Logger::error << "getcwd failed" << endl;
				exit(1);
			}

			working += wd;
			working += "\\";
		}

		// build command
		string command;

		command += "\"";
		command += path;
		command += "\"";

		command += " --start-service --service-name \"" + options.serviceName + "\" --data \"" + working + data + "\"";

		// register service
		InstallServiceCommand(options, command);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief service control
	////////////////////////////////////////////////////////////////////////////////
	static void WINAPI service_ctrl(DWORD dwCtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
		Logger::trace << "got service control request " << dwCtrlCode << endl;

		switch (dwCtrlCode) {
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_PRESHUTDOWN:
			StatusUpdater.BeginUpdateForStop();
			initiatePaloStop();
			break;

		case SERVICE_CONTROL_INTERROGATE:
			//MSDN
			//Notifies a service that it should report its current status information to the service control manager. The hService handle must have the SERVICE_INTERROGATE access right.
			//Note that this control is not generally useful as the SCM is aware of the current state of the service.
			//symplify the code by leaving it unimplemented.
			break;

		case 128:
			paloServerSave();
			break;

		default:
			break;
		}
	}

	static void WINAPI service_main(DWORD dwArgc, LPSTR *lpszArgv) {
		Logger::trace << "registering control handler" << endl;

		// register the service ctrl handler,  lpszArgv[0] contains service name
		service_status = RegisterServiceCtrlHandlerExA(lpszArgv[0], (LPHANDLER_FUNCTION_EX) service_ctrl, NULL);

		StatusUpdater.SetControlledService(service_status);
		StatusUpdater.BeginUpdateForStart();

		if (startPalo(*PaloServiceOptions, CallBack)) {
			StatusUpdater.BeginUpdateForStop();
			CallBack();
		}
	}

	static int startPaloService(PaloOptions& options) {
		PaloServiceOptions = &options;

		SERVICE_TABLE_ENTRY ste[] = { {TEXT(""), (LPSERVICE_MAIN_FUNCTION)service_main}, {NULL, NULL}};

		Logger::trace << "starting service control dispatcher" << endl;

		if (!StartServiceCtrlDispatcher(ste)) {
			Logger::error << "StartServiceCtrlDispatcher has failed with " << windows_error(GetLastError()) << endl;
			exit(1);
		}

		return 0;
	}

	static void CallBack() {
		StatusUpdater.CleanupTimer();
	}

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief service status handler
	////////////////////////////////////////////////////////////////////////////////
	static SERVICE_STATUS_HANDLE service_status;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates the status during startup/shutdown
	////////////////////////////////////////////////////////////////////////////////
	static Win32StatusUpdater StatusUpdater;

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief placeholder for the instance pointer
	////////////////////////////////////////////////////////////////////////////////
	static Palo_Win32_Service* instance;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief options for starting palo
	////////////////////////////////////////////////////////////////////////////////
	static PaloOptions* PaloServiceOptions;
};

}
;

#endif
