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

#ifndef LIBPALONGEXCEPTIONFACTORY_H
#define LIBPALONGEXCEPTIONFACTORY_H

#include <exception>
#include <string>

#include <libpalo_ng/config_ng.h>
#include <libpalo_ng/Palo/Exception/ExceptionFactory.h>

#define PALO_NG_ERROR_OFFLINECACHE_MSG "A problem with the offline cache occured"
#define PALO_NG_ERROR_OFFLINECACHE_MSG2 "Offline cache error"
#define PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE_MSG "cell not found in datacache"
#define PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE_MSG2 "datacache problem"

namespace jedox {
namespace palo {

class LIBPALO_NG_CLASS_EXPORT LibPaloNGExceptionFactory : public ExceptionFactory {
public:

	enum ErrorType {

		// Errors reported by Libpalo_ng -- Error codes 100-999
		// are reserved for errors of this kind
		// PaloNGExceptions inherit from PaloExceptions

		//General undefined type error
		PALO_NG_ERROR_UNDEFINED_TYPE = 100,
		PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE = 101,
		PALO_NG_ERROR_UNDEFINED_DBINFOTYPE = 102,
		PALO_NG_ERROR_UNDEFINED_ELEMENTINFOTYPE = 103,
		PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE = 104,
		PALO_NG_ERROR_UNDEFINED_CUBEINFOTYPE = 105,
		PALO_NG_ERROR_UNDEFINED_CELLVALUEPATHTYPE = 106,
		PALO_NG_ERROR_ONLINECACHE = 107,
		PALO_NG_ERROR_OFFLINECACHE = 108,
		PALO_NG_ERROR_NO_OFFLINECONNECTION = 109,
		PALO_NG_ERROR_NOT_FOUND_IN_DATACACHE = 110,
		PALO_NG_ERROR_CERTIFIAKTION_FILE_NOT_LOADED = 111,
		PALO_NG_ERROR_OPENSSL_LIBRARY_NOT_INITIALIZED = 112,
		PALO_NG_ERROR_SUBSET = 113,
		PALO_NG_ERROR_SERVER_RESPONSE_SYNTACTICALLY_INVALID = 114,
		PALO_NG_ERROR_SERVER_NOT_FOUND = 115,
		PALO_NG_ERROR_SERVER_ALIAS_EXISTS = 116,
		PALO_NG_ERROR_WRONG_PARAMETER = 117,
		PALO_NG_ERROR_NEGOTIATION_FAILED = 118,
		PALO_NG_ERROR_NO_VALUE_YET = 119,
		PALO_NG_ERROR_NO_VALUE_YET_DUPLICATE = 120,

		// Exceptions without special handling
		PALO_NG_GENERAL_EXCEPTION = 999
	};

	static void raise(unsigned int errorCode, const std::string *descr = 0);
};

} /* palo */
} /* jedox */
#endif							 // LIBPALONGEXCEPTIONFACTORY_H
