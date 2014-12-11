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

#include "PaloDispatcher/PaloJobRequest.h"

#include "Olap/PaloSession.h"
#include "PaloDispatcher/PaloJob.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// constructors and destructors
// /////////////////////////////////////////////////////////////////////////////

PaloJobRequest::PaloJobRequest(const string& name) :
	HttpJobRequest(name)
{
	initialize();

	sid = PaloSession::NO_SESSION;
	hasSession = false;
}

PaloJobRequest::~PaloJobRequest()
{

	// tokens
	if (serverToken) {
		delete serverToken;
	}
	if (databaseToken) {
		delete databaseToken;
	}
	if (dimensionToken) {
		delete dimensionToken;
	}
	if (cubeToken) {
		delete cubeToken;
	}
	if (clientCacheToken) {
		delete clientCacheToken;
	}

	// strings
	if (action) {
		delete action;
	}
	if (comment) {
		delete comment;
	}
	if (condition) {
		delete condition;
	}
	if (cubeName) {
		delete cubeName;
	}
	if (databaseName) {
		delete databaseName;
	}
	if (definition) {
		delete definition;
	}
	if (dimensionName) {
		delete dimensionName;
	}
	if (elementName) {
		delete elementName;
	}
	if (event) {
		delete event;
	}
	if (externPassword) {
		delete externPassword;
	}
	if (externalIdentifier) {
		delete externalIdentifier;
	}
	if (functions) {
		delete functions;
	}
	if (newName) {
		delete newName;
	}
	if (password) {
		delete password;
	}
	if (source) {
		delete source;
	}
	if (user) {
		delete user;
	}
	if (value) {
		delete value;
	}

	// list of identifiers
	if (dimensions) {
		delete dimensions;
	}
	if (path) {
		delete path;
	}
	if (pathTo) {
		delete pathTo;
	}
	if (elements) {
		delete elements;
	}
	if (properties) {
		delete properties;
	}
	if (rule) {
		delete rules;
	}
	if (positions) {
		delete positions;
	}

	// list of unsigned integers
	if (types) {
		delete types;
	}

	// list of doubles
	if (weights) {
		delete weights;
	}
	if (dPositions) {
		delete dPositions;
	}

	// list of strings
	if (dimensionsName) {
		delete dimensionsName;
	}
	if (elementsName) {
		delete elementsName;
	}
	if (pathName) {
		delete pathName;
	}
	if (pathToName) {
		delete pathToName;
	}
	if (values) {
		delete values;
	}

	// list of list of identifiers
	if (children) {
		delete children;
	}
	if (paths) {
		delete paths;
	}
	if (lockedPaths) {
		delete lockedPaths;
	}

	// list of strings
	if (areaName) {
		delete areaName;
	}
	if (childrenName) {
		delete childrenName;
	}
	if (pathsName) {
		delete pathsName;
	}

	if (viewSubsets) {
		delete viewSubsets;
	}
	if (viewAxes) {
		delete viewAxes;
	}
	if (viewArea) {
		delete viewArea;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// Job methods
// /////////////////////////////////////////////////////////////////////////////

void PaloJobRequest::handleDone(Job* job)
{
	const bool isDebug = false;

	PaloJob * paloJob = dynamic_cast<PaloJob*>(job);

	if (paloJob != 0) {
		setResponse(paloJob->getResponse());

		if (isDebug) {
			Logger::debug << "PaloJob: Request: '" << this->getName().c_str() << "', Response: '" << this->getResponse()->getBody().c_str() << "'" << endl;
		}
	}

	done = true;
}

// /////////////////////////////////////////////////////////////////////////////
// private methods
// /////////////////////////////////////////////////////////////////////////////

void PaloJobRequest::initialize()
{
	response = 0;
	httpsPort = 0;

	// tokens
	serverToken = 0;
	databaseToken = 0;
	dimensionToken = 0;
	cubeToken = 0;
	clientCacheToken = 0;

	// identifiers
	cube = NO_IDENTIFIER;
	database = NO_IDENTIFIER;
	dimension = NO_IDENTIFIER;
	element = NO_IDENTIFIER;
	lock = NO_IDENTIFIER;
	rule = NO_IDENTIFIER;
	parent = ALL_IDENTIFIERS;
	limitStart = 0;
	limitCount = NO_IDENTIFIER;

	// booleans
	add = false;
	baseOnly = false;
	complete = false;
	eventProcess = true;
	showAttribute = false;
	showInfo = false;
	showGputype = true;
	showLockInfo = false;
	showNormal = true;
	showRule = false;
	showSystem = false;
	skipEmpty = true;
	useIdentifier = false;
	useRules = false;
	showUserInfo = false;
	showPermission = false;

	// strings
	action = 0;
	comment = 0;
	condition = 0;
	cubeName = 0;
	databaseName = 0;
	definition = 0;
	dimensionName = 0;
	elementName = 0;
	event = 0;
	externPassword = 0;
	externalIdentifier = 0;
	functions = 0;
	newName = 0;
	password = 0;
	source = 0;
	user = 0;
	value = 0;
	machineString = 0;
	optionalFeatures = 0;
	requiredFeatures = 0;
	lickey = 0;
	actcode = 0;

	blockSize = 1000;
	mode = 0;
	position = 0;
	splash = 1;
	steps = 0;
	type = 0;
	function = 0;
	activate = 1;

	// list of identifiers
	dimensions = 0;
	path = 0;
	pathTo = 0;
	elements = 0;
	properties = 0;
	rules = 0;
	positions = 0;

	// list unsigned integers
	types = 0;
	expand = 0;

	// list of doubles
	weights = 0;
	dPositions = 0;

	// list of strings
	dimensionsName = 0;
	elementsName = 0;
	pathName = 0;
	pathToName = 0;
	values = 0;

	// list of list of identifiers
	area = 0;
	children = 0;
	paths = 0;
	lockedPaths = 0;

	// list of strings
	areaName = 0;
	childrenName = 0;
	pathsName = 0;

	//view
	viewSubsets = 0;
	viewAxes = 0;
	viewArea = 0;
}
}
