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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace jedox {
namespace palo {

#define SERVERTOKENNAME     "x-palo-sv"
#define DATABASETOKENNAME   "x-palo-db"
#define DIMENSIONTOKENNAME  "x-palo-dim"
#define CUBETOKENNAME       "x-palo-cb"
#define CUBEDATATOKENNAME   "x-palo-cc"

class Token {
public:
	enum Type {
		SERVER, DATABASE, DIMENSION, CUBE
	};
	Token(const std::string tokenName, unsigned int sequence, Type type);

	const std::string getTokenName() const;

	unsigned int getSequenceNumber() const;

	void setSequenceNumber(unsigned int sequencenumber);

	Type getType() const;

	

private:
	unsigned int m_Sequence;
	Type m_Type;
	std::string m_TokenName;
	
};

typedef Token TOKEN;

} /* palo */
} /* jedox */
#endif							 // TOKEN_H
