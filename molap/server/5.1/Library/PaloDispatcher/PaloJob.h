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
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef PALO_DISPATCHER_PALO_JOB_H
#define PALO_DISPATCHER_PALO_JOB_H 1

#include "palo.h"

#include "Dispatcher/Job.h"
#include "HttpServer/HttpResponse.h"
#include "Olap/Server.h"
#include "PaloHttpServer/PaloRequestHandler.h"
#include "PaloDispatcher/PaloJobRequest.h"
#include <boost/date_time.hpp>

namespace palo {
class PaloSession;
class Rule;
class AreaDoubleResultStorage;

struct StringBuffer;

////////////////////////////////////////////////////////////////////////////////
/// @brief palo job
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloJob : public Job {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloJob(PaloJobRequest* jobRequest);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	~PaloJob();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	bool initialize();

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	IdentifierType getSessionInternalId() {
		if (session) {
			return session->getInternalId();
		} else {
			return 0;
		}
	}

	string getSid() {
		if (session) {
			return session->getSid();
		} else {
			return PaloSession::NO_SESSION;
		}
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief compute the response
	////////////////////////////////////////////////////////////////////////////////

	virtual void compute() = 0;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the response
	////////////////////////////////////////////////////////////////////////////////

	HttpResponse* getResponse() {
		return response;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the context o the job
	////////////////////////////////////////////////////////////////////////////////

	Context *getContext () {return context;}
	const Context *getContext () const {return context;}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the duration of the job
	////////////////////////////////////////////////////////////////////////////////

	double getDuration() const;
	time_t getStartTime() const;

	string getRequest() const;

	IdentifierType getId() const {return id;}

	static SplashMode splashMode(uint32_t splash);
	static uint32_t splashNumber(SplashMode splash);

protected:
	static Element::Type elementTypeByIdentifier(uint32_t);

	static PCubeArea area(PDatabase database, PCube cube, vector<IdentifiersType>* paths, const vector<CPDimension> * dimensions, uint32_t& numResult, bool useBaseOnly);

	static PCubeArea area(PDatabase database, PCube cube, vector<vector<string> >* paths, const vector<CPDimension> * dimensions, uint32_t& numResult, bool useBaseOnly);

	static string areaToString(vector<IdentifiersType>* paths);
	static string areaToString(vector<vector<string> >* paths);
	static string areaToString(size_t dims);

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets server token in response
	////////////////////////////////////////////////////////////////////////////////

	void setToken(CPServer server) {
		response->setToken(PaloRequestHandler::X_PALO_SERVER, server->getToken());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks server token in request
	////////////////////////////////////////////////////////////////////////////////

	void checkToken(CPServer);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks database token in request
	////////////////////////////////////////////////////////////////////////////////

	void checkToken(CPDatabase);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks dimension token in request
	////////////////////////////////////////////////////////////////////////////////

	void checkToken(CPDimension);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks cube token in request
	////////////////////////////////////////////////////////////////////////////////

	void checkToken(CPCube);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets cube token in response
	////////////////////////////////////////////////////////////////////////////////

	void setToken(CPCube cube) {
		response->setToken(PaloRequestHandler::X_PALO_CUBE, cube->getToken());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets cube client cache token in response
	////////////////////////////////////////////////////////////////////////////////

	void setSecondToken(CPCube cube) {
		response->setSecondToken(PaloRequestHandler::X_PALO_CUBE_CLIENT_CACHE, cube->getClientCacheToken());
	}

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets dimension token in response
	////////////////////////////////////////////////////////////////////////////////

	void setToken(CPDimension dimension) {
		response->setToken(PaloRequestHandler::X_PALO_DIMENSION, dimension->getToken());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets database token in response
	////////////////////////////////////////////////////////////////////////////////

	void setToken(CPDatabase database) {
		response->setToken(PaloRequestHandler::X_PALO_DATABASE, database->getToken());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds database
	////////////////////////////////////////////////////////////////////////////////

	void findDatabase(bool requireLoad, bool write);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds cube
	////////////////////////////////////////////////////////////////////////////////

	void findCube(bool requireLoad, bool write);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds path
	////////////////////////////////////////////////////////////////////////////////

	void findPath();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds path to
	////////////////////////////////////////////////////////////////////////////////

	void findPathTo();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds dimension
	////////////////////////////////////////////////////////////////////////////////

	void findDimension(bool write);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds element
	////////////////////////////////////////////////////////////////////////////////

	void findElement(bool write);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds a rule
	////////////////////////////////////////////////////////////////////////////////

	void findRules(bool write);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief finds pathes
	////////////////////////////////////////////////////////////////////////////////

	void findCellPaths(set<size_t> *invalidPaths, User *user);
	void findLockedPaths(set<size_t> *invalidPaths);

protected:

	void checkProperties();

	void findPathsIntern(vector<IdentifiersType> *sourcePaths, PPaths &targetPaths, set<size_t> *invalidPaths, User *user);

	void updateLicenses();

protected:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates login response
	////////////////////////////////////////////////////////////////////////////////

	void generateLoginResponse(boost::shared_ptr<PaloSession>, const string &);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates ok response
	////////////////////////////////////////////////////////////////////////////////

	void generateOkResponse();

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates ok response with a token
	////////////////////////////////////////////////////////////////////////////////

	template<class P> void generateOkResponse(P* object) {
		generateOkResponse();
		setToken(object);
	}

	template<class P> void generateOkResponse(P &object) {
		generateOkResponse();
		setToken(object);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates ok response with message
	////////////////////////////////////////////////////////////////////////////////

	void generateMessageResponse(const string &message);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief generates cell value response
	////////////////////////////////////////////////////////////////////////////////

	void generateCellValueResponse(const IdentifiersType &key, const CellValue &value, const vector<CellValue> &prop_vals);

	void generateDatabaseResponse(CPDatabase database);
	void generateDatabasesResponse(CPServer server, vector<CPDatabase>* databases);
	void generateCubeResponse(CPCube cube);
	void generateCubesResponse(CPDatabase database, vector<CPCube>* cubes, bool showNormal, bool showSystem, bool showAttribute, bool showInfo, bool showGputype);
	void generateDimensionResponse(CPDimension dimension);
	void generateDimensionsResponse(CPDatabase database, vector<CPDimension>* dimensions, bool showNormal, bool showSystem, bool showAttribute, bool showInfo);
	void generateElementResponse(CPDimension dimension, Element* element, bool showPermission);
	void generateElementsResponse(CPDimension dimension, ElementsType* elements, uint64_t *pHiddenCount);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a rule response
	////////////////////////////////////////////////////////////////////////////////

	void generateRuleResponse(CPCube cube, CPRule rule, bool useIdentifier);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a rule response
	////////////////////////////////////////////////////////////////////////////////

	void generateRulesResponse(CPCube cube, const vector<PRule>* rules, bool useIdentifier);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a lock response
	////////////////////////////////////////////////////////////////////////////////

	void generateLockResponse(CPServer server, CPCube cube, PLock lock, bool completeContainsArea = false);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructs a lock response
	////////////////////////////////////////////////////////////////////////////////

	void generateLocksResponse(CPServer server, CPCube cube, PUser user, bool completeContainsArea = false);

	void clear(bool setSession = true);


protected:

	template<class W> void assertParameter(const string& name, const W* value) {
		if (value == 0) {
			throw ParameterException(ErrorException::ERROR_PARAMETER_MISSING, "parameter missing", name, "");
		}
	}

protected:
	void appendCell(StringBuffer& body, const CellValue& value, bool showRule, bool showLockInfo, Cube::CellLockInfo lockInfo, const vector<CellValue> &prop_vals);
	void appendElement(StringBuffer* sb, CPDimension dimension, const Element* element, IdentifierType elemId, bool showPermission, vector<User::RoleDbCubeRight> &vRights, IdentifierType depth = NO_IDENTIFIER, IdentifierType indent = NO_IDENTIFIER);
	void appendError(StringBuffer& sb, ErrorException::ErrorType type, IdentifierType idRule, bool showRule, bool showLockInfo, const vector<CellValue> &prop_vals);
private:
	void appendCube(StringBuffer* sb, CPCube cube);
	void appendDatabase(StringBuffer* sb, CPDatabase database);
	void appendDimension(StringBuffer* sb, CPDimension dimension, RightsType dimRight);
	void appendRule(StringBuffer* sb, CPRule rule, CPCube cube, bool useIdentifier);
	void appendLock(CPServer server, StringBuffer* sb, CPLock lock, bool completeContainsArea);
	void appendArea(StringBuffer* sb, const vector<IdentifiersType> &area);
	void appendArea(StringBuffer* sb, CPArea area);
	void appendIdentifier(StringBuffer* sb, IdentifierType id);

private:
	static map<IdentifierType, PaloJob *> jobs;
	static Mutex m_main_Lock;
	static IdentifierType lastId;
	IdentifierType id;
	boost::posix_time::ptime startTime;
protected:
	HttpResponse* response;
	PaloJobRequest* jobRequest;

	PUser user;
	PServer server;
	PDatabase database;
	PCube cube;
	PCubeArea cellPath;
	PCubeArea cellPathTo;
	PDimension dimension;
	Element* element;
	vector<PRule> rules;
	PPaths cellPaths;
	PPaths lockedPaths;
	boost::shared_ptr<PaloSession> session;
	Context *context;
	string command;
	bool updateLics;

	friend class JobInfoCube;
	friend class JobInfoProcessor;
};

}

#endif
