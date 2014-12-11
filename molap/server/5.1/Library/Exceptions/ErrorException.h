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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef EXCEPTIONS_ERROR_EXCEPTION_H
#define EXCEPTIONS_ERROR_EXCEPTION_H 1

#include "palo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief base class for error exceptions
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS ErrorException {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief error number
	////////////////////////////////////////////////////////////////////////////////

	enum ErrorType {
		ERROR_UNKNOWN = 1, ERROR_INTERNAL = 2,

		// 100-999 is used by libpalo_ng

		// general error messages
		ERROR_ID_NOT_FOUND = 1000,
		ERROR_INVALID_FILENAME = 1001,
		ERROR_MKDIR_FAILED = 1002,
		ERROR_RENAME_FAILED = 1003,
		ERROR_AUTHORIZATION_FAILED = 1004,
		ERROR_INVALID_TYPE = 1005,
		ERROR_INVALID_COORDINATES = 1006,
		ERROR_CONVERSION_FAILED = 1007,
		ERROR_FILE_NOT_FOUND_ERROR = 1008,
		ERROR_NOT_AUTHORIZED = 1009,
		ERROR_CORRUPT_FILE = 1010,
		ERROR_WITHIN_EVENT = 1011,
		ERROR_NOT_WITHIN_EVENT = 1012,
		ERROR_INVALID_PERMISSION = 1013,
		ERROR_INVALID_SERVER_PATH = 1014,
		ERROR_INVALID_SESSION = 1015,
		ERROR_PARAMETER_MISSING = 1016,
		ERROR_SERVER_TOKEN_OUTDATED = 1017,
		ERROR_INVALID_SPLASH_MODE = 1018,
		ERROR_WORKER_AUTHORIZATION_FAILED = 1019,
		ERROR_WORKER_MESSAGE = 1020,
		ERROR_API_CALL_NOT_IMPLEMENTED = 1021,
		ERROR_HTTP_DISABLED = 1022,
		ERROR_OUT_OF_MEMORY = 1023,
		ERROR_SSL_FAILED = 1024,
		ERROR_GPU_SERVER_NOT_ENABLED = 1025,
		ERROR_INVALID_STRING = 1026,
		ERROR_INVALID_VERSION = 1027,
		ERROR_INVALID_AGGR_FUNCTION = 1028,
		ERROR_INVALID_EXPAND = 1029,
		ERROR_INVALID_PREDICT_FUNCTION = 1030,
		ERROR_INVALID_PREDICT_AREA = 1031,
		ERROR_SSO_FAILED = 1032,
		ERROR_COPY_FAILED = 1033,
		ERROR_ZIP = 1034,

		// database
		ERROR_INVALID_DATABASE_NAME = 2000,
		ERROR_DATABASE_NOT_FOUND = 2001,
		ERROR_DATABASE_NOT_LOADED = 2002,
		ERROR_DATABASE_UNSAVED = 2003,
		ERROR_DATABASE_STILL_LOADED = 2004,
		ERROR_DATABASE_NAME_IN_USE = 2005,
		ERROR_DATABASE_UNDELETABLE = 2006,
		ERROR_DATABASE_UNRENAMABLE = 2007,
		ERROR_DATABASE_TOKEN_OUTDATED = 2008,
		ERROR_INVALID_DATABASE_TYPE = 2009,
		ERROR_DATABASE_BACKUP = 2010,

		// dimension related errors
		ERROR_DIMENSION_EMPTY = 3000,
		ERROR_DIMENSION_EXISTS = 3001,
		ERROR_DIMENSION_NOT_FOUND = 3002,
		ERROR_INVALID_DIMENSION_NAME = 3003,
		ERROR_DIMENSION_UNCHANGABLE = 3004,
		ERROR_DIMENSION_NAME_IN_USE = 3005,
		ERROR_DIMENSION_IN_USE = 3006,
		ERROR_DIMENSION_UNDELETABLE = 3007,
		ERROR_DIMENSION_UNRENAMABLE = 3008,
		ERROR_DIMENSION_TOKEN_OUTDATED = 3009,
		ERROR_DIMENSION_LOCKED = 3010,

		// dimension element related errors
		ERROR_ELEMENT_EXISTS = 4000,
		ERROR_ELEMENT_CIRCULAR_REFERENCE = 4001,
		ERROR_ELEMENT_NAME_IN_USE = 4002,
		ERROR_ELEMENT_NAME_NOT_UNIQUE = 4003,
		ERROR_ELEMENT_NOT_FOUND = 4004,
		ERROR_ELEMENT_NO_CHILD_OF = 4005,
		ERROR_INVALID_ELEMENT_NAME = 4006,
		ERROR_INVALID_OFFSET = 4007,
		ERROR_INVALID_ELEMENT_TYPE = 4008,
		ERROR_INVALID_POSITION = 4009,
		ERROR_ELEMENT_NOT_DELETABLE = 4010,
		ERROR_ELEMENT_NOT_RENAMABLE = 4011,
		ERROR_ELEMENT_NOT_CHANGABLE = 4012,
		ERROR_INVALID_MODE = 4013,

		// cube related errors
		ERROR_CUBE_NOT_FOUND = 5000,
		ERROR_INVALID_CUBE_NAME = 5001,
		ERROR_CUBE_NOT_LOADED = 5002,
		ERROR_CUBE_EMPTY = 5003,
		ERROR_CUBE_UNSAVED = 5004,
		ERROR_SPLASH_DISABLED = 5005,
		ERROR_COPY_PATH_NOT_NUMERIC = 5006,
		ERROR_INVALID_COPY_VALUE = 5007,
		ERROR_CUBE_NAME_IN_USE = 5008,
		ERROR_CUBE_UNDELETABLE = 5009,
		ERROR_CUBE_UNRENAMABLE = 5010,
		ERROR_CUBE_TOKEN_OUTDATED = 5011,
		ERROR_SPLASH_NOT_POSSIBLE = 5012,
		ERROR_CUBE_LOCK_NOT_FOUND = 5013,
		ERROR_CUBE_WRONG_USER = 5014,
		ERROR_CUBE_WRONG_LOCK = 5015,
		ERROR_CUBE_BLOCKED_BY_LOCK = 5016,
		ERROR_CUBE_LOCK_NO_CAPACITY = 5017,
		ERROR_GOALSEEK = 5018,
		ERROR_CUBE_IS_SYSTEM_CUBE = 5019,
		ERROR_COPY_NOT_POSSIBLE = 5020,
		ERROR_INVALID_CUBE_TYPE = 5021,
		ERROR_DEFRAGMENTATION = 5022,

		// legacy interface
		ERROR_NET_ARG = 6000,
		ERROR_INV_CMD = 6001,
		ERROR_INV_CMD_CTL = 6002,
		ERROR_NET_SEND = 6003,
		ERROR_NET_CONN_TERM = 6004,
		ERROR_NET_RECV = 6005,
		ERROR_NET_HS_HALLO = 6006,
		ERROR_NET_HS_PROTO = 6007,
		ERROR_INV_ARG_COUNT = 6008,
		ERROR_INV_ARG_TYPE = 6009,
		ERROR_CLIENT_INV_NET_REPLY = 6010,

		// worker
		ERROR_INVALID_WORKER_REPLY = 7000,
		ERROR_WORKER_OPERATION = 7001,

		// rules
		ERROR_PARSING_RULE = 8001,
		ERROR_RULE_NOT_FOUND = 8002,
		ERROR_RULE_HAS_CIRCULAR_REF = 8003,
		ERROR_RULE_DIVISION_BY_ZERO = 8004,

		// commit
		ERROR_COMMIT_OBJECTNOTCHECKEDOUT = 9000,
		ERROR_COMMIT_OBJECTCHECKEDOUT = 9001,
		ERROR_COMMIT_CANTCOMMIT = 9002,

		// limits
		ERROR_MAX_CELL_REACHED = 90001,
		ERROR_MAX_ELEM_REACHED = 90002,

		// gpu engine
		ERROR_GPU_CANCELED_READ_REQUEST = 10000,
		ERROR_GPU_CANCELED_WRITE_REQUEST = 10001,
		ERROR_CUDA_RUNTIME = 10002,
		ERROR_GPU_MEMORY_FULL = 10003,

		// job management
		ERROR_STOPPED_BY_ADMIN = 11000,
		ERROR_INVALID_COMMAND = 11001,

		// licensing
		ERROR_LICENSE_EXPIRED = 12000,
		ERROR_LICENSE_NOVALID = 12001,
		ERROR_LICENSE_NOFREENAMED = 12002,
		ERROR_LICENSE_NOKEY = 12003,
		ERROR_LICENSE_NOACTCODE = 12004,
		ERROR_LICENSE_ACTCODEINVALID = 12005,
		ERROR_LICENSE_ALREADYACTIVATED = 12006,
		ERROR_LICENSE_OVERUSED = 12007,
		ERROR_LICENSE_NOAPI = 12008,
};

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief descriptive text for error number
	////////////////////////////////////////////////////////////////////////////////

	static string getDescriptionErrorType(ErrorType type);

	static string getVerboseDescriptionErrorType(ErrorType type);

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief constructor
	////////////////////////////////////////////////////////////////////////////////

	ErrorException(ErrorType type);

	ErrorException(ErrorType type, const string& message);

	ErrorException(ErrorType type, const string& message, uint32_t ruleId);

	ErrorException(ErrorType type, const string& message, const string& details, uint32_t ruleId);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief destructor
	////////////////////////////////////////////////////////////////////////////////

	virtual ~ErrorException();

public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get error message
	////////////////////////////////////////////////////////////////////////////////

	virtual const string& getMessage() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get error details
	////////////////////////////////////////////////////////////////////////////////

	virtual const string& getDetails() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get error type
	////////////////////////////////////////////////////////////////////////////////

	virtual ErrorType getErrorType() const;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief get rule Id
	////////////////////////////////////////////////////////////////////////////////

	virtual uint32_t getRuleId() const {
		return ruleId;
	}
	;

protected:
	ErrorType type;
	string message;
	string details;
	uint32_t ruleId;
};

}

#endif
