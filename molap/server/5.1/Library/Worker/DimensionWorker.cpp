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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Worker/DimensionWorker.h"
#include "Collections/StringUtils.h"
#include "Logger/Logger.h"
#include "Exceptions/WorkerException.h"

namespace palo {

// /////////////////////////////////////////////////////////////////////////////
// Worker methods
// /////////////////////////////////////////////////////////////////////////////

DimensionWorker::~DimensionWorker()
{
	terminate(false);
}

bool DimensionWorker::start()
{
	if (shutdownInProgress) {
		return false;
	}

	Logger::trace << "starting dimension worker" << endl;

	return Worker::startint();
}


ResultStatus DimensionWorker::notifyShutdown()
{
	shutdownInProgress = true;

	terminate(false);

	return RESULT_OK;
}


bool DimensionWorker::triggerDestroyElement(IdentifierType database, IdentifierType dimension, long &function)
{
	return triggerElement(database, dimension, function, mapElementDestroy);
}


bool DimensionWorker::triggerRenameElement(IdentifierType database, IdentifierType dimension, long &function)
{
	return triggerElement(database, dimension, function, mapElementRename);
}


bool DimensionWorker::triggerCreateElement(IdentifierType database, IdentifierType dimension, long &function)
{
	return triggerElement(database, dimension, function, mapElementCreate);
}


bool DimensionWorker::triggerElement(IdentifierType database, IdentifierType dimension, long &function, const MapDatabase2WatchDef &m)
{
	MapDatabase2WatchDef::const_iterator found = m.find(database);
	if (found != m.end()) {
		MapDimension2FunctionId::const_iterator found2 = found->second.find(dimension);
		if (found2 != found->second.end()) {
			function = found2->second;
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}


ResultStatus DimensionWorker::getDimensionsToWatch()
{
	ResultStatus status = getDimensionsToWatch(mapElementDestroy, "DEFINE ELEMENT DESTROY");
	if (status == RESULT_FAILED) {
		return status;
	}
	if (mapElementDestroy.empty()) {
		Logger::info << "no dimensions to watch for element/destroy" << endl;
	}

	status = getDimensionsToWatch(mapElementCreate, "DEFINE ELEMENT CREATE");
	if (status == RESULT_FAILED) {
		return status;
	}
	if (mapElementCreate.empty()) {
		Logger::info << "no dimensions to watch for element/create" << endl;
	}

	status = getDimensionsToWatch(mapElementRename, "DEFINE ELEMENT RENAME");
	if (status == RESULT_FAILED) {
		return status;
	}
	if (mapElementRename.empty()) {
		Logger::info << "no dimensions to watch for element/rename" << endl;
	}

	return status;
}


ResultStatus DimensionWorker::getDimensionsToWatch(MapDatabase2WatchDef &mDims, string command)
{
	message.clear();

	vector<string> result;

	// send request to dimension worker
	ResultStatus status = execute(command + ";", result);

	if (status == RESULT_FAILED) {
		return status;
	}

	// check result
	if (!result.empty()) {
		if (result.at(0).substr(0, 6) == "ERROR;") {
			message = result[0].substr(6);
			return RESULT_FAILED;
		} else if (isExceptionStatus(result)) {
			message = result[0].substr(10);
			Logger::error << "SVS initialization failed: " << message << endl;
			return RESULT_FAILED;
		} else {
			status = readDimensionLines(result, mDims);

			if (status == RESULT_FAILED) {
				return status;
			}
		}
	}

	return RESULT_OK;
}


ResultStatus DimensionWorker::readDimensionLines(const vector<string> &result, MapDatabase2WatchDef &mDims)
{
	for (vector<string>::const_iterator i = result.begin(); i != result.end(); ++i) {
		vector<string> values;
		StringUtils::splitString(*i, &values, ';');

		// check for AREA string
		if (values.size() == 4 && values[0] == "DIMENSION") {
			try {
				IdentifierType database = StringUtils::stringToInteger(values[1]);
				IdentifierType dimension = StringUtils::stringToInteger(values[2]);
				long functionId = StringUtils::stringToInteger(values[3]);
				mDims[database][dimension] = functionId;
			} catch (...) {
				Logger::error << "error in worker response DIMENSION: '" << *i << "'" << endl;
				return RESULT_FAILED;
			}
		} else {
			Logger::error << "error in worker response: '" << *i << "'" << endl;
			return RESULT_FAILED;
		}
	}

	return RESULT_OK;
}


ResultStatus DimensionWorker::notifyElementDestroyed(IdentifierType database, IdentifierType dimension, const string &elementName, long function, string session)
{
	vector<string> result;
	ResultStatus status = execute("ELEMENT DESTROYED;" + StringUtils::convertToString(database) + ";" + StringUtils::convertToString(dimension) + ";" + StringUtils::escapeString(elementName) + ";" + StringUtils::convertToString((uint32_t)function) + ";" + session + ";", result, WORKER_TIMEOUT_MSEC);

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	} else if (isExceptionStatus(result)) {
		throw WorkerException("SVS ElementDestroy trigger function failed", false);
	}

	return status;
}


ResultStatus DimensionWorker::notifyElementRenamed(IdentifierType database, IdentifierType dimension, const string &oldName, const string &newName, long function, string session)
{
	vector<string> result;
	ResultStatus status = execute("ELEMENT RENAMED;" + StringUtils::convertToString(database) + ";" + StringUtils::convertToString(dimension) + ";" + oldName + ";" + newName + ";" + StringUtils::convertToString((uint32_t)function) + ";" + session + ";", result, WORKER_TIMEOUT_MSEC);

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	} else if (isExceptionStatus(result)) {
		throw WorkerException("SVS ElementRename trigger function failed", false);
	}

	return status;
}


ResultStatus DimensionWorker::notifyElementCreated(IdentifierType database, IdentifierType dimension, IdentifierType element, long function, string session)
{
	vector<string> result;
	ResultStatus status = execute("ELEMENT CREATED;" + StringUtils::convertToString(database) + ";" + StringUtils::convertToString(dimension) + ";" + StringUtils::convertToString(element) + ";" + StringUtils::convertToString((uint32_t)function) + ";" + session + ";", result, WORKER_TIMEOUT_MSEC);

	if (isErrorStatus(result)) {
		throw WorkerException(result[0].substr(6), true);
	} else if (isExceptionStatus(result)) {
		throw WorkerException("SVS ElementCreate trigger function failed", false);
	}

	return status;
}


}
