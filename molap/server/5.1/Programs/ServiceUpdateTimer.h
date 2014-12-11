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
 * \author Radu Ialovoi, Yalos Solutions, Bucharest, Romania
 * 
 *
 */

#ifndef PROGRAMS_SERVICE_UPDATE_TIMER_H
#define PROGRAMS_SERVICE_UPDATE_TIMER_H 1

#include <boost/thread/thread.hpp>

namespace palo {

	DWORD getWindowsMajorVersion() {
		OSVERSIONINFO vi;
		memset(&vi, 0, sizeof(vi));
		vi.dwOSVersionInfoSize = sizeof(vi);
		GetVersionEx(&vi);
		if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			return vi.dwMajorVersion;
		} else {
			return 0;
		}
	}

class Win32StatusUpdater {
private:
	class ServiceUpdateTimer {
	private:
		bool* m_Active;
		DWORD m_WaitHint;
		DWORD m_CheckPoint;
		DWORD m_UpdateCounter;
		SERVICE_STATUS m_ServiceStatus;
		SERVICE_STATUS_HANDLE m_ServiceHandle;

		void UpdateStatus() {
			m_ServiceStatus.dwWaitHint = m_WaitHint;
			m_ServiceStatus.dwCheckPoint = m_CheckPoint;

			SetServiceStatus(m_ServiceHandle, &m_ServiceStatus);

			m_UpdateCounter = m_WaitHint / 1000 - 2;
			m_CheckPoint++;
		}

		void UpdateStatus2Started() {
			if (getWindowsMajorVersion() >= 6) {
				m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PRESHUTDOWN;
			} else {
				m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
			}
			m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
			Logger::info << "Service started successfully." << endl;
			SetServiceStatus(m_ServiceHandle, &m_ServiceStatus);
		}

		void UpdateStatus2eStopped() {
			m_ServiceStatus.dwControlsAccepted = 0;
			m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			Logger::info << "Service stopped successfully." << endl;
			SetServiceStatus(m_ServiceHandle, &m_ServiceStatus);
		}
	public:

		ServiceUpdateTimer(SERVICE_STATUS_HANDLE service_handle, bool start_stop, bool* signal) :
			m_ServiceHandle(service_handle), m_CheckPoint(0), m_WaitHint(1000 * 30), m_Active(signal) {
			m_ServiceStatus.dwControlsAccepted = 0;
			m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
			m_ServiceStatus.dwServiceSpecificExitCode = 0;
			m_ServiceStatus.dwCurrentState = start_stop ? SERVICE_START_PENDING : SERVICE_STOP_PENDING;
			m_ServiceStatus.dwWin32ExitCode = 0;

			m_UpdateCounter = 1;
		}

		void operator()() {
			while (*m_Active) {
				if (0 == --m_UpdateCounter) {
					switch (m_ServiceStatus.dwCurrentState) {
					case SERVICE_START_PENDING:
						Logger::debug << "Waiting for the service to startup..." << endl;
						break;
					case SERVICE_STOP_PENDING:
						Logger::debug << "Waiting for the service to shutdown..." << endl;
						break;
					default:
						break;
					}
					UpdateStatus();
				}
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}

			switch (m_ServiceStatus.dwCurrentState) {
			case SERVICE_START_PENDING:
				UpdateStatus2Started();
				break;
			case SERVICE_STOP_PENDING:
				UpdateStatus2eStopped();
				break;
			default:
				break;
			}
		}
	};

	SERVICE_STATUS_HANDLE m_ServiceHandle;

	boost::thread* timer;
	bool control;

public:
	Win32StatusUpdater() {
		timer = NULL;
	}

	~Win32StatusUpdater() {
		CleanupTimer();
	}

	void SetControlledService(SERVICE_STATUS_HANDLE handle) {
		m_ServiceHandle = handle;
	}

	void BeginUpdateForStart() {
		CleanupTimer();
		control = true;
		timer = new boost::thread(ServiceUpdateTimer(m_ServiceHandle, true, &control));
	}

	void BeginUpdateForStop() {
		CleanupTimer();
		control = true;
		timer = new boost::thread(ServiceUpdateTimer(m_ServiceHandle, false, &control));
	}

	void CleanupTimer() {
		if (NULL != timer) {
			control = false;
			timer->join();
			delete timer;
			timer = NULL;
		}
	}

};
}
;
#endif
