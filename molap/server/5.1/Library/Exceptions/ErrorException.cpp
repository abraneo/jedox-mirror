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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * \author Ladislav Morva, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#include "Exceptions/ErrorException.h"

namespace palo {

string ErrorException::getDescriptionErrorType(ErrorType type)
{
	switch (type) {
	case ERROR_UNKNOWN:
		return "unknown";

	case ERROR_INTERNAL:
		return "internal error";

	case ERROR_ID_NOT_FOUND:
		return "identifier not found";

	case ERROR_INVALID_FILENAME:
		return "invalid filename";

	case ERROR_MKDIR_FAILED:
		return "cannot create directory";

	case ERROR_RENAME_FAILED:
		return "cannot rename file";

	case ERROR_AUTHORIZATION_FAILED:
		return "authorization failed";

	case ERROR_WORKER_AUTHORIZATION_FAILED:
		return "worker authorization failed";

	case ERROR_WORKER_MESSAGE:
		return "worker error";

	case ERROR_INVALID_TYPE:
		return "invalid type";

	case ERROR_INVALID_COORDINATES:
		return "invalid coordinates";

	case ERROR_CONVERSION_FAILED:
		return "conversion failed";

	case ERROR_FILE_NOT_FOUND_ERROR:
		return "file not found";

	case ERROR_NOT_AUTHORIZED:
		return "not authorized for operation";

	case ERROR_CORRUPT_FILE:
		return "corrupt file";

	case ERROR_API_CALL_NOT_IMPLEMENTED:
		return "api call not implemented";

	case ERROR_HTTP_DISABLED:
		return "insecure communication disabled";

	case ERROR_OUT_OF_MEMORY:
		return "not enough memory";

	case ERROR_SSL_FAILED:
		return "ssl failed";

	case ERROR_GPU_SERVER_NOT_ENABLED:
		return "gpu server not enabled";

	case ERROR_INVALID_STRING:
		return "invalid string";

	case ERROR_INVALID_VERSION:
		return "invalid version";

	case ERROR_INVALID_AGGR_FUNCTION:
		return "invalid function";

	case ERROR_INVALID_EXPAND:
		return "invalid expand type";

	case ERROR_INVALID_PREDICT_FUNCTION:
		return "invalid function";

	case ERROR_INVALID_PREDICT_AREA:
		return "invalid area";

	case ERROR_SSO_FAILED:
		return "SSO authentication failed";

	case ERROR_WITHIN_EVENT:
		return "already within event";

	case ERROR_NOT_WITHIN_EVENT:
		return "not within event";

	case ERROR_INVALID_PERMISSION:
		return "invalid permission entry";

	case ERROR_INVALID_SERVER_PATH:
		return "invalid server path";

	case ERROR_INVALID_SESSION:
		return "invalid session";

	case ERROR_PARAMETER_MISSING:
		return "missing parameter";

	case ERROR_PARSING_RULE:
		return "parse error in rule";

	case ERROR_RULE_NOT_FOUND:
		return "rule not found";

	case ERROR_RULE_HAS_CIRCULAR_REF:
		return "rule has circular reference";

	case ERROR_INVALID_SPLASH_MODE:
		return "invalid splash mode";

	case ERROR_SERVER_TOKEN_OUTDATED:
		return "server token outdated";

	case ERROR_COPY_FAILED:
		return "copy operation failed";

	case ERROR_ZIP:
		return "zip operation failed";

	case ERROR_INVALID_DATABASE_NAME:
		return "invalid database name";

	case ERROR_DATABASE_NOT_FOUND:
		return "database not found";

	case ERROR_DATABASE_NOT_LOADED:
		return "database not loaded";

	case ERROR_DATABASE_UNSAVED:
		return "database not saved";

	case ERROR_DATABASE_STILL_LOADED:
		return "database still loaded";

	case ERROR_DATABASE_NAME_IN_USE:
		return "database name in use";

	case ERROR_DATABASE_UNDELETABLE:
		return "database is not deletable";

	case ERROR_DATABASE_UNRENAMABLE:
		return "database in not renamable";

	case ERROR_DATABASE_TOKEN_OUTDATED:
		return "database token outdated";

	case ERROR_DIMENSION_EMPTY:
		return "dimension empty";

	case ERROR_DIMENSION_EXISTS:
		return "dimension already exists";

	case ERROR_DIMENSION_NOT_FOUND:
		return "dimension not found";

	case ERROR_INVALID_DIMENSION_NAME:
		return "invalid dimension name";

	case ERROR_DIMENSION_UNCHANGABLE:
		return "dimension is not changable";

	case ERROR_DIMENSION_NAME_IN_USE:
		return "dimension name in use";

	case ERROR_DIMENSION_IN_USE:
		return "dimension in use";

	case ERROR_DIMENSION_UNDELETABLE:
		return "dimension not deletable";

	case ERROR_DIMENSION_UNRENAMABLE:
		return "dimension not renamable";

	case ERROR_DIMENSION_TOKEN_OUTDATED:
		return "dimension token outdated";

	case ERROR_DIMENSION_LOCKED:
		return "dimension is locked";

	case ERROR_ELEMENT_EXISTS:
		return "element already exists";

	case ERROR_ELEMENT_CIRCULAR_REFERENCE:
		return "cirular reference";

	case ERROR_ELEMENT_NAME_IN_USE:
		return "element name in use";

	case ERROR_ELEMENT_NAME_NOT_UNIQUE:
		return "element name not unique";

	case ERROR_ELEMENT_NOT_FOUND:
		return "element not found";

	case ERROR_ELEMENT_NO_CHILD_OF:
		return "element is no child";

	case ERROR_INVALID_ELEMENT_NAME:
		return "invalid element name";

	case ERROR_INVALID_OFFSET:
		return "invalid element offset";

	case ERROR_INVALID_ELEMENT_TYPE:
		return "invalid element type";

	case ERROR_INVALID_POSITION:
		return "invalid element position";

	case ERROR_INVALID_MODE:
		return "invalid mode";

	case ERROR_ELEMENT_NOT_DELETABLE:
		return "element not deletable";

	case ERROR_ELEMENT_NOT_RENAMABLE:
		return "element not renamable";

	case ERROR_ELEMENT_NOT_CHANGABLE:
		return "element not changable";

	case ERROR_CUBE_NOT_FOUND:
		return "cube not found";

	case ERROR_INVALID_CUBE_NAME:
		return "invalid cube name";

	case ERROR_INVALID_CUBE_TYPE:
		return "invalid cube type";

	case ERROR_DEFRAGMENTATION:
		return "defragmentation error";

	case ERROR_CUBE_NOT_LOADED:
		return "cube not loaded";

	case ERROR_CUBE_EMPTY:
		return "cube empty";

	case ERROR_CUBE_UNSAVED:
		return "cube not saved";

	case ERROR_CUBE_LOCK_NOT_FOUND:
		return "cube lock not found";

	case ERROR_CUBE_IS_SYSTEM_CUBE:
		return "cube is system cube";

	case ERROR_COPY_NOT_POSSIBLE:
		return "copy operation not possible";

	case ERROR_CUBE_WRONG_USER:
		return "wrong user for locked area";

	case ERROR_CUBE_WRONG_LOCK:
		return "could not create lock";

	case ERROR_CUBE_BLOCKED_BY_LOCK:
		return "is blocked because of a locked area";

	case ERROR_CUBE_LOCK_NO_CAPACITY:
		return "not enough rollback capacity";

	case ERROR_GOALSEEK:
		return "goalseek error";

	case ERROR_SPLASH_DISABLED:
		return "splash disabled";

	case ERROR_COPY_PATH_NOT_NUMERIC:
		return "copy path must be numeric";

	case ERROR_INVALID_COPY_VALUE:
		return "invalid copy value";

	case ERROR_CUBE_NAME_IN_USE:
		return "cube name in use";

	case ERROR_CUBE_UNDELETABLE:
		return "cube is not deletable";

	case ERROR_CUBE_UNRENAMABLE:
		return "cube is not renamable";

	case ERROR_CUBE_TOKEN_OUTDATED:
		return "cube token outdated";

	case ERROR_SPLASH_NOT_POSSIBLE:
		return "splashing is not possible";

	case ERROR_NET_ARG:
		return "legacy error";

	case ERROR_INV_CMD:
		return "legacy error";

	case ERROR_INV_CMD_CTL:
		return "legacy error";

	case ERROR_NET_SEND:
		return "legacy error";

	case ERROR_NET_CONN_TERM:
		return "legacy error";

	case ERROR_NET_RECV:
		return "legacy error";

	case ERROR_NET_HS_HALLO:
		return "legacy error";

	case ERROR_NET_HS_PROTO:
		return "legacy error";

	case ERROR_INV_ARG_COUNT:
		return "legacy error";

	case ERROR_INV_ARG_TYPE:
		return "legacy error";

	case ERROR_CLIENT_INV_NET_REPLY:
		return "legacy error";

	case ERROR_INVALID_WORKER_REPLY:
		return "illegal worker response";

	case ERROR_WORKER_OPERATION:
		return "invalid worker operation";

	case ERROR_INVALID_DATABASE_TYPE:
		return "invalid database type";

	case ERROR_DATABASE_BACKUP:
		return "database backup error";

	case ERROR_RULE_DIVISION_BY_ZERO:
		return "division by zero";

	case ERROR_MAX_CELL_REACHED:
		return "reached maximum cell count";

	case ERROR_MAX_ELEM_REACHED:
		return "reached maximum elem count";

	case ERROR_COMMIT_OBJECTNOTCHECKEDOUT:
		return "Object is not checked out";

	case ERROR_COMMIT_OBJECTCHECKEDOUT:
		return "Object is already checked out";

	case ERROR_COMMIT_CANTCOMMIT:
		return "Commit failed.";

	case ERROR_GPU_CANCELED_READ_REQUEST:
		return "read request canceled on GPU";

	case ERROR_GPU_CANCELED_WRITE_REQUEST:
		return "write request canceled on GPU";

	case ERROR_GPU_MEMORY_FULL:
		return "GPU memory full";

	case ERROR_CUDA_RUNTIME:
		return "cuda runtime api error";

	case ERROR_STOPPED_BY_ADMIN:
		return "job stopped";

	case ERROR_INVALID_COMMAND:
		return "invalid job/session command received";

	case ERROR_LICENSE_EXPIRED:
		return "license expired";

	case ERROR_LICENSE_NOVALID:
		return "no valid license found";

	case ERROR_LICENSE_NOFREENAMED:
		return "no free named license";

	case ERROR_LICENSE_NOKEY:
		return "no license key specified";

	case ERROR_LICENSE_NOACTCODE:
		return "no activation code specified";

	case ERROR_LICENSE_ACTCODEINVALID:
		return "activation code invalid";

	case ERROR_LICENSE_ALREADYACTIVATED:
		return "license already activated";

	case ERROR_LICENSE_OVERUSED:
		return "no free concurrent license";

	case ERROR_LICENSE_NOAPI:
		return "license API not supported";
}

	return "internal error in ErrorException::getDescriptionErrorType";
}

string ErrorException::getVerboseDescriptionErrorType(ErrorType type)
{
	switch (type) {
	case ERROR_UNKNOWN:
		return "unknown";

	case ERROR_INTERNAL:
		return "internal error";

	case ERROR_ID_NOT_FOUND:
		return "identifier not found";

	case ERROR_INVALID_FILENAME:
		return "invalid filename";

	case ERROR_MKDIR_FAILED:
		return "cannot create directory";

	case ERROR_RENAME_FAILED:
		return "cannot rename file";

	case ERROR_AUTHORIZATION_FAILED:
		return "authorization failed";

	case ERROR_WORKER_AUTHORIZATION_FAILED:
		return "worker authorization failed";

	case ERROR_WORKER_MESSAGE:
		return "worker error";

	case ERROR_INVALID_TYPE:
		return "invalid type";

	case ERROR_INVALID_COORDINATES:
		return "invalid coordinates";

	case ERROR_CONVERSION_FAILED:
		return "conversion failed";

	case ERROR_FILE_NOT_FOUND_ERROR:
		return "file not found";

	case ERROR_NOT_AUTHORIZED:
		return "not authorized for operation";

	case ERROR_CORRUPT_FILE:
		return "corrupt file";

	case ERROR_API_CALL_NOT_IMPLEMENTED:
		return "api call not implemented";

	case ERROR_HTTP_DISABLED:
		return "insecure communication disabled";

	case ERROR_OUT_OF_MEMORY:
		return "not enough memory";

	case ERROR_SSL_FAILED:
		return "ssl failed";

	case ERROR_GPU_SERVER_NOT_ENABLED:
		return "gpu server not enabled";

	case ERROR_INVALID_STRING:
		return "invalid characters in string value";

	case ERROR_INVALID_VERSION:
		return "old version used";

	case ERROR_INVALID_AGGR_FUNCTION:
		return "invalid aggregation function";

	case ERROR_INVALID_EXPAND:
		return "invalid aggregation expand type";

	case ERROR_INVALID_PREDICT_FUNCTION:
		return "invalid predictive function";

	case ERROR_INVALID_PREDICT_AREA:
		return "invalid predictive area";

	case ERROR_SSO_FAILED:
		return "SSO authentication failed";

	case ERROR_WITHIN_EVENT:
		return "already within event";

	case ERROR_NOT_WITHIN_EVENT:
		return "not within event";

	case ERROR_INVALID_PERMISSION:
		return "invalid permission entry";

	case ERROR_INVALID_SERVER_PATH:
		return "invalid server path";

	case ERROR_INVALID_SESSION:
		return "invalid session";

	case ERROR_PARAMETER_MISSING:
		return "missing parameter";

	case ERROR_PARSING_RULE:
		return "parse error in rule";

	case ERROR_RULE_NOT_FOUND:
		return "rule not found";

	case ERROR_RULE_HAS_CIRCULAR_REF:
		return "rule has circular reference";

	case ERROR_INVALID_SPLASH_MODE:
		return "invalid splash mode";

	case ERROR_SERVER_TOKEN_OUTDATED:
		return "server token outdated";

	case ERROR_COPY_FAILED:
		return "copy operation failed";

	case ERROR_ZIP:
		return "zip operation failed";

	case ERROR_INVALID_DATABASE_NAME:
		return "invalid database name";

	case ERROR_DATABASE_NOT_FOUND:
		return "database not found";

	case ERROR_DATABASE_NOT_LOADED:
		return "database not loaded";

	case ERROR_DATABASE_UNSAVED:
		return "database not saved";

	case ERROR_DATABASE_STILL_LOADED:
		return "database still loaded";

	case ERROR_DATABASE_NAME_IN_USE:
		return "database name in use";

	case ERROR_DATABASE_UNDELETABLE:
		return "database is not deletable";

	case ERROR_DATABASE_UNRENAMABLE:
		return "database in not renamable";

	case ERROR_DATABASE_TOKEN_OUTDATED:
		return "database token outdated";

	case ERROR_DIMENSION_EMPTY:
		return "dimension empty";

	case ERROR_DIMENSION_EXISTS:
		return "dimension already exists";

	case ERROR_DIMENSION_NOT_FOUND:
		return "dimension not found";

	case ERROR_INVALID_DIMENSION_NAME:
		return "invalid dimension name";

	case ERROR_DIMENSION_UNCHANGABLE:
		return "dimension is not changable";

	case ERROR_DIMENSION_NAME_IN_USE:
		return "dimension name in use";

	case ERROR_DIMENSION_IN_USE:
		return "dimension in use";

	case ERROR_DIMENSION_UNDELETABLE:
		return "dimension not deletable";

	case ERROR_DIMENSION_UNRENAMABLE:
		return "dimension not renamable";

	case ERROR_DIMENSION_TOKEN_OUTDATED:
		return "dimension token outdated";

	case ERROR_DIMENSION_LOCKED:
		return "dimension is locked";

	case ERROR_ELEMENT_EXISTS:
		return "element already exists";

	case ERROR_ELEMENT_CIRCULAR_REFERENCE:
		return "cirular reference";

	case ERROR_ELEMENT_NAME_IN_USE:
		return "element name in use";

	case ERROR_ELEMENT_NAME_NOT_UNIQUE:
		return "element name not unique";

	case ERROR_ELEMENT_NOT_FOUND:
		return "element not found";

	case ERROR_ELEMENT_NO_CHILD_OF:
		return "element is no child";

	case ERROR_INVALID_ELEMENT_NAME:
		return "invalid element name";

	case ERROR_INVALID_OFFSET:
		return "invalid element offset";

	case ERROR_INVALID_ELEMENT_TYPE:
		return "invalid element type";

	case ERROR_INVALID_POSITION:
		return "invalid element position";

	case ERROR_INVALID_MODE:
		return "invalid mode";

	case ERROR_ELEMENT_NOT_DELETABLE:
		return "element not deletable";

	case ERROR_ELEMENT_NOT_RENAMABLE:
		return "element not renamable";

	case ERROR_ELEMENT_NOT_CHANGABLE:
		return "element not changable";

	case ERROR_CUBE_NOT_FOUND:
		return "cube not found";

	case ERROR_INVALID_CUBE_NAME:
		return "invalid cube name";

	case ERROR_INVALID_CUBE_TYPE:
		return "invalid cube type";

	case ERROR_DEFRAGMENTATION:
		return "defragmentation error";

	case ERROR_CUBE_NOT_LOADED:
		return "cube not loaded";

	case ERROR_CUBE_EMPTY:
		return "cube empty";

	case ERROR_CUBE_UNSAVED:
		return "cube not saved";

	case ERROR_CUBE_LOCK_NOT_FOUND:
		return "cube lock not found";

	case ERROR_CUBE_IS_SYSTEM_CUBE:
		return "cube is system cube";

	case ERROR_COPY_NOT_POSSIBLE:
		return "copy operation not possible";

	case ERROR_CUBE_WRONG_USER:
		return "wrong user for locked area";

	case ERROR_CUBE_WRONG_LOCK:
		return "could not create lock";

	case ERROR_CUBE_BLOCKED_BY_LOCK:
		return "is blocked because of a locked area";

	case ERROR_CUBE_LOCK_NO_CAPACITY:
		return "not enough rollback capacity";

	case ERROR_GOALSEEK:
		return "goalseek error";

	case ERROR_SPLASH_DISABLED:
		return "splash disabled";

	case ERROR_COPY_PATH_NOT_NUMERIC:
		return "copy path must be numeric";

	case ERROR_INVALID_COPY_VALUE:
		return "invalid copy value";

	case ERROR_CUBE_NAME_IN_USE:
		return "cube name in use";

	case ERROR_CUBE_UNDELETABLE:
		return "cube is not deletable";

	case ERROR_CUBE_UNRENAMABLE:
		return "cube is not renamable";

	case ERROR_CUBE_TOKEN_OUTDATED:
		return "cube token outdated";

	case ERROR_SPLASH_NOT_POSSIBLE:
		return "splashing is not possible";

	case ERROR_NET_ARG:
		return "legacy error";

	case ERROR_INV_CMD:
		return "legacy error";

	case ERROR_INV_CMD_CTL:
		return "legacy error";

	case ERROR_NET_SEND:
		return "legacy error";

	case ERROR_NET_CONN_TERM:
		return "legacy error";

	case ERROR_NET_RECV:
		return "legacy error";

	case ERROR_NET_HS_HALLO:
		return "legacy error";

	case ERROR_NET_HS_PROTO:
		return "legacy error";

	case ERROR_INV_ARG_COUNT:
		return "legacy error";

	case ERROR_INV_ARG_TYPE:
		return "legacy error";

	case ERROR_CLIENT_INV_NET_REPLY:
		return "legacy error";

	case ERROR_INVALID_WORKER_REPLY:
		return "illegal worker response";

	case ERROR_WORKER_OPERATION:
		return "invalid worker operation";

	case ERROR_INVALID_DATABASE_TYPE:
		return "invalid database type";

	case ERROR_DATABASE_BACKUP:
		return "database backup error";

	case ERROR_RULE_DIVISION_BY_ZERO:
		return "division by zero in rule";

	case ERROR_MAX_CELL_REACHED:
		return "reached maximum cell count";

	case ERROR_MAX_ELEM_REACHED:
		return "reached maximum elem count";

	case ERROR_COMMIT_OBJECTNOTCHECKEDOUT:
		return "Object is not checked out";

	case ERROR_COMMIT_OBJECTCHECKEDOUT:
		return "Object is already checked out";

	case ERROR_COMMIT_CANTCOMMIT:
		return "Commit failed.";

	case ERROR_GPU_CANCELED_READ_REQUEST:
		return "read request canceled on GPU";

	case ERROR_GPU_CANCELED_WRITE_REQUEST:
		return "write request canceled on GPU";

	case ERROR_GPU_MEMORY_FULL:
		return "GPU memory full";

	case ERROR_CUDA_RUNTIME:
		return "cuda runtime api error";

	case ERROR_STOPPED_BY_ADMIN:
		return "job stopped";

	case ERROR_INVALID_COMMAND:
		return "invalid job/session command received";

	case ERROR_LICENSE_EXPIRED:
		return "license expired";

	case ERROR_LICENSE_NOVALID:
		return "no valid license found";

	case ERROR_LICENSE_NOFREENAMED:
		return "no free named license";

	case ERROR_LICENSE_NOKEY:
		return "no license key specified";

	case ERROR_LICENSE_NOACTCODE:
		return "no activation code specified";

	case ERROR_LICENSE_ACTCODEINVALID:
		return "activation code invalid";

	case ERROR_LICENSE_ALREADYACTIVATED:
		return "license already activated";

	case ERROR_LICENSE_OVERUSED:
		return "no free concurrent license";

	case ERROR_LICENSE_NOAPI:
		return "license API not supported";
	}

	return "internal error in ErrorException::getDescriptionErrorType";
}

ErrorException::ErrorException(ErrorType type) :
	type(type), ruleId(0)
{
}

ErrorException::ErrorException(ErrorType type, const string& message) :
	type(type), message(message), ruleId(0)
{
}

ErrorException::ErrorException(ErrorType type, const string& message, uint32_t ruleId) :
	type(type), message(message), ruleId(ruleId)
{
}

ErrorException::ErrorException(ErrorType type, const string& message, const string& details, uint32_t ruleId) :
	type(type), message(message), details(details), ruleId(ruleId)
{
}

ErrorException::~ErrorException()
{
}

const string& ErrorException::getMessage() const
{
	return message;
}

const string& ErrorException::getDetails() const
{
	return details;
}

ErrorException::ErrorType ErrorException::getErrorType() const
{
	return type;
}

}
