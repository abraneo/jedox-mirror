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
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#ifndef OLAP_PASSWORD_H_
#define OLAP_PASSWORD_H_

#include "palo.h"

namespace palo {
////////////////////////////////////////////////////////////////////////////////
/// @brief OLAP password
///
/// An OLAP database consists of dimensions and cubes
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS Password {
public:
	const static int UNKNOWN_PWD_LEVEL = -1;
	const static int PLAIN_PWD_LEVEL = 0;
	const static int MAX_PWD_LEVEL = 1;
	const static int SAVED_PASSWORD_LEVEL = MAX_PWD_LEVEL;
	Password() : hashLevel(0) {}
	Password(const string &password, int level = UNKNOWN_PWD_LEVEL) {setPassword(password, level);}
	Password(const Password &o) : hashLevel(o.hashLevel), password(o.password) {}
	bool operator==(const Password &other) const;
	Password& operator=(const string& password) {setPassword(password); return *this;}
	Password& increaseLevel(int level);
	int getLevel() const {return hashLevel;}
	string getHashed() const {return password;}
private:
	void setPassword(const string &password, int level = UNKNOWN_PWD_LEVEL);
	int hashLevel;
	string password;
};

}
#endif /* OLAP_PASSWORD_H_ */
