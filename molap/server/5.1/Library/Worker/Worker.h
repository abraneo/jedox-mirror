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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef WORKER_WORKER_H
#define WORKER_WORKER_H 1

#include "palo.h"

#include "Collections/StringBuffer.h"
#include "Thread/Mutex.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base class for worker
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Worker {
public:
	static const int WORKER_TIMEOUT_MSEC = 2000;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	Worker(const string& session);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~Worker();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets executable for workers
	////////////////////////////////////////////////////////////////////////////////

	static void setExecutable(const string& executable);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets executable for workers
	////////////////////////////////////////////////////////////////////////////////

	static const string &getExecutable() {return executable;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets arguments for workers
	////////////////////////////////////////////////////////////////////////////////

	static void setArguments(const vector<string>& arguments);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns if vector of results is in exception status
	////////////////////////////////////////////////////////////////////////////////

	static bool isErrorStatus(const vector<string>& result);
	static bool isExceptionStatus(const vector<string>& result);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sends a request to the worker and reads result
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus execute(const string& line, vector<string>& result);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sends a request to the worker and reads result
	////////////////////////////////////////////////////////////////////////////////

	virtual ResultStatus execute(const string& line, vector<string>& result, time_t timeout);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sends a terminate request to the worker and kills the process
	////////////////////////////////////////////////////////////////////////////////

	virtual void terminate(const string& line, time_t timeout);
	virtual void terminate(bool restart);

	void releaseSession();

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief starts worker process and sets up communication
	////////////////////////////////////////////////////////////////////////////////

	bool startint();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief path to worker executable
	////////////////////////////////////////////////////////////////////////////////

	static string executable;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief arguments for worker
	////////////////////////////////////////////////////////////////////////////////

	static vector<string> arguments;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief maximal number of failures
	////////////////////////////////////////////////////////////////////////////////

	static int maxFailures;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief result lines
	////////////////////////////////////////////////////////////////////////////////

	vector<string> result;

private:
#if defined(_MSC_VER)

	bool startProcess(const string& executable, HANDLE, HANDLE);

	bool createPipes(HANDLE&, HANDLE&, HANDLE&, HANDLE&);

#else

	bool startProcess(const string& executable, char** args, int*, int*);

	bool createPipes(int*, int*);

#endif

	bool startProcess();

	void terminateProcess();

	bool sendSession();

	bool sendLine(const string& line, time_t timeout);

	bool restart();

	bool readLine(string& line, time_t timeout);

	bool fillReadBuffer(time_t timeout);

	bool readResult(time_t timeout);

protected:
	Mutex mutex;
	WorkerStatus status;
	int numFailures;

private:
	bool started;

	string session;

	StringBuffer readBuffer;
	size_t readPosition;

#if defined(_MSC_VER)

	HANDLE processHandle;
	HANDLE hChildStdoutRd;
	HANDLE hChildStdinWr;

	string commandLine;

#else

	socket_t writePipe;
	socket_t readPipe;
	pid_t processPid;

	char** args;

#endif
};
}

#endif
