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
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Olap/PaloSession.h"

#include "Exceptions/ParameterException.h"

#include "Olap/SystemDatabase.h"
#include "Olap/Server.h"
#include "Olap/Context.h"

#include "PaloDispatcher/PaloJob.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <iostream>
#include <iomanip>

namespace palo {
map<string, boost::shared_ptr<PaloSession> > PaloSession::sessions;
map<IdentifierType, boost::shared_ptr<PaloSession> > PaloSession::sessionIds;
list<PaloSession::JOB_INFO> PaloSession::finishedJobs;

Mutex PaloSession::m_main_Lock;
IdentifierType PaloSession::lastSessionId = 0;

const string PaloSession::NO_SESSION = "";
const string PaloSession::FAKE_SESSION = "0000";


PaloSession::PaloSession(string sid, IdentifierType id, PUser user, bool worker, time_t ttlIntervall, const string peerName, const string description, const string &machineId, const string &locale) :
	sid(sid), sidId(id), worker(worker), timeToLiveIntervall(ttlIntervall), workercontext(0), requestCounter(0), totalTime(0), peerName(peerName), description(description), machineId(machineId), locale(locale.empty() ? UTF8ComparerInternal::coll.getName() : locale)
{
	userIdentifier = (user == 0) ? NO_IDENTIFIER : user->getId();

	if (timeToLiveIntervall == 0) {
		timeToLiveIntervall = 24L * 3600L * 364L * 10L;
	}

	loginTime = time(0);
	timeToLive = loginTime + timeToLiveIntervall;

	databaseId = 0;
	cubeId = 0;
	gettimeofday(&lastTime, 0);
}

PaloSession::~PaloSession()
{
}

boost::shared_ptr<PaloSession> PaloSession::createSession(string sid, IdentifierType sidId, PUser user, bool worker, time_t ttlIntervall, bool testSession, const string peerName, const string *machine, const string *required, string *optional, const string description, const string &locale)
{
	boost::shared_ptr<PaloSession> session(new PaloSession(sid, sidId, user, worker, ttlIntervall, peerName, description, machine ? *machine : "", locale));
	if (!testSession) {
		Context::getContext()->getServer()->reserveLicense(session, machine, required, optional);
		Logger::debug << "session key " << shortenSid(sid, 3) << endl;
		sessions[sid] = session;
		sessionIds[sidId] = session;

		if (user) {
			Logger::info << "user '" << user->getName() << "' logged in" << endl;
		}
	}

	return session;
}

boost::shared_ptr<PaloSession> PaloSession::createSession(PUser user, bool worker, time_t ttlIntervall, bool shortSid, bool testSession, const string peerName, const string *machine, const string *required, string *optional, const string description, const string &locale)
{
	boost::shared_ptr<PaloSession> result;
	string sidString;

	string sidChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	{
		WriteLocker write_locker(&m_main_Lock);

		do {
			sidString = "";
			if (shortSid) {
				for (int i = 0; i < 4; i++) {
					sidString += sidChars[rand() % sidChars.size()];
				}
			} else {
				for (int i = 0; i < 32; i++) {
					sidString += sidChars[rand() % sidChars.size()];
				}

				string str;
				stringstream out;
				out << hex << setfill('0') << std::setw(2);

				out << time(NULL);
				str = out.str();
				while (str.size() < 8) {
					str += 't';
				}
				for (int i = 0; i < 8; i++) {
					sidString[29 - i*4] = str[i]; // 29,25,21,...,5,1
				}

				out.str("");
				out.clear();
				if (user) {
					out << user->getId();
				}
				str = out.str();
				while (str.size() < 4) {
					str += 'u';
				}
				for (int i = 0; i < 4; i++) {
					sidString[3 + i*4] = str[i]; // 3,7,11,15
				}

				out.str("");
				out.clear();
#if defined(WIN32)
				out << GetCurrentProcessId();
#else
				out << getpid();
#endif
				str = out.str();
				while (str.size() < 4) {
					str += 'p';
				}
				for (int i = 0; i < 4; i++) {
					sidString[3 + 16 + i*4] = str[i]; // 19,23,27,31
				}
			}
		} while (sidString == FAKE_SESSION || sidString == NO_SESSION || sessions.find(sidString) != sessions.end());

		result = createSession(sidString, testSession ? 0 : ++lastSessionId, user, worker, ttlIntervall, testSession, peerName, machine, required, optional, description, locale);
	}
	return result;
}

void PaloSession::deleteSession(boost::shared_ptr<PaloSession> session, bool forced)
{
	if (session->getUser()) {
		Logger::info << "user '" << session->getUser()->getName() << "' logged out" << (forced ? " by administrator" : "") << endl;
	}
	{
		WriteLocker write_locker(&m_main_Lock);
		Context::getContext()->getServer()->freeLicense(session);
		sessions.erase(session->getSid());
		sessionIds.erase(session->getInternalId());
	}
}

boost::shared_ptr<PaloSession> PaloSession::findSession(const string &sid, bool setWorkerContext)
{
	WriteLocker read_locker(&m_main_Lock);

	map<string, boost::shared_ptr<PaloSession> >::iterator iter = sessions.find(sid);

	if (iter == sessions.end()) {
		throw ParameterException(ErrorException::ERROR_INVALID_SESSION, "wrong session identifier", "session identifier", sid);
	}

	boost::shared_ptr<PaloSession> session = iter->second;

	if (session->activeJobs.size() == 0 && time(0) > session->getTtl()) {
		Context::getContext()->getServer()->freeLicense(session);
		throw ParameterException(ErrorException::ERROR_INVALID_SESSION, "old session identifier", "session identifier", sid);
	}

	if (setWorkerContext && session->isWorker()) {
		Context *c = session->getWorkerContext();
		if (c) {
			Context::setWorkersContext(c);
		}
	}

	return session;
}

void PaloSession::printActiveJobs(const string &prefix, const string &sufix)
{
	WriteLocker read_locker(&m_main_Lock);
	for (map<string, boost::shared_ptr<PaloSession> >::iterator it = sessions.begin(); it != sessions.end(); ++it) {
		boost::shared_ptr<PaloSession> session = it->second;
		WriteLocker sl(&session->thisLock);
		for (set<const PaloJob *>::iterator itj = session->activeJobs.begin(); itj != session->activeJobs.end(); ++itj) {
			Logger::info << prefix << (*itj)->getName() << sufix << endl;
		}
	}
}

PUser PaloSession::getUser() const
{
	CPServer server = Context::getContext(0, false)->getServer();
	PSystemDatabase sd = server->getSystemDatabase();
	PUser user;

	if (sd) {
		if (userIdentifier != NO_IDENTIFIER) {
			user = sd->getUser(userIdentifier);
		} else {
			return PUser();
		}
	}

	if (user == 0) {
		if (sd) {
			Logger::debug << "user in session not found, sd exists, userIdentifier: " << userIdentifier << ", useExternalUser: " << sd->getUseExternalUser() << ", sid: " << sid << endl;
		} else {
			Logger::error << "user in session not found, sd is null, userIdentifier: " << userIdentifier << endl;
		}
		throw ParameterException(ErrorException::ERROR_INVALID_SESSION, "user in session not found", "user", (int)userIdentifier);
	}

	return user;
}

void PaloSession::increaseTime(uint64_t microseconds, const PaloJob *job) {
	moveJobToFinished(job);

	WriteLocker wl(&thisLock);
	totalTime += (double)microseconds/1000000;
	requestCounter++;
	activeJobs.erase(job);
	updateTtl();
}

void PaloSession::moveJobToFinished(const PaloJob *job) {
	if (Server::flightRecorderEnabled()) {
		WriteLocker wl(&m_main_Lock);
		PServer server = getServer();
		if (isWorker()) {
			finishedJobs.push_back(JOB_INFO("SVS-worker", job->getStartTime(), server->getObjectRevision(), job->getDuration(), job->getRequest()));
		} else {
			PUser user;
			try {
				user = getUser();
			} catch (...) {
				Logger::debug << "user not found exception caught in increaseTime, deleted user" << endl;
			}
			finishedJobs.push_back(JOB_INFO(user ? user->getName() : "DELETED", job->getStartTime(), server->getObjectRevision(), job->getDuration(), job->getRequest()));
		}
		if (finishedJobs.size() > 10) {
			finishedJobs.pop_front();
		}
	}
}

void PaloSession::onJobStart(const PaloJob *job) {
	WriteLocker wl(&thisLock);
	activeJobs.insert(job);
}


void PaloSession::setWorkerContext(Context *c)
{
	workercontext = c;
	if (workercontext) {
		workercontext->setCellValueContext(PCellValueContext());
	}
}

Context *PaloSession::getWorkerContext()
{
	return workercontext;
}

void PaloSession::clearOldSessions()
{
	WriteLocker write_locker(&m_main_Lock);

	for (map<string, boost::shared_ptr<PaloSession> >::iterator iter = sessions.begin(); iter != sessions.end();) {
		boost::shared_ptr<PaloSession> session = iter->second;
		if (session->activeJobs.size() > 0) {
			session->updateTtl();
			++iter;
		} else if (time(0) > session->getTtl()) {
			Logger::debug << "old session removed " << shortenSid(session->getSid(), 3) << endl;
			Context::getContext()->getServer()->freeLicense(session);
			sessionIds.erase(sessionIds.find(session->getInternalId()));
			sessions.erase(iter++);
		} else {
			++iter;
		}
	}
}

PServer PaloSession::getServer() const
{
	return Context::getContext(0, false)->getServer();
}

string PaloSession::shortenSid(const string &sid, size_t trailCharCount)
{
	if (sid.size() < trailCharCount * 2) {
		return sid;
	}

	return sid.substr(0, trailCharCount) + "..." + sid.substr(sid.size() - trailCharCount, trailCharCount);
}

void PaloSession::requestRecord(string req, string ext)
{
	if (!Server::flightRecorderEnabled()) {
		return;
	}

	WriteLocker write_locker(&m_main_Lock);

	std::ofstream *recorderFile = new std::ofstream((req + "_temp." + ext).c_str());
	if (recorderFile) {
		*recorderFile << "#" << StringUtils::convertTimeToString(time(0)) << endl;
		*recorderFile << "#USER" << '\t' << "START" << '\t' << "REVISION" << '\t' << "DURATION" << '\t' << "REQUEST" << endl;
		for (map<string, boost::shared_ptr<PaloSession> >::iterator iter = sessions.begin(); iter != sessions.end(); ++iter) {
			iter->second->writeActiveJobsRecords(recorderFile);
		}

		*recorderFile << endl << "LAST FINISHED JOBS:" << endl;
		{
			for (list<JOB_INFO>::iterator it = finishedJobs.begin(); it != finishedJobs.end(); ++it) {
				*recorderFile << it->username << '\t';
				*recorderFile << StringUtils::convertTimeToString(it->starttime) << '\t';
				*recorderFile << it->revision << '\t';
				*recorderFile << it->duration << '\t';
				*recorderFile << it->request << endl;
			}
		}

		recorderFile->close();
		delete recorderFile;

		FileName reqFN("", req, ext);
		if (FileUtils::isRegularFile(reqFN.fullPath()) && !FileUtils::remove(reqFN)) {
			Logger::error << "Cannot remove requests.txt file." << endl;
		} else {
			if (!FileUtils::rename(FileName("", req + "_temp", ext), reqFN)) {
				Logger::error << "Cannot rename to requests.txt file." << endl;
			}
		}
	}
}

void PaloSession::writeActiveJobsRecords(ofstream *stream)
{
	WriteLocker wl(&thisLock);

	for (set<const PaloJob *>::iterator jobIt = activeJobs.begin(); jobIt != activeJobs.end(); ++jobIt) {
		PServer server = getServer();
		if (isWorker()) {
			*stream << "SVS-worker" << '\t';
		} else {
			PUser user;
			try {
				user = getUser();
			} catch (...) {
				Logger::debug << "user not found exception caught in writeActiveJobsRecords, deleted user" << endl;
			}
			*stream << (user ? user->getName() : "DELETED") << '\t';
		}
		*stream << StringUtils::convertTimeToString((*jobIt)->getStartTime()) << '\t';
		*stream << (server ? server->getObjectRevision() : 0) << '\t';
		*stream << (*jobIt)->getDuration() << '\t';
		*stream << (*jobIt)->getRequest() << endl;
	}
}

}
