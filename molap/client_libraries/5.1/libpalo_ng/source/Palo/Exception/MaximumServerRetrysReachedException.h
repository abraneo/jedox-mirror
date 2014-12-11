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

#ifndef MAXIMUMSERVERRETRYSREACHEDEXCEPTION_H
#define MAXIMUMSERVERRETRYSREACHEDEXCEPTION_H

#include <libpalo_ng/Palo/Exception/PaloServerException.h>

#include <libpalo_ng/Palo/Exception/LibPaloNGExceptionFactory.h>

#define MaximumServerRetrysReachedExceptionLong "Maximum number of retries to contact the server reached."
#define MaximumServerRetrysReachedExceptionShort "MaximumServerRetrysReachedException"

namespace jedox {
namespace palo {

class MaximumServerRetrysReachedException : public PaloException {
public:
	MaximumServerRetrysReachedException() :
		PaloException(MaximumServerRetrysReachedExceptionLong, MaximumServerRetrysReachedExceptionShort, LibPaloNGExceptionFactory::PALO_NG_GENERAL_EXCEPTION)
	{
	}
	MaximumServerRetrysReachedException(const std::string &lastmsg) :
		PaloException(MaximumServerRetrysReachedExceptionLong + ((!lastmsg.empty()) ? std::string("\nLast message was : ") + lastmsg : std::string("")), MaximumServerRetrysReachedExceptionShort, LibPaloNGExceptionFactory::PALO_NG_GENERAL_EXCEPTION)
	{
	}
};

} /* palo */
} /* jedox */
#endif							 // MAXIMUMSERVERRETRYSREACHEDEXCEPTION_H
