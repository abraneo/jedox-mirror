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
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4275)
#endif

#ifndef NETINITIALISATION_H
#define NETINITIALISATION_H

#include <string>

#include <boost/noncopyable.hpp>

#if defined(WIN32) || defined(WIN64)
#undef ERROR
#undef ERROR_NOT_FOUND
#endif

namespace palo {

class NetInitialisation : private boost::noncopyable {
public:
	static NetInitialisation& instance();
	~NetInitialisation();

	void initSSL(std::string trustFile);
	void *getSslContext();
private:
	void *ctx;
	NetInitialisation();
};
} /* palo */
#endif							 // NETINITIALISATION_H
