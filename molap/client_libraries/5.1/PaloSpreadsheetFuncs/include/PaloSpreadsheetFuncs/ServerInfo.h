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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * 
 *
 */

#ifndef SERVER_INFO_H
#define SERVER_INFO_H

#include <libpalo_ng/Palo/types.h>

namespace Palo {
namespace Types {

struct ServerInfo {

	UINT major_version;
	UINT minor_version;
	UINT bugfix_version;
	UINT build_number;
	UINT encryption;
	UINT httpsPort;
	UINT data_sequence_number;
	std::string sid;
	UINT ttl;

	ServerInfo(const jedox::palo::SERVER_INFO& si, const std::string &sid, unsigned int ttl) :
			major_version(si.major_version), minor_version(si.minor_version), bugfix_version(si.bugfix_version), build_number(si.build_number), encryption(si.encryption), httpsPort(si.httpsPort), data_sequence_number(si.data_sequence_number), sid(sid), ttl(ttl)
	{
	}

private:
	ServerInfo()
	{
	}
};

}
}
#endif
