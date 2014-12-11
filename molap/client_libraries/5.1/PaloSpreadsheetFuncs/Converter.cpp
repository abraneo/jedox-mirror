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

#include <unicode/ucnv_err.h>
#include <unicode/uenum.h>
#include <unicode/ucnv.h>
#include <cassert>

#include <PaloSpreadsheetFuncs/Converter.hpp>

namespace jedox {
namespace i18n {

ConverterException::ConverterException(const char * message) :
		std::runtime_error(message)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace internal {
Converter::Converter() :
		m_Converter(NULL)
{
	init();
}

Converter::Converter(const std::string& lang) :
		m_Converter(NULL)
{
	init(lang.c_str());
}

Converter::Converter(const Converter& other) :
		m_Converter(NULL)
{
	UErrorCode error = U_ZERO_ERROR;
	m_Buffer.reset(new char[U_CNV_SAFECLONE_BUFFERSIZE]);
	int32_t bufferSize = U_CNV_SAFECLONE_BUFFERSIZE;
	m_Converter = ucnv_safeClone(other.m_Converter, m_Buffer.get(), &bufferSize, &error);
	if (m_Converter == NULL || U_FAILURE(error)) {
		throw ConverterException("icu: error during copy construction (ucnv_safeClone)");
	}
}

Converter::~Converter()
{
	if (m_Converter != NULL) {
		ucnv_close(m_Converter);
	}
	ucnv_flushCache();
}

void Converter::init(const char * converterName)
{
	UErrorCode code = U_ZERO_ERROR;
	m_Converter = ucnv_open(converterName, &code);
	if (m_Converter == NULL) {
		switch (code) {
		case U_MEMORY_ALLOCATION_ERROR:
			throw ConverterException("icu: memory allocation error");
			break;

		case U_FILE_ACCESS_ERROR:
			throw ConverterException("icu: file access error");
			break;

		default:
			throw ConverterException("icu: unknown error");
			break;
		}
	}
}

UConverter * Converter::get()
{
	return m_Converter;
}

ConverterUTF8::ConverterUTF8() :
		Converter("UTF8")
{
}
} /* namespace internal */

} /* namespace i18n */

} /* namespace jedox */
