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

#include "PaloHttpInterface.h"

#include "Collections/StringUtils.h"
#include "HttpServer/HttpServer.h"
#include "HttpServer/HttpServerTask.h"
#include "Logger/Logger.h"
#include "Olap/Server.h"
#include "PaloHttpServer/PaloHttpServer.h"
#include "Exceptions/CommunicationException.h"

#if defined(ENABLE_HTTPS) && ! defined(ENABLE_HTTPS_MODULE)
#include "HttpsServer/PaloHttpsServer.h"
#endif

#include "PaloOptions.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructor
// /////////////////////////////////////////////////////////////////////////////

PaloHttpInterface::PaloHttpInterface(PaloOptions* options, JobAnalyser* analyser) :
		templateDirectory(options->templateDirectory), encryptionType(options->encryptionType), traceFile(options->traceFile), traceFileCounter(0), adminPorts(options->adminPorts), httpPorts(options->httpPorts), httpsPorts(options->httpsPorts), keyFiles(options->keyFiles), keyFilePassword(options->keyFilePassword), externalHttpInterface(options->externalHttpInterface), externalHttpsInterface(options->externalHttpsInterface), analyser(analyser)
{
}

PaloHttpInterface::~PaloHttpInterface()
{
	for (vector<HttpServer*>::iterator it = servers.begin(); it != servers.end(); ++it) {
		delete *it;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpInterface::addServers()
{
	int ports = 0;

	// http & https ports
	vector<string>::const_iterator j = httpsPorts.begin();

	for (vector<string>::const_iterator i = httpPorts.begin(); i != httpPorts.end();) {
		const string& address = *i++;
		const string& portString = *i++;

		int port = StringUtils::stringToInteger(portString);

		if (port == 0) {
			Logger::error << "illegal port number: " << portString << endl;
			continue;
		}

		if (j < httpsPorts.end()) {
			const string& portsString = *j++;

			ports = StringUtils::stringToInteger(portsString);

			if (ports == 0) {
				Logger::error << "illegal port number: " << portsString << endl;
				continue;
			}

			Logger::trace << "trying to open http/https server on " << address << ":" << port << " / " << ports << endl;

			addHttpsServer(address, ports);
			addHttpServer(address, port, ports, false);
		} else {
			Logger::trace << "trying to open http server on " << address << ":" << port << endl;

			addHttpServer(address, port, 0, false);
		}
	}

	// admin ports
	for (vector<string>::const_iterator i = adminPorts.begin(); i != adminPorts.end();) {
		const string& address = *i++;
		const string& portString = *i++;

		int port = StringUtils::stringToInteger(portString);

		if (port == 0) {
			Logger::error << "illegal port number: " << portString << endl;
			continue;
		}

		Logger::trace << "trying to open admin server on " << address << ":" << port << endl;

		addHttpServer(address, port, ports, true);
	}

	if (servers.empty()) {
		throw CommunicationException(ErrorException::ERROR_UNKNOWN, "No listener initialized.");
	}
}

void PaloHttpInterface::run()
{
	fd_set sockets;
	task_list_type tasks;
	socket_t max = 0;

	runnable = true;
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	FD_ZERO(&sockets);
	for (server_list_type::iterator i = servers.begin(); i != servers.end(); ++i) {
		socket_t crt = (*i)->getReadSocket();
		FD_SET(crt, &sockets);
		if (max < crt) {
			max = crt;
		}
	}

	while (0 <= select((int)max + 1, &sockets, NULL, NULL, &tv)) {

		if (!runnable) {
			return;
		}

		task_list_type::iterator pTask = tasks.begin();
		while (pTask != tasks.end()) {
			if (FD_ISSET((*pTask)->getReadSocket(), &sockets)) {
				if (!(*pTask)->Tranzact()) {
					delete (*pTask);
					pTask = tasks.erase(pTask);
					continue;
				}
			}
			++pTask;
		}

		server_list_type::iterator pServer = servers.begin();
		while (pServer != servers.end()) {
			if (FD_ISSET((*pServer)->getReadSocket(), &sockets)) {
				tasks.push_back((*pServer)->CreateConnectionTask());
			}
			++pServer;
		}

		FD_ZERO(&sockets);
		max = 0;
		for (pServer = servers.begin(); pServer != servers.end(); ++pServer) {
			socket_t crt = (*pServer)->getReadSocket();
			FD_SET(crt, &sockets);
			if (max < crt) {
				max = crt;
			}
		}
		for (pTask = tasks.begin(); pTask != tasks.end(); ++pTask) {
			socket_t crt = (*pTask)->getReadSocket();
			FD_SET(crt, &sockets);
			if (max < crt) {
				max = crt;
			}
		}

		tv.tv_sec = 10;
		tv.tv_usec = 0;
	}
}

void PaloHttpInterface::handleShutdown()
{
	bool wasNew;
	PServer server = Context::getContext(&wasNew)->getServerCopy();
	Logger::info << "beginning shutdown sequence" << endl;
	runnable = false;
	for (vector<HttpServer*>::iterator i = servers.begin(); i != servers.end(); ++i) {
		(*i)->handleShutdown();
	}
	server->beginShutdown(PUser());
	server->commit();
	Context::reset(wasNew);
}

void PaloHttpInterface::commitAndSave()
{
	WriteLocker wl(&Server::getSaveLock());
	PServer server = Context::getContext()->getServer();
	server->commitAndSave();
	Context::reset();
}

#ifdef ENABLE_GPU_SERVER
bool PaloHttpInterface::optimizeGpuEngine()
{
	bool ret = false;
	if((Context::getContext()->getServer()->getCopiesCount() == 0)){
		PServer server = Context::getContext()->getServerCopy();
		ret = server->optimizeGpuEngine();
		if(server->getCopiesCount() == 1 && ret){
			server->commit();
			Context::reset();
			ret = true;
		} else {
			server.reset();
			Context::reset();
			ret = false;
		}
	}
	return ret;
}

bool PaloHttpInterface::needGpuOptimization()
{
	bool ret = false;
	PServer server = Context::getContext()->getServer();
	ret = server->needGpuOptimization();
	Context::reset();
	return ret;
}
#endif

void PaloHttpInterface::clearOldSessions()
{
	PaloSession::clearOldSessions();
	Context::reset();
}

void PaloHttpInterface::requestRecord()
{
	PaloSession::requestRecord("requests", "txt");
	Context::reset();
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloHttpInterface::addHttpServer(const string& a, int port, int httpsPort, bool admin)
{
	string address = a;

	if (address == "-") {
		address.clear();
	}

	pair<string, int> ap(address, port);

	if (seen.find(ap) != seen.end()) {
		Logger::warning << "address/port " << address << "/" << port << " already defined, ignoring duplicate entry" << endl;
		return;
	}

	seen.insert(ap);

	// construct a new http server
	HttpServer * httpServer = 0;

	if (externalHttpInterface != 0) {
		httpServer = (*externalHttpInterface)(templateDirectory);
	} else {
		httpServer = new PaloHttpServer(templateDirectory);
	}

	if (httpServer == 0) {
		Logger::error << "construction of http server failed, aborting!" << endl;
		throw CommunicationException(ErrorException::ERROR_UNKNOWN, "cannot construct http server");
	}

	PaloHttpServer* paloHttpServer = dynamic_cast<PaloHttpServer*>(httpServer);

	if (paloHttpServer == 0) {
		Logger::info << "http interface is not a palo http server" << endl;
		return;
	} else {
		if (admin) {
			paloHttpServer->enableBroswer();
			EnablePaloInterface(paloHttpServer);
			paloHttpServer->enableServerInfo(httpsPort);
			paloHttpServer->enableLogin();
		} else {
			if (Server::getInstance(false)->getEncryptionType() != ENC_REQUIRED) {
				EnablePaloInterface(paloHttpServer);
			}

			paloHttpServer->enableServerInfo(httpsPort);
			paloHttpServer->enableLogin();
		}
	}

	paloHttpServer->setParams(address, port);

	if (!httpServer->start(analyser))
		return;
	Logger::info << "http port '" << port << "' on address '" << address << "' open" << endl;

	// set trace file
	if (!traceFile.empty()) {
		httpServer->setTraceFile(traceFile + "-" + StringUtils::convertToString(traceFileCounter++));
	}

	// add possible extensions
#ifdef FIXMEMEMEMEMEM
	for (vector<ServerInfo_t*>::iterator iter = httpExtensions.begin(); iter != httpExtensions.end(); ++iter) {
		ServerInfo_t * info = *iter;

		(*(info->httpExtensions))(httpServer, admin, false);
	}
#endif

	// keep a list of servers
	servers.push_back(httpServer);
}

void PaloHttpInterface::addHttpsServer(const string& a, int port)
{
	string address = a;

	if (address == "-") {
		address.clear();
	}

	pair<string, int> ap(address, port);

	if (seen.find(ap) != seen.end()) {
		Logger::warning << "address/port " << address << "/" << port << " already defined, ignoring duplicate entry" << endl;
		return;
	}

	seen.insert(ap);

	// construct a new http server
	if (keyFiles.size() != 3) {
		Logger::error << "ssl keys are not defined, aborting!" << endl;
		throw CommunicationException(ErrorException::ERROR_UNKNOWN, "no ssl keys");
	}

	HttpServer * httpsServer = 0;

	if (externalHttpsInterface != 0) {
		httpsServer = (*externalHttpsInterface)(keyFiles[0], keyFiles[1], keyFilePassword, keyFiles[2], templateDirectory);
	} else {
#if defined(ENABLE_HTTPS) && ! defined(ENABLE_HTTPS_MODULE)
		httpsServer = new PaloHttpsServer(keyFiles[0], keyFiles[1], keyFilePassword, keyFiles[2], templateDirectory);
#else
		return;
#endif
	}

	if (httpsServer == 0) {
		Logger::error << "construction of https server failed, aborting!" << endl;
		throw CommunicationException(ErrorException::ERROR_UNKNOWN, "cannot construct https server");
	}

	PaloHttpServer* paloHttpServer = dynamic_cast<PaloHttpServer*>(httpsServer);

	if (paloHttpServer == 0) {
		Logger::info << "https interface is not a palo https server" << endl;
		return;
	} else {
		EnablePaloInterface(paloHttpServer);
		paloHttpServer->enableServerInfo(port);
		paloHttpServer->enableLogin();
	}

	paloHttpServer->setParams(address, port);

	if (!paloHttpServer->start(analyser))
		return;
	Logger::info << "https port '" << port << "' on address '" << address << "' open" << endl;

	// set trace file
	if (!traceFile.empty()) {
		httpsServer->setTraceFile(traceFile + "-" + StringUtils::convertToString(traceFileCounter++));
	}

	// add possible extensions
#ifdef FIXMEMEMEMEMEM
	for (vector<ServerInfo_t*>::iterator iter = httpExtensions.begin(); iter != httpExtensions.end(); ++iter) {
		ServerInfo_t * info = *iter;

		(*(info->httpExtensions))(httpsServer, admin, false);
	}
#endif

	// keep a list of servers
	servers.push_back(httpsServer);
}

void PaloHttpInterface::EnablePaloInterface(PaloHttpServer *paloHttpServer)
{
	paloHttpServer->enablePalo();
}

}
