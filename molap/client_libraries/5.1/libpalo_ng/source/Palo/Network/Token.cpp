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
 * \author Florian Schaper <florian.schaper@jedox.com>
 * 
 *
 */

#include <libpalo_ng/Palo/Network/Token.h>

namespace jedox {
namespace palo {

Token::Token(const std::string tokenName, unsigned int sequence, Type type) :
	m_Sequence(sequence), m_Type(type), m_TokenName(tokenName)
{
}

const std::string Token::getTokenName() const
{
	return m_TokenName;
}

unsigned int Token::getSequenceNumber() const
{
	return m_Sequence;
}

void Token::setSequenceNumber(unsigned int sequencenumber)
{
	m_Sequence = sequencenumber;
}

Token::Type Token::getType() const
{
	return m_Type;
}

} /* palo */
} /* jedox */
