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

/* Palo Exception is the exception class all other exceptions inherit from.  */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#ifndef PALOEXCEPTION_H
#define PALOEXCEPTION_H

#include <exception>
#include <string>

namespace jedox {
namespace palo {

class PaloException : public std::exception {
public:
	explicit PaloException(unsigned int code) :
		m_Code(code), m_LongText(""), m_ShortText(""), m_whatText(Description())
	{
	}

	PaloException(const std::string& longText, const std::string& shortText, unsigned int code) :
		m_Code(code), m_LongText(longText), m_ShortText(shortText), m_whatText(Description())
	{
	}

	~PaloException() throw ()
	{
	}

	unsigned int code() const
	{
		return m_Code;
	}

	const std::string& longDescription() const
	{
		return m_LongText;
	}

	const std::string& shortDescription() const
	{
		return m_ShortText;
	}

	const std::string Description() const
	{
		const std::string& msg = longDescription();
		if (msg.empty()) {
			return shortDescription();
		} else {
			return shortDescription() + " : " + msg;
		}
	}

	virtual const char* what() const throw ()
	{
		return m_whatText.c_str();
	}
	// needed by serialization lib
	std::string getName()
	{
		return m_LongText;
	}

private:
	unsigned int m_Code;
	std::string m_LongText;
	std::string m_ShortText;
	std::string m_whatText;
};

} /* palo */
} /* jedox */
#endif							 // PALOEXCEPTION_H
