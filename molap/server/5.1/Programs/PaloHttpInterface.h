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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PROGRAMS_PALO_HTTP_INTERFACE_H
#define PROGRAMS_PALO_HTTP_INTERFACE_H 1

#include "palo.h"

namespace palo {
class PaloOptions;
class HttpServer;
class HttpServerTask;
class Server;
class PaloHttpServer;

////////////////////////////////////////////////////////////////////////////////
/// @brief init function for an http interface
////////////////////////////////////////////////////////////////////////////////

typedef HttpServer*(*InitHttpInterface_fptr)(const string& templateDirectory);

////////////////////////////////////////////////////////////////////////////////
/// @brief init function for an https interface
////////////////////////////////////////////////////////////////////////////////

typedef HttpServer*(*InitHttpsInterface_fptr)(const string& rootfile, const string& keyfile, const string& password, const string& dhfile, const string& templateDirectory);

////////////////////////////////////////////////////////////////////////////////
/// @brief init function for a job analyser
////////////////////////////////////////////////////////////////////////////////

class JobAnalyser;
typedef JobAnalyser*(*InitJobAnalyser_fptr)();

////////////////////////////////////////////////////////////////////////////////
/// @brief palo http/https interface
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloHttpInterface {
protected:
	typedef std::vector<HttpServer*> server_list_type;
	typedef std::vector<HttpServerTask*> task_list_type;
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloHttpInterface(PaloOptions*, JobAnalyser*);
	virtual ~PaloHttpInterface();
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds servers
	////////////////////////////////////////////////////////////////////////////////

	void addServers();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief run the servers
	////////////////////////////////////////////////////////////////////////////////
	virtual void run();
	virtual void handleShutdown();

	void commitAndSave();
	void clearOldSessions();
	void requestRecord();

#ifdef ENABLE_GPU_SERVER
	bool optimizeGpuEngine();
	bool needGpuOptimization();
#endif
private:
	void addHttpServer(const string& address, int port, int httpsPort, bool admin);

	void addHttpsServer(const string& address, int port);

protected:
	virtual void EnablePaloInterface(PaloHttpServer *paloHttpServer);

private:
	const string templateDirectory;
	const Encryption_e encryptionType;
	const string traceFile;

	set<pair<string, int> > seen;

	int traceFileCounter;

	const vector<string> adminPorts;
	const vector<string> httpPorts;
	const vector<string> httpsPorts;
	const vector<string> keyFiles;
	const string keyFilePassword;

	InitHttpInterface_fptr externalHttpInterface;
	InitHttpsInterface_fptr externalHttpsInterface;

	JobAnalyser* analyser;
public:
	vector<HttpServer*> servers;
protected:
	bool runnable;
};

}

#endif
