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

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <iomanip>

#include "Olap/Password.h"
#include "Exceptions/ErrorException.h"
#include "Collections/StringUtils.h"
#include "Logger/Logger.h"

namespace palo {

//static bool passwordsTested = false;
//static void testPasswords()
//{
//	passwordsTested = true;
//	Password pwd("admin");
//	Logger::debug << "Hashed password: \"" <<  pwd.getHashed() << "\" level: " << pwd.getLevel() << endl;
//	Password pwd1 = pwd;
//	pwd1.increaseLevel(1);
//	Logger::debug << "Hashed password1: \"" <<  pwd1.getHashed() << "\" level: " << pwd1.getLevel() << endl;
//	bool test = pwd == pwd1;
//	Logger::debug << (test ? "Comparison test OK!" : "Comparison test failed!") << endl;
//	Password pwd2("\t1\t21232f297a57a5a743894a0e4a801fc3"); // serialized md5(level 1) form of the password "admin"
//	Logger::debug << "Hashed password2: \"" <<  pwd2.getHashed() << "\" level: " << pwd2.getLevel() << endl;
//	test = pwd == pwd2;
//	Logger::debug << (test ? "Comparison test OK!" : "Comparison test failed!") << endl;
//}

static string bin2Hex(const unsigned char *input, int length)
{
	stringstream str;
	str << hex << setfill('0');
	for (int i = 0; i < length; ++i) str << std::setw(2) << (int)input[i];
	return str.str();
}

bool Password::operator==(const Password &other) const
{
	// make sure we are comparing the same levels of hashing
	if (getLevel() == other.getLevel()) {
		return password == other.password;
	} else if (getLevel() < other.getLevel()) {
		Password hashedPassword(*this);
		hashedPassword.increaseLevel(other.getLevel());
		return hashedPassword == other;
	} else {
		Password hashedPassword(other);
		hashedPassword.increaseLevel(getLevel());
		return hashedPassword == *this;
	}
}

Password &Password::increaseLevel(int level)
{
	if (level < getLevel() || level > MAX_PWD_LEVEL) {
		throw ErrorException(ErrorException::ERROR_INTERNAL, "invalid hash level requested in Password::getHashed!");
	}
	// only possible level conversion in v 4.0 lvl_0->lvl_1 = plain -> MD5
	if (level > getLevel()) {
		unsigned char omd5buf[MD5_DIGEST_LENGTH];
		MD5((const unsigned char *)password.c_str(), password.size(), omd5buf);
		hashLevel++;
		password = "\t"+StringUtils::convertToString(hashLevel)+"\t"+bin2Hex((const unsigned char *)omd5buf, sizeof(omd5buf));
	}
	return *this;
}

void Password::setPassword(const string &password, int level)
{
	this->password = password;
	this->hashLevel = level;
	if (hashLevel == UNKNOWN_PWD_LEVEL) { // detect level from the text
		hashLevel = PLAIN_PWD_LEVEL;
		const char *str = password.c_str();
		if (*str == '\t') {
			str++;
			while (*str && isdigit(*str)) {
				hashLevel *= 10;
				hashLevel += *str-'0';
				str++;
			}
			if (*str != '\t') {
				hashLevel = PLAIN_PWD_LEVEL; // not a valid level code - set as plain password (level 0)
			}
		}
	}
//	if (!passwordsTested) {
//		testPasswords();
//	}
}

}
