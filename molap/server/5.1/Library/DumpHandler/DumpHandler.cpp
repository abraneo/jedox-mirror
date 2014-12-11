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
 * \author Florian Schaper, Jedox AG, Freiburg, Germany
 * \author Frieder Hofmann, Jedox AG, Freiburg, Germany
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "DumpHandler.h"

#if defined(WIN32) && !defined(_DEBUG)

#include "Logger/Logger.h"
#include "Olap/Server.h"

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

#include <Windows.h>
#include <DbgHelp.h> // windows specific library that handles minidump creation
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <crtdbg.h>

#include "InputOutput/ZipUtils.h"
#include "InputOutput/zip/zip.h"

namespace palo {

	Mutex DumpHandler::m_lock;
	bool DumpHandler::m_enable = true;
	bool DumpHandler::m_initialized = false;
	string DumpHandler::m_palo_ini;

	class crash_dump_manager : boost::noncopyable
	{
	public:
		/* \brief
		* singleton creation and access
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		static crash_dump_manager& get_instance() {
			static crash_dump_manager instance;
			return instance;
		}

		/* \brief
		* triggers the creation of the crash dump file.
		*
		* will create a minidump file in an predefined folder
		* under an unique file name.
		*
		* \param
		* EXCEPTION_POINTERS * windows exception information of the crashed thread.
		*
		* \param
		* thread_id id of the crashed thread.
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		void generate_crash_dump(EXCEPTION_POINTERS* p, unsigned long thread_id) {
			if (!m_is_armed) {
				return;
			}

			{
				boost::lock_guard<boost::mutex> lock(m_mutex_creating_mutex);
				m_exception_pointers = p;
				m_thread_id = thread_id;
				m_generate_crash_dump = true;
			}

			boost::unique_lock<boost::mutex> lock(m_mutex_finished_creating);
			m_create_crash_dump_condition.notify_one();
			while (!m_finished_crash_dump) {
				m_finish_crash_dump_condition.wait(lock);
			}
		}

		/* \brief
		* sets the crash dump filename.
		*
		* can be passed an format string. %1% will be replaced with an
		* unique id.
		*
		* \param
		* const std::string& format string to be used to create the filename
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		const std::string& set_crash_dump_file(const std::string &format) {
			// format string to create the crash dump file name.
			boost::mt19937 ran;
			ran.seed((uint32_t)time(NULL));
			boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
			m_crash_dump_file = (boost::format(format) % gen()).str();
			return m_crash_dump_file;
		}

		void set_palo_ini(const std::string &paloIniPath) {
			if (paloIniPath != "") {
				m_palo_ini = paloIniPath;
			}
		}

		/* \brief
		* sets the crash report program to be called.
		*
		* can be passed an format string. %1% will be replaced with an string
		* representing the path to the crash dump file.
		*
		* \param
		* const std::string& format string to be used to create the filename
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		void set_crash_report_system_call(const std::string& format) {
			m_crash_report_call_format = format;
		}

		void set_palo_binary_path(const std::string &path) {
			m_paloBinaryFolder = path;
		}

		/* \brief
		* enable or disable the call the the crash_report software.
		*
		* if set to true (default) will call the crash report handler
		* after minidump file creation
		*
		* \param
		* bool true = creash_report software should be called
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		void enable_crash_reporting(bool enable) {
			m_execute_crash_reporting = enable;
		}

		/*!
		* \brief
		* disarm the crash handler
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		void disarm() {
			m_is_armed = false;
			m_execute_crash_reporting = false;
			m_create_crash_dump_condition.notify_all();
			m_finish_crash_dump_condition.notify_all();
			SetUnhandledExceptionFilter(NULL);
		}

	private:
		/* \brief
		* constructor.
		*
		* \remarks
		* will start a new thread to have an (hopefully) intact stack
		* by the time we have to create a dump file.
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		crash_dump_manager() : m_generate_crash_dump(false), m_finished_crash_dump(false), m_execute_crash_reporting(true), m_crash_dump_file("crash"), m_palo_ini(""), m_is_armed(true) {
				// load the dbghelp.dll from our current directory since we
				// can not trust the system DLL search path to come up with
				// the correct version.
				HMODULE module_handle = LoadLibraryA("dbghelp.dll");

				if (module_handle) {
					m_writedump_callback = reinterpret_cast<minidump_write_dump_type>(GetProcAddress(module_handle, "MiniDumpWriteDump"));
					boost::thread::thread(boost::bind(&crash_dump_manager::wait_for_crash, this));
				} else {
					Logger::error << "could not load required library dbghelp.dll!" << endl;
				}
		}

		/* \brief
		* creates the on demand one-time crash dump of the process.
		*
		* creates the crash dump and tries to start an crash handler
		* external process for further processing and user information.
		*
		* \author
		* Florian Schaper <florian.schaper@jedox.com>
		*/
		void wait_for_crash() {
			// wait for the method generate_crash_dump to be called.
			boost::unique_lock<boost::mutex> lock( m_mutex_creating_mutex );
			while (! m_generate_crash_dump) {
				m_create_crash_dump_condition.wait(lock);
			}

			if (m_is_armed && m_writedump_callback) {
				// create file at the path dictated
				HANDLE file_handle = CreateFileA((m_crash_dump_file + ".dmp").c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				// if we could not open the file there is nothing we can do
				if ((file_handle != NULL) && (file_handle != INVALID_HANDLE_VALUE)) {

					// create the minidump by calling the DbgHelp library
					MINIDUMP_EXCEPTION_INFORMATION minidump_information;
					minidump_information.ThreadId = m_thread_id;
					minidump_information.ExceptionPointers = m_exception_pointers;
					minidump_information.ClientPointers = FALSE;

					MINIDUMP_TYPE minidump_type = MiniDumpWithProcessThreadData;

					int successful = (*m_writedump_callback)(GetCurrentProcess(), GetCurrentProcessId(), file_handle, minidump_type, (m_exception_pointers != 0) ? &minidump_information : 0, 0, 0);

					// close the minidump file
					CloseHandle(file_handle);

					try {
						PaloSession::requestRecord(m_crash_dump_file, "txt");
					} catch (...) {
						Logger::error << "Can't create crashed requests file." << endl;
					}

					zipFile z = zipOpen((m_crash_dump_file + ".zip").c_str(), APPEND_STATUS_CREATE);
					if (z) {
						ZipUtils::addToZip(z, m_crash_dump_file + ".dmp");
						ZipUtils::addToZip(z, m_crash_dump_file + ".txt");
						ZipUtils::addToZip(z, m_palo_ini);

						int errclose = zipClose(z, NULL);
						if (errclose == ZIP_OK) {
							FileName fnDMP("", m_crash_dump_file, "dmp");
							FileUtils::remove(fnDMP);
							FileName fnTXT("", m_crash_dump_file, "txt");
							FileUtils::remove(fnTXT);
						} else {
							Logger::error << "Can't close ZIP file." << endl;
						}

					} else {
						Logger::error << "Can't open ZIP file." << endl;
					}

					Logger::error << "Server has encountered a problem and needs to close. Crash dump with detailed information saved to Data directory: " << m_crash_dump_file << endl;
				}

				// try to call the user defined crash report handler
				if (m_execute_crash_reporting && !m_crash_report_call_format.empty() && !m_paloBinaryFolder.empty()) {
					// commented, crash_report.exe not used (no sense to start it from win service)
					//chdir(m_paloBinaryFolder.c_str());
					//std::system((boost::format(m_crash_report_call_format) % m_crash_dump_file).str().c_str());
				}

				// signal the end of minidump creation
				{
					boost::lock_guard<boost::mutex> lock(m_mutex_finished_creating);
					m_finished_crash_dump = true;
				}
			}
			m_finish_crash_dump_condition.notify_all();
		}

	private:
		// mutex in place while we wait for the creation trigger.
		boost::condition_variable m_create_crash_dump_condition;
		boost::mutex m_mutex_creating_mutex;
		bool m_generate_crash_dump;

		// mutex in place while we wait for the dump to complete.
		boost::condition_variable m_finish_crash_dump_condition;
		boost::mutex m_mutex_finished_creating;
		bool m_finished_crash_dump;

		bool m_execute_crash_reporting;

		// exception information that needs to be passed from the
		// crashing thread.
		EXCEPTION_POINTERS* m_exception_pointers;
		unsigned long m_thread_id;

		// formating rules for crash reporter call and
		// dump file creation.
		std::string m_crash_dump_file;
		std::string m_crash_report_call_format;
		std::string m_paloBinaryFolder;
		std::string m_palo_ini;

		// minidump function
		typedef BOOL (WINAPI* minidump_write_dump_type)(__in HANDLE, __in DWORD, __in HANDLE, __in MINIDUMP_TYPE, __in_opt PMINIDUMP_EXCEPTION_INFORMATION, __in_opt PMINIDUMP_USER_STREAM_INFORMATION, __in_opt PMINIDUMP_CALLBACK_INFORMATION);
		minidump_write_dump_type m_writedump_callback;
		// create crash_dumps
		bool m_is_armed;
	};

	/* \brief
	* callback routine for the unhandled exception handler
	*
	* will trigger the call of the crash_dump_manager in the event of an unhandled
	* exception under windows.
	*
	* \author
	* Florian Schaper <florian.schaper@jedox.com>
	*/
	static long __stdcall create_crash_dump(EXCEPTION_POINTERS* exception_pointers)
	{
		// we have an 'fire once' exception handler if we have been called once
		// we pass any further on to any other registered handlers on the chain.
		static bool has_been_called = false;
		if ( has_been_called ) {
			// pass the exception on to the next handler
			return EXCEPTION_CONTINUE_SEARCH;
		}
		has_been_called = true;

		if (exception_pointers->ExceptionRecord->ExceptionCode == 0xE06D7363) {
			/*if( process_dispatcher::instance().is_above_memory_quota_threshold() ) {
			printf( "Memory quota exceeded, terminating process... \n" );
			return EXCEPTION_EXECUTE_HANDLER;
			}*/
		}

		// fire off crash dump creation and wait for the completion
		crash_dump_manager::get_instance().generate_crash_dump(exception_pointers, GetCurrentThreadId());
		// we handled the exception, no need for other handlers.
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// Patch for SetUnhandledExceptionFilter xor eax,eax; ret 0x4;
	const BYTE ex_handler_patch_bytes[5] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };

	// Original bytes at the beginning of SetUnhandledExceptionFilter
	BYTE ex_handler_original_bytes[5] = {0};

	/* \brief
	* write_memory
	* writes a memory by preserving protection bits
	*
	* \author
	* Radu Racariu <radu@yalos-solutions.com>
	* http://www.debuginfo.com/articles/debugfilters.html#enforce
	*/
	bool write_memory(BYTE* target, const BYTE* source, DWORD size)
	{
		DWORD error_code = 0;

		// Validate parameters
		if ( NULL == target ) {
			_ASSERTE(!_T("Target address is null."));
			return false;
		}

		if ( NULL == source ) {
			_ASSERTE(!_T("Source address is null."));
			return false;
		}

		if ( 0 == size ) {
			_ASSERTE(!_T("Source size is null."));
			return false;
		}

		if ( IsBadReadPtr( source, size ) ) {
			_ASSERTE(!_T("Source is unreadable."));
			return false;
		}

		// Modify protection attributes of the target memory page
		DWORD original_protection_bits = 0;

		if (!VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &original_protection_bits)) {
			error_code = GetLastError();
			_ASSERTE(!_T("VirtualProtect() failed."));
			return false;
		}

		// Write memory
		memcpy(target, source, size);

		// Restore memory protection attributes of the target memory page
		DWORD temp = 0;

		if (!VirtualProtect(target, size, original_protection_bits, &temp)) {
			error_code = GetLastError();
			_ASSERTE(!_T("VirtualProtect() failed."));
			return false;
		}

		// Success
		return true;
	}

	/* \brief
	* enforce_exception_filter overrides runtime installed functions
	*
	* If do_enforce is "true", the function overwrites the beginning of
	* SetUnhandledExceptionFilter function with a patch which rejects
	* all subsequent attempts to register a filter.
	* If do_enforce is "false", the original functionality of
	* SetUnhandledExceptionFilter is restored.
	*
	* \author
	* Radu Racariu <radu@yalos-solutions.com>
	* http://www.debuginfo.com/articles/debugfilters.html#enforce
	*/
	bool enforce_exception_filter(bool do_enforce)
	{
		DWORD error_code = 0;

		// Obtain the address of SetUnhandledExceptionFilter
		HMODULE dll_handle = GetModuleHandle(_T("kernel32.dll"));

		if (NULL == dll_handle) {
			error_code = GetLastError();
			_ASSERTE(!_T("GetModuleHandle(kernel32.dll) failed."));
			return false;
		}

		BYTE* callback_handle = (BYTE*)GetProcAddress(dll_handle, "SetUnhandledExceptionFilter");

		if (NULL == callback_handle) {
			error_code = GetLastError();
			_ASSERTE(!_T("GetProcAddress(SetUnhandledExceptionFilter) failed."));
			return false;
		}

		if (IsBadReadPtr(callback_handle, sizeof(ex_handler_original_bytes))) {
			_ASSERTE(!_T("Target is unreadable."));
			return false;
		}

		if (do_enforce) {
			// Save the original contents of SetUnhandledExceptionFilter
			memcpy(ex_handler_original_bytes, callback_handle, sizeof(ex_handler_original_bytes));

			// Patch SetUnhandledExceptionFilter
			if (!write_memory(callback_handle, ex_handler_patch_bytes, sizeof(ex_handler_patch_bytes))) {
				return false;
			}

		} else {
			// Restore the original behavior of SetUnhandledExceptionFilter
			if (!write_memory(callback_handle, ex_handler_original_bytes, sizeof(ex_handler_original_bytes))) {
				return false;
			}
		}
		// Success
		return true;
	}

	/* \brief
	* register crash handler with the system.
	*
	* \author
	* Florian Schaper <florian.schaper@jedox.com>
	* Radu Racariu <radu@yalos-solutions.com>
	*/
	void DumpHandler::register_handler(string paloIniPath)
	{
		WriteLocker write_locker(&m_lock);

		if (!m_enable) {
			return;
		}

		if (!m_initialized) {
			m_initialized = true;

			char buff[MAX_PATH];
			string exePath;
			if (GetModuleFileName(NULL, buff, MAX_PATH)) {
				string::size_type pos = string(buff).find_last_of("\\/");
				if (pos != string::npos) {
					exePath = string(buff).substr(0, pos) + '\\';
				}
			}

			// initialize the singleton object if not already done so.
			crash_dump_manager& cdm = crash_dump_manager::get_instance();
			// configure the report handling in the event of a crash
			// - file to be created
			// - program to be executed in the event of an crash.
			cdm.set_palo_binary_path(exePath);
			cdm.set_crash_report_system_call("crash_report.exe \"%1%\" \"" + Server::getVersionRevisionDots() + "\"");
			cdm.set_crash_dump_file("crash_%1%");
			m_palo_ini = paloIniPath;
			cdm.set_palo_ini(paloIniPath);
		}

		SetUnhandledExceptionFilter(create_crash_dump);
	}

	void DumpHandler::enable_crash_reporting(bool enable)
	{
		// initialize the singleton object if not already done so.
		crash_dump_manager& cdm = crash_dump_manager::get_instance();
		cdm.enable_crash_reporting(enable);
	}

	void DumpHandler::disarm()
	{
		// initialize the singleton object if not already done so.
		crash_dump_manager& cdm = crash_dump_manager::get_instance();
		cdm.disarm();
//	not necessary if disarming on shutdown, WinXP problem 12221, 64bit not used at all
//#ifndef _WIN64
//		// Load msvcrt.dll to restore back its own filter
//		// for unhandled exceptions (_CxxUnhandledExceptionFilter)
//		HMODULE dll_handle = LoadLibrary(_T("msvcrt.dll"));
//
//		if (NULL == dll_handle) {
//			Logger::error << "LoadLibrary(msvcrt.dll) failed. Error: %u" << GetLastError() << endl;
//		}
//		// Restore the functionality of SetUnhandledExceptionFilter
//		if (!enforce_exception_filter(false)) {
//			Logger::error << "EnforceFilter(false) failed." << endl;
//			return;
//		}
//		// Free msvcrt.dll
//		if (NULL != dll_handle)
//			if (!FreeLibrary(dll_handle)) {
//				Logger::error << "FreeLibrary(msvcrt.dll) failed. Error: %u" << GetLastError() << endl;
//			}
//#endif		
	}

}

#else

namespace palo {

Mutex DumpHandler::m_lock;
bool DumpHandler::m_enable = false;
bool DumpHandler::m_initialized = false;
string DumpHandler::m_palo_ini;

void DumpHandler::register_handler(string paloIniPath)
{
	if (!m_initialized) {
		m_initialized = true;
		m_palo_ini = paloIniPath;
	}
}

void DumpHandler::enable_crash_reporting(bool enable) {}
void DumpHandler::disarm() {}
}

#endif
