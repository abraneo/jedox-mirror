/* 
 *
 * Copyright (C) 2006-2013 Jedox AG
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

#ifndef PALO_DISPATCHER_PALO_JOB_REQUEST_H
#define PALO_DISPATCHER_PALO_JOB_REQUEST_H 1

#include "palo.h"

#include "HttpServer/HttpJobRequest.h"
#include "Olap/PaloSession.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief palo job request
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS PaloJobRequest : public HttpJobRequest {

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	PaloJobRequest(const string& name);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	~PaloJobRequest();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief gets response
	////////////////////////////////////////////////////////////////////////////////

	HttpResponse* getResponse() {
		return response;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief sets response
	////////////////////////////////////////////////////////////////////////////////

	void setResponse(HttpResponse* response) {
		this->response = response;
	}

public:

	////////////////////////////////////////////////////////////////////////////////
	/// {@inheritDoc}
	////////////////////////////////////////////////////////////////////////////////

	void handleDone(Job*);

private:
	void initialize();

public:

	// session
	string sid;
	bool hasSession;
	string negotiationId;

	// https port
	int httpsPort;

	// directory paths
	string browserPath;

	// tokens
	uint32_t * serverToken;
	uint32_t * databaseToken;
	uint32_t * dimensionToken;
	uint32_t * cubeToken;
	uint32_t * clientCacheToken;

	// identifiers
	IdentifierType cube; // default NO_IDENTIFIER
	IdentifierType database; // default NO_IDENTIFIER
	IdentifierType dimension; // default NO_IDENTIFIER
	IdentifierType element; // default NO_IDENTIFIER
	IdentifierType lock; // default NO_IDENTIFIER
	IdentifierType rule; // default NO_IDENTIFIER
	IdentifierType parent; // default ALL_IDENTIFIERS
	IdentifierType limitStart; // default 0
	IdentifierType limitCount; // default NO_IDENTIFIER

	// booleans
	bool add; // default FALSE
	bool baseOnly; // default FALSE
	bool complete; // default FALSE
	bool eventProcess; // default TRUE
	bool showAttribute; // default FALSE
	bool showInfo; // default FALSE
	bool showGputype; // default TRUE
	bool showLockInfo; // default FALSE
	bool showNormal; // default TRUE
	bool showRule; // default FALSE
	bool showSystem; // default FALSE
	bool skipEmpty; // default TRUE
	bool useIdentifier; // default FALSE
	bool useRules; // default FALSE
	bool showUserInfo; // default FALSE

	// strings
	string * actcode;
	string * action;
	string * comment;
	string * condition;
	string * cubeName;
	string * databaseName;
	string * definition;
	string * dimensionName;
	string * elementName;
	string * event;
	string * externPassword;
	string * externalIdentifier;
	string * functions;
	string * lickey;
	string * machineString;
	string * newName;
	string * optionalFeatures;
	string * password;
	string * requiredFeatures;
	string * source;
	string * user;
	string * value;

	// unsigned integers
	uint32_t blockSize; // default 1000
	uint32_t mode; // default 0
	uint32_t position; // default 0
	uint32_t splash; // default 1
	uint32_t steps; // default 0
	uint32_t type; // default 0
	uint32_t function; // default 0
	uint32_t activate; // default 1=ACTIVE

	// list of identifiers
	IdentifiersType *dimensions;
	IdentifiersType *path;
	IdentifiersType *pathTo;
	IdentifiersType *elements;
	IdentifiersType *properties;
	IdentifiersType *rules;

	// list of unsigned integers
	vector<uint32_t> * types;
	vector<uint32_t> * expand;

	// list of doubles
	vector<vector<double> > * weights;

	// list of strings
	vector<string> * dimensionsName;
	vector<string> * elementsName;
	vector<string> * pathName;
	vector<string> * pathToName;
	vector<string> * values;

	// list of list of identifiers
	vector<IdentifiersType> *area;
	vector<IdentifiersType> *children;
	vector<IdentifiersType> *paths;
	vector<IdentifiersType> *lockedPaths;

	// list of strings
	vector<vector<string> > * areaName;
	vector<vector<string> > * childrenName;
	vector<vector<string> > * pathsName;

	string httpRequest;

private:
	HttpResponse* response;
};
}

#endif
