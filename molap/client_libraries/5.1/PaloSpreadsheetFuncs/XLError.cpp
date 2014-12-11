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

#include <PaloSpreadsheetFuncs/XLError.h>

using namespace jedox::palo;
using namespace Palo::Types;

XLError::eXLError XLError::libpalo_ng_code2XLError(unsigned int code)
{
	switch (code) {
	/* sensless requests / type mismatch */
	case PaloExceptionFactory::ERROR_CUBE_EMPTY:
	case PaloExceptionFactory::ERROR_INVALID_ELEMENT_TYPE:
	case PaloExceptionFactory::ERROR_INVALID_OFFSET:
	case PaloExceptionFactory::ERROR_ELEMENT_NO_CHILD_OF:
	case PaloExceptionFactory::ERROR_DIMENSION_EMPTY:
	case PaloExceptionFactory::ERROR_ELEMENT_NAME_NOT_UNIQUE:
	case PaloExceptionFactory::ERROR_ELEMENT_CIRCULAR_REFERENCE:
	case PaloExceptionFactory::ERROR_AUTHORIZATION_FAILED:
	case PaloExceptionFactory::ERROR_INVALID_TYPE:
	case PaloExceptionFactory::ERROR_INVALID_COORDINATES:
	case PaloExceptionFactory::ERROR_INVALID_PERMISSION:
	case PaloExceptionFactory::ERROR_INVALID_SERVER_PATH:
	case PaloExceptionFactory::ERROR_WITHIN_EVENT:
	case PaloExceptionFactory::ERROR_NOT_WITHIN_EVENT:
	case PaloExceptionFactory::ERROR_INVALID_SESSION:
	case PaloExceptionFactory::ERROR_PARAMETER_MISSING:
	case PaloExceptionFactory::ERROR_INVALID_SPLASH_MODE:
	case PaloExceptionFactory::ERROR_DATABASE_NOT_LOADED:
	case PaloExceptionFactory::ERROR_DATABASE_UNSAVED:
	case PaloExceptionFactory::ERROR_DATABASE_STILL_LOADED:
	case PaloExceptionFactory::ERROR_DATABASE_UNDELETABLE:
	case PaloExceptionFactory::ERROR_DATABASE_UNRENAMABLE:
	case PaloExceptionFactory::ERROR_DIMENSION_UNCHANGABLE:
	case PaloExceptionFactory::ERROR_DIMENSION_IN_USE:
	case PaloExceptionFactory::ERROR_DIMENSION_UNDELETABLE:
	case PaloExceptionFactory::ERROR_DIMENSION_UNRENAMABLE:
	case PaloExceptionFactory::ERROR_INVALID_POSITION:
	case PaloExceptionFactory::ERROR_ELEMENT_NOT_DELETABLE:
	case PaloExceptionFactory::ERROR_ELEMENT_NOT_RENAMABLE:
	case PaloExceptionFactory::ERROR_CUBE_NOT_LOADED:
	case PaloExceptionFactory::ERROR_CUBE_UNSAVED:
	case PaloExceptionFactory::ERROR_SPLASH_DISABLED:
	case PaloExceptionFactory::ERROR_COPY_PATH_NOT_NUMERIC:
	case PaloExceptionFactory::ERROR_INVALID_COPY_VALUE:
	case PaloExceptionFactory::ERROR_CUBE_UNDELETABLE:
	case PaloExceptionFactory::ERROR_CUBE_UNRENAMABLE:
	case PaloExceptionFactory::ERROR_SPLASH_NOT_POSSIBLE:
	case PaloExceptionFactory::ERROR_RULE_NOT_FOUND:
	case PaloExceptionFactory::ERROR_RULE_HAS_CIRCULAR_REF:
	case PaloExceptionFactory::ERROR_RULE_DIVISION_BY_ZERO:
	case PaloExceptionFactory::ERROR_INVALID_VERSION:
		return VALUExl;

		/* invalid name / not found */
	case PaloExceptionFactory::ERROR_INVALID_FILENAME:
	case PaloExceptionFactory::ERROR_DATABASE_NOT_FOUND:
	case PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND:
	case PaloExceptionFactory::ERROR_CUBE_NOT_FOUND:
	case PaloExceptionFactory::ERROR_ELEMENT_NOT_FOUND:
	case PaloExceptionFactory::ERROR_NOT_AUTHORIZED:
	case PaloExceptionFactory::ERROR_INVALID_DATABASE_NAME:
	case PaloExceptionFactory::ERROR_INVALID_CUBE_NAME:
	case PaloExceptionFactory::ERROR_INVALID_DIMENSION_NAME:
	case PaloExceptionFactory::ERROR_INVALID_ELEMENT_NAME:
	case PaloExceptionFactory::ERROR_ELEMENT_EXISTS:
	case PaloExceptionFactory::ERROR_DIMENSION_EXISTS:
	case PaloExceptionFactory::ERROR_FILE_NOT_FOUND_ERROR:
	case PaloExceptionFactory::ERROR_WORKER_AUTHORIZATION_FAILED:
	case PaloExceptionFactory::ERROR_DATABASE_NAME_IN_USE:
	case PaloExceptionFactory::ERROR_DIMENSION_NAME_IN_USE:
	case PaloExceptionFactory::ERROR_ELEMENT_NAME_IN_USE:
	case PaloExceptionFactory::ERROR_CUBE_NAME_IN_USE:
	case PaloExceptionFactory::ERROR_INVALID_STRING:
	case PaloExceptionFactory::ERROR_INVALID_AGGR_FUNCTION:
		return NAMExl;

		/* internal / system / parse error  / all others*/
	default:
		return NULLxl;
	}
}
