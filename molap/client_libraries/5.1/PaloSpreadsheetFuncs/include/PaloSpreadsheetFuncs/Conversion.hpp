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

#pragma once

#include <string>

#include "Converter.hpp"

namespace jedox {
namespace i18n {

class ConversionException : public std::runtime_error {
public:
	ConversionException(const char * message);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class Conversion {
public:
	Conversion(const internal::Converter& from, const internal::Converter& to);

	std::string operator()(const std::string& toConvert, bool force_conversion = false); // force: convert, even if conversion is trivial

private:
	internal::Converter m_ConvertFrom;
	internal::Converter m_ConvertTo;
};

template<class From, class To>
class ConversionEx : public Conversion {
public:
	ConversionEx() :
			Conversion(From(), To())
	{
	}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef ConversionEx<internal::Converter, internal::ConverterUTF8> Local2UTF8Converter;
typedef ConversionEx<internal::ConverterUTF8, internal::Converter> UTF82LocalConverter;
typedef ConversionEx<internal::Converter, internal::Converter> NeutralConverter;

} /* namespace i18n */
} /* namespace jedox */
