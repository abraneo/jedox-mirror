/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 *
 */

/*
 To seperate exception handling from the rest and to make future extensions easier,
 we use an exception-factory for all libpalo_ng related exceptions.
 This strategy centralizes the exception handling and makes maintainance easier, too.
 */

#include <libpalo_ng/Palo/Exception/LibPaloNGExceptionFactory.h>
#include "PaloNGTypeErrorException.h"
#include "PaloNGGeneralException.h"

namespace jedox {
namespace palo {

void LibPaloNGExceptionFactory::raise(unsigned int errorCode, const std::string *descr)
{
	switch (errorCode) {

	case PALO_NG_ERROR_UNDEFINED_TYPE:
		throw PaloNGTypeErrorException("CSV value is undefined", "Type Error", errorCode);

	case PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE:
		throw PaloNGTypeErrorException("Serializer Type does not exist", "Undefined Serializer", errorCode);

	case PALO_NG_ERROR_UNDEFINED_DBINFOTYPE:
		throw PaloNGTypeErrorException("Received Database Info Type is not defined", "Undefined DB Info", errorCode);

	case PALO_NG_ERROR_UNDEFINED_ELEMENTINFOTYPE:
		throw PaloNGTypeErrorException("Received Element Info Type is not defined", "Undefined Element Info", errorCode);

	case PALO_NG_ERROR_UNDEFINED_CUBEINFOTYPE:
		throw PaloNGTypeErrorException("Received Cube Info Type is not defined", "Undefined Cube Info", errorCode);

	case PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE:
		throw PaloNGTypeErrorException("Received Cell Value Type is not defined", "Undefined Cell Value Info", errorCode);

	case PALO_NG_ERROR_UNDEFINED_CELLVALUEPATHTYPE:
		throw PaloNGTypeErrorException("Received Cell Value Path Type is not defined", "Undefined Cell Value Path", errorCode);

	case PALO_NG_ERROR_ONLINECACHE:
		throw PaloNGTypeErrorException("A problem with the online cache occured", "online cache error", errorCode);

	case PALO_NG_ERROR_OFFLINECACHE:
		throw PaloNGTypeErrorException(descr ? *descr : PALO_NG_ERROR_OFFLINECACHE_MSG, PALO_NG_ERROR_OFFLINECACHE_MSG2, errorCode);

	case PALO_NG_ERROR_NO_OFFLINECONNECTION:
		throw PaloNGTypeErrorException("No suitable connection in offline cache", "offline connection error", errorCode);

	case PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE:
		throw PaloNGTypeErrorException(PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE_MSG, PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE_MSG2, errorCode);

	case PALO_NG_ERROR_CERTIFIAKTION_FILE_NOT_LOADED:
		throw PaloNGTypeErrorException("Certificate file could not be loaded!", "Bad certificate file!", errorCode);

	case PALO_NG_ERROR_OPENSSL_LIBRARY_NOT_INITIALIZED:
		throw PaloNGTypeErrorException("OpenSSL Library not initialized! Call NetInitialisation::instance().initSSL().", "SSL not initialized!", errorCode);

	case PALO_NG_ERROR_SERVER_RESPONSE_SYNTACTICALLY_INVALID:
		throw PaloNGTypeErrorException(descr ? *descr : "Server response is syntactically invalid, boost::lexical_cast returned error.", "Server response syntactically invalid!", errorCode);

	case PALO_NG_ERROR_SERVER_NOT_FOUND:
		throw PaloNGTypeErrorException(descr ? *descr : "Server connection for specified identifier was not found.", "No such connection.", errorCode);

	case PALO_NG_ERROR_SERVER_ALIAS_EXISTS:
		throw PaloNGTypeErrorException(descr ? *descr : "This alias was already defined.", "Alias already defined.", errorCode);

	case PALO_NG_ERROR_WRONG_PARAMETER:
		throw PaloNGTypeErrorException(descr ? *descr : "Function received a wrong argument.", "Wrong argument received.", errorCode);

	case PALO_NG_ERROR_NEGOTIATION_FAILED:
		throw PaloNGTypeErrorException(descr ? *descr : "SSO automatic negotiation failed.", "SSO negotiation failed.", errorCode);

	case PALO_NG_ERROR_NO_VALUE_YET:
		throw PaloNGTypeErrorException("No value yet. (initial)", "No value yet. (initial)", errorCode);

	case PALO_NG_ERROR_NO_VALUE_YET_DUPLICATE:
		throw PaloNGTypeErrorException("No value yet. (duplicate)", "No value yet. (duplicate)", errorCode);

	default:
		throw PaloNGGeneralException();
	}
}
} /* palo */
} /* jedox */
