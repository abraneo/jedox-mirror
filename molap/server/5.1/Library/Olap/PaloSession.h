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

#ifndef OLAP_PALO_SESSION_H
#define OLAP_PALO_SESSION_H 1

#include "palo.h"

#include <map>
#include <deque>

extern "C" {
#include <time.h>
}

#include "Olap/Cube.h"
#include "Olap/Database.h"

#include "Thread/WriteLocker.h"

namespace palo {
class User;
class PaloJob;

////////////////////////////////////////////////////////////////////////////////
/// @brief session handler for http
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloSession {
public:
	static const string NO_SESSION;
	static const string FAKE_SESSION;

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a new session
	////////////////////////////////////////////////////////////////////////////////

	static boost::shared_ptr<PaloSession> createSession(string sid, IdentifierType sidId, PUser user, bool worker, time_t ttl, bool testSession, const string peerName, const string *machine, const string *required, string *optional, const string description, const string &locale);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief creates a new session
	////////////////////////////////////////////////////////////////////////////////

	static boost::shared_ptr<PaloSession> createSession(PUser user, bool worker, time_t ttl, bool shortSid, bool testSession, const string peerName, const string *machine, const string *required, string *optional, const string description, const string &locale);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief deletes a session
	////////////////////////////////////////////////////////////////////////////////

	static void deleteSession(boost::shared_ptr<PaloSession>, bool forced);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds session by encoded identifier
	////////////////////////////////////////////////////////////////////////////////

	static boost::shared_ptr<PaloSession> findSession(const string& sid, bool setWorkerContext);
	static void printActiveJobs(const string &prefix, const string &sufix);

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloSession(string sid, IdentifierType id, PUser user, bool worker, time_t, const string peerName, const string description, const string &machineId, const string &locale);

public:
	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	~PaloSession();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the session identifier
	////////////////////////////////////////////////////////////////////////////////

	string getSid() const {
		return sid;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the session internal identifier
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getInternalId() const {
		return sidId;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks if session is a worker session
	////////////////////////////////////////////////////////////////////////////////

	bool isWorker() const {
		return worker;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the time to live
	////////////////////////////////////////////////////////////////////////////////

	time_t getTtl() const {
		return timeToLive;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the time to live interval
	////////////////////////////////////////////////////////////////////////////////

	time_t getTtlIntervall() const {
		return timeToLiveIntervall;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the time of the login
	////////////////////////////////////////////////////////////////////////////////

	time_t getLoginTime() const {
		return loginTime;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the user or 0
	////////////////////////////////////////////////////////////////////////////////

	PUser getUser() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief increases the total time session was active
	////////////////////////////////////////////////////////////////////////////////

	void increaseTime(uint64_t microseconds, const PaloJob *job);
	void moveJobToFinished(const PaloJob *job);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief on job start
	////////////////////////////////////////////////////////////////////////////////
	void onJobStart(const PaloJob *job);

	void setWorkerContext(Context *c);
	Context *getWorkerContext();

	static void clearOldSessions();

	static void requestRecord(string req, string ext);

	void writeActiveJobsRecords(ofstream *stream);

	PServer getServer() const;

	static string shortenSid(const string &sid, size_t trailCharCount);

	const string &getLocale() {return locale;}

private:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief updates TTL
	////////////////////////////////////////////////////////////////////////////////

	void updateTtl() {
		timeToLive = time(0) + timeToLiveIntervall;
	}

	struct JOB_INFO {
		string username;
		time_t starttime;
		uint64_t revision;
		double duration;
		string request;

		JOB_INFO(string username, time_t starttime, uint64_t revision, double duration, string request) :
			username(username), starttime(starttime), revision(revision), duration(duration), request(request) {}
	};

private:
	static map<string, boost::shared_ptr<PaloSession> > sessions;
	static map<IdentifierType, boost::shared_ptr<PaloSession> > sessionIds;
	static list<JOB_INFO> finishedJobs;
	static Mutex m_main_Lock;
	static IdentifierType lastSessionId;
private:
	string sid; // external session id string
	IdentifierType sidId; // internal id
	IdentifierType userIdentifier;
	bool worker;
	time_t timeToLive;
	time_t timeToLiveIntervall;
	time_t loginTime;

private:
	IdentifierType databaseId;
	IdentifierType cubeId;
	timeval lastTime;
	Context *workercontext;

	typedef pair<IdentifierType, IdentifierType> DbDimPair;
	Mutex thisLock;

	uint64_t requestCounter;
	double totalTime;
	set<const PaloJob *> activeJobs;
	string command;
	const string peerName;
	string description;
	const string machineId;
	const string locale;

	friend class SessionInfoProcessor;
	friend class SessionInfoCube;
	friend class ServerLogoutJob;
};

}

#endif
