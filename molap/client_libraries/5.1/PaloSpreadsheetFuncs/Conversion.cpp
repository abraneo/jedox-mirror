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
 * \author Florian Schaper <florian.schaper@jedox.com>
 *
 *
 */

#include <unicode/uenum.h>
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>
#include <boost/shared_array.hpp>

#include <PaloSpreadsheetFuncs/Conversion.hpp>

namespace jedox {
namespace i18n {

ConversionException::ConversionException(const char * message) :
		std::runtime_error(message)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Conversion::Conversion(const internal::Converter& from, const internal::Converter& to) :
		m_ConvertFrom(from), m_ConvertTo(to)
{
}

std::string Conversion::operator()(const std::string& toConvert, bool force_conversion)
{
	UErrorCode errorCode = U_ZERO_ERROR;
	if (!force_conversion && ucnv_compareNames(ucnv_getName(m_ConvertTo.get(), &errorCode), ucnv_getName(m_ConvertFrom.get(), &errorCode)) == 0) {
		return toConvert;
	}

	if (toConvert.size() == 0) {
		return toConvert;
	}
	errorCode = U_ZERO_ERROR;
	int32_t bufferSize = UCNV_GET_MAX_BYTES_FOR_STRING(toConvert.length(), ucnv_getMaxCharSize(m_ConvertTo.get()));
	boost::shared_array<char> targetBuffer(new char[bufferSize]);
	char * targetBufferPtr = targetBuffer.get();
	const char * sourcePtr = &*toConvert.begin();
	ucnv_convertEx(m_ConvertTo.get(), m_ConvertFrom.get(), &targetBufferPtr, targetBufferPtr + bufferSize, &sourcePtr, sourcePtr + toConvert.length(), NULL, NULL, NULL, NULL, TRUE, TRUE, &errorCode);
	if (U_FAILURE(errorCode)) {
		throw ConversionException("icu: conversion failed");
	}
	return std::string(targetBuffer.get(), targetBufferPtr);
}

} /* namespace i18n */
} /* namespace jedox */
