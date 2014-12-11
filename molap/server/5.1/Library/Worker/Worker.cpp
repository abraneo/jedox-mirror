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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Worker/Worker.h"

#ifdef USE_POLL
#include <poll.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined (HAVE_SIGNAL_H) || defined (_MSC_VER)
#include <signal.h>
#endif

#include "Logger/Logger.h"
#include "Thread/WriteLocker.h"
#include "Olap/PaloSession.h"
#include "Exceptions/WorkerException.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// global variables
// /////////////////////////////////////////////////////////////////////////////

string Worker::executable = "";

vector<string> Worker::arguments;

int Worker::maxFailures = 5;

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)

Worker::Worker(const string& session)
	: status(WORKER_NOT_RUNNING), numFailures(0), session(session)
{

	commandLine = executable;

	for (vector<string>::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
		commandLine += " " + *i;
	}

	readPosition = 0;

	started = false;
}

#else

Worker::Worker(const string& session) :
	status(WORKER_NOT_RUNNING), numFailures(0), session(session)
{

	args = new char*[arguments.size() + 2];

	args[0] = new char[executable.size() + 1];
	strcpy(args[0], executable.c_str());
	int count = 1;

	for (vector<string>::const_iterator i = arguments.begin(); i != arguments.end(); ++i) {
		args[count] = new char[(*i).size() + 1];
		strcpy(args[count], (*i).c_str());
		count++;
	}

	args[count] = 0;

	readPosition = 0;

	started = false;
}

#endif

Worker::~Worker()
{
	terminateProcess();

#if ! defined(_MSC_VER)
	delete args;
#endif
}

// /////////////////////////////////////////////////////////////////////////////
// getters and setters
// /////////////////////////////////////////////////////////////////////////////

void Worker::setExecutable(const string& executable)
{
	Worker::executable = executable;
}

void Worker::setArguments(const vector<string>& arguments)
{
	Worker::arguments = arguments;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface
// /////////////////////////////////////////////////////////////////////////////

bool Worker::isErrorStatus(const vector<string> &result)
{
	return (result.size() >= 1 && result[0].substr(0, 6) == "ERROR;");
}

bool Worker::isExceptionStatus(const vector<string> &result)
{
	return (result.size() >= 1 && result[0].substr(0, 10) == "EXCEPTION;");
}


bool Worker::startint()
{

	WriteLocker locker(&mutex);

	if (status == WORKER_RUNNING) {
		return true;
	}

	bool ok = startProcess();

	if (!ok) {
		return false;
	}

	ok = sendSession();

	if (!ok) {
		terminateProcess();
		return false;
	}

	started = true;

	return true;
}

ResultStatus Worker::execute(const string& line, vector<string>& result)
{
	return execute(line, result, 0);
}

ResultStatus Worker::execute(const string& line, vector<string>& result, time_t timeout)
{
	WriteLocker locker(&mutex);

	result.clear();

	if (status != WORKER_RUNNING) {
		return RESULT_FAILED;
	}

	Context::getContext()->check();

	if (numFailures >= maxFailures) {
		// SVS failed to execute the required command, has to be recovered manually by administrator
		throw WorkerException("SVS script error, can't recover, contact administrator.", false);
	}

	boost::shared_ptr<PaloSession> s = PaloSession::findSession(session, false);
	s->setWorkerContext(Context::getContext());

	while (true) {
		bool ok = sendLine(line, timeout);

		if (!ok) {
			if (timeout == 0) {
				ok = restart();
			}

			if (!ok) {
				s->setWorkerContext(0);
				return RESULT_FAILED;
			}

			continue;
		}

		ok = readResult(timeout);

		if (!ok) {
			if (timeout == 0) {
				ok = restart();
			}

			if (!ok) {
				s->setWorkerContext(0);
				return RESULT_FAILED;
			}

			continue;
		}

		result = this->result;

		s->setWorkerContext(0);
		return RESULT_OK;
	}
}

void Worker::terminate(const string& line, time_t timeout)
{
	WriteLocker locker(&mutex);

	if (status != WORKER_RUNNING) {
		return;
	}

	bool ok = sendLine(line, timeout);

	if (ok) {
		ok = readResult(timeout);
	}

	terminateProcess();
}

void Worker::terminate(bool restart)
{
	if (restart) {
		terminate("RESTART", WORKER_TIMEOUT_MSEC);
	} else {
		terminate("TERMINATE", WORKER_TIMEOUT_MSEC);
	}
}

void Worker::releaseSession()
{
	boost::shared_ptr<PaloSession> s = PaloSession::findSession(session, false);
	PaloSession::deleteSession(s, true);
}

// /////////////////////////////////////////////////////////////////////////////
// starting and stopping worker process
// /////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)

bool Worker::startProcess()
{
	readPosition = 0;
	readBuffer.clear();

	HANDLE hChildStdinRd, hChildStdoutWr;

	bool fSuccess = createPipes(hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr);

	if (! fSuccess) {
		status = WORKER_NOT_RUNNING;
		return false;
	}

	// now create the child process.
	fSuccess = startProcess(commandLine, hChildStdinRd, hChildStdoutWr);

	if (! fSuccess) {
		status = WORKER_NOT_RUNNING;

		CloseHandle(hChildStdoutRd);
		CloseHandle(hChildStdoutWr);
		CloseHandle(hChildStdinRd);
		CloseHandle(hChildStdinWr);
		CloseHandle(processHandle);

		return false;
	}

	CloseHandle(hChildStdinRd);
	CloseHandle(hChildStdoutWr);

	status = WORKER_RUNNING;

	return true;
}

#else

bool Worker::startProcess()
{
	readPosition = 0;
	readBuffer.clear();

	int pipe_server_to_child[2];
	int pipe_child_to_server[2];

	bool ok = createPipes(pipe_server_to_child, pipe_child_to_server);

	if (!ok) {
		status = WORKER_NOT_RUNNING;

		return false;
	}

	ok = startProcess(executable, args, pipe_server_to_child, pipe_child_to_server);

	if (!ok) {
		status = WORKER_NOT_RUNNING;
		return false;
	}

	status = WORKER_RUNNING;

	return true;
}

#endif

#if defined(_MSC_VER)

bool Worker::createPipes(HANDLE& hChildStdinRd, HANDLE& hChildStdinWr,
        HANDLE& hChildStdoutRd, HANDLE& hChildStdoutWr)
{

	// set the bInheritHandle flag so pipe handles are inherited
	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// create a pipe for the child process's STDOUT
	if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
		Logger::error << "stdout pipe creation failed" << endl;
		return false;
	}

	// create a pipe for the child process's STDIN
	if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) {
		Logger::error << "stdin pipe creation failed" << endl;
		return false;
	}

	return true;
}

#else

bool Worker::createPipes(int * pipe_server_to_child, int * pipe_child_to_server)
{
	if (pipe(pipe_server_to_child) == -1) {
		Logger::error << "cannot create pipe" << endl;
		return false;
	}

	if (pipe(pipe_child_to_server) == -1) {
		Logger::error << "cannot create pipe" << endl;

		close(pipe_server_to_child[0]);
		close(pipe_server_to_child[1]);

		return false;
	}

	return true;
}

#endif

#if defined(_MSC_VER)

bool Worker::startProcess(const string& executable, HANDLE rd, HANDLE wr)
{
	readPosition = 0;
	readBuffer.clear();

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bFuncRetn = FALSE;

	// set up members of the PROCESS_INFORMATION structure
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// set up members of the STARTUPINFO structure
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);

	siStartInfo.dwFlags = STARTF_USESTDHANDLES;
	siStartInfo.hStdInput = rd;
	siStartInfo.hStdOutput = wr;
	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	// create the child process
	bFuncRetn = CreateProcess(NULL,
	        (char*) executable.c_str(), // command line
	        NULL, // process security attributes
	        NULL, // primary thread security attributes
	        TRUE, // handles are inherited
	        CREATE_NEW_PROCESS_GROUP, // creation flags
	        NULL, // use parent's environment
	        NULL, // use parent's current directory
	        &siStartInfo, // STARTUPINFO pointer
	        &piProcInfo); // receives PROCESS_INFORMATION

	if (bFuncRetn == FALSE) {
		Logger::error << "execute of '" << executable << "' failed" << endl;
		return false;
	} else {
		processHandle = piProcInfo.hProcess;
		CloseHandle(piProcInfo.hThread);
		return true;
	}
}

#else

bool Worker::startProcess(const string& executable, char** args, int * pipe_server_to_child, int * pipe_child_to_server)
{
	readPosition = 0;
	readBuffer.clear();

	processPid = fork();

	// child process
	if (processPid == 0) {
		Logger::trace << "fork succeeded" << endl;

		// set stdin and stdout of child process
		dup2(pipe_server_to_child[0], 0);
		dup2(pipe_child_to_server[1], 1);

		fcntl(0, F_SETFD, 0);
		fcntl(1, F_SETFD, 0);

		// close pipes
		close(pipe_server_to_child[0]);
		close(pipe_server_to_child[1]);
		close(pipe_child_to_server[0]);
		close(pipe_child_to_server[1]);

		// ignore signals in worker process
		signal(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGUSR1, SIG_IGN);

		// execute worker
		execv(executable.c_str(), args);

		// should not happen
		cerr << "ERROR: execution of '" << executable << "' failed" << endl;

		exit(1);
	}

	// parent
	if (processPid == -1) {
		Logger::error << "fork failed " << endl;

		close(pipe_server_to_child[0]);
		close(pipe_server_to_child[1]);
		close(pipe_child_to_server[0]);
		close(pipe_child_to_server[1]);

		return false;
	}

	Logger::debug << "worker '" << executable << "' started with pid " << processPid << endl;

	close(pipe_server_to_child[0]);
	close(pipe_child_to_server[1]);

	writePipe = pipe_server_to_child[1];
	readPipe = pipe_child_to_server[0];

	return true;
}

#endif

#if defined(_MSC_VER)

void Worker::terminateProcess()
{
	if (status != WORKER_NOT_RUNNING) {
		UINT uExitCode = 0;
		DWORD exitCode;

		// kill worker process
		if (TerminateProcess(processHandle, uExitCode)) {
			Logger::trace << "kill of worker process succeeded" << endl;
			CloseHandle(processHandle);
		} else {
			DWORD e1 = GetLastError();
			BOOL ok = GetExitCodeProcess(processHandle, &exitCode);

			if (ok) {
				Logger::debug << "worker process already dead: " << exitCode << endl;
			} else {
				Logger::warning << "kill of worker process failed: " << exitCode << endl;
			}
		}

		CloseHandle(hChildStdoutRd);
		CloseHandle(hChildStdinWr);
	}

	status = WORKER_NOT_RUNNING;
}

#else

void Worker::terminateProcess()
{
	if (status != WORKER_NOT_RUNNING) {

		// kill worker process
		int val = kill(processPid, SIGKILL);

		if (val) {
			Logger::warning << "kill returned '" << val << "'" << endl;
		} else {
			Logger::trace << "kill of worker process succeeded" << endl;
		}

		int childExitStatus;

		pid_t ws = 0;

		while (ws == 0) {
			usleep(1000);
			ws = waitpid(processPid, &childExitStatus, WNOHANG);
		}

		if (!(WIFEXITED(childExitStatus) || WIFSIGNALED(childExitStatus))) {
			Logger::warning << "worker process still running: exit code " << childExitStatus << endl;
		}

		if (ws == processPid) {
			Logger::debug << "got exit status of worker pid = '" << processPid << "' in Worker::~Worker" << endl;
		} else {
			Logger::debug << "got different process id '" << ws << "', expected '" << processPid << "'" << endl;
		}

		close(writePipe);
		close(readPipe);
	} else {
		int childExitStatus;
		pid_t ws = waitpid(processPid, &childExitStatus, WNOHANG);

		if (ws == processPid && ws != 0) {
			Logger::debug << "got exit status of worker pid = '" << processPid << "' in Worker::~Worker" << endl;
		};
	}

	status = WORKER_NOT_RUNNING;
}

#endif

// /////////////////////////////////////////////////////////////////////////////
// communication with worker process
// /////////////////////////////////////////////////////////////////////////////

bool Worker::sendSession()
{
	bool ok = sendLine("SESSION;" + session, 0);

	if (!ok) {
		return false;
	}

	ok = readResult(0);

	if (!ok) {
		return false;
	}

	return true;
}

bool Worker::restart()
{
	numFailures++;

	if (numFailures >= maxFailures) {
		Logger::error << "Max worker failures ('" << maxFailures << "') reached, can't recover." << endl;
		throw WorkerException("SVS script error, can't recover, contact administrator.", false);
	}

	Logger::warning << "trying to restart worker" << endl;

	terminateProcess();

	bool ok = startProcess();

	if (!ok) {
		return false;
	}

	ok = sendSession();

	if (!ok) {
		terminateProcess();
		return false;
	}

	return true;
}

#if defined(_MSC_VER)

bool Worker::sendLine(const string& text, time_t timeout)
{
	if (numFailures >= maxFailures) {
		return false;
	}

	Logger::trace << "send line '" << text << "'" << endl;

	result.clear();

	string line = text + "\n";
	const char* buffer = line.c_str();
	size_t len = line.size();

	while (0 < len) {
		DWORD result;
		BOOL ok = WriteFile(hChildStdinWr, buffer, (int) len, &result, 0);

		if (ok == FALSE) {
			Logger::debug << "WriteFile failed in " << __FUNCTION__ << "(" << __FILE__
			        << "@" << __LINE__ << ")" << " with " << errno_socket
			        << " (" << strerror_socket(GetLastError()) << ")" << endl;

			return false;
		}

		len -= result;
		buffer += result;
	}

	return true;
}

#else

bool Worker::sendLine(const string& text, time_t timeout)
{
	if (numFailures >= maxFailures) {
		return false;
	}

	Logger::trace << "send line '" << text << "'" << endl;

	result.clear();

	string line = text + "\n";
	const char* buffer = line.c_str();
	size_t len = line.size();

	// without timeout
	if (timeout == 0) {
		while (0 < len) {
			int nr = write(writePipe, buffer, (int)len);

			if (nr < 0) {
				if (errno_socket == EINTR_SOCKET) {
					continue;
				} else if (errno_socket != EWOULDBLOCK_SOCKET) {
					Logger::debug << "write failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

					return false;
				} else {
					continue;
				}
			}

			len -= nr;
			buffer += nr;
		}
	}

	// with timeout
	else {
		while (0 < len) {

#ifdef USE_POLL
			pollfd fds[1];

			fds[0].fd = writePipe;
			fds[0].events = POLLWRNORM | POLLHUP;
			fds[0].revents = 0;

			int np = ::poll(fds, 1, timeout);

			if (np < 0) {
				Logger::trace << "terminating worker, poll failed" << endl;
				return false;
			} else if (np == 0) {
				continue;
			} else if (fds[0].revents & POLLHUP) {
				Logger::trace << "terminating worker, poll returned POLLHUP" << endl;
				return false;
			} else if (!(fds[0].revents & POLLWRNORM)) {
				continue;
			}
#else
fd_set writeFds;
FD_ZERO(&writeFds);
FD_SET(writePipe, &writeFds);

fd_set exceptFds;
FD_ZERO(&exceptFds);
FD_SET(writePipe, &exceptFds);

timeval tv;

tv.tv_sec = 0;
tv.tv_usec = timeout * 1000;

int np = (int) select((int) writePipe + 1, 0, &writeFds, &exceptFds, &tv);

if (np < 0) {
	Logger::trace << "terminating worker, select failed" << endl;
	return false;
} else if (np == 0) {
	continue;
} else if (FD_ISSET(writePipe, &exceptFds)) {
	Logger::trace << "terminating worker, select returned exception" << endl;
	return false;
} else if (! FD_ISSET(writePipe, &writeFds)) {
	continue;
}
#endif

			int nr = write(writePipe, buffer, (int)len);

			if (nr < 0) {
				if (errno_socket == EINTR_SOCKET) {
					continue;
				} else if (errno_socket != EWOULDBLOCK_SOCKET) {
					Logger::debug << "write failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

					return false;
				} else {
					continue;
				}
			}

			len -= nr;
			buffer += nr;
		}
	}

	return true;
}

#endif

bool Worker::readResult(time_t timeout)
{
	string line;

	while (true) {
		bool ok = readLine(line, timeout);

		if (!ok) {
			return false;
		}

		if (line == "DONE") {
			return true;
		} else if (line.substr(0, 6) == "ERROR;") {
			Logger::warning << "got error message '" << line.substr(6) << "' from Worker" << endl;
			result.push_back(line);
		} else if (line.substr(0, 10) == "EXCEPTION;") {
			//Logger::error << "SVS function call failed: " << line.substr(10) << endl;
			result.push_back(line);
		} else {
			result.push_back(line);
		}
	}
}

bool Worker::readLine(string& line, time_t timeout)
{
	while (true) {
		const char * ptr = readBuffer.c_str() + readPosition;
		const char * end = readBuffer.end();

		while (ptr < end) {
			for (; ptr < end; ptr++) {
				if (*ptr == '\n') {
					break;
				}
			}

			if (ptr < end) {
				readPosition = ptr - readBuffer.c_str();

				size_t lineLength = readPosition;

				if (lineLength > 0 && *(ptr - 1) == '\r') {
					lineLength--;
				}

				string result(readBuffer.c_str(), lineLength);

				Logger::trace << "got result '" << result << "' from worker" << endl;

				readBuffer.erase_front(readPosition + 1);
				readPosition = 0;

				line = result;

				return true;
			}
		}

		bool ok = fillReadBuffer(timeout);

		if (!ok) {
			return false;
		}
	}
}

#if defined(_MSC_VER)

bool Worker::fillReadBuffer(time_t timeout)
{
	char buffer[1024];
	DWORD nr = 0;

	if (! ReadFile(hChildStdoutRd, buffer, (DWORD) sizeof(buffer), &nr, 0)) {
		Logger::debug << "ReadFile failed in " << __FUNCTION__ << "(" << __FILE__ << "@"
		        << __LINE__ << ")" << " with " << errno_socket << " ("
		        << strerror_socket(GetLastError()) << ")" << endl;

		return false;
	}

	readBuffer.appendText(buffer, nr);

	return true;
}

#else

bool Worker::fillReadBuffer(time_t timeout)
{
	char buffer[1024];

	// with timeout
	if (timeout != 0) {
#ifdef USE_POLL
		pollfd fds[1];

		fds[0].fd = readPipe;
		fds[0].events = POLLRDNORM | POLLHUP;
		fds[0].revents = 0;

		int np = ::poll(fds, 1, timeout);

		if (np < 0) {
			Logger::trace << "terminating worker, poll failed" << endl;
			return false;
		} else if (np == 0) {
			return fillReadBuffer(timeout);
		} else if (fds[0].revents & POLLHUP) {
			Logger::trace << "terminating worker, poll returned POLLHUP" << endl;
			return false;
		} else if (!(fds[0].revents & POLLRDNORM)) {
			return fillReadBuffer(timeout);
		}
#else
fd_set readFds;
FD_ZERO(&readFds);
FD_SET(readPipe, &readFds);

fd_set exceptFds;
FD_ZERO(&exceptFds);
FD_SET(readPipe, &exceptFds);

timeval tv;

tv.tv_sec = 0;
tv.tv_usec = timeout * 1000;

int np = (int) select((int) readPipe + 1, &readFds, 0, &exceptFds, &tv);

if (np < 0) {
	Logger::trace << "terminating worker, select failed" << endl;
	return false;
} else if (np == 0) {
	return fillReadBuffer(timeout);
} else if (FD_ISSET(readPipe, &exceptFds)) {
	Logger::trace << "terminating worker, select returned exception" << endl;
	return false;
} else if (! FD_ISSET(readPipe, &readFds)) {
	return fillReadBuffer(timeout);
}
#endif
	}

	// start reading
	int nr = read(readPipe, buffer, sizeof(buffer));

	if (nr > 0) {
		readBuffer.appendText(buffer, nr);
		return true;
	} else if (nr == 0) {
		return false;
	} else if (errno_socket == EINTR_SOCKET) {
		return fillReadBuffer(timeout);
	} else if (errno_socket != EWOULDBLOCK_SOCKET) {
		Logger::debug << "read failed in " << __FUNCTION__ << "(" << __FILE__ << "@" << __LINE__ << ")" << " with " << errno_socket << " (" << strerror_socket(errno_socket) << ")" << endl;

		return false;
	} else {
		return fillReadBuffer(timeout);
	}
}

#endif
}
