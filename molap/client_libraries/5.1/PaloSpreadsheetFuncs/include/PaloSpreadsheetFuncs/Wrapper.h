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
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef WRAPPER_H
#define WRAPPER_H

#include <libpalo_ng/Network/SocketException.h>
#include <libpalo_ng/Palo/Exception/PaloException.h>

#include <PaloSpreadsheetFuncs/GenericArgumentArray.h>

#include "SpreadsheetFuncsException.h"
#include "GenericCell.h"
#include "GenericContext.h"
#include "AbstractLogger.h"

#include "InvalidRequestException.h"
#include "WrongParamCountException.h"

namespace Palo {
namespace SpreadsheetFuncs {
/*! \author Marek Pikulski <marek.pikulski@jedox.com>
 *  \brief Wrapper class.
 *
 *  This can be used to implement a wrapper around SpreadsheetFuncs. In this case T should specify SpreadsheetFuncs.
 */
template<class T>
class Wrapper {
public:
	Wrapper<T>(AbstractLogger& log, T* thisptr, GenericContext& opts, void(T::*p)(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)) :
			fp(p), t(thisptr), opts(opts), log(log)
	{
	}

protected:
	void operator()(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
	{
		if (!fp)
			throw InvalidRequestException(CurrentSourceLocation);
		try {
			(t->*fp)(retval, opts, arg);
		} catch (const jedox::palo::PaloServerException& e) {
			log.log(AbstractLogger::LOG_WARNING, "Palo returned error: " + e.longDescription());
			retval.setError(e);
		} catch (const jedox::palo::PaloException& e) {
			log.log(AbstractLogger::LOG_WARNING, "libpalo_ng returned error: " + e.longDescription());
			retval.setError(e);
			// ZendValueException
			// ArgumentException
			// InvalidRequestException
			// QueryCacheEntryNotFoundException
			// SpreadsheetFuncsException
			// PHPPaloException
		} catch (const WrongParamCountException&) {
			// this is caught in SpreadsheetFuncsWrapper
			throw;
		} catch (const PSFException& e) {
			std::string msg = e.GetType() + ": " + e.GetMessage();
			log.log(AbstractLogger::LOG_WARNING, "Exception caught! " + msg);
			retval.setError(ErrorInfo(XLError::VALUExl, EXCEPTION_ERROR_CODE, msg));
		} catch (const jedox::palo::SocketException& e) {
			log.log(AbstractLogger::LOG_WARNING, "Socket exception! " + std::string(e.what()));
			retval.setError(ErrorInfo(XLError::NULLxl, SOCKET_EXCEPTION_ERROR_CODE, std::string(e.what())));
		} catch (const std::exception& e) {
			log.log(AbstractLogger::LOG_WARNING, "Unknown exception! " + std::string(e.what()));
			retval.setError(ErrorInfo(XLError::NULLxl, STD_EXCEPTION_ERROR_CODE, std::string(e.what())));
		} catch (...) {
			log.log(AbstractLogger::LOG_WARNING, "Unexpected exception!");
			retval.setError(ErrorInfo(XLError::NULLxl, STD_EXCEPTION_ERROR_CODE, "Unexpected exception!"));
		}
	}

	void(T::*fp)(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg);
	T* t;
	GenericContext& opts;
	AbstractLogger& log;
};
}
}
#endif
